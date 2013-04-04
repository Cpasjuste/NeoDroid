LOCAL_PATH := $(call my-dir)

COMMON_OBJS := main.c android.c \
	gnutil.c pd4990a.c video.c \
	profiler.c list.c _memory.c \
	resfile.c timer.c frame_skip.c messages.c screen.c emu.c \
	neocrypt.c sound.c unzip.c debug.c state.c \
	conf.c transpack.c roms.c mame_layer.c \
	neoboot.c event.c stb_image.c \
	effect/scale2x.c effect/scanline.c \
	effect/hq2x.c effect/interp.c effect/lq2x.c effect/hq3x.c \
	effect/lq3x.c effect/hqx_common.c \
	blitter/overlay.c blitter/soft.c \
	ym2610/2610intf.c ym2610/ym2610.c 

#fileio.c 

include $(CLEAR_VARS)

LOCAL_MODULE := main
LOCAL_ARM_MODE   := arm
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../SDL/include 

LOCAL_CFLAGS += -DHAVE_CONFIG_H -DANDROID \
	-D_GNU_SOURCE=1 -D_REENTRANT -g -O3 -Wall \
	-ftracer -fstrength-reduce -Wno-unused -funroll-loops  -fomit-frame-pointer -fstrict-aliasing -ffast-math

ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
    LOCAL_CFLAGS += -mcpu=cortex-a8 -march=armv7-a -mtune=cortex-a8 -mfpu=vfpv3 -O3 -Os
endif

# Add your application source files here...
LOCAL_SRC_FILES := $(COMMON_OBJS) \
	generator68k_interf.c mamez80_interf.c \
	mamez80/z80.c \
	generator68k/cpu68k-0.c generator68k/cpu68k-1.c \
	generator68k/cpu68k-2.c generator68k/cpu68k-3.c \
	generator68k/cpu68k-4.c generator68k/cpu68k-5.c generator68k/cpu68k-6.c generator68k/cpu68k-7.c  \
	generator68k/cpu68k-8.c generator68k/cpu68k-9.c generator68k/cpu68k-a.c generator68k/cpu68k-b.c  \
	generator68k/cpu68k-c.c generator68k/cpu68k-d.c generator68k/cpu68k-e.c generator68k/cpu68k-f.c \
	generator68k/cpu68k.c generator68k/reg68k.c generator68k/diss68k.c generator68k/tab68k.c
#menu.c 

LOCAL_SHARED_LIBRARIES := SDL
LOCAL_LDLIBS := -lGLESv1_CM -llog -lz

include $(BUILD_SHARED_LIBRARY)

#################
### ASM CORES ###
#################

include $(CLEAR_VARS)

LOCAL_MODULE := main_asm
LOCAL_ARM_MODE   := arm
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../SDL/include 

LOCAL_CFLAGS += -DASM -DHAVE_CONFIG_H -DANDROID \
	-D_GNU_SOURCE=1 -D_REENTRANT -g -O3 -Wall \
	-ftracer -fstrength-reduce -Wno-unused -funroll-loops  -fomit-frame-pointer -fstrict-aliasing -ffast-math

ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
    LOCAL_CFLAGS += -mcpu=cortex-a8 -march=armv7-a -mtune=cortex-a8 -mfpu=vfpv3 -O3 -Os
endif

# Add your application source files here...
LOCAL_SRC_FILES := $(COMMON_OBJS) \
			cyclone_interf.c drz80_interf.c video_arm.s memcpy.S \
			cyclone/Cyclone.s drz80/DrZ80.s
#menu.c

LOCAL_SHARED_LIBRARIES := SDL
LOCAL_LDLIBS := -lGLESv1_CM -llog -lz

include $(BUILD_SHARED_LIBRARY)

