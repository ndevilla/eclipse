import c_eclipse
eclipse_py_version = "4.3.0"
#
# Check underlying C library version
#
if c_eclipse.get_eclipse_version:
	cver = c_eclipse.get_eclipse_version()
	if cver != eclipse_py_version:
		print "warning: eclipse.py and c_eclipse.so from different versions"
		print "         eclipse.py is", eclipse_py_version
		print "         c_eclipse.so is", cver


import string, time, os, types, sys

_number_types = [types.IntType, types.FloatType, types.LongType]
_FAILURE = -1
c_eclipse.cube_set_fits_bpp(-16)


class EclipseError(Exception):
    pass

class statstruct:
    '''A class to hold image statistics.

    Each instance has the following attributes
    min_pix -- The minimum value
    max_pix -- The maximum value
    avg_pix -- The average value
    median -- The median value
    stdev -- The standard deviation
    energy -- The total energy
    flux -- The total flux
    absflux -- The total absolute flux
    min_x -- The x-coordinate of the minimum value
    min_y -- The y-coordinate of the minimum value
    max_x -- The x-coordinate of the maximum value
    max_y -- The y-coordinate of the maximum value
    npix -- The total number of pixels
    '''
    def __init__(self, stat_tuple):
        self.min_pix = stat_tuple[0]
        self.max_pix = stat_tuple[1]
        self.avg_pix = stat_tuple[2]
        self.median = stat_tuple[3]
        self.stdev = stat_tuple[4]
        self.energy = stat_tuple[5]
        self.flux = stat_tuple[6]
        self.absflux = stat_tuple[7]
        self.min_x = stat_tuple[8]
        self.min_y = stat_tuple[9]
        self.max_x = stat_tuple[10]
        self.max_y = stat_tuple[11]
        self.npix = stat_tuple[12]
  
class cube:
    '''A fits data cube
    
    A cube represents a stack of fits images. It can be constructed
    from 1 or more fits files, each containing 1 or more 2-dimensional
    frmaes, or planes. Each plane in a cube is of equal size.
    '''
    def __init__(self, filename=None):
	'''filename may have the following values:

        None (default) : the cube is an anonymous temporary object
        (used internally)
        'name.fits' : the cube will loaded from name.fits
        ['name1.fits', 'name2.fits'] : the cube will include all
        files from the list.
        '''
	self.filename = filename
        self.p_cube = None

    def load_cube(self):
        '''Load a cube into memory

        No arguments

        Load a cube into memory. Most cube methods will perform this
        operation before calling their processing algorithm
        '''
        if self.p_cube:
            return

        if not self.filename:
            raise EclipseError, 'Error loading cube, no filename specified'

        if type(self.filename) == types.ListType:
            self.p_cube = c_eclipse.cube_load_strings(self.filename,
                                                      len(self.filename))
        else:
            self.p_cube = c_eclipse.cube_load(self.filename)

        if self.p_cube is None:
            raise EclipseError, 'Error loading cube from'+`self.filename`
            
    def save_cube(self, filename, hdr=None):
        '''Save a cube

        Arguments:
        filename -- Name of fits file to write
        hdr -- a header() object (optional)

        Save the cube to filename, using either a default header, or the
        header supplied as an argument to this method
        '''

	self.load_cube()
	self.filename = filename
        if hdr is None:
            if c_eclipse.cube_save_fits(self.p_cube,
                                        self.filename) == _FAILURE:
                raise EclipseError, 'Error saving cube to '+`self.filename`
        else:
            if not isinstance(hdr, header):
                raise EclipseError, '%s Not a valid header' % hdr
            if c_eclipse.cube_save_fits_hdrdump(self.p_cube,
                                                self.filename,
                                                hdr.header) == _FAILURE:
                raise EclipseError, 'Error saving cube to '+`self.filename`
                
    def copy(self):
        '''Copy the data to a new cube() object

        No arguments
        '''
	self.load_cube()
	result = cube()
        result.p_cube = c_eclipse.cube_copy(self.p_cube)
        if result.p_cube == 'NULL':
            raise EclipseError, 'Error copying cube'
	return result

    def fast_copy(self):
        '''Move the data to a new cube() object

        No Arguments

        This method copies the data pointer to a new cube() object,
        iff the data is reloadable from disk. If the data is not reloadable,
        the data is copied. This method is used for internal optimizations.
        Use with caution (Only if you have looked at the source code)
        '''
        
	if self.filename:
            # I can reload, so just move the pointer (cheap)
	    self.load_cube()
	    result = cube()
	    result.p_cube, self.p_cube = self.p_cube, None
	else:
	    # I cannot re-load, so do an explicit copy
	    result = self.copy()
	return result

    def __del__(self):
	if self.p_cube:
	    c_eclipse.cube_del(self.p_cube)

    def __str__(self):
	if self.filename:
	    return '<cube of %s>' % self.filename
	else:
	    return '<temporary cube>'

    def __len__(self):
	self.load_cube()
        res = c_eclipse.cube_getnp(self.p_cube)
        if res == _FAILURE:
            raise EclipseError, 'Error determining number of planes'
        return res

########################################################################
# 
# Arithmetic operations
#
    def __iadd__(self, other):
        self.load_cube()
	if type(other) in _number_types:
            if c_eclipse.cube_cst_op(self.p_cube,
                                     other, ord('+')) == _FAILURE:
                raise EclipseError, 'Error adding constant %s to cube' % `other`
	else:
	    other.load_cube()
            if c_eclipse.cube_add(self.p_cube, other.p_cube) == _FAILURE:
                raise EclipseError, 'Error adding cubes'
	return self
 
    def __isub__(self, other):
	self.load_cube()
	if type(other) in _number_types:
            if c_eclipse.cube_cst_op(self.p_cube,
                                     other, ord('-')) == _FAILURE:
                raise EclipseError, 'Error subtracting constant %s from cube' % `other`
	else:
	    other.load_cube()
            if c_eclipse.cube_sub(self.p_cube, other.p_cube) == _FAILURE:
                raise EclipseError, 'Error subtracting cubes'
	return self

    def __imul__(self, other):
	self.load_cube()
	if type(other) in _number_types:
            if c_eclipse.cube_cst_op(self.p_cube,
                                     other, ord('*')) == _FAILURE:
                raise EclipseError, 'Error multiplying constant %s to cube' % `other`
	else:
	    other.load_cube()
            if c_eclipse.cube_mul(self.p_cube, other.p_cube) == _FAILURE:
                raise EclipseError, 'Error multiplying cubes'
	return self

    def __idiv__(self, other):
	self.load_cube()
	if type(other) in _number_types:
            if other == 0:
                raise ZeroDivisionError
            if c_eclipse.cube_cst_op(self.p_cube, other, ord('/')) == _FAILURE:
                raise EclipseError, 'Error dividing cube by constant %s' % `other`
	else:
	    other.load_cube()
            if c_eclipse.cube_div(self.p_cube, other.p_cube) == _FAILURE:
                raise EclipseError, 'Error dividing cubes'
	return self

    def __ipow__(self, other):
	self.load_cube()
	if type(other) in _number_types:
            if c_eclipse.cube_cst_op(self.p_cube, other, ord('^')) == _FAILURE:
                raise EclipseError, 'Error raising cube to power %s' % `other`
	else:
            raise EclipseError, 'Operation cube**cube, not supported'
	return self

    def __add__(self, other):
        if sys.getrefcount(self) < 6:
	    return self.__iadd__(other)
        else:
            return self.fast_copy().__iadd__(other)

    def __sub__(self, other):
        if sys.getrefcount(self) < 6:
	    return self.__isub__(other)
        else:
            return self.fast_copy().__isub__(other)

    def __mul__(self, other):
        if sys.getrefcount(self) < 6:
	    return self.__imul__(other)
        else:
            return self.fast_copy().__imul__(other)

    def __div__(self, other):
        if sys.getrefcount(self) < 6:
	    return self.__idiv__(other)
        else:
            return self.fast_copy().__idiv__(other)

    def __pow__(self, other):
        if sys.getrefcount(self) < 6:
	    return self.__ipow__(other)
        else:
            return self.fast_copy().__ipow__(other)

    def __radd__(self, other):
        return self+other

    def __rsub__(self, other):
        return -(self-other)

    def __rmul__(self, other):
        return self*other

    def __rdiv__(self, other):
        return self**(-1) * other

    def __rpow__(self, other):
        raise EclipseError, 'Operation scalar**cube not supported'

    def __neg__(self):
        return self*-1
    
