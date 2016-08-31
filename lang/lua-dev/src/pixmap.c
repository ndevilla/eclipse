
/*-------------------------------------------------------------------------*/
/**
    load_pixmap -- Load a pixel map into memory

    Usage:
    % a = load_pixmap("filename")

    Use this function to load a pixel map. A pixel map is by definition
    a FITS image (NAXIS=2) containing binary pixels in integer format
    (BITPIX=8, 16 or 32). Any pixel value different from zero will be
    understood as 1, any zero value understood as 0.

    A pixel map is currently one single plane. This might be changed
    in the future if there is a need for a collection of pixel maps,
    to make them similar to cubes.
*/
/*-------------------------------------------------------------------------*/

int wrap_load_pixmap(lua_State * L)
{
    pixelmap    *   map ;

    /* Check input argument */
    if (!lua_isstring(L, 1)) {
        e_error("in arguments to load_pixmap()\n");
        return 0 ;
    }
    /* Activate GC */
    lua_setgcthreshold(L,1);

    e_comment(0, "loading %s\n", lua_tostring(L,1));
    map = pixelmap_load((char*)lua_tostring(L,1));
    if (map==NULL) {
        e_error("loading pixel map: aborting");
        return 0 ;
    }
    lua_pushusertag(L, (void*)map, LUA_TPIXMAP);
    lua_userdatasize(L, pixelmap_getbytesize(map));
    return 1 ;
}



/* internal: LUA garbage collector for pixel maps */
int wrap_pixmap_gc(lua_State * L)
{
    if (!lua_ispixmap(L,1)) {
        e_error("in arguments for pixmap GC");
        return 0 ;
    }

    debug_code(
        e_comment(0, "GC collecting pixmap");
    )

    pixelmap_del((pixelmap*)lua_touserdata(L,1));
    return 0 ;
}
