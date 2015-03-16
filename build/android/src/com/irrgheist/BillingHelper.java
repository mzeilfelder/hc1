// Originally based on code which was copyright 2011-2014 by Patryk Nadrowski
// and released with zlib license. 
// But so many modifications by Michael Zeilfelder that original is probably no longer recognizable.
// Note that Patryk's code was likely better for the general case, this code does only the minimum
// I needed for h-craft championship.

package com.irrgheist.hcraft_championship;
 
import android.app.Activity;
import android.app.NativeActivity;
import android.content.Intent;
import android.os.Bundle;
import android.view.WindowManager;
import android.net.Uri;
import android.widget.LinearLayout;
import android.widget.PopupWindow;
import android.view.View;
import android.view.ViewGroup.LayoutParams;
import android.view.ViewGroup.MarginLayoutParams;
import android.view.Gravity;
import android.content.ComponentName;
import android.content.Context;
import android.content.ServiceConnection;
import android.os.IBinder;
import org.json.JSONObject;
import org.json.JSONException;
import java.util.ArrayList;
import android.app.PendingIntent;
import android.content.IntentSender.SendIntentException;
import android.os.RemoteException;
import android.util.Log;
import java.util.Random;

import com.android.vending.billing.IInAppBillingService;
 
public class BillingHelper
{
	public BillingHelper(Activity activity)
	{
		mActivity = activity;

		if (mActivity != null)
			mPackagePrefix = mActivity.getPackageName() + ".";
	}

	// Add product id's for which we want to get the items
	// This is the name you used in in-app products in the google developer console.
	// This must be done before the call to requestServerConnection
	public void registerExpectedProduct(String productId)
	{
		// Log.d("Irrlicht", "registerExpectedProduct:" + productId );		
		String packageProductId = productId.replace(mPackagePrefix, "");
		mExpectedObject.add(packageProductId);
		
		if ( mSynchronizeState == STATE_ERROR_NO_PRODUCT_EXPECTED )
			mSynchronizeState = STATE_UNCONNECTED;
	}
	
	public int getSynchronizeState()
	{
		return mSynchronizeState;
	}

	public int getNumAvailableItems()
	{
		return mAvailableItems.size();
	}
	
	public String getItemName(int index)
	{
		return mAvailableItems.get(index).getName();
	}

	public String getItemPrice(int index)
	{
		return mAvailableItems.get(index).getPrice();
	}
	
	
	public int getNumPurchasedItems()
	{
		return mPurchasedItems.size();
	}
	
	public String getPurchasedItem(int index)
	{
		return mPurchasedItems.get(index);
	}

