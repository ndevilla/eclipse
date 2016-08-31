#
# eclipse jitter Makefile
# N. Devillard
#
#

#
# General definitions
#

include ../../config.make

PFITSDIR=../../ins/pfits

IFLAGS  = -I. -I../include -I$(QFITSDIR)/include -I$(PFITSDIR)/include

LD      = ld
LDFLAGS = -L../lib -L$(QFITSDIR)/lib -L$(PFITSDIR)/lib $(LFLAGS)
LDLIBS  = -leclipse -lpfits -lqfits -lm $(LDLIBSLOCAL)

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

default: all

all: $(BINDIR)/jitter

clean:
	@(echo "no objects to clean here...")

veryclean:
	@(echo "cleaning jitter binaries...")
	@($(RM) $(BINDIR)/jitter)


SRCS =	jcalib.c \
	   	jconfig.c \
		jengine.c \
		jgui.c \
		jini.c \
		jload.c \
		jmain.c \
		jpproc.c \
		jsaa.c \
		jsave.c \
		jsky.c

$(BINDIR)/jitter:	$(SRCS)
	@(echo "building $@ ...")
	@($(CC) $(COMPF) $(SRCS) -o $(BINDIR)/jitter $(LINKF))

