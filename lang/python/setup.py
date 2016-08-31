#!/usr/bin/env python
#
# Python makefile for the eclipse-python module
# N. Devillard - April 2001
# created
#
# R. Rengelink - Jan 2002
# rewrite for distutils
#

from distutils.core import  setup, Extension
import os, os.path, glob

#abspath is required because the linker chokes on relative paths in
#distutils (os.spawn problem??)

# Returns the first path in 'path_list' that contains all the files
# listed in 'file_list'.
def find_files(path_list, file_list):
    for path in path_list:
        for file in file_list:
            if not os.path.exists(os.path.join(path, file)):
                break
        else:
            # No break encountered
            return path
    return None


libdirs = []

# Locate qfits
qfitsdir = find_files(["../../qfits",
                       "../../../qfits",
                       "/usr",
                       "/usr/local",
                       "/usr/local/qfits",
                       "/opt/qfits"],
                      ["include/qfits.h",
                       "lib/libqfits.a"])

if qfitsdir:
    qfitsdir = os.path.abspath(qfitsdir)
    print "found qfits in", qfitsdir
    qfitsinc = os.path.join(qfitsdir, "include")
    qfitslib = os.path.join(qfitsdir, "lib")
    libdirs.append(qfitslib)
else:
    print "WARNING cannot find qfits on this system"
    qfitsinc = qfitslib = ''


# To make sure we build with python compiler options, we rebuild
# the library
eclipse_incl = os.path.abspath('../../src/include')
eclipse_src = ['src/c_eclipse_wrap.c']
eclipse_src.extend(glob.glob('../../src/iproc/*.c'))
eclipse_src.extend(glob.glob('../../src/unix/*.c'))
eclipse_src.extend(glob.glob('../../src/math/*.c'))
eclipse_src.extend(glob.glob('../../src/spectro/*.c'))


setup(name='Pyeclipse',
      version='0.1',
      description='Eclipse Pysthon bindings',
      package_dir = {'': 'lib'},
      py_modules = ['eclipse', 'eclipse_test', 'eclipse_unittest'],
      ext_modules = [Extension("c_eclipse", eclipse_src,
                               define_macros=[('_ECLIPSE_', None)],
                               include_dirs=[qfitsinc, eclipse_incl],
                               library_dirs=libdirs,
                               libraries=['qfits', 'm'])])
                     
