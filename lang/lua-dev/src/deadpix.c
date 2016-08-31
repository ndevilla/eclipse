
/*-------------------------------------------------------------------------*/
/**
    deadpix -- dead pixel handling over a cube

    Usage:
    % deadpix(cube, badpixmap)

    This function is used to correct bad pixels in a cube according to a
    bad pixel map. The same pixel map is applied over each plane in the
    cube. See iproc/dead_pixels.c for details about the replacement
    implementation.
*/
/*-------------------------------------------------------------------------*/

int wrap_cube_deadpix(lua_State * L)
{
    cube_t      *   c_in ;
    pixelmap    *   badpixmap ;
    int             status ;

    if (!lua_iscube(L,1)) {
        e_error("deadpix expects a cube as first argument");
        return 0 ;
    }
    if (!lua_ispixmap(L,2)) {
        e_error("deadpix expects a pixelmap as second argument");
        return 0 ;
    }
    c_in = (cube_t*)lua_touserdata(L,1);
    badpixmap = (pixelmap*)lua_touserdata(L,2);

    status = cube_clean_deadpix(c_in, badpixmap);
    if (status!=0) {
        e_error("during cleaning: deadpix failed");
    }
    return 0 ;
}

