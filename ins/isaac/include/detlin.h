
const char isaac_detlin_version[]="$Revision: 1.4 $";
const char isaac_detlin_date[]="$Date: 2002/12/10 09:39:12 $";

cmdline_spec isaac_detlin_cmd[] = {
	{'o', "output", "Output file base name", 1, "filename"},
	{'f', "force", "To force the computation for unstable lamps",0, NULL},
	{0, 0, 0, 0, 0}
};


int isaac_detlin_main(void * d);

const char isaac_detlin_man[] =
"NAME\n"
"       detlin - ISAAC detector linearity check\n"
"\n"
"SYNOPSIS\n"
"       isaacp detlin [options] in\n"
"\n"
"DESCRIPTION\n"
"       This recipe computes linearity checks on the detector.  It\n"
"       expects in input a series of frames taken with  increasing\n"
"       DIT values.  This  increasing  signal  is  fitted foe each\n"
"       pixel, and the coefficients are stored in  output  images.\n"
"       Along the acquisition, each 4 or 5 frames, an  acquisition\n"
"       with a given DIT is  done  to  verify  the lamp stability.\n"
"\n"
"ALGORITHM\n"
"       The first image DIT is assumed to be the one used to check\n"
"       the lamp stability (the expected DIT sequence is something\n"
"       like 0.13, 0.2, 0.3, 0.4, 0.13, 0.5, 0.6, 0.7,  0.13,  ...\n"
"       The recipe first gets the frames with the same DIT as  the\n"
"       first one, and checks that the level difference  does  not\n"
"       exceed 1%%. If it does, the recipe stops,  unless  the  -f\n"
"       flag is used.\n"
"       The remaining frames are then loaded. Their is supposed to\n"
"       be two frames per DIT: one dark, one  lamp.  The  dark  is\n"
"       first subtracted from the  corresponding  lamp.  For  each\n"
"       pixel, DIT = a*flux + b*flux^2 + c*flux^3 is fitted. The 3\n"
"       coefficients and the fit error are written  in  4  images.\n"
"OPTIONS\n"
"       -o or --output outname\n"
"              outname is the output files basename.\n"
"\n"
"       -f or --force\n"
"              flag to still compute the fit, even if the lamp  is\n"
"              not stable.\n"
"\n"
"FILE\n"
"       Output frames are called basename_A.fits, _B.fits, _C.fits\n"
"       for the coefficients, and _Q.fits  for  the  fit  quality.\n"
"       The basename  can  be  changed  with  the --output option.\n"
"\n";

