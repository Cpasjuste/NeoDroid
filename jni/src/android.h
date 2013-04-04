#ifndef ANDROID_H
#define ANDROID_H

#include <SDL.h>

char android_img_state_path[2048];
int neo_emu_done;
const char *getRomsPath();
const char *getDataPath();

enum  { ANDROID_UP=0x1,		ANDROID_LEFT=0x4,	ANDROID_DOWN=0x10,  ANDROID_RIGHT=0x40,
        ANDROID_SWITCH=1<<16,	ANDROID_1P2P=1<<17,	ANDROID_START=1<<8,	ANDROID_COINS=1<<9,
        ANDROID_1=1<<12,	ANDROID_2=1<<13,        ANDROID_3=1<<14,    ANDROID_4=1<<15,
	ANDROID_5=1<<10,    	ANDROID_6=1<<11
};

#endif
