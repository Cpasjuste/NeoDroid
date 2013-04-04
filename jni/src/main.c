/*  gngeo a neogeo emulator
 *  Copyright (C) 2001 Peponas Mathieu
 * 
 *  This program is free software; you can redistribute it and/or modify  
 *  it under the terms of the GNU General Public License as published by   
 *  the Free Software Foundation; either version 2 of the License, or    
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA. 
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <signal.h>

#include "SDL.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ym2610/2610intf.h"
#include "video.h"
#include "screen.h"
#include "emu.h"
#include "sound.h"
#include "messages.h"
#include "_memory.h"
#include "debug.h"
#include "blitter.h"
#include "effect.h"
#include "conf.h"
#include "transpack.h"
#include "event.h"
#include "menu.h"
#include "frame_skip.h"
#include "gnutil.h"
#include "roms.h"

#ifdef USE_GUI
#include "gui_interf.h"
#endif
#ifdef GP2X
#include "gp2x.h"
#include "ym2610-940/940shared.h"
#endif
#ifdef WII
extern bool fatInitDefault(void);
#endif

#ifdef __AMIGA__
# include <proto/dos.h>
#endif

#ifdef ANDROID
extern char gnerror[1024];
extern void setErrorMsg( char *msg );
#endif //ANDROID

static void catch_me(int signo) {
	printf("Catch a sigsegv\n");
	//SDL_Quit();
	exit(-1);
}
int main(int argc, char *argv[])
{
    char *rom_name;
    int rc;


#ifdef __AMIGA__
    BPTR file_lock = GetProgramDir();
    SetProgramDir(file_lock);
#endif
#ifndef ANDROID
	signal(SIGSEGV, catch_me);
#endif

#ifdef WII
	//   SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_NOPARACHUTE);

	fatInitDefault();
#endif

    cf_init(); /* must be the first thing to do */
    cf_init_cmd_line();
    cf_open_file(NULL); /* Open Default configuration file */

    rom_name=cf_parse_cmd_line(argc,argv);

    /* print effect/blitter list if asked by user */
    if (!strcmp(CF_STR(cf_get_item_by_name("effect")),"help")) {
	print_effect_list();
	exit(0);
    }
    if (!strcmp(CF_STR(cf_get_item_by_name("blitter")),"help")) {
	print_blitter_list();
	exit(0);
    }

	init_sdl();

	init_event();

/* GP2X stuff */
#ifdef GP2X
    gp2x_init();
#endif
#ifndef ANDROID
    if ((rc=gn_init_skin())!=GN_TRUE) {
	    printf("Can't load skin...\n");
            exit(1);
    }    
#endif
	reset_frame_skip();

    if (conf.debug) conf.sound=0;

#ifdef ANDROID
	if (init_game(rom_name)!=GN_TRUE) {
		char msg[2048];
		snprintf( msg, 2048, "Can't init %s...\n%s",rom_name,gnerror);
		//printf(msg);
		setErrorMsg( msg );
		//SDL_Quit();
		return 0;
            	//exit(1);
	}    
#else
/* Launch the specified game, or the rom browser if no game was specified*/
	if (!rom_name) {
	//	rom_browser_menu();
		run_menu();
		printf("GAME %s\n",conf.game);
		if (conf.game==NULL) return 0;
	} else {

		if (init_game(rom_name)!=GN_TRUE) {
			printf("Can't init %s...\n",rom_name);
            exit(1);
		}    
	}
#endif //ANDROID

	/* If asked, do a .gno dump and exit*/
    if (CF_BOOL(cf_get_item_by_name("dump"))) {
        char dump[8+4+1];
        sprintf(dump,"%s.gno",rom_name);
        dr_save_gno(&memory.rom,dump);
        close_game();
        return 0;
    }

    if (conf.debug)
	    debug_loop();
    else
	    main_loop();

    close_game();

#ifdef ANDROID
	close_sdl_audio();
	SDL_Quit();
#endif
    return 0;
}
