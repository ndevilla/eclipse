
const char isaac_dark_version[] = "$Revision: 1.13 $";
const char isaac_dark_date[]="$Date: 2003/01/29 15:19:54 $";

cmdline_spec isaac_dark_cmd[] = {
	{'a', "average", "Flag to only compute average", 0, NULL},
	{'r', "ron", "Flag to only compute ron", 0, NULL},
	{'h', "hsize", "Half size of the squares to compute RON", 1, "half-size"},
	{'n', "nsamples", "Number of samples to be used for RON", 1, "nsamples"},
	{'o', "output", "Output file base name", 1, "filename"},
	{0, 0, 0, 0, 0}
};

int isaac_dark_main(void * d);


const char isaac_dark_man[] =
"NAME\n"
"       dark - ISAAC dark recipe\n"
"\n"
"SYNOPSIS\n"
"       isaacp dark [options] in\n"
"\n"
"DESCRIPTION\n"
"       This recipe first classify the input frames  per  setting.\n"
"       A setting is defined by the DIT, NDIT and  read-out  mode.\n"  
"       In each setting, a linear average of  the input frames  is\n"
"       produced, and on each difference of  successive frames,  a\n"
"       read-out noise computation is applied.  If a  setting  has\n"
"       N frames, N files are produced:  1 average frame, and  N-1\n"
"       PAF files with the result  of  the  RON  computation.  The\n"
"       produced PAF files are compliant  with  VLT  product  DICB\n"
"       scheme.\n"
"\n"
"ALGORITHM\n"
"       The RON computation is done like this:\n"
"\n"
"       100 random windows of size 9x9 pixels are generated in the\n"
"       zone of interest. For each window, the standard  deviation\n"
"       of the signal is computed, and the median of those  values\n"
"       is obtained. The readout noise is:\n"
"\n"
"       RON = median * sqrt(ndit/2)\n"
"\n"
"       For the ISAAC SW arm, this process is applied over  the  4\n"
"       quadrants, yielding 4 RON values.\n"
"\n"
"OPTIONS\n"
"       -o or --output outname\n"
"              outname is the output files basename.\n"
"\n"
"       -a or --average\n"
"              flag to only compute the average part.\n"
"\n"
"       -r or --ron\n"
"              flag to only compute the RON.\n"
"\n"
"       -h or --hsize half-size\n"
"              To specify the RON squares size.\n"
"\n"
"       -n or --nsamples nsamp\n"
"              To specify the number of samples used to compute the RON.\n"
"\n"
"FILE\n"
"       Output frames are called  basename_<set_nb>.fits  for  the\n"
"       average  results  and  basename_<set_nb>_<pair_nb>_ron.paf\n"
"       for the RON results. This basename can be changed with the\n"
"       -o option.\n"
"\n";

