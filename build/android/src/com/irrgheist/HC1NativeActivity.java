// Copyright (C) Michael Zeilfelder
// The advertisement code is from Patryk Nadrowski.
// This file is unter the zlib license.
//
// Admob sdk is unfortunately only available for Java. Otherwise this would all be a lot easier.
// I really hope google makes a native admob sdk one day so we can kick all that stuff out.

package com.irrgheist.hcraft_championship;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;

import android.view.WindowManager;
import android.view.View;

public class HC1NativeActivity extends android.app.NativeActivity {

    static 	{
		// Current loader of Android fails to load our own shared libraries for a native application.
		// So we're loading them manually.		
		System.loadLibrary("openal");
		System.loadLibrary("freealut");
    }
	
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

		mAdHelper = new AdHelper(this);
		mBillingHelper = new BillingHelper(this);

		getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
		
		hideSystemUI() ;
   }
   
	@Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);
        if (hasFocus) {
			hideSystemUI();
        }
    }   

	@Override
	public void onResume()
	{
		super.onResume();

		if (mAdHelper != null)
			mAdHelper.resume();
	}

	@Override
	public void onPause()
	{
		if (mAdHelper != null)
			mAdHelper.pause();

		super.onPause();
	}

	@Override
	public void onDestroy()
	{
		if (mBillingHelper != null)
			mBillingHelper.dispose();

		super.onDestroy();
	}

	@Override
	protected void onActivityResult(int requestCode, int resultCode, Intent data)
	{
		super.onActivityResult(requestCode, resultCode, data);
		
		if (mBillingHelper != null)
			mBillingHelper.onActivityResult(requestCode, resultCode, data);		
	}

	public AdHelper getAdHelper() 
	{
		return mAdHelper;
	}
	
	public BillingHelper getBillingHelper()
	{
		return mBillingHelper;
	}
	
	private void hideSystemUI()
	{
		// Get rid of taskbar buttons etc.
		if ( android.os.Build.VERSION.SDK_INT >= 19 )
		{
			getWindow().getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_LAYOUT_STABLE
								| View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
								| View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
								| View.SYSTEM_UI_FLAG_HIDE_NAVIGATION 
								| View.SYSTEM_UI_FLAG_FULLSCREEN 
								| View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY);
		}
    }	
	
	private AdHelper mAdHelper = null;
	private BillingHelper mBillingHelper = null;
 }
 