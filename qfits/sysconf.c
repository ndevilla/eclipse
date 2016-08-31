/*----------------------------------------------------------------------------*/
/**
   @file    sysconf.c
   @author  Nicolas Devillard
   @date    February 2001
   @version	$Revision: 1.33 $
   @brief   configure-like in C.
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: sysconf.c,v 1.33 2005/02/24 15:59:59 yjung Exp $
	$Author: yjung $
	$Date: 2005/02/24 15:59:59 $
	$Revision: 1.33 $
 */

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <sys/stat.h>
 
#define MAXSTRSZ    512

#define MACROS_FILE		"config.make"
#define HEADER_FILE 	"config.h"
#define PREFIX_DEFAULT	"/usr/local"

#define COMPILER_AUTO	0
#define COMPILER_CC		1
#define COMPILER_GCC	2

#define XMEMDEBUG       " -DXMEMORY_DEBUG=1"

/* Global variable */

struct {
    /* List of supported OS's */
    enum {
        os_linux,
        os_cygwin,
        os_hp08,
        os_hp09,
        os_hp10,
        os_hp11,
        os_solaris,
        os_irix,
        os_aix,
        os_dec,
        os_bsd,
        os_darwin
    } local_os ;
    char sysname[MAXSTRSZ];
    char release[MAXSTRSZ];
    char machine[MAXSTRSZ];

    /* Bits per byte on this machine */
    int     bits_per_byte ;
    /* Big endian machine? */
    int     big_endian ;

    /* x86 architecture? */
    int     arch_x86 ;
    /* CPU type: 386, 486, 586, 686 */
    int     cpu_x86 ;

    /* Compile with debug options on */
    int		debug_compile ;
    /* Compile with warnings on */
    int		lint_compile ;
    /* Compiler type: native cc or gcc */
    int		compiler ;
    /* Compile with threads */
    int		with_threads ;
    /* Number of detected CPUs */
    int		ncpus ;
    /* Compile static library */
    int		lib_static ;
    /* Compile dynamic library */
    int		lib_dynamic ;
    /* Prefix for installation */
    char *	prefix ;
    /* With or without xmemory handling */
    int     xmemory_mode ;
    /* Maximum number of pointers allowed */
    int     max_ptrs ;
} config ;


/*
 * Check that all files present in the list of names are
 * somewhere present in the list of paths.
 */
int config_findfiles(paths, names)
    char ** paths ;
    char ** names ;
{
    struct stat sta ;
    char spath[MAXSTRSZ];
    int i, j ;
    int nfound ;

    /* Loop on all paths */
    i=0 ;
    while (paths[i]) {
        printf("searching in %s...\n", paths[i]);
        /* Loop on all file names */
        j=0 ;
        nfound=0 ;
        while (names[j]) {
            sprintf(spath, "%s/%s", paths[i], names[j]);
            if (stat(spath, &sta)==0) {
                nfound++ ;
            }
            j++ ;
        }
        if (j==nfound) {
            printf("found in %s\n", paths[i]);
            return i ;
        }
        i++ ;
    }
    return -1 ;
}


