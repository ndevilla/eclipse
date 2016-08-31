import eclipse, unittest, os
    
def build_test_data():
    '''This function builds fits files to be used in testing

    The function creates two global lists. The first list contains the names
    of the fitsfiles The second list contains the statiscs of each
    fits image. The function also writes a file "filelist.dat" containing
    the names of the fits files.
    '''

    global filenames, statistics
    print 'Generating testdata...'
    statistics = []
    filenames = []
    filelist = open('filelist.dat', 'w')
    cg = eclipse.CubeGenerator(128, 128)
    for i in range(1,6):
        name = 'IM%2.2i.fits' % i
        new_cube = cg.generate_random_gauss(0.5*i, 1.0*i)
        new_cube.save_cube(name)
        statistics.append(new_cube.stat())
        filenames.append(name)
        filelist.write(name+'\n')
    filelist.close()
    cg = eclipse.CubeGenerator(129, 129)
    new_cube = cg.generate_random_gauss(0.5*i, 1.0*i)
    new_cube.save_cube('IM01.large.fits')
    

def delete_test_data():
    '''Clean up the mess'''
    global filenames
    print 'Deleting test data...'
    for filename in filenames:
        os.remove(filename)
    os.remove('filelist.dat')
    os.remove('IM01.large.fits')
    
class cube_tests(unittest.TestCase):
    def test_load_good_filename(self):
        cube = eclipse.cube('IM01.fits')
        cube.load_cube()
        self.failIfEqual(cube.p_cube, None)
        
    def test_load_good_filename_list(self):
        cube = eclipse.cube(filenames)
        cube.load_cube()
        self.failIfEqual(cube.p_cube, None)

    def test_load_good_filename_file(self):
        cube = eclipse.cube('filelist.dat')
        cube.load_cube()
        self.failIfEqual(cube.p_cube, None)

    def test_load_bad_filename(self):
        cube = eclipse.cube('IM.fits')
        self.failUnlessRaises(eclipse.EclipseError, cube.load_cube)

    def test_load_bad_filename_list(self):
        cube = eclipse.cube(filenames+['IM.fits'])
        self.failUnlessRaises(eclipse.EclipseError, cube.load_cube)

    def test_load_bad_filename_file(self):
        cube = eclipse.cube('FILELIST.DAT')
        self.failUnlessRaises(eclipse.EclipseError, cube.load_cube)

    def test_save_good(self):
        cube = eclipse.cube('IM01.fits')
        cube.save_cube('IM06.fits')
        self.failUnless(os.path.exists('IM06.fits'))
        os.remove('IM06.fits')

    def test_save_header_good(self):
        cube = eclipse.cube('IM01.fits')
        hdr = eclipse.header()
        hdr.default()
        hdr['AKEY'] = 1
        cube.save_cube('IM06.fits', hdr)
        self.failUnless(os.path.exists('IM06.fits'))
        os.remove('IM06.fits')

    def test_save_bad_no_image(self):
        cube = eclipse.cube()
        self.failUnlessRaises(eclipse.EclipseError,
                              cube.save_cube,
                              'IM06.fits')

    def test_save_bad_filename(self):
        cube = eclipse.cube('IM01.fits')
        self.failUnlessRaises(eclipse.EclipseError,
                              cube.save_cube,
                              'dddd/IM06.fits')
        
    def test_save_header_bad_filename(self):
        cube = eclipse.cube('IM01.fits')
        hdr = eclipse.header()
        hdr.default()
        hdr['AKEY'] = 1
        self.failUnlessRaises(eclipse.EclipseError,
                              cube.save_cube,
                              'dddd/IM06.fits', hdr)

    def test_copy(self):
        cube = eclipse.cube('IM01.fits')
        newcube = cube.copy()
        self.failUnless(cube.p_cube)
        self.failUnless(newcube.p_cube)

    def test_fastcopy_reloadable(self):
        cube = eclipse.cube('IM01.fits')
        newcube = cube.fast_copy()
        self.failIf(cube.p_cube)
        self.failUnless(newcube.p_cube)

    def test_fastcopy_not_reloadable(self):
        cube = eclipse.cube('IM01.fits').fast_copy()
        newcube = cube.fast_copy()
        self.failUnless(cube.p_cube)
        self.failUnless(newcube.p_cube)
        
class cube_statistics_tests(unittest.TestCase):
    def setUp(self):
        self.c = eclipse.cube('IM01.fits')
    def tearDown(self):
        self.c = None

    def test_stat(self):
        s = self.c.stat()

    def test_stat_opt_pixmap(self):
        pm = self.c.thresh_to_pixmap(0.8, 1.2)
        s = self.c.stat_opts(pixmap = pm)

    def test_stat_opt_range(self):
        s = self.c.stat_opts(pixrange=[0.8, 1.2])

    def test_stat_opt_zone(self):
        s = self.c.stat_opts(zone=[10, 20, 10, 20])

class cube_arithmetic_tests(unittest.TestCase):
    def setUp(self):
        self.c1 = eclipse.cube('IM01.fits')
        self.c2 = eclipse.cube('IM02.fits')
        self.bad = eclipse.cube('IM01.large.fits')
        
    def tearDown(self):
        self.c1 = None
        self.c2 = None
        
    def test_add(self):
        res = self.c1+self.c2
        self.failIf(self.c1.p_cube)
        self.failUnless(res.p_cube)
        self.failUnless(self.c2.p_cube)

    def test_radd(self):
        res = 1+self.c1
        self.failUnless(res.p_cube)
        
    def test_add_bad(self):
        self.failUnlessRaises(eclipse.EclipseError,
                              eval,
                              "self.c1+self.bad")

    def test_sub(self):
        res = self.c1-self.c2
        self.failIf(self.c1.p_cube)
        self.failUnless(res.p_cube)
        self.failUnless(self.c2.p_cube)

    def test_rsub(self):
        res = 1-self.c1
        self.failUnless(res.p_cube)

    def test_sub_bad(self):
        self.failUnlessRaises(eclipse.EclipseError,
                              eval,
                              "self.c1-self.bad")

    def test_mul(self):
        res = self.c1*self.c2
        self.failIf(self.c1.p_cube)
        self.failUnless(res.p_cube)
        self.failUnless(self.c2.p_cube)

    def test_rmul(self):
        res = 2*self.c1
        self.failUnless(res.p_cube)

    def test_mul_bad(self):
        self.failUnlessRaises(eclipse.EclipseError,
                              eval,
                              "self.c1*self.bad")

    def test_div(self):
        res = self.c1/self.c2
        self.failIf(self.c1.p_cube)
        self.failUnless(res.p_cube)
        self.failUnless(self.c2.p_cube)

    def test_zerodiv(self):
        self.failUnlessRaises(ZeroDivisionError,
                              eval,
                              "self.c1/0")

    def test_rdiv(self):
        res = 1/self.c1
        self.failUnless(res.p_cube)
           
    def test_div_bad(self):
        self.failUnlessRaises(eclipse.EclipseError,
                              eval,
                              "self.c1/self.bad")
        
    def test_pow(self):
        res = self.c1**0.5
        self.failIf(self.c1.p_cube)
        self.failUnless(res.p_cube)

    def test_pow_bad(self):
        self.failUnlessRaises(eclipse.EclipseError,
                              eval,
                              "self.c1**self.c2")

    def test_rpow(self):
        self.failUnlessRaises(eclipse.EclipseError,
                              eval,
                              "2**self.c1")

