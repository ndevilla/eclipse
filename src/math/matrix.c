/*-------------------------------------------------------------------------*/
/**
   @file	matrix.c
   @author	Nicolas Devillard
   @date	1994
   @version	$Revision: 1.10 $
   @brief	basic 2d matrix handling routines
*/
/*--------------------------------------------------------------------------*/

/*
	$Id: matrix.c,v 1.10 2001/11/13 13:33:46 yjung Exp $
	$Author: yjung $
	$Date: 2001/11/13 13:33:46 $
	$Revision: 1.10 $
*/

/*----------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include "matrix.h"
#include "comm.h"

/*----------------------------------------------------------------------------
   								Macros
 ---------------------------------------------------------------------------*/

#define dtiny(a) ((a)<0?(a)> -1.e-30:(a)<1.e-30)

/*----------------------------------------------------------------------------
						Private function prototypes
 ---------------------------------------------------------------------------*/

static int matrix_gausspiv(double *ptra, double *ptrc, int n);

/*----------------------------------------------------------------------------
  							Function codes
 ---------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/**
  @brief    Allocates a new matrix.
  @param    nr  Number of rows.
  @param    nc  Number of columns.
  @return   Pointer to newly allocated matrix.
 */
/*--------------------------------------------------------------------------*/
matrix * matrix_new(int nr, int nc)
{
	matrix	*	b;
	b = (matrix*)calloc(1, sizeof(matrix));
	b->m = (double*)calloc(nr*nc,sizeof(double));
	b->nr= nr;
	b->nc= nc;
	return b;
}


/*-------------------------------------------------------------------------*/
/**
  @brief    Copy a matrix.
  @param    a   matrix to copy.
  @return   Pointer to newly allocated matrix.
 */
/*--------------------------------------------------------------------------*/
matrix * matrix_copy(matrix * a)
{
    matrix * b = matrix_new(a->nr,a->nc);
    if (b!=NULL) {
        register int s = a->nr*a->nc;
        register double *mm = b->m+s;
        register double *am = a->m+s;
        while (s--) *--mm = *--am;
    }
    return b;
}


/*-------------------------------------------------------------------------*/
/**
  @brief    Frees memory associated to a matrix.
  @param    a   matrix to free.
  @return   void
 */
