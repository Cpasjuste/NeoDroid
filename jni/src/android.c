#include <jni.h>
#include <android/log.h>
#include "android.h"
#include <SDL.h>

#include "_memory.h"
#include "state.h"
#include "emu.h"
#include "conf.h"

extern void SDL_Android_Init(JNIEnv* env, jclass cls);
static jclass pActivityClass;
static JNIEnv* pEnv = NULL;
jmethodID showBar;
jmethodID hideBar;
jmethodID setBar;
jmethodID setError;
jmethodID JNIgetRomsPath;
jmethodID JNIgetDataPath;
const char* rom_path;
const char* data_path;
const char* cache_path;
int android_pause = 0;

void Java_fr_mydedibox_utility_UtilitySDL_nativeInitWithArgs(JNIEnv* env, jclass cls, jobjectArray strArray)
{
	int status, i;

	SDL_Android_Init(env, cls);

	pEnv = env;
	pActivityClass = (jclass)(*env)->NewGlobalRef(env,cls);

	JNIgetRomsPath = (*env)->GetStaticMethodID( env, pActivityClass, "getRomsPath","()Ljava/lang/String;" );
	jstring rompath = (jstring)(*env)->CallStaticObjectMethod( env, pActivityClass, JNIgetRomsPath );
        rom_path = (*env)->GetStringUTFChars( env, rompath, 0 );

	JNIgetDataPath = (*env)->GetStaticMethodID( env, pActivityClass, "getDataPath","()Ljava/lang/String;" );
	jstring datapath = (jstring)(*env)->CallStaticObjectMethod( env, pActivityClass, JNIgetDataPath );
        data_path = (*env)->GetStringUTFChars( env, datapath, 0 );

	jsize len = (*env)->GetArrayLength(env,strArray);
	const char *argv[len];
	argv[0] = strdup("NeoDroid");

	for( i=0; i<len; i++ )
	{
		jstring str = (jstring)(*env)->GetObjectArrayElement(env,strArray,i);
		argv[i+1] = (*env)->GetStringUTFChars( env, str, 0 );
	}

	showBar = (*env)->GetStaticMethodID( env, pActivityClass, "showProgressBar","(Ljava/lang/String;I)V" );
	hideBar = (*env)->GetStaticMethodID( env, pActivityClass, "hideProgressBar","()V" );
	setBar = (*env)->GetStaticMethodID( env, pActivityClass,"setProgressBar","(I)V" );
	setError = (*env)->GetStaticMethodID( env, pActivityClass, "setErrorMessage","(Ljava/lang/String;)V" );

	if( !showBar || !hideBar || !setBar || !setError )
		printf( "ERROR: could not find static methods" );

	android_pause = 0;

	status = SDL_main(i+1, (char **)argv);
}

const char *getRomsPath()
{
	return rom_path;
}

const char *getDataPath()
{
	return data_path;
}

void Java_fr_mydedibox_utility_UtilitySDL_emustop( JNIEnv *env, jobject thiz )
{
	neo_emu_done = 1;
}

jint Java_fr_mydedibox_utility_UtilitySDL_ispaused( JNIEnv *env, jobject thiz )
{
	return android_pause;
}

void Java_fr_mydedibox_utility_UtilitySDL_pauseemu( JNIEnv *env, jobject thiz )
{
	android_pause = 1;
}

void Java_fr_mydedibox_utility_UtilitySDL_resumeemu( JNIEnv *env, jobject thiz )
{
	android_pause = 0;
}

jint Java_fr_mydedibox_utility_UtilitySDL_getslotnum( JNIEnv *env, jobject thiz )
{
	return how_many_slot(conf.game);
}

void Java_fr_mydedibox_utility_UtilitySDL_statesave( JNIEnv *env, jobject thiz, jint statenum )
{
	Uint32 slot = statenum;
	Uint32 nb_slot = how_many_slot(conf.game);

	if (slot > nb_slot)
		slot = nb_slot;

	printf( "saving game %s in slot %i", conf.game, slot );
	save_state(conf.game, slot);
}

void Java_fr_mydedibox_utility_UtilitySDL_stateload( JNIEnv *env, jobject thiz, jint statenum )
{
	Uint32 slot = statenum;
	printf( "loading game %s from slot %i", conf.game, slot );
	load_state(conf.game, slot);
}

void Java_fr_mydedibox_utility_UtilitySDL_setPadData( JNIEnv *env, jobject thiz, jint i, jlong jl )
{
	unsigned long l = (unsigned long)jl;

	// Update dip switch
	if (l & ANDROID_1P2P)
		conf.test_switch = 1;

	// Update coin data 
	memory.intern_coin = 0x7;
	if (l & ANDROID_COINS)
		memory.intern_coin &= 0x6;
	
	// Update start data TODO: Select 
	memory.intern_start = 0x8F;
	if (l & ANDROID_START)
		memory.intern_start &= 0xFE;

	// Update P1 
	memory.intern_p1 = 0xFF;
	if ((l & ANDROID_UP) && (!(l & ANDROID_DOWN)))
	   	 memory.intern_p1 &= 0xFE;
	if ((l & ANDROID_DOWN) && (!(l & ANDROID_UP)))
		memory.intern_p1 &= 0xFD;
	if ((l & ANDROID_LEFT) && (!(l & ANDROID_RIGHT)))
	    	memory.intern_p1 &= 0xFB;
	if ((l & ANDROID_RIGHT) && (!(l & ANDROID_LEFT)))
		memory.intern_p1 &= 0xF7;

	if (l & ANDROID_1)
		memory.intern_p1 &= 0xEF;	// A
	if (l & ANDROID_2)
		memory.intern_p1 &= 0xDF;	// B
	if (l & ANDROID_3)
		memory.intern_p1 &= 0xBF;	// C
	if (l & ANDROID_4)
		memory.intern_p1 &= 0x7F;	// D

	// Update P2
	memory.intern_p2 = 0xFF;
}

void setErrorMsg( char *msg )
{
	if( setError )
	{
		(*pEnv)->CallStaticVoidMethod(pEnv,pActivityClass, setError, (*pEnv)->NewStringUTF(pEnv,msg) );
	}
}

void gn_init_pbar(char *name, int size) 
{
	if(showBar)
	{
		(*pEnv)->CallStaticVoidMethod(pEnv,pActivityClass, showBar, (*pEnv)->NewStringUTF(pEnv,name), size );
	}
}

void gn_update_pbar(int pos) 
{
	if (setBar) 
	{
		(*pEnv)->CallStaticVoidMethod(pEnv, pActivityClass, setBar, pos );
	}
}

void gn_terminate_pbar(void) 
{
	if(hideBar)
	{
		(*pEnv)->CallStaticVoidMethod(pEnv,pActivityClass, hideBar);
	}
}


