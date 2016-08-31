#include "lua.h"

void lua_parseargs(lua_State * L, int argc, char * argv[])
{
    int     i ;
 
    /* Create a new table and push it onto the stack */
    lua_newtable(L);
 
    /* insert argc keys into table */
    for (i=0 ; i<argc ; i++) {
        lua_pushstring(L, argv[i]);
        lua_rawseti(L, 1, i);
    }

	/* insert number of keys into table */
	lua_pushstring(L, "n");
	lua_pushnumber(L, (double)argc);
	lua_settable(L, 1);
 
    /* Set table as global variable 'args' */
    lua_setglobal(L, "args");
    return ;
}