class cube_inplace_tests(unittest.TestCase):
    '''Test procedures that modify the cube in place''' 
    def setUp(self):
        self.cube = eclipse.cube(filenames)
    def tearDown(self):
        self.cube = None

    def test_norm_mean(self):
        self.cube.norm()

    def test_norm_scale(self):
        self.cube.norm("scale")

    def test_norm_flux(self):
        self.cube.norm("flux")

    def test_norm_aflux(self):
        self.cube.norm("aflux")

    def test_scale(self):
        self.cube.scale(100.0)

    def test_threshold(self):
        self.cube.threshold(0.0, 10.0, 0, 10.0)

    def test_threshold_bad(self):
        self.failUnlessRaises(eclipse.EclipseError,
                              self.cube.threshold,
                              0, -1, 0, 1)

    def test_apply_named_filter(self):
        self.cube.apply_named_filter('mean3', [0.0, 0.0])

    def test_median_filter(self):
        self.cube.median_filter()

class cube_to_cube_tests(unittest.TestCase):
    def setUp(self):
        self.cube = eclipse.cube(filenames)
    def tearDown(self):
        self.cube = None

    def test_index(self):
        newcube = self.cube[0]
        self.failUnlessEqual(len(newcube), 1)

    def test_bad_index(self):
        self.failUnlessRaises(IndexError,
                              eval,
                              "self.cube[6]")

    def test_slice(self):
        newcube = self.cube[1:4]
        self.failUnlessEqual(len(newcube), 3)

    def test_empty_slice(self):
        self.failUnlessRaises(eclipse.EclipseError,
                              eval,
                              "self.cube[3:1]")

    def test_bad_slice(self):
        self.failUnlessRaises(IndexError,
                              eval,
                              "self.cube[3.5:1.2]")


    def test_time_stdev(self):
        newcube = self.cube.time_stdev_on_cube()

    def test_average(self):
        newcube = self.cube.average()

    def test_average_with_rejection(self):
        self.cube.norm()
        newcube = self.cube.average_with_rejection(0.9, 1.1)

    def test_sum(self):
        newcube = self.cube.sum_cube_to_image()

    def test_median_average(self):
        self.cube.norm()
        newcube = self.cube.median_average()

    def test_cycle_average(self):
        self.cube.norm()
        newcube = self.cube.cycle_average(3)

    def test_cycle_sum(self):
        newcube = self.cube.cycle_sum(2)

    def test_cycle_median(self):
        newcube = self.cube.cycle_median(3)

    def test_running_average(self):
        newcube = self.cube.running_average(1)

    def test_running_sum(self):
        newcube = self.cube.running_sum(1)

    def test_running_median(self):
        newcube = self.cube.running_median(2)

    def test_extract_region(self):
        newcube = self.cube.extract_region(0, 0, 64, 64)

cube_test_suite = unittest.TestSuite()
cube_test_suite.addTest(unittest.makeSuite(cube_tests))
cube_test_suite.addTest(unittest.makeSuite(cube_statistics_tests))
cube_test_suite.addTest(unittest.makeSuite(cube_arithmetic_tests))
cube_test_suite.addTest(unittest.makeSuite(cube_inplace_tests))
cube_test_suite.addTest(unittest.makeSuite(cube_to_cube_tests))

class header_write_tests(unittest.TestCase):
    def setUp(self):
        self.hdr = eclipse.header()
        self.hdr.default()
        self.cube = eclipse.cube('IM01.fits')
    def tearDown(self):
        self.hdr = None
        self.cube = None

    def _test_write(self, val):
        '''Auxilary function to test writing header'''
        self.hdr['MYKEY'] = val
        self.failUnlessEqual(self.hdr['MYKEY'], val)
        self.cube.save_cube('tmp.fits', self.hdr)
        newhdr = eclipse.header('tmp.fits')
        self.failUnlessEqual(newhdr['MYKEY'], val)
        os.remove('tmp.fits')

    def test_write_int(self):
        self._test_write(1)

    def test_write_float(self):
        self._test_write(0.5)

    def test_write_str(self):
        self._test_write('abcd')
        
header_test_suite = unittest.TestSuite()
header_test_suite.addTest(unittest.makeSuite(header_write_tests))

if __name__ == '__main__':
    build_test_data()

    test_runner = unittest.TextTestRunner()

    print 'Testing cubes...'
    test_runner.run(cube_test_suite)
    print 'Testing headers...'
    test_runner.run(header_test_suite)
    
    delete_test_data()
