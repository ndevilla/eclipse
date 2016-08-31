/*----------------------------------------------------------------------------*/
/**
   @file    poisson.c
   @author  N. Devillard
   @date    Feb 26 1998
   @version	$Revision: 1.6 $
   @brief   Random 2d point generator according to a Poisson distribution law
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: poisson.c,v 1.6 2002/11/22 11:39:20 yjung Exp $
	$Author: yjung $
	$Date: 2002/11/22 11:39:20 $
	$Revision: 1.6 $
 */

/*-----------------------------------------------------------------------------
                                Includes
 -----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/*-----------------------------------------------------------------------------
                               	Defines 
 -----------------------------------------------------------------------------*/

#ifndef M_SQRT1_2
#define M_SQRT1_2            0.70710678118654752440 
#endif
#define DEFAULT_XMIN        -75.0
#define DEFAULT_XMAX         75.0
#define DEFAULT_YMIN        -75.0
#define DEFAULT_YMAX         75.0
#define DEFAULT_MIN_NP       20
#define DEFAULT_HOMOG       -1
#define DEFAULT_FLOAT_FLAG   0

/* Support for Mac OS X, aka Darwin (BSD clone) */
#ifdef OS_DARWIN
#define srand48 srandom
#define drand48 random
#endif

/*-----------------------------------------------------------------------------
								New types
 -----------------------------------------------------------------------------*/

typedef struct _DPOINT_ {
    double  x ;
    double  y ;
} dpoint ;

typedef struct _RECTANGLE_ {
    double  xmin ;
    double  xmax ;
    double  ymin ;
    double  ymax ;
} rectangle ;

/*-----------------------------------------------------------------------------
							Function prototypes
 -----------------------------------------------------------------------------*/

static double   pdist(dpoint *p1, dpoint *p2) ;
static dpoint * generate_points(rectangle *r, int np, int homog) ;
static void     usage(char *pname) ;

/*-----------------------------------------------------------------------------
							Global definitions	
 -----------------------------------------------------------------------------*/

extern char * optarg ;
extern int    optind ;
static char bubulle[] =
"\t\to   _/,_\n\t\t . /o...\\__//\n\t\t   \\_'__/``\\`\n\t\t     \\`\n" ;

/*-----------------------------------------------------------------------------
 									Main
 -----------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
    int         np, i ;
    int         homog ;
    dpoint  *   list ;
    int         float_flag ;
    rectangle   r ;
    float       fl[4] ;
    int         c ;

    if (argc<2) {
        usage(argv[0]) ;
        return 0 ;
    }

    srand48((long)getpid()) ;

    float_flag  = DEFAULT_FLOAT_FLAG ;
    r.xmin      = DEFAULT_XMIN ;
    r.xmax      = DEFAULT_XMAX ;
    r.ymin      = DEFAULT_YMIN ;
    r.ymax      = DEFAULT_YMAX ;
    np          = DEFAULT_MIN_NP ;
    homog       = DEFAULT_HOMOG ;

    while ((c = getopt(argc, argv, "c:fh:n:r:")) != EOF)
        switch(c) {
            case 'f':
                /* create points with floating point coordinates */
                float_flag = 1 ;
                break ;

            case 'h':
                /* set homogeneity factor */
                homog = (int)atoi(optarg) ;
                break ;

            case 'n':
                /* set total number of points to generate */
                np = (int)atoi(optarg) ;
                if (np<1) {
                    printf("wrong number of points: cannot generate\n") ;
                    return -1 ;
                }
                break ;

            case 'r':
                /* set generation rectangle */
                sscanf(optarg, "%g %g %g %g", fl, fl+1, fl+2, fl+3) ;
                r.xmin = (double)fl[0] ;
                r.xmax = (double)fl[1] ;
                r.ymin = (double)fl[2] ;
                r.ymax = (double)fl[3] ;
                /* Check that the input rectangle is not silly */
                if ((r.xmin>r.xmax) || (r.ymin>r.ymax)) {
                    printf("wrong generation window: aborting\n") ; 
                    return -1 ;
                }
                break ;

            default:
                /* give program usage and exit */
                usage(argv[0]) ;
                return 1 ;
        }

    /* test homogeneity factor */
    if ((homog == -1) || (homog > np)) homog = np ;

    /* Generate the list */
    list = generate_points(&r, np, homog) ;

    /* Print out the generated points */
    for (i=0 ; i<np ; i++) {
        if (float_flag) fprintf(stdout, "%g %g\n", list[i].x, list[i].y) ;
        else fprintf(stdout, "%d %d\n", (int)(0.5+list[i].x), 
                (int)(0.5+list[i].y)) ;
    }
    free(list) ;
    return 0 ;
}

