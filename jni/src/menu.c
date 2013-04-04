/*  gngeo, a neogeo emulator
 *  Copyright (C) 2001 Peponas Thomas & Peponas Mathieu
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
#include "config.h"
#endif
#include <errno.h>

#include "SDL.h"
#include "SDL_thread.h"

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <math.h>

#include "menu.h"

#include "messages.h"
#include "_memory.h"
#include "screen.h"
#include "video.h"
#include "event.h"
#include "state.h"
#include "emu.h"
#include "frame_skip.h"
#include "video.h"
#include "conf.h"
#include "resfile.h"
#include "sound.h"
#include "effect.h"
#include "gnutil.h"
#include "roms.h"

#if defined (WII)
#define ROOTPATH "sd:/apps/gngeo/"
#elif defined (__AMIGA__)
#define ROOTPATH "/PROGDIR/data/"
#else
#define ROOTPATH ""
#endif

#define ICOL_BLACK  0
#define ICOL_RED    1
#define ICOL_GREEN  2
#define ICOL_GREY_L 3
#define ICOL_GREY_M 4
#define ICOL_GREY_D 5
#define ICOL_MAX	 6

#define COL_BLACK  0x000000
#define COL_RED    0x1010FF
#define COL_GREEN  0x10FF10
#define COL_GREY_L 0xA0A0A0
#define COL_GREY_M 0x808080
#define COL_GREY_D 0x404040

Uint32 font_color[] = { COL_BLACK, COL_RED, COL_GREEN, COL_GREY_L, COL_GREY_M,
		COL_GREY_D };

typedef struct GNFONT {
	SDL_Surface *bmp;
	SDL_Surface *cbmp[ICOL_MAX];
	Uint8 ysize;
	Uint16 xpos[128 - 32];
	Uint8 xsize[128 - 32];
	Sint8 pad;
} GNFONT;

static SDL_Surface *menu_buf;
static SDL_Surface *menu_back;
static SDL_Surface *back;
static GNFONT *sfont;
static GNFONT *mfont;
static SDL_Surface *gngeo_logo, *gngeo_mask, *pbar_logo;

static SDL_Surface *arrow_l, *arrow_r, *arrow_u, *arrow_d;
//static int interp;

#define MENU_BIG   0
#define MENU_SMALL 1

#define MENU_TITLE_X (24 + 30)
#define MENU_TITLE_Y (16 + 20)

#define MENU_TEXT_X (24 + 26)
#define MENU_TEXT_Y (16 + 43)

#define MENU_TEXT_X_END (24 + 277)
#define MENU_TEXT_Y_END (16 + 198)

#define ALIGN_LEFT   (1<<16)
#define ALIGN_RIGHT  (2<<16)
#define ALIGN_CENTER (3<<16)
#define ALIGN_UP     (1<<16)
#define ALIGN_DOWN   (2<<16)

#define MENU_CLOSE        1
#define MENU_STAY         0
#define MENU_EXIT         2
#define MENU_RETURNTOGAME 3

static GN_MENU *main_menu;
static GN_MENU *rbrowser_menu;
static GN_MENU *option_menu;
static GN_MENU *effect_menu;
static GN_MENU *srate_menu;
static GN_MENU *yesno_menu;

static char *romlist[] = { "/2020bb.zip", "/2020bba.zip", "/2020bbh.zip",
		"/3countb.zip", "/alpham2.zip", "/androdun.zip", "/aodk.zip",
		"/aof.zip", "/aof2.zip", "/aof2a.zip", "/aof3.zip", "/aof3k.zip",
		"/bakatono.zip", "/bangbead.zip", "/bjourney.zip", "/blazstar.zip",
		"/breakers.zip", "/breakrev.zip", "/bstars.zip", "/bstars2.zip",
		"/burningf.zip", "/burningfh.zip", "/crsword.zip", "/ct2k3sa.zip",
		"/ct2k3sp.zip", "/cthd2003.zip", "/ctomaday.zip", "/cyberlip.zip",
		"/diggerma.zip", "/doubledr.zip", "/eightman.zip", "/fatfursa.zip",
		"/fatfursp.zip", "/fatfury1.zip", "/fatfury2.zip", "/fatfury3.zip",
		"/fbfrenzy.zip", "/fightfev.zip", "/fightfeva.zip", "/flipshot.zip",
		"/fswords.zip", "/galaxyfg.zip", "/ganryu.zip", "/garou.zip",
		"/garoubl.zip", "/garouo.zip", "/garoup.zip", "/ghostlop.zip",
		"/goalx3.zip", "/gowcaizr.zip", "/gpilots.zip", "/gpilotsh.zip",
		"/gururin.zip", "/irrmaze.zip", "/janshin.zip", "/jockeygp.zip",
		"/joyjoy.zip", "/kabukikl.zip", "/karnovr.zip", "/kf10thep.zip",
		"/kf2k2mp.zip", "/kf2k2mp2.zip", "/kf2k2pla.zip", "/kf2k2pls.zip",
		"/kf2k3bl.zip", "/kf2k3bla.zip", "/kf2k3pcb.zip", "/kf2k3pl.zip",
		"/kf2k3upl.zip", "/kf2k5uni.zip", "/kizuna.zip", "/kof10th.zip",
		"/kof2000.zip", "/kof2000n.zip", "/kof2001.zip", "/kof2001h.zip",
		"/kof2002.zip", "/kof2002b.zip", "/kof2003.zip", "/kof2003h.zip",
		"/kof2k4se.zip", "/kof94.zip", "/kof95.zip", "/kof95h.zip",
		"/kof96.zip", "/kof96h.zip", "/kof97.zip", "/kof97a.zip",
		"/kof97pls.zip", "/kof98.zip", "/kof98k.zip", "/kof98n.zip",
		"/kof99.zip", "/kof99a.zip", "/kof99e.zip", "/kof99n.zip",
		"/kof99p.zip", "/kog.zip", "/kotm.zip", "/kotm2.zip", "/kotmh.zip",
		"/lans2004.zip", "/lastblad.zip", "/lastbladh.zip", "/lastbld2.zip",
		"/lastsold.zip", "/lbowling.zip", "/legendos.zip", "/lresort.zip",
		"/magdrop2.zip", "/magdrop3.zip", "/maglord.zip", "/maglordh.zip",
		"/mahretsu.zip", "/marukodq.zip", "/matrim.zip", "/matrimbl.zip",
		"/miexchng.zip", "/minasan.zip", "/mosyougi.zip", "/ms4plus.zip",
		"/ms5pcb.zip", "/ms5plus.zip", "/mslug.zip", "/mslug2.zip",
		"/mslug3.zip", "/mslug3b6.zip", "/mslug3h.zip", "/mslug3n.zip", /* Driver don't have the good name */
		"/mslug4.zip", "/mslug5.zip", "/mslug5h.zip", "/mslugx.zip",
		"/mutnat.zip", "/nam1975.zip", "/ncombat.zip", "/ncombath.zip",
		"/ncommand.zip", "/neobombe.zip", "/neocup98.zip", "/neodrift.zip",
		"/neomrdo.zip", "/ninjamas.zip", "/nitd.zip", "/nitdbl.zip",
		"/overtop.zip", "/panicbom.zip", "/pbobbl2n.zip", "/pbobblen.zip",
		"/pbobblena.zip", "/pgoal.zip", "/pnyaa.zip", "/popbounc.zip",
		"/preisle2.zip", "/pspikes2.zip", "/pulstar.zip", "/puzzldpr.zip",
		"/puzzledp.zip", "/quizdai2.zip", "/quizdais.zip", "/quizkof.zip",
		"/ragnagrd.zip", "/rbff1.zip", "/rbff1a.zip", "/rbff2.zip",
		"/rbff2h.zip", "/rbff2k.zip", "/rbffspec.zip", "/ridhero.zip",
		"/ridheroh.zip", "/roboarmy.zip", "/rotd.zip", "/s1945p.zip",
		"/samsh5sp.zip", "/samsh5sph.zip", "/samsh5spn.zip", /* Driver don't have the good name */
		"/samsho.zip", "/samsho2.zip", "/samsho3.zip", "/samsho3h.zip",
		"/samsho4.zip", "/samsho5.zip", "/samsho5b.zip", "/samsho5h.zip",
		"/samshoh.zip", "/savagere.zip", "/sdodgeb.zip", "/sengokh.zip",
		"/sengoku.zip", "/sengoku2.zip", "/sengoku3.zip", "/shocktr2.zip",
		"/shocktra.zip", "/shocktro.zip", "/socbrawl.zip", "/socbrawla.zip",
		"/sonicwi2.zip", "/sonicwi3.zip", "/spinmast.zip", "/ssideki.zip",
		"/ssideki2.zip", "/ssideki3.zip", "/ssideki4.zip", "/stakwin.zip",
		"/stakwin2.zip", "/strhoop.zip", "/superspy.zip", "/svc.zip",
		"/svcboot.zip", "/svcpcb.zip", "/svcpcba.zip", "/svcplus.zip",
		"/svcplusa.zip", "/svcsplus.zip", "/tophuntr.zip", "/tophuntra.zip",
		"/tpgolf.zip", "/trally.zip", "/turfmast.zip", "/twinspri.zip",
		"/tws96.zip", "/viewpoin.zip", "/vliner.zip", "/vlinero.zip",
		"/wakuwak7.zip", "/wh1.zip", "/wh1h.zip", "/wh1ha.zip", "/wh2.zip",
		"/wh2j.zip", "/wh2jh.zip", "/whp.zip", "/wjammers.zip", "/zedblade.zip",
		"/zintrckb.zip", "/zupapa.zip", NULL };

