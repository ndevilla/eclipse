
const char conica_detlin_version[]="$Revision: 1.5 $";
const char conica_detlin_date[]="$Date: 2003/08/18 14:22:40 $";

cmdline_spec conica_detlin_cmd[] = {
	{'o', "output", "Output file base name", 1, "filename"},
	{'f', "force", "To force the computation", 0,  NULL},
	{0, 0, 0, 0, 0}
};


int conica_detlin_main(void * d);

const char conica_detlin_man[] =
"NAME\n"
"\tdetlin- CONICA detector linearity check\n"
"\n"
"SYNOPSIS\n"
"\tconicap detlin [-o/--output basename] in\n"
"\n"
"DESCRIPTION\n"
"\tThis recipe computes linearity checks on the detector.\n"
"\n"
"ALGORITHM\n"
"\tNot validated yet.\n"
"\n"
"OPTIONS\n"
"\t-o or --output 'filename'\n"
"\t\tChange the default output base name to 'filename'.\n"
"\n\n"
;