###########################################################################
#
# Statistical operations
#
    def stat_plane(self, plane=0):
        '''Compute the statics of an individual plane in the cube.

        Arguments:
        plane -- index of the plane (default=0)

        Returns an imstat object
        '''

	self.load_cube()

        p_ima = c_eclipse.cube_getplane(self.p_cube, plane)
        if p_ima is None:
            raise EclipseError, 'Error extracting plane %i from cube' % plane
        
        stat_ima = c_eclipse.image_getstats(p_ima)
        if stat_ima is None:
            raise EclipseError, 'Error obtaining statistics from cube'
        
	return statstruct(stat_ima)

    def stat(self):
        '''Compute the statistics for all planes in a cube

        No Arguments

        If the cube contains one plane, this returns the same as
        cube.stat_plane(), otherwise a list of statiscs, one for each
        plane is returned
        '''
	self.load_cube()
	np = self.__len__()
	if np==1:
	    return self.stat_plane()
	if np>1:
	    result=[]
	    for i in range(np):
		result.append(self.stat_plane(plane=i))
	    return result

    def stat_plane_opts(self, plane=0, pixmap=None, pixrange=None, zone=None):
        '''Compute the statics of an individual plane in the cube,
        using a pixmap and/or a pixrange and/or a zone to define
        included pixels

        Arguments:
        plane -- index of the plane (default=0)
        pixmap -- map of valid pixelsd (sefault=None)
        pixrange -- a range of valid values [low, high] (default=None)
        zone -- a valid zone [xmin, xmax, ymin, ymax]

        Returns an imstat object
        '''
        self.load_cube()

        if pixmap:
            pixmap.load_pixelmap()
            pmap = pixmap.p_pmap
        else:
            pmap = None
        
        p_ima = c_eclipse.cube_getplane(self.p_cube, plane)
        if p_ima is None:
            raise EclipseError, 'Error extracting plane %i from cube' % plane

        stat_ima = c_eclipse.image_getstats_opts(p_ima,
                                                 pmap,
                                                 pixrange,
                                                 zone)
        if stat_ima is None:
            raise EclipseError, 'Error obtaining statistics from cube'
        
	return statstruct(stat_ima)

    def stat_opts(self, pixmap=None, pixrange=None, zone=None):
        '''Compute the statistics for all planes in a cube,
        using a pixmap and/or a pixrange and/or a zone to define
        included pixels

        Arguments:
        pixmap -- map of valid pixelsd (sefault=None)
        pixrange -- a range of valid values [low, high] (default=None)
        zone -- a valid zone [xmin, xmax, ymin, ymax]

        No Arguments

        If the cube contains one plane, this returns the same as
        cube.stat_plane(), otherwise a list of statiscs, one for each
        plane is returned
        '''
	self.load_cube()
	np = self.__len__()
	if np==1:
	    return self.stat_plane_opts(0, pixmap, pixrange, zone)
	if np>1:
	    result=[]
	    for i in range(np):
		result.append(self.stat_plane_opts(i, pixmap, pixrange, zone))
	    return result

    def median_plane(self, plane=0):
        '''Compute the median of a plane'''
	self.load_cube()
        if plane>=self.__len__():
            raise EclipseError, 'Error, plane %i not in cube' % plane
        try:
            p_ima = c_eclipse.cube_getplane(self.p_cube, plane)
            median = c_eclipse.image_getmedian(p_ima)
        except:
            raise EclipseError, 'Error obtaining statistics from cube'
       	return median
		

    def median(self):
        '''Compute the median for each plane in a cube

        No Arguments

        If the cube contains one plane the median of that plane is
        returned, otherwise a list of medians, one for each plane, is
        returned.
        '''
	self.load_cube()
	np = self.__len__()
	if np==1:
	    return self.median_plane()
	if np>1:
	    result=[]
	    for i in range(np):
		result.append(self.median_plane(plane=i))
	    return result

