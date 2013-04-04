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

/* cyclone interface */



#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef USE_CYCLONE

#include <stdlib.h>

#include "cyclone/Cyclone.h"
#include "_memory.h"
#include "emu.h"
#include "state.h"
#include "debug.h"
#include "video.h"
#include "conf.h"

struct Cyclone MyCyclone;
static int total_cycles;
static int time_slice;
Uint32 cyclone_pc;
extern int current_line;
Uint8 save_buffer[128];

extern Uint32 irq2pos_value;
static __inline__ void cyclone68k_store_video_word(Uint32 addr, Uint16 data)
{
	//SDL_Swap16(data);
	//printf("mem68k_store_video_word %08x %04x\n",addr,data);

    addr &= 0xFFFF;
    switch (addr) {
    case 0x0:
    	memory.vid.vptr = data & 0xffff;
	break;
    case 0x2:	
	//if (((vptr<<1)==0x10800+0x8) ) printf("Store to video %08x @pc=%08x\n",vptr<<1,cpu_68k_getpc());
	/*
	  if (((vptr<<1)==0x10000+0x17e) ||
	  ((vptr<<1)==0x10400+0x17e) ||
	  ((vptr<<1)==0x10800+0x17e) ) printf("Store to video %08x @pc=%08x\n",vptr<<1,cpu_68k_getpc());
	*/
	WRITE_WORD(&memory.vid.ram[memory.vid.vptr << 1], data);
	memory.vid.vptr = (memory.vid.vptr + memory.vid.modulo) & 0xffff;
	break;
    case 0x4:
    	memory.vid.modulo = (int) data;
	break;
    case 0x6:
	write_neo_control(data);
	break;
    case 0x8:
	write_irq2pos((memory.vid.irq2pos & 0xffff) | ((Uint32) data << 16));
	break;
    case 0xa:
	write_irq2pos((memory.vid.irq2pos & 0xffff0000) | (Uint32) data);
	break;
    case 0xc:
	/* games write 7 or 4 at 0x3c000c at every frame */
	/* IRQ acknowledge */
	break;
    }
       

}

static void print_one_reg(Uint32 r) {
	printf("reg=%08x\n",r);
}

static void swap_memory(Uint8 * mem, Uint32 length)
{
    int i, j;

    /* swap bytes in each word */
    for (i = 0; i < length; i += 2) {
	j = mem[i];
	mem[i] = mem[i + 1];
	mem[i + 1] = j;
    }
}

static unsigned int   MyCheckPc(unsigned int pc) {
	int i;
	pc-=MyCyclone.membase; // Get the real program counter

	pc&=0xFFFFFF;
	MyCyclone.membase=-1;

/*	printf("## Check pc %08x\n",pc);
	for(i=0;i<8;i++) printf("  d%d=%08x a%d=%08x\n",i,MyCyclone.d[i],i,MyCyclone.a[i]); */

	//printf("PC %08x %08x\n",(pc&0xF00000),(pc&0xF00000)>>20);
	switch((pc&0xF00000)>>20) {
	case 0x0:
		MyCyclone.membase=(int)memory.rom.cpu_m68k.p;
		break;
	case 0x2:
		MyCyclone.membase=(int)(memory.rom.cpu_m68k.p+bankaddress)-0x200000;
		break;
	case 0x1:
		if (pc<=0x10FFff) 
			MyCyclone.membase=(int)memory.ram-0x100000;
		break;
	case 0xC:
		if (pc<=0xc1FFff)
			MyCyclone.membase=(int)memory.rom.bios_m68k.p-0xc00000;
		break;
	}

 	if (MyCyclone.membase==-1) {
 		printf("ERROROROOR %08x\n",pc);
 		exit(1);
 	}

	return MyCyclone.membase+pc; // New program counter
}

#define MEMHANDLER_READ(start,end,func) {if (a>=start && a<=end) return func(a);} 
#define MEMHANDLER_WRITE(start,end,func) {if (a>=start && a<=end) {func(a,d);return;}}

