#
# ISAAC spjitter Makefile
# Y. Jung
#

#
# General definitions
#

include ../../../config.make

PFITSDIR= ../../pfits
ELIB    = ../../../src

IFLAGS  = -I. -I../common -I$(ELIB)/include -I$(QFITSDIR)/include \
		  -I$(PFITSDIR)/include

LDFLAGS = -L../lib -L$(ELIB)/lib -L$(QFITSDIR)/lib -L$(PFITSDIR)/lib $(LFLAGS)
LDLIBS  = -lisaacp -leclipse -lpfits -lqfits -lm $(LDLIBSLOCAL)

AR      = ar
ARFLAGS = rcvs

RM      = rm -f
RRM     = rm -rf
CP      = cp
MV      = mv

BINDIR = ../../../bin

#
# Redefinition of implicit rules
#

SUFFIXES = .o .c .h .a .so .sl

COMPILE.c=$(CC) $(CFLAGS) $(FTRACE) $(RELOC) $(IFLAGS) $(OSFLAG) -c
LINK.c=$(CC) $(CFLAGS) $(IFLAGS) $(LDFLAGS)

.c:
	@($(LINK.c) -o $@ $< $(LDLIBS))
.c.o:
	@(echo "compiling $< ...")
	@($(COMPILE.c) -o $@ $<)

COMPF  = $(CFLAGS) $(IFLAGS) $(FTRACE)
LINKF  = $(LDFLAGS) $(LDLIBS)

default: all

all: $(BINDIR)/spjitter

clean:
	@(echo "no objects to clean here...")

veryclean:
	@(echo "cleaning spjitter binaries...")
	@($(RM) $(BINDIR)/spjitter)


SRCS =	spjcalib.c \
	   	spjclassif.c \
	   	spjconfig.c \
		spjengine.c \
		spjextract.c \
		spjgui.c \
		spjini.c \
		spjload.c \
		spjmain.c \
		spjsaa.c \
		spjsave.c

$(BINDIR)/spjitter:	$(SRCS)
	@(echo "building $@ ...")
	@($(CC) $(COMPF) $(SRCS) -o $(BINDIR)/spjitter $(LINKF))

