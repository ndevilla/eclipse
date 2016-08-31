/*----------------------------------------------------------------------------*/
/**
   @file    imgen.c
   @author  Nicolas Devillard
   @date    May 13, 1996
   @version	$Revision: 1.34 $
   @brief   Image generation
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: imgen.c,v 1.34 2002/11/21 15:27:42 yjung Exp $
	$Author: yjung $
	$Date: 2002/11/21 15:27:42 $
	$Revision: 1.34 $
 */

/*-----------------------------------------------------------------------------
                                Includes
 -----------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include "eclipse.h"

/*-----------------------------------------------------------------------------
                                Define
 -----------------------------------------------------------------------------*/

#define OPT_BITSPERPIX					1001
#define OPT_REFIMAGE					1002

#define OPT_AIRY						1101
#define OPT_GAUSS						1102
#define OPT_LORENTZ						1103

#define OPT_OTF							1201
#define OPT_PSF							1202
#define OPT_M1							1203
#define OPT_M2							1204
#define OPT_W0							1205
#define OPT_DW							1206
#define OPT_PIXELSCALE					1207

#define OPT_URAND						1301
#define OPT_GRAND						1302
#define OPT_LRAND						1303

#define OPT_BDISK						1401
#define OPT_BRECT						1402
#define OPT_UNIFORM						1403
#define OPT_POLYGON						1404

#define OPT_POLY_2 						1501

#define OPT_TESTIMAGE					1601

#define OPT_JITTER_ARGS					1701

#define OPT_MANDELBROT					1801
#define OPT_MANDELBROT_LOC				1802

#define PRIMARY_3_60                    3.47
#define SECONDARY_3_60                  1.66
#define LAMBDA_0_3_60                   2.20
#define D_LAMBDA_3_60                   0.30
#define PIXSCALE_3_60                   0.05

#define PATT_UNKNOWN      				-1
#define PATT_UNIFORM					 0
#define PATT_AIRY            			 1
#define PATT_OTF             			 3
#define PATT_PSF						 4
#define PATT_GAUSSIAN       			 5
#define PATT_DISK            			 6
#define PATT_RECTANGLE       			 7
#define PATT_LORENTZ         			 8
#define PATT_POLYGON					 9

#define PATT_RANDOM_UNIFORM          	10
#define PATT_RANDOM_GAUSSIAN          	11
#define PATT_RANDOM_LORENTZIAN         	12

#define PATT_POLY2						13
#define PATT_TESTIMAGE					14
#define PATT_JITTER						15
#define PATT_MANDELBROT					16

/*-----------------------------------------------------------------------------
                                Function prototypes
 -----------------------------------------------------------------------------*/

static void usage(char * pname) ;
static char prog_desc[] = "image generation" ;

