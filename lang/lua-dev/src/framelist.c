

/*-------------------------------------------------------------------------*/
/**

    framelist -- Read a framelist into a table of strings.

    Usage:
    tab = framelist("framelist.ascii")

    The returned table (tab) contains the following informations:

    n       Number of files found in the list.
    name    A table containing file names from 1 to n.
    type    A table containing file types from 1 to n.

    If there is no second column in the framelist file, there is no
    'type' table in the returned table. If some file types are empty
    in the input table, the 'type' table contains nil values.
 */
/*--------------------------------------------------------------------------*/


int wrap_framelist(lua_State * L)
{
    framelist * flist ;
    char      * name ;
    int         i ;

    if (lua_isstring(L,1)) {
        name = (char*)lua_tostring(L,1);
    } else {
        e_error("in framelist arguments: expecting a string");
        return 0 ;
    }

    /* Try to load the framelist */
    flist = framelist_load(name);
    if (flist==NULL) {
        e_error("cannot load framelist %s", name);
        return 0 ;
    }

    /* Create a new table, push it onto stack */
    lua_newtable(L);
    /* Assign n */
    lua_pushstring(L, "n");
    lua_pushnumber(L, (double)flist->n);
    lua_settable(L, -3);

    /* Create a new table for names */
    lua_pushstring(L, "name");
    lua_newtable(L);
    /* Push names onto stack */
    for (i=0 ; i<flist->n ; i++) {
        lua_pushstring(L, flist->name[i]);
        lua_rawseti(L, -2, i+1);
    }
    /* Push name table into result table */
    lua_settable(L, -3);

    /* If needed, create a new table for types */
    if (flist->type!=NULL) {
        lua_pushstring(L, "type");
        lua_newtable(L);
        /* Push types onto stack */
        for (i=0 ; i<flist->n ; i++) {
            if (flist->type[i]!=NULL) {
                lua_pushstring(L, flist->type[i]);
            } else {
                lua_pushnil(L);
            }
            lua_rawseti(L, -2, i+1);
        }
        /* Push type table into result table */
        lua_settable(L, -3);
    }
    framelist_del(flist);
    return 1 ;
}

