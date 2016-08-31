/*----------------------------------------------------------------------------
   
   File name    :   isaacp.c
   Author       :	N. Devillard
   Created on   :	January 2001
   Description  :	ISAAC recipe launcher

 ---------------------------------------------------------------------------*/

/*
   $Id: isaacp.c,v 1.66 2004/02/09 16:14:17 yjung Exp $
   $Author: yjung $
   $Date: 2004/02/09 16:14:17 $
   $Revision: 1.66 $
 */

/*----------------------------------------------------------------------------
                                Includes
 ---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

/* Get eclipse functionalities */
#include "eclipse.h"
/* Get command-line handling routines */
#include "cmdline.h"
/* Get recipe definitions */
#include "recipes.h"
/* Get man page for this executable */
#include "isaacp_man.h"

/* An ISAAC engine has a fixed prototype */
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

    {	"arc",
		"Arc recipe",
		isaac_arc_version,
		isaac_arc_date,
		isaac_arc_main,
		isaac_arc_cmd,
		isaac_arc_man},

	{	"dark",
		"Dark recipe",
		isaac_dark_version,
		isaac_dark_date,
		isaac_dark_main,
		isaac_dark_cmd,
		isaac_dark_man },

	{	"detlin",
		"LW detector linearity checks",
		isaac_detlin_version,
		isaac_detlin_date,
		isaac_detlin_main,
		isaac_detlin_cmd,
		isaac_detlin_man},

	{	"ghost",
		"Ghost correction",
		isaac_ghost_version,
		isaac_ghost_date,
		isaac_ghost_main,
		isaac_ghost_cmd,
		isaac_ghost_man},

	{	"illum",
		"Illumination frame creation",
		isaac_illum_version,
		isaac_illum_date,
		isaac_illum_main,
		isaac_illum_cmd,
		isaac_illum_man},
	
    {	"rename",
		"Renaming recipe",
		isaac_rename_version,
		isaac_rename_date,
		isaac_rename_main,
		isaac_rename_cmd,
		isaac_rename_man},

    {	"respfunc",
		"Response function recipe",
		isaac_respfunc_version,
		isaac_respfunc_date,
		isaac_respfunc_main,
		isaac_respfunc_cmd,
		isaac_respfunc_man},

	{	"skybg",
		"Sky background computation",
		isaac_skybg_version,
		isaac_skybg_date,
		isaac_skybg_main,
		isaac_skybg_cmd,
		isaac_skybg_man},
	
	{	"slitpos",
		"Slit position",
		isaac_slitpos_version,
		isaac_slitpos_date,
		isaac_slitpos_main,
		isaac_slitpos_cmd,
		isaac_slitpos_man},
	
    {	"sp_flat",
		"Flat recipe in spectro",
		isaac_sp_flat_version,
		isaac_sp_flat_date,
		isaac_sp_flat_main,
		isaac_sp_flat_cmd,
		isaac_sp_flat_man},

	{	"startrace",
		"Startrace recipe in spectro",
		isaac_startrace_version,
		isaac_startrace_date,
		isaac_startrace_main,
		isaac_startrace_cmd,
		isaac_startrace_man},

	{	"twflat",
		"Master flat-field creation from twilight flat data",
		isaac_twflat_version,
		isaac_twflat_date,
		isaac_twflat_main,
		isaac_twflat_cmd,
		isaac_twflat_man},
		
    {	"wltest",
		"Wavelength calibration testing",
		isaac_wltest_version,
		isaac_wltest_date,
		isaac_wltest_main,
		isaac_wltest_cmd,
		isaac_wltest_man},
	
    {	"wavecal",
		"Wavelength calibration",
		isaac_wavecal_version,
		isaac_wavecal_date,
		isaac_wavecal_main,
		isaac_wavecal_cmd,
		isaac_wavecal_man},
	
	{	"zpoint",
		"Night zero points",
		isaac_zpoint_version,
		isaac_zpoint_date,
		isaac_zpoint_main,
		isaac_zpoint_cmd,
		isaac_zpoint_man},
	
	/* The following NULL definition ensures end of iteration */
	{NULL, NULL, NULL, NULL, NULL, NULL, NULL}
} ;

/*
 * Usage for this program
 */
static void usage(void)
{
	int i ;

	hello_world("isaacp", "ISAAC pipeline");
	printf(
"------------------------------------------------------------------------\n"
"\n"
"use: isaacp man     [recipe]           get a recipe documentation\n"
"use: isaacp version [recipe]           get a recipe version number\n"
"use: isaacp recipe  in [parameters]    launch a recipe\n"
"use: isaacp manual                     generate full documentation\n"
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
		printf( "%s", isaacp_man);
		return ;
	}
	/* Look for relevant man page */
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
	char		filename[FILENAMESZ];
	FILE	*	fp ;
	FILE	*	indexhtml ;
	int			i ;

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
				"<title>isaacp manual</title>\n"
				"<body>\n"
				"\n"
				"\n"
				"<hr size=\"4\">\n"
				"<h2>isaacp manual</h2>\n"
				"<hr size=\"4\">\n"
				"\n"
				"<p>General help about the isaacp command:\n"
				"<a href=\"isaacp.html\">isaacp command help</a>\n"
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

	/* Dump isaacp man page */
	sprintf(filename, "%s/isaacp.%s", format, format);
	printf("-> %s...\n", filename);
	fp = fopen(filename, "w");
	if (fp==NULL) {
		e_error("cannot create file %s", filename);
		return -1 ;
	}
	manpage_dump("isaacp",
				  (char*)isaacp_man,
				  NULL,
				  rcs_value((char*)isaacp_man_date),
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
	if (!strcmp(argv[1], "version")	|| !strcmp(argv[1], "--version")) {
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