/*-----------------------------------------------------------------------------
									    Main 
 -----------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
	image_t		*	image_out;
	cube_t		*	cube_out ;
	pixelmap	*	pixelmap_out;
	char            outname[FILENAMESZ+1],
					refname[FILENAMESZ+1];
	cube_info	*	refcubeinfo;
	int             c;
	pixelvalue      min_pix,
					max_pix,
					uniform;
	double          width,
					dx,
					dy;
	double         	x_center,
					y_center,
					mean,
					sigma,
					intensity,
					dispersion;
	double          radius;
	int		        llx,
					lly,
					urx,
					ury;
	int		        sx,
					sy;
	int             mode;
	int		        i;
	int             ptype;
	char            random_type;
	int				size_flag,
					sx_flag,
					sy_flag;
	double          rf[5];
	double			coeff_poly[6] ;
	int				found_items ;

	int				jit_nframes ;
	int				jit_nobj ;
	int				jit_p_homog ;
	double			jit_ampl ;

	/* For polygon generation in binary map */
	FILE	*		polygon_file ;
	double3	*		polygon ;
	char	*		polygon_fname ;
	pixelmap	*	polymap ;

	/* These are for the OTF generation */
	double          m1_diam, m2_diam, w0, dw, pixel_scale;

	/* These are for Mandelbrot set location */
	double		*	mandel_loc ;

    /* Initialize */
	size_flag = sx_flag = sy_flag = 0;
	min_pix = max_pix = uniform = (pixelvalue) 0;
	width = 0.0;
	sx = sy = 256L;
	mode = PATT_UNKNOWN;
	strncpy(outname, "out.fits", FILENAMESZ);
	ptype = BPP_DEFAULT;
	llx = lly = urx = ury = 0L;
	radius = 0.0;
	dx = dy = 0 ;
	x_center = y_center = 0 ;
	mean = sigma = 0 ;
	intensity = dispersion = 0 ;
	polygon_fname = NULL ;
	m1_diam = m2_diam = 0 ;
	w0 = dw = 0 ;
	pixel_scale = 0 ;
	mandel_loc = NULL ;

    if (argc<2) usage(argv[0]);

	/* Initialize eclipse environment */
	eclipse_init();

	while (1) {
		int             option_index = 0;
		static struct option long_options[] =
		{
			{"license", 0, 0, OPT_LICENSE},
			{"help", 0, 0, OPT_HELP},
			{"version", 0, 0, OPT_VERSION},

			{"bitsperpix", 1, 0, OPT_BITSPERPIX},
			{"output", 1, 0, OPT_OUTPUT},
			{"ref", 1, 0, OPT_REFIMAGE},

			{"airy", 1, 0, OPT_AIRY},
			{"gauss", 1, 0, OPT_GAUSS},
			{"lorentz", 1, 0, OPT_LORENTZ},

			{"otf", 0, 0, OPT_OTF},
			{"psf", 0, 0, OPT_PSF},
			{"m1", 1, 0, OPT_M1},
			{"m2", 1, 0, OPT_M2},
			{"lambda0", 1, 0, OPT_W0},
			{"dlambda", 1, 0, OPT_DW},
			{"pixelscale", 1, 0, OPT_PIXELSCALE},

			{"urand", 1, 0, OPT_URAND},
			{"grand", 1, 0, OPT_GRAND},
			{"lrand", 1, 0, OPT_LRAND},

			{"bdisk", 1, 0, OPT_BDISK},
			{"brect", 1, 0, OPT_BRECT},
			{"uniform", 1, 0, OPT_UNIFORM},

			{"poly2", 1, 0, OPT_POLY_2},

			{"polygon", 1, 0, OPT_POLYGON},
			{"test", 0, 0, OPT_TESTIMAGE},
			{"jitter", 1, 0, OPT_JITTER_ARGS},
			{"mandel", 0, 0, OPT_MANDELBROT},
			{"mloc", 1, 0, OPT_MANDELBROT_LOC},

			{0, 0, 0, 0}

		};
		c = getopt_long(argc,
				argv,
				"a:b:d:g:hj:l:mo:p:r:t:u:x:y:LP:R:siS:",
				long_options,
				&option_index);
		if (c == -1) break;

		switch (c) {
			/* Standard option: display license undocumented option */
			case OPT_LICENSE:
			case 'L':
    			eclipse_display_license();
	    		return 0;

			/* Standard option : help */
			case OPT_HELP:
			case 'h':
		    	usage(argv[0]);
			    break;

			/* Standard option: version */
			case OPT_VERSION:
    			print_eclipse_version();
	    		return 0;

			/* Pixel depth  */
			case OPT_BITSPERPIX:
			case 'b':
		    	ptype = (int) atoi(optarg);
			    if (BYTESPERPIXEL(ptype) == 0) {
    				e_error("invalid requested pixel depth");
	    			return -1;
		    	}
			    break;

			/* output name  */
			case OPT_OUTPUT:
			case 'o':
    			strncpy(outname, optarg, FILENAMESZ);
	    		break;

			/* Size in X    */
			case 'x':
		    	sx_flag = 1;
			    sx = (int) atoi(optarg);
    			break;

			/* Size in Y   */
			case 'y':
	    		sy_flag = 1;
		    	sy = (int) atoi(optarg);
			    break;

			/* Reference image for the output size */
			case OPT_REFIMAGE:
			case 'S':
    			e_comment(0, "using reference file to set output image size");
	    		size_flag = 1;
		    	strncpy(refname, optarg, FILENAMESZ);
			    refcubeinfo = cube_getinfo(refname);
    			if (refcubeinfo == NULL) {
	    			e_error("cannot read reference file [%s]", refname);
		    		return -1;
			    }
    			sx = refcubeinfo->lx;
	    		sy = refcubeinfo->ly;
		    	free(refcubeinfo);
			    break;

			/* Airy mode    */
			case OPT_AIRY:
			case 'a':
    			mode = PATT_AIRY;
	    		found_items=sscanf(optarg,"%lg %lg %lg %lg",rf,rf+1,rf+2,rf+3);
			    if (found_items != 4) {
    				e_error("-a/--airy require 4 values enclosed in quotes") ;
	    			return -1 ;
		    	}
			    width = (double) rf[0];
    			max_pix = (pixelvalue) rf[1];
	    		dx = (double) rf[2];
		    	dy = (double) rf[3];
			    break;

			/* Gaussian mode    */
			case OPT_GAUSS:
			case 'g':
    			mode = PATT_GAUSSIAN;
	    		found_items = sscanf(optarg, "%lg %lg %lg", rf, rf+1, rf+2);
		    	if (found_items != 3) {
			    	e_error("-g/--gauss require 3 values enclosed in quotes") ;
				    return -1 ;
    			}
	    		x_center = (double) rf[0];
		    	y_center = (double) rf[1];
			    sigma = (double) rf[2];
    			break;

			/* Lorentz mode */
			case OPT_LORENTZ:
			case 'l':
    			mode = PATT_LORENTZ;
	    		found_items =
		    		sscanf(optarg, "%lg %lg %lg %lg", rf, rf+1, rf+2, rf+3);
			    if (found_items != 4) {
    				e_error("-l/--lorentz require 4 values enclosed in quotes");
	    			return -1 ;
		    	}
			    x_center = (double) rf[0];
    			y_center = (double) rf[1];
	    		intensity = (double) rf[2];
		    	dispersion = (double) rf[3];
			    break;

			/* theoretical OTF mode */
			case 'p':
    			mode = PATT_OTF;
	    		found_items =
		    		sscanf(optarg,"%lg %lg %lg %lg %lg",rf,rf+1,rf+2,rf+3,rf+4);
    			if (found_items != 5) {
	    			e_error("-p requires 5 values enclosed in quotes") ;
		    		return -1 ;
			    }
    			m1_diam = (double) rf[0];
	    		m2_diam = (double) rf[1];
		    	w0 = (double) rf[2];
			    dw = (double) rf[3];
    			pixel_scale = (double) rf[4];
	    		break;

			case OPT_OTF:           mode = PATT_OTF;                    break;
			case OPT_M1:            m1_diam = (double) atof(optarg);    break;
			case OPT_M2:            m2_diam = (double) atof(optarg);    break;
			case OPT_W0:            w0 = (double) atof(optarg);         break;
			case OPT_DW:            dw = (double) atof(optarg);         break;
			case OPT_PIXELSCALE:    pixel_scale = (double) atof(optarg); break;

			/* theoretical PSF mode */
			case 't':
    			mode = PATT_PSF;
	    		found_items =
		    		sscanf(optarg,"%lg %lg %lg %lg %lg",rf,rf+1,rf+2,rf+3,rf+4);
    			if (found_items != 5) {
	    			e_error("-t requires 5 values enclosed in quotes") ;
		    		return -1 ;
			    }
    			m1_diam = (double) rf[0];
	    		m2_diam = (double) rf[1];
		    	w0 = (double) rf[2];
			    dw = (double) rf[3];
    			pixel_scale = (double) rf[4];
	    		break;

			case OPT_PSF:           mode = PATT_PSF;                     break;

			/* Random modes  */

			case 'r':
    			if (toupper(optarg[0]) == 'U') {
	    			mode = PATT_RANDOM_UNIFORM;
		    		found_items =
			    		sscanf(optarg, "%c %lg %lg", &random_type, rf, rf+1);
				    if (found_items != 3) {
    					e_error("in -r parameters") ;
	    				return -1 ;
		    		}
    				min_pix = (pixelvalue) rf[0];
	    			max_pix = (pixelvalue) rf[1];
    			} else if (toupper(optarg[0]) == 'G') {
	    			mode = PATT_RANDOM_GAUSSIAN;
		    		found_items =
			    		sscanf(optarg, "%c %lg %lg", &random_type, rf, rf+1);
				    if (found_items != 3) {
    					e_error("in -r parameters") ;
	    				return -1 ;
		    		}
			    	mean = (double) rf[0];
    				sigma = (double) rf[1];
	    		} else if (toupper(optarg[0]) == 'L') {
		    		mode = PATT_RANDOM_LORENTZIAN;
			    	found_items =
    					sscanf(optarg, "%c %lg %lg", &random_type, rf, rf+1);
	    			if (found_items != 3) {
		    			e_error("in -r parameters") ;
			    		return -1 ;
    				}
	    			mean = (double) rf[0];
		    		dispersion = (double) rf[1];
    			} else {
	    			e_error("unknown random pattern: %c - aborting", optarg[0]);
		    		return -1;
			    }
    			break;

			case OPT_URAND:
	    		mode = PATT_RANDOM_UNIFORM;
		    	found_items = sscanf(optarg, "%lg %lg", rf, rf+1);
			    if (found_items!=2) {
    				e_error("--urandom requires 2 values enclosed in quotes") ;
	    			return -1 ;
		    	}
			    min_pix = (pixelvalue) rf[0];
    			max_pix = (pixelvalue) rf[1];
	    		break;

			case OPT_GRAND:
                mode = PATT_RANDOM_GAUSSIAN;
                found_items = sscanf(optarg, "%lg %lg", rf, rf+1);
                if (found_items!=2) {
                    e_error("--grandom requires 2 values enclosed in quotes") ;
                    return -1 ;
                }
                mean = (double) rf[0];
                sigma = (double) rf[1];
                break;

			case OPT_LRAND:
                mode = PATT_RANDOM_LORENTZIAN;
                found_items = sscanf(optarg, "%lg %lg", rf, rf+1);
                if (found_items!=2) {
                    e_error("--lrandom requires 2 values enclosed in quotes") ;
                    return -1 ;
                }
                mean = (double) rf[0];
                dispersion = (double) rf[1];
                break;

			/* Binary disk mode */
			case OPT_BDISK:
			case 'd':
                mode = PATT_DISK;
                found_items = sscanf(optarg, "%lg %lg %lg", rf, rf+1, rf+2);
                if (found_items != 3) {
                    e_error("-d/--bdisk require 3 values enclosed in quotes");
                    return -1 ;
                }
                x_center = (double) rf[0];
                y_center = (double) rf[1];
                radius = (double) rf[2];
                break;

			/* Binary Rectangle mode    */
			case OPT_BRECT:
			case 'R':
                mode = PATT_RECTANGLE;
                found_items =
                    sscanf(optarg, "%d %d %d %d", &llx, &lly, &urx, &ury);
                if (found_items != 4) {
                    e_error("-R/--rect require 4 values enclosed in quotes") ;
                    return -1 ;
                }
                break;

			/* Uniform mode */
			case OPT_UNIFORM:
			case 'u':
                mode = PATT_UNIFORM;
                uniform = (pixelvalue) atof(optarg);
                break;

			case OPT_POLY_2:
                mode = PATT_POLY2 ;
                found_items =
                    sscanf(optarg, "%lg %lg %lg %lg %lg %lg",
                            coeff_poly,
                            coeff_poly+1,
                            coeff_poly+2,
                            coeff_poly+3,
                            coeff_poly+4,
                            coeff_poly+5) ;
                if (found_items != 6) {
                    e_error("poly2 generation requires 6 parameters") ;
                    return -1 ;
                }
                break ;

			case OPT_POLYGON:
			case 'P':
                mode = PATT_POLYGON ;
                polygon_fname = optarg;
                break ;


			case OPT_TESTIMAGE:
                mode = PATT_TESTIMAGE ;
                break ;

			case OPT_JITTER_ARGS:
			case 'j':
                mode = PATT_JITTER ;
                found_items =
                    sscanf(optarg,
                           "%d %d %d %lg",
                           &jit_nframes,
                           &jit_nobj,
                           &jit_p_homog,
                           &jit_ampl);
                if (found_items!=4) {
                    e_error("jitter expects 4 arguments, received %d", 
                            found_items);
                    return -1 ;
                }
                break ;

			case OPT_MANDELBROT:
			case 'm':
                mode = PATT_MANDELBROT ;
                break ;

			case OPT_MANDELBROT_LOC:
                mandel_loc = malloc(4 * sizeof(double));
                found_items =
                    sscanf(optarg,
                           "%lg %lg %lg %lg",
                           &(mandel_loc[0]),
                           &(mandel_loc[1]),
                           &(mandel_loc[2]),
                           &(mandel_loc[3]));
                if (found_items!=4) {
                    e_error("--mloc expects 4 arguments, received %d",
                            found_items);
                    return -1 ;
                }
                break ;

			default:
                usage(argv[0]);
                break;
		}
	}

	if ((size_flag && sx_flag) || (size_flag && sy_flag)) {
		e_error("cannot use reference image AND specify x or y sixe");
		return -1;
	}
	switch (mode) {

	
        case PATT_POLYGON:
            polygon_file = fopen(polygon_fname, "r");
            if (polygon_file == NULL) {
                e_error("no such file: [%s]", polygon_fname);
                return -1 ;
            }
            pixelmap_out = NULL ;
            while ((polygon = polygon_load_from_file(polygon_file))!=NULL) {
                polymap = pixelmap_gen_polygon(sx, sy, polygon, 1);
                if (pixelmap_out == NULL) {
                    pixelmap_out = polymap;
                } else {
                    pixelmap_binary_OR(pixelmap_out, polymap);
                    pixelmap_del(polymap);
                }
                double3_del(polygon);
            }
            pixelmap_dump(pixelmap_out, outname);
            pixelmap_del(pixelmap_out);
            break;

	
        case PATT_POLY2:
            printf("pattern: poly2 with\n") ;
            printf("f(x,y) =   %g * x^2\n", coeff_poly[0]) ;
            printf("         + %g * y^2\n", coeff_poly[1]) ;
            printf("         + %g * x*y\n", coeff_poly[2]) ;
            printf("         + %g * x\n", coeff_poly[3]) ;
            printf("         + %g * y\n", coeff_poly[4]) ;
            printf("         + %g\n", coeff_poly[5]) ;

            image_out = image_gen_poly2d(sx, sy, coeff_poly) ;
            if (image_out == NULL) {
                e_error("cannot generate image") ;
                return -1 ;
            }
            image_save_fits(image_out, outname, ptype) ;
            break ;

	
        case PATT_UNIFORM:
            printf("pattern: uniform, NAXIS1= %d NAXIS2= %d value= %g\n",
                  sx,
                  sy,
                  uniform);
            image_out = image_new(sx, sy);
            if (image_out == NULL) {
                e_error("cannot generate image");
                return -1;
            } else {
                for (i = 0; i < (image_out->lx * image_out->ly); i++) {
                    image_out->data[i] = uniform;
                }
            }
            image_save_fits(image_out, outname, ptype);
            image_del(image_out);
            break;


        case PATT_AIRY:
            printf("Pattern: Airy width= %g max= %g\n", width, max_pix);
            printf("dx= %g dy= %g\n", dx, dy);
            image_out = image_gen_airy(sx, sy, (double)(sx/2)+dx, 
                    (double)(sy/2)+dy, max_pix, width);
            if (image_out == NULL) {
                e_error("cannot generate image");
                return -1;
            }
            image_save_fits(image_out, outname, ptype);
            image_del(image_out);
            break;

        case PATT_GAUSSIAN:
            printf("pattern: Gauss center= %g %g sigma= %g\n", x_center, 
                    y_center, sigma);
            image_out = image_gen_gauss(sx, sy, x_center-1, y_center-1, sigma);
            if (image_out == NULL) {
                e_error("cannot generate image");
                return -1;
            }
            image_save_fits(image_out, outname, ptype);
            image_del(image_out);
            break;

        case PATT_LORENTZ:
            printf("Pattern: Lorentz center= %g,%g intensity= %g disp.= %g\n",
                   x_center, y_center, intensity, dispersion);
            image_out = image_gen_lorentz(sx, sy, x_center-1, y_center-1, 
                    intensity, dispersion);
            if (image_out == NULL) {
                e_error("cannot generate image");
                return -1;
            }
            image_save_fits(image_out, outname, ptype);
            image_del(image_out);
            break;

        case PATT_RANDOM_UNIFORM:
            printf("Pattern: random min= %g max= %g\n", min_pix, max_pix);
            image_out = image_gen_random_uniform(sx, sy, min_pix, max_pix);
            if (image_out == NULL) {
                e_error("cannot generate image");
                return -1;
            }
            image_save_fits(image_out, outname, ptype);
            image_del(image_out);
            break;

        case PATT_RANDOM_GAUSSIAN:
            printf("pattern: gaussian noise, mean= %g sigma= %g\n",mean, sigma);
            printf("image size: [%d x %d]\n", sx, sy);
            image_out = image_gen_random_gauss(sx, sy, sigma, mean);
            if (image_out == NULL) {
                e_error("cannot generate image");
                return -1;
            }
            image_save_fits(image_out, outname, ptype);
            image_del(image_out);
            break;

        case PATT_RANDOM_LORENTZIAN:
            printf("pattern: lorentzian noise, dispersion= %g mean= %g\n ",
                   dispersion, mean);
            printf("image size: [%d x %d]\n", sx, sy);
            image_out = image_gen_random_lorentz(sx, sy, dispersion, mean);
            if (image_out == NULL) {
                e_error("cannot generate image");
                return -1;
            }
            image_save_fits(image_out, outname, ptype);
            image_del(image_out);
            break;

        case PATT_OTF:
            /* Assign default values for those missing  */
            if (m1_diam == -1.0)        m1_diam = PRIMARY_3_60;
            if (m2_diam == -1.0)        m2_diam = SECONDARY_3_60;
            if (w0 == -1.0)             w0 = LAMBDA_0_3_60;
            if (dw == -1.0)             dw = D_LAMBDA_3_60;
            if (pixel_scale == -1.0)    pixel_scale = PIXSCALE_3_60;

            printf("Pattern: theoretical OTF, NAXIS1= %d NAXIS2= %d\n", sx, sy);
            printf("m1 diameter: %g meters\n", m1_diam);
            printf("m2 diameter: %g meters\n", m2_diam);
            printf("central wavelength: %g microns\n", w0);
            printf("filter bandwidth: %g microns\n", dw);
            printf("Pixel scale: %g arcseconds\n", pixel_scale);
            image_out = image_gen_otf(m1_diam, m2_diam, w0, dw, sx,pixel_scale);
            if (image_out == NULL) {
                e_error("cannot generate image");
                return -1;
            }
            image_save_fits(image_out, outname, ptype);
            image_del(image_out);
            break;

        case PATT_PSF:
            /* Assign default values for those missing  */
            if (m1_diam == -1.0)        m1_diam = PRIMARY_3_60;
            if (m2_diam == -1.0)        m2_diam = SECONDARY_3_60;
            if (w0 == -1.0)             w0 = LAMBDA_0_3_60;
            if (dw == -1.0)             dw = D_LAMBDA_3_60;
            if (pixel_scale == -1.0)    pixel_scale = PIXSCALE_3_60;

            printf("Pattern: theoretical PSF, NAXIS1= %d NAXIS2= %d\n", sx, sx);
            printf("m1 diameter: %g meters\n", m1_diam);
            printf("m2 diameter: %g meters\n", m2_diam);
            printf("central wavelength: %g microns\n", w0);
            printf("filter bandwidth: %g microns\n", dw);
            printf("Pixel scale: %g arcseconds\n", pixel_scale);
            image_out = image_gen_psf(m1_diam, m2_diam, w0, dw, pixel_scale,sx);
            if (image_out == NULL) {
                e_error("cannot generate image");
                return -1;
            }
            image_save_fits(image_out, outname, ptype);
            image_del(image_out);
            break;

        case PATT_DISK:
            printf("binary disk centered in %4.2f %4.2f, radius is %6.2f\n",
                   x_center, y_center, radius);
            pixelmap_out = pixelmap_gen_disk(sx, sy, x_center, y_center,radius);
            if (pixelmap_out == NULL) {
                e_error("in disk map generation: aborting");
                return -1;
            }
            pixelmap_dump(pixelmap_out, outname);
            pixelmap_del(pixelmap_out);
            break;

        case PATT_RECTANGLE:
            printf("binary rect. with lower left corner in (%d,%d)\n",llx,lly);
            printf("and upper right corner in (%d,%d)\n", urx, ury);
            pixelmap_out = pixelmap_gen_rectangle(sx, sy, llx, lly, urx, ury);
            if (pixelmap_out == NULL) {
                e_error("in disk map generation: aborting");
                return -1;
            }
            pixelmap_dump(pixelmap_out, outname);
            pixelmap_del(pixelmap_out);
            break;

        case PATT_TESTIMAGE:
            printf("test image\n");
            image_out = image_gen_testimage();
            image_save_fits(image_out, outname, ptype);
            image_del(image_out);
            break ;

        case PATT_JITTER:
            printf("Jitter test cube\n");
            cube_out = cube_gen_jittercube(jit_nframes, sx, sy, jit_nobj,
                    jit_p_homog, jit_ampl);
            cube_save_fits(cube_out, "jitcube.fits");
            cube_del(cube_out);
            break ;

        case PATT_MANDELBROT:
            printf("Fractal image\n");
            image_out = image_gen_mandelbrot(sx, sy, mandel_loc);
            if (mandel_loc!=NULL) free(mandel_loc);
            image_save_fits(image_out, outname, ptype);
            image_del(image_out);
            break ;

        default:
            e_error("unrecognized requested pattern mode");
            return -1;
        }

	if (debug_active()) xmemory_status();
	return 0;
}

