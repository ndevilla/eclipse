/*----------------------------------------------------------------------------*/
/**
   @file    extract.c
   @author  Nicolas Devillard
   @date    October 13, 1995
   @version	$Revision: 1.23 $
   @brief   Extract data from a cube
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: extract.c,v 1.23 2002/11/19 15:27:19 yjung Exp $
	$Author: yjung $
	$Date: 2002/11/19 15:27:19 $
	$Revision: 1.23 $
 */

/*-----------------------------------------------------------------------------
   								Includes
 -----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "eclipse.h"

/*-----------------------------------------------------------------------------
   								Defines
 -----------------------------------------------------------------------------*/

#define OPT_PLANE		1001
#define OPT_CUBE		1002
#define	OPT_QUADRANT	1003
#define OPT_PATTERN		1004
#define OPT_LIST		1005
#define OPT_RECTANGLE	1006

#define OPT_BEGIN		2001
#define OPT_END			2002
#define OPT_EXTSTRING	2003
#define OPT_LISTNAME	2004
#define OPT_STEP		2005

/*-----------------------------------------------------------------------------
   								New types
 -----------------------------------------------------------------------------*/

typedef enum extraction_mode {
	EXT_PLANE,
	EXT_PATTERN,
	EXT_CUBE,
	EXT_QUAD,
	EXT_LIST,
	EXT_RECTANGLE,
	EXT_UNDEFINED
} extraction_mode ;

static int cube_extract(
	char			*	name_i,
	char			*	name_o,
	extraction_mode		emode,
	int					begin,
	int					end,
	char			*	listname,
	char			*	extstring,
	int					step
) ;

/*-----------------------------------------------------------------------------
   								Function prototypes
 -----------------------------------------------------------------------------*/

static char prog_desc[] = "extract data from a cube" ;
static void usage(char * pname) ;

