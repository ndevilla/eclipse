#ifndef _CN_FILTERS_STA_H_
#define _CN_FILTERS_STA_H_

/*
 * Structure holding label/name pairs
 *
 * Warning: filter names changed 01-Jul-2002,
 * both string and attached label. The previous name is indicated
 * on each line as a comment.
 *
 * This change is backwards-incompatible. Data generated with different
 * filter names will not be correctly identified.
 */
static struct {
	conica_filter_id	filtid ;
	char			 *	key ;
	double				central ;
	double				width ;
} conica_filter_list[] = {

	{conica_filter_j, 			"J",		1.265,	0.250}, /* a new one */
	{conica_filter_jc, 			"Jc",		1.265,	0.250}, /* J_redleak */
	{conica_filter_h, 			"H",		1.660,	0.330}, /* H */
	{conica_filter_k, 			"K",		2.230,	0.390}, /* K */
	{conica_filter_ks, 			"Ks",		2.180,	0.350}, /* Ks */
	{conica_filter_l, 			"L",		3.500,	0.610}, /* L */
	{conica_filter_lprime, 		"L_prime",	3.800,	0.620}, /* L_prime */
	{conica_filter_mprime, 		"M_prime",	4.780,	0.590}, /* M_prime */
	{conica_filter_sj, 			"SJ",		1.160,	0.470}, /* S1 */
	{conica_filter_sh, 			"SH",		1.630,	0.430}, /* S2 */
	{conica_filter_sk, 			"SK",		2.270,	0.760}, /* S3 */
	{conica_filter_nb104, 		"NB_1.04",	1.040,	0.015}, /* NB1040 */
	{conica_filter_nb108, 		"NB_1.08",	1.083,	0.015}, /* HeI */
	{conica_filter_nb109, 		"NB_1.09",	1.094,	0.015}, /* P_gamma */
	{conica_filter_nb124, 		"NB_1.24",	1.237,	0.015}, /* OII */
	{conica_filter_nb126, 	    "NB_1.26",	1.257,	0.014}, /* FeII1257 */
	{conica_filter_nb128, 		"NB_1.28",	1.282,	0.014}, /* P_beta */
	{conica_filter_nb164, 	    "NB_1.64",	1.644,	0.018}, /* FeII1644 */
	{conica_filter_nb175, 		"NB_1.75",  1.748,	0.026}, /* H2(1-0)S7 */
	{conica_filter_nb374, 	    "NB_3.74",	3.740,	0.020}, /* Pf_gamma */
	{conica_filter_ib200, 		"IB_2.00",	2.000,	0.060}, /* NB2000 */
	{conica_filter_ib203, 		"IB_2.03",	2.030,	0.060}, /* NB2030 */
	{conica_filter_ib206, 		"IB_2.06",	2.060,	0.060}, /* NB2060 */ 
	{conica_filter_ib209, 		"IB_2.09",	2.090,	0.060}, /* NB2090 */
	{conica_filter_ib212, 		"IB_2.12",	2.120,	0.060}, /* NB2120 */
	{conica_filter_nb212, 	    "NB_2.12",	2.122,	0.022}, /* NBH2(1-0)S1 */
	{conica_filter_ib215, 		"IB_2.15",	2.150,	0.060}, /* NB2150 */
	{conica_filter_nb217, 	    "NB_2.17",	2.166,	0.023}, /* Br_gamma */
	{conica_filter_ib218, 		"IB_2.18",	2.180,	0.060}, /* NB2180 */
	{conica_filter_ib221, 		"IB_2.21",	2.210,	0.060}, /* NB2210 */
	{conica_filter_ib224, 		"IB_2.24",	2.240,	0.060}, /* NB2240 */
	{conica_filter_ib227, 		"IB_2.27",	2.270,	0.060}, /* NB2270 */
	{conica_filter_ib230, 		"IB_2.30",	2.300,	0.060}, /* NB2300 */
	{conica_filter_ib233,		"IB_2.33",	2.330,	0.060}, /* NB2330 */
	{conica_filter_ib236,		"IB_2.36",	2.360,	0.060}, /* NB2360 */
	{conica_filter_ib239, 		"IB_2.39",	2.390,	0.060}, /* NB2390 */
	{conica_filter_ib242, 		"IB_2.42",	2.420,	0.060}, /* NB2420 */
	{conica_filter_ib245, 		"IB_2.45",	2.450,	0.060}, /* NB2450 */
	{conica_filter_ib248, 		"IB_2.48",	2.480,	0.060}, /* NB2480 */
	{conica_filter_nb405, 	    "NB_4.05",	4.051,	0.020}, /* Br_alpha */

	{conica_filter_end, "END", 0, 0}
};

/* Declare search functions */
conica_filter_id conica_get_filterid(char * key);

#endif
