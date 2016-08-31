
const char isaac_rename_version[] = "$Revision: 1.3 $" ;
const char isaac_rename_date[]="$Date: 2002/12/10 12:33:42 $" ;
int  isaac_rename_main(void * d) ;

cmdline_spec isaac_rename_cmd[] = {
	{'a', "archive", "Use archive name to rename", 0, NULL},
	{'o', "origfile", "Use origfile name to rename", 0, NULL},
	{'r', "ref_name", "Reference file to compute new name", 1, "refname"},

    {0, 0, 0, 0, 0}
};

const char isaac_rename_man[] =
"NAME\n"
"       isaacp rename  - ISAAC products renaming recipe\n"
"\n"
"SYNOPSIS\n"
"       isaacp rename [options] in\n"
"\n"
"DESCRIPTION\n"
"       The files are identified with their PRO_CATG keyword, and \n"
"       the new name is derived from it  and  from  keywords  read\n"
"       in the header.\n"
"\n"
"OPTIONS\n"
"       -r or --ref_name file\n"
"              The provided file is used to get infos needed from\n"
"              its header\n"
"\n"
"       -a or --archive\n"
"              To rename the file in the ARCFILE keyword\n"
"\n"
"       -o or --origfile\n"
"              To rename the file in the ORIGFILE keyword\n"
"\n"
"NOTE\n"
"       Without any option, the new file  name  is  defined  according\n"
"       a naming convention that differs for each  different  kind  of\n"
"       files.\n"
"       The file type is determined by its PRO.CATG keyword,  and  for\n"
"       each of these products, there is one way to build the new name\n"
"       with informations found  in  the  header  (date, filter, ...).\n"
"       For the details of the naming convention, see \n\n"
"       http://www.eso.org/observing/dfo/quality/index_isaac.html\n"
"\n"
"\n";