/*-----------------------------------------------------------------------------
  								    Main
 -----------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
	char		    name_i[FILENAMESZ+1],
				    name_o[FILENAMESZ+1],
				    listname[FILENAMESZ+1] ;
	char		    extstring[MAX_IMAGE_NUMBER] ;
	int			    begin, 
                    end ;
	int			    step ;
	extraction_mode emode ;
	int			    c ;

	if (argc<2) usage(argv[0]) ;

    /* Initialize */
	name_i[0] = name_o[0] = listname[0] = 0 ; 
	begin = end = -1 ;
	emode = EXT_UNDEFINED ;
	memset(extstring, 0, MAX_IMAGE_NUMBER);
	step = 1 ;

    while (1) {
        int     option_index = 0 ;
        static struct option long_options[] =
        {
            {"license", 0, 0, OPT_LICENSE},
            {"help",    0, 0, OPT_HELP},
            {"version", 0, 0, OPT_VERSION},

            {"plane",     0, 0, OPT_PLANE},
            {"cube",      0, 0, OPT_CUBE},
            {"quadrant",  0, 0, OPT_QUADRANT},
            {"pattern",   0, 0, OPT_PATTERN},
            {"list",      0, 0, OPT_LIST},
            {"rectangle", 0, 0, OPT_RECTANGLE},

            {"begin", 1, 0, OPT_BEGIN},
            {"end",   1, 0, OPT_END},
            {"ext",   1, 0, OPT_EXTSTRING},
            {"name",  1, 0, OPT_LISTNAME},
            {"step",  1, 0, OPT_STEP},

            {"in",      1, 0, OPT_INPUT},
            {"out",     1, 0, OPT_OUTPUT},

            {0, 0, 0, 0}

        } ;
        c = getopt_long(argc,
                        argv,
                        "Lhm:i:o:b:e:x:f:s:",
                        long_options,
                        &option_index) ;
        if (c==-1) break ;

        switch(c) {

            /* Standard option: display license undocumented option */
            case OPT_LICENSE:
            case 'L':
                eclipse_display_license() ;
                return 0 ;
            /* Standard option : help */
            case OPT_HELP:
            case 'h':
                usage(argv[0]) ;
                break ;
            /* Standard option: version */
            case OPT_VERSION:
                print_eclipse_version() ;
                return 0 ;

            /* Local options: extraction modes  */
			case 'm':
	    		if (!strcmp(optarg, "plane"))
		    		emode = EXT_PLANE ;
		    	else if (!strcmp(optarg, "cube"))
			    	emode = EXT_CUBE ;
    			else if (!strcmp(optarg, "quad"))
	    			emode = EXT_QUAD ;
		    	else if (!strcmp(optarg, "pattern"))
			    	emode = EXT_PATTERN ;
    			else if (!strcmp(optarg, "list"))
	    			emode = EXT_LIST ;
		    	else if (!strcmp(optarg, "rect"))
			    	emode = EXT_RECTANGLE ;
    			else {
	    			e_error("unrecognized extraction mode : %s", optarg) ;
		    		return -1 ;
		    	}
			    break ;
			case OPT_PLANE:
			    emode = EXT_PLANE ;
			    break ;
			case OPT_CUBE:
    			emode = EXT_CUBE ;
	    		break ;
			case OPT_QUADRANT:
		    	emode = EXT_QUAD ;
			    break ;
			case OPT_PATTERN:
    			emode = EXT_PATTERN ;
	    		break ;
			case OPT_LIST:
		    	emode = EXT_LIST ;
			    break ;
			case OPT_RECTANGLE:
    			emode = EXT_RECTANGLE ;
	    		break ;

		    /* Local options: input and output file names */
			case OPT_INPUT:
			case 'i':
			    strncpy(name_i, optarg, FILENAMESZ) ;
    			break ;
			case OPT_OUTPUT:
			case 'o':
	    		strncpy(name_o, get_rootname(optarg), FILENAMESZ) ;
		    	break ;
    		/* Processing options : begin and end planes	*/
			case OPT_BEGIN:
			case 'b':
	    		begin = (int)atoi(optarg) ;
		    	break ;
			case OPT_END:
		    case 'e':
			    end = (int)atoi(optarg) ;
    			break ;
	    	/* Processing option : extraction string	*/
			case OPT_EXTSTRING:
			case 'x':
		    	strncpy(extstring, optarg, MAX_IMAGE_NUMBER-1) ;
			    break ;
    		/* Processing option: number of images per cycle step	*/
			case OPT_STEP:
			case 's':
	    		step = (int)atoi(optarg) ;
		    	break ;
    		/* Processing option : list file name	*/
			case OPT_LISTNAME:
			case 'f':
	    		strncpy(listname, optarg, FILENAMESZ) ;
		    	break ;
            default:
                usage(argv[0]) ;
              break ;
        }
	}

	/* Initialize eclipse environment */
	eclipse_init();

	if (name_i[0] == 0) {
		e_error("no input file name provided, use the -i option");
		return -1 ;
	}

	if (name_o[0] == 0) {
		strncpy(name_o, get_basename(get_rootname(name_i)), FILENAMESZ);
	}
	
	e_comment(1, "input     : %s", name_i);
	e_comment(1, "output    : %s", name_o);

	cube_extract(name_i, name_o, emode, begin, end, listname, extstring, step);

	if (debug_active()) xmemory_status() ;
    return 0 ;

}