static void usage(char * pname)
{
	hello_world(pname, prog_desc);
	printf("use : %s [commands]\n", pname);
	printf(
		"commands are:\n"
		"\t-x value -y value                output image size\n"
		"\t-b (--bitsperpix) val            output pixel depth (FITS)\n"
		"\t-o (--output) name               output file name\n"
		"\t-S (--ref) name                  to give reference file name\n"
		"\t                                 for image size.\n");
	printf(
		"\t-a or --airy 'width max dx dy'   Airy pattern\n"
		"\t-g or --gauss 'X Y sigma'        Gauss pattern\n"
		"\t-l or --lorentz 'X Y intensity dispersion'\n"
		"\t                                 Lorentz pattern\n"
		"\t-p 'm1 m2 w0 dw pixelscale'      "
		"telescope optical transfer function\n");
	printf(
		"\t--otf                            alternate form with longopts:\n"
		"\t\t--m1 value               primary mirror diameter (m)\n"
		"\t\t--m2 value               secondary mirror diameter (m)\n"
		"\t\t--lambda0 value          central wavelength (microns)\n"
		"\t\t--dlambda value          wavelength interval (microns)\n");
	printf(
		"\t\t--pixelscale value       pixelscale in arcseconds\n"
		"\t-t 'm1 m2 w0 dw pixelscale'      telescope point-spread function\n"
		"\t--psf                            alternate form with longopts:\n"
		"\t\t                         side options as for --otf\n");
	printf(
		"\t-r 'U min max' or --urand 'mean sigma'\n"
		"\t                                 uniform random image\n"
		"\t-r 'G mean sigma' or --grand 'mean sigma'\n"
		"\t                                 gaussian random image\n"
		"\t-r 'L mean dispersion' or --lrand 'mean disp'\n"
		"\t                                 lorentzian random image\n");
	printf(
		"\t-d 'x0 y0 r' or --bdisk 'x0 y0 r' binary disk\n"
		"\t-R 'llx lly urx ury' or --brect 'llx lly urx ury'\n"
		"\t                                 binary rectangle\n"
		"\t-P (--polygon) <filename>        binary polygons\n"
		"\t-u (--uniform) value             uniform image\n");
	printf(
		"\t--poly2 '6 coeffs'               2 degree polynomial image\n"
		"\t-j or --jitter 'nframes nobj p_homog ampl'\n"
		"\t                                 Jitter test cube\n"
		"\t-m or --mandel                   Mandelbrot set (test) image\n"
		"\t--mloc 'xmin xmax ymin ymax'     Mandelbrot set location\n"
		"\n\n");
	exit(0);
}
