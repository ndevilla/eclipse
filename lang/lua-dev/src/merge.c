
/*-------------------------------------------------------------------------*/
/**

    merge -- Merge several cubes together.

    Usage:
    % big = merge( { cube1, cube2, ..., cubei } )
    The following form is also valid, since the only argument to this
    function is a list (the parentheses can then be omitted).
    % big = merge { cube1, cube2, ..., cubei }

    The following merges a number of input cubes into a single one.
    The input cubes are then deallocated within this function. Notice that
    image pointers are copied to the new (returned) cube, and only the
    top-level structs for each input cube are destroyed.
    The input must be a list of cubes, i.e. {a,b,c,...} where a,b,c are valid
    cubes.

 */
/*--------------------------------------------------------------------------*/



int wrap_cube_merge(lua_State * L)
{
    cube_t  **  all_cubes ;
    cube_t  *   concat ;
    int         i, j, k ;
    int         nc ;
    int         np ;
    int         lx, ly ;

    /* Retrieve list of input cubes */
    /* Check that input is indeed a table */
    if (!lua_istable(L,1)) {
        e_error("in merge() function: input must be a list");
        return 0 ;
    }

    /* Retrieve number of cube pointers */
    nc = lua_getn(L,1);
    all_cubes = malloc(nc * sizeof(cube_t*)) ;

    /* Initialization before table traversal */
    lua_pushnil(L);
    i=0 ;
    lx = -1 ;
    ly = -1 ;
    np = 0 ;

    /* Traverse table and retrieve cube pointers */
    while (lua_next(L,1)!=0) {
        /* Key is at -2 and value at -1 */
        /* No need for key */
        all_cubes[i] = (cube_t*)lua_touserdata(L,-1);
        /* Add up number of planes */
        np += all_cubes[i]->np ;
        if (i==0) {
            /* First input cube: remember size in x and y */
            lx = all_cubes[i]->lx ;
            ly = all_cubes[i]->ly ;
        } else {
            /* Other input cubes: check size consistency */
            if ((all_cubes[i]->lx != lx) ||
                (all_cubes[i]->ly != ly)) {
                e_error("inconsistent cube sizes: cannot merge");
                free(all_cubes);
                return 0 ;
            }
        }
        /* Iterate */
        i++ ;
        lua_pop(L,1);
    }

    /* Build a new cube from the input list of cubes */
    concat = cube_new(lx, ly, np);
    k=0 ;
    for (i=0 ; i<nc ; i++) {
        for (j=0 ; j<all_cubes[i]->np ; j++) {
            concat->plane[k] = all_cubes[i]->plane[j] ;
            /*
             * Set plane pointer to NULL so that destroy_cube does not try
             * to de-allocate the planes: they now exist in concat.
             */
            all_cubes[i]->plane[j] = NULL ;
            k++ ;
        }
    }
    /* Free temporary cube pointer holder */
    free(all_cubes);

    /* Push new cube onto stack */
    lua_pushusertag(L, concat, LUA_TCUBE);
    /* Activate GC */
    lua_setgcthreshold(L,1);
    return 1 ;
}

