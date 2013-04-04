package com.littlegreatapps.neodroid;

import java.io.File;

import android.os.Bundle;
import fr.mydedibox.emufrontend.CompatibilityList;
import fr.mydedibox.emufrontend.EmuFrontendActivity;
import fr.mydedibox.emufrontend.FileInfo;
import fr.mydedibox.emufrontend.SDLActivity;
import fr.mydedibox.utility.Utility;
import fr.mydedibox.utility.UtilityExtractRaw;
import fr.mydedibox.utility.UtilityFrontendPreferences;
import fr.mydedibox.utility.UtilityLibs;

public class ActivityMain extends EmuFrontendActivity
{
	private UtilityLibrary libs;
	private Prefs mNeoPrefs;

	@Override
	protected void onCreate(Bundle savedInstanceState) 
	{
		Utility.setTAG( "NeoDroid" );
		UtilityFrontendPreferences.DATA_PATH = Prefs.DATA_PATH;
		UtilityFrontendPreferences.ROM_PATH = Prefs.ROM_PATH;
		UtilityFrontendPreferences.STATE_PATH = Prefs.DATA_PATH;
		
		this.mCompatList = new CompatibilityList( new Compatibility().getList() );
		mNeoPrefs = new Prefs( this );
		
		super.onCreate(savedInstanceState);	
	}
	
	@Override 
	public void onResume()
	{
		super.onResume();
		
		libs = new UtilityLibrary( this );
		
		/*
		if( ! libs.isLibsAvailable() )
		{
			dialogConfirmDownloadLibs( );
			return;
		}
		else
		{
			if( mPrefs.updatePrefs() )
			{
	        	this.mMessage.showMessageInfo( "\nA new version of NeoDroid was installed, " +
	        			"to prevent any problem with the new update your preferences have been reseted (sorry for that)\n" );
			}
		}
		*/
		
		if( mPrefs.updatePrefs() )
		{
        	this.mMessage.showMessageInfo( "\nA new version of NeoDroid was installed, " +
        			"to prevent any problem with the new update your preferences have been reseted (sorry for that)\n" );
		}
		extractDataFile();
	}
	
	@Override
	public void runEmulator( final FileInfo pFile )
	{
		if( ! new File( mPrefs.getRomsPath() + "/neogeo.zip" ).exists() )
		{
			mMessage.showMessageError( "No bios archive was found in the rom path. You need to put your bios archive (neogeo.zip) in the same directory as your roms." );
			return;
		}
		else if( ! pFile.isRom )
		{
			mMessage.showMessageError( "Sorry, you are trying to run an unsuported rom." );
			return;
		}
		else if( mNeoPrefs.getSystem().contentEquals( "unibios" ) && ! new File( mPrefs.getRomsPath() + "/uni-bios.rom" ).exists() )
		{
			mMessage.showMessageError( "Sorry, you are trying to use the uni-bios.rom but the bios file is not in your roms directory. " +
					"Please copy the \"uni-bios.rom\" file to your roms directory first, or change your system setting in preferences." );
			return;
		}
		
		// TODO: load main or main_asm from prefs
		UtilityLibs.add( libs.getLibSDLPath() );
		if( mNeoPrefs.useAsm() )
			UtilityLibs.add( libs.getLibMainASMPath() );
		else
			UtilityLibs.add( libs.getLibMainPath() );
		SDLActivity.mArgs = getArgs();
		SDLActivity.mScreenHolderSizeX = 320;
		SDLActivity.mScreenHolderSizeY = 240;
		SDLActivity.mScreenEmuSizeX = 320;
		SDLActivity.mScreenEmuSizeY = 224;
		
		super.runEmulator( pFile );
	}
	
	/*
	private void dialogConfirmDownloadLibs( )
	{
    	runOnUiThread( new Runnable()
        {
        	public void run()
        	{
				new AlertDialog.Builder( ActivityMain.this )
				.setTitle( "Confirm" )
				.setCancelable( false )
				.setMessage( "\nTo use this emulator, you need to download an extra package named \"GnGeoLibs\" on the Android Market.\n" )
				.setPositiveButton( "Download", new DialogInterface.OnClickListener()
				{
					public void onClick(DialogInterface dialog, int whichButton)
					{
						Intent i = new Intent(Intent.ACTION_VIEW, Uri.parse( "market://details?id=" + UtilityLibrary.LIBRARY_PACKAGE ) );
						startActivity( i );
					}
				})
				.setNegativeButton( "Cancel", new DialogInterface.OnClickListener()
				{
					public void onClick(DialogInterface dialog, int whichButton)
					{
						finish();
					}
				}).create().show();
            }
        });
	}
	*/
	
	private void extractDataFile()
	{
		mMessage.showDialogWait( "Please wait while extracting resources" );
		
		final UtilityExtractRaw raw = new UtilityExtractRaw( this );
		raw.add( "gngeo_data", mNeoPrefs.getDataFilePath() );
		
		new Thread( new Runnable()
		{
			@Override
			public void run() 
			{
				raw.extract();
		        mMessage.hideDialog();
			}
		}).start();
	}
	
	private String[] getArgs()
	{
		String[] args = new String[28];
    	args[0] = "--no-dump";
    	args[1] = "--no-joystick";
    	args[2] = "--no-debug";
    	args[3] = "--hwsurface";
    	args[4] = "--no-fullscreen";
    	args[5] = "--no-pal";
    	args[6] = "--no-bench";
    	args[7] = "--blitter=soft";
    	
    	if( mNeoPrefs.getForcepc() )
    		args[8] = "--forcepc";
    	else
    		args[8] = "--no-forcepc";
    	
    	if( mNeoPrefs.getInterpolation() )
    		args[9] = "--interpolation";
    	else
    		args[9] = "--no-interpolation";
    	
    	if( mNeoPrefs.getRaster() )
    		args[10] = "--raster";
    	else
    		args[10] = "--no-raster";
    	
    	if( mNeoPrefs.getSound() )
    		args[11] = "--sound";
    	else
    		args[11] = "--no-sound";
    	
    	if( mNeoPrefs.getShowfps() )
    		args[12] = "--showfps";
    	else
    		args[12] = "--no-showfps";
    	
    	if( mNeoPrefs.getSleepidle() )
    		args[13] = "--sleepidle";
    	else
    		args[13] = "--no-sleepidle";
    	
    	args[14] = "--no-vsync";
    	
    	if( mNeoPrefs.getAutoframeskip() )
    		args[15] = "--autoframeskip";
    	else
    		args[15] = "--no-autoframeskip";
    	
    	args[16] = "--country="+mNeoPrefs.getCountry();
    	args[17] = "--system="+mNeoPrefs.getSystem();
    	args[18] = "--rompath="+mPrefs.getRomsPath();
    	args[19] = "--biospath="+mPrefs.getRomsPath();
    	args[20] = "--datafile="+mNeoPrefs.getDataFilePath();
    	args[21] = "--effect=none";
    	args[22] = "--scale=1";
    	args[23] = "--samplerate="+mNeoPrefs.getSamplerate();
    	args[24] = "--68kclock="+mNeoPrefs.get68kClock();
    	args[25] = "--z80clock="+mNeoPrefs.getZ80Clock();
    	args[26] = "--no-screen320";
    	args[27] = mPrefs.getRom();
    	
    	return args;
	}
}
