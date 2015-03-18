// Copyright (C) 2011-2014 Patryk Nadrowski.
// This file was originally part of the "Chernobyl Game Engine", but 
// is now put unter the zlib license with the friendly allowance of Patryk Nadrowski.
// All kind of modifications by Michael Zeilfelder

package com.irrgheist.hcraft_championship;
 
import android.app.Activity;
import android.view.View;
import android.view.ViewGroup.LayoutParams;
import android.view.ViewGroup.MarginLayoutParams;
import android.view.Gravity;
import android.widget.PopupWindow;
import android.widget.RelativeLayout;
import android.util.Log;
import java.util.ArrayList;

import com.google.android.gms.ads.InterstitialAd;
import com.google.android.gms.ads.AdListener;
import com.google.android.gms.ads.AdRequest;
import com.google.android.gms.ads.AdSize;
import com.google.android.gms.ads.AdView;
 
public class AdHelper
{
	public AdHelper(Activity activity)
	{
		mActivity = activity;
	}

	public void resume()
	{
		if (mState[0] != -1)
			mAdView[0].resume();

		if (mState[1] != -1)
			mAdView[1].resume();
	}

	public void pause()
	{
		if (mState[0] != -1)
			mAdView[0].pause();

		if (mState[1] != -1)
			mAdView[1].pause();
	}

	public void request(final int type)
	{
		//Log.d("Irrlicht", "AD request");
		if (mActivity != null && mState[type] != 0 && mAdUnitID[type].length() != 0)
		{
			//Log.d("Irrlicht", "AD state and unit ok");
			
			mState[type] = 0;
			
			AdListener adListener = new AdListener()
			{
				@Override
				public void onAdLoaded()
				{
					Log.d("Irrlicht", "*** onAdLoaded ***");
					mState[type] = 1;
				}

				@Override
				public void onAdFailedToLoad(int errorCode)
				{
					Log.d("Irrlicht", "*** onAdFailedToLoad *** error=" + errorCode);
					remove(type);
				}

				@Override
				public void onAdOpened()
				{
					//Log.d("Irrlicht", "*** onAdOpened ***");
				}

				@Override
				public void onAdClosed()
				{
					//Log.d("Irrlicht", "*** onAdClosed ***");
				}

				@Override
				public void onAdLeftApplication()
				{
					//Log.d("Irrlicht", "*** onAdLeftApplication ***");
				}
			};

			if (!isInterstitialType(type))
			{
				//Log.d("Irrlicht", "AD !isInterstitialType");
				mAdView[type] = new AdView(mActivity);
		    	mAdView[type].setAdUnitId(mAdUnitID[type]);
		    	mAdView[type].setAdSize(AdSize.SMART_BANNER);
				mAdView[type].setLayoutParams(new RelativeLayout.LayoutParams(LayoutParams.MATCH_PARENT, LayoutParams.WRAP_CONTENT));
				mAdView[type].setAdListener(adListener);

				mActivity.runOnUiThread(new Runnable()
				{
					@Override
					public void run()
					{
						mLayout[type] = new RelativeLayout(mActivity);
						mLayout[type].addView(mAdView[type]);

						mPopUp[type] = new PopupWindow(mActivity);
						mPopUp[type].setClippingEnabled(false);
						mPopUp[type].setBackgroundDrawable(null);
						mPopUp[type].setWindowLayoutMode(LayoutParams.MATCH_PARENT, LayoutParams.WRAP_CONTENT);
						mPopUp[type].setContentView(mLayout[type]);

						AdRequest.Builder builder = new AdRequest.Builder();
						builder = builder.addTestDevice(AdRequest.DEVICE_ID_EMULATOR);        // All emulators
						for (String device : mTestDevices)
						{
							builder = builder.addTestDevice(device);
						}
						AdRequest request = builder.build();
						mAdView[type].loadAd(request);
						mAdView[type].setEnabled(false);
						mAdView[type].setVisibility(View.INVISIBLE);
					}
				});
			}
			else
			{
				Log.d("Irrlicht", "AD isInterstitialType");
				
				mInterstitial = new InterstitialAd(mActivity);
		    	mInterstitial.setAdUnitId(mAdUnitID[type]);
				mInterstitial.setAdListener(adListener);
				
				mActivity.runOnUiThread(new Runnable()
				{
					@Override
					public void run()
					{
						Log.d("Irrlicht", "*** loadAd ***");					
						
						AdRequest.Builder builder = new AdRequest.Builder();
						builder = builder.addTestDevice(AdRequest.DEVICE_ID_EMULATOR);        // All emulators
						for (String device : mTestDevices)
						{
							builder = builder.addTestDevice(device);
						}
						AdRequest request = builder.build();
						mInterstitial.loadAd(request);
					}
				});
			}
		}
	}
		