/*--------------------------------------------------------------------------*/
void matrix_del(matrix * a)
{
	if (a == NULL) return ;
	if (a->m != NULL) free(a->m) ;
	free(a) ;
	return ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief    Multiplies 2 matrices.
  @param    a   matrix on the left side of the multiplication.
  @param    b   matrix on the right side of the multiplication.
  @return   matrix a*b.
 */
/*--------------------------------------------------------------------------*/
matrix * matrix_mul(matrix * a, matrix * b)
{
	matrix			*	c, 
					*	d ;
	int 				n1, 
						n2, 
						n3 ;
	register double	*	a0;
	register double *	c0;
	register double *	d0;
	register int 		i,j,k;

	/* Initialize */
	n1 = a->nr ;
	n2 = a->nc ;
	n3 = b->nc ;

	
	if(n2 != b->nr) return NULL;
	c = matrix_new(n1,n3);
	d = matrix_transpose(b);

	for (i=0,c0=c->m ; i<n1 ; i++)
		for (j=0,d0=d->m ; j<n3 ; j++,c0++)
			for (k=0,*c0=0,a0=a->m+i*n2 ; k<n2 ; k++) *c0 += *a0++ * *d0++;
	matrix_del(d);
	return c;
}


/*-------------------------------------------------------------------------*/
/**
  @brief    Inverts a matrix.
  @param    aa  (Square) matrix to invert
  @return   Newly allocated matrix.
 
  The matrix inversion procedure is hardcoded for optimized speed in
  the case of 1x1, 2x2 and 3x3 matrices. This function is not suitable
  for large matrices.
 */
/*--------------------------------------------------------------------------*/
matrix * matrix_invert(matrix * aa)
{
    matrix		*	bb ;
	matrix		*	temp ;
    int 			test ;
	double 			det ;
	register double	ted ;
	double			a, b, c, d, e, f, g, h, i ;
	/* Initialize */
	test = 1 ;

    if(aa->nr!=aa->nc) return NULL ; 
    bb = matrix_new(aa->nr, aa->nc) ;

    if(aa->nr==1) {
        det= *(aa->m) ;
        if (dtiny(det)) test = 0 ;
        ted = 1. / det ;
        *(bb->m) = ted ;
    } else if (aa->nr == 2) {
        register double *mm=aa->m;
        a = *(mm++) ;
		b = *(mm++) ;
        c = *(mm++) ;
		d = *(mm) ;
        det = a*d - b*c ;
        if (dtiny(det)) test = 0 ;
        ted = 1. / det ;
        mm = bb->m ;
        *(mm++) = d*ted ;
		*(mm++) = -b*ted ;
        *(mm++) = -c*ted ;
		*(mm) = a*ted ;
    } else if (aa->nr == 3) {
        register double *mm=aa->m ;
        a = *(mm++) ;
		b = *(mm++) ;
		c = *(mm++) ;
        d = *(mm++) ;
		e = *(mm++) ;
		f = *(mm++) ;
        g = *(mm++) ;
		h = *(mm++) ;
		i = *(mm) ;
        det = a*e*i - a*h*f - b*d*i + b*g*f + c*d*h - c*g*e ;
        if (dtiny(det)) test = 0 ;
        ted = 1. /det ;
        mm = bb->m ;
        *(mm++) = (e*i - f*h) * ted ;
		*(mm++) = (c*h - b*i) * ted ;
		*(mm++) = (b*f - e*c) * ted ;
        *(mm++) = (f*g - d*i) * ted ;
		*(mm++) = (a*i - g*c) * ted ;
		*(mm++) = (d*c - a*f) * ted ;
        *(mm++) = (d*h - g*e) * ted ;
		*(mm++) = (g*b - a*h) * ted ;
		*(mm) = (a*e - d*b) * ted ;
    } else {
        temp=matrix_copy(aa);
        if(matrix_gausspiv(temp->m,bb->m,aa->nr)==0) test=0;
        matrix_del(temp);
    }
    if (test == 0) return NULL ;

    return bb ;
}



/*-------------------------------------------------------------------------*/
/**
  @brief    Transposes a matrix.
  @param    a   matrix to transpose.
  @return   Newly allocated matrix.
 */
/*--------------------------------------------------------------------------*/
matrix * matrix_transpose(matrix * a)
{
	register int		nc, 
						nr	;
	register double	*	a0 ;
	register double *	b0 ;
	register int 		i, j ;
	matrix 			*	b ;

	/* Initialize */
	nc = a->nc ;
	nr = a->nr ;
	b = matrix_new(nc, nr) ;
	if (b == NULL) return NULL ;

	for (i=0,b0=b->m ; i<nc ; i++)
		for (j=0,a0=a->m+i ; j<nr ; j++,a0+=nc,b0++) *b0 = *a0 ;
	return b ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief    Line simplification with Gauss method.
  @param    ptra    A matrix line.
  @param    ptrc    A matrix line.
  @param    n       Number of rows in each line.
  @return   int 1 if Ok, 0 else.
 
  This function is used only for the general case in matrix inversion.
 */
/*--------------------------------------------------------------------------*/
static int matrix_gausspiv(double *ptra, double *ptrc, int n)
/* c(n,n) = a(n,n)^-1 */
{
#define ABS(a) (((a) > 0) ? (a) : -(a))
	register int		i, j, k, l ;
	int 				maj ;
	double 				max, r, t ;  
	double 			*	ptrb ;

	ptrb = (double*)calloc(n*n, sizeof(double)) ;
	for (i=0 ; i<n ; i++) ptrb[i*n+i] = 1.0 ;

	for (i=1 ; i<=n ; i++) {
		/* Search max in current column  */
		max = ABS(*(ptra + n*i-n)) ;
		maj = i ;
		for (j=i ; j<=n ; j++)
			if (ABS(*(ptra+n*j+i-n-1)) > max) {
				maj = j ;
				max = ABS(*(ptra+n*j+i-n-1)) ;
			}

		/* swap lines i and maj */
		if (maj != i) {
			for (j = i;j <= n;j++) {
				r = *(ptra + n * maj + j - n - 1) ;
				*(ptra + n *maj + j - n -1) = *(ptra +n * i + j - n - 1) ;
				*(ptra + n * i + j - n - 1) = r ;
			}
			for(l=0 ; l<n ; l++) {
				r = *(ptrb + l * n + maj - 1) ;
				*(ptrb + l * n + maj - 1) = *(ptrb + l * n + i - 1) ;
				*(ptrb + l * n + i - 1) = r ;
			}
		}

		/* Subtract line by line */
		for (j=i+1 ; j<=n ; j++) {
			t = (*(ptra+(n+1)*i-n-1)) ;
			if (dtiny(t)) return 0 ;
			r = (*(ptra+n*j+i-n-1)) / t ;
			for (l=0 ; l<n ; l++) *(ptrb+l*n+j-1) -= r * (*(ptrb+l*n+i-1)) ;
			for (k=i ; k<=n ; k++) *(ptra+n*j+k-n-1) -= r * (*(ptra+n*i+k-n-1));
		}
	}

	/* Triangular system resolution	*/
	for(l=0 ; l<n ; l++)
		for (i=n ; i>=1 ; i--) {
			t = (*(ptra+(n+1)*i-n-1)) ;
			if (dtiny(t)) return 0 ;
			*(ptrc+l+(i-1)*n) = (*(ptrb+l*n+i-1)) / t ;
			if (i>1)
				for (j=i-1 ; j>0 ; j--)
					*(ptrb+l*n+j-1)-=(*(ptra+n*j+i-n-1))*(*(ptrc+l+(i-1)*n)) ;
		}
	free(ptrb) ;
	return 1 ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief    Compute the solution of an equation using a pseudo-inverse.
  @param    a   matrix.
  @param    b   matrix.
  @return   Pointer to newly allocated matrix.
 
  The equation is XA=B.
 
  The pseudo-inverse solution to this equation is defined as:
  \begin{verbatim}
  P = B.tA.inv(A.tA)
  \end{verbatim}
 
  P is solving the equation using a least-squares criterion.
  Demonstration left to the reader.
 */
/*--------------------------------------------------------------------------*/
matrix * matrix_leastsq(
		matrix	*	a,
		matrix	*	b)
{
	matrix	*	m1,
			*	m2,
			*	m3,
			*	m4,
			*	m5 ;

	m1 = matrix_transpose(a) ;
	m2 = matrix_mul(a, m1) ;
	m3 = matrix_invert(m2) ;
	if (m3 == NULL) return NULL ;
	m4 = matrix_mul(b, m1) ;
	m5 = matrix_mul(m4, m3) ;
	matrix_del(m1) ;
	matrix_del(m2) ;
	matrix_del(m3) ;
	matrix_del(m4) ;
	return m5 ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief    Prints out a matrix on stdout.
  @param    m       matrix to print out
  @param    name    Name of the matrix to print out.
  @return   void
 
  The matrix name is printed out, then all values row by row.
  Used for debugging purposes mostly.
 */
/*--------------------------------------------------------------------------*/
void matrix_dump(
		matrix	*	m,
		char 	*	name)
{
	int	i, j ;

	fprintf(stdout, "# matrix %s is [%d x %d]\n", name, m->nr, m->nc) ;
	for (j=0 ; j<m->nr ; j++) {
		for (i=0 ; i<m->nc ; i++) {
			fprintf(stdout, "%g\t", m->m[i+j*m->nc]) ;
		}
		fprintf(stdout, "\n") ;
	}
	fprintf(stdout, "\n") ;
}

