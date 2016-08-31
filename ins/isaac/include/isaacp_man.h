
static const char isaacp_man_date[] = "$Date: 2002/12/10 12:15:50 $" ;
static const char isaacp_man[] =

"\n"
"\n"
"***** ISAAC pipeline use\n"
"\n"
"\n"
"\n"
"This command implements most of the ISAAC pipeline  recipes.  All\n"
"supported recipes share a similar user-interface  for  parameters\n"
"or command-line arguments, these conventions are described below.\n"
"\n"
"To get more help, you can try:\n"
"\n"
"% isaacp             -- list of all supported recipes.\n"
"% isaacp man         -- this help.\n"
"% isaacp recipe      -- short command-line help.\n"
"% isaacp man recipe  -- man page for the recipe.\n"
"\n"
"% isaacp manual      -- generate full documentation\n"
"% isaacp manual html -- generate full HTML documentation\n"
"\n"
"The two latter options will create a  directory  in  the  current\n"
"working directory, and populate it with files  containing  manual\n"
"pages for all supported recipes. This might be handy for printing\n"
"or browsing the documentation. You can request the doc  in  ASCII\n"
"format (isaacp manual) or in  HTML  format  (isaacp manual html).\n"
"\n"
"OPTIONS\n"
"\n"
"All isaacp recipes share the following command-line  conventions:\n"
"\n"
"\n"
" * The first argument on the command-line  must  be  'isaacp'  of\n"
"   course. The  second  argument  has  to  be  the  recipe  name.\n"
"\n"
" * All recipes require at least one argument, which is the  input\n"
"   name of the file or files  to  process.  Some  recipes  accept\n"
"   several input files, some accept only one file.  Some  recipes\n"
"   are expecting FITS frames, some  are  expecting  ASCII  lists.\n"
"   Check out the recipe manual to see the kind of input expected.\n"
"\n"
" * Without any other option, all recipes  will  start  processing\n"
"   the input file(s) using default parameters. You can change the\n"
"   behaviour of a recipe by specifying  different  values.  These\n"
"   options are local to each recipe, check out the recipe  manual\n"
"   to see what the different options are.\n"
"\n"
" * Recipe-specific options can be placed anywhere on the  command\n"
"   line. The following are equivalent:\n"
"\n"
"   % isaacp recipe input -a alpha -b beta --flag\n"
"   % isaacp recipe -a alpha input -b beta --flag\n"
"   % isaacp recipe --flag -a alpha -b beta input\n"
"\n"
" * All recipes handle  short  and  long  options.  Short  options\n"
"   consist of adash (-) and one character  (a letter or a digit).\n"
"   All short options have a long option equivalent. Long  options\n"
"   consist of a double dash (--) and a  string.  Both  forms  are\n"
"   equivalent. The short option is easy to type, the long  option\n"
"   is usually preferred in scripts to enhance readability of  the\n"
"   command call.\n"
"\n"
" * You can always provide an output file  name  to  a  recipe  by\n"
"   using the -o/--output option. You are expected  to  provide  a\n"
"   base name which will be used by the recipe to name its  output\n"
"   files. An output file name looks like:\n"
"\n"
"   base_type.ext\n"
"\n"
"   where:\n"
"\n"
"   base is the base name you provide with -o/--output.\n"
"   type is a suffix indicating the file type.\n"
"   ext  is the file extension (fits, tfits, paf, ...).\n"
"\n"
"   For example, using the 'twflat' recipe with the option -o like\n"
"\n"
"   % isaacp twflat input -o twilight\n"
"\n"
"   will produce a flat-field called 'twilight_flat.fits'.  Notice\n"
"   that you can provide a base  name  containing  a  path,  like:\n"
"   /data/reduced/calib/twilight, in which case  the  output  name\n"
"   would be: /data/reduced/calib/twilight_flat.fits\n"
"   But you must then make sure that the path you give is suitable\n"
"   for writing.\n"
"\n"
" * If you do not use the -o/--output option, the recipe will make\n"
"   an output name for you, by using the input file name. The rule\n"
"   is to truncate any  path  in  the  input  file  name  and  any\n"
"   extension. So if your input file is  called  /data/raw/in.txt,\n"
"   the recipe will use 'in' as an output base  name.  This  means\n"
"   that all recipe products are  then  produced  in  the  current\n"
"   working directory. The following calls will all produce  files\n"
"   with the same name in the same directory (.):\n"
"\n"
"   isaacp recipe input\n"
"   isaacp recipe input.fits\n"
"   isaacp recipe /data/input\n"
"   isaacp recipe /data/raw/input.fits\n"
"\n"
" * It is usually impossible to predict the number of files  which\n"
"   will be produced by a given recipe. This is due  to  the  fact\n"
"   that the products naturally depend on the input  data.  To  be\n"
"   able to write scripts dealing with data produced by a pipeline\n"
"   recipe, it is usually a good idea to  force  the  output  name\n"
"   using the -o/--output option to a place where you can retrieve\n"
"   all files with a wildcard. Example:\n"
"\n"
"   % isaacp dark input -a -o ./tmp/result\n"
"\n"
"   The above command will e.g. produce files called:\n"
"\n"
"   ./tmp/result_001.fits\n"
"   ./tmp/result_002.fits\n"
"   ./tmp/result_003.fits\n"
"\n"
"   Using the  wildcard  ./tmp/result*  you can  group  all  files\n"
"   produced  by  the  recipe  in  the same command-line argument.\n"
"\n"
"-- \n"
"The ISAAC pipeline team\n";
