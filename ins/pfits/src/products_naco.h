
#ifndef _PRODUCTS_NACO_H_
#define _PRODUCTS_NACO_H_

/* Structure holding label/key pairs */
static prodlist_t prodlist_naco[] = {
{ procat_spec_slitpos_qc, "SLITPOS_QC", "Slit position" },
{ procat_spec_slitpos_table, "SLITPOS_TABLE", "Slit position" },
{ procat_imag_zpoint_qc, "ZPOINT_QC", "zero-point measurement" },
{ procat_imag_zpoint_result, "ZPOINT", "zero-point measurement" },

{ procat_imag_jitter_qc, "JITTER_QC", "jitter quality control level 1 parameters" },
{ procat_imag_sw_jitter_result, "COADDED_IMG", "jitter co-added image" },
{ procat_imag_sw_jitter_diff, "JITTER_DIFF", "jitter object - sky difference" },

{ procat_imag_illum, "ILLUM_FRAME", "flat-field llumination correction image" },
{ procat_imag_sw_flat_interce, "MASTER_IMG_FLAT_INTERC", "intercept frame from flat-field fit" },
{ procat_imag_sw_flat_errmap, "MASTER_IMG_FLAT_ERRMAP", "error map frame from flat-field fit" },
{ procat_imag_sw_flat_result, "MASTER_IMG_FLAT", "master flat-field frame" },
{ procat_imag_sw_flat_badpix, "MASTER_IMG_FLAT_BADPIX", "bad pixel map" },
{ procat_imag_lampflat_result, "MASTER_LAMP_FLAT", "master lamp flat-field" },
{ procat_imag_lampflat_qc, "LAMP_FLAT_QC", "lamp flat-field qc" },
{ procat_imag_bg, "SKY_BACKGROUND", "sky background measurements" },
{ procat_dark_ron, "MASTER_DARK_RON", "read-out noise measurements" },
{ procat_dark_result, "MASTER_DARK", "master dark frame" },
{ procat_dark_hot, "MASTER_DARK_HOT", "hot pixel map" },
{ procat_dark_dev, "MASTER_DARK_DEV", "deviant pixel map" },
{ procat_dark_cold, "MASTER_DARK_COLD", "cold pixel map" },
{ procat_focus, "QC_FOCUS", "Optimal focus for Quality Control" },
{ procat_qc_strehl, "QC_STREHL", "Strehl for Quality Control" },
{ procat_imag_detlin_coeff_Q, "DETLIN_Q", "image of detlin goodness of fit" },
{ procat_imag_detlin_coeff_A, "DETLIN_A", "image of detlin A coefficients" },
{ procat_imag_detlin_coeff_B, "DETLIN_B", "image of detlin B coefficients" },
{ procat_imag_detlin_coeff_C, "DETLIN_C", "image of detlin C coefficients" },
{ procat_imag_detlin_coeff_D, "DETLIN_D", "image of detlin D coefficients" },
{ procat_imag_detlin_limit, "DETLIN_LIMIT", "linearity limit image" },
{ procat_imag_detlin_bpm, "DETLIN_BPM", "bad pixels map image" },
{ procat_imag_detlin_QC, "DETLIN_QC", "QC parameters file for detlin" },

{ procat_end, NULL, NULL }
} ;


#endif
