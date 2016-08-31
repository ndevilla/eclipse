
const char isaac_ghost_version[] = "$Revision: 1.5 $";
const char isaac_ghost_date[]="$Date: 2002/12/10 10:15:33 $";
int  isaac_ghost_main(void * d);

cmdline_spec isaac_ghost_cmd[] = {
    {'f', "force", "Force ghost correction", 0, NULL},
    {0, 0, 0, 0, 0}
};

const char isaac_ghost_man[]=
"NAME\n"
"       ghost - ISAAC electrical ghost correction\n"
"\n"
"SYNOPSIS\n"
"       isaacp ghost [options] in\n"
"\n"
"DESCRIPTION\n"
"       The recipe applies simple arithmetic methods to remove the\n"
"       effects  of  an  electrical ghost  in  ISAAC  raw  frames.\n"
"\n"
"       To  avoid  applying  the  same  algorithm  several  times,\n"
"       is_ghost  leaves two keywords in the FITS header: GHOSTREM\n"
"       should be equal to 1, it is a  flag  indicating  that  the\n"
"       work  has  been  done  already  (it  is  called the 'ghost\n"
"       flag'), and GHOSTVER indicates the date when the algorithm\n"
"       has been latest modified.\n"
"\n"
"       If some files contain the ghost flag but must be processed\n"
"       anyway, you can force  it  by  using  the  -f  or  --force\n"
"       option.\n"
"\n"
"       All output files are written  in the current directory. It\n"
"       means that when you are running the command in the current\n"
"       directory,    previous    files   will   be   overwritten.\n"
"       If you do not want to overwrite your files, cd to  another\n"
"       directory,  and call  the recipe with  the pathname of the\n"
"       files you want to process. See the examples for more info.\n"
"\n"
"ALGORITHM\n"
"       The  algorithm  is the following: create a 1d signal which\n"
"       is for each element the sum of all pixels along a line  of\n"
"       the input image. Create another 1d signal that is mirrored\n"
"       from the first, or rather swapped: [1..512][513..1024]  is\n"
"       copied  to  [513..1024][1..512]. Add up these two signals,\n"
"       multiply by a constant (1.35e-5) and you get a  single  1d\n"
"       signal.  Every  element  in this signal is a value that is\n"
"       subtracted from all pixels belonging to the  corresponding\n"
"       line in the input image.\n"
"\n"
"OPTIONS\n"
"       -f or --force\n"
"              Force  deghosting  of  all given files, whether the\n"
"              ghost flag is found or not.\n"
"\n"
"EXAMPLES\n"
"       To correct for the ghost all files in the  current  direc-\n"
"       tory matching *.fits, you would type:\n"
"       isaacp ghost *.fits\n"
"\n"
"       To  correct  for the ghost all files in a remote directory\n"
"       called /cdrom/data/, and create the cleaned  copies  in  a\n"
"       directory called /scratch, you would type:\n"
"\n"
"       cd /scratch\n"
"      isaacp ghost /cdrom/data/*.fits\n"
"\n"
"\n";