#define COL32_TO_16(col) ((((col&0xff0000)>>19)<<11)|(((col&0xFF00)>>10)<<5)|((col&0xFF)>>3))
struct RGB_set {
	unsigned char r;
	unsigned char g;
	unsigned char b;
} RGB_set;

struct HSV_set {
	signed int h;
	unsigned char s;
	unsigned char v;
} HSV_set;

Uint32 RGB2HSV(Uint32 RGB) {
	unsigned char min, max, delta;
	Uint16 h;
	Uint8 s, v;
	Uint8 r, g, b;
	r = (RGB >> 16) & 0xFF;
	g = (RGB >> 8) & 0xFF;
	b = RGB & 0xFF;

	if (r < g)
		min = r;
	else
		min = g;
	if (b < min)
		min = b;

	if (r > g)
		max = r;
	else
		max = g;
	if (b > max)
		max = b;

	v = max; // v, 0..255

	delta = max - min; // 0..255, < v

	if (max != 0)
		s = (int) (delta) * 255 / max; // s, 0..255
	else {
		// r = g = b = 0        // s = 0, v is undefined
		s = 0;
		h = 0;
		return (h << 16) | (s << 8) | v;
	}

	if (r == max)
		h = (g - b) * 60 / delta; // between yellow & magenta
	else if (g == max)
		h = 120 + (b - r) * 60 / delta; // between cyan & yellow
	else
		h = 240 + (r - g) * 60 / delta; // between magenta & cyan

	if (h < 0)
		h += 360;

	return (h << 16) | (s << 8) | v;
}
Uint32 HSV2RGB(Uint32 HSV) {
	int i;
	float f, p, q, t, h, s, v;
	Uint8 r, g, b;

	h = (float) (HSV >> 16);
	s = (float) ((HSV >> 8) & 0xFF);
	v = (float) (HSV & 0xFF);

	s /= 255.0;

	if (s == 0) { // achromatic (grey)
		r = g = b = v;
		return (r << 16) | (g << 8) | b;
	}

	h /= 60; // sector 0 to 5
	i = floor(h);
	f = h - i; // factorial part of h
	p = (unsigned char) (v * (1 - s));
	q = (unsigned char) (v * (1 - s * f));
	t = (unsigned char) (v * (1 - s * (1 - f)));

	switch (i) {
	case 0:
		r = v;
		g = t;
		b = p;
		break;
	case 1:
		r = q;
		g = v;
		b = p;
		break;
	case 2:
		r = p;
		g = v;
		b = t;
		break;
	case 3:
		r = p;
		g = q;
		b = v;
		break;
	case 4:
		r = t;
		g = p;
		b = v;
		break;
	default: // case 5:
		r = v;
		g = p;
		b = q;
		break;
	}
	return (r << 16) | (g << 8) | b;
}

void font_set_color(GNFONT *ft, int col) {
	if (col >= ICOL_MAX)
		col = 0;
	ft->bmp = ft->cbmp[col];
}

GNFONT *load_font(char *file) {
	GNFONT *ft = malloc(sizeof(GNFONT));
	Uint32 HSV;
	Uint32 RGB;
	int i;
	int x = 0;
	Uint32 *b;
	if (!ft)
		return NULL;
	memset(ft, 0, sizeof(GNFONT));
	for (i = 0; i < ICOL_MAX; i++)
		ft->cbmp[i] = res_load_stbi(file);
	ft->bmp = ft->cbmp[0];
	if (!ft->bmp) {
		free(ft);
		return NULL;
	}
	//SDL_SetAlpha(ft->bmp,SDL_SRCALPHA,128);
	b = ft->bmp->pixels;
	//ft->bmp->format->Amask=0xFF000000;
	//ft->bmp->format->Ashift=24;
	//printf("shift=%d %d\n",ft->bmp->pitch,ft->bmp->w*4);
	if (ft->bmp->format->BitsPerPixel != 32) {
		printf("Unsupported font (bpp=%d)\n", ft->bmp->format->BitsPerPixel);
		SDL_FreeSurface(ft->bmp);
		free(ft);
		return NULL;
	}
	ft->xpos[0] = 0;
	for (i = 0; i < ft->bmp->w; i++) {
		//printf("%08x\n",b[i]);
		if (b[i] != b[0]) {
			ft->xpos[x + 1] = i + 1;
			if (x > 0)
				ft->xsize[x] = i - ft->xpos[x];
			else
				ft->xsize[x] = i;
			x++;
		}
	}

	//printf("NB char found:%d\n",x);
	if (x <= 0 || x > 95)
		return NULL;
	/*	b=ft->bmp->pixels+ft->bmp->pitch*3;
	 for (i=0;i<ft->bmp->w;i++) {
	 //printf("%08x\n",b[i]);
	 }
	 */
	ft->xsize[94] = ft->bmp->w - ft->xpos[94];
	ft->ysize = ft->bmp->h;

	/* Default y padding=0 */
	ft->pad = 0;

	for (x = 1; x < ICOL_MAX; x++) {
		int j;
		b = ft->cbmp[x]->pixels;
		for (i = 0; i < ft->bmp->w; i++) {
			for (j = 0; j < ft->bmp->h; j++) {
				b[i * ft->bmp->h + j] &= 0xFF000000;
				b[i * ft->bmp->h + j] |= font_color[x];
			}
		}
	}
	//font_set_color(ft,ICOL_GREY_L);
	return ft;
}

static Uint32 string_len(GNFONT *f, char *str) {
	int i;
	int size = 0;
	if (str) {
		for (i = 0; i < strlen(str); i++) {
			switch (str[i]) {
			case ' ':
				size += f->xsize[0];
				break;
			case '\t':
				size += (f->xsize[0] * 8);
				break;
			default:
				size += (f->xsize[(int) str[i] - 32] + f->pad);
				break;
			}
		}
		return size;
	} else
		return 0;
}

