
/*---------------------------------------------------------------------------
   
   File name 	:	eclipse-lua.c
   Author 		:	N. Devillard
   Created on	:	Nov 2000
   Description	:	eclipse/lua interface

 *--------------------------------------------------------------------------*/

/*
	$Id: eclipse-lua.c,v 1.17 2001/12/03 12:23:45 ndevilla Exp $
	$Author: ndevilla $
	$Date: 2001/12/03 12:23:45 $
	$Revision: 1.17 $
*/

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include "eclipse.h"
#include "qfits.h"
#include "lua.h"

/*---------------------------------------------------------------------------
   								Defines
 ---------------------------------------------------------------------------*/

/* Comment out the following define to get rid of debugging code */
/* #define DEBUG_ECLIPSE_LUA */

/* Only defined in debug mode */
#ifdef DEBUG_ECLIPSE_LUA
#define debug_code( code )  { code }
#else
#define debug_code( code )
#endif


/*---------------------------------------------------------------------------
   								Macros
 ---------------------------------------------------------------------------*/

#define lua_iscube(L,i)		(lua_tag(L,i)==LUA_TCUBE)
#define lua_ispixmap(L,i)	(lua_tag(L,i)==LUA_TPIXMAP)

/*---------------------------------------------------------------------------
					Global variables, private to this module.
 ---------------------------------------------------------------------------*/

/* Tag defined for CUBE type in Lua */
static int LUA_TCUBE ;
/* Tag defined for PIXMAP type in Lua */
static int LUA_TPIXMAP ;



#include "src/cube.c"
#include "src/arith.c"
#include "src/filter.c"
#include "src/merge.c"
#include "src/collapse.c"
#include "src/pixmap.c"
#include "src/deadpix.c"
#include "src/stats.c"
#include "src/fits.c"
#include "src/framelist.c"


/*---------------------------------------------------------------------------
								Init function
 ---------------------------------------------------------------------------*/

/* Used to define names within a namespace */
#define lua_registermethod(L,s,f) \
	lua_pushstring(L,s);\
	lua_pushcfunction(L,f);\
	lua_rawset(L,1)

void lua_eclipselibopen(lua_State * L)
{

	/* Get new tags for eclipse types */
	LUA_TCUBE 	= lua_newtag(L);
	LUA_TPIXMAP = lua_newtag(L);

	/* Create a cube namespace (a table) */
	/*
	lua_newtable(L);
	lua_setglobal(L, "cube");
	lua_getglobal(L, "cube");
	*/

	/* Register cube methods */
	lua_register(L, "load", 	wrap_cube_load);
	lua_register(L, "save",		wrap_save_cube);
	lua_register(L, "del",		wrap_cube_del);

	lua_register(L, "add",		wrap_cube_add);
	lua_register(L, "sub",		wrap_cube_sub);
	lua_register(L, "mul",		wrap_cube_mul);
	lua_register(L, "div",		wrap_cube_div);
	
	lua_register(L, "filter", 	wrap_cube_filter);
	lua_register(L, "merge", 	wrap_cube_merge);
	lua_register(L, "collapse",	wrap_cube_collapse);

	lua_register(L, "load_pixmap",	wrap_load_pixmap);
	lua_register(L, "deadpix",	wrap_cube_deadpix);
	lua_register(L, "stats",	wrap_cube_stats);

	/* Declare destructors */
	lua_pushcfunction(L, wrap_cube_gc) ;
	lua_settagmethod(L, LUA_TCUBE, "gc");
	lua_pushcfunction(L, wrap_pixmap_gc) ;
	lua_settagmethod(L, LUA_TPIXMAP, "gc");

	/* Register FITS method */
	lua_register(L, "fits_get", wrap_fits_get);

	/* Register framelist method */
	lua_register(L, "framelist", wrap_framelist);

	return ;
}