/* Guess local OS name and potentially release number */
void detect_config()
{
	struct utsname  name ;
	int		i ;
	unsigned char c ;
	int    x ;

    if (config.sysname[0]==0) {
        printf("detecting local OS............. ");
        /* Get machine OS type and release number */
        if (uname(&name)==-1) {
            printf("error calling uname\n");
            exit(-1);
        }
        printf("%s %s", name.sysname, name.release);
        strcpy(config.sysname, name.sysname);
        strcpy(config.release, name.release);
        strcpy(config.machine, name.machine);
    } else {
        printf("forcing config for OS.......... %s %s",
                config.sysname,
                config.release);
    }

	/* Lowercase everything */
	i=0 ;
	while(config.sysname[i]!=0) {
		config.sysname[i] = tolower(config.sysname[i]);
		i++ ;
	}
	i=0 ;
	while(config.release[i]!=0) {
		config.release[i] = tolower(config.release[i]);
		i++ ;
	}

	/* Switch on OS type and release number */
	if (strstr(config.sysname, "linux")!=NULL) {
		config.local_os = os_linux ;
		printf(" - linux\n");
	} else if (strstr(config.sysname, "cygwin")!=NULL) {
		config.local_os = os_cygwin ;
		printf(" - cygwin\n");
	} else if (strstr(config.sysname, "hp")!=NULL) {
		if (strstr(config.release, "8.")!=NULL) {
			config.local_os = os_hp08 ;
			printf(" - hpux_08\n");
		} else if (strstr(config.release, "9.")!=NULL) {
			config.local_os = os_hp09 ;
			printf(" - hpux_09\n");
		} else if (strstr(config.release, "10.")!=NULL) {
			config.local_os = os_hp10 ;
			printf(" - hpux_10\n");
		} else if (strstr(config.release, "11.")!=NULL) {
			config.local_os = os_hp11 ;
			printf(" - hpux_11\n");
		}
	} else if ((strstr(config.sysname, "sun")!=NULL) ||
			   (strstr(config.sysname, "solaris")!=NULL)) {
		config.local_os = os_solaris ;
		printf(" - solaris\n");
	} else if (strstr(config.sysname, "irix")!=NULL) {
		config.local_os = os_irix ;
		printf(" - irix\n");
	} else if (strstr(config.sysname, "aix")!=NULL) {
		config.local_os = os_aix ;
		printf(" - aix\n");
	} else if (strstr(config.sysname, "osf")!=NULL) {
		config.local_os = os_dec ;
		printf(" - osf/1\n");
	} else if (strstr(config.sysname, "bsd")!=NULL) {
		config.local_os = os_bsd ;
		printf(" - %s\n", config.sysname);
	} else if (strstr(config.sysname, "darwin")!=NULL) {
		config.local_os = os_darwin ;
		printf(" - %s\n", config.sysname);
	} else {
		printf("cannot identify your OS\n");
		printf("Use the option --os=NAME to force an OS type\n");
        exit(-1);
	}

    /* Find out about x86 architectures */
    if ((config.machine[0]=='i') && (strstr(config.machine, "86")!=NULL)) {
        config.arch_x86 = 1 ;
        config.cpu_x86 = atoi(config.machine+1) ;
        printf("detected x86 architecture...... %d\n", config.cpu_x86);
    } else {
        config.arch_x86 = 0 ;
        printf("detected x86 architecture...... no\n");
    }
    
    /* Compute number of bits in a byte on this machine */
	printf("computing bits per byte........ ");
	c=1 ; i=0 ;
	while (c) {
		c<<=1 ; i++ ;
	}
	config.bits_per_byte = i ;
    printf("%d\n", i);

    /* Detect endian-ness */
	printf("detecting byte-order........... ");
	x = 1 ;
	x = (*(char*)&x) ;

    config.big_endian = !x ;
    if (config.big_endian) {
        printf("big endian (motorola)\n");
    } else {
        printf("little endian (intel)\n");
    }

	return ;
}

/* Locate a program in the user's PATH */
int locate_program(pname)
	char * pname ;
{
	int		i, j, lg;
	char *	path ;
	char	buf[MAXSTRSZ];

	path = getenv("PATH");
    if (path!=NULL) {
        for (i=0; path[i]; ) {
            for (j=i ; (path[j]) && (path[j]!=':') ; j++);
            lg = j - i;
            strncpy(buf, path + i, lg);
            if (lg == 0) buf[lg++] = '.';
            buf[lg++] = '/';
            strcpy(buf + lg, pname);
            if (access(buf, X_OK) == 0) {
                printf("using [%s]\n", buf);
				return 1 ;
            }
            buf[0] = 0;
            i = j;
            if (path[i] == ':') i++ ;
        }
    } else {
		/* No PATH variable: abort with an error */
		return -1 ;
    }
    /* If the buffer is still empty, the command was not found */
    if (buf[0] == 0) return 0 ;
    /* Otherwise Ok */
    return 1 ;
}
	