void draw_string(SDL_Surface *dst, GNFONT *f, int x, int y, char *str) {
	SDL_Rect srect, drect;
	int i, s;

	if (!f) {
		if ((x & 0xff0000) == ALIGN_LEFT)
			x += MENU_TEXT_X;
		if ((x & 0xff0000) == ALIGN_RIGHT)
			x += (MENU_TEXT_X_END - strlen(str) * 8);
		if ((x & 0xff0000) == ALIGN_CENTER)
			x += (MENU_TEXT_X
					+ (MENU_TEXT_X_END - MENU_TEXT_X - strlen(str) * 8) / 2);
		if ((y & 0xff0000) == ALIGN_UP)
			y += MENU_TEXT_Y;
		if ((y & 0xff0000) == ALIGN_DOWN)
			y += (MENU_TEXT_Y_END - 8);
		if ((y & 0xff0000) == ALIGN_CENTER)
			y += (MENU_TEXT_Y + (MENU_TEXT_Y_END - MENU_TEXT_Y - 8) / 2);
		SDL_textout(dst, x & 0xffff, y & 0xffff, str);
		return;
	}

	if ((x & 0xff0000) == ALIGN_LEFT)
		x += MENU_TEXT_X;
	if ((x & 0xff0000) == ALIGN_RIGHT)
		x += (MENU_TEXT_X_END - string_len(f, str));
	if ((x & 0xff0000) == ALIGN_CENTER)
		x += (MENU_TEXT_X
				+ (MENU_TEXT_X_END - MENU_TEXT_X - string_len(f, str)) / 2);
	if ((y & 0xff0000) == ALIGN_UP)
		y += MENU_TEXT_Y;
	if ((y & 0xff0000) == ALIGN_DOWN)
		y += (MENU_TEXT_Y_END - f->ysize);
	if ((y & 0xff0000) == ALIGN_CENTER)
		y += (MENU_TEXT_Y + (MENU_TEXT_Y_END - MENU_TEXT_Y - f->ysize) / 2);

	x &= 0xffff;
	y &= 0xffff;

	drect.x = x;
	drect.y = y;
	drect.w = 32;
	drect.h = f->bmp->h;
	srect.h = f->bmp->h;
	srect.y = 0;
	for (i = 0; i < strlen(str); i++) {
		switch (str[i]) {
		case ' ': /* Optimize space */
			drect.x += f->xsize[0];
			break;
		case '\t':
			drect.x += (f->xsize[0] * 8);
			break;
		case '\n':
			drect.x = x;
			drect.y += f->bmp->h;
			break;
		default:
			s = (unsigned char) (str[i]);
			if (s >= 96 + 32) {
				s = (unsigned char) '.';
			}

			srect.x = f->xpos[s - 32];
			srect.w = f->xsize[s - 32];

			SDL_BlitSurface(f->bmp, &srect, dst, &drect);
			drect.x += (f->xsize[s - 32] + f->pad);

			break;
		}
	}
}

static void init_back(void) {
	SDL_Rect dst_r = { 24, 16, 304, 224 };
	static SDL_Rect screen_rect = { 0, 0, 304, 224 };
	SDL_BlitSurface(state_img, &screen_rect, menu_back, &dst_r);
	SDL_BlitSurface(back, NULL, menu_back, &dst_r);
}

static void draw_back(void) {
	SDL_Rect dst_r = { 24, 16, 304, 224 };
	static SDL_Rect screen_rect = { 0, 0, 304, 224 };
	if (back) {
		//		SDL_BlitSurface(state_img, &screen_rect, menu_buf, &dst_r);
		//		SDL_BlitSurface(back, NULL, menu_buf, &dst_r);
		SDL_BlitSurface(menu_back, NULL, menu_buf, NULL);
	} else {
		SDL_Rect r1 = { 24 + 16, 16 + 16, 271, 191 };
		SDL_Rect r2 = { 24 + 22, 16 + 35, 259, 166 };
		SDL_Rect r3 = { 24 + 24, 16 + 24, 271, 191 };

		SDL_FillRect(menu_buf, &r3, COL32_TO_16(0x111111));
		SDL_FillRect(menu_buf, &r1, COL32_TO_16(0xE8E8E8));
		SDL_FillRect(menu_buf, &r2, COL32_TO_16(0x1C57A2));
	}

}

#define ALEFT  0
#define ARIGHT 1
#define ADOWN  2
#define AUP    3

static void draw_arrow(int type, int x, int y) {
	SDL_Rect dst_r = { x, y - 13, 32, 32 };
	switch (type) {
	case ARIGHT:
		SDL_BlitSurface(arrow_r, NULL, menu_buf, &dst_r);
		break;
	case ALEFT:
		SDL_BlitSurface(arrow_l, NULL, menu_buf, &dst_r);
		break;
	case AUP:
		SDL_BlitSurface(arrow_u, NULL, menu_buf, &dst_r);
		break;
	case ADOWN:
		SDL_BlitSurface(arrow_d, NULL, menu_buf, &dst_r);
		break;
	}
}

int gn_init_skin(void) {
	//menu_buf = SDL_CreateRGBSurface(SDL_SWSURFACE, 352, 256, 32, 0xFF0000, 0xFF00, 0xFF, 0x0);
	menu_buf = SDL_CreateRGBSurface(SDL_SWSURFACE, 352, 256, 16, 0xF800, 0x7E0,
			0x1F, 0);
	//	menu_back= SDL_CreateRGBSurface(SDL_SWSURFACE, 352, 256, 32, 0xFF0000, 0xFF00, 0xFF, 0x0);
	menu_back = SDL_CreateRGBSurface(SDL_SWSURFACE, 352, 256, 16, 0xF800, 0x7E0,
			0x1F, 0);
	if (res_verify_datafile(NULL) == GN_FALSE) {
		gn_popup_error("Datafile Not Found!",
				"Gngeo could not find his \n datafile :( \n\n%s", gnerror);
		return GN_FALSE;
	}
	back = res_load_stbi("skin/back.tga");
	sfont = load_font("skin/font_small.tga");
	mfont = load_font("skin/font_large.tga");
	arrow_r = res_load_stbi("skin/arrow_right.tga");
	arrow_l = res_load_stbi("skin/arrow_left.tga");
	arrow_d = res_load_stbi("skin/arrow_down.tga");
	arrow_u = res_load_stbi("skin/arrow_up.tga");
	gngeo_logo = res_load_stbi("skin/gngeo.tga");
	gngeo_mask = res_load_stbi("skin/gngeo_mask.tga");

	pbar_logo = SDL_CreateRGBSurface(SDL_SWSURFACE, gngeo_logo->w,
			gngeo_logo->h, 32, 0xFF, 0xFF00, 0xFF0000, 0xFF000000);
	SDL_SetAlpha(gngeo_logo, 0, 0);
	//SDL_SetAlpha(gngeo_mask,0,0);
	init_back();

	if (!back || !sfont || !mfont || !arrow_r || !arrow_l || !arrow_u
			|| !arrow_d || !gngeo_logo || !menu_buf)
		return GN_FALSE;
	return GN_TRUE;
}

static int pbar_y;

void gn_reset_pbar(void) {
	pbar_y = 0;
}

static SDL_Thread *pbar_th;

typedef struct pbar_data {
	char *name;
	int pos;
	int size;
	int running;
} pbar_data;

static volatile pbar_data pbar;

int pbar_anim_thread(void *data) {
	pbar_data *p = (pbar_data*) data;
	SDL_Rect src_r = { 2, 0, gngeo_logo->w, gngeo_logo->h };
	SDL_Rect dst_r = { 219 + 26, 146 + 16, gngeo_logo->w, gngeo_logo->h };
	SDL_Rect dst2_r = { 0, 0, gngeo_logo->w, gngeo_logo->h };
	int x = 0;

	while (p->running) {
		draw_back();
		draw_string(menu_buf, mfont, MENU_TITLE_X, MENU_TITLE_Y + 150, p->name);
		draw_string(menu_buf, sfont, MENU_TITLE_X, MENU_TITLE_Y + 165,
				"Please wait!");

		SDL_BlitSurface(gngeo_logo, NULL, pbar_logo, NULL);

		dst2_r.y = -22 - (p->pos * 64.0) / p->size;
		x += 3;
		if (x > gngeo_logo->w)
			x -= gngeo_logo->w;

		dst2_r.x = x;
		SDL_BlitSurface(gngeo_mask, NULL, pbar_logo, &dst2_r);

		dst2_r.x = x - gngeo_logo->w;
		dst2_r.y = -22 - (p->pos * 64.0) / p->size;
		SDL_BlitSurface(gngeo_mask, NULL, pbar_logo, &dst2_r);

		SDL_BlitSurface(pbar_logo, &src_r, menu_buf, &dst_r);

		SDL_BlitSurface(menu_buf, NULL, buffer, NULL);
		screen_update();
		frame_skip(0);
		//printf("TOTO %d %d %d\n",p->pos,p->size,dst2_r.x);
	}
	SDL_BlitSurface(gngeo_logo, NULL, pbar_logo, NULL);
	SDL_BlitSurface(pbar_logo, &src_r, menu_buf, &dst_r);
	screen_update();
	frame_skip(0);
	return 0;
}

void gn_init_pbar(char *name, int size) {
	pbar.name = name;
	pbar.pos = 0;
	pbar.size = size;
	pbar.running = 1;
	pbar_th = SDL_CreateThread(pbar_anim_thread, (void*) &pbar);
}