static void usage(char * pname)
{
	hello_world(pname, prog_desc) ; 
	printf("use: %s -i infile [-o outfile] [mode options]\n", pname) ;
	printf("use: %s --in infile [--out outfile] [mode options]\n", pname) ;
	printf(
		"modes and associated options are:\n"
		"\n");
	printf(
		"plane extraction:\n"
		"\textract -m plane [-b <#>] [-e <#>]\n"
		"\textract --plane [--begin <#>] [--end <#>]\n"
		"\n");
	printf(
		"cube extraction:\n"
		"\textract -m cube [-b <#>] [-e <#>]\n"
		"\textract --cube [--begin <#>] [--end <#>]\n"
		"\n");
	printf(
		"quadrant extraction:\n"
		"\textract -m quad [-x '1234']\n"
		"\textract --quadrant [--ext '1234']\n"
		"\n");
	printf(
		"pattern extraction:\n"
		"\textract -m pattern [-x '011010...'] [-s <step>]\n"
		"\textract --pattern [--ext '011010...'] [--step <step>]\n"
		"\n");
	printf(
		"list extraction:\n"
		"\textract -m list [-f file]\n"
		"\textract --list [--name file]\n"
		"\n");
	printf(
		"rectangle extraction:\n"
		"\textract -m rect [-x 'llx lly urx ury']\n"
		"\textract --rectangle [--ext 'llx lly urx ury']\n"
		"\n\n");
	printf(
		"\tAlways supply an input name through -i or --in\n"
		"\tDefault output name depends on the mode. see doc\n"
		"\n\n") ;
	exit(0) ;
}

