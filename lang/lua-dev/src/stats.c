
/*-------------------------------------------------------------------------*/
/**
    stats -- Cube statistics

    Usage:
    % s = stats(cube)
    % s = stats(cube, badpixmap)
    % s = stats(cube, {xmin, xmax, ymin, ymax})
    % s = stats(cube, badpixmap, {xmin, xmax, ymin, ymax})
    % s = stats(cube, {xmin, xmax, ymin, ymax}, badpixmap)

    This function computes a number of statistical values on a cube and
    returns a table containing all results. Further arguments are
    optional, they specify the name of a possible bad pixel map to take
    into account for computation, and/or a table containing four integers
    specifying the zone where to compute statistics as:
    {xmin, xmax, ymin, ymax}.

    Coordinates are given using the FITS convention (x from 1 to lx, y
    from 1 to ly, x increasing from left to right, y increasing from bottom
    to top).

    The returned object is a table containing one table for each input
    plane in the cube. Each plane table contains the following double
    values:

    { plane, min, max, mean, median, rms, energy, flux }

    Notice that values are /named/ in the returned tables, i.e. they are
    not sorted by order. Example:

    s = stats(cube)
    print("median is", s[1].median)
    print("min is   ", s[1].min)
*/
/*-------------------------------------------------------------------------*/

int wrap_cube_stats(lua_State * L)
{
    cube_t      *   c_in ;
    image_stats *   stats ;
    int             p ;

    /* Initialize local variables */
    c_in = NULL ;

    /* First argument must be a cube */
    if (!lua_iscube(L, 1)) {
        e_error("stats expects a cube as first argument");
        return 0 ;
    }

    c_in = (cube_t*)lua_touserdata(L,1);

    /* Create a new table to hold results */
    lua_newtable(L);

    /* Compute stats for each plane */
    for (p=0 ; p<c_in->np ; p++) {
        lua_newtable(L);
        stats = image_getstats(c_in->plane[p]);
        if (stats==NULL) {
            e_error("stats: computing stats for plane %d");
            /* Pop two tables */
            lua_settop(L, -3);
            return 0 ;
        }

        /* Add plane */
        lua_pushstring(L, "plane");
        lua_pushnumber(L, (double)(p+1));
        lua_settable(L, -3);

        /* Add min */
        lua_pushstring(L, "min");
        lua_pushnumber(L, (double)stats->min_pix);
        lua_settable(L, -3);

        /* Add max */
        lua_pushstring(L, "max");
        lua_pushnumber(L, (double)stats->max_pix);
        lua_settable(L, -3);

        /* Add mean */
        lua_pushstring(L, "mean");
        lua_pushnumber(L, (double)stats->avg_pix);
        lua_settable(L, -3);

        /* Add median */
        lua_pushstring(L, "median");
        lua_pushnumber(L, (double)stats->median_pix);
        lua_settable(L, -3);

        /* Add rms */
        lua_pushstring(L, "rms");
        lua_pushnumber(L, (double)stats->stdev);
        lua_settable(L, -3);

        /* Add energy */
        lua_pushstring(L, "energy");
        lua_pushnumber(L, (double)stats->energy);
        lua_settable(L, -3);

        /* Add flux */
        lua_pushstring(L, "flux");
        lua_pushnumber(L, (double)stats->flux);
        lua_settable(L, -3);

        /* Add absflux */
        lua_pushstring(L, "absflux");
        lua_pushnumber(L, (double)stats->absflux);
        lua_settable(L, -3);

        /* Add table to general table */
        lua_rawseti(L, -2, p+1);

        free(stats);
    }

    return 1 ;
}