###########################################################################
#
#   Operations modifying cube in place
#
    norm_methods = {'scale':1, 'mean':2, 'flux':3, 'aflux':4}

    def norm(self, method='mean'):
        '''Normalize each plane in a cube

        method should be one of:
        mean -- Scale each plane to a mean of 1.0
        scale -- Scale each plane, such that min=0.0 and max=1.0
        flux -- Divide each plane by its total flux is 1.0
        aflux -- Divide each plane by its total absoult flux

        Modifies the cube in place
        '''
        if not(method in self.norm_methods.keys()):
            raise EclipseError, 'Error, unknown normalization method '+method
        self.load_cube()
        if c_eclipse.cube_normalize(self.p_cube,
                                    self.norm_methods[method]) == _FAILURE:
            raise EclipseError, 'Error, normalization failed'

    def scale(self, toFlux):
        '''Normalize each plane in a cube to toFlux

        toFlux -- the flux to which each plane should be scaled

        Modifies the cube in place
        '''
        self.load_cube()
        if c_eclipse.cube_scale_flux(self.p_cube, toFlux) == _FAILURE:
            raise EclipseError, 'Error, scaling failed'
        
    def threshold(self, low_cut, high_cut, low_value, high_value):
        '''Replace values outside thresholds with constants

        Arguments:
        low_cut -- the minimum value below which values should be replaced
        high_cut -- the maximum value, above which values should be replaced
        low_value -- the constant that replaces values below low_cut
        high_value -- the constant that replaces values above high_cut

        Modifies the cube in place
        '''
        self.load_cube()
        if c_eclipse.cube_threshold(self.p_cube,
                                    low_cut, high_cut,
                                    low_value, high_value) == _FAILURE:
            raise EclipseError, 'Error, thresholding failed'
        
    def apply_named_filter(self, filtername, filtervalue):
        self.load_cube()
        if c_eclipse.cube_filter(self.p_cube,
                                 filtername, filtervalue) == _FAILURE:
            raise EclipseError, 'Error, applying filter %s failed' % filtername

    def median_filter(self):
        '''Apply a 3x3 median filter on the cube'''
        self.load_cube()
        if c_eclipse.cube_filter_median(self.p_cube) == _FAILURE:
            raise EclipseError, 'Error applying median filter'
        

    def thresh_to_pixmap(self, low_cut, hi_cut):
        self.load_cube()
	if self.__len__() > 1:
	    raise EclipseError, 'Error, thresholding to pixelmap for multiple planes not supported'
	res = Pixelmap()
        try:
            p_ima = c_eclipse.cube_getplane(self.p_cube, 0)
            res.p_pmap = c_eclipse.image_threshold2pixelmap(p_ima,
                                                            low_cut, hi_cut)
        except:
            raise EclipseError, 'Error, thresholding failed'
	return res

# TO IMPLEMENT:
#int cube_filter_3x3(cube_t *cube1, double  *filter_array)
#int cube_filter_5x5(cube_t *cube1, double  *filter_array);
#int cube_filter_morpho(cube_t *cube1, double  *filter_array);
#int median_filter_cube(cube_t * cube1);

