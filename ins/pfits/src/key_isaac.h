/*-------------------------------------------------------------------------*/
/**
   @file	key_isaac.c
   @author	Y. Jung
   @date	July 2000
   @version	$Revision: 1.9 $
   @brief	ISAAC functions using FITS header keywords
*/
/*--------------------------------------------------------------------------*/

/*
	$Id: key_isaac.h,v 1.9 2004/02/09 09:19:14 yjung Exp $
	$Author: yjung $
	$Date: 2004/02/09 09:19:14 $
	$Revision: 1.9 $
*/

#ifndef _KEY_ISAAC_H_
#define _KEY_ISAAC_H_

#include "keyfits.h"

char * isaac_get_airmass_start(char * filename) ;
char * isaac_get_airmass_end(char * filename) ;
char * isaac_get_arcfile(char * filename) ;
char * isaac_get_arm(char * filename) ;
char * isaac_get_chip(char * filename) ;
char * isaac_get_chopping_cycle(char * filename) ;
char * isaac_get_chopping_frequency(char * filename) ;
char * isaac_get_chopping_status(char * filename) ;
char * isaac_get_chopping_throw(char * filename) ;
char * isaac_get_cumoffsetx(char * filename) ;
char * isaac_get_cumoffsety(char * filename) ;
char * isaac_get_current_exp_nb(char * filename) ;
char * isaac_get_date(char * filename) ;
char * isaac_get_date_obs(char * filename) ;
char * isaac_get_dec(char * filename) ;
char * isaac_get_detector_frame_type(char * filename) ;
char * isaac_get_detector_readout_mode(char * filename) ;
char * isaac_get_dit(char * filename) ;
char * isaac_get_dpr_catg(char * filename) ;
char * isaac_get_dpr_tech(char * filename) ;
char * isaac_get_dpr_type(char * filename) ;
char * isaac_get_filter(char * filename) ;
char * isaac_get_filter_lw(char * filename) ;
char * isaac_get_filter_sw(char * filename) ;
char * isaac_get_hist_disp1(char * filename) ;
char * isaac_get_hist_disp2(char * filename) ;
char * isaac_get_hist_disp3(char * filename) ;
char * isaac_get_hist_disp4(char * filename) ;
char * isaac_get_humidity_level(char * filename) ;
char * isaac_get_instrument(char * filename) ;
char * isaac_get_lamp1_name(char * filename) ;
char * isaac_get_lamp1_status(char * filename) ;
char * isaac_get_lamp2_name(char * filename) ;
char * isaac_get_lamp2_status(char * filename) ;
char * isaac_get_lamp3_intensity(char * filename) ;
char * isaac_get_mjdobs(char * filename) ;
char * isaac_get_mode(char * filename) ;
char * isaac_get_ndit(char * filename) ;
char * isaac_get_ndsamples(char * filename) ;
char * isaac_get_numbexp(char * filename) ;
char * isaac_get_objective(char * filename) ;
char * isaac_get_obs_id(char * filename) ;
char * isaac_get_obs_targ_name(char * filename) ;
char * isaac_get_optical_id(char * filename) ;
char * isaac_get_order(char * filename) ;
char * isaac_get_origfile(char * filename) ;
char * isaac_get_pixscale(char * filename) ;
char * isaac_get_pro_catalog(char * filename) ;
char * isaac_get_pro_catg(char * filename) ;
char * isaac_get_ra(char * filename) ;
char * isaac_get_resolution(char * filename) ;
char * isaac_get_romode_id(char * filename) ;
char * isaac_get_romode_name(char * filename) ;
char * isaac_get_romode_name2(char * filename) ;
char * isaac_get_rspeed(char * filename) ;
char * isaac_get_templateid(char * filename) ;

static keyfits keylist_isaac[] = {
    {"airmass_start",         0, isaac_get_airmass_start},
    {"airmass_end",           0, isaac_get_airmass_end},
    {"arcfile",               0, isaac_get_arcfile},
    {"arm",                   0, isaac_get_arm},
    {"chip",                  0, isaac_get_chip},
    {"chopping_cycle",        0, isaac_get_chopping_cycle},
    {"chopping_frequency",    0, isaac_get_chopping_frequency},
    {"chopping_status",       0, isaac_get_chopping_status},
    {"chopping_throw",        0, isaac_get_chopping_throw},
    {"cumoffsetx",            0, isaac_get_cumoffsetx},
    {"cumoffsety",            0, isaac_get_cumoffsety},
    {"current_exp_nb",        0, isaac_get_current_exp_nb},
    {"date",                  0, isaac_get_date},
    {"date_obs",              0, isaac_get_date_obs},
    {"dec",                   0, isaac_get_dec},
    {"detector_frame_type",   0, isaac_get_detector_frame_type},
    {"detector_readout_mode", 0, isaac_get_detector_readout_mode},
    {"dit",                   0, isaac_get_dit},
    {"dpr_catg",              0, isaac_get_dpr_catg},
    {"dpr_tech",              0, isaac_get_dpr_tech},
    {"dpr_type",              0, isaac_get_dpr_type}, 
    {"filter",                0, isaac_get_filter},
    {"filter_lw",             0, isaac_get_filter_lw},
    {"filter_sw",             0, isaac_get_filter_sw},
    {"hist_disp1",            0, isaac_get_hist_disp1},
    {"hist_disp2",            0, isaac_get_hist_disp2},
    {"hist_disp3",            0, isaac_get_hist_disp3},
    {"hist_disp4",            0, isaac_get_hist_disp4},
    {"humidity_level",        0, isaac_get_humidity_level},
    {"instrument",            0, isaac_get_instrument},
    {"lamp1_name",            0, isaac_get_lamp1_name},
    {"lamp1_status",          0, isaac_get_lamp1_status},
    {"lamp2_name",            0, isaac_get_lamp2_name},
    {"lamp2_status",          0, isaac_get_lamp2_status},
    {"lamp3_intensity",       0, isaac_get_lamp3_intensity},
    {"mjdobs",                0, isaac_get_mjdobs},
    {"mode",                  0, isaac_get_mode},
    {"ndit",                  0, isaac_get_ndit},
    {"ndsamples",             0, isaac_get_ndsamples},
    {"numbexp",               0, isaac_get_numbexp},
    {"objective",             0, isaac_get_objective},
    {"obs_id",                0, isaac_get_obs_id},
    {"obs_targ_name",         0, isaac_get_obs_targ_name},
    {"optical_id",            0, isaac_get_optical_id},
    {"order",                 0, isaac_get_order},
    {"origfile",              0, isaac_get_origfile},
    {"pixscale",              0, isaac_get_pixscale},
    {"pro_catalog",           0, isaac_get_pro_catalog},
    {"pro_catg",              0, isaac_get_pro_catg},
    {"ra",                    0, isaac_get_ra},
    {"resolution",            0, isaac_get_resolution},
    {"romode_id",             0, isaac_get_romode_id},
    {"romode_name",           0, isaac_get_romode_name},
    {"romode_name2",          0, isaac_get_romode_name2},
    {"rspeed",                0, isaac_get_rspeed},
    {"templateid",            0, isaac_get_templateid},
    
    {0, 0, 0}
};

#endif