static double pdist(dpoint *p1, dpoint *p2)
{ return (p1->x-p2->x)*(p1->x-p2->x) + (p1->y-p2->y)*(p1->y-p2->y) ; }



/*----------------------------------------------------------------------------*/
/**
  @brief    Poisson points generation
  @param    r       Rectangle where the points are generated
  @param    np      Number of points to generate
  @param    homog   Homogeneity factor
  @return   The generated Poisson points

  Without homogeneity factor, the idea is to generate a set of np points within
  a given rectangle defined by (xmin xmax ymin ymax). All these points obey a 
  Poisson law, i.e. no couple of points is closer to each other than a minimal 
  distance. This minimal distance is defined as a function of the input 
  requested rectangle and the requested number of points to generate. We apply 
  the following formula: 
  dmin = sqrt( W * H / np * sqrt(2) )
  Where W and H stand for the rectangle width and height.
  This ensures a global homogeneity in the output point set.

  With a specified homogeneity factor h (2 < h <= np), the generation algorithm
  is different. The definition of h is: the Poisson law applies for any h 
  consecutive points in the final output, but not for the whole point set. 
  This enables us to generate groups of points which statisfy the Poisson law, 
  without constraining the whole set. This actually is equivalent to dividing 
  the rectangle in h regions of equal surface, and generate points randomly in 
  each of these region, changing region at each point.  
  It is possible to visualize this behaviour by plotting e.g.  
  poisson -h 9 -n 1000 (generates 1000 points in 9 patches)
 */
/*----------------------------------------------------------------------------*/
static dpoint * generate_points(rectangle * r, int np, int homog)
{
    double      min_dist ;
    int         gnp ;
    dpoint  *   list ;
    dpoint  *   seq_list ;
    dpoint      candidate ;
    int         ok ;
    int         i ;

    /* Test inputs */
    if (r == NULL) return NULL ;
    if (np<1) return NULL ;
    if ((homog<1) || (homog>np)) homog = np ;
    
    /* Initialize */
    list = (dpoint*)calloc(np, sizeof(dpoint)) ;
    min_dist=M_SQRT1_2*((r->xmax-r->xmin)*(r->ymax-r->ymin)/(double)(homog+1));
    gnp       = 1 ;
    list[0].x = 0 ;
    list[0].y = 0 ;

    /* First: generate <homog> points */
    while (gnp < homog) {

        /* Pick a random point within requested range */
        candidate.x = drand48() * (r->xmax - r->xmin) + r->xmin ;
        candidate.y = drand48() * (r->ymax - r->ymin) + r->ymin ;

        /* Check the candidate obeys the minimal Poisson distance */
        ok = 1 ;
        for (i=0 ; i<gnp ; i++) {
            if (pdist(&candidate, list+i) < min_dist) {
                /* does not check Poisson law: reject point */
                ok = 0 ;
                break ;
            }
        }
        if (ok) {
            /* obeys Poisson law: register the point as valid */
            list[gnp].x = candidate.x ;
            list[gnp].y = candidate.y ;
            gnp ++ ;
        }
    }

    /* Save the first <homog-1> points in the output array, then find a  */
    /* point to add which also respects the Poisson law, and iterate  */
    seq_list = list ;
    while (gnp < np) {

        /* Pick a random point within requested range */
        candidate.x = drand48() * (r->xmax - r->xmin) + r->xmin ;
        candidate.y = drand48() * (r->ymax - r->ymin) + r->ymin ;

        /* Check the candidate obeys the minimal Poisson distance */
        ok = 1 ;
        for (i=0 ; i<homog ; i++) {
            if (pdist(&candidate, seq_list+i) < min_dist) {
                /* does not check Poisson law: reject point */
                ok = 0 ;
                break ;
            }
        }
        if (ok) {
            /* obeys Poisson law: register the point as valid */
            list[gnp].x = candidate.x ;
            list[gnp].y = candidate.y ;
            gnp ++ ;
            seq_list ++ ;
        }
    }
    return list ;
}

static void usage(char * pname)
{
    printf("\n\n") ;
    printf("\t*** Random 2d Poisson point generator ***\n") ;
    printf("\tVersion from $Date: 2002/11/22 11:39:20 $\n") ;
    printf("\n%s\n", bubulle) ;
    printf("use: %s [options]\n", pname) ;
    printf("options are:\n") ;
    printf("\t[-r 'xmin xmax ymin ymax'] to define a rectangle\n") ;
    printf("\t[-f] to request floating point coordinates in output\n") ;
    printf("\t[-n <npoints>] to specify number of points to generate\n") ;
    printf("\t[-h <npoints>] to specify an homogeneity factor\n") ;
    printf("\n") ;
}