static unsigned int  MyRead8  (unsigned int a) {
	unsigned int addr=a&0xFFFFF;
	unsigned int b=((a&0xF0000)>>16);
	a&=0xFFFFFF;
	switch((a&0xFF0000)>>20) {
	case 0x0:
		return (READ_BYTE_ROM(memory.rom.cpu_m68k.p + addr))&0xFF;
		break;
	case 0x2:
		if (memory.bksw_unscramble)
			return mem68k_fetch_bk_normal_byte(a);
		return (READ_BYTE_ROM(memory.rom.cpu_m68k.p + bankaddress + addr))&0xFF;
		break;
	case 0x1:
		return (READ_BYTE_ROM(memory.ram + (addr&0xFFFF)))&0xFF;
		break;
	case 0xC:
		if (b<=1) return (READ_BYTE_ROM(memory.rom.bios_m68k.p + addr))&0xFF;
		break;
	case 0xd:
		if (b==0) return mem68k_fetch_sram_byte(a)&0xFF;
		break;
	case 0x4:
		if (b==0) return mem68k_fetch_pal_byte(a)&0xFF;
		break;
	case 0x3:
		if (b==0xC) return mem68k_fetch_video_byte(a)&0xFF;
		if (b==0) return mem68k_fetch_ctl1_byte(a)&0xFF;
		if (b==4) return mem68k_fetch_ctl2_byte(a)&0xFF;
		if (b==8) return mem68k_fetch_ctl3_byte(a)&0xFF;
		if (b==2) return mem68k_fetch_coin_byte(a)&0xFF;
		break;
	case 0x8:
		if (b==0) return mem68k_fetch_memcrd_byte(a)&0xFF;
		break;
	}

	return 0xFF;
}
static unsigned int MyRead16 (unsigned int a) {
	unsigned int addr=a&0xFFFFF;
	unsigned int b=((a&0xF0000)>>16);
	//printf("read 32 %08x\n",a);
	a&=0xFFFFFF;

	switch((a&0xFF0000)>>20) {
	case 0x0:
		return (READ_WORD_ROM(memory.rom.cpu_m68k.p + addr))&0xFFFF;
		break;
	case 0x2:
		if (memory.bksw_unscramble)
			return mem68k_fetch_bk_normal_word(a);
		return (READ_WORD_ROM(memory.rom.cpu_m68k.p + bankaddress + addr))&0xFFFF;

		break;
	case 0x1:
		return (READ_WORD_ROM(memory.ram + (addr&0xFFFF)))&0xFFFF;
		break;
	case 0xC:
		if (b<=1) return (READ_WORD_ROM(memory.rom.bios_m68k.p + addr))&0xFFFF;
		break;

	case 0xd:
		if (b==0) return mem68k_fetch_sram_word(a)&0xFFFF;
		break;
	case 0x4:
		if (b==0) return mem68k_fetch_pal_word(a)&0xFFFF;
		break;
	case 0x3:
		if (b==0xC) return mem68k_fetch_video_word(a)&0xFFFF;
		if (b==0) return mem68k_fetch_ctl1_word(a)&0xFFFF;
		if (b==4) return mem68k_fetch_ctl2_word(a)&0xFFFF;
		if (b==8) return mem68k_fetch_ctl3_word(a)&0xFFFF;
		if (b==2) return mem68k_fetch_coin_word(a)&0xFFFF;
		break;
	case 0x8:
		if (b==0) return mem68k_fetch_memcrd_word(a)&0xFFFF;
		break;
	}

	return 0xF0F0;
}
static unsigned int   MyRead32 (unsigned int a) {
	//int i;
	unsigned int addr=a&0xFFFFF;
	unsigned int b=((a&0xF0000)>>16);
	a&=0xFFFFFF;

	switch((a&0xFF0000)>>20) {
	case 0x0:
		//return mem68k_fetch_cpu_long(a);
		return ((READ_WORD_ROM(memory.rom.cpu_m68k.p + addr))<<16) | 
			(READ_WORD_ROM(memory.rom.cpu_m68k.p + (addr+2)));
		break;
	case 0x2:
		//return mem68k_fetch_bk_normal_long(a);
		if (memory.bksw_unscramble)
			return mem68k_fetch_bk_normal_long(a);
		return ((READ_WORD_ROM(memory.rom.cpu_m68k.p + bankaddress + addr))<<16) | 
			(READ_WORD_ROM(memory.rom.cpu_m68k.p + bankaddress + (addr+2)));
		break;
	case 0x1:
		//return mem68k_fetch_ram_long(a);
		addr&=0xFFFF;
		return ((READ_WORD_ROM(memory.ram + addr))<<16) | 
			(READ_WORD_ROM(memory.ram + (addr+2)));
		break;
	case 0xC:
		//return mem68k_fetch_bios_long(a);
		if (b<=1) return ((READ_WORD_ROM(memory.rom.bios_m68k.p + addr))<<16) | 
				 (READ_WORD_ROM(memory.rom.bios_m68k.p + (addr+2)));
		break;

	case 0xd:
		if (b==0) return mem68k_fetch_sram_long(a);
		break;
	case 0x4:
		if (b==0) return mem68k_fetch_pal_long(a);
		break;
	case 0x3:
		if (b==0xC) return mem68k_fetch_video_long(a);
		if (b==0) return mem68k_fetch_ctl1_long(a);
		if (b==4) return mem68k_fetch_ctl2_long(a);
		if (b==8) return mem68k_fetch_ctl3_long(a);
		if (b==2) return mem68k_fetch_coin_long(a);
		break;
	case 0x8:
		if (b==0) return mem68k_fetch_memcrd_long(a);
		break;
	}

	return 0xFF00FF00;
}
static void MyWrite8 (unsigned int a,unsigned int  d) {
	unsigned int b=((a&0xF0000)>>16);
	a&=0xFFFFFF;
    d&=0xFF;
	switch((a&0xFF0000)>>20) {
	case 0x1:
		WRITE_BYTE_ROM(memory.ram + (a&0xffff),d);
		return ;
		break;
	case 0x3:
		if (b==0xc) {mem68k_store_video_byte(a,d);return;}
		if (b==8) {mem68k_store_pd4990_byte(a,d);return;}
		if (b==2) {mem68k_store_z80_byte(a,d);return;}
		if (b==0xA) {mem68k_store_setting_byte(a,d);return;}
		break;
	case 0x4:
		if (b==0) mem68k_store_pal_byte(a,d);
		return;
		break;
	case 0xD:
		if (b==0) mem68k_store_sram_byte(a,d);return;
		break;
	case 0x2:
		if (b==0xF) mem68k_store_bk_normal_byte(a,d);return;
		break;
	case 0x8:
		if (b==0) mem68k_store_memcrd_byte(a,d);return;
		break;

	}

	if(a==0x300001) memory.watchdog=0; // Watchdog

	//printf("Unhandled write8  @ %08x = %02x\n",a,d);
}
static void MyWrite16(unsigned int a,unsigned int d) {
	unsigned int b=((a&0xF0000)>>16);
	a&=0xFFFFFF;
    d&=0xFFFF;
    //if (d&0x8000) printf("WEIRD %x %x\n",a,d);

	switch((a&0xFF0000)>>20) {
	case 0x1:
		WRITE_WORD_ROM(memory.ram + (a&0xffff),d);
		return;
		//mem68k_store_ram_word(a,d);return;
		break;
	case 0x3:
		if (b==0xc) {
            mem68k_store_video_word(a,d);return;}
		if (b==8) {mem68k_store_pd4990_word(a,d);return;}
		if (b==2) {mem68k_store_z80_word(a,d);return;}
		if (b==0xA) {mem68k_store_setting_word(a,d);return;}
		break;	
	case 0x4:
		if (b==0) mem68k_store_pal_word(a,d);return;
		break;
	case 0xD:
		if (b==0) mem68k_store_sram_word(a,d);return;
		break;
	case 0x2:
		if (b==0xF) mem68k_store_bk_normal_word(a,d);return;
		break;
	case 0x8:
		if (b==0) mem68k_store_memcrd_word(a,d);return;
		break;
	}

	//printf("Unhandled write16 @ %08x = %04x\n",a,d);
}
static void MyWrite32(unsigned int a,unsigned int   d) {
	unsigned int b=((a&0xF0000)>>16);
	a&=0xFFFFFF;
    d&=0xFFFFFFFF;
		switch((a&0xFF0000)>>20) {
	case 0x1:
		WRITE_WORD_ROM(memory.ram + (a&0xffff),d>>16);
		WRITE_WORD_ROM(memory.ram + (a&0xffff)+2,d&0xFFFF);
		return;
		break;
	case 0x3:
		if (b==0xc) {
			mem68k_store_video_word(a,d>>16);
			mem68k_store_video_word(a+2,d & 0xffff);
			return;
		}
		if (b==8) {mem68k_store_pd4990_long(a,d);return;}
		if (b==2) {mem68k_store_z80_long(a,d);return;}
		if (b==0xA) {mem68k_store_setting_long(a,d);return;}
		break;	
	case 0x4:
		if (b==0) mem68k_store_pal_long(a,d);return;
		break;
	case 0xD:
		if (b==0) mem68k_store_sram_long(a,d);return;
		break;
	case 0x2:
		if (b==0xF) mem68k_store_bk_normal_long(a,d);return;
		break;
	case 0x8:
		if (b==0) mem68k_store_memcrd_long(a,d);return;
		break;
	}

	//printf("Unhandled write32 @ %08x = %08x\n",a,d);
}