void make_config_make()
{
	FILE	*	sysc ;

	/* Set compiler name */
	printf("looking for a C compiler....... ");
    if (getenv("PATH")==NULL) {
		printf("error: undefined PATH variable, cannot locate a compiler\n");
		exit(-1) ;
	}

	/* Open output sysconf */
	if ((sysc=fopen(MACROS_FILE, "w"))==NULL) {
		printf("cannot create %s: aborting compilation", MACROS_FILE);
		exit(-1) ;
	}

	/* In automatic mode, find out which compiler to use */
	if (config.compiler==COMPILER_AUTO) {
		switch (config.local_os) {

			/* Use gcc under Linux, BSD, Cygwin & IRIX */
			case os_linux:
			case os_cygwin:
			case os_bsd:
			case os_irix:
			config.compiler = COMPILER_GCC ;
			break ;

			/* All others use 'cc' (default) */
			default:
			config.compiler = COMPILER_CC ;
			break ;
		}
	}


	/* Make sure the compiler can be found */
	if (config.compiler==COMPILER_CC) {
		if (locate_program("cc")!=1) {
			printf("cannot locate cc\n");
			/* Try out with gcc */
			config.compiler = COMPILER_GCC ;
		}
	}
	if (config.compiler==COMPILER_GCC) {
		if (locate_program("gcc")!=1) {
			printf("cannot locate gcc\n");
			exit(-1);
		}
	}


	/* Set compiler name and cflags */
	switch (config.compiler) {

		case COMPILER_CC:
		fprintf(sysc, "CC      = cc\n");
		fprintf(sysc, "CFLAGS  = ");
        if (config.xmemory_mode == 0) {
            fprintf(sysc, "-DXMEMORY_MODE=%d ", config.xmemory_mode) ;
            fprintf(sysc, "-DXMEMORY_MAXPTRS=1 ") ;
        } else if (config.xmemory_mode == 1) {
            fprintf(sysc, "-DXMEMORY_MODE=%d ", config.xmemory_mode) ;
            fprintf(sysc, "-DXMEMORY_MAXPTRS=1 ") ;
        } else if (config.xmemory_mode == 2) {
            fprintf(sysc, "-DXMEMORY_MODE=%d ", config.xmemory_mode) ;
            if (config.max_ptrs > 0) {
                fprintf(sysc, "-DXMEMORY_MAXPTRS=%d ", config.max_ptrs) ;
            }
        } else if (config.xmemory_mode < 0) {
            if (config.max_ptrs > 0) {
                fprintf(sysc, "-DXMEMORY_MAXPTRS=%d ", config.max_ptrs) ;
            }
        }
        switch (config.local_os) {
			case os_hp08:
			case os_hp09:
			case os_hp10:
			if (config.with_threads) {
				printf("threads not supported on this platform\n");
			}
			if (config.debug_compile) {
				fprintf(sysc, "-Ae -g%s\n", XMEMDEBUG);
			} else {
				fprintf(sysc, "-Ae -O\n");
			}
			fprintf(sysc, "RELOC   = +z\n");
			fprintf(sysc, "SHARED  = -b\n");
			break ;

			case os_hp11:
			if (config.with_threads) {
				fprintf(sysc, "-lpthread ");
			}
			if (config.debug_compile) {
				fprintf(sysc, "-g%s\n", XMEMDEBUG);
			} else {
				fprintf(sysc, "-O\n");
			}
			fprintf(sysc, "RELOC   = +z\n");
			fprintf(sysc, "SHARED  = -b\n");
			break ;

			case os_solaris:
			if (config.with_threads) {
				fprintf(sysc, "-mt -lpthread ");
			}
			if (config.debug_compile) {
				fprintf(sysc, "-g%s\n", XMEMDEBUG);
			} else {
				fprintf(sysc, "-xO5\n");
			}
			fprintf(sysc, "RELOC   = -G\n");
			fprintf(sysc, "SHARED  = -G\n");
			break ;

			case os_dec:
			if (config.with_threads) {
				printf("threads not supported on this platform\n");
			}
			if (config.debug_compile) {
				fprintf(sysc, "-g%s\n", XMEMDEBUG);
			} else {
				fprintf(sysc, "-O\n");
			}
			fprintf(sysc, "RELOC   =\n");
			fprintf(sysc, "SHARED  = -shared -expect_unresolved \"*\"\n");
			break ;

			case os_irix:
			case os_aix:
			case os_bsd:
			if (config.with_threads) {
				printf("threads not supported on this platform\n");
			}
			if (config.debug_compile) {
				fprintf(sysc, "-g%s\n", XMEMDEBUG);
			} else {
				fprintf(sysc, "-O\n");
			}
			fprintf(sysc, "RELOC   =\n");
			fprintf(sysc, "SHARED  =\n");
			break ;

			case os_cygwin:
			case os_linux:
			/* cc with Linux? Why not! */
			if (config.with_threads) {
				fprintf(sysc, "-pthread ");
			}
			if (config.debug_compile) {
				fprintf(sysc, "-g%s\n", XMEMDEBUG);
			} else {
				fprintf(sysc, "-O3\n");
			}
			fprintf(sysc, "RELOC   = -fpic\n");
			fprintf(sysc, "SHARED  = -shared\n");
			break ;

			case os_darwin:
			/* Darwin uses gcc but calls it 'cc' */
			if (config.with_threads) {
				fprintf(sysc, "-pthread ");
			}
			if (config.debug_compile) {
				fprintf(sysc, "-g%s\n", XMEMDEBUG);
			} else {
				fprintf(sysc, "-O3\n");
			}
			fprintf(sysc, "RELOC   = -fPIC\n");
			fprintf(sysc, "SHARED  = -shared\n");
			break ;


			default:
			printf("error: unsupported OS\n");
			exit(-1) ;
		}
		break ;

		case COMPILER_GCC:	
        fprintf(sysc, "CC      = gcc\n");
		fprintf(sysc, "CFLAGS  = ");

        if (config.xmemory_mode == 0) {
            fprintf(sysc, "-DXMEMORY_MODE=%d ", config.xmemory_mode) ;
            fprintf(sysc, "-DXMEMORY_MAXPTRS=1 ") ;
        } else if (config.xmemory_mode == 1) {
            fprintf(sysc, "-DXMEMORY_MODE=%d ", config.xmemory_mode) ;
            fprintf(sysc, "-DXMEMORY_MAXPTRS=1 ") ;
        } else if (config.xmemory_mode == 2) {
            fprintf(sysc, "-DXMEMORY_MODE=%d ", config.xmemory_mode) ;
            if (config.max_ptrs > 0) {
                fprintf(sysc, "-DXMEMORY_MAXPTRS=%d ", config.max_ptrs) ;
            }
        } else if (config.xmemory_mode < 0) {
            if (config.max_ptrs > 0) {
                fprintf(sysc, "-DXMEMORY_MAXPTRS=%d ", config.max_ptrs) ;
            }
        }
		if (config.with_threads) {
			fprintf(sysc, "-pthread ");
		}
		if (config.lint_compile) {
			fprintf(sysc, " -Wall -pedantic ");
		}
		if (config.debug_compile) {
            fprintf(sysc, "-g%s\n", XMEMDEBUG);
		} else {
			fprintf(sysc, "-O3\n");
		}
		if (config.local_os != os_cygwin)
                  fprintf(sysc, "RELOC   = -fPIC -DPIC\n");
		fprintf(sysc, "SHARED  = -shared\n");
		break ;

		default:
		printf("error in compiler option switch: aborting\n");
		exit(-1);
		break ;
	}

	if (config.debug_compile) {
		printf("                                in debug mode\n");
	} else {
		printf("                                all optimizations on\n");
	}
	if (config.lint_compile) {
		printf("                                with all warnings\n");
	}

	/* LFLAGS */
    fprintf(sysc, "LFLAGS  = ");
	switch (config.local_os) {

		case os_hp10:
		case os_hp11:
		fprintf(sysc, "-Wl,+vnocompatwarnings ");
		break ;

		default:
		break ;
	}
    fprintf(sysc, "\n");

	/* DYNSUF */
	switch (config.local_os) {
		case os_hp08:
		case os_hp09:
		case os_hp10:
		case os_hp11:
		fprintf(sysc, "DYNSUF  = sl\n");
		break ;

		case os_darwin:
		fprintf(sysc, "DYNSUF  = so\n");
		break ;
				

		default:
		fprintf(sysc, "DYNSUF  = so\n");
		break ;
	}

	/* STRIP */
	if (config.debug_compile) {
		fprintf(sysc, "STRIP   = true\n");
	} else {
		fprintf(sysc, "STRIP   = strip\n");
	}

	/* TARGETS */
	fprintf(sysc, "TARGETS =");
    printf("static library................. %s\n",
            config.lib_static ? "yes" : "no");
	if (config.lib_static) {
		fprintf(sysc, " static");
	}
    printf("shared library................. %s\n",
            config.lib_dynamic ? "yes" : "no");
	if (config.lib_dynamic) {
		fprintf(sysc, " dynamic");
	}
	fprintf(sysc,"\n");

	/* PREFIX */
	fprintf(sysc, "prefix  = %s\n",
            config.prefix ? config.prefix : PREFIX_DEFAULT);
	printf("setting installation prefix as: [%s]\n",
			config.prefix ? config.prefix : PREFIX_DEFAULT);

	fclose(sysc);
}


