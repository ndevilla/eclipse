#ifndef _DO_CATG_NACO_H_
#define _DO_CATG_NACO_H_

/* Structure holding label/key pairs */
static docat_list_t docat_list_naco[] = {
{ docat_imag_flat, "IM_TWFLAT", "Twillight flat in imaging" },
{ docat_imag_dark, "IM_DARK", "Dark calibration data" },
{ docat_imag_badpix, "BAD_PIXEL_MAP", "BAd pixel map" },
{ docat_end, NULL, NULL }
} ;

#endif