void cpu_68k_mkstate(gzFile gzf,int mode) {
	printf("Save state mode %s PC=%08x\n",(mode==STREAD?"READ":"WRITE"),MyCyclone.pc-MyCyclone.membase);
	if (mode==STWRITE) CyclonePack(&MyCyclone, save_buffer);
	mkstate_data(gzf, save_buffer, 128, mode);
	if (mode == STREAD) CycloneUnpack(&MyCyclone, save_buffer);
	printf("Save state Phase 2 PC=%08x\n", (mode == STREAD ? "READ" : "WRITE"), MyCyclone.pc - MyCyclone.membase);
}
int cpu_68k_getcycle(void) {
	return total_cycles-MyCyclone.cycles;
}
void bankswitcher_init() {
	bankaddress=0;
}
int cyclone_debug(unsigned short o) {
	printf("CYCLONE DEBUG %04x\n",o);
	return 0;
}

void cpu_68k_init(void) {
	int overclk=CF_VAL(cf_get_item_by_name("68kclock"));
	//printf("INIT \n");
	CycloneInit();
	memset(&MyCyclone, 0,sizeof(MyCyclone));
	/*
	swap_memory(memory.rom.cpu_m68k.p, memory.rom.cpu_m68k.size);
	swap_memory(memory.rom.bios_m68k.p, memory.rom.bios_m68k.size);
	swap_memory(memory.game_vector, 0x80);
	*/
	MyCyclone.read8=MyRead8;
	MyCyclone.read16=MyRead16;
	MyCyclone.read32=MyRead32;

	MyCyclone.write8=MyWrite8;
	MyCyclone.write16=MyWrite16;
	MyCyclone.write32=MyWrite32;

	MyCyclone.checkpc=MyCheckPc;

	MyCyclone.fetch8  =MyRead8;
        MyCyclone.fetch16 =MyRead16;
        MyCyclone.fetch32 =MyRead32;

	//MyCyclone.InvalidOpCallback=cyclone_debug;
	//MyCyclone.print_reg=print_one_reg;
	bankswitcher_init();

	

	if (memory.rom.cpu_m68k.size > 0x100000) {
		bankaddress = 0x100000;
	}
	//cpu_68k_init_save_state();


	time_slice=(overclk==0?
		    200000:200000+(overclk*200000/100.0))/262.0;
}

