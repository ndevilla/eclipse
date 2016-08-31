#ifndef _DO_CATG_ISAAC_H_
#define _DO_CATG_ISAAC_H_

/* Structure holding label/key pairs */
static docat_list_t docat_list_isaac[] = {
{ docat_imag_flat,   "MASTER_IMG_FLAT",  "Twillight flat in imaging" },
{ docat_imag_dark,   "IM_DARK", "Dark calibration frame" },
{ docat_imag_badpix, "BAD_PIXEL_MAP", "Bad pixel map" },
{ docat_imag_detlin_coeff_A, "DETLIN_A", "Non-linearity coefficient A" },
{ docat_imag_detlin_coeff_B, "DETLIN_B", "Non-linearity coefficient B" },
{ docat_imag_detlin_coeff_C, "DETLIN_C", "Non-linearity coefficient C" },
{ docat_spec_arc, "SP_ARC", "Arc table" },
{ docat_spec_sttr, "SP_STARTRACE", "Startrace file" },
{ docat_spec_flat, "SP_FLAT", "Spectroscopic flat" },
{ docat_end, NULL, NULL }
} ;

#endif
