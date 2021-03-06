
const char isaac_illum_version[] = "$Revision: 1.3 $";
const char isaac_illum_date[]="$Date: 2002/12/10 10:26:13 $";
int  isaac_illum_main(void * d);

cmdline_spec isaac_illum_cmd[] = {
    {'o', "output", "Specify an output file base name", 1, "filename"},
    {'d', "dark", "Specify a dark for subtraction", 1, "filename"},
    {'f', "flat", "Specify a flat-field for division", 1, "filename"},
    {'b', "badpix", "Specify a bad pixel mask", 1, "filename"},
    {'s', "search", "Standard star search half-sizes", 1, "'hx hy'"},
    {'r', "radius", "Photometry computation radiuses", 1, "'star bgin bgout'"},
    {'F', "flux", "Output flux computation to ASCII file", 1, "filename"},
    {0, 0, 0, 0, 0}
};

const char isaac_illum_man[]=
"NAME\n"
"       illum - ISAAC illumination frame handling\n"
"\n"
"SYNOPSIS\n"
"       isaacp illum [options] in\n"
"\n"
"DESCRIPTION\n"
"       illum  applies  the pipeline data  reduction  process  for\n"
"       illumination frames taken with ISAAC. The  algorithms  are\n"
"       hopefully the same for any similar detector.\n"
"\n"
"ALGORITHM\n"
"       illum  expects  a  list  of illumination  frames  to  work\n"
"       with. Illumination frames are a set of images of  a  stan�\n"
"       dard star taken on a regular grid over the detector, typi�\n"
"       cally 4x4 or 5x5. The offsets  in  pixels  between  frames\n"
"       must be provided in the header. The first frame in the set\n"
"       must be at offset (0,0) (the star is roughly at the center\n"
"       of  the  image),  all  other offsets being specified rela�\n"
"       tively to this one.\n"
"\n"
"       illum will subtract dark, divide by flat-field and correct\n"
"       bad pixels if the adequate calibration files are available\n"
"       (see -d -f and -b options). Offsets are then read from the\n"
"       FITS headers, and a peak re-location is performed to  find\n"
"       precisely where the star lies in each  image.  The  search\n"
"       domain size around the provided places can be changed with\n"
"       the -s option if you know that header offsets are false by\n"
"       a larger amount than the default one (50).\n"
"\n"
"       Aperture photometry in each plane is then computed accord�\n"
"       ing  to  three radii you can change through the -r option.\n"
"       The first radius encloses the star, the two others enclose\n"
"       the  background to be subtracted out.  Computed fluxes can\n"
"       be written to an ASCII file upon user request.\n"
"\n"
"       Next step is then to fit a 2d polynomial  surface  to  the\n"
"       list  of photometric values, and normalize  this  surface.\n"
"       The result frame is saved with a default name.\n"
"\n"
"OPTIONS\n"
"\n"
"       -d or --dark filename\n"
"              Specifies the name of the dark file to use. Default\n"
"              is no dark file.\n"
"\n"
"       -f or --ff filename\n"
"              Specifies  the  name  of  the  flat-field  to  use.\n"
"              Default is no flat-field.\n"
"\n"
"       -b or --badpix filename\n"
"              Specifies  the  name  of  a  bad  pixel map to use.\n"
"              Default is no bad pixel map.\n"
"\n"
"       -s or --search 'hx hy'\n"
"              Changes the size of the search  domain  around  the\n"
"              provided  positions.   hx  and hy are half sizes in\n"
"              pixels, i.e. if hx=50 and hy=50 the  search  domain\n"
"              is  101x101  pixels  around  each  position. If you\n"
"              enlarge  too  much  this  domain,  you  risk  false\n"
"              matches  (the  detected peak does not correspond to\n"
"              the standard star but to another bright  object  in\n"
"              the  neighborhood).  Default is hx=50 and hy=50. Of\n"
"              course, peak re-location will NOT work  in  crowded\n"
"              fields.\n"
"\n"
"       -r or --radii 'rstar rbgin rbgout'\n"
"              Specifies three radii for fixed-aperture photometry\n"
"              computation.  belonging to the  star.  'rbgin'  and\n"
"              'rbgout'  specify  a  crown  centered  on the star,\n"
"              within which all pixels will be considered as back�\n"
"              ground.   The  algorithm  gathers all pixels within\n"
"              that crown, picks the median value,  and  subtracts\n"
"              it  from all star pixels while summing up.  Default\n"
"              values are rstar=10, rbgin=12, rbgout=30.\n"
"\n"
"       -F or --flux filename\n"
"              Will produce an  ASCII  file  containing  for  each\n"
"              input plane the position of the re-located star and\n"
"              the computed flux.\n"
"\n"
"\n";

