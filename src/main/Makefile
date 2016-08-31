
#
# eclipse commands Makefile
# N. Devillard
#
#

#
# General definitions
#

include ../../config.make

IFLAGS  = -I../dfs -I../include -I$(QFITSDIR)/include

LD      = ld
LDFLAGS = -L../lib -L$(QFITSDIR)/lib $(LFLAGS)
LDLIBS  = -leclipse -lqfits -lm $(LDLIBSLOCAL)

AR      = ar
ARFLAGS = rcvs

RM      = rm -f
RRM     = rm -rf
CP      = cp
MV      = mv

BINDIR = ../../bin

#
# Redefinition of implicit rules
#

SUFFIXES = .o .c .h .a .so .sl

COMPILE.c=$(CC) $(CFLAGS) $(FTRACE) $(RELOC) -D_ECLIPSE_ $(IFLAGS) $(OSFLAG) -c
LINK.c=$(CC) $(CFLAGS) $(IFLAGS) -D_ECLIPSE_ $(LDFLAGS)

.c:
	@($(LINK.c) -o $@ $< $(LDLIBS))
.c.o:
	@(echo "compiling $< ...")
	@($(COMPILE.c) -o $@ $<)

COMPF  = $(CFLAGS) $(IFLAGS) $(FTRACE)
LINKF  = $(LDFLAGS) $(LDLIBS)

PROGS =	$(BINDIR)/3color \
		$(BINDIR)/average \
		$(BINDIR)/catcube \
		$(BINDIR)/ccube \
		$(BINDIR)/collapse \
		$(BINDIR)/deadpix \
		$(BINDIR)/distortion \
		$(BINDIR)/dumppix \
		$(BINDIR)/encircl \
		$(BINDIR)/extract \
		$(BINDIR)/extract_spec \
		$(BINDIR)/fft \
		$(BINDIR)/filt \
		$(BINDIR)/fit \
		$(BINDIR)/fitsppm \
		$(BINDIR)/flat \
		$(BINDIR)/histog \
		$(BINDIR)/imgen \
		$(BINDIR)/ipaste \
		$(BINDIR)/norm \
		$(BINDIR)/oddeven \
		$(BINDIR)/peak \
		$(BINDIR)/poisson \
		$(BINDIR)/setpix \
		$(BINDIR)/speedtest \
		$(BINDIR)/stcube \
		$(BINDIR)/std \
		$(BINDIR)/strehl \
		$(BINDIR)/thresh \
		$(BINDIR)/warping \
		$(BINDIR)/xcorr1d \
		$(BINDIR)/xcorr2d

QFITSPROGS = $(BINDIR)/dfits \
			 $(BINDIR)/dtfits \
			 $(BINDIR)/fitsmd5 \
			 $(BINDIR)/fitsort \
			 $(BINDIR)/flipx \
			 $(BINDIR)/frameq \
			 $(BINDIR)/hierarch28 \
			 $(BINDIR)/iofits \
			 $(BINDIR)/replacekey \
			 $(BINDIR)/qextract

default: all

all: $(PROGS) $(QFITSPROGS)

clean:
	@(echo "no objects to clean here...")

veryclean:
	@(echo "cleaning eclipse programs...")
	@($(RM) $(PROGS) $(QFITSPROGS))


#
# Programs belonging to eclipse
#

$(BINDIR)/3color:	3color.c
	@(echo "building $@ ...")
	@($(CC) $(COMPF) 3color.c -o $(BINDIR)/3color $(LINKF))

$(BINDIR)/average:	average.c
	@(echo "building $@ ...")
	@($(CC) $(COMPF) average.c -o $(BINDIR)/average $(LINKF))

$(BINDIR)/catcube:	catcube.c
	@(echo "building $@ ...")
	@($(CC) $(COMPF) catcube.c -o $(BINDIR)/catcube $(LINKF))

$(BINDIR)/ccube:	ccube.c
	@(echo "building $@ ...")
	@($(CC) $(COMPF) ccube.c -o $(BINDIR)/ccube $(LINKF))

$(BINDIR)/collapse:	collapse.c
	@(echo "building $@ ...")
	@($(CC) $(COMPF) collapse.c -o $(BINDIR)/collapse $(LINKF))