void gn_update_pbar(int pos) {
	pbar.pos = pos;
}

void gn_terminate_pbar(void) {
	pbar.running = 0;
	SDL_WaitThread(pbar_th, NULL);
}

void gn_popup_error(char *name, char *fmt, ...) {
	char buf[512];
	va_list pvar;
	va_start(pvar, fmt);

	reset_event();

	draw_back();
	draw_string(menu_buf, mfont, MENU_TITLE_X, MENU_TITLE_Y, name);

	vsnprintf(buf, 511, fmt, pvar);

	draw_string(menu_buf, sfont, MENU_TEXT_X, MENU_TEXT_Y, buf);

	draw_string(menu_buf, sfont, ALIGN_RIGHT, ALIGN_DOWN, "Press any key ...");
	SDL_BlitSurface(menu_buf, NULL, buffer, NULL);
	screen_update();

	while (wait_event() == 0)
		;
}

static int yes_action(GN_MENU_ITEM *self, void *param) {
	return 1;
}

static int no_action(GN_MENU_ITEM *self, void *param) {
	return 0;
}

int gn_popup_question(char *name, char *fmt, ...) {
	char buf[512];
	va_list pvar;
	va_start(pvar, fmt);
	int a;

	reset_event();

	while (1) {
		draw_back();

		draw_string(menu_buf, mfont, MENU_TITLE_X, MENU_TITLE_Y, name);

		vsnprintf(buf, 511, fmt, pvar);
		draw_string(menu_buf, sfont, MENU_TEXT_X, MENU_TEXT_Y, buf);
		//draw_string(menu_buf, sfont, ALIGN_RIGHT, ALIGN_DOWN, "   Yes     No");
		if (yesno_menu->current == 0)
			draw_string(menu_buf, sfont, ALIGN_RIGHT, ALIGN_DOWN,
					" > Yes     No");
		else
			draw_string(menu_buf, sfont, ALIGN_RIGHT, ALIGN_DOWN,
					"   Yes  >  No");
		SDL_BlitSurface(menu_buf, NULL, buffer, NULL);
		screen_update();

		//effect_menu->draw(effect_menu); //frame_skip(0);printf("fps: %s\n",fps_str);
		if ((a = yesno_menu->event_handling(yesno_menu)) >= 0) {
			printf("return %d\n", a);
			return a;
		}
	}
	return 0;
}

//#define NB_ITEM_2 4

static void draw_menu(GN_MENU *m) {
	int start, end, i;
	static int cx = 0;
	static int cy = 0;
	//static int cx_val[]={0,1,1,2,2,2,1,1,0,-1,-1,-2,-2,-2,-1,-1};
	static int cx_val[] = { 0, 1, 1, 1, 2, 2, 2, 2, 2, 1, 1, 1, 0, 0, -1, -1,
			-1, -2, -2, -2, -2, -2, -1, -1, -1, 0 };
	int nb_item;
	int ypos, cpos;
	GNFONT *fnt;
	LIST *l = m->item;
	int j;

	if (m->draw_type == MENU_BIG)
		fnt = mfont;
	else
		fnt = sfont;

	/* Arrow moving cursor */
	cx++;
	if (cx > 25)
		cx = 0;

	nb_item = (MENU_TEXT_Y_END - MENU_TEXT_Y) / fnt->ysize - 1;

	draw_back();

	if (m->title) {
		draw_string(menu_buf, mfont, MENU_TITLE_X, MENU_TITLE_Y, m->title);
	}
	/*
	 start = (m->current - nb_item >= 0 ? m->current - nb_item : 0);
	 end = (start + nb_item < m->nb_elem ? start + nb_item : m->nb_elem - 1);
	 */
	start = m->current - nb_item / 2;
	end = m->current + nb_item / 2;
	if (start < 0)
		start = 0;
	if (end >= m->nb_elem - 1) {
		end = m->nb_elem - 1;
	} else {
		draw_arrow(ADOWN, 24, 200 + cx_val[cx]);
	}

	if (m->current <= nb_item / 2) {
		j = nb_item / 2 - m->current;
	} else {
		j = 0;
		draw_arrow(AUP, 24, 76 - cx_val[cx]);
	}

	//printf("%d %d %d %d\n",start,end,m->current,j);
	for (i = 0; i < start; i++, l = l->next) {
		GN_MENU_ITEM *mi = (GN_MENU_ITEM *) l->data;
		if (mi->enabled == 0) {
			i--;
		}
	}
	for (i = start; i <= end; i++, l = l->next) {
		GN_MENU_ITEM *mi = (GN_MENU_ITEM *) l->data;
		if (mi->enabled == 0) {
			i--;
			continue;
		}
		//int j = (i + nb_item / 2) - start;
		//if (start<nb_item) j+=nb_item/2;
		if (m->draw_type == MENU_BIG) {
			font_set_color(fnt, ICOL_GREY_L);
			draw_string(menu_buf, fnt, ALIGN_CENTER + 2,
					MENU_TEXT_Y + 2 + (j * fnt->ysize + 2), mi->name);
			font_set_color(fnt, ICOL_BLACK);
			draw_string(menu_buf, fnt, ALIGN_CENTER,
					MENU_TEXT_Y + (j * fnt->ysize + 2), mi->name);
			if (i == m->current) {
				int len = string_len(fnt, mi->name) / 2;
				draw_arrow(ARIGHT, 176 - len - 32 + cx_val[cx],
						MENU_TEXT_Y + (j * fnt->ysize + 2) + fnt->ysize / 2);
				draw_arrow(ALEFT, 176 + len - cx_val[cx],
						MENU_TEXT_Y + (j * fnt->ysize + 2) + fnt->ysize / 2);
			}

		} else {
			draw_string(menu_buf, fnt, MENU_TEXT_X + 10,
					MENU_TEXT_Y + (j * fnt->ysize + 2), mi->name);
			if (i == m->current)
				draw_string(menu_buf, fnt, MENU_TEXT_X,
						MENU_TEXT_Y + (j * fnt->ysize + 2), ">");
			if (mi->type == MENU_CHECK) {
				if (mi->val)
					draw_string(menu_buf, fnt, MENU_TEXT_X + 210,
							MENU_TEXT_Y + (j * fnt->ysize + 2), "true");
				else
					draw_string(menu_buf, fnt, MENU_TEXT_X + 210,
							MENU_TEXT_Y + (j * fnt->ysize + 2), "false");
			}
			if (mi->type == MENU_LIST) {
				draw_string(menu_buf, fnt, MENU_TEXT_X + 210,
						MENU_TEXT_Y + (j * fnt->ysize + 2), mi->str);
			}
		}
		j++;
	}
	SDL_BlitSurface(menu_buf, NULL, buffer, NULL);
	screen_update();
	frame_skip(0);
}

//#undef NB_ITEM_2

GN_MENU_ITEM *gn_menu_create_item(char *name, Uint32 type,
		int(*action)(GN_MENU_ITEM *self, void *param), void *param) {
	GN_MENU_ITEM *t = malloc(sizeof(GN_MENU_ITEM));
	t->name = strdup(name);
	//t->name=name;
	t->type = type;
	t->action = action;
	t->arg = param;
	t->enabled = 1;
	return t;
}

GN_MENU_ITEM *gn_menu_get_item_by_index(GN_MENU *gmenu, int index) {
	GN_MENU_ITEM *gitem;
	LIST *l = gmenu->item;
	int i = 0;
	for (l = gmenu->item; l; l = l->next) {
		gitem = (GN_MENU_ITEM *) l->data;
		if (gitem->enabled) {
			if (i == index)
				return gitem;
			i++;
		}

	}
	return NULL;
}

int test_action(GN_MENU_ITEM *self, void *param) {
	printf("Action!!\n");
	return 0;
}