###########################################################################
#
#   Operations producing new Cube
#
    def time_stdev_on_cube(self):
	self.load_cube()
        
        p_ima = c_eclipse.cube_stdev_z(self.p_cube)
        if p_ima is None:
            raise EclipseError, 'time_stdev_on_cube failed'
        result = cube()
        result.p_cube = c_eclipse.cube_from_image(p_ima)
        if result.p_cube is None:
            raise EclipseError, 'time_stdev_on_cube failed' 
	return result

    def average(self):
	self.load_cube()
        p_ima = c_eclipse.cube_avg_linear(self.p_cube)
        if p_ima is None:
            raise EclipseError, 'average failed'
        result = cube()
        result.p_cube = c_eclipse.cube_from_image(p_ima)
        if result.p_cube is None:
            raise EclipseError, 'average failed' 
	return result
    
    def average_with_rejection(self, low_reject, high_reject):
        '''Average the planes in the cube

        low_reject, and high_reject give the number lowest and highest pixels
        to reject for the average of each pixel'''
	self.load_cube()
        p_ima = c_eclipse.cube_avg_reject(self.p_cube, low_reject, high_reject)
        if p_ima is None:
            raise EclipseError, 'average with rejection failed'
        result = cube()
        result.p_cube = c_eclipse.cube_from_image(p_ima)
        if result.p_cube is None:
            raise EclipseError, 'average with rejection failed' 
	return result

    def sum_cube_to_image(self):
	self.load_cube()
        p_ima = c_eclipse.cube_avg_sum(self.p_cube)
        if p_ima is None:
            raise EclipseError, 'cube sum failed'
        result = cube()
        result.p_cube = c_eclipse.cube_from_image(p_ima)
        if result.p_cube is None:
            raise EclipseError, 'cube sum failed' 
	return result

    def median_average(self):
	self.load_cube()
        p_ima = c_eclipse.cube_avg_median(self.p_cube)
        if p_ima is None:
            raise EclipseError, 'median average failed'
        result = cube()
        result.p_cube = c_eclipse.cube_from_image(p_ima)
        if result.p_cube is None:
            raise EclipseError, 'median average failed' 
	return result

    def cycle_average(self, cycle_length):
	self.load_cube()
        result = cube()
        result.p_cube = c_eclipse.cube_avgcyc_linear(self.p_cube, cycle_length)
        if result.p_cube is None:
            raise EclipseError, 'Error, cube cycle averageing failed'
	return result

    def cycle_sum(self, cycle_length):
	self.load_cube()
	result = cube()
        result.p_cube = c_eclipse.cube_avgcyc_sum(self.p_cube, cycle_length)
        if result.p_cube is None:
            raise EclipseError, 'Error, cube cycle summing failed'
	return result

    def cycle_median(self, cycle_length):
	self.load_cube()
	result = cube()
        result.p_cube = c_eclipse.cube_avgcyc_median(self.p_cube, cycle_length)
        if result.p_cube is None:
            raise EclipseError, 'Error, cube cycle median failed'        
	return result
      
    def running_average(self, half_cycle):
	self.load_cube()
	result = cube()
        result.p_cube = c_eclipse.cube_avgrun_linear(self.p_cube, half_cycle)
        if result.p_cube is None:
            raise EclipseError, 'Error, cube running average failed'
	return result

    def running_sum(self, half_cycle):
	self.load_cube()
	result = cube()
        result.p_cube = c_eclipse.cube_avgrun_sum(self.p_cube, half_cycle)
        if result.p_cube is None:
            raise EclipseError, 'Error, cube running sum failed'
	return result

    def running_median(self, half_cycle):
	self.load_cube()
	result = cube()
        result.p_cube = c_eclipse.cube_avgrun_median(self.p_cube, half_cycle)
        if result.p_cube is None:
            raise EclipseError, 'Error, cube running median failed'
	return result

####################################################################################################
#
#   Cube extraction
#
    def extract_region(self, llx, lly, urx, ury):
	self.load_cube()
        result = cube()
        result.p_cube = c_eclipse.cube_getvig(self.p_cube,
                                              llx+1, lly+1, urx+1, ury+1)
        if result.p_cube is None:
            raise EclipseError, 'Error extracting (%d, %d, %d, %d) from cube' % (llx, lly, urx, ury)
	return result

    def cube_copy_planes(self, planelist):
	self.load_cube()
	result = cube()
        result.p_cube = c_eclipse.cube_copy_planes(self.p_cube,
                                                   planelist,
                                                   len(planelist))
        if result.p_cube is None:
            raise EclipseError, 'Error copying planes %s from cube' % `planelist`
        return result

    def __getitem__(self, index):
        if index < 0:
            index = len(self)-index
        if index<0 or index>=len(self):
            raise IndexError
	return self.cube_copy_planes([index])

    def __getslice__(self, low, high):
        planelist = range(self.__len__())[low:high]
        if not planelist:
            raise EclipseError, 'Empty slice'
	return self.cube_copy_planes(planelist)	

class CubeGenerator:
    '''Provides object capable of producing artificially generated data
    with various properties. Generated sibngle-plane cubes only'''

    def __init__(self, xsize, ysize):
        '''Constructor
        
        xsize -- the x dimension of the cubes generated
        ysize -- the y dimension of the cubes generated
        '''
	self.xsize = xsize
	self.ysize = ysize
