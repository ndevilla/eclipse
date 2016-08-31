/*-----------------------------------------------------------------------------
   
   File name    :   conicap.c
   Author       :	Y. Jung
   Created on   :	June 2001
   Description  :	CONICA recipe launcher
 -----------------------------------------------------------------------------*/

/*
   $Id: conicap.c,v 1.21 2003/04/25 09:13:43 yjung Exp $
   $Author: yjung $
   $Date: 2003/04/25 09:13:43 $
   $Revision: 1.21 $
 */

/*-----------------------------------------------------------------------------
                                Includes
 -----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Get eclipse functionalities */
#include "eclipse.h"
/* Get command-line handling routines */
#include "cmdline.h"
/* Get recipe definitions */
#include "recipes.h"
/* Get man page for this executable */
#include "conicap_man.h"

/* A CONICA engine has a fixed prototype */
typedef int (*engine)(void*);

/* Registration table: associate strings to recipes */
struct {
	const char	 *	name ;	/* Recipe name				*/
	const char	 *	desc ;	/* One-liner description	*/
	const char	 *	version ; /* Recipe version         */
	const char	 *	date ;  /* Recipe modification date */
	engine			func ;	/* Main recipe function		*/
	cmdline_spec *	cmd ;	/* Command-line specs		*/
	const char	 *	man_page ; /* Complete man page     */

} engine_table[] = {

    {	"check-focus",
		"Check focus recipe",
		conica_checkfocus_version,
		conica_checkfocus_date,
		conica_checkfocus_main,
		conica_checkfocus_cmd,
		conica_checkfocus_man },

	{	"dark",
		"Dark recipe",
		conica_dark_version,
		conica_dark_date,
		conica_dark_main,
		conica_dark_cmd,
		conica_dark_man },

	{	"detlin",
		"Detector linearity",
		conica_detlin_version,
		conica_detlin_date,
		conica_detlin_main,
		conica_detlin_cmd,
		conica_detlin_man },

	{	"lampflat",
		"Lamp flat-field processing",
		conica_lampflat_version,
		conica_lampflat_date,
		conica_lampflat_main,
		conica_lampflat_cmd,
		conica_lampflat_man },

	{	"qc-strehl",
		"Strehl computation for Quality Control",
		conica_qcstrehl_version,
		conica_qcstrehl_date,
		conica_qcstrehl_main,
		conica_qcstrehl_cmd,
		conica_qcstrehl_man },

	{	"slitpos",
		"Slit position analysis",
		conica_slitpos_version,
		conica_slitpos_date,
		conica_slitpos_main,
		conica_slitpos_cmd,
		conica_slitpos_man },

	{	"twflat",
		"Twilight flat-field processing",
		conica_twflat_version,
		conica_twflat_date,
		conica_twflat_main,
		conica_twflat_cmd,
		conica_twflat_man },

	{	"zpoint",
		"Zero point recipe",
		conica_zpoint_version,
		conica_zpoint_date,
		conica_zpoint_main,
		conica_zpoint_cmd,
		conica_zpoint_man },

	/* The following NULL definition ensures end of iteration */
	{NULL, NULL, NULL, NULL, NULL, NULL, NULL}
} ;

/*
 * Usage for this program
 */
static void usage(void)
{
	int i ;

	hello_world("conicap", "CONICA pipeline");
	printf(
"------------------------------------------------------------------------\n"
"\n"
"use: conicap man     [recipe]           get a recipe documentation\n"
"use: conicap version [recipe]           get a recipe version number\n"
"use: conicap recipe  in [parameters]    launch a recipe\n"
"use: conicap manual                     generate full documentation\n"
"\n"
"Registered recipes are:\n"
"------------------------------------------------------------------------\n");
	i=0 ;
	while (engine_table[i].name!=NULL) {
		printf("%15s -- %s\n",
				engine_table[i].name,
				engine_table[i].desc);
		i++ ;
	}
	printf(
"------------------------------------------------------------------------\n");
	return ;
}

/*
 * Help system for all recipes
 */

