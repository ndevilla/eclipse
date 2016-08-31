#
# eclipse/pfits Makefile
# N. Devillard
#

# Path to eclipse library
ELIB=../../src

# Load compilation options (valid for all compiled sources hereafter) 
include $(ELIB)/../config.make

#
# General definitions are all here
#

SUFFIXES = .o .c .h
OUTPUT_OPTION= -o $@
IFLAGS  = 	-I./src -I./include -I$(QFITSDIR)/include

LD      = ld
LDFLAGS = -L$(QFITSDIR)/lib $(LFLAGS)
LDLIBS  = -lqfits $(LDLIBSLOCAL)
RM      = rm -f
RRM     = rm -rf
CP      = cp
MV      = mv

AR		= ar
ARFLAGS = rcvs

#
# Redefinition of implicit rules
#

SUFFIXES = .o .c .h .a .so .sl

COMPILE.c=$(CC) $(CFLAGS) $(FTRACE) $(IFLAGS) -c

.c:
	@($(LINK.c) -o $@ $< $(LDLIBS))
.c.o:
	@(echo "compiling $< ...")
	@($(COMPILE.c) -o $@ $<)

COMPF  = $(CFLAGS) $(IFLAGS)
LINKF  = $(LDFLAGS) $(LDLIBS)


default: all

all: 	libpfits

clean:
	@(echo "cleaning pfits object files...")
	@($(RM) src/*.o)

veryclean:
	@(echo "cleaning pfits library and object files...")
	@($(RM) src/*.o lib/libpfits.a)

#
# pfits library
#

SRC_PFITS =	src/do_catg.c \
			src/key_isaac.c \
			src/key_naco.c \
			src/pfits.c \
			src/products.c

OBJ_PFITS = $(SRC_PFITS:.c=.o)

libpfits: lib/libpfits.a

lib/libpfits.a: $(OBJ_PFITS)
	@(echo "Building pfits library...")
	@($(AR) $(ARFLAGS) lib/libpfits.a $(OBJ_PFITS))

#
# Test program: main is directly present inside pfits.c
#

test: src/test_pfits.c
	$(CC) $(CFLAGS) -I./include -I$(QFITSDIR)/include \
	-L$(QFITSDIR)/lib -L./lib -o test_pfits src/test_pfits.c -lpfits -lqfits

