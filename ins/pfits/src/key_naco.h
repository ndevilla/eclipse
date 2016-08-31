/*-------------------------------------------------------------------------*/
/**
   @file	key_naco.c
   @author	Y. Jung
   @date	March 2002
   @version	$Revision: 1.12 $
   @brief	NACO functions using FITS header keywords
*/
/*--------------------------------------------------------------------------*/

/*
	$Id: key_naco.h,v 1.12 2004/01/07 16:37:11 yjung Exp $
	$Author: yjung $
	$Date: 2004/01/07 16:37:11 $
	$Revision: 1.12 $
*/

#ifndef _KEY_NACO_H_
#define _KEY_NACO_H_

#include "keyfits.h"

char * naco_get_absrot_start(char * filename) ;
char * naco_get_airmass_start(char * filename) ;
char * naco_get_airmass_end(char * filename) ;
char * naco_get_arcfile(char * filename) ;
char * naco_get_cumoffsetx(char * filename) ;
char * naco_get_cumoffsety(char * filename) ;
char * naco_get_date_obs(char * filename) ;
char * naco_get_dec(char * filename) ;
char * naco_get_dich_posname(char * filename) ;
char * naco_get_dit(char * filename) ;
char * naco_get_dpr_catg(char * filename) ;
char * naco_get_dpr_tech(char * filename) ;
char * naco_get_dpr_type(char * filename) ;
char * naco_get_ecmean(char * filename) ;
char * naco_get_expno(char * filename) ;
char * naco_get_exptime(char * filename) ;
char * naco_get_filter(char * filename) ;
char * naco_get_fluxmean(char * filename) ;
char * naco_get_focus(char * filename) ;
char * naco_get_frame_type(char * filename) ;
char * naco_get_humidity_level(char * filename) ;
char * naco_get_instrument(char * filename) ;
char * naco_get_l0mean(char * filename) ;
char * naco_get_lamp2_cur(char * filename) ;
char * naco_get_lamp2_name(char * filename) ;
char * naco_get_lamp2(char * filename) ;
char * naco_get_lamp2_type(char * filename) ;
char * naco_get_mjdobs(char * filename) ;
char * naco_get_mode(char * filename) ;
char * naco_get_ndit(char * filename) ;
char * naco_get_ndsamples(char * filename) ;
char * naco_get_numbexp(char * filename) ;
char * naco_get_obs_id(char * filename) ;
char * naco_get_opti1_id(char * filename) ;
char * naco_get_opti3_id(char * filename) ;
char * naco_get_opti7_id(char * filename) ;
char * naco_get_opti4_id(char * filename) ;
char * naco_get_opti3_name(char * filename) ;
char * naco_get_opti7_name(char * filename) ;
char * naco_get_opti7_no(char * filename) ;
char * naco_get_pixscale(char * filename) ;
char * naco_get_r0mean(char * filename) ;
char * naco_get_ra(char * filename) ;
char * naco_get_refzerox(char * filename) ;
char * naco_get_refzeroy(char * filename) ;
char * naco_get_rom(char * filename) ;
char * naco_get_rom_name(char * filename) ;
char * naco_get_t0mean(char * filename) ;
char * naco_get_templateid(char * filename) ;
char * naco_get_wfs_mode(char * filename) ;
char * naco_get_wfs_type(char * filename) ;

static keyfits keylist_naco[] = {
    {"absrot_start",   0, naco_get_absrot_start},
    {"airmass_start",  0, naco_get_airmass_start},
    {"airmass_end",    0, naco_get_airmass_end},
    {"arcfile",        0, naco_get_arcfile},
    {"cumoffsetx",     0, naco_get_cumoffsetx},
    {"cumoffsety",     0, naco_get_cumoffsety},
    {"date_obs",       0, naco_get_date_obs},
    {"dec",            0, naco_get_dec},
    {"dich_posname",   0, naco_get_dich_posname},
    {"dit",            0, naco_get_dit},
    {"dpr_catg",       0, naco_get_dpr_catg},
    {"dpr_tech",       0, naco_get_dpr_tech},
    {"dpr_type",       0, naco_get_dpr_type},
    {"ecmean",         0, naco_get_ecmean},
    {"expno",          0, naco_get_expno},
    {"exptime",        0, naco_get_exptime},
    {"filter",         0, naco_get_filter},
    {"fluxmean",       0, naco_get_fluxmean},
    {"focus",          0, naco_get_focus},
    {"frame_type",     0, naco_get_frame_type},
    {"humidity_level", 0, naco_get_humidity_level},
    {"instrument",     0, naco_get_instrument},
    {"l0mean",         0, naco_get_l0mean},
    {"lamp2_cur",      0, naco_get_lamp2_cur},
    {"lamp2_name",     0, naco_get_lamp2_name},
    {"lamp2",          0, naco_get_lamp2},
    {"lamp2_type",     0, naco_get_lamp2_type},
    {"mjdobs",         0, naco_get_mjdobs},
    {"mode",           0, naco_get_mode},
    {"ndit",           0, naco_get_ndit},
    {"ndsamples",      0, naco_get_ndsamples},
    {"numbexp",        0, naco_get_numbexp},
    {"obs_id",         0, naco_get_obs_id},
    {"opti1_id",       0, naco_get_opti1_id},
    {"opti3_id",       0, naco_get_opti3_id},
    {"opti7_id",       0, naco_get_opti7_id},
    {"opti4_id",       0, naco_get_opti4_id},
    {"opti3_name",     0, naco_get_opti3_name},
    {"opti7_name",     0, naco_get_opti7_name},
    {"opti7_no",       0, naco_get_opti7_no},
    {"pixscale",       0, naco_get_pixscale},
    {"r0mean",         0, naco_get_r0mean},
    {"ra",             0, naco_get_ra},
    {"refzerox",       0, naco_get_refzerox},
    {"refzeroy",       0, naco_get_refzeroy},
    {"rom",            0, naco_get_rom},
    {"rom_name",       0, naco_get_rom_name},
    {"t0mean",         0, naco_get_t0mean},
    {"templateid",     0, naco_get_templateid},
    {"wfs_mode",       0, naco_get_wfs_mode},
    {"wfs_type",       0, naco_get_wfs_type},
    
    {0, 0, 0}
};

#endif
