
/*-------------------------------------------------------------------------*/
/**
    filter -- Apply a named filter to a cube.

    Usage:
    % filter(c, "filter_name")
    % filter(c, "filter_name", { values })

    Applies a named filter to a cube. Most filters do not require any extra
    parameter. Some filters require extra numerical values to be passed in
    a table as a third argument.
    Valid filters are:

    - user-linear
    - mean3
    - dx
    - dy
    - d2x
    - d2y
    - contour1
    - contour2
    - contour3
    - contrast1
    - mean5
    - min
    - max
    - median
    - max-min
    - user-morpho
    - 3x1
    - flat

 */
/*--------------------------------------------------------------------------*/

int wrap_cube_filter(lua_State * L)
{
    cube_t  *   c1 ;
    char    *   filter_name ;
    double  *   filter_val ;
    int         i, nval ;
    int         ret ;

    /* Retrieve cube */
    if (lua_iscube(L,1)) {
        c1 = (cube_t*)lua_touserdata(L,1);
    } else {
        e_error("in filter(): expecting cube as first arg");
        return 0 ;
    }
    /* Retrieve filter name */
    if (lua_isstring(L,2)) {
        filter_name = (char*)lua_tostring(L,2);
    } else {
        e_error("in filter(): expecting filter name (string) as 2nd arg");
        return 0 ;
    }
    /* For some filters, retrieve values too */
    /* Look for third argument as a table */
    if (!lua_istable(L,3)) {
        filter_val = NULL ;
    } else {
        /* Get number of values in the table */
        nval = lua_getn(L,3);

        /* Check consistency with expected number of values */
        if (!strcmp(filter_name, "user-linear") && (nval != 9)) {
            e_error("expecting 9 values for filter %s", filter_name);
            return 0 ;
        }
        if (!strcmp(filter_name, "user-morpho") && (nval != 9)) {
            e_error("expecting 9 values for filter %s", filter_name);
            return 0 ;
        }
        if (!strcmp(filter_name, "3x1") && (nval != 3)) {
            e_error("expecting 3 values for filter %s", filter_name);
            return 0 ;
        }
        if (!strcmp(filter_name, "flat") && (nval != 3)) {
            e_error("expecting 1 value for filter %s", filter_name);
            return 0 ;
        }
        filter_val = malloc(nval * sizeof(double));
        /* Traverse table */
        i=0 ;
        lua_pushnil(L);
        while (lua_next(L, 3)!=0) {
            /* Key is at index -2, value at -1 */
            filter_val[i] = lua_tonumber(L,-1);
            i++ ;
            lua_pop(L,1);
        }
    }
    ret = cube_filter(c1, filter_name, filter_val);
    if (filter_val!=NULL)
        free(filter_val);

    return 0 ;
}

