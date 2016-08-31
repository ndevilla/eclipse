
const char isaac_startrace_version[] = "$Revision: 1.5 $" ;
const char isaac_startrace_date[]="$Date: 2004/02/25 10:58:19 $" ;
int  isaac_startrace_main(void * d) ;

cmdline_spec isaac_startrace_cmd[] = {
	{'o', "output", "Output file", 1, "outfile"},
	{'d', "degree", "Polynomial degree", 1, "deg"},
	{'w', "width", "Spectrum width", 1, "width"},
	{'r', "sky_dist", "Sky distance to spectrum", 1, "sky_dist"},
	{'R', "sky_width", "Sky width", 1, "sky_width"},
	{'b', "reject_l", "Rejected pixels on the left", 1, "reject_l"},
	{'B', "reject_r", "Rejected pixels on the right", 1, "reject_r"},
	{'p', "display", "Plot intermediate results", 0, NULL},
	{'l', "disto_lr", "To correct the distorsion in LR", 1, "disto_lr"},
	{'m', "disto_mr", "To correct the distorsion in MR", 1, "disto_mr"},
	{'c', "out_corr", "Flag to output corrected images", 0, NULL},
    {0, 0, 0, 0, 0}
};

const char isaac_startrace_man[] =
"NAME\n"
"       startrace - ISAAC startrace analysis\n"
"\n"
"SYNOPSIS\n"
"       isaacp startrace [options] in\n"
"\n"
"DESCRIPTION\n"
"       startrace takes as input an ASCII images list and produces\n"
"       6 tfits tables in output. The input images are composed by\n"
"       n star images (imaging mode), n spectra  (LR mode)  and  n\n"
"       spectra (MR mode). The output tables are:\n"
"       1.  a  correspandance  table  with  two   polynomials   (2\n"
"       columns):   1   for  the  relation  star_position-LR_spec­\n"
"       tra_position, 1 for the relation star_position-MR_spectra.\n"
"       Name: outname_corresp.tfits\n"
"       2. a table with the extracted spectra: first column is the\n"
"       wavelengths (LR), the  n  following  columns  are  the  LR\n"
"       extracted  spectra,  the  column  (n+2) is the wavelengths\n"
"       (MR), the last n columns are  the  MR  extracted  spectra.\n"
"       Name: outname_extracted.tfits\n"
"       3.  a  table  with the positions of the stars and spectra.\n"
"       Name: outname_positions.tfits\n"
"       4. a table  with  the  polynomials  fitting  the  spectra.\n"
"       Name: outname_shapes.tfits\n"
"       5. a table with the 2d polynomial describing the startrace\n"
"       distortion in LR.  Name: outname_poly2d_LR.tfits (used  by\n"
"       spjitter).\n"
"       6. Same as 5 in MR\n"
"\n"
"ALGORITHM\n"
"       The following steps are performed:\n"
"       1.  Classify  the  input frames in three batches (imaging,\n"
"       LR, MR).\n"
"       2. Correct the distortion in input frames.\n"
"       3. Detect the star and spectra  positions  and  write  the\n"
"       output table.\n"
"       4. Fit the positions (imag.-LR ans imag.-MR) and write the\n"
"       polynomials in output tables.\n"
"       5. Wavelength calibration (physical model).\n"
"       6. Extract the spectra and write the output table.\n"
"       7. Fit the spectra shapes and write the output table.\n"
"       8. Compute the two deformation 2d polynomials (LR ans  MR)\n"
"       and write the output tables.\n"
"\n"
"OPTIONS\n"
"       -o or --output outfile\n"
"              Specify the output files base name\n"
"\n"
"       -d or --degree deg\n"
"              Specify  the  degree of the polynomials used to fit\n"
"              the spectra (default is 3).\n"
"\n"
"       -w or --width\n"
"              Specify the spectrum width. Used for extraction and\n"
"              shape analysis.\n"
"\n"
"       -R or --sky_width\n"
"              Specify  the width of the residual sky used for the\n"
"              extraction.\n"
"\n"
"       -r or --sky_dist\n"
"              Specify the distance between the spectrum  ans  the\n"
"              residual sky. Used for the extraction.\n"
"\n"
"       -b or --reject_l\n"
"              Specify the number of columns to reject at the left\n"
"              of the image. Used for shape analysis.\n"
"\n"
"       -B or --reject_r\n"
"              number of columns to reject at the right.\n"
"\n"
"       -p or --display\n"
"              flag to activate the display mode.\n"
"\n"
"       -l or --disto_lr\n"
"              Specify either an ARC table, or a calibration  lamp\n"
"              image.  Used  for the slitcurvature distortion cor­\n"
"              rection in LR.\n"
"\n"
"       -m or --disto_mrfile\n"
"              the same in MR.\n"
"\n"
"       -c or --out_corr\n"
"              Flag to output distortion corrected images\n"
"\n"
"\n";



