
#ifndef _FILTERS_STA_H_
#define _FILTERS_STA_H_

/* Structure holding label/name pairs */
static struct {
    isaac_filter_id   filtid ;
    char          	* key ;
} isaac_filter_list[] = {
	{isaac_filter_z,	    "Z"},
	{isaac_filter_sz,	    "SZ"},
	{isaac_filter_js,	    "Js"},
	{isaac_filter_j,	    "J"},
	{isaac_filter_jblock,    "J+Block"},
	{isaac_filter_sh,	    "SH"},
	{isaac_filter_h,	    "H"},
	{isaac_filter_ks,	    "Ks"},
	{isaac_filter_sk,	    "SK"},
	{isaac_filter_k,	    "K"},
	{isaac_filter_sl,	    "SL"},
	{isaac_filter_l,	    "L"},
	{isaac_filter_mnb,	    "M_NB"},
	{isaac_filter_m,	    "M"},
	{isaac_filter_nb106,	"NB_1.06"},
	{isaac_filter_nb108,	"NB_1.08"},
	{isaac_filter_nb119,	"NB_1.19"},
	{isaac_filter_nb121,	"NB_1.21"},
	{isaac_filter_nb126,	"NB_1.26"},
	{isaac_filter_nb128,	"NB_1.28"},
	{isaac_filter_nb164,	"NB_1.64"},
	{isaac_filter_nb171,	"NB_1.71"},
	{isaac_filter_nb207,	"NB_2.07"},
	{isaac_filter_nb209,	"NB_2.09"},
	{isaac_filter_nb213,	"NB_2.13"},
	{isaac_filter_nb217,	"NB_2.17"},
	{isaac_filter_nb219,	"NB_2.19"},
	{isaac_filter_nb225,	"NB_2.25"},
	{isaac_filter_nb229,	"NB_2.29"},
	{isaac_filter_nb234,	"NB_2.34"},
	{isaac_filter_nb321,	"NB_3.21"},
	{isaac_filter_nb328,	"NB_3.28"},
	{isaac_filter_nb380,	"NB_3.80"},
	{isaac_filter_nb407,	"NB_4.07"},
	{isaac_filter_end,		"END"}
} ;

/* Declare search functions */

isaac_filter_id isaac_get_filterid(char * key);

#endif