#	self.nplanes = nplanes

    def generate_random_uniform(self, min_pix, max_pix):
        '''Generate a cube with noise uniformly distributed
        betwwen min_pix and max_pix'''

        p_ima = c_eclipse.image_gen_random_uniform(self.xsize, self.ysize,
                                                       min_pix, max_pix)
        if p_ima == 'NULL':
            raise EclipseError, 'Error generating random cube'
        result = cube()
        result.p_cube = c_eclipse.cube_from_image(p_ima)
	return result

    def generate_random_gauss(self, dispersion, mean):
        '''Generate a cube with noise distributed normally around mean,
        with rms dispersion'''
        
        p_ima = c_eclipse.image_gen_random_gauss(self.xsize, self.ysize,
                                                     dispersion, mean)
        if p_ima == 'NULL':
            raise EclipseError, 'Error generating random cube'
        result = cube()
        result.p_cube = c_eclipse.cube_from_image(p_ima)
        return result
    
    def generate_random_lorentz(self, dispersion, mean):
        '''Generate a cube with lorentzian distributed noise'''
        
        p_ima = c_eclipse.image_gen_random_lorentz(self.xsize, self.ysize,
                                                     dispersion, mean)
        if p_ima == 'NULL':
            raise EclipseError, 'Error generating random cube'
        result = cube()
        result.p_cube = c_eclipse.cube_from_image(p_ima)
        return result
    
    def generate_poly2d(self, c=0.0, c_x=0.0, c_y=0.0, c_xx=0.0, c_xy=0.0, c_yy=0.0):
        '''Generate an image from a 2d polynomial

        ima(x,y) = c+c_x*x+c_y*y+c_xx*x*x+c_xy*x*y+c_yy*y*y
        '''
        p_ima = c_eclipse.image_gen_poly2d(self.xsize, self.ysize,
                                           [c_xx, c_yy, c_xy, c_x, c_y, c])
        if p_ima == 'NULL':
            raise EclipseError, 'Error generating cube from 2D poly'
        result = cube()
        result.p_cube = c_eclipse.cube_from_image(p_ima)
        return result

class Pixelmap:
    def __init__(self, filename=None):
        self.filename = filename
        self.p_pmap = None

    def load_pixelmap(self):
        if not(self.p_pmap):
            self.p_pmap = c_eclipse.pixelmap_load(self.filename)
            if self.p_pmap is None:
                raise EclipseError, 'Error loading pixelmap from '+self.filename
            
    def dump_pixelmap(self, filename):
        self.filename = filename
        if self.p_pmap:
            if c_eclipse.pixelmap_dump(self.p_pmap, self.filename) == _FAILURE:
                raise EclipseError, 'Error saving pixelmap to '+self.filename
            
    def copy(self, filename=None):
        self.load_pixelmap()
        result = Pixelmap(filename)
        try:
            result.p_pmap = c_eclipse.pixelmap_copy(self.p_pmap)
        except:
            raise EclipseError, 'Error copying pixelmap'
        return result

    def as_cube(self):
        self.load_pixelmap()
        result = cube()
        try:
            p_ima = c_eclipse.pixelmap_2_image(self.p_pmap)
            result.p_cube = c_eclipse.cube_from_image(p_ima)
        except:
            raise EclipseError, 'Error copying pixelmap to cube'
        return result

    def __del__(self):
	if self.p_pmap:
	    c_eclipse.pixelmap_del(self.p_pmap)

