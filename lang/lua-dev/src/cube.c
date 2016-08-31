
/*-------------------------------------------------------------------------*/
/**
 
  load -- Load a cube into memory.

    Usage:
    % c = load("a.fits")
        Load the file 'a.fits' as a cube in memory.

    Usage:
    % c = load("framelist.ascii")
        Load the ASCII list 'framelist.ascii' as a cube in memory.

  This function loads a cube into memory. It expects one single argument
  (a string) and will return either a new cube object, or nil.

 */
/*--------------------------------------------------------------------------*/
int wrap_cube_load(lua_State * L)
{
    cube_t  *   c1 ;

    /* Check input argument */
    if (!lua_isstring(L, 1)) {
        e_error("in arguments to load()\n");
        return 0 ;
    }
    /* Activate GC */
    lua_setgcthreshold(L,1);

    e_comment(0, "loading %s\n", lua_tostring(L,1));
    c1 = cube_load((char*)lua_tostring(L,1));
    if (c1==NULL) {
        e_error("loading cube: aborting");
        return 0 ;
    }
    lua_pushusertag(L, (void*)c1, LUA_TCUBE);
    lua_userdatasize(L, cube_get_bytesize(c1));
    return 1 ;
}


/*-------------------------------------------------------------------------*/
/**

  save -- Save a cube to disk

    Usage:
    % save(c, "output.fits")
        Save the cube 'c' to a file called 'output.fits', with a minimal
        (default) FITS header.

    Usage:
    % save(c, "output.fits", "ref.fits")
        Save the cube 'c' to a file called 'output.fits', using the file
        'ref.fits' as a reference for the FITS header.

    Usage:
    % save(c, "base_%d.fits")
        Save the cube 'c' to split files called base_1.fits, base_2.fits,
        etc., using a default minimal FITS header.
    
    Usage:
    % save(c, "base_%d.fits", "ref.fits")
        Save the cube 'c' to split files called base_1.fits, base_2.fits,
        etc., using the file 'ref.fits' as a reference for the FITS header.

    'save' will recognize that you require split file outputs by detecting
    the presence of a '%d' formatter in the output name string. You can
    also specify up to 6 leading zeros in the counter, using the C
    notation.
    Examples:

    "base_%d.fits"      -> base_1.fits, base_2.fits
    "base_%02d.fits"    -> base_01.fits, base_02.fits
    "base_%03d.fits"    -> base_001.fits, base_002.fits
    ...
    "base_%06d.fits"    -> base_000001.fits, base_000002.fits, etc.

 */
/*--------------------------------------------------------------------------*/

int wrap_save_cube(lua_State * L)
{
    cube_t *    c1 ;
    char    *   name ;
    char    *   refname ;
    char        outname[FILENAMESZ];
    int         split ;
    int         i ;

    /* First argument must be a cube */
    if (!lua_iscube(L,1)) {
        e_error("in save(): first argument must be a cube");
        return 0 ;
    } else {
        c1 = (cube_t*)lua_touserdata(L,1);
    }
    /* Second argument must be a string */
    if (!lua_isstring(L,2)) {
        e_error("in save(): second argument must be a string");
        return 0 ;
    } else {
        name = (char*)lua_tostring(L,2);
    }
    /* Get optional 3rd argument */
    if (lua_isstring(L,3)) {
        refname = (char*)lua_tostring(L,3);
    } else {
        refname = NULL ;
    }

    /* Detect split output */
    split = 0 ;
    if (strstr(name, "%d")!=NULL ||
        strstr(name, "%02d")!=NULL ||
        strstr(name, "%03d")!=NULL ||
        strstr(name, "%04d")!=NULL ||
        strstr(name, "%05d")!=NULL ||
        strstr(name, "%06d")!=NULL) {
        split=1 ;
    }

    if (!split) {
        if (refname==NULL) {
            cube_save_fits(c1, name);
        } else {
            cube_save_fits_hdrcopy(c1, name, refname);
        }
    } else {
        /* Loop on all input planes */
        for (i=0 ; i<c1->np ; i++) {
            sprintf(outname, name, i+1);
            if (refname==NULL) {
                image_save_fits(c1->plane[i], outname, BPP_DEFAULT);
            } else {
                image_save_fits_hdrcopy(    c1->plane[i],
                                            outname,
                                            refname,
                                            BPP_DEFAULT);
            }
        }
    }
    return 0 ;
}

/*-------------------------------------------------------------------------*/
/**

    del -- Discard a cube.

    Usage:
    % del(c)

    This function forces the deletion of all data contained within the
    given cube. This is not mandatory, since the garbage collector will
    eventually take care of deleting the cube after it got out of scope,
    but it might be useful in situations where there is only one global
    scope, or when you want to make sure that memory will be available for
    the next operation.

    NB: Deleting a cube the soft way can be done by assigning nil to it.

 */
/*--------------------------------------------------------------------------*/
int wrap_cube_del(lua_State * L)
{
    if (!lua_iscube(L,1)) {
        e_error("in arguments for del()");
        return 0 ;
    }
    debug_code(
        e_comment(0, "deleting cube");
    )

    cube_del_contents((cube_t*)lua_touserdata(L,1)) ;
    return 0 ;
}

/* internal: LUA garbage collector for cubes */
int wrap_cube_gc(lua_State * L)
{
    if (!lua_iscube(L,1)) {
        e_error("in arguments for cube GC");
        return 0 ;
    }

    debug_code(
        e_comment(0, "GC collecting cube");
    )

    cube_del((cube_t*)lua_touserdata(L,1));
    return 0 ;
}
