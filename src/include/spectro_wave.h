/*----------------------------------------------------------------------------*/
/**
   @file    spectro_wave.h
   @author  N.Devillard
   @date    October 1999
   @version $Revision: 1.22 $
   @brief   spectroscopy routines
*/
/*----------------------------------------------------------------------------*/

/*
    $Id: spectro_wave.h,v 1.22 2003/11/18 18:01:36 yjung Exp $
    $Author: yjung $
    $Date: 2003/11/18 18:01:36 $
    $Revision: 1.22 $
*/

#ifndef _SPECTRO_WAVE_H_
#define _SPECTRO_WAVE_H_

/*-----------------------------------------------------------------------------
   								Includes
 -----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include "eclipse.h"

/*-----------------------------------------------------------------------------
   								Defines
 -----------------------------------------------------------------------------*/

#define MIN_WAVELENGTH		0.0
#define MAX_WAVELENGTH		(1000000) /* 100 microns */

/*-----------------------------------------------------------------------------
   							structs
 -----------------------------------------------------------------------------*/

typedef struct _computed_disprel_ {
    double * poly;    /* List of solution polynomial coefficients */
    int      degree;  /* The degree of the polynomial (currently 3) */
    double   cc;      /* Cross-correlation number, -1 <= cc <= 1 */
    double   mean;    /* Mean line misaligment [pixel] of detected lines */
    double   rms;     /* RMS Mean line misaligment [pixel] of detected lines */
    double   offset;  /* Change in the constant term of the polynomial */
    double   scal1;   /* Scaling of 1st degree coefficient */
    double   scal2;   /* Scaling of 2nd degree coefficient */
    double   scal3;   /* Scaling of 3rd degree coefficient */
    int      clines;  /* Number of catalog lines used in the fit */
    int      dlines;  /* Number of catalog lines detected in the image */
} computed_disprel ;

/*-----------------------------------------------------------------------------
   							Function prototypes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief                  Compute a 3rd degree dispersion relation
  @param  in              Allocated spectroscopic image
  @param  discard_lo      Number of pixels to discard at the bottom
  @param  discard_hi      Number of pixels to discard at the top
  @param  discard_le      Number of pixels to discard on the left
  @param  discard_ri      Number of pixels to discard on the right
  @param  remove_thermal  Flag to force thermal background removal.
  @param  table_name      Spectral table name (see below)
  @param  slit_width      Width in pixels of the slit used
  @param  order           Order used in the spectral table look-up
  @param  phdisprel       4 polynomial coefficients as first guess (phys. mod.?)
  @return a computed_disprel object containing among others the solution poly.

  Compute a dispersion relation from a spectroscopic image showing some
  strong emission lines. A vital assumption is that strong emission lines
  can be seen in the image.

  The first guess polynomial may have degree 2. In that case the 3rd degree
  coefficient (4th value) must be zero.

  The removal of thermal background will attempt to remove from the
  spectroscopic image any low-frequency components, i.e. any features that
  are a lot wider than the slit width.

  The spectral table name is a character string. Possible values are:

  \begin{tabular}{ll}
  "oh"            &   OH lines \\
  "Xe"            &   Xenon lines \\
  "Ar"            &   Argon lines \\
  "Xe+Ar"         &   Xenon and Argon lines \\
  "/path/file"    &   Full pathname of an ASCII table
  \end{tabular}

  The latter specifies an ASCII table containing the lines you want to use for
  spectral calibration. Notice that this table must respect the format
  described in spectral_lines.c.

  The order determines at which location in the spectral table the look-up
  for emission lines will be done.

  The output cross-correlation factor is a number between 0 and 1. A good
  fit typically results in a value in the range 0.7 to 0.95, while a
  spectroscopic image without a number of well separated strong emission
  lines typically will yield a value down to about 0.5. Early spectroscopic
  images from the ESO ISAAC instrument cross-correlate down to 0.38.

  Algorithm:
  \begin{itemize}
  \item Spectrum extraction along the spectrum direction (horizontal).
  For each column the lower and higher
  pixels are discarded, then a median value of the remaining pixels in the
  column is returned. This forms a 1d signal of same size as the image in the
  spectrum direction.
  \item If a thermal background should be removed, this is done at this point.
  \item If some values must be discarded (set to zero) in the input spectrum,
  they are zeroed at that point.
  \item Spectral lines are retrieved from a catalog (internal).
  \item A search is done for the polynomial that maximizes the
  cross-correlation of the extracted signal and the spectral lines catalog.
  \end{itemize}

  Setting parts of the input spectrum to zero enables a correct wavelength
  calibration in the thermal regime. You should provide -1 and -1 if you want
  to let this function decide for you about a correct zeroing interval, (0,0)
  if you do not want to zero the spectrum at all, and any other values
  depending on which interval you want to set to zero on each side of the
  spectrum.
  Notice that setting these values to (10,20) will set to zero the 10 left
  pixels and 20 rightmost pixels of the input spectrum before throwing it into
  the cross-correlation procedure.
*/
/*----------------------------------------------------------------------------*/
/* <python> */
computed_disprel * spectro_compute_disprel(
        image_t *   in,
        int         discard_lo,
        int         discard_hi,
        int         discard_le,
        int         discard_ri,
        int         remove_thermal,
        char    *   table_name,
        double      slit_width,
        int         order,
        double  *   phdisprel) ;
/* </python> */

#endif