static void help(char * what)
{
	int i ;
	int found ;

	i=0 ;
	found=0 ;
	while (engine_table[i].name!=NULL) {
		if (!strcmp(what, engine_table[i].name)) {
			printf(
"------------------------------------------------------------------------\n"
" Parameters for %s version %s\n"
"------------------------------------------------------------------------\n"
"\n",
			engine_table[i].name,
			rcs_value((char*)engine_table[i].version));
			if (engine_table[i].cmd==NULL) {
				printf("No parameter for this command\n");
			} else {
				cmdline_dump(engine_table[i].cmd);
			}
			printf("\n\n");
			found++ ;
			break ;
		}
		i++ ;
	}
	if (!found) {
		e_error("cannot find anything about [%s]", what);
	}
	return ;
}

/*
 * Versioning for all recipes
 */

static void version(char * what)
{
	int	i ;
	int found ;

	if (what==NULL) {
		/* Get all version numbers */
		printf(
"------------------------------------------------------------------------\n");
		printf("eclipse version: %s\n", get_eclipse_version());
		printf(
"------------------------------------------------------------------------\n");
		i=0 ;
		while (engine_table[i].name!=NULL) {
			printf("%15s -- %5s ",
					engine_table[i].name,
					rcs_value((char*)engine_table[i].version));
			printf("(%s)\n", rcs_value((char*)engine_table[i].date));
			i++;
		}
		printf(
"------------------------------------------------------------------------\n");
	} else {
		/* Find relevant subject */

		if (!strcmp(what, "eclipse")) {
			printf("eclipse version: %s\n", get_eclipse_version());
			return ;
		}
		i=0 ;
		found=0 ;
		while (engine_table[i].name!=NULL) {
			if (!strcmp(what, engine_table[i].name)) {
				printf("%15s -- %s\n",
						engine_table[i].name,
						rcs_value((char*)engine_table[i].version));
				found++ ;
				break ;
			}
			i++;
		}
		if (!found) {
			e_error("cannot find anything about [%s]", what);
		}
	}
	return ;
}

/*
 * Man pages for all recipes
 */

static void print_manpage(char * what, FILE * fp, char * format)
{
	int	i ;
	int	found ;

	if (what==NULL) {
		/* Give more help */
		printf( "%s", conicap_man);
		return ;
	}
	i=0 ;
	found=0 ;
    while (engine_table[i].name!=NULL) {
        if (!strcmp(what, engine_table[i].name)) {
            manpage_dump((char*)engine_table[i].name,
                         (char*)engine_table[i].man_page,
                         (char*)engine_table[i].version,
                         (char*)engine_table[i].date,
                         fp,
                         format);
            found++ ;
            break ;
        }
        i++;
    }
    if (!found) {
        e_error("cannot find anything about [%s]", what);
    }
    return ;
}


