

/*-------------------------------------------------------------------------*/
/**

    fits_get -- FITS header interface in Lua.

    Usage:
    val = fits_get("a.fits", "naxis", "det.dit")
    val = fits_get(a, "naxis", "det.dit")

 */
/*--------------------------------------------------------------------------*/


int wrap_fits_get(lua_State * L)
{
    char    *   name ;
    int         i ;
    int         nk ;
    char    **  key ;
    char    **  val ;
    char    *   sval ;

    if (lua_isstring(L,1)) {
        name = (char*)lua_tostring(L,1);
    } else {
        e_error("in fits_get arguments: expecting a cube name");
        return 0 ;
    }

    /* Retrieve number of string arguments */
    nk = lua_gettop(L)-1 ;
    if (nk<1) {
        e_error("in fits_get arguments: expecting FITS keywords");
        return 0 ;
    }
    key = malloc(nk * sizeof(char*));
    for (i=0 ; i<nk ; i++) {
        key[i] = (char*)lua_tostring(L, i+2);
    }

    /* Retrieve FITS values */
    val = malloc(nk * sizeof(char*));
    for (i=0 ; i<nk ; i++) {
        sval = qfits_query_hdr(name, key[i]);
        if (sval==NULL) {
            val[i]=NULL ;
        } else {
            val[i]=strdup(sval);
        }
    }

    /* Create a new table to contain the results */
    lua_newtable(L);
    for (i=0 ; i<nk ; i++) {
        /* Push values on the table */
        lua_pushstring(L, key[i]);
        if (val[i]!=NULL) {
            lua_pushstring(L, val[i]);
            free(val[i]);
        } else {
            lua_pushnil(L);
        }
        lua_settable(L, -3);
    }
    free(val);
    free(key);
    return 1 ;
}

