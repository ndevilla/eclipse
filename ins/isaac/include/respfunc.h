
const char isaac_respfunc_version[] = "$Revision: 1.2 $" ;
const char isaac_respfunc_date[]= "$Date: 2002/12/10 12:38:37 $" ;
int  isaac_respfunc_main(void * d) ;

cmdline_spec isaac_respfunc_cmd[] = {
	{'o', "output", "Output file", 1, "outfile"},
	{'s', "width", "Spectrum width", 1, "spec_width"},
	{'g', "sky_dist_lo", "Residual sky under the spectrum", 1, "sky_dist_lo"},
	{'G', "sky_dist_hi", "Residual sky above the spectrum", 1, "sky_dist_hi"},
	{'w', "sky_width_lo", "Residual sky width (under)", 1, "sky_width_lo"},
	{'W', "sky_width_hi", "Residual sky width (above)", 1, "sky_width_hi"},
	{'d', "display", "To display results", 0, NULL},
	{'c', "wavelength", "Force the dispersion", 1, "wl"},
	{'f', "filter", "Flag to use filter", 0, NULL},
	{'i', "star_infos", "Standard star mag and temp", 1, "std_star_infos"},
	
	{0, 0, 0, 0, 0}
};

const char isaac_respfunc_man[] =
"NAME\n"
"       respfunc - ISAAC spectroscopic response function\n"
"\n"
"SYNOPSIS\n"
"       isaacp respfunc [options] in\n"
"\n"
"DESCRIPTION\n"
"       respfunc accepts an image  in  input,  and  produces  four\n"
"       tables. The input image should be a combined image with  a\n"
"       bright  standard  star spectrum which will be detected and\n"
"       extracted. The header is read  to  identify  the  standard\n"
"       star and deduce its temperature and magnitude.  The output\n"
"       tables are:\n"
"       1.  the  extracted  spectrum  wavelength   calibrated   (2\n"
"       columns).  Name:outname_extr.tfits\n"
"       2.   the  background  spectrum  wavelength  calibrated  (2\n"
"       columns).  Name:outname_back.tfits\n"
"       3. the conversion  file  (see  documentation).   Name:out­\n"
"       name_conversion.tfits\n"
"       4.  the  efficiency  curve (see documentation).  Name:out­\n"
"       name_efficiency.tfits\n"
"\n"
"ALGORITHM\n"
"       The  spectrum  is  detected,  extracted,  background  sub­\n"
"       tracted,  wavelength  calibrated in the first step.  Then,\n"
"       many coefficient are computed: the star position  is  read\n"
"       in the header and used to identify the observed star. With\n"
"       this identfication, the star temperature and magnitude (in\n"
"       the  observation band) are found, the DIT is read from the\n"
"       header, the average dispersion comes from  the  wavelength\n"
"       calibration,  the  central  wavelength  and the zero point\n"
"       from a table (depend  on  observation  band).   Thanks  to\n"
"       these values, the conversion file and the efficiency curve\n"
"       are  computed  with  formulas  given in the documentation.\n"
"\n"
"OPTIONS\n"
"       -o or --output outfile\n"
"              outfile is the output files base name\n"
"\n"
"       -s or --width\n"
"              Specify the extraction window size for the spectrum\n"
"              extraction. Default is 15 pixels.\n"
"\n"
"       -w or --sky_width_lo\n"
"              Specify  the  width  of  the residual sky below the\n"
"              spectrum.  Default is 10 pixels.\n"
"\n"
"       -W or --sky_width_hi\n"
"              Specify the width of the  residual  sky  above  the\n"
"              spectrum.  Default is 10 pixels.\n"
"\n"
"       -g or --sky_dist_lo\n"
"              Specify the gap between the spectrum and the resid­\n"
"              ual sky below the  spectrum.  Default is 20 pixels.\n"
"\n"
"       -G or --sky_dist_hi\n"
"              Specify the gap between the spectrum and the resid­\n"
"              ual sky above the spectrum.  Default  is 20 pixels.\n"
"\n"
"       -d or --display\n"
"              Flag to activate the display mode.\n"
"\n"
"       -c or --wavelength \"a b\"\n"
"              a and b are the  coefficients  for  the  dispersion\n" 
"              relation : wavelength(pix) = a + b * pix\n"
"\n"
"       -f or --filter\n"
"              flag to use a median filter or not before  spectrum\n" 
"              extraction\n"
"\n"
"       -i or --star_infos \"a b \"\n"
"              a is the magnitude, b is the star temperature\n"
"\n"
"\n";

