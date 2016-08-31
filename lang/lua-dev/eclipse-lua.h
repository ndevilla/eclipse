
/*---------------------------------------------------------------------------
   
   File name 	:	eclipse-lua.h
   Author 		:	N. Devillard
   Created on	:	Nov 2000
   Description	:	eclipse/lua interfacing.

 *--------------------------------------------------------------------------*/

/*
	$Id: eclipse-lua.h,v 1.2 2001/03/30 13:33:31 ndevilla Exp $
	$Author: ndevilla $
	$Date: 2001/03/30 13:33:31 $
	$Revision: 1.2 $
*/

#ifndef _ECLIPSE_LUA_H_
#define _ECLIPSE_LUA_H_

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include "eclipse.h"
#include "lua.h"

void lua_eclipselibopen(lua_State * L);

#endif