static int load_state_action(GN_MENU_ITEM *self, void *param) {
	static Uint32 slot = 0;
	SDL_Rect dstrect = { 24 + 75, 16 + 66, 304 / 2, 224 / 2 };
	SDL_Rect dstrect_binding = { 24 + 73, 16 + 64, 304 / 2 + 4, 224 / 2 + 4 };
	//SDL_Rect dst_r={24,16,304,224};
	//SDL_Event event;
	SDL_Surface *tmps, *slot_img;
	char slot_str[32];

	Uint32 nb_slot = how_many_slot(conf.game);

	if (slot > nb_slot - 1)
		slot = nb_slot - 1;

	if (nb_slot == 0) {
		gn_popup_info("Load State",
				"There is currently no save state available");
		return 0; // nothing to do
	}

	//slot_img=load_state_img(conf.game,slot);
	tmps = load_state_img(conf.game, slot);
	slot_img = SDL_ConvertSurface(tmps, menu_buf->format, SDL_SWSURFACE);

	while (1) {
		//if (back) SDL_BlitSurface(back,NULL,menu_buf,&dst_r);
		//else SDL_FillRect(menu_buf,NULL,COL32_TO_16(0x11A011));
		draw_back();
		SDL_FillRect(menu_buf, &dstrect_binding, COL32_TO_16(0xFEFEFE));
		SDL_SoftStretch(slot_img, NULL, menu_buf, &dstrect);

		draw_string(menu_buf, mfont, MENU_TITLE_X, MENU_TITLE_Y, "Load State");
		sprintf(slot_str, "Slot Number %03d", slot);
		//draw_string(menu_buf,sfont,24+102,16+40,slot_str);
		draw_string(menu_buf, sfont, ALIGN_CENTER, ALIGN_UP, slot_str);

		if (slot > 0)
			draw_arrow(ALEFT, 44 + 16, 224 / 2 + 16);
		if (slot < nb_slot - 1)
			draw_arrow(ARIGHT, 304 - 43, 224 / 2 + 16);

		SDL_BlitSurface(menu_buf, NULL, buffer, NULL);
		screen_update();
		frame_skip(0);
		switch (wait_event()) {
		case GN_LEFT:
			if (slot > 0)
				slot--;
			slot_img = SDL_ConvertSurface(load_state_img(conf.game, slot),
					menu_buf->format, SDL_SWSURFACE);
			break;
		case GN_RIGHT:
			if (slot < nb_slot - 1)
				slot++;
			slot_img = SDL_ConvertSurface(load_state_img(conf.game, slot),
					menu_buf->format, SDL_SWSURFACE);
			break;
		case GN_A:
			return MENU_STAY;
			break;
		case GN_B:
		case GN_C:
			printf("Loading state %p!!\n", buffer);
			load_state(conf.game, slot);
			printf("Loaded state %p!!\n", buffer);
			return MENU_RETURNTOGAME;
			break;
		default:
			break;
		}
	}
	return 0;
}

static int save_state_action(GN_MENU_ITEM *self, void *param) {
	static Uint32 slot = 0;
	SDL_Rect dstrect = { 24 + 75, 16 + 66, 304 / 2, 224 / 2 };
	SDL_Rect dstrect_binding = { 24 + 73, 16 + 64, 304 / 2 + 4, 224 / 2 + 4 };
	SDL_Surface *tmps, *slot_img = NULL;
	char slot_str[32];
	Uint32 nb_slot = how_many_slot(conf.game);

	if (slot > nb_slot)
		slot = nb_slot;

	if (nb_slot != 0 && slot < nb_slot) {
		tmps = load_state_img(conf.game, slot);
		slot_img = SDL_ConvertSurface(tmps, menu_buf->format, SDL_SWSURFACE);
	}

	while (1) {
		draw_back();
		if (slot != nb_slot) {
			SDL_FillRect(menu_buf, &dstrect_binding, COL32_TO_16(0xFEFEFE));
			SDL_SoftStretch(slot_img, NULL, menu_buf, &dstrect);
		} else {
			SDL_FillRect(menu_buf, &dstrect_binding, COL32_TO_16(0xFEFEFE));
			SDL_FillRect(menu_buf, &dstrect, COL32_TO_16(0xA0B0B0));
			draw_string(menu_buf, sfont, ALIGN_CENTER, ALIGN_CENTER,
					"Create a new Slot");
		}

		draw_string(menu_buf, mfont, MENU_TITLE_X, MENU_TITLE_Y, "Save State");
		sprintf(slot_str, "Slot Number %03d", slot);
		draw_string(menu_buf, sfont, ALIGN_CENTER, ALIGN_UP, slot_str);

		if (slot > 0)
			draw_arrow(ALEFT, 44 + 16, 224 / 2 + 16);
		if (slot < nb_slot)
			draw_arrow(ARIGHT, 304 - 43, 224 / 2 + 16);

		SDL_BlitSurface(menu_buf, NULL, buffer, NULL);
		screen_update();
		frame_skip(0);

		switch (wait_event()) {
		case GN_LEFT:
			if (slot > 0)
				slot--;
			if (slot != nb_slot)
				slot_img = SDL_ConvertSurface(load_state_img(conf.game, slot),
						menu_buf->format, SDL_SWSURFACE);
			break;
		case GN_RIGHT:
			if (slot < nb_slot)
				slot++;
			if (slot != nb_slot)
				slot_img = SDL_ConvertSurface(load_state_img(conf.game, slot),
						menu_buf->format, SDL_SWSURFACE);
			break;
		case GN_A:
			return MENU_STAY;
			break;
		case GN_B:
		case GN_C:

			save_state(conf.game, slot);
			printf("Save state!!\n");
			return MENU_RETURNTOGAME;
			break;
		default:
			break;
		}
	}
	return 0;
}

static int credit_action(GN_MENU_ITEM *self, void *param) {

	return MENU_STAY;
}

static int exit_action(GN_MENU_ITEM *self, void *param) {
	//exit(0);
//	if (gn_popup_question("Quit?","Do you really want to quit gngeo?")==0)
//		return 0;
	printf("Save all\n");
//	/* Save the last rom path used */
//	cf_reset_all_changed_flag();
//	cf_item_has_been_changed(cf_get_item_by_name("rompath"));
//	cf_save_file(NULL, 0);
	return MENU_EXIT;
}

int menu_event_handling(struct GN_MENU *self) {
	//static SDL_Event event;
	GN_MENU_ITEM *mi;
	int a;
	LIST *l;
	switch (wait_event()) {
	case GN_UP:
		if (self->current > 0)
			self->current--;
		else
			self->current = self->nb_elem - 1;
		break;
	case GN_DOWN:
		if (self->current < self->nb_elem - 1)
			self->current++;
		else
			self->current = 0;
		break;

	case GN_LEFT:
		self->current -= 10;
		if (self->current < 0)
			self->current = 0;
		break;
	case GN_RIGHT:
		self->current += 10;
		if (self->current >= self->nb_elem)
			self->current = self->nb_elem - 1;
		break;
	case GN_A:
		return MENU_CLOSE;
		break;
	case GN_B:
	case GN_C:
		//l = list_get_item_by_index(self->item, self->current);
		mi = gn_menu_get_item_by_index(self, self->current);
		if (mi && mi->action) {
			reset_event();
			if ((a = mi->action(mi, NULL)) > 0)
				/*
				 if (a == MENU_CLOSE) return MENU_STAY;
				 else
				 */
				return a;
		}
		break;
	default:
		break;

	}
	return -1;
}

GN_MENU *create_menu(char *name, int type, int(*action)(struct GN_MENU *self),
		void(*draw)(struct GN_MENU *self)) {
	GN_MENU *gmenu;
	gmenu = malloc(sizeof(GN_MENU));
	gmenu->title = name;
	gmenu->nb_elem = 0;
	gmenu->current = 0;
	gmenu->draw_type = type;
	if (action)
		gmenu->event_handling = action;
	else
		gmenu->event_handling = menu_event_handling;
	if (draw)
		gmenu->draw = draw;
	else
		gmenu->draw = draw_menu;
	gmenu->item = NULL;
	return gmenu;
}

GN_MENU_ITEM *gn_menu_add_item(GN_MENU *gmenu, char *name, int type,
		int(*action)(struct GN_MENU_ITEM *self, void *param), void *param) {
	GN_MENU_ITEM *gitem;
	gitem = gn_menu_create_item(name, type, action, param);
	gmenu->item = list_append(gmenu->item, (void*) gitem);
	gmenu->nb_elem++;
	return gitem;
}

