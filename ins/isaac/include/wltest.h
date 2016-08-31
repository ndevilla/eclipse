
const char isaac_wltest_version[] = "$Revision: 1.1 $";
const char isaac_wltest_date[]="$Date: 2003/09/10 19:17:51 $";
int  isaac_wltest_main(void * dict);

cmdline_spec isaac_wltest_cmd[] = {
    {'b', "border", "Declare invalid image borders", 1, "low high"},
    {'z', "zero", "Declare region to ignore on each side", 1, "left right"},
    {'T', "thermal", "Remove thermal background", 0, NULL},
    {'t', "table", "Name of spectral table to use", 1, "name"},
    {'w', "wave", "Force input wavelength range", 1, "min max"},
    {'h', "header", "Modify input file header", 0, NULL},
    {'o', "order", "Specify the order used", 1, "order"},
    {0, 0, 0, 0, 0}
};

const char isaac_wltest_man[] =
"NAME\n"
"       wltest - ISAAC wavelength calibration testing recipe\n"
"\n"
"SYNOPSIS\n"
"       isaacp wltest [options] in\n"
"\n"
"DESCRIPTION\n"
"\n"
"ALGORITHM\n"
"\n"
"OPTIONS\n"
"\n";

