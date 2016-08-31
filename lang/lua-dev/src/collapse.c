
/*-------------------------------------------------------------------------*/
/**
    collapse -- collapse a cube over X or Y.

    Usage:
    % a = collapse(b)

    % params = {
                    dir="vertical",     -- "vertical" or "horizontal"
                    median=1,           -- optional median filtering
                    reject= {           -- rejection parameters
                        min=3,          -- min values to reject
                        max=3           -- max values to reject
                    }
                }
    % a = collapse(b, params)

    This function is mostly used to collapse images, although it will also
    work on cubes by applying the collapse to every plane in the input
    cube. A parameter table is expected (otherwise the function runs with
    default values). This table should contain the following fields:

    dir
        is a string containing either "vertical" or "horizontal". This
        indicates the direction for collapsing. Default is "vertical".
    
    median
        is an integer. If non-zero, only the median value of each row or
        column will be kept as result of the collapse, with possible
        rejection. Default is 0 (no median, thus no rejection).

    reject
        is a table, expected to contain two fields:
        min     is the number of min values to reject.
        max     is the number of max values to reject.
        The reject option will only be considered if median is activated
        (non-zero). The min lowest and max highest pixel values on each row
        or column are rejected before computing the median and keeping it
        as the result of the collapse.
 */
/*--------------------------------------------------------------------------*/

int wrap_cube_collapse(lua_State * L)
{
    cube_t   *  c_in ;
    cube_t   *  collapsed ;
    image_t *   im ;
    int         i ;
    int         direction ;
    int         median ;
    int         rejlo, rejhi ;
    char    *   sval ;

    direction = 0 ; /* y is the default */
    median    = 0 ; /* No median collapse by default */
    rejlo = 0 ;     /* No rejection by default */
    rejhi = 0 ;

    /* Expected arguments: 1 cube and a table of parameters */
    if (!lua_iscube(L,1)) {
        e_error("collapse expects a cube as first argument");
        return 0 ;
    }
    c_in = (cube_t*)lua_touserdata(L,1);

    if (lua_istable(L,2)) {
        /* Gather parameters from table */
        /* Retrieve collapse direction */
        lua_pushstring(L, "dir");
        lua_gettable(L,2);
        if (lua_isstring(L,-1)) {
            sval = (char*)lua_tostring(L,-1);
            if (!strcmp(sval, "vertical")) {
                printf("dir = vertical\n");
                direction = 0 ;
            } else if (!strcmp(sval, "horizontal")) {
                printf("dir = horizontal\n");
                direction = 1 ;
            } else {
                e_error("in collapse parameters");
                e_error("'dir' must be 'vertical' or 'horizontal'");
                return 0 ;
            }
        }
        lua_pop(L,1);

        /* Retrieve median flag */
        lua_pushstring(L, "median");
        lua_gettable(L, 2);
        if (lua_isnumber(L,-1)) {
            median = (int)lua_tonumber(L,-1);   
            if (median!=0) {
                median = 1 ;
            }
            printf("median = %d\n", median);
        }
        lua_pop(L,1);

        /* Retrieve rejection parameters */
        lua_pushstring(L, "reject");
        lua_gettable(L,2);
        if (lua_istable(L,-1)) {
            /* Retrieve rejection Min */
            lua_pushstring(L, "min");
            lua_gettable(L,-2);
            if (lua_isnumber(L,-1)) {
                rejlo = (int)lua_tonumber(L,-1);
                printf("rejmin=%d\n", rejlo);
            }
            lua_pop(L, 1);
            /* Retrieve rejection Max */
            lua_pushstring(L, "max");
            lua_gettable(L,-2);
            if (lua_isnumber(L,-1)) {
                rejhi = (int)lua_tonumber(L,-1);
                printf("rejmax=%d\n", rejhi);
            }
            lua_pop(L,1);
        }
        lua_pop(L,1);
    }

    /* Collapse first image to get size information */
    if (median) {
        im = image_collapse_median(c_in->plane[0],
                                   direction,
                                   rejlo,
                                   rejhi);
    } else {
        im = image_collapse(c_in->plane[0], direction);
    }
    if (im==NULL) {
        return 0 ;
    }

    /* Create output cube */
    collapsed = cube_new(im->lx, im->ly, c_in->np);
    collapsed->plane[0] = im ;

    /* Loop on all input planes, collapse them all */
    if (c_in->np>1) {
        for (i=1 ; i<c_in->np ; i++) {
            if (median) {
                im = image_collapse_median(c_in->plane[i],
                                           direction,
                                           rejlo,
                                           rejhi);
            } else {
                im = image_collapse(c_in->plane[i], direction);
            }
            if (im==NULL) {
                cube_del(collapsed);
                return 0 ;
            }
            collapsed->plane[i] = im ;
        }
    }

    /* Push result onto stack, return */
    lua_pushusertag(L, collapsed, LUA_TCUBE);
    return 1 ;
}