static int generate_manpages(char * format)
{
    char        filename[FILENAMESZ];
    FILE    *   fp ;
    FILE    *   indexhtml ;
    int         i ;

    if (format==NULL) return -1 ;

    /* Identify input format */
    if (strcmp(format, "man") && strcmp(format, "html")) {
        printf("unknown output format for man pages: %s\n", format);
        return -1 ;
    }
    /* Create output directory */
    printf("creating output directory '%s'\n", format);
    if (mkdir(format, 0777)==-1) {
        e_error("cannot create output directory '%s'", format) ;
        return -1 ;
    }

    indexhtml=NULL ;
    if (!strcmp(format, "html")) {
        indexhtml = fopen("html/index.html", "w");
        if (indexhtml==NULL) {
            e_error("cannot create html/index.html");
            return -1 ;
        }
        /* Dump header */
        fprintf(indexhtml,
                "<html>\n"
                "<title>conicap manual</title>\n"
                "<body>\n"
                "\n"
                "\n"
                "<hr size=\"4\">\n"
                "<h2>conicapp manual</h2>\n"
                "<hr size=\"4\">\n"
                "\n"
                "<p>General help about the conicap command:\n"
                "<a href=\"conicap.html\">conicap command help</a>\n"
                "</p>\n"
                "\n"
                "<p>\n"
                "The following recipes are supported:\n"
                "</p>\n"
                "\n"
                "<ul>\n");
    }

    /* Dump recipe man pages */
    printf("creating man pages...\n");
    i=0 ;
    while (engine_table[i].name!=NULL) {
        sprintf(filename, "%s/%s.%s", format, engine_table[i].name, format);
        printf("-> %s...\n", filename);
        fp = fopen(filename, "w");
        if (fp==NULL) {
            e_error("cannot create file %s", filename);
            return -1 ;
        }
        print_manpage((char*)engine_table[i].name, fp, format);
        fclose(fp);

        /* Add one more entry to index.html */
        if (indexhtml!=NULL) {
            fprintf(indexhtml,
                    "<li><a href=\"%s.html\">%s</a></li>\n",
                    engine_table[i].name,
                    engine_table[i].name);
        }
        i++ ;
    }

    /* Dump conicap man page */
    sprintf(filename, "%s/conicap.%s", format, format);
    printf("-> %s...\n", filename);
    fp = fopen(filename, "w");
    if (fp==NULL) {
        e_error("cannot create file %s", filename);
        return -1 ;
    }
    manpage_dump("conicap",
                  (char*)conicap_man,
                  NULL,
                  rcs_value((char*)conicap_man_date),
                  fp,
                  format);
    fclose(fp);

    /* Close index.html */
    if (indexhtml!=NULL) {
        printf("-> html/index.html...\n");
        fprintf(indexhtml,
                "</ul>\n"
                "<hr size=\"4\">\n"
                "</body>\n"
                "</html>\n");
        fclose(indexhtml);
    }
    printf("done\n");
    return 0 ;
}

/*
 * Generic engine caller
 */
static int call_engine(char * name, int argc, char ** argv)
{
	int	i ;
	int	found ;
	int	status ;
	dictionary * d ;

	i=0 ;
	found = -1 ;
	/* Look for requested name in the table */
	while (engine_table[i].name != NULL) {
		if (!strcmp(engine_table[i].name, name)) {
			/* Found the requested recipe */
			found = i ;
			break ;
		}
		i++ ;
	}

	if (found<0) {
		/* No corresponding name was found in the table */
		e_error("no recipe called [%s]", name);
		return -1 ;
	}

	/* If no further option was passed, print out help message */
	if (argc==1) {
		help(name);
		return 1 ;
	}

	/* Correct options were passed, launch the recipe engine */
	d = cmdline_parse(argc, argv, engine_table[found].cmd);
	if (d==NULL) {
		return -1 ;
	}
	/* Execute engine */
	status = engine_table[found].func(d);
	/* Discard dictionary and exit */
	dictionary_del(d);
	return status ;
}

/*----------------------------------------------------------------------------
                            Main code
 ---------------------------------------------------------------------------*/

int main(int argc, char *argv[])
{
	int	status ;

	/* No argument: print out usage */
	if (argc<2) {
		usage();
		return 1 ;
	}

	status=0 ;

	/* See if a special command was given */
	if (!strcmp(argv[1], "version")	||
			   !strcmp(argv[1], "--version")) {
		if (argc>2)
			version(argv[2]);
		else
			version(NULL);
	} else if (!strcmp(argv[1], "man")) {
		if (argc>2) {
			status = 1 ;
			print_manpage(argv[2], stdout, "man");
		} else {
			status = 1 ;
			print_manpage(NULL, stdout, "man");
		}
	} else if (!strcmp(argv[1], "license")) {
		eclipse_display_license();
		status = 1;
    } else if (!strcmp(argv[1], "manual")) {
        if (argc>2) {
            status = generate_manpages(argv[2]);
        } else {
            status = generate_manpages("man");
        }
    } else {
		/* Initialize eclipse environment */
		eclipse_init();
		status = call_engine(argv[1], argc-1, argv+1);
	}
	if (debug_active()) xmemory_status() ;
	return status ;
}
