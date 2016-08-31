#!/usr/bin/env python
#
#Tests for the eclipse.py module
#R. Rengelink - April 2001
#

import eclipse
import os

def test_cube():
    print "Running tests on cube()..."
    
    print "Testing image generation and saving..."
    print "Generating 5 random 256x256 images and writing filelist.dat..."
    stat_list = []
    name_list = []
    filelist = open('filelist.dat', 'w')

    cg = eclipse.CubeGenerator(128, 128)
    for i in range(1,6):
        name = 'IM%2.2i.fits' % i
	print "Generating random image %s with mean=%5.2f and stdev=%5.2f" % (name, 1.0*i, 0.5*i)
	new_cube = cg.generate_random_gauss(0.5*i, 1.0*i)
	new_cube.save_cube(name)
	st = new_cube.stat()
	print 'Measured statistics (mean, stdev):', st.avg_pix, st.stdev

	stat_list.append(st)
	name_list.append(name)
	filelist.write(name+'\n')

    filelist.close()
    print
    
    print "Testing loading..."
    print "Loading", name_list, "into one cube"
    acube = eclipse.cube(name_list)
    print "Number of planes in cube :", len(acube)
    print

    print "Loading filelist.dat into one cube"
    acube = eclipse.cube('filelist.dat')
    print "Number of planes in cube :", len(acube)
    print
    
    print "Testing indexing and slicing..."
    print "Extracting each plane and computing statistics"
    for i in range(len(acube)):
        st = acube[i].stat()
        print "plane: %i, mean: %5.2f, stdev: %7.4f" % (i, st.avg_pix, st.stdev)
    print

    print 'Testing statistics'
    print acube.stat_plane().__dict__
    
    print "Extracting slice [1:4] and computing statistics"
    subcube = acube[1:4]
    stat = subcube.stat()
    for i in range(len(subcube)):
	print "plane %i, mean: %5.2f, stdev: %7.4f" % (i, stat[i].avg_pix, stat[i].stdev)
    print

    print "Testing averaging of slice[1:4]..."
    avcube = subcube.average()
    st = avcube.stat()
    print "Statistics of average: mean: %5.2f, stdev: %7.4f" % (st.avg_pix, st.stdev)

    print "Testing normalization..."
    acube.norm('mean')
    for i in range(len(acube)):
        st = acube[i].stat()
        print "plane: %i, mean: %5.2f, stdev: %7.4f" % (i, st.avg_pix, st.stdev)
    print

    print "Averaging with rejection..."
    av1cube = acube.average()
    av2cube = acube.average_with_rejection(1, 1)
    st1 = av1cube.stat()
    st2 = av2cube.stat()
    print "Statistics of average without rejection: mean: %5.2f, stdev: %7.4f" % (st.avg_pix, st.stdev)
    print "Statistics of average with rejetion    : mean: %5.2f, stdev: %7.4f" % (st.avg_pix, st.stdev)

    
    print "Testing image arithmetic..."

    print "a, b, c = cube('IM01.fits'), cube('IM02.fits'), cube('IM03.fits')"
    print "d = a+b+c"
    print "e = d+b+c"
    print "a += b (a=a+b)"
    a, b, c = eclipse.cube('IM01.fits'), eclipse.cube('IM02.fits'), eclipse.cube('IM03.fits')

    d = a+b+c
    e = d+b+c
    a += b
    st_d = d.stat()
    st_e = e.stat()
    st_a = a.stat()
    print "mean d =", st_d.avg_pix, " e =", st_e.avg_pix, " a =", st_a.avg_pix

def test_pixelmap():
    print "Testing pixelmaps..."
    cg = eclipse.CubeGenerator(256, 256)
    acube = cg.generate_random_gauss(0.5, 1.0)
    pmap1 = acube.thresh_to_pixmap(2.0, 5.0)
    pmap1.dump_pixelmap('PM.fits')
    pmap2 = acube.thresh_to_pixmap(-5.0, 1.0)

    pmap3 = pmap1 & pmap2
    pmap4 = pmap1 | pmap2
    pmap5 = ~pmap1

    bcube = acube*pmap1.as_cube()
    bcube.save_cube(filename='IM.thresh.fits')

def test_header():
    print "\nTesting headers..."
    print "Generating and saving new cube IMH1.fits..."
    cg = eclipse.CubeGenerator(256, 256)
    acube = cg.generate_random_gauss(0.5, 1.0)
    acube.save_cube('IMH1.fits')

    print "Reading header from IMH1.fits..."
    h = eclipse.header(filename='IMH1.fits')
    print "Writing new key NEWKEY with value 12345.0...."
    h['NEWKEY'] = 12345.0
    print "Saving cube to IMH2.fits with new header..."
    acube.save_cube('IMH2.fits', h)
    print "Reading NEWKEY from the header of IMH2.fits..."
    h = eclipse.header(filename='IMH2.fits')
    print h['NEWKEY']

    print "Copying a header..."
    h  = eclipse.header(filename='IMH1.fits')
    new_h = h.copy()

    print "Writing a defualt header..."
    h = eclipse.header().default()
    acube.save_cube('IMH2.fits', h)
    h = eclipse.header(filename='IMH2.fits')
    if h['BITPIX'] != -32:
        print 'Error... BITPIX =', h['BITPIX']

def cleanup_tempfiles():
	print "cleaning up temporary files..."
	tmpfiles = ["IM.thresh.fits",
				"IM01.fits",
				"IM02.fits",
				"IM03.fits",
				"IM04.fits",
				"IM05.fits",
				"IMH1.fits",
				"IMH2.fits",
				"PM.fits",
				"filelist.dat"]
	for tempfile in tmpfiles:
		try:
			os.remove(tempfile)
		except OSError:
			pass
    
if __name__ == '__main__':
    test_cube()
    test_pixelmap()
    test_header()
    cleanup_tempfiles()
    print 'Finished...'