$(BINDIR)/deadpix:	deadpix.c
	@(echo "building $@ ...")
	@($(CC) $(COMPF) deadpix.c -o $(BINDIR)/deadpix $(LINKF))

$(BINDIR)/distortion:	distortion.c
	@(echo "building $@ ...")
	@($(CC) $(COMPF) distortion.c -o $(BINDIR)/distortion $(LINKF))

$(BINDIR)/dumppix:	dumppix.c
	@(echo "building $@ ...")
	@($(CC) $(COMPF) dumppix.c -o $(BINDIR)/dumppix $(LINKF))

$(BINDIR)/encircl:	encircl.c
	@(echo "building $@ ...")
	@($(CC) $(COMPF) encircl.c -o $(BINDIR)/encircl $(LINKF))

$(BINDIR)/extract:	extract.c
	@(echo "building $@ ...")
	@($(CC) $(COMPF) extract.c -o $(BINDIR)/extract $(LINKF))

$(BINDIR)/extract_spec: extract_spec.c
	@(echo "building $@ ...")
	@($(CC) $(COMPF) extract_spec.c -o $(BINDIR)/extract_spec $(LINKF))

$(BINDIR)/fft:	fft.c
	@(echo "building $@ ...")
	@($(CC) $(COMPF) fft.c -o $(BINDIR)/fft $(LINKF))

$(BINDIR)/filt:	filt.c
	@(echo "building $@ ...")
	@($(CC) $(COMPF) filt.c -o $(BINDIR)/filt $(LINKF))

$(BINDIR)/fit:	fit.c
	@(echo "building $@ ...")
	@($(CC) $(COMPF) fit.c -o $(BINDIR)/fit $(LINKF))

$(BINDIR)/fitsppm:	fitsppm.c
	@(echo "building $@ ...")
	@($(CC) $(COMPF) fitsppm.c -o $(BINDIR)/fitsppm $(LINKF))

$(BINDIR)/flat:	flat.c
	@(echo "building $@ ...")
	@($(CC) $(COMPF) flat.c -o $(BINDIR)/flat $(LINKF))

$(BINDIR)/histog:	histog.c
	@(echo "building $@ ...")
	@($(CC) $(COMPF) histog.c -o $(BINDIR)/histog $(LINKF))

$(BINDIR)/imgen:	imgen.c
	@(echo "building $@ ...")
	@($(CC) $(COMPF) imgen.c -o $(BINDIR)/imgen $(LINKF))

$(BINDIR)/ipaste:	ipaste.c
	@(echo "building $@ ...")
	@($(CC) $(COMPF) ipaste.c -o $(BINDIR)/ipaste $(LINKF))

$(BINDIR)/norm:	norm.c
	@(echo "building $@ ...")
	@($(CC) $(COMPF) norm.c -o $(BINDIR)/norm $(LINKF))

$(BINDIR)/oddeven:	oddeven.c
	@(echo "building $@ ...")
	@($(CC) $(COMPF) oddeven.c -o $(BINDIR)/oddeven $(LINKF))

$(BINDIR)/peak:	peak.c
	@(echo "building $@ ...")
	@($(CC) $(COMPF) peak.c -o $(BINDIR)/peak $(LINKF))

$(BINDIR)/poisson:	poisson.c
	@(echo "building $@ ...")
	@($(CC) $(CFLAGS) $(LFLAGS) poisson.c -o $(BINDIR)/poisson)

$(BINDIR)/setpix:	setpix.c
	@(echo "building $@ ...")
	@($(CC) $(COMPF) setpix.c -o $(BINDIR)/setpix $(LINKF))

$(BINDIR)/speedtest:	speedtest.c
	@(echo "building $@ ...")
	@($(CC) $(COMPF) speedtest.c -o $(BINDIR)/speedtest $(LINKF))

$(BINDIR)/stcube:	stcube.c
	@(echo "building $@ ...")
	@($(CC) $(COMPF) stcube.c -o $(BINDIR)/stcube $(LINKF))

$(BINDIR)/std: std.c
	@(echo "building $@ ...")
	@($(CC) $(COMPF) -o $(BINDIR)/std std.c $(LINKF))

