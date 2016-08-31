
const char conica_checkfocus_version[] = "$Revision: 1.3 $";
const char conica_checkfocus_date[]="$Date: 2003/04/28 10:29:07 $";
int  conica_checkfocus_main(void * d);

cmdline_spec conica_checkfocus_cmd[] = {
	{'o', "output", "Output file(s) base name", 1, "filename"},
	{'d', "display", "Enable plots", 0, NULL},
    {0, 0, 0, 0, 0}
};

const char conica_checkfocus_man[]=
"\n"
"NAME\n"
"\tcheck-focus -- CONICA Focus checking for Quality Control\n"
"\n"
"SYNOPSIS\n"
"\tconicap check-focus [options] in\n"
"\n"
"DESCRIPTION\n"
"\tSorry, not available yet.\n"
"\n"
"OPTIONS\n"
"\t-o or --output 'filename'\n"
"\t\tChange the output file base name.\n"
"\n"
"\t-d or --display\n"
"\t\tEnable plots.\n"
"\n"
"\n\n";