	// Create the connection to the google-play billing service
	// Once successful do get a list of all available items
	public void requestServerConnection()
	{
		if ( mExpectedObject.isEmpty() )
		{
			mSynchronizeState = STATE_ERROR_NO_PRODUCT_EXPECTED;
			return;
		}
		
		if (mActivity == null || mSynchronizeState != STATE_UNCONNECTED)
			return;
			
		mSynchronizeState = STATE_CONNECTING;
	
		mInAppBillingServiceConnection = new ServiceConnection()
		{
			// Callbacks, they will be called after the bindService from below has run.
			// Note they can be run several times (if service disconnects in between) and the mInAppBillingServiceConnection is not destroyed automatically when there is a disconnect in between.
			@Override public void onServiceDisconnected(ComponentName name)
			{
				mSynchronizeState = STATE_UNCONNECTED;
				mInAppBillingService = null;
			}

			@Override public void onServiceConnected(ComponentName name, IBinder service)
			{
				mAvailableItems.clear();
				mPurchaseQuery.clear();
				
				mInAppBillingService = IInAppBillingService.Stub.asInterface(service);
				
				// Check for API support
				try 
				{
					int response = mInAppBillingService.isBillingSupported(API_VERSION, mActivity.getPackageName(), "inapp");
					if 	(response != RESULT_OK) 
					{
						dispose(STATE_ERROR_BILLING_UNSUPPORTED);
						return;
					}
				}
				catch (RemoteException err)
				{
					dispose(STATE_ERROR_REMOTE_EXCEPTION);
				}

				new Thread(new Runnable()
				{
					@Override public void run()
					{
						if (mInAppBillingService == null)
							return;

						try
						{
							// Get a list of all items which are available for buying
							
							Bundle expected = new Bundle();
							expected.putStringArrayList("ITEM_ID_LIST", mExpectedObject);

							Bundle requestSkuDetail = mInAppBillingService.getSkuDetails(API_VERSION, mActivity.getPackageName(), "inapp", expected);

							if (requestSkuDetail.getInt("RESPONSE_CODE") == RESULT_OK)
							{
								ArrayList<String> responseList = requestSkuDetail.getStringArrayList("DETAILS_LIST");

								if (responseList != null)
								{
									for (String thisResponse : responseList)
									{
										JSONObject object = new JSONObject(thisResponse);

										Item item = new Item(object.getString("productId"), object.getString("price"), "");
										mAvailableItems.add(item);
									}
								}
							}
							else
							{
								dispose(STATE_ERROR_REQUEST_ITEM_ID_LIST);
								return;
							}
							
				
							// Get all purchases user owns already
							Bundle requestPurchases = mInAppBillingService.getPurchases(API_VERSION, mActivity.getPackageName(), "inapp", null);

							if (requestPurchases.getInt("RESPONSE_CODE") == RESULT_OK)
							{
								ArrayList<String> responseList = requestPurchases.getStringArrayList("INAPP_PURCHASE_DATA_LIST");

								if (responseList != null)
								{
									for (String thisResponse : responseList)
									{
										JSONObject object = new JSONObject(thisResponse);

										String objectName = object.getString("productId");
										//String objectToken = object.getString("purchaseToken");
										
										if ( objectName.equals("android.test.purchased") )
										{
											// Only way to get rid of test-buys - consume them again.											
											String purchaseToken = "inapp:" + mActivity.getPackageName() + ":android.test.purchased";
											int response = mInAppBillingService.consumePurchase(API_VERSION, mActivity.getPackageName(), purchaseToken);
										}
										else
										{
											Log.d("Irrlicht", "* Add item user already had: " + objectName);
											mPurchasedItems.add(objectName);
										}
									}
								}
							}
							else
							{
								dispose(STATE_ERROR_REQUEST_INAPP_PURCHASE_DATA_LIST);
								return;
							}
							
						}
						catch (RemoteException err)
						{
							dispose(STATE_ERROR_REMOTE_EXCEPTION);
						}
						catch (JSONException err)
						{
							dispose(STATE_ERROR_JSON_EXCEPTION);
						}
						
						// Yay
						mSynchronizeState = STATE_SYNCHRONIZED;
					}
				}).start();
			}
		};

		Intent InAppBillingServiceIntent = new Intent("com.android.vending.billing.InAppBillingService.BIND");
		InAppBillingServiceIntent.setPackage("com.android.vending");
		mActivity.bindService(InAppBillingServiceIntent, mInAppBillingServiceConnection, Context.BIND_AUTO_CREATE);
	}

	// Close down the connection to the google-play billing service
	public void dispose()
	{
		dispose(STATE_UNCONNECTED);
	}
	
	// Close down the connection to the google-play billing service
	public void dispose(int error_state)
	{
		if (mInAppBillingServiceConnection != null && mActivity != null)
		{
			mActivity.unbindService(mInAppBillingServiceConnection); 
			mInAppBillingServiceConnection = null;
		}
		mSynchronizeState = error_state;
		mInAppBillingService = null;
	}

	// Ask to buy an item
	public void requestPurchase(String name)
	{
		if (mSynchronizeState != STATE_SYNCHRONIZED)
			return;

		try
		{
			mSynchronizeState = STATE_REQUESTING;
			
			String thisName = name.replace(mPackagePrefix, "");

			String code = generateCode(name);

			Bundle request = mInAppBillingService.getBuyIntent(API_VERSION, mActivity.getPackageName(), thisName, "inapp", code);

			if (request.getInt("RESPONSE_CODE") == RESULT_OK)
			{
				Item item = new Item(thisName, "", code);
				mPurchaseQuery.add(item);

				PendingIntent pendingIntent = request.getParcelable("BUY_INTENT");

				// result will be in onActivityResult (in the activity class)
				mActivity.startIntentSenderForResult(pendingIntent.getIntentSender(), REQUEST_CODE, new Intent(), Integer.valueOf(0), Integer.valueOf(0), Integer.valueOf(0));
			}
			else
			{
				Log.d("Irrlicht", "requestPurchase error, RESPONSE_CODE is " + Integer.toString(request.getInt("RESPONSE_CODE")));
				dispose(STATE_ERROR_REQUEST_RESPONSE_CODE);
			}
		}
		catch (SendIntentException err)
		{
			dispose(STATE_ERROR_SEND_INTENT_EXCEPTION);
		}
		catch (RemoteException err)
		{
			dispose(STATE_ERROR_REMOTE_EXCEPTION);
		}
	}

