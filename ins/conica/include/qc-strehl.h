
const char conica_qcstrehl_version[] = "$Revision: 1.2 $";
const char conica_qcstrehl_date[]="$Date: 2003/08/25 13:44:12 $";
int  conica_qcstrehl_main(void * d);

cmdline_spec conica_qcstrehl_cmd[] = {
	{'o', "output", "Output file(s) base name", 1, "filename"},
    {'r', "star_radius", "Radius of the star", 1, "star_radius"},
    {'R', "background", "Radii for the bkg", 1, "'r1 r2'"},
    {0, 0, 0, 0, 0}
};

const char conica_qcstrehl_man[]=
"\n"
"NAME\n"
"\tqc-strehl -- CONICA Strehl computation for Quality Control\n"
"\n"
"SYNOPSIS\n"
"\tconicap qc-strehl [flags] [-o/--output filename] in\n"
"\n"
"DESCRIPTION\n"
"\tSorry, not available yet.\n"
"\n"
"OPTIONS\n"
"\t-o or --output 'filename'\n"
"\t\tChange the output file base name. Default is 'flat':\n"
"\n"
"\n\n";

