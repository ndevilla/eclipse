#
# eclipse global Makefile
# N. Devillard
#

#
# General definitions
#

include ../config.make

IFLAGS  = -I./include

LD      = ld
LDFLAGS = -L./lib -L$(QFITSDIR)/lib $(LFLAGS)
LDLIBS  = -leclipse -lqfits -lm $(LDLIBSLOCAL)

AR      = ar
ARFLAGS = rcvs

RM      = rm -f
RRM     = rm -rf
CP      = cp
MV      = mv

#
# Redefinition of implicit rules
#

SUFFIXES = .o .c .h .a .so .sl

COMPILE.c=$(CC) $(CFLAGS) -I$(QFITSDIR)/include $(FTRACE) $(RELOC) -D_ECLIPSE_ $(IFLAGS) $(OSFLAG) -c
LINK.c=$(CC) $(CFLAGS) $(IFLAGS) -D_ECLIPSE_ $(LDFLAGS)

.c:
	@($(LINK.c) -o $@ $< $(LDLIBS))
.c.o:
	@(echo "compiling $< ...")
	@($(COMPILE.c) -o $@ $<)

COMPF  = $(CFLAGS) $(IFLAGS)
LINKF  = $(LDFLAGS) $(LDLIBS)

#
# Compiler stuff are defined outside, depending on the platform
#

#
# Eclipse directories
#

default: all

all: 	eclipse \
		commands \
		cp_scripts \
		../bin/e_setup \
		finish

finish:
	@(echo "")
	@(echo "")
	@(echo "finished compilation")
	@(echo "Type bin/e_setup to setup your environment now.")
	@(echo "")
	@(echo "")

clean:
	@(echo "cleaning object files...")
	@($(RM) */*.o */*/*.o)

veryclean:
	@(echo "cleaning object files and libraries...")
	@($(RM) */*.o */*/*.o ./lib/*.a ./lib/*.s*)
	@(cd ./main ; $(MAKE) veryclean ; cd ..)
	@(cd ./jitter ; $(MAKE) veryclean ; cd ..)

#
# The Eclipse library
#

SRCS =	iproc/corner.c \
		iproc/cube2image.c \
		iproc/cube_arith.c \
		iproc/cube_filters.c \
		iproc/cube_handling.c \
		iproc/cube_load.c \
		iproc/cube_save.c \
		iproc/dead_pixels.c \
		iproc/detect.c \
		iproc/detect_ks.c \
		iproc/detect_sq.c \
		iproc/detector.c \
		iproc/extraction.c \
		iproc/fourier.c \
		iproc/framelist.c \
		iproc/generate.c \
		iproc/histogram.c \
		iproc/history.c \
		iproc/image_arith.c \
		iproc/image_filters.c \
		iproc/image_handling.c \
		iproc/image_intops.c \
		iproc/image_io.c \
		iproc/image_rtd.c \
		iproc/image_stats.c \
		iproc/intimage.c \
		iproc/irstd.c \
		iproc/matchpoint.c \
		iproc/photometry.c \
		iproc/pixel_handling.c \
		iproc/pixelmaps.c \
		iproc/resampling.c \
		iproc/shift.c \
		iproc/slitposition.c \
		iproc/xcorrelation.c \
		math/chebyshev.c \
		math/doubles.c \
		math/dstats.c \
		math/fft_base.c \
		math/fit_curve.c \
		math/function_1d.c \
		math/ipow.c \
		math/legendre.c \
		math/matrix.c \
		math/median.c \
		math/optimization.c \
		math/poly2d.c \
		math/polygon.c \
		math/random.c \
		spectro/spectral_lines.c \
		spectro/spectro_arcs.c \
		spectro/spectro_detect.c \
		spectro/spectro_wave.c \
		unix/calendar.c \
		unix/charmatrix.c \
		unix/chronos.c \
		unix/cmdline.c \
		unix/comm.c \
		unix/dictionary.c \
		unix/e_config.c \
		unix/e_version.c \
		unix/file_handling.c \
		unix/filelock.c \
		unix/filename.c \
		unix/filesys.c \
		unix/getopt.c \
		unix/gnuplot_i.c \
		unix/iniparser.c \
		unix/manpage.c \
		unix/memstr.c \
		unix/parse_tok.c \
		unix/pid_i.c \
		unix/ptrace.c \
		unix/rtd_i.c \
		unix/show.c \
		unix/strlib.c \
		unix/t_iso8601.c \
		unix/t_stamp.c \
		unix/userid.c

OBJS = $(SRCS:.c=.o)

#
# Main eclipse library is built here in static mode (libeclipse.a)
#

eclipse: ./lib/libeclipse.a
	
./lib/libeclipse.a: $(OBJS)
	@(echo "Building eclipse library file ...")
	@($(AR) $(ARFLAGS) ./lib/libeclipse.a $(OBJS))

#
# eclipse setup tool
#

../bin/e_setup: sysconfig/e_setup.c
	@(echo "compiling $@ ...")
	@($(CC) $(CFLAGS) $(OSFLAG) -o ../bin/e_setup sysconfig/e_setup.c)

#
# commands: executable routines
#

commands:
	@(if test -d ./main ; then\
	(cd ./main ; $(MAKE))\
	else (true) fi)
	@(if test -d ./jitter ; then\
	(cd ./jitter ; $(MAKE))\
	else (true) fi)

#
# copy scripts to ../bin
#

cp_scripts:
	@(if test ! -f ../bin/load_rtd; then\
	  (cp ./scripts/load_rtd ../bin ;\
	  chmod a+x ../bin/load_rtd) ;\
	  else (true) fi)
	@(if test ! -f ../bin/load_rtd-98; then\
	  (cp ./scripts/load_rtd-98 ../bin ;\
	  chmod a+x ../bin/load_rtd-98) ;\
	  else (true) fi)
	@(if test ! -f ../bin/load_rtd-99; then\
	  (cp ./scripts/load_rtd-99 ../bin ;\
	  chmod a+x ../bin/load_rtd-99) ;\
	  else (true) fi)
	@(if test ! -f ../bin/midas_status.csh; then\
	  (cp ./scripts/midas_status.csh ../bin ;\
	  chmod a+x ../bin/midas_status.csh) ;\
	  else (true) fi)