	// Result for requestPurchase. So tells if the purchase did go through
	// Note: This works because our activity class passes the result back in here
	public void onActivityResult(int requestCode, int resultCode, Intent data)
	{ 
		if (requestCode == REQUEST_CODE)
		{           
			String purchaseData = data.getStringExtra("INAPP_PURCHASE_DATA");

			if (resultCode == Activity.RESULT_OK)	// not to confuse with RESULT_OK from this class! (not even same value)
			{
				// Log.d("Irrlicht", "onActivityResult, result is RESULT_OK");
				
				try
				{
					JSONObject object = new JSONObject(purchaseData);

					String name = object.getString("productId");
					String code = object.getString("developerPayload");

					processPurchase(name, code);
				}
				catch (JSONException err)
				{
					dispose(STATE_ERROR_JSON_EXCEPTION);
				}
			}
			else // Activity.RESULT_CANCELED 
			{
				// Log.d("Irrlicht", "onActivityResult result RESULT_CANCELED ");
			}
			mSynchronizeState = STATE_SYNCHRONIZED;			
		}
	}

	// Result of a purchase 
	// \param name Item name
	// \param code Some application provided string (from generateCode)
	private void processPurchase(final String name, final String code)
	{
		for (int i = 0; i < mPurchaseQuery.size(); i++)
		{
			Item item = mPurchaseQuery.get(i);

			// Is it a purchase for which we asked for?
			if (item.getName().equals(name) && item.getCode().equals(code))
			{
				Log.d("Irrlicht", "* add the purchased item: " + name);
				mPurchasedItems.add(name);
				mPurchaseQuery.remove(i);

				return;
			}
		}
		dispose(STATE_ERROR_ITEM_NOT_REQUESTED);
	}
	
	private String generateCode(String name)
	{
		// String is used to compare if the requested object is the one which got bought. Should be random. but probably doesn't matter in my case 
		// (worst case as far as I can see it - someone sends a fake answer to get a free game).
		return "code" + name;
	}

	// google play billing api version to use
	private static final int API_VERSION = 3;	
	
	// Just a number which will be the same on buy-request and in the result in onActivityResult. Must be > 0
	private static final int REQUEST_CODE = 1001;
	
	// Synchronization states of BillingHelper
	public static final int STATE_SYNCHRONIZED = 0;		
	public static final int STATE_UNCONNECTED = 1;
	public static final int STATE_CONNECTING = 2;
	public static final int STATE_REQUESTING = 3;	
	public static final int STATE_ERROR = -1;	// general error
	public static final int STATE_ERROR_UNSUPPORTED = -2;
	public static final int STATE_ERROR_ITEM_NOT_REQUESTED = -3;
	public static final int STATE_ERROR_BILLING_UNSUPPORTED	= -4;
	public static final int STATE_ERROR_NO_PRODUCT_EXPECTED = -5;
	public static final int STATE_ERROR_REMOTE_EXCEPTION = -10;
	public static final int STATE_ERROR_JSON_EXCEPTION = -11;
	public static final int STATE_ERROR_SEND_INTENT_EXCEPTION = -12;
	public static final int STATE_ERROR_REQUEST_ITEM_ID_LIST = -20;
	public static final int STATE_ERROR_REQUEST_INAPP_PURCHASE_DATA_LIST = -21;
	public static final int STATE_ERROR_REQUEST_RESPONSE_CODE = -22;
	public static final int STATE_ERROR_INAPP_PURCHASE_DATA = -23;
	
    // Billing response codes
    private static final int RESULT_OK = 0;
    private static final int RESULT_USER_CANCELED = 1;
	private static final int RESULT_SERVICE_UNAVAILABLE = 2;
    private static final int RESULT_BILLING_UNAVAILABLE = 3;
    private static final int RESULT_ITEM_UNAVAILABLE = 4;
    private static final int RESULT_DEVELOPER_ERROR = 5;
    private static final int RESULT_ERROR = 6;
    private static final int RESULT_ITEM_ALREADY_OWNED = 7;
    private static final int RESULT_ITEM_NOT_OWNED = 8;	
	
	private int mSynchronizeState = STATE_UNCONNECTED;	

	private IInAppBillingService mInAppBillingService = null;
	private ServiceConnection mInAppBillingServiceConnection = null;

	private ArrayList<String> mExpectedObject = new ArrayList<String>();   // product id's for which we want to get items
	private ArrayList<Item> mAvailableItems = new ArrayList<Item>();       // items available for buying
	private ArrayList<Item> mPurchaseQuery = new ArrayList<Item>();        // request to buy this item is on it's way to the server
	private ArrayList<String> mPurchasedItems = new ArrayList<String>();     // names of items already bought

	private Activity mActivity = null;

	private String mPackagePrefix = "";
	
	// Items which can be purchased
	private class Item
	{
		public Item(String name, String price, String code)
		{
			mName = name;
			mPrice = price;
			mCode = code;
		}

		public String getName()
		{
			return mName;
		}

		public String getPrice()
		{
			return mPrice;
		}

		// custom code generated by app when requesting a purchase (can be compared on receiving for security)
		public String getCode()
		{
			return mCode;
		}

		private String mName = "";
		private String mPrice = "";
		private String mCode = "";
	}
}
