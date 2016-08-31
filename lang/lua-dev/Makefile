#
# eclipse/lua-dev Makefile
# N. Devillard
#

# Path to eclipse lib
ELIB=../../src

#
# General definitions are all here
# Directory locations are given relatively from the lua-dev/ dir.

SUFFIXES = .o .c .h
OUTPUT_OPTION= -o $@
IFLAGS  = -I$(ELIB)/include -I.  -I../lua/include -I$(QFITSDIR)/include

LD      = ld
LDFLAGS = -L$(ELIB)/lib -L$(QFITSDIR)/lib $(LFLAGS)
LDLIBS  = -leclipse -lqfits -llua -lm $(LDLIBSLOCAL)
RM      = rm -f
RRM     = rm -rf
CP      = cp
MV      = mv

COMPILE.c=$(CC) $(CFLAGS) $(FTRACE) $(IFLAGS) -c
LINK.c=$(CC) $(CFLAGS) $(IFLAGS) $(LDFLAGS)
.c:
	$(LINK.c) $(OUTPUT_OPTION) $< $(LDLIBS)
.c.o:
	$(COMPILE.c) $(OUTPUT_OPTION) $<
.c.a:
	$(COMPILE.c) -o $% $<
	$(AR) $(ARFLAGS) $@ $

COMPF  = $(CFLAGS) $(IFLAGS) $(FTRACE)
LINKF  = $(LDFLAGS) $(LDLIBS)

ALLOBJS =

#
# Eclipse directories
#
BINDIR = ../../bin


# Load compilation options (valid for all compiled sources hereafter) 
include $(ELIB)/../config.make


default: all


all:	$(BINDIR)/eclipse

clean:
	@(echo "cleaning lua-dev object files")
	@($(RM) ./src/*.o)

veryclean:
	@(echo "cleaning lua-dev object files and interpreter program")
	@($(RM) $(BINDIR)/eclipse ./src/*.o)

SRCS = 	src/arith.c \
		src/collapse.c \
		src/cube.c \
		src/deadpix.c \
		src/filter.c \
		src/fits.c \
		src/framelist.c \
		src/merge.c \
		src/pixmap.c \
		src/stats.c


$(BINDIR)/eclipse: eclipse.c eclipse-lua.c $(SRCS)
	@(echo "compiling $@ ...")
	@($(CC) $(COMPF) eclipse.c eclipse-lua.c -o $(BINDIR)/eclipse $(LINKF))

# end of file
