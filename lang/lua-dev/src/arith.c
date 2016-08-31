
/*-------------------------------------------------------------------------*/
/**
    add -- Add two cubes or a cube and a double.

    Usage:
    % add(c1, c2)   --> c1 = c1 + c2
    % add(c1, 3)    --> c1 = c1 + 3
    % add( 7, c1)   --> c1 =  7 + c1

    This function performs an addition with cube arguments. If two cubes
    are given, the first one is modified to contain the results of the
    operation. If only one cube is given in its arguments, the cube is
    modified to contain the results of the operation.
 */
/*--------------------------------------------------------------------------*/

int wrap_cube_add(lua_State * L)
{
    cube_t  *   c1 ;
    cube_t *    c2 ;
    double      d ;
    int         status ;

    if (lua_iscube(L,1) && lua_iscube(L,2)) {
        c1 = (cube_t*)lua_touserdata(L,1);
        c2 = (cube_t*)lua_touserdata(L,2);
        status = cube_op(&c1, c2, '+');
    } else if (lua_iscube(L,1)) {
        c1 = (cube_t*)lua_touserdata(L,1);
        d   = lua_tonumber(L,2);
        status = cube_cst_op(c1, d, '+');
    } else {
        c1 = (cube_t*)lua_touserdata(L,2);
        d   = lua_tonumber(L,1);
        status = cube_cst_op(c1, d, '+');
    }
    lua_pushusertag(L, (void*)c1, LUA_TCUBE);
    return 1 ;
}


/*-------------------------------------------------------------------------*/
/**
    mul -- Multiply two cubes or a cube and a double.

    Usage:
    % mul(c1, c2)   --> c1 = c1 * c2
    % mul(c1, 3)    --> c1 = c1 * 3
    % mul( 7, c1)   --> c1 = 7  * c1

    This function performs a multiplication with cube arguments. If two
    cubes are given, the first one is modified to contain the results of
    the operation. If only one cube is given in its arguments, the cube is
    modified to contain the results of the operation.
 */
/*--------------------------------------------------------------------------*/

int wrap_cube_mul(lua_State * L)
{
    cube_t  *   c1 ;
    cube_t *    c2 ;
    double      d ;
    int         status ;

    if (lua_iscube(L,1) && lua_iscube(L,2)) {
        c1 = (cube_t*)lua_touserdata(L,1);
        c2 = (cube_t*)lua_touserdata(L,2);
        status = cube_op(&c1, c2, '*');
    } else if (lua_iscube(L,1)) {
        c1 = (cube_t*)lua_touserdata(L,1);
        d   = lua_tonumber(L,2);
        status = cube_cst_op(c1, d, '*');
    } else {
        c1 = (cube_t*)lua_touserdata(L,2);
        d   = lua_tonumber(L,1);
        status = cube_cst_op(c1, d, '*');
    }
    lua_pushusertag(L, (void*)c1, LUA_TCUBE);
    return 1 ;
}

/*-------------------------------------------------------------------------*/
/**
    sub -- Subtraction between two cubes or a cube and a double.

    Usage:
    % sub(c1, c2)       --> c1 = c1 - c2
    % sub(c1, 3)        --> c1 = c1 - 3
    % sub( 7, c1)       --> c1 =  7 - c1

    This function performs a subtraction with cube arguments. If two
    cubes are given, the first one is modified to contain the results of
    the operation. If only one cube is given in its arguments, the cube is
    modified to contain the results of the operation.

 */
/*--------------------------------------------------------------------------*/

int wrap_cube_sub(lua_State * L)
{
    cube_t  *   c1 ;
    cube_t *    c2 ;
    double      d ;
    int         status ;

    if (lua_iscube(L,1) && lua_iscube(L,2)) {
        c1 = (cube_t*)lua_touserdata(L,1);
        c2 = (cube_t*)lua_touserdata(L,2);
        status = cube_op(&c1, c2, '-');
    } else if (lua_iscube(L,1)) {
        c1 = (cube_t*)lua_touserdata(L,1);
        d   = lua_tonumber(L,2);
        status = cube_cst_op(c1, d, '-');
    } else {
        c1 = (cube_t*)lua_touserdata(L,2);
        d   = lua_tonumber(L,1);
        cube_invert(c1);
        status = cube_cst_op(c1, d, '+');
    }
    lua_pushusertag(L, (void*)c1, LUA_TCUBE);
    return 1 ;
}


/*-------------------------------------------------------------------------*/
/**
    div -- Division between two cubes or a cube and a double.

    Usage:
    % div(c1, c2)       --> c1 = c1 / c2
    % div(c1, 3)        --> c1 = c1 / 3
    % div( 7, c1)       --> c1 =  7 / c1

    This function performs a subtraction with cube arguments. If two
    cubes are given, the first one is modified to contain the results of
    the operation. If only one cube is given in its arguments, the cube is
    modified to contain the results of the operation.

 */
/*--------------------------------------------------------------------------*/

int wrap_cube_div(lua_State * L)
{
    cube_t  *   c1 ;
    cube_t *    c2 ;
    double      d ;
    int         status ;

    if (lua_iscube(L,1) && lua_iscube(L,2)) {
        c1 = (cube_t*)lua_touserdata(L,1);
        c2 = (cube_t*)lua_touserdata(L,2);
        status = cube_op(&c1, c2, '/');
    } else if (lua_iscube(L,1)) {
        c1 = (cube_t*)lua_touserdata(L,1);
        d   = lua_tonumber(L,2);
        status = cube_cst_op(c1, d, '/');
    } else {
        c1 = (cube_t*)lua_touserdata(L,2);
        d   = lua_tonumber(L,1);
        cube_recip(c1);
        status = cube_cst_op(c1, d, '*');
    }
    lua_pushusertag(L, (void*)c1, LUA_TCUBE);
    return 1 ;
}