void make_config_h()
{
	FILE * out ;

	out = fopen(HEADER_FILE, "w");
	if (out==NULL) {
		printf("cannot create header file %s: aborting", HEADER_FILE);
		return ;
	}

	fprintf(out,
	"/* This file automatically generated */\n"
	"#ifndef _CONFIG_H_\n"
	"#define _CONFIG_H_\n"
	"\n"
	"%s",
    config.big_endian ? "#define WORDS_BIGENDIAN 1\n" : "\n"
	);

	/* Generate SIZEOF macros for basic types */
	printf("detecting basic size types\n");
	fprintf(out,
	"#define SIZEOF_CHAR     %d\n"
	"#define SIZEOF_SHORT    %d\n"
	"#define SIZEOF_INT      %d\n"
	"#define SIZEOF_LONG     %d\n"
	"#define SIZEOF_FLOAT    %d\n"
	"#define SIZEOF_DOUBLE   %d\n",
			sizeof(char),
			sizeof(short),
			sizeof(int),
			sizeof(long),
			sizeof(float),
			sizeof(double));
	printf(
    "sizeof(char)................... %d\n"
    "sizeof(short).................. %d\n"
    "sizeof(int).................... %d\n"
    "sizeof(long)................... %d\n"
    "sizeof(float).................. %d\n"
    "sizeof(double)................. %d\n",
			sizeof(char),
			sizeof(short),
			sizeof(int),
			sizeof(long),
			sizeof(float),
			sizeof(double));

	/* Do not output CHAR_BIT on AIX */
	if (config.local_os!=os_aix) {
	fprintf(out,
	"\n"
	"#ifndef CHAR_BIT\n"
	"#define CHAR_BIT\t%d\n"
	"#endif\n"
	"\n\n",
    config.bits_per_byte);
	}

    if (config.arch_x86) {
        fprintf(out, "#define CPU_X86         %d\n", config.cpu_x86);
    }

	if (config.with_threads) {
		fprintf(out, "#define HAS_PTHREADS    1\n");
	}
    fprintf(out, "\n");
	switch (config.local_os) {
		case os_hp08:
		case os_hp09:
		case os_hp10:
		case os_hp11:
		fprintf(out, "#define OS_HPUX         1\n");
		break ;

		case os_linux:
		fprintf(out, "#define OS_LINUX        1\n");
		break ;

		case os_cygwin:
		fprintf(out, "#define OS_CYGWIN        1\n");
		break ;

		case os_irix:
		fprintf(out, "#define OS_IRIX          1\n");
		break ;

		case os_aix:
		fprintf(out, "#define OS_AIX          1\n");
		break ;

		case os_dec:
		fprintf(out, "#define OS_DEC          1\n");
		break ;

		case os_solaris:
		fprintf(out, "#define OS_SOLARIS      1\n");
		break ;

		case os_bsd:
		fprintf(out, "#define OS_BSD          1\n");
		break ;

		case os_darwin:
		fprintf(out, "#define OS_DARWIN       1\n");
		break ;

		default:
		fprintf(out, "#define OS_UNKNOWN      1\n");
		break ;
	}
	fprintf(out, "#endif\n");
	fclose(out);
	printf("done\n");
	return ;

}