###########################################################################################
#
# Binary operations on pixelmaps
#
    def bin_not(self):
        self.load_pixelmap()
        if c_eclipse.pixelmap_binary_NOT(self.p_pmap) == _FAILURE:
            raise EclipseError, 'Error, failed to invert pixelmap'
        return self
   
    def __iand__(self, other):
        self.load_pixelmap()
        other.load_pixelmap()
        if c_eclipse.pixelmap_binary_AND(self.p_pmap, other.p_pmap) == _FAILURE:
            raise EclipseError, 'Error, conjunction of pixelmaps failed'
        return self
        
    def __ior__(self, other):
        self.load_pixelmap()
        other.load_pixelmap()
        if c_eclipse.pixelmap_binary_OR(self.p_pmap, other.p_pmap) == _FAILURE:
            raise EclipseError, 'Error, disjunction of pixelmaps failed'
        return self
    
    def bin_xor(self, other):
        self.load_pixelmap()
        other.load_pixelmap()
        if c_eclipse.pixelmap_binary_XOR(self.p_pmap, other.p_pmap) == _FAILURE:
            raise EclipseError, 'Error, disjunction of pixelmaps failed'
        return self
    
    def __and__(self, other):
        if sys.getrefcount(self) < 6:
	    return self.__iand__(other)
        else:
            return self.copy().__iand__(other)

    def __or__(self, other):
        if sys.getrefcount(self) < 6:
	    return self.__ior__(other)
        else:
            return self.copy().__ior__(other)

    def __invert__(self):
        if sys.getrefcount(self) < 6:
	    return self.bin_not()
        else:
            return self.copy().bin_not()


class header:
    def __init__(self, filename=None, extension=0):
        self.filename = filename
        self.header = None
        if filename:
            try:
                if extension:
                    self.header = c_eclipse.qfits_header_readext(self.filename, extension)
                else:
                    self.header = c_eclipse.qfits_header_read(self.filename)
            except Exception, e:
                print e
                raise EclipseError, 'Error loading header from '+self.filename
        else:
            self.header = None

    def new(self):
        if self.header:
            c_eclipse.qfits_header_destroy(self.header)
        try:
            self.header = c_eclipse.qfits_header_new()
        except:
            raise EclipseError, 'Error creating new header'
        return self
    
    def default(self):
        if self.header:
            c_eclipse.qfits_header_destroy(self.header)
        try:
            self.header = c_eclipse.qfits_header_default()
        except:
            raise EclipseError, 'Error creating default header'
        return self
    
    def add(self, key, val, comment):
        if type(val) != types.StringType:
            val = `val`
        if c_eclipse.qfits_header_add(self.header, key,
                                     val, comment, '') == _FAILURE:
            raise EclipseError, 'Error adding %s to header' % `(key, val, comment)`
        
    def add_after(self, after_key, key, val, comment):
        if type(val) != types.StringType:
            val = `val`
        if c_eclipse.qfits_header_add_after(self.header, after_key,
                                           key, val, comment, '') == _FAILURE:
            raise EclipseError, 'Error adding %s to header' % `(key, val, comment)`

    def append(self, key, val, comment):
        if type(val) != types.StringType:
            val = `val`
        if c_eclipse.qfits_header_append(self.header, key,
                                        val, comment, '') == _FAILURE:
            raise EclipseError, 'Error adding %s to header' % `(key, val, comment)`

    def modify(self, key, val, comment):
        if type(val) != types.StringType:
            val = `val`
        if c_eclipse.qfits_header_mod(self.header, key,
                                     val, comment) == _FAILURE:
            raise EclipseError, 'Error modifying %s in header' % `(key, val, comment)`

    def copy(self):
        result = header()
        try:
            result.header = c_eclipse.qfits_header_copy(self.header)
        except:
            raise EclipseError, 'Error copying header'
        return result

    def get_valstr(self, key):
        return c_eclipse.qfits_header_getstr(self.header, key)
        
    def get_commentstr(self, key):
        return c_eclipse.qfits_header_getcom(self.header, key)

    def get_value(self, key):
        valstr = self.get_valstr(key)
        if not(valstr):
            return None
        try:
            return int(valstr)
        except ValueError:
            pass

        try:
            return float(valstr)
        except ValueError:
            pass

        if valstr[0] == "'" and valstr[-1] == "'":
            return valstr[1:-1].strip()
        
        return valstr

    def __getitem__(self, key):
        return self.get_value(key)

    def __setitem__(self, key, val):
        comment = ''
        if type(val) == types.TupleType:
            val, comment = val
        if self.get_value(key) == None:
            self.add(key, val, comment)
        else:
            self.modify(key, val, comment)

    def dump(self):
        c_eclipse.qfits_header_consoledump(self.header)

    def __del__(self):
        if self.header:
            c_eclipse.qfits_header_destroy(self.header)