static int cube_extract(
        char			*	name_i,
    	char			*	name_o,
	    extraction_mode		emode,
        int					begin,
        int					end,
        char			*	listname,
        char			*	extstring,
        int					step)
{
    cube_t	*	in ;
	cube_t	*	ext ;
	cube_t	*	sav ;
	int			p ;
	char		cname_o[FILENAMESZ];
	char		cmt[ASCIILINESZ];
	FILE	*	listfile ;
	int			pnum ;
	int		*	ext_flags ;
	int			plen, ppos ;
	int			qnum ;
	int			llx, lly, urx, ury ;
	history	*	hs ;

	switch (emode) {
		/*
		 * ---------- Plane extraction
		 */ 
		case EXT_PLANE:
		e_comment(1, "extracting: planes");
		if (begin<0) {
			e_comment(1, "begin     : first");
		} else {
			e_comment(1, "begin     : %d", begin);
		}
		if (end<0) {
			e_comment(1, "end       : last");
		} else {
			e_comment(1, "end       : %d", end);
		}

		/* Load input cube */
		if ((in=cube_load(name_i))==NULL) {
			e_error("cannot load cube [%s]: aborting", name_i);
			return -1 ;
		}

		if (begin<0) begin=1 ;
		if (end<0)   end = in->np ;

		if ((begin<1) || (end>in->np) || (end<begin)) {
			e_error("begin/end incorrectly defined");
			cube_del(in);
			return -1 ;
		}
		/* Save planes one by one */
		for (p=begin ; p<=end ; p++) {
			sav = cube_from_image(in->plane[p-1]);
			if (sav==NULL) {
				e_error("cannot get plane %d from cube", p);
				cube_del(in);
				return -1 ;
			}
			hs = history_new() ;
			history_add(hs, "--- eclipse extract");
			history_add(hs, "input file:");
			history_add(hs, get_basename(name_i));
			history_add(hs, "this file is plane %d out of %d",p,in->np);

			sprintf(cname_o, "%s_%04d.fits", name_o, p);
			e_comment(1, "saving plane %d as [%s]", p, cname_o);
			cube_save_fits_hdrcopy_wh(sav, cname_o, name_i, hs) ;
			history_del(hs);
			cube_del(sav);
		}
		cube_del(in);
		break ;

		case EXT_CUBE:
		/*
		 * ---------- Cube extraction
		 */ 
		e_comment(1, "extracting: cube");
		if (begin<0) {
			e_comment(1, "begin     : first");
		} else {
			e_comment(1, "begin     : %d", begin);
		}
		if (end<0) {
			e_comment(1, "end       : last");
		} else {
			e_comment(1, "end       : %d", end);
		}
		/* Load input cube */
		if ((in=cube_load(name_i))==NULL) {
			e_error("cannot load cube [%s]: aborting", name_i);
			return -1 ;
		}

		if (begin<0) begin=1 ;
		if (end<0)   end = in->np ;

		if ((begin<1) || (end>in->np) || (end<begin)) {
			e_error("begin/end incorrectly defined");
			cube_del(in);
			return -1 ;
		}
		/* Reshuffle plane pointers */
		ext = cube_new(in->lx, in->ly, (end-begin+1));
		for (p=begin ; p<=end ; p++) {
			ext->plane[p-begin] = in->plane[p-1];
		}
		cube_del_shallow(in);
		/* Save result cube */
		hs = history_new() ;
		history_add(hs, "--- eclipse extract");
		history_add(hs, "input file:");
		history_add(hs, get_basename(name_i));
		history_add(hs, "planes %d to %d (incl)", begin, end);

		sprintf(cname_o, "%s.fits", name_o);
		e_comment(1, "saving cube [%d-%d] as [%s]", begin, end, cname_o);
		cube_save_fits_hdrcopy_wh(ext, cname_o, name_i, hs) ;
		history_del(hs);
		cube_del(ext);
		break ;

		case EXT_LIST:
		/*
		 * ---------- List extraction
		 */ 
		e_comment(1, "extracting: planes from a given list");
		e_comment(1, "listname  : %s", listname);
		/* Load input cube */
		if ((in=cube_load(name_i))==NULL) {
			e_error("cannot load cube [%s]: aborting", name_i);
			return -1 ;
		}
		if ((listfile=fopen(listname, "r"))==NULL) {
			e_error("cannot open list file [%s]", listname);
			cube_del(in);
			return -1 ;
		}
		ext_flags = calloc(in->np, sizeof(int));
		pnum = 0 ;
		while (fscanf(listfile, "%d", &p) != EOF) {
			ext_flags[p-1] = 1 ;
			pnum++ ;
		}
		fclose(listfile);
		ext = cube_new(in->lx, in->ly, pnum);
		hs = history_new();
		history_add(hs, "--- eclipse extract");
		history_add(hs, "input file:");
		history_add(hs, get_basename(name_i));
		pnum = 0 ;
		for (p=0 ; p<in->np ; p++) {
			if (ext_flags[p]) {
				ext->plane[pnum++] = in->plane[p] ;
				history_add(hs, "extracted plane %d out of %d",p+1,in->np);
			} else {
				image_del(in->plane[p]);
			}
			in->plane[p] = NULL ;
		}
		free(ext_flags);
		cube_del_shallow(in);
		sprintf(cname_o, "%s.fits", name_o);
		cube_save_fits_hdrcopy_wh(ext, cname_o, name_i, hs) ;
		history_del(hs);
		cube_del(ext);
		break ;

		case EXT_PATTERN:
		/*
		 * ---------- Pattern extraction
		 */ 
		e_comment(1, "extracting: cube with pattern");
		e_comment(1, "pattern   : %s", extstring);
		e_comment(1, "cyclestep : %d", step);
		/* Load input cube */
		if ((in=cube_load(name_i))==NULL) {
			e_error("cannot load cube [%s]: aborting", name_i);
			return -1 ;
		}
		/* Build extraction flag list */
		plen = strlen(extstring);
		if (plen<1) {
			e_error("invalid extraction string");
			cube_del(in);
			return -1 ;
		}
		ext_flags = calloc(in->np, sizeof(int));
		for (p=0 ; p<in->np ; p++) {
			ppos = (p/step) % plen ;
			if (extstring[ppos]=='1') {
				ext_flags[p] = 1 ;
			} else if (extstring[ppos]=='0') {
				ext_flags[p] = 0 ;
			} else {
				e_error("invalid character in pattern: [%s]", extstring);
				free(ext_flags);
				cube_del(in);
				return -1 ;
			}
		}
		for (p=0 ; p<in->np ; p++) {
			if (ext_flags[p]) {
				sav = cube_from_image(in->plane[p]);
				if (sav==NULL) {
					e_error("cannot get plane %d from cube", p+1);
					cube_del(in);
					free(ext_flags);
					return -1 ;
				}
				hs = history_new() ;
				history_add(hs, "--- eclipse extract");
				history_add(hs, "input file:");
				history_add(hs, get_basename(name_i));
				history_add(hs, "this file is plane %d out of %d",p+1,in->np);
				sprintf(cname_o, "%s_%04d.fits", name_o, p+1);
				e_comment(1, "saving plane %d as [%s]", p+1, cname_o);
				cube_save_fits_hdrcopy_wh(sav, cname_o, name_i, hs) ;
				history_del(hs);
				cube_del(sav);
			}
		}
		cube_del(in);
		free(ext_flags);
		break ;

		case EXT_QUAD:
		/*
		 * ---------- Quadrant extraction
		 */ 
		e_comment(1, "extracting: quadrants");
		e_comment(1, "quadrants : %s", extstring);
		/* Load input cube */
		if ((in=cube_load(name_i))==NULL) {
			e_error("cannot load cube [%s]: aborting", name_i);
			return -1 ;
		}
		plen = strlen(extstring);
		for (p=0 ; p<plen ; p++) {
			switch(extstring[p]) {
				case '1':
				ext = cube_getvig(in,
											 1,       		1+in->ly/2,
											 in->lx/2,  	in->ly);
				sprintf(cmt, "this is the upper left quadrant");
				qnum = 1 ;
				break ;

				case '2':
				ext = cube_getvig(in,
											 1+in->lx/2,  	1+in->ly/2,
											 in->lx,      	in->ly);
				sprintf(cmt, "this is the upper right quadrant");
				qnum = 2 ;
				break ;

				case '3':
				ext = cube_getvig(in,
											 1+in->lx/2,  	1,
											 in->lx,      	in->ly/2);
				sprintf(cmt, "this is the lower right quadrant");
				qnum = 3 ;
				break ;

				case '4':
				ext = cube_getvig(in,
											 1,         	1,
											 in->lx/2,  	in->ly/2);
				sprintf(cmt, "this is the lower left quadrant");
				qnum = 4 ;
				break ;

				default:
				e_error("unexpected quadrant ID in list [%s]", extstring);
				cube_del(in);
				return -1 ;
			}
			hs = history_new();
			history_add(hs, "--- eclipse extract");
			history_add(hs, "input file:");
			history_add(hs, get_basename(name_i));
			sprintf(cname_o, "%s_quad%d.fits", name_o, qnum);
			e_comment(1, "saving quadrant %d as [%s]", qnum, cname_o);
			cube_save_fits_hdrcopy_wh(ext, cname_o, name_i, hs) ;
			history_del(hs);
			cube_del(ext);
		}
		cube_del(in);
		break ;

		case EXT_RECTANGLE:
		/*
		 * ---------- Rectangle extraction
		 */ 
		e_comment(1, "extracting: rectangle");
		e_comment(1, "rectangle : %s", extstring);
		/* Load input cube */
		if ((in=cube_load(name_i))==NULL) {
			e_error("cannot load cube [%s]: aborting", name_i);
			return -1 ;
		}
		sscanf(extstring, "%d %d %d %d", &llx, &lly, &urx, &ury);
		ext = cube_getvig(in, llx, lly, urx, ury);
		cube_del(in);

		hs = history_new();
		history_add(hs, "--- eclipse extract");
		history_add(hs, "input file:");
		history_add(hs, get_basename(name_i));
		history_add(hs, "extraction zone: %d %d %d %d", llx, lly, urx, ury);

		sprintf(cname_o, "%s_ext.fits", name_o);
		e_comment(1, "saving extracted rectangle as [%s]", cname_o);
		cube_save_fits_hdrcopy_wh(ext, cname_o, name_i, hs) ;
		cube_del(ext);
		break ;

		case EXT_UNDEFINED:
		default:
		e_error("undefined extraction mode: aborting");
		return -1 ;
	}
	return 0 ;
}

