
const char conica_slitpos_version[] = "$Revision: 1.2 $" ;
const char conica_slitpos_date[]="$Date: 2003/04/30 06:47:10 $"  ;
int  conica_slitpos_main(void * d)   ;

cmdline_spec conica_slitpos_cmd[] = {
	{'o', "output", "Output file", 1, "outfile"},
	{'p', "products", "Flag to produce products", 0, NULL},
    {0, 0, 0, 0, 0}
};

const char conica_slitpos_man[] =
"\n"
"NAME\n"
"       slitpos - slit position analysis\n"
"\n"
"SYNOPSIY\n"
"       slitpos [options] infiles\n"
"\n"
"DESCRIPTION\n"
"       slitpos finds the exact position of a slit. The  slit  has\n"
"       to be horizontal: less than 10 degrees with the horizontal\n"
"\n"
"ALGORITHM\n"
"       slitpos expects an image of a slit or an ascii  file  with\n"
"       fits slit images names.\n"
"\n"
"       1.  Find  the  position  (in  x) of the slit (collapse the\n"
"       image, find the maximum).\n"
"\n"
"       2. Extract a thin image containing only the slit.\n"
"\n"
"       3. Find the edges of the slit. Binarize  the  thin  image,\n"
"       erode the result with a vertical kernel until there is one\n"
"       object left (supposed to be the slit) and  reconstruct  it\n"
"       with dilatations.\n"
"\n"
"       4.  Line by line, detect the edges (right and left) of the\n"
"       slit (comparison with a threshold equal to the mean of the\n"
"       current line).\n"
"\n"
"       5.  Linear regression of the points to have the 2 edges of\n"
"       the slit, and write the result in the output table.\n"
"\n"
"OPTIONS\n"
"       -o or --output outfile\n"
"               Specify the output files base name.\n"
"\n"
"FILES\n"
"       The output table name is [out].tfits if an output basename\n"
"       is  specified.   In  the  other  case,  it  is  [in]_slit­\n"
"       pos.tfits.   The  output  table  contains  3 columns whose\n"
"       labels are: BOT_POSITION CENTER_POSITION and  TOP_POSI­\n"
"       TION.  These columns give the positions in FITS convention\n"
"       of the slit.\n"
"\n"
"       Besides,  the  slit  center,  the slit angle are displayed\n"
"       on stdout.\n"
"\n"
"\n";