void help()
{
	printf(
			"\n\n"
			"***** qfits configure help\n"
			"Use: configure [options]\n"
			"\n"
			"options are:\n"
			"\t--debug            Compile modules in debug mode\n"
			"\t--help             Get this help\n"
			"\n"
			"\t--with-cc          Force compilation with local cc\n"
			"\t--with-gcc         Force compilation with gcc\n"
			"\n"
			"\t--enable-static    Compile static library (default)\n"
			"\t--enable-shared    Compile shared library\n"
			"\t--disable-shared   Do not compile shared library (default)\n"
			"\t--disable-static   Do not compile static library\n"
			"\n"
			"\t--xmemory-mode=M   M=0 switches off the internal xmemory\n"
            "\t                   handling, M=1 exits the program\n"
            "\t                   whenever a memory allocation fails,\n"
            "\t                   M=2 switches on the internal xmemory\n"
            "\t                   handling.\n" 
			"\n"
			"\t--max-ptrs=MAXPTRS Set MAXPTRS as the maximum number of\n"
            "\t                   pointers allowed\n"
			"\n"
			"\t--prefix=PATH      Install in PATH (must be absolute)\n"
			"\t--mt               Compile with multithreading support\n"
			"\n"
			"options specific to compilation with gcc (for developers):\n"
			"\t--lint             Compile with -Wall\n"
			"\n"
            "If your platform is not or incorrectly recognized, you\n"
            "can force a given configuration with this option:\n"
            "\n"
            "\t--os=NAME        Where NAME is one of the following:\n"
            "\n"
            "\tlinux      - Linux systems (any processor type)\n"
            "\tcygwin     - Cygwin (UNIX environment for Windows)\n"
            "\thp08       - HPUX version 8.x\n"
            "\thp09       - HPUX version 9.x\n"
            "\thp10       - HPUX version 10.x\n"
            "\thp11       - HPUX version 11.x\n"
            "\tirix       - SGI IRIX64\n"
            "\taix        - IBM AIX (any version)\n"
            "\tdec        - Dec OSF/1 or Tru64 Unix\n"
            "\tsolaris    - Sun Solaris >=2.5\n"
            "\tbsd        - BSD compatible Unix\n"
            "\tdarwin     - Darwin (BSD compatible on Mac)\n"
            "\n"
	);
}

