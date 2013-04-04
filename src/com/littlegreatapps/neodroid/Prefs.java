package com.littlegreatapps.neodroid;

import android.content.Context;
import android.content.SharedPreferences;
import android.os.Environment;
import android.preference.PreferenceManager;

public class Prefs
{
	public static String ROM_PATH = Environment.getExternalStorageDirectory().getAbsolutePath() + "/neodroid/roms";
	public static String DATA_PATH = Environment.getExternalStorageDirectory().getAbsolutePath() + "/neodroid";
	
	private final Context mCtx;
	private SharedPreferences mPrefs;
	private SharedPreferences.Editor mEditor;

	public Prefs( Context pCtx ) 
	{
		this.mCtx = pCtx;
		this.mPrefs = PreferenceManager.getDefaultSharedPreferences(pCtx);
		this.mEditor = this.mPrefs.edit();
	}
	
	public SharedPreferences getSharedPreferences()
	{
		return this.mPrefs;
	}
	
	public String getDataFilePath()
	{
		return DATA_PATH + "/gngeo_data.zip";
		//return this.mCtx.getFilesDir().getAbsolutePath() + "/gngeo_data.zip";
	}
	
	public boolean useAsm()
	{
		return this.mPrefs.getBoolean( "useasm", false );
	}
	public void useAsm( boolean pValue )
	{
		this.mEditor.putBoolean( "useasm", pValue );
		this.mEditor.commit();
	}
	public String getCountry()
	{
		return this.mPrefs.getString( "country", "europe" );
	}
	public String getSystem()
	{
		return this.mPrefs.getString( "system", "arcade" );
	}
	public boolean getSound()
	{
		return this.mPrefs.getBoolean( "sound", true );
	}
	public String getSamplerate()
	{
		return this.mPrefs.getString( "samplerate", "22050" );
	}
	public boolean getInterpolation()
	{
		return this.mPrefs.getBoolean( "interpolation", false );
	}
	public boolean getRaster()
	{
		return this.mPrefs.getBoolean( "raster", false );
	}
	public boolean getShowfps()
	{
		return this.mPrefs.getBoolean( "showfps", false );
	}
	public boolean getSleepidle()
	{
		return this.mPrefs.getBoolean( "sleepidle", false );
	}
	public boolean getVsync()
	{
		return this.mPrefs.getBoolean( "vsync", false );
	}
	public boolean getAutoframeskip()
	{
		return this.mPrefs.getBoolean( "autoframeskip", true );
	}
	public boolean getForcepc()
	{
		return this.mPrefs.getBoolean( "forcepc", false );
	}
	public String getZ80Clock()
	{
		return String.valueOf(this.mPrefs.getInt( "z80clock", 0 ));
	}
	public String get68kClock()
	{
		return String.valueOf(this.mPrefs.getInt( "68kclock", 0 ));
	}
}