GN_MENU_ITEM *gn_menu_get_item_by_name(GN_MENU *gmenu, char *name) {
	GN_MENU_ITEM *gitem;
	LIST *l = gmenu->item;

	for (l = gmenu->item; l; l = l->next) {
		gitem = (GN_MENU_ITEM *) l->data;
		if (strncmp(gitem->name, name, 128) == 0 && gitem->enabled != 0) {
			return gitem;
		}
	}
	return NULL;
}

void gn_menu_disable_item(GN_MENU *gmenu, char *name) {
	GN_MENU_ITEM *gitem;
	LIST *l = gmenu->item;

	for (l = gmenu->item; l; l = l->next) {
		gitem = (GN_MENU_ITEM *) l->data;
		if (strncmp(gitem->name, name, 128) == 0 && gitem->enabled != 0) {
			gitem->enabled = 0;
			gmenu->nb_elem--;
			return;
		}
	}
}

void gn_menu_enable_item(GN_MENU *gmenu, char *name) {
	GN_MENU_ITEM *gitem;
	LIST *l = gmenu->item;

	for (l = gmenu->item; l; l = l->next) {
		gitem = (GN_MENU_ITEM *) l->data;
		if (strcmp(gitem->name, name) == 0 && gitem->enabled == 0) {
			gitem->enabled = 1;
			gmenu->nb_elem++;
			return;
		}
	}
}

int icasesort(const struct dirent **a, const struct dirent **b) {
	const char *ca = (*a)->d_name;
	const char *cb = (*b)->d_name;
	return strcasecmp(ca, cb);
}

static int romnamesort(void *a, void *b) {
	GN_MENU_ITEM *ga = (GN_MENU_ITEM *) a;
	GN_MENU_ITEM *gb = (GN_MENU_ITEM *) b;

	return strcmp(ga->name, gb->name);
}

static int loadrom_action(GN_MENU_ITEM *self, void *param) {
	char *game = (char*) self->arg;

	printf("Loading %s\n", game);
	close_game();
	//if (conf.sound) close_sdl_audio();

	if (init_game(game) != GN_TRUE) {
		printf("Can't init %s...\n", game);
		gn_popup_error("Error! :", "Gngeo Couldn't init %s: \n\n%s\n"
				"Maybe the romset you're using is too old", game, gnerror);
		return MENU_STAY;
	}

	return MENU_RETURNTOGAME;
}

static volatile int scaning = 0;

int rom_browser_scanning_anim(void *data) {
	int i = 0;
	while (scaning) {
		draw_back();
		if (i > 20)
			draw_string(menu_buf, sfont, MENU_TITLE_X, MENU_TITLE_Y,
					"Scanning ...");
		else
			draw_string(menu_buf, sfont, MENU_TITLE_X, MENU_TITLE_Y,
					"Scanning");
		SDL_BlitSurface(menu_buf, NULL, buffer, NULL);
		screen_update();
		frame_skip(0);
		i++;
		if (i > 40)
			i = 0;
	}
	return 0;
}
static void free_rom_browser_menu(void);
void init_rom_browser_menu(void);
static int changedir_action(GN_MENU_ITEM *self, void *param) {
	char *dir = (char*) self->arg;
	SDL_Thread *anim_th;

//	char *rpath = strdup(CF_STR(cf_get_item_by_name("rompath")));
//	char *apath;
//	printf("change to dir %s\n", dir);
//	apath = realpath(rpath, NULL);
//	printf("%p\n", apath);
//	printf("%s\n", apath);
//	snprintf(CF_STR(cf_get_item_by_name("rompath")), CF_MAXSTRLEN, "%s", apath);
//	printf("Old dir %s\n", CF_STR(cf_get_item_by_name("rompath")));
	gn_strncat_dir(CF_STR(cf_get_item_by_name("rompath")), dir, CF_MAXSTRLEN);
	//printf("New path %s\n", CF_STR(cf_get_item_by_name("rompath")));
	cf_save_option(NULL, "rompath", 0);

	scaning = 1;
	anim_th = SDL_CreateThread(rom_browser_scanning_anim, NULL);
	free_rom_browser_menu();
	init_rom_browser_menu();
	scaning = 0;
	SDL_WaitThread(anim_th, NULL);

	return MENU_STAY;
}
static void free_rom_browser_menu(void) {
	LIST *i, *t;
	if (rbrowser_menu == NULL)
		return;

	for (i = rbrowser_menu->item; i; i = i->next) {
		GN_MENU_ITEM *gitem;
		gitem = (GN_MENU_ITEM*) i->data;
		if (gitem->name)
			free(gitem->name);
		if (gitem->arg)
			free(gitem->arg);
		free(gitem);
	}
	i = rbrowser_menu->item;
	while (i) {
		t = i;
		i = i->next;
		free(t);
	}
}

void init_rom_browser_menu(void) {
	int i;
	int nbf;
	char filename[strlen(CF_STR(cf_get_item_by_name("rompath"))) + 256];
	struct stat filestat;
	struct dirent **namelist;
	ROM_DEF *drv = NULL;
	//char name[32];
	int nb_roms = 0;
	DIR *dh;
	struct dirent *file;

	rbrowser_menu = create_menu("Load Game", MENU_SMALL, NULL, NULL);

	if ((dh = opendir(CF_STR(cf_get_item_by_name("rompath")))) != NULL) {
		while ((file = readdir(dh)) != NULL) {
			sprintf(filename, "%s/%s", CF_STR(cf_get_item_by_name("rompath")),
					file->d_name);
			if (stat(filename, &filestat) == 0) {
				/* Directory */
				if (S_ISDIR(filestat.st_mode)
						|| (strcmp(file->d_name, ".") == 0
								&& strcmp(file->d_name, "..") == 0)) {
					rbrowser_menu->item = list_insert_sort(
							rbrowser_menu->item,
							(void*) gn_menu_create_item(file->d_name,
									MENU_ACTION, changedir_action,
									strdup(file->d_name)), romnamesort);
					rbrowser_menu->nb_elem++;
					continue;
				}
				if (S_ISREG(filestat.st_mode)) {
					/* GNO files */
					if (strstr(filename, ".gno") != NULL) {
						printf("GNO %s\n",filename);
						char *gnoname = dr_gno_romname(filename);
						printf("gnoname %s\n",gnoname);
						if (gnoname != NULL && (drv = res_load_drv(gnoname))!=NULL) {
							rbrowser_menu->item = list_insert_sort(
									rbrowser_menu->item,
									(void*) gn_menu_create_item(drv->longname,
											MENU_ACTION, loadrom_action,
											strdup(filename)), romnamesort);
							rbrowser_menu->nb_elem++;
							nb_roms++;
							free(drv);
						}
						continue;
					}
					if (strstr(filename, ".zip") == NULL)
						continue;
					i = 0;
					/* Standard .zip roms */
					while (romlist[i]) {
						if (strstr(filename, romlist[i]) != NULL) {
							if ((drv = dr_check_zip(filename)) != NULL) {
								rbrowser_menu->item = list_insert_sort(
										rbrowser_menu->item,
										(void*) gn_menu_create_item(
												drv->longname, MENU_ACTION,
												loadrom_action,
												strdup(drv->name)),
										romnamesort);
								rbrowser_menu->nb_elem++;

								free(drv);
								nb_roms++;
							}
						}
						i++;
					}

				}
			}
		}
		closedir(dh);
	}
//
//	i = 0;
//	while (romlist[i]) {
//		sprintf(filename, "%s/%s.zip", CF_STR(cf_get_item_by_name("rompath")),
//				romlist[i]);
//		if (stat(filename, &filestat) == 0 && S_ISREG(filestat.st_mode)) {
//			if ((drv = dr_check_zip(filename)) != NULL) {
//				rbrowser_menu->item = list_insert_sort(
//						rbrowser_menu->item,
//						(void*) gn_menu_create_item(drv->longname, MENU_ACTION,
//								loadrom_action, strdup(drv->name)),
//						romnamesort);
//				rbrowser_menu->nb_elem++;
//
//				free(drv);
//				nb_roms++;
//			}
//		}
//		sprintf(filename, "%s/%s.gno", CF_STR(cf_get_item_by_name("rompath")),
//				romlist[i]);
//		if (stat(filename, &filestat) == 0 && S_ISREG(filestat.st_mode)) {
//			char *gnoname = dr_gno_romname(filename);
//			if (gnoname != NULL) {
//				rbrowser_menu->item = list_insert_sort(
//						rbrowser_menu->item,
//						(void*) gn_menu_create_item(filename, MENU_ACTION,
//								loadrom_action, strdup(filename)), romnamesort);
//				rbrowser_menu->nb_elem++;
//				nb_roms++;
//			}
//}
//
//		i++;
//	}

if (nb_roms == 0) {
	rbrowser_menu->item = list_append(
			rbrowser_menu->item,
			(void*) gn_menu_create_item("No Games Found...", MENU_ACTION,
					NULL, NULL));
	rbrowser_menu->nb_elem++;
}
}

