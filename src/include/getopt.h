#ifndef _GET_OPTIONS_H
#define _GET_OPTIONS_H

/* macros defined by this include file */
#define NO_ARG          0
#define REQUIRED_ARG    1
#define OPTIONAL_ARG    2

#define OPT_LICENSE 900
#define OPT_HELP	901
#define OPT_VERSION	902
#define OPT_INPUT	903
#define OPT_OUTPUT	904

#define getopt 			internal_getopt
#define getopt_long 	internal_getopt_long

#define optarg			internal_optarg
#define optind			internal_optind
#define opterr			internal_opterr
#define optopt			internal_optopt

struct option {
    char *name;   /* the name of the long option */
    int has_arg;  /* one of the above macros */
    int *flag;    /* determines if getopt_long() returns a
                   * value for a long option; if it is
                   * non-NULL, 0 is returned as a function
                   * value and the value of val is stored
                   * the area pointed to by flag.  Otherwise,
                   * val is returned. */
    int val;       /* determines the value to return if flag is NULL. */
} ;


#ifdef __cplusplus
extern "C" {
#endif

    /* externally-defined variables */
    extern char *	internal_optarg;
    extern int 		internal_optind;
    extern int 		internal_opterr;
    extern int 		internal_optopt;

    /* function prototypes */
    int internal_getopt(int argc, char **argv, char *optstring);
    int internal_getopt_long(int argc, char **argv, char *shortopts,
                     struct option * longopts, int *longind);

#ifdef __cplusplus
};
#endif

#endif


