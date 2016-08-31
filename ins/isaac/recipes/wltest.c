/*----------------------------------------------------------------------------*/
/**
   @file    wltest.c
   @author  Y. Jung
   @date    September 2003
   @version $Revision: 1.18 $
   @brief   ISAAC wavelength calibration testing programm
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: wltest.c,v 1.18 2003/11/18 18:01:36 yjung Exp $
	$Author: yjung $
	$Date: 2003/11/18 18:01:36 $
	$Revision: 1.18 $
*/

/*-----------------------------------------------------------------------------
               			Includes
 -----------------------------------------------------------------------------*/

#include "eclipse.h"
#include "spectral_lines.h"
#include "isaacp_lib.h"

/*-----------------------------------------------------------------------------
   				Defines
 -----------------------------------------------------------------------------*/

#define DISCARD_LE_BORDER		-1
#define DISCARD_RI_BORDER		-1
#define DISCARD_LO_BORDER		80
#define DISCARD_HI_BORDER		80

/*-----------------------------------------------------------------------------
                            Function prototypes
 -----------------------------------------------------------------------------*/

static int isaac_wavecal_engine(char *, int, int, int, int, int) ;
static char * identify_spectral_table(char *) ;
static instrument_t INSID ;

/*-----------------------------------------------------------------------------
                                Main 
 -----------------------------------------------------------------------------*/
int isaac_wltest_main(void * dict)
{
	dictionary	*	d ;
	int			discard_lo, discard_hi, discard_le, discard_ri ;
	int			remove_thermal ;
	char            	argname[10] ;
	char		*	name_i ;
	int     	        nfiles ;
	char        	*   	tmp_string ;
	int            	 	errors ;
	int             	i ;
	
	d = (dictionary*)dict ;
	/* Get options */
	remove_thermal = dictionary_getint(d, "arg.thermal", 0);
	/* Get image border definition */
	tmp_string = dictionary_get(d, "arg.border", NULL);
	if (tmp_string != NULL) {
		if (sscanf(tmp_string, "%d %d", &discard_lo, &discard_hi)!=2) {
			e_error("in -b/--border: expected two values");
		}
	} else {
		discard_lo = DISCARD_LO_BORDER ;
		discard_hi = DISCARD_HI_BORDER ;
	}
	/* Get zero spectrum definition */
	tmp_string = dictionary_get(d, "arg.zero", NULL);
	if (tmp_string != NULL) {
		if (sscanf(tmp_string, "%d %d", &discard_le, &discard_ri)!=2) {
			e_error("in -z/--zero: expected two values");
		}
	} else {
		discard_le = DISCARD_LE_BORDER ;
		discard_ri = DISCARD_RI_BORDER ;
	}
    
	/* Get input/output file names */
	nfiles = dictionary_getint(d, "arg.n", -1) ;
       	if (nfiles<0) {
	       	e_error("missing input file name(s): aborting");
		return -1 ;
	}

	INSID = pfits_identify_insstr("isaac");
    
	/* Loop on input file names */
	errors = 0 ;
	for (i=1 ; i<nfiles ; i++) {
		sprintf(argname, "arg.%d", i);
	       	name_i = dictionary_get(d, argname, NULL) ;
		/* Call the engine */
		errors += isaac_wavecal_engine(name_i,
				discard_lo,
				discard_hi,
				discard_le,
				discard_ri,
				remove_thermal) ;
	}
	return errors ;
}

/*-----------------------------------------------------------------------------
				Function code
 -----------------------------------------------------------------------------*/

static int isaac_wavecal_engine(
        char     *    name_i,
        int         discard_lo,
        int         discard_hi,
        int         discard_le,
        int         discard_ri,
        int         remove_thermal)
{
    double              *   phdisprel ;
    computed_disprel    *   disprel ;
    int                     npix ;
    image_t             *   image_in ;
    char                *   sval ;
    char                *   table_name ;
    int                     order;
    double              *   wlarray ;
    double                  slit_width = 0;
    int                     i ;
    
    /* Compute the slit_width */
    slit_width = isaac_get_slitwidth(name_i) ;
    if (slit_width == -1) {
        e_error("cannot get the slit width") ;
        return -1 ;
    }

    /* Get the wavelength order */
    if ((order = isaac_find_order(name_i)) == -1) {
        e_warning("Cannot find order, defaulting to 1") ;
        order = 1 ;
    }

    /* Gather number of pixels in the input spectrum */
    if ((sval = qfits_query_hdr(name_i, "NAXIS1")) == NULL) {
        e_error("cannot read NAXIS1 in input file") ;
        return -1 ;
    }
    npix = atoi(sval);

    /* Physical model of the instrument. */
    if ((phdisprel = isaac_get_disprel_estimate(name_i, 3)) == NULL) {
        e_error("getting estimate for dispersion relation: aborting");
        return -1 ;
    }

    table_name = identify_spectral_table(name_i);
    e_comment(0, "using spectral table: [%s]\n", table_name);


    if (remove_thermal == 0) remove_thermal = isaac_has_thermal(name_i) > 0;

    /* Load input image */
    image_in = image_load(name_i) ;
    if (image_in == NULL) {
        e_error("in loading image [%s]: aborting", name_i) ;
        return -1 ;
    }

    /* Compute dispersion relation */
    disprel = spectro_compute_disprel(image_in, discard_lo, discard_hi, 
                    discard_le, discard_ri, remove_thermal, table_name,
                    slit_width, order, phdisprel) ;

    image_del(image_in);
    free(phdisprel);
    if (disprel==NULL) {
        e_error("computing dispersion relation: aborting");
        return -1 ;
    }

    /* Print out results on stdout */
    e_comment(0, "Cross-correlation quality: %g\n", disprel->cc) ;
    e_comment(0, "dispersion relation:\n");
    e_comment(0, "lambda = %g + %g * pix + %g * pix^2 + %g * pix^3\n", 
            disprel->poly[0], disprel->poly[1], disprel->poly[2], 
            disprel->poly[3]);
    
    if (disprel->poly != NULL) free(disprel->poly) ;
    free(disprel);

    return 0 ;
}

static char * identify_spectral_table(char * filename)
{
    char  *    name ;
    int        xenon, argon ;

    xenon = isaac_is_xenon_lamp_active(filename) ; /*  */
    argon = isaac_is_argon_lamp_active(filename) ;

    if ((argon==-1) || (xenon==-1)) {
        e_error("cannot determine lamp status: using OH line table");
        return "oh" ;
    }

    if (argon && xenon) name = "Xe+Ar";
    else if (argon && !xenon) name = "Ar";
    else if (!argon && xenon) name = "Xe";
    else name = "oh";
    return name ;
}