int rom_browser_menu(void) {
static Uint32 init = 0;
int a;
SDL_Thread *anim_th;

if (init == 0) {
	char *rpath = strdup(CF_STR(cf_get_item_by_name("rompath")));
	char *apath;
	init = 1;

	/* in case the user  */
	apath = realpath(rpath, NULL);
	if (apath == NULL)
#ifdef EMBEDDED_FS
		apath=realpath(ROOTPATH"roms/", NULL);
#else
		apath = realpath(".", NULL);
#endif
	snprintf(CF_STR(cf_get_item_by_name("rompath")), CF_MAXSTRLEN, "%s", apath);
	free(apath);
	free(rpath);

	scaning = 1;
	anim_th = SDL_CreateThread(rom_browser_scanning_anim, NULL);
	init_rom_browser_menu();
	scaning = 0;
	SDL_WaitThread(anim_th, NULL);
}

while (1) {
	rbrowser_menu->draw(rbrowser_menu); //frame_skip(0);printf("fps: %s\n",fps_str);
	if ((a = rbrowser_menu->event_handling(rbrowser_menu)) > 0) {
		if (a == MENU_CLOSE)
			return MENU_STAY;
		else
			return a;
	}
}
/* Should never go to this place */
return MENU_STAY;
}

static int rbrowser_action(GN_MENU_ITEM *self, void *param) {
//exit(0);
return rom_browser_menu();
}

static int toggle_fullscreen(GN_MENU_ITEM *self, void *param) {
screen_fullscreen();
self->val = 1 - self->val;
cf_item_has_been_changed(cf_get_item_by_name("fullscreen"));
CF_BOOL(cf_get_item_by_name("fullscreen")) = self->val;
return MENU_STAY;
}
#ifdef PANDORA
static int toggle_wide(GN_MENU_ITEM *self, void *param) {
self->val = 1 - self->val;

cf_item_has_been_changed(cf_get_item_by_name("wide"));
CF_BOOL(cf_get_item_by_name("wide")) = self->val;
screen_reinit();
return MENU_STAY;
}
#endif
static int toggle_vsync(GN_MENU_ITEM *self, void *param) {

self->val = 1 - self->val;
conf.vsync = self->val;
cf_item_has_been_changed(cf_get_item_by_name("vsync"));
CF_BOOL(cf_get_item_by_name("vsync")) = self->val;
screen_reinit();
return MENU_STAY;
}

static int toggle_autoframeskip(GN_MENU_ITEM *self, void *param) {
self->val = 1 - self->val;
conf.autoframeskip = self->val;
cf_item_has_been_changed(cf_get_item_by_name("autoframeskip"));
CF_BOOL(cf_get_item_by_name("autoframeskip")) = self->val;
reset_frame_skip();
return MENU_STAY;
}

static int toggle_sleepidle(GN_MENU_ITEM *self, void *param) {
self->val = 1 - self->val;
conf.sleep_idle = self->val;
cf_item_has_been_changed(cf_get_item_by_name("sleepidle"));
CF_BOOL(cf_get_item_by_name("sleepidle")) = self->val;

return MENU_STAY;
}

static int toggle_raster(GN_MENU_ITEM *self, void *param) {
self->val = 1 - self->val;
conf.raster = self->val;
cf_item_has_been_changed(cf_get_item_by_name("raster"));
CF_BOOL(cf_get_item_by_name("raster")) = self->val;

return MENU_STAY;
}

static int toggle_showfps(GN_MENU_ITEM *self, void *param) {
self->val = 1 - self->val;
conf.show_fps = self->val;
cf_item_has_been_changed(cf_get_item_by_name("showfps"));
CF_BOOL(cf_get_item_by_name("showfps")) = self->val;

return MENU_STAY;
}

static int change_effect_action(GN_MENU_ITEM *self, void *param) {
char *ename = (char *) self->arg;
printf("Toggle to effect %s\n", self->name);
if (strcmp(ename, "none") != 0 || strcmp(ename, "soft") != 0) {
	scale = 1;
}
strncpy(CF_STR(cf_get_item_by_name("effect")), ename, 254);
cf_item_has_been_changed(cf_get_item_by_name("effect"));
screen_reinit();
return MENU_STAY;
}
extern effect_func effect[];

static int change_effect(GN_MENU_ITEM *self, void *param) {
static int init = 0;
int a;
int i;
if (init == 0) {

	init = 1;
	effect_menu = create_menu("Choose an Effect", MENU_SMALL, NULL, NULL);

	i = 0;
	while (effect[i].name != NULL) {
		effect_menu->item = list_append(
				effect_menu->item,
				(void*) gn_menu_create_item(effect[i].desc, MENU_ACTION,
						change_effect_action, (void*) effect[i].name));
		effect_menu->nb_elem++;
		i++;
	}
}
while (1) {
	effect_menu->draw(effect_menu); //frame_skip(0);printf("fps: %s\n",fps_str);
	if ((a = effect_menu->event_handling(effect_menu)) > 0) {
		self->str = CF_STR(cf_get_item_by_name("effect"));
		return MENU_STAY;
	}
}
return 0;
}

static int change_samplerate_action(GN_MENU_ITEM *self, void *param) {
int rate = (int) self->arg;

if (rate != 0) {

	CF_VAL(cf_get_item_by_name("samplerate")) = rate;
	cf_item_has_been_changed(cf_get_item_by_name("samplerate"));
	if (conf.sound && conf.game)
		close_sdl_audio();
	else
		cf_item_has_been_changed(cf_get_item_by_name("sound"));
	conf.sound = 1;
	CF_BOOL(cf_get_item_by_name("sound")) = 1;
	conf.sample_rate = rate;
	//init_sdl_audio();
	//YM2610ChangeSamplerate(conf.sample_rate);
	if (conf.game) {
		init_sdl_audio();
		YM2610ChangeSamplerate(conf.sample_rate);
	}

} else {
	if (conf.sound)
		cf_item_has_been_changed(cf_get_item_by_name("sound"));
	conf.sound = 0;
	conf.sample_rate = 0;
	if (conf.game)
		close_sdl_audio();
	CF_BOOL(cf_get_item_by_name("sound")) = 0;
}

return MENU_CLOSE;
}

static int change_samplerate(GN_MENU_ITEM *self, void *param) {
static int init = 0;
int a;
GN_MENU_ITEM *gitem;
if (init == 0) {
	init = 1;
	srate_menu = create_menu("Choose a sample rate", MENU_SMALL, NULL, NULL);
	gitem = gn_menu_create_item("No sound", MENU_ACTION,
			change_samplerate_action, (void*) 0);
	srate_menu->item = list_append(srate_menu->item, (void*) gitem);
	srate_menu->nb_elem++;

	gitem = gn_menu_create_item("11025 (Fast but poor quality)", MENU_ACTION,
			change_samplerate_action, (void*) 11025);
	srate_menu->item = list_append(srate_menu->item, (void*) gitem);
	srate_menu->nb_elem++;

	gitem = gn_menu_create_item("22050 (Good compromise)", MENU_ACTION,
			change_samplerate_action, (void*) 22050);
	srate_menu->item = list_append(srate_menu->item, (void*) gitem);
	srate_menu->nb_elem++;

	gitem = gn_menu_create_item("44100 (Best quality)", MENU_ACTION,
			change_samplerate_action, (void*) 44100);
	srate_menu->item = list_append(srate_menu->item, (void*) gitem);
	srate_menu->nb_elem++;
}

//	gn_menu_disable_item(srate_menu,"No sound");

while (1) {
	srate_menu->draw(srate_menu); //frame_skip(0);printf("fps: %s\n",fps_str);
	if ((a = srate_menu->event_handling(srate_menu)) > 0) {
		if (conf.sound)
			sprintf(self->str, "%d", conf.sample_rate);
		else
			sprintf(self->str, "No sound");

		return MENU_STAY;
	}
}
return 0;
}