void cpu_68k_reset(void) {

	//printf("Reset \n");
	MyCyclone.srh=0x27; // Set supervisor mode
	//CycloneSetSr(&MyCyclone,0x27);
	//MyCyclone.srh=0x20;
	//MyCyclone.irq=7;
	MyCyclone.irq=0;
	MyCyclone.a[7]=MyCyclone.read32(0);
	
	MyCyclone.membase=0;
	MyCyclone.pc=MyCyclone.checkpc(MyCyclone.read32(4)); // Get Program Counter

	//printf("PC=%08x\n SP=%08x\n",MyCyclone.pc-MyCyclone.membase,MyCyclone.a[7]);
}

int cpu_68k_run(Uint32 nb_cycle) {
	Uint32 i;
	if (conf.raster) {
		total_cycles=nb_cycle;MyCyclone.cycles=nb_cycle;	
		CycloneRun(&MyCyclone);
		return -MyCyclone.cycles;
	} else {
#if 1
		current_line=0;
		
		total_cycles=nb_cycle;
		//MyCyclone.cycles=nb_cycle;
		MyCyclone.cycles=0;//time_slice;
		for (i=0;i<261;i++) {
			MyCyclone.cycles+=time_slice;
			CycloneRun(&MyCyclone);
			current_line++;
		}
#else
		total_cycles=nb_cycle;MyCyclone.cycles=nb_cycle;	
		CycloneRun(&MyCyclone);
#endif
		return -MyCyclone.cycles;
	}
		//return 0;
}

void cpu_68k_interrupt(int a) {
	//printf("Interrupt %d\n",a);
	MyCyclone.irq=a;
}

void cpu_68k_bankswitch(Uint32 address) {
	//printf("Bankswitch %08x\n",address);
	bankaddress = address;
}

void cpu_68k_disassemble(int pc, int nb_instr) {
	/* TODO */
}

void cpu_68k_dumpreg(void) {
	int i;
	for(i=0;i<8;i++) printf("  d%d=%08x a%d=%08x\n",i,MyCyclone.d[i],i,MyCyclone.a[i]);
	/*printf("stack:\n");
	for (i=0;i<10;i++) {
		printf("%02d - %08x\n",i,mem68k_fetch_ram_long(MyCyclone.a[7]+i*4));
		}*/
	
}

int cpu_68k_run_step(void) {
	MyCyclone.cycles=0;
	CycloneRun(&MyCyclone);
	return -MyCyclone.cycles;
}

Uint32 cpu_68k_getpc(void) {
	return MyCyclone.pc-MyCyclone.membase;
}


int cpu_68k_debuger(void (*execstep)(void),void (*dump)(void)) {
	/* TODO */
	return 0;
}



#endif
