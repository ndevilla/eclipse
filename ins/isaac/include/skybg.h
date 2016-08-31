
const char isaac_skybg_version[]="$Revision: 1.9 $" ;
const char isaac_skybg_date[]="$Date: 2002/12/10 13:10:02 $" ;
int  isaac_skybg_main(void * d) ;

cmdline_spec isaac_skybg_cmd[] = {
	{'m', "mode", "Force algorithm to a given mode", 1, "mode"},
	{0, 0, 0, 0, 0}
} ;

const char isaac_skybg_man[] =
"NAME\n"
"       skybg - ISAAC sky background estimation\n"
"\n"
"SYNOPSIS\n"
"       isaacp skybg [options] in\n"
"\n"
"DESCRIPTION\n"
"       This recipe tries to estimate the sky background in a  set\n"
"       of input frames. There are two  different algorithms to be\n"
"       considered, depending on the input data type:\n"
"              - LW imaging\n"
"              - LW spectroscopy\n"
"\n"
"       You can force one of the two  algorithms  to  be  used  by\n"
"       using -m option. Default is to let the  recipe  decide  on\n"
"       which algorithm to use by looking into the  input  header.\n"
"\n"
"OPTIONS\n"
"       -m or --mode \"mode\"\n"
"             Force algorithm to a given mode.\n"
"             Possible modes are:\n"
"               - lw-imag\n"
"               - lw-spec\n"
"\n\n" ;