static int save_conf_action(GN_MENU_ITEM *self, void *param) {
int type = (int) self->arg;
if (type == 0)
	cf_save_file(NULL, 0);
else {
	char *gpath;
	char *drconf;
	char *name = memory.rom.info.name;
#ifdef EMBEDDED_FS
	gpath = ROOTPATH"conf/";
#else
	gpath = get_gngeo_dir();
#endif
	drconf = alloca(strlen(gpath) + strlen(name) + strlen(".cf") + 1);
	sprintf(drconf, "%s%s.cf", gpath, name);
	cf_save_file(drconf, 0);
}
return GN_TRUE;
}

#define RESET_BOOL(name,id) gitem=gn_menu_get_item_by_name(option_menu,name);\
if (gitem) gitem->val = CF_BOOL(cf_get_item_by_name(id));

static void reset_menu_option(void) {
GN_MENU_ITEM *gitem;
//gitem=gn_menu_get_item_by_name(option_menu,"Fullscreen");
//if (gitem) gitem->val = CF_BOOL(cf_get_item_by_name("fullscreen"));
RESET_BOOL("Fullscreen", "fullscreen");
RESET_BOOL("Vsync", "vsync");
RESET_BOOL("Auto Frame Skip", "autoframeskip");
RESET_BOOL("Sleep while idle", "sleepidle");
RESET_BOOL("Show FPS", "showfps");
#ifdef PANDORA
RESET_BOOL("16/9","wide");
#endif
gitem = gn_menu_get_item_by_name(option_menu, "Effect");
gitem->str = CF_STR(cf_get_item_by_name("effect"));

gitem = gn_menu_get_item_by_name(option_menu, "Sample Rate");
if (conf.sound)
	sprintf(gitem->str, "%d", conf.sample_rate);
else
	sprintf(gitem->str, "No sound");
}

static int option_action(GN_MENU_ITEM *self, void *param) {
//exit(0);
int a;
reset_menu_option();
while (1) {
	option_menu->draw(option_menu); //frame_skip(0);printf("fps: %s\n",fps_str);
	if ((a = option_menu->event_handling(option_menu)) > 0) {
		reset_menu_option();
		return MENU_STAY;
	}
}
/* should never go here */
return 0;
}

void gn_init_menu(void) {
GN_MENU_ITEM *gitem;
main_menu = create_menu(NULL, MENU_BIG, NULL, NULL);

main_menu->item = list_append(
		main_menu->item,
		(void*) gn_menu_create_item("Load game", MENU_ACTION, rbrowser_action,
				NULL));
main_menu->nb_elem++;

main_menu->item = list_append(
		main_menu->item,
		(void*) gn_menu_create_item("Load state", MENU_ACTION,
				load_state_action, NULL));
main_menu->nb_elem++;
main_menu->item = list_append(
		main_menu->item,
		(void*) gn_menu_create_item("Save state", MENU_ACTION,
				save_state_action, NULL));
main_menu->nb_elem++;

main_menu->item = list_append(
		main_menu->item,
		(void*) gn_menu_create_item("Option", MENU_ACTION, option_action,
				NULL));
main_menu->nb_elem++;

/*
 main_menu->item = list_append(main_menu->item,
 (void*) gn_menu_create_item("Credit", MENU_ACTION, credit_action, NULL));
 main_menu->nb_elem++;
 */

main_menu->item = list_append(main_menu->item,
		(void*) gn_menu_create_item("Exit", MENU_ACTION, exit_action, NULL));
main_menu->nb_elem++;

option_menu = create_menu("Options", MENU_SMALL, NULL, NULL);

gitem = gn_menu_create_item("Fullscreen", MENU_CHECK, toggle_fullscreen, NULL);
gitem->val = CF_BOOL(cf_get_item_by_name("fullscreen"));
option_menu->item = list_append(option_menu->item, (void*) gitem);
option_menu->nb_elem++;
#ifdef PANDORA
gitem = gn_menu_create_item("16/9", MENU_CHECK, toggle_wide, NULL);
gitem->val = CF_BOOL(cf_get_item_by_name("wide"));
option_menu->item = list_append(option_menu->item, (void*) gitem);
option_menu->nb_elem++;
#endif
gitem = gn_menu_create_item("Vsync", MENU_CHECK, toggle_vsync, NULL);
gitem->val = CF_BOOL(cf_get_item_by_name("vsync"));
option_menu->item = list_append(option_menu->item, (void*) gitem);
option_menu->nb_elem++;

gitem = gn_menu_create_item("Auto Frame Skip", MENU_CHECK, toggle_autoframeskip,
		NULL);
gitem->val = CF_BOOL(cf_get_item_by_name("autoframeskip"));
option_menu->item = list_append(option_menu->item, (void*) gitem);
option_menu->nb_elem++;

gitem = gn_menu_create_item("Sleep while idle", MENU_CHECK, toggle_sleepidle,
		NULL);
gitem->val = CF_BOOL(cf_get_item_by_name("sleepidle"));
option_menu->item = list_append(option_menu->item, (void*) gitem);
option_menu->nb_elem++;

gitem = gn_menu_create_item("Show FPS", MENU_CHECK, toggle_showfps, NULL);
gitem->val = CF_BOOL(cf_get_item_by_name("showfps"));
option_menu->item = list_append(option_menu->item, (void*) gitem);
option_menu->nb_elem++;

gitem = gn_menu_create_item("Enable Raster effect", MENU_CHECK, toggle_raster,
		NULL);
gitem->val = CF_BOOL(cf_get_item_by_name("raster"));
option_menu->item = list_append(option_menu->item, (void*) gitem);
option_menu->nb_elem++;

gitem = gn_menu_create_item("Effect", MENU_LIST, change_effect, NULL);
gitem->str = CF_STR(cf_get_item_by_name("effect"));
option_menu->item = list_append(option_menu->item, (void*) gitem);
option_menu->nb_elem++;

gitem = gn_menu_create_item("Sample Rate", MENU_LIST, change_samplerate, NULL);
gitem->str = malloc(32);
if (conf.sound)
	sprintf(gitem->str, "%d", conf.sample_rate);
else
	sprintf(gitem->str, "No sound");
option_menu->item = list_append(option_menu->item, (void*) gitem);
option_menu->nb_elem++;

gitem = gn_menu_create_item("Save conf for every game", MENU_ACTION,
		save_conf_action, (void*) 0);
option_menu->item = list_append(option_menu->item, (void*) gitem);
option_menu->nb_elem++;

gitem = gn_menu_create_item("Save conf for this game", MENU_ACTION,
		save_conf_action, (void*) 1);
option_menu->item = list_append(option_menu->item, (void*) gitem);
option_menu->nb_elem++;

yesno_menu = create_menu(NULL, MENU_SMALL, NULL, NULL);
gitem = gn_menu_create_item("Yes", MENU_ACTION, yes_action, NULL);
yesno_menu->item = list_append(yesno_menu->item, (void*) gitem);
yesno_menu->nb_elem++;
gitem = gn_menu_create_item("no", MENU_ACTION, no_action, NULL);
yesno_menu->item = list_append(yesno_menu->item, (void*) gitem);
yesno_menu->nb_elem++;
}

Uint32 run_menu(void) {
static Uint32 init = 0;
int a;

if (init == 0) {
	init = 1;
	gn_init_menu();
}

init_back();

reset_event();
//	conf.autoframeskip = 1;
reset_frame_skip();

gn_menu_disable_item(main_menu, "Load state");
if (conf.game == NULL) {
	gn_menu_disable_item(main_menu, "Save state");
	gn_menu_disable_item(option_menu, "Save conf for this game");
} else {
	Uint32 nb_slot = how_many_slot(conf.game);
	gn_menu_enable_item(main_menu, "Save state");
	gn_menu_enable_item(option_menu, "Save conf for this game");
	if (nb_slot > 0)
		gn_menu_enable_item(main_menu, "Load state");
}

while (1) {
	main_menu->draw(main_menu); //frame_skip(0);printf("fps: %s\n",fps_str);
	if ((a = main_menu->event_handling(main_menu)) > 0)
		//reset_event();
		return a;
}
//reset_event();
if (conf.game == NULL)
	return 2; /* Exit */
return 0;
}