	public void remove(final int type)
	{
		if (mState[type] == 0 || mState[type] == 1 || mState[type] == 2)
		{
			final int originalState = mState[type];
			mState[type] = -1;

			if (!isInterstitialType(type))
			{
				mActivity.runOnUiThread(new Runnable()
				{
					@Override
					public void run()
					{
						if (originalState == 2)
							mPopUp[type].dismiss();

						mPopUp[type].setContentView(null);
						mLayout[type].removeView(mAdView[type]);

						mAdView[type].destroy();
						mAdView[type] = null;
					}
				});
			}
		}
	}

	public boolean show(final int type)
	{
		boolean status = false;

		if (mState[type] == 1)
		{
			//Log.d("Irrlicht", "AD show state was 1");
			mState[type] = 2;

			if (!isInterstitialType(type))
			{
				//Log.d("Irrlicht", "AD show !isInterstitialType");
				mActivity.runOnUiThread(new Runnable()
				{
					@Override
					public void run()
					{
						mAdView[type].setEnabled(true);
				    	mAdView[type].setVisibility(View.VISIBLE);

						int gravity = (type == 0) ? Gravity.BOTTOM : Gravity.TOP;

						mPopUp[type].showAtLocation(mActivity.getWindow().getDecorView(), gravity, 0, 0);
						mPopUp[type].update();
						Log.d("Irrlicht", "AD show trying the popup thing");
					}
				});
			}
			else
			{
				mActivity.runOnUiThread(new Runnable()
				{
					@Override
					public void run()
					{
						if (mInterstitial.isLoaded())
						{
							try
							{
								mInterstitial.show();
							}
							catch (java.lang.IllegalStateException e)
							{
								// TOOD: Not sure yet how I get this state. Most likely because the next ad is requested already
								// before this thread is run and so ads are in an inconsistent state.
								Log.d("Irrlicht", "*** Ad does not want to be shown - give up before it crashes.");
							}
						}
					}
				});
			}

			status = true;
		}
		else if (mState[type] == 2)
		{
			Log.d("Irrlicht", "AD show state was 2");
			
		    status = true;
		}

		return status;
	}

	public void setAdUnitID(String id, int type)
	{
		if (type <= 2)
			mAdUnitID[type] = id;
	}
	
	public void addTestDevice(String device)
	{
		mTestDevices.add(device);
	}

	private boolean isInterstitialType(int type)
	{
		return (type == 2);
	}

	private String[] mAdUnitID = new String[] { "", "", "" };

	//! State advertisement (-1 no ad, 0 during creation, 1 available, (not visible), 2 available (visible)
	private int[] mState = new int[] { -1, -1, -1 };

	private PopupWindow[] mPopUp = new PopupWindow[] { null, null };
	private RelativeLayout[] mLayout = new RelativeLayout[] { null, null };

	private AdView[] mAdView = new AdView[] { null, null };	// only 2 types using AdView, fullscreen is InterstitialAd
	private InterstitialAd mInterstitial = null;

	private Activity mActivity = null;
	
	private ArrayList<String> mTestDevices = new ArrayList<String>();   // id's of test-devices (show test-ad's which you should always do for testing).
}

