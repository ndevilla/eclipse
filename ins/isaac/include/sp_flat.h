
const char isaac_sp_flat_version[] = "$Revision: 1.2 $" ;
const char isaac_sp_flat_date[]= "$Date: 2002/12/10 13:26:21 $" ;
int  isaac_sp_flat_main(void * d);

cmdline_spec isaac_sp_flat_cmd[] = {
	{'o', "output", "Output file", 1, "outfile"},
	{'r', "rectangle", "Vignet", 1, "vig"},
	{'l', "low", "Low threshold", 1, "low_thresh"},
	{'h', "high", "High threshold", 1, "high_thresh"},
	{'f', "fit_order", "Order of the fit", 1, "fit_order"},
	{'s', "fit_size", "X size of the fitted region", 1, "fit_size"},
	{'g', "offset", "Offset used to detect bad zones", 1, "offset"},
	{'i', "save", "Flag to output intermediate results", 0, NULL},
	{'p', "save_poly", "Flag to output poly images", 0, NULL},
	
	{0, 0, 0, 0, 0}
};


const char isaac_sp_flat_man[] =
"NAME\n"
"       sp_flat - spectroscopic flatfielding\n"
"\n"
"SYNOPSIS\n"
"       isaacp sp_flat [options] in\n"
"\n"
"DESCRIPTION\n"
"       sp_flat expects an ascii file as input  file.  This  ascii\n"
"       file contains 1 or more pairs of 'on' - 'off' flat frames.\n"
"       For each pair, the first frame has to be the 'on' one, and\n"
"       the second the 'off' one. It is possible to have different\n"
"       settings for the different pairs. In such  a  case,  pairs\n"
"       are first classified by settings and a reduction  is  done\n"
"       for each setting (one product by setting).\n"
"\n"
"       To compare the settings of 2 frames,  the  following  key­\n"
"       words are compared:\n"
"\n"
"       HIERARCH.ESO.INS.OPTI1.ID\n"
"       HIERARCH.ESO.INS.GRAT.NAME\n"
"       HIERARCH.ESO.INS.GRAT.WLEN\n"
"\n"
"       These  keywords have to match to have the same setting\n"
"\n"
"ALGORITHM\n"
"       This algorithm is applied to each pair of frames.\n"
"\n"
"       The difference 'on'-'off' is computed and the result frame\n"
"       is divided by its mean. The zone where the mean is defined\n"
"       can be specified in the options. In the  end,  the  pixels\n"
"       whose value is lower than 0.1 or greater than 2 are set to\n"
"       1.\n"
"\n"
"       If there are more than 1 pair of frames for  one  setting,\n"
"       the results are averaged in one image.\n"
"\n"
"       The  master  flat created is then fitted (2d second degree\n"
"       polynomial) and divided by the fit.\n"
"\n"
"       If there is no output name specified, the  default  output\n"
"       frame  name  is  in_flat_X.fits where X goes from 1 to the\n"
"       number of different settings.\n"
"\n"
"OPTIONS\n"
"       -o or --output outname\n"
"              outname is the output files basename.\n"
"\n"
"       -r or --rectangle 'llx lly urx ury'\n"
"              Specify the zone where the mean is  computed.  Pro­\n"
"              vide  a  set of 4 coordinates enclosed in single or\n"
"              double quotes, in this  order:  lower  left  corner\n"
"              coordinates  in X and Y, upper right corner coordi­\n"
"              nates in X and Y. The corners of the rectangle (and\n"
"              borders)  are  included in the zone. The coordinate\n"
"              system is respecting  the  FITS  convention:  lower\n"
"              left  pixel  in the image is at (1,1), X increasing\n"
"              from left to right and Y from bottom to top.\n"
"\n"
"       -l or --low low_thresh\n"
"              Specify the low threshold under  which  pixels  are\n"
"              set to 1.\n"
"\n"
"       -h or --high high_thresh\n"
"              Specify  the  high threshold above which pixels are\n"
"              set to 1.\n"
"\n"
"       -f or --fit_order n\n"
"              n is the order of the final fit (degree n-1).\n"
"\n"
"       -s or --fit_size size\n"
"              size is the X size of the fitted region\n"
"\n"
"       -g or --offset offset\n"
"              As the zones at the top and at the  bottom  of  the\n"
"              flat  are  usually  set  to 1 (bad zones), they are\n"
"              rejected before the fit. The rejected lines are the\n"
"              one detected plus offset.\n"
"\n"
"       -i or --save\n"
"              Flag  to  write  all  the intermediate master flats\n"
"              (names: tmp_pairnumber_outname).\n"
"\n"
"       -p or --save_poly\n"
"              Flag to save the image  of  the  fitted  polynomial\n"
"              (names: poly_settingnb_pairnb.fits).\n"
"\n"
"FILES\n"
"       The  default  output  name  is  the base name of the first\n"
"       input file followed by \"_flat_X.fits\". It is possible to\n"
"       specify  an  output  basename on the command line. All the\n"
"       created files are FITS files whose header is the same  one\n"
"       as for the input files with some more \"PRO\" keywords. If\n"
"       'out' is the specified output name, the output files  will\n"
"       be named out_X.fits.\n"
"\n";


