
const char conica_lampflat_version[] = "$Revision: 1.3 $";
const char conica_lampflat_date[]="$Date: 2002/10/04 08:18:43 $";
int  conica_lampflat_main(void * d);

cmdline_spec conica_lampflat_cmd[] = {
	{'o', "output", "Output file(s) base name", 1, "filename"},
    {'r', "rej_bord", "Rejected borders for gain norm.", 1, "'le ri bo to'"},

    {0, 0, 0, 0, 0}
};

const char conica_lampflat_man[]=
"\n"
"NAME\n"
"\tlampflat -- CONICA imaging flat-field creation from lamp images.\n"
"\n"
"SYNOPSIS\n"
"\tconicap lampflat [flags] [-o/--output filename] in\n"
"\n"
"DESCRIPTION\n"
"\n"
"ALGORITHM\n"
"\n"
"OPTIONS\n"
"\t-o or --output 'filename'\n"
"\t\tChange the output file base name. Default is 'flat':\n"
"\n"
"\t-r or --rej_bord 'le ri bo to'\n"
"\t\tSpecify the rejected borders (left, right, bottom, top)\n"
"\t\tfor the mean computation for gain normalization.\n"
"\n"
"\n\n";

