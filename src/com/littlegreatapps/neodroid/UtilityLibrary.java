package com.littlegreatapps.neodroid;

import java.io.File;

import fr.mydedibox.utility.Utility;

import android.annotation.SuppressLint;
import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager.NameNotFoundException;

public class UtilityLibrary 
{
	public static final String LIBRARY_PACKAGE = "com.littlegreatapps.neodroid";
	
	private ApplicationInfo mApplication = null;
	private String mLibSDLPath = null;
	private String mLibMainASMPath = null;
	private String mLibMainPath = null;
	private boolean isAvailable = false;
	
	@SuppressLint("NewApi")
	public UtilityLibrary( Context pCtx )
	{
		//try 
		//{
			mApplication = pCtx.getApplicationInfo();
		//} 
		//catch (NameNotFoundException e) 
		//{
		//	e.printStackTrace();
		//	Utility.loge( "Library package not installed" );
		//	return;
		//}
		

		final int currentapiVersion = android.os.Build.VERSION.SDK_INT;
		if(currentapiVersion <= android.os.Build.VERSION_CODES.FROYO)
		{
			mLibSDLPath = mApplication.dataDir + "/lib/libSDL.so";
			mLibMainPath = mApplication.dataDir + "/lib/libmain.so";
			mLibMainASMPath = mApplication.dataDir + "/lib/libmain_asm.so";
		} 
		else
		{
			mLibSDLPath = mApplication.nativeLibraryDir + "/libSDL.so";
			mLibMainPath = mApplication.nativeLibraryDir + "/libmain.so";
			mLibMainASMPath = mApplication.nativeLibraryDir + "/libmain_asm.so";
		}
		
		Utility.log( mLibSDLPath );
		Utility.log( mLibMainPath );
		Utility.log( mLibMainASMPath );
		
		File main_asm = new File( mLibMainASMPath );
		File main = new File( mLibMainPath );
		File sdl = new File( mLibMainPath );
		if( main_asm.exists() && main.exists() && sdl.exists() )
			isAvailable = true;
	}
	
	public boolean isLibsAvailable()
	{
		return this.isAvailable;
	}
	public String getLibMainASMPath()
	{
		return this.mLibMainASMPath;
	}
	public String getLibMainPath()
	{
		return this.mLibMainPath;
	}
	public String getLibSDLPath()
	{
		return this.mLibSDLPath;
	}
}