int main(argc, argv)
	int argc ;
	char * argv[];
{
    char        sysname[MAXSTRSZ];
    char    *   sval ;
	int	        i ;

	config.debug_compile = 0;
	config.lint_compile  = 0 ;
	config.compiler      = COMPILER_AUTO ;
	config.with_threads  = 0 ;
	config.lib_static	 = 1 ;
	config.lib_dynamic   = 0 ;
	config.prefix		 = NULL ;
	config.max_ptrs		 = -1 ;
	config.xmemory_mode  = -1 ;

    memset(config.sysname, 0, MAXSTRSZ);
    memset(config.release, 0, MAXSTRSZ);
    memset(config.machine, 0, MAXSTRSZ);

	for (i=0 ; i<argc ; i++) {
		if (!strcmp(argv[i], "--help")) {
			help() ;
			return 1 ;
		} else if (!strcmp(argv[i], "--debug")) {
			config.debug_compile = 1 ;
		} else if (!strcmp(argv[i], "--with-cc")) {
			config.compiler = COMPILER_CC ;
		} else if (!strcmp(argv[i], "--with-gcc")) {
			config.compiler = COMPILER_GCC ;
		} else if (!strcmp(argv[i], "--lint")) {
			config.lint_compile = 1 ;
		} else if (!strcmp(argv[i], "--mt")) {
			config.with_threads=1 ;
		} else if (!strcmp(argv[i], "--enable-shared")) {
			config.lib_dynamic = 1;
		} else if (!strcmp(argv[i], "--disable-shared")) {
			config.lib_dynamic = 0;
		} else if (!strcmp(argv[i], "--enable-static")) {
			config.lib_static = 1;
		} else if (!strcmp(argv[i], "--disable-static")) {
			config.lib_static = 0;
		} else if (!strncmp(argv[i], "--prefix=", 9)) {
			config.prefix = strchr(argv[i], '=') + 1 ;
		} else if (!strncmp(argv[i], "--max-ptrs=", 11)) {
			sval = strchr(argv[i], '=') + 1 ;
			config.max_ptrs = (int)atoi(sval) ;
        } else if (!strncmp(argv[i], "--xmemory-mode=", 15)) {
			sval = strchr(argv[i], '=') + 1 ;
			config.xmemory_mode = (int)atoi(sval) ;
        } else if (!strncmp(argv[i], "--os=", 5)) {
            strcpy(sysname, argv[i]+5);
            if (!strcmp(sysname, "linux")) {
                strcpy(config.sysname, "Linux");
            } else if (!strcmp(sysname, "cygwin")) {
                strcpy(config.sysname, "CYGWIN");
            } else if (!strcmp(sysname, "hp08")) {
                strcpy(config.sysname, "HPUX");
                strcpy(config.release, "8.x");
            } else if (!strcmp(sysname, "hp09")) {
                strcpy(config.sysname, "HPUX");
                strcpy(config.release, "9.x");
            } else if (!strcmp(sysname, "hp10")) {
                strcpy(config.sysname, "HPUX");
                strcpy(config.release, "10.x");
            } else if (!strcmp(sysname, "hp11")) {
                strcpy(config.sysname, "HPUX");
                strcpy(config.release, "11.x");
            } else if (!strcmp(sysname, "irix")) {
                strcpy(config.sysname, "IRIX64");
            } else if (!strcmp(sysname, "aix")) {
                strcpy(config.sysname, "AIX");
            } else if (!strcmp(sysname, "dec")) {
                strcpy(config.sysname, "Dec OSF/1 or Tru64");
            } else if (!strcmp(sysname, "solaris")) {
                strcpy(config.sysname, "Solaris");
                strcpy(config.release, ">= 2.5");
            } else if (!strcmp(sysname, "bsd")) {
                strcpy(config.sysname, "BSD compatible");
            } else if (!strcmp(sysname, "darwin")) {
                strcpy(config.sysname, "Darwin");
            } else {
                printf("unsupported OS: %s\n", sysname);
                return -1 ;
            }
        }
	}

    detect_config();
	make_config_make();
    make_config_h();
	return 0 ;
}



