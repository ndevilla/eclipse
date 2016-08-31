/*----------------------------------------------------------------------------*/
/**
   @file	pixelmaps.c
   @author	Nicolas Devillard
   @date	Mar 03, 1997
   @version	$Revision: 1.34 $
   @brief	pixel map handling
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: pixelmaps.c,v 1.34 2003/04/24 14:33:28 yjung Exp $
	$Author: yjung $
	$Date: 2003/04/24 14:33:28 $
	$Revision: 1.34 $
*/

/*-----------------------------------------------------------------------------
  								Includes
 -----------------------------------------------------------------------------*/

#include "pixelmaps.h"

/*-----------------------------------------------------------------------------
  							Function codes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief	Allocate a new pixel map.
  @param	lx	Size in x.
  @param	ly	Size in y.
  @return	1 newly allocated pixelmap.

  Allocates space to hold a pixelmap and its pixel buffer. All pixels in
  the buffer are set to PIXELMAP_1.

  The returned object must be deallocated using pixelmap_del().
 */
/*----------------------------------------------------------------------------*/
pixelmap * pixelmap_new(int lx, int ly)
{
    pixelmap * p ;

    if ((lx<=0)||(lx>MAX_COLUMN_NUMBER)||(ly<=0)||(ly>MAX_LINE_NUMBER)) {
        e_error("cannot create pixel map with size [%dx%d]", lx, ly) ;
        return NULL ;
    }
    p = malloc(sizeof(pixelmap)) ;
    p->lx = lx ;
    p->ly = ly ;
    p->ngoodpix = lx * ly ;
    p->data = malloc(lx * ly * sizeof(binpix));
    /* Set all pixel values to 1 */
    memset(p->data, PIXELMAP_1, p->lx * p->ly);
    return p ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Get the number of PIXELMAP_1	
  @param	Pointer to a pixelmap struct.
  @return	int
 */
/*----------------------------------------------------------------------------*/
int pixelmap_getselected(pixelmap * map)
{
	int     nb_selected ;
    int     i ;
    
    /* Initialize */
    nb_selected = 0 ;
	if (map==NULL) return nb_selected ;

    for (i=0 ; i<(map->lx * map->ly) ; i++) {	
        if (map->data[i] == PIXELMAP_1) nb_selected++ ;
    }
    return nb_selected ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Get the size in bytes of a pixelmap structure in memory.
  @param	Pointer to a pixelmap struct.
  @return	int, size of the struct and associated fields in bytes

  This function computes approximately the size occupied by a pixelmap
  struct in memory. It takes into account both the size of the struct
  itself and the size of the associated memory zone (pixels).

  It is meant to be used with a garbage collector, to declare the size
  in bytes for future collection.
 */
/*----------------------------------------------------------------------------*/
int pixelmap_getbytesize(pixelmap * map)
{
	int size ;

	size = 0 ;
	if (map==NULL) return size ;
	size += sizeof(pixelmap);
	size += map->lx * map->ly * sizeof(binpix) ;
	return size ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Deallocates a pixelmap.
  @param	p	Pixel map to deallocate.
  @return	void

  Deallocates pixel buffer and structure associated to a pixel map.
 */
/*----------------------------------------------------------------------------*/
void pixelmap_del(pixelmap * p)
{
    if (p == NULL) return ;
    if (p->data != NULL) {
        free(p->data) ;
    }
    free(p) ;
    return ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief        Make a copy of a pixel map.
  @param        in      Original pixelmap.
  @return       1 newly allocated pixelmap.

  Make a copy of a pixelmap. Copies both the structure and the pixel array.
  The returned object must be deallocated using pixelmap_del().
 */
/*----------------------------------------------------------------------------*/
pixelmap * pixelmap_copy(pixelmap * in)
{
    pixelmap * out ;
    int 		pmsize;

    if (in == NULL) return NULL ;
    /* allocate output pixel map        */
    out = pixelmap_new(in->lx, in->ly) ;
    out->ngoodpix = in->ngoodpix;
    pmsize = in->lx * in->ly * sizeof(binpix);
    memcpy(out->data, in->data, pmsize);
    return out;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Threshold an image to a pixel map.
  @param	in		Image to threshold.
  @param	lo_cut	Lower bound for threshold.
  @param	hi_cut	Higher bound for threshold.
  @return	1 newly allocated pixelmap.

  Create a pixel map from an image. All pixels outside of the provided
  bounds will produce a PIXELMAP_0 in the output pixel map, all other
  pixels (i.e. within bounds) will produce a PIXELMAP_1 in the output map.

  The returned map must be deallocated using pixelmap_del().
 */
/*----------------------------------------------------------------------------*/
pixelmap * image_threshold2pixelmap(
    image_t    *	in,
    double       	lo_cut,
    double       	hi_cut)
{
    pixelmap    *	p ;
    int		     	i ;

	if (in==NULL) return NULL ;
    p = pixelmap_new(in->lx, in->ly) ;
    /* Loop on all pixels   */
    for (i=0 ; i<(p->lx * p->ly) ; i++) {
        if ((in->data[i]>lo_cut) && (in->data[i]<hi_cut)) {
            p->data[i] = PIXELMAP_1 ;
		} else {
			p->data[i] = PIXELMAP_0 ;
            p->ngoodpix -- ;
        }
    }           
    return p ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Update a pixel map with another.
  @param	p1	Pixel map to update.
  @param	p2 	Pixel map to consider to update p1.
  @return	void

  Updates the first pixel map with the second, i.e. all pixels set to zero
  in the second map are also set to zero in the first map.
 */
/*----------------------------------------------------------------------------*/
void pixelmap_update(pixelmap * p1, pixelmap * p2) 
{
    int	i ;

	if (p1==NULL || p2==NULL) return ;
	if ((p1->lx != p2->lx) || (p1->ly != p2->ly)) return ;
	if (p2->ngoodpix == (p2->lx * p2->ly)) return ;
    for (i=0 ; i<(p1->lx * p1->ly) ; i++) {
		if (p2->data[i] == PIXELMAP_0) {
			p1->data[i] = PIXELMAP_0 ;
			p1->ngoodpix -- ;
		}
	}
	return ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Dump a pixel map to disk in FITS format.
  @param	p			Pixel map to dump.
  @param	filename	Name of the output FITS file.
  @return	void

  Dumps a pixel map to disk as a FITS file in 8 bits/pel format. The header
  is minimal.
 */
/*----------------------------------------------------------------------------*/
void pixelmap_dump(pixelmap * p, char *	filename)
{
    FILE       	*	out ;
    qfits_header*	fh ;
    int		      	to_write,
					written ;
	char			cval[80];

	if (p==NULL || filename==NULL) return ;

    /* Create a simple FITS header  */
	fh = qfits_header_default();
	
	sprintf(cval, "%d", BPP_8_UNSIGNED);
	qfits_header_add(fh, "BITPIX", cval, "bits per pixel", NULL);
	sprintf(cval, "2");
	qfits_header_add(fh, "NAXIS", cval, "single image", NULL);
	sprintf(cval, "%d", p->lx) ;
	qfits_header_add(fh, "NAXIS1", cval, "x axis", NULL);
	sprintf(cval, "%d", p->ly) ;
	qfits_header_add(fh, "NAXIS2", cval, "y axis", NULL);

    /* Now output data  */
	if (!strcmp(filename, "STDOUT")) {
		out = stdout;
	} else {
		out = fopen(filename, "w") ;
	}
    if (out == NULL) {
        e_error("cannot append dump file [%s]", filename) ;
		qfits_header_destroy(fh);
        return ;
    }

	qfits_header_dump(fh, out);
	qfits_header_destroy(fh);

    to_write = p->lx * p->ly ; 
    written = fwrite(p->data, sizeof(binpix), to_write, out) ;
	if (out!=stdout)
		fclose(out) ;

    if (to_write != written) {
        e_error("cannot write data to disk (disk full?)") ;
        return ;
    }   
    qfits_zeropad(filename);
    return ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Load a pixel map from disk into memory.
  @param	filename	Name of the file to load.
  @return	1 newly allocated pixelmap.

  Loads a pixel map from a FITS file into a pixelmap object. A pixel map
  on disk can be any integer FITS image (BITPIX is 8, 16 or 32). Any
  non-zero pixel will be read as PIXELMAP_1, any zero pixel will be read as
  PIXELMAP_0.

  The returned map must be deallocated using pixelmap_del().
 */
/*----------------------------------------------------------------------------*/
pixelmap * pixelmap_load(char * filename)
{
	image_t	*	in ;
    pixelmap   *	loaded_map ;
	char		*	bitpix ;
	register pixelvalue	*	pix ;
    int		     	i ;
	int				ptype ;

    if (filename == NULL) return NULL ;
    
	/* Check out pixel type in input */
	bitpix = qfits_query_hdr(filename, "BITPIX");
	if (bitpix==NULL) {
		e_error("checking BITPIX for pixelmap [%s]: aborting load", filename);
		return NULL ;
	}
	ptype = atoi(bitpix);
	if ((ptype!=8) && (ptype!=16) && (ptype!=32)) {
		e_error("pixelmap [%s] has BITPIX=[%s], not an integer type",
				filename, bitpix);
		return NULL ;
	}
	
	/* Load as a normal image */
	in = image_load(filename);
	if (in==NULL) {
		e_error("cannot load [%s]", filename);
		return NULL ;
	}
    /* Allocate map     */
    loaded_map = pixelmap_new(in->lx, in->ly) ;
    /*
	 * Convert pixels to binary values, using the fact that all pixels
	 * are set to ONE in a default pixelmap.
	 */
	pix = in->data ;
    for (i=0 ; i<(in->lx * in->ly) ; i++) {
		if ((*pix)==0) {
			loaded_map->data[i] = PIXELMAP_0;
            loaded_map->ngoodpix -- ;
		}
		pix++;
    }
	image_del(in);
    return loaded_map ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Convert a pixelmap to an image object.
  @param	p	Pixel map to be promoted.
  @return	1 newly allocated image.

  Convert a pixel map to a valid image. The returned object is a newly
  allocated image which must be deallocated using image_del().

  PIXELMAP_0 is converted to (pixelvalue)0 and PIXELMAP_1 is converted to
  (pixelvalue)1.
 */
/*----------------------------------------------------------------------------*/
image_t * pixelmap_2_image(pixelmap * p)
{
    image_t * promoted ;
	int		   i ;

	if (p==NULL) return NULL ;
	promoted = image_new(p->lx, p->ly) ;
    for (i=0 ; i<(p->lx * p->ly) ; i++) {
        promoted->data[i] = (pixelvalue)p->data[i] ;
    }
    return promoted ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Update the ngoodpix field in a pixelmap structure.
  @param	p	Pixel map to update.
  @return	void

  Update the ngoodpix field in a pixelmap structure, by simply counting
  the number of pixels set to PIXELMAP_1.
 */
/*----------------------------------------------------------------------------*/
void pixelmap_updatecount(pixelmap * p)
{
    int	i ;

	if (p==NULL) return ;
	p->ngoodpix = p->lx * p->ly ;
    for (i=0 ; i<(p->lx * p->ly) ; i++){
		if (p->data[i]==PIXELMAP_0)
			p->ngoodpix -- ;
	}
	return ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Performs a binary AND between two pixel maps.
  @param	p1	First operand.
  @param	p2	Second operand.
  @return	int 0 if Ok, anything else otherwise.

  Modifies the first input pixel map to contain the result of the operation
  p1 AND p2.
 */
/*----------------------------------------------------------------------------*/
int pixelmap_binary_AND(pixelmap * p1, pixelmap * p2)
{
	int		i ;

	if ((p1==NULL) || (p2==NULL)) return -1 ;
	if ((p1->lx != p2->lx) || (p1->ly != p2->ly)) return -1 ;
	for (i=0 ; i<(p1->lx * p1->ly) ; i++) {
		p1->data[i] = (binpix)((int)p1->data[i] & (int)p2->data[i]);
	}
	return 0 ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Performs a binary OR between two pixel maps.
  @param	p1	First operand.
  @param	p2	Second operand.
  @return	int 0 if Ok, anything else otherwise.

  Modifies the first input pixel map to contain the result of the operation
  p1 OR p2.
 */
/*----------------------------------------------------------------------------*/
int pixelmap_binary_OR(pixelmap * p1, pixelmap * p2)
{
	int		i ;

	if ((p1==NULL) || (p2==NULL)) return -1 ;
	if ((p1->lx != p2->lx) || (p1->ly != p2->ly)) return -1 ;
	for (i=0 ; i<(p1->lx * p1->ly) ; i++) {
		p1->data[i] = (binpix)((int)p1->data[i] | (int)p2->data[i]);
	}
	return 0 ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Performs a binary XOR between two pixel maps.
  @param	p1	First operand.
  @param	p2	Second operand.
  @return	int 0 if Ok, anything else otherwise.

  Modifies the first input pixel map to contain the result of the operation
  p1 XOR p2.
 */
/*----------------------------------------------------------------------------*/
int pixelmap_binary_XOR(pixelmap * p1, pixelmap * p2)
{
	int		i ;

	if ((p1==NULL) || (p2==NULL)) return -1 ;
	if ((p1->lx!=p2->lx) || (p1->ly!=p2->ly)) return -1 ;
	for (i=0 ; i<(p1->lx * p1->ly) ; i++) {
		p1->data[i] = (binpix)((int)p1->data[i] ^ (int)p2->data[i]);
	}
	return 0 ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Performs a binary NOT on a pixel map.
  @param	p1	Pixel map to modify.
  @return	int 0 if Ok, anything else otherwise.

  Modifies the input pixel map to contain the result of the operation
  NOT p1.
 */
/*----------------------------------------------------------------------------*/
int pixelmap_binary_NOT(pixelmap * p1)
{
	int		i ;

	if (p1==NULL) return -1 ;
	for (i=0 ; i<(p1->lx*p1->ly) ; i++) {
		p1->data[i] = (binpix)(!(int)p1->data[i]);
	}
	return 0 ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Performs a morphological erosion with a 3x3 kernel.
  @param    in  Input pixel map.
  @return   int 0 if Ok, -1 otherwise

  This function performs a binary erosion and modifies the input pixel map
  to store the result of the operation.
 */
/*----------------------------------------------------------------------------*/
int pixelmap_morpho_erosion(pixelmap * in)
{
    pixelmap    *out ;
    int         i, j ;
    int         s0, s1, s2 ;
    int         pos ;

    /* error handling: test entries */
    if (in==NULL) return -1 ;

    /* allocate temporary output pixel map  */
    out = pixelmap_new(in->lx, in->ly) ;

    /* 
     * Initialize steps:
     *  -s2     -s1     -s0 
     *   -1       0       1
     *   s0      s1      s2
     */

    s0 = + in->lx - 1 ;
    s1 = + in->lx ;
    s2 = + in->lx + 1 ;

    /* Main filter loop */

    pos = 0 ;
    for (j=0 ; j<in->ly ; j++) {
        for (i=0 ; i<in->lx ; i++) {
            /* Get the neighbors */
            /* edges are not computed   */
            if ((i==0) || (i==in->lx-1) || (j==0) || (j==in->ly-1)) {
                out->data[pos] = PIXELMAP_0 ;
                out->ngoodpix -- ;
            } else if (
                    (in->data[pos-s2] == PIXELMAP_0) ||
                    (in->data[pos-s1] == PIXELMAP_0) ||
                    (in->data[pos-s0] == PIXELMAP_0) ||
                    (in->data[pos-1] == PIXELMAP_0) ||
                    (in->data[pos] == PIXELMAP_0) ||
                    (in->data[pos+1] == PIXELMAP_0) ||
                    (in->data[pos+s0] == PIXELMAP_0) ||
                    (in->data[pos+s1] == PIXELMAP_0) ||
                    (in->data[pos+s2] == PIXELMAP_0)) {
                out->data[pos] = PIXELMAP_0 ;
                out->ngoodpix -- ;
            } else {
                out->data[pos] = PIXELMAP_1 ;
            }
            pos ++ ;
        }
    }
    memcpy(in->data, out->data, in->lx * in->ly * sizeof(binpix));
    in->ngoodpix = out->ngoodpix ;
    pixelmap_del(out);
    return 0 ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Performs a morphological erosion with a user-defined kernel.
  @param    mi      Input pixel map.
  @param    mk      User-specified binary kernel.
  @return   int 0 if Ok, -1 else.
 
  This function performs a binary erosion using a user-defined kernel.
  The input pixel map is modified.
 */
/*----------------------------------------------------------------------------*/
int pixelmap_morpho_erosion_k(pixelmap * mi, pixelmap * mk)
{
    int		i, j, k, fsiz, neigh ;
    int		maxpos;
    int	*	s ;
    int		pos ;
	pixelmap	*	mo ;

    if (mi==NULL || mk==NULL) return -1 ;

	mo = pixelmap_copy(mi);
	
    /* define relative & relevant indexes for the filtering */
    fsiz = mk->lx * mk->ly;
    s = calloc(fsiz, sizeof(int)) ;

    for (j=0; j<mk->ly; j++){
        for (i=0; i<mk->lx; i++){
            if (mk->data[i+j*mk->lx]!=PIXELMAP_0){
                s[i+j*mk->lx] = (j-mk->ly/2)*mi->lx +i-mk->lx/2;
			} else {
                s[j*mk->lx+i]=0;
			}
        }
    }
    pos = 0 ;
    maxpos = mi->ly * mi->lx ;
    for (j=0; j<mi->ly; j++) {
        for (i=0; i<mi->lx; i++){
            if (mi->data[pos]==PIXELMAP_0) {
                for (k=0; k<fsiz; k++){
                    neigh = pos+s[k];
                    if (s[k] && (neigh>=0) && (neigh<maxpos) &&
                        mi->data[neigh] && mo->data[neigh]) {
                        mo->data[neigh] = PIXELMAP_0 ;
						mo->ngoodpix -- ;
                    }
                }
            }
            pos ++ ;
        }
    }
	free(s);
    memcpy(mi->data, mo->data, mi->lx * mi->ly * sizeof(binpix));
    mi->ngoodpix = mo->ngoodpix ;
    pixelmap_del(mo);
    return 0;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Performs a morphological dilation with a user-defined kernel.
  @param    mi      Input pixel map.
  @param    mk      User-specified binary kernel.
  @return   int 0 if Ok, -1 else.
 
  This function performs a binary dilation using a user-defined kernel.
  The input pixel map is modified.
 */
/*----------------------------------------------------------------------------*/
int pixelmap_morpho_dilation_k(pixelmap * mi, pixelmap * mk)
{
    int		i, j, k, fsiz, neigh ;
    int		maxpos;
    int	*	s ;
    int		pos ;
	pixelmap	*	mo ;

    if (mi==NULL || mk==NULL) return -1 ;

	mo = pixelmap_copy(mi);
	
    /* define relative & relevant indexes for the filtering */
    fsiz = mk->lx * mk->ly;
    s = calloc(fsiz, sizeof(int)) ;

    for (j=0; j<mk->ly; j++){
        for (i=0; i<mk->lx; i++){
            if (mk->data[i+j*mk->lx]!=PIXELMAP_0){
                s[i+j*mk->lx] = (j-mk->ly/2)*mi->lx +i-mk->lx/2;
			} else {
                s[j*mk->lx+i]=0;
			}
        }
    }
    pos = 0 ;
    maxpos = mi->ly * mi->lx ;
    for (j=0; j<mi->ly; j++) {
        for (i=0; i<mi->lx; i++){
            if (mi->data[pos]!=PIXELMAP_0) {
                for (k=0; k<fsiz; k++){
                    neigh = pos+s[k];
                    if (s[k] &&
						(neigh>=0) &&
						(neigh<maxpos) &&
                        (mi->data[neigh]==PIXELMAP_0) &&
						(mo->data[neigh]==PIXELMAP_0)) {
                        mo->data[neigh] = PIXELMAP_1 ;
						mo->ngoodpix ++ ;
					}
                }
            }
            pos ++ ;
        }
    }
	free(s);
    memcpy(mi->data, mo->data, mi->lx * mi->ly * sizeof(binpix));
    mi->ngoodpix = mo->ngoodpix ;
    pixelmap_del(mo);
    return 0;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Performs a morphological dilation with a 3x3 kernel.
  @param    in  Input pixel map.
  @return   int 0 if Ok, -1 otherwise

  This function performs a binary dilation and modifies the input pixel map
  to store the result of the operation.
 */
/*----------------------------------------------------------------------------*/
int pixelmap_morpho_dilation(pixelmap * in)
{
    pixelmap    *out ;
    int         i, j ;
    int         s0, s1, s2 ;
    int         pos ;

    /* error handling: test entries */
    if (in == NULL) return -1 ;

    /* allocate temporary output pixel map  */
    out = pixelmap_new(in->lx, in->ly) ;

    /* 
     * Initialize steps:
     *  -s2     -s1     -s0 
     *   -1       0       1
     *   s0      s1      s2
     */

    s0 = + in->lx - 1 ;
    s1 = + in->lx ;
    s2 = + in->lx + 1 ;

    /* Main filter loop */

    pos = 0 ;
    for (j=0 ; j<in->ly ; j++) {
        for (i=0 ; i<in->lx ; i++) {
            /* Get the neighbors */
            /* edges are not computed   */
            if ((i==0) || (i==in->lx-1) || (j==0) || (j==in->ly-1)) {
                out->data[pos] = PIXELMAP_0 ;
                out->ngoodpix -- ;
            } else  if (
                    (in->data[pos-s2] == PIXELMAP_1) ||
                    (in->data[pos-s1] == PIXELMAP_1) ||
                    (in->data[pos-s0] == PIXELMAP_1) ||
                    (in->data[pos-1] == PIXELMAP_1) ||
                    (in->data[pos] == PIXELMAP_1) ||
                    (in->data[pos+1] == PIXELMAP_1) ||
                    (in->data[pos+s0] == PIXELMAP_1) ||
                    (in->data[pos+s1] == PIXELMAP_1) ||
                    (in->data[pos+s2] == PIXELMAP_1)) {
                    out->data[pos] = PIXELMAP_1 ;
                } else {
                    out->data[pos] = PIXELMAP_0 ;
                    out->ngoodpix -- ;
                }
            pos++ ;
        }
    }
    memcpy(in->data, out->data, in->lx * in->ly * sizeof(binpix));
    in->ngoodpix = out->ngoodpix ;
    pixelmap_del(out);
    return 0 ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Perform a morphological closing with a 3x3 kernel.
  @param    in  Input pixel map.
  @return   int 0 if Ok, -1 otherwise.
 
  This function performs a binary morphological closing and modifies the
  input pixel map to store the result of the operation.
 */
/*----------------------------------------------------------------------------*/
int pixelmap_morpho_closing(pixelmap *in)
{
    if (in == NULL) return -1 ;
    if (pixelmap_morpho_erosion(in) != 0) {
        return -1 ;
    }
    if (pixelmap_morpho_dilation(in) != 0) {
        return -1 ;
    }
    return 0 ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Perform a morphological opening with a 3x3 kernel.
  @param    in  Input pixel map.
  @return   int 0 if Ok, -1 otherwise.
 
  This function performs a binary morphological opening and modifies the
  input pixel map to store the result of the operation.
 */
/*----------------------------------------------------------------------------*/
int pixelmap_morpho_opening(pixelmap *in)
{
    if (in == NULL) return -1 ;

    if (pixelmap_morpho_dilation(in) != 0) {
        return -1 ;
    }
    if (pixelmap_morpho_erosion(in) != 0) {
        return -1 ;
    }
    return 0 ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Extract a rectangular zone from an pixelmap into another pixelmap.
  @param    map_in      Input pixelmap
  @param    loleft_x    Lower left X coordinate
  @param    loleft_y    Lower left Y coordinate
  @param    upright_x   Upper right X coordinate
  @param    upright_y   Upper right Y coordinate
  @return   1 newly allocated pixelmap.
 
  The input coordinates define the extracted region by giving the
  coordinates of the lower left and upper right corners (inclusive).
 
  Coordinates must be provided in the FITS convention: lower left
  corner of the pixelmap is at (1,1), x growing from left to right,
  y growing from bottom to top.
 */
/*----------------------------------------------------------------------------*/
pixelmap * pixelmap_getvig(
        pixelmap    *   map_in,
        int             loleft_x,
        int             loleft_y,
        int             upright_x,
        int             upright_y)
{
    pixelmap    *   extr_pixmap ;
    register
    binpix      *   inpt,
                *   outpt ;
    int             outlx, 
                    outly ;
    int             i, j ;

    if (map_in==NULL) return NULL ;

    if ((loleft_x<1) || (loleft_x>map_in->lx) ||
        (loleft_y<1) || (loleft_y>map_in->ly) ||
        (upright_x<1) || (upright_x>map_in->lx) ||
        (upright_y<1) || (upright_y>map_in->ly) ||
        (loleft_x>upright_x) || (loleft_y>upright_y)) {
        e_error("extraction zone is [%d %d] [%d %d]\n"
                "cannot extract such zone: aborting slit extraction",
                loleft_x, loleft_y, upright_x, upright_y) ;
        return NULL ;
    }

    outlx = upright_x - loleft_x + 1 ;
    outly = upright_y - loleft_y + 1 ;
    extr_pixmap = pixelmap_new(outlx, outly) ;

    for (j=0 ; j<outly ; j++) {
        inpt = map_in->data+loleft_x-1 + (j+loleft_y-1)*map_in->lx ;
        outpt = extr_pixmap->data + j*outlx ;
        for (i=0 ; i<outlx ; i++) {
            *outpt++ = *inpt++ ;
        }
    }
    return extr_pixmap ;
}