$(BINDIR)/strehl:	strehl.c
	@(echo "building $@ ...")
	@($(CC) $(COMPF) strehl.c -o $(BINDIR)/strehl $(LINKF))

$(BINDIR)/thresh:	thresh.c
	@(echo "building $@ ...")
	@($(CC) $(COMPF) thresh.c -o $(BINDIR)/thresh $(LINKF))

$(BINDIR)/warping:	warping.c
	@(echo "building $@ ...")
	@($(CC) $(COMPF) warping.c -o $(BINDIR)/warping $(LINKF))

$(BINDIR)/xcorr1d:	xcorr1d.c
	@(echo "building $@ ...")
	@($(CC) $(COMPF) xcorr1d.c -o $(BINDIR)/xcorr1d $(LINKF))

$(BINDIR)/xcorr2d:	xcorr2d.c
	@(echo "building $@ ...")
	@($(CC) $(COMPF) xcorr2d.c -o $(BINDIR)/xcorr2d $(LINKF))


#
# Programs copied over from qfits
#

$(BINDIR)/dfits:
	@(if test -x $(QFITSDIR)/bin/dfits ; then\
	(echo "copying dfits from $(QFITSDIR)/bin to $(BINDIR)") ; \
	cp -f $(QFITSDIR)/bin/dfits $(BINDIR) ; \
	else (true) fi)

$(BINDIR)/dtfits:
	@(if test -x $(QFITSDIR)/bin/dtfits ; then\
	(echo "copying dtfits from $(QFITSDIR)/bin to $(BINDIR)") ; \
	cp -f $(QFITSDIR)/bin/dtfits $(BINDIR) ; \
	else (true) fi)

$(BINDIR)/fitsmd5:
	@(if test -x $(QFITSDIR)/bin/fitsmd5 ; then\
	(echo "copying fitsmd5 from $(QFITSDIR)/bin to $(BINDIR)") ; \
	cp -f $(QFITSDIR)/bin/fitsmd5 $(BINDIR) ; \
	else (true) fi)

$(BINDIR)/fitsort:
	@(if test -x $(QFITSDIR)/bin/fitsort ; then\
	(echo "copying fitsort from $(QFITSDIR)/bin to $(BINDIR)") ; \
	cp -f $(QFITSDIR)/bin/fitsort $(BINDIR) ; \
	else (true) fi)

$(BINDIR)/flipx:
	@(if test -x $(QFITSDIR)/bin/flipx ; then\
	(echo "copying flipx from $(QFITSDIR)/bin to $(BINDIR)") ; \
	cp -f $(QFITSDIR)/bin/flipx $(BINDIR) ; \
	else (true) fi)

$(BINDIR)/frameq:
	@(if test -x $(QFITSDIR)/bin/frameq ; then\
	(echo "copying frameq from $(QFITSDIR)/bin to $(BINDIR)") ; \
	cp -f $(QFITSDIR)/bin/frameq $(BINDIR) ; \
	else (true) fi)

$(BINDIR)/hierarch28:
	@(if test -x $(QFITSDIR)/bin/hierarch28 ; then\
	(echo "copying hierarch28 from $(QFITSDIR)/bin to $(BINDIR)") ; \
	cp -f $(QFITSDIR)/bin/hierarch28 $(BINDIR) ; \
	else (true) fi)

$(BINDIR)/iofits:
	@(if test -x $(QFITSDIR)/bin/iofits ; then\
	(echo "copying iofits from $(QFITSDIR)/bin to $(BINDIR)") ; \
	cp -f $(QFITSDIR)/bin/iofits $(BINDIR) ; \
	else (true) fi)

$(BINDIR)/replacekey:
	@(if test -x $(QFITSDIR)/bin/replacekey ; then\
	(echo "copying replacekey from $(QFITSDIR)/bin to $(BINDIR)") ; \
	cp -f $(QFITSDIR)/bin/replacekey $(BINDIR) ; \
	else (true) fi)

$(BINDIR)/qextract:
	@(if test -x $(QFITSDIR)/bin/qextract ; then\
	(echo "copying qextract from $(QFITSDIR)/bin to $(BINDIR)") ; \
	cp -f $(QFITSDIR)/bin/qextract $(BINDIR) ; \
	else (true) fi)

