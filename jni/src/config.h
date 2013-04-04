/* config.h.  Generated from config.h.in by configure.  */
/* config.h.in.  Generated from configure.in by autoheader.  */

/* Define if building universal (internal helper macro) */
/* #undef AC_APPLE_UNIVERSAL_BUILD */

/* Undefine if your processor can read unaligned 32-bit values */
#define ALIGNLONGS ALIGNLONGS

/* Define if you build for dingux */
/* #undef DINGUX */

/* Define for embedded directory structure */
/* #undef EMBEDDED_FS */

/* Define to process audio on the 940t */
/* #undef ENABLE_940T */

/* Define if you build for gp2x */
/* #undef GP2X */

/* Define to 1 if you have the `atexit' function. */
/* #undef HAVE_ATEXIT */

/* Define to 1 if you have the `basename' function. */
/* #undef HAVE_BASENAME */

/* Define to 1 if you have the <dirent.h> header file, and it defines `DIR'.
   */
#define HAVE_DIRENT_H 1

/* Define to 1 if you don't have `vprintf' but do have `_doprnt.' */
/* #undef HAVE_DOPRNT */

/* Define to 1 if you have the <errno.h> header file. */
#define HAVE_ERRNO_H 1

/* Define to 1 if you have the `floor' function. */
/* #undef HAVE_FLOOR */

/* Define to 1 if you have the `getopt_long' function. */
/* #undef HAVE_GETOPT_LONG */

/* Define to 1 if you have the `gettimeofday' function. */
/* #undef HAVE_GETTIMEOFDAY */

/* Define to 1 if you have the <GL/gl.h> header file. */
/* #undef HAVE_GL_GL_H */

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the `m' library (-lm). */
/* #undef HAVE_LIBM */
#define HAVE_LIBM 1

/* Define to 1 if you have the `z' library (-lz). */
/* #undef HAVE_LIBZ */
#define HAVE_LIBZ 1

/* Define to 1 if you have the <limits.h> header file. */
#define HAVE_LIMITS_H 1

/* Define to 1 if you have the <malloc.h> header file. */
#define HAVE_MALLOC_H 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the `memset' function. */
/* #undef HAVE_MEMSET */
#define HAVE_MEMSET 1

/* Define to 1 if you have the `mkdir' function. */
/* #undef HAVE_MKDIR */
#define HAVE_MKDIR 1

/* Define to 1 if you have the `mmap' function. */
/* #undef HAVE_MMAP */
#define HAVE_MMAP 1

/* Define to 1 if you have the <ndir.h> header file, and it defines `DIR'. */
/* #undef HAVE_NDIR_H */

/* Define to 1 if you have the `pow' function. */
/* #undef HAVE_POW */

/* Define to 1 if you have the `scandir' function. */
/* #undef HAVE_SCANDIR */

/* Define to 1 if you have the <stddef.h> header file. */
#define HAVE_STDDEF_H 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the `strchr' function. */
/* #undef HAVE_STRCHR */

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the `strstr' function. */
/* #undef HAVE_STRSTR */

/* Define to 1 if you have the `strtoul' function. */
/* #undef HAVE_STRTOUL */

/* Define to 1 if you have the <sys/dir.h> header file, and it defines `DIR'.
   */
/* #undef HAVE_SYS_DIR_H */

/* Define to 1 if you have the <sys/ndir.h> header file, and it defines `DIR'.
   */
/* #undef HAVE_SYS_NDIR_H */

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/time.h> header file. */
#define HAVE_SYS_TIME_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Define to 1 if you have the `vprintf' function. */
/* #undef HAVE_VPRINTF */

/* Name of package */
#define PACKAGE "gngeo"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT ""

/* Define to the full name of this package. */
#define PACKAGE_NAME "gngeo"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "gngeo 0.8"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "gngeo"

/* Define to the home page for this package. */
#define PACKAGE_URL ""

/* Define to the version of this package. */
#define PACKAGE_VERSION "0.8"

#ifdef ASM
#define PROCESSOR_ARM 1
#define USE_CYCLONE 1
#define USE_DRZ80 1
#else
#define USE_GENERATOR68K 1
#define USE_MAMEZ80 1
#endif

/* Define if you build for pandora */
/* #undef PANDORA */

/* Define if you have an x86_64 processor */
/* #undef PROCESSOR_ADM64 */

/* Define if you have an ARM processor */
//#define PROCESSOR_ARM 1

/* Define if you have an x86 processor */
/* #undef PROCESSOR_INTEL */

/* Define if you have a sparc processor */
/* #undef PROCESSOR_SPARC */

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Define to 1 if you can safely include both <sys/time.h> and <time.h>. */
#define TIME_WITH_SYS_TIME 1

/* Define to 1 if your <sys/time.h> declares `struct tm'. */
/* #undef TM_IN_SYS_TIME */

/* Define to enable cyclone */
//#define USE_CYCLONE 1

/* Define to enable drz80 */
//#define USE_DRZ80 1

/* Define to enable generator68k */
/* #undef USE_GENERATOR68K */

/* Define to use alternative opengl blitter */
// #define USE_GL2 1

/* Define to enable mamez80 */
/* #undef USE_MAMEZ80 */

/* Define to enable raze */
/* #undef USE_RAZE */

/* Define to enable starscream */
/* #undef USE_STARSCREAM */
//#define USE_STARSCREAM 1

/* Version number of package */
#define VERSION "0.8"

/* Define if you build for wii */
/* #undef WII */

/* Define if you build for win32 */
/* #undef WIN32 */

/* Define if you build for wiz */
/* #undef WIZ */


/* Define WORDS_BIGENDIAN to 1 if your processor stores words with the most
   significant byte first (like Motorola and SPARC, unlike Intel). */
#if defined AC_APPLE_UNIVERSAL_BUILD
# if defined __BIG_ENDIAN__
#  define WORDS_BIGENDIAN 1
# endif
#else
# ifndef WORDS_BIGENDIAN
/* #  undef WORDS_BIGENDIAN */
# endif
#endif

/* Define to empty if `const' does not conform to ANSI C. */
/* #undef const */

/* Define to `__inline__' or `__inline' if that's what the C compiler
   calls it, or to nothing if 'inline' is not supported under any name.  */
#ifndef __cplusplus
/* #undef inline */
#endif
