#
# eclipse/conica Makefile
# N. Devillard
#

# Path to eclipse library
ELIB=../../src

# Path to pfits library
PFITSLIB=../pfits

#
# General definitions are all here
#

SUFFIXES = .o .c .h
OUTPUT_OPTION= -o $@
IFLAGS  = 	-I$(ELIB)/include -I./common -I./include -I$(QFITSDIR)/include \
			-I$(PFITSLIB)/include

LD      = ld
LDFLAGS = -Llib -L$(ELIB)/lib -L$(QFITSDIR)/lib -L$(PFITSLIB)/lib \
		$(LFLAGS)
LDLIBS  = -leclipse -lpfits -lqfits -lm $(LDLIBSLOCAL)
RM      = rm -f
RRM     = rm -rf
CP      = cp
MV      = mv

AR      = ar
ARFLAGS = rcvs

#
# Redefinition of implicit rules
#

SUFFIXES = .o .c .h .a .so .sl

COMPILE.c=$(CC) $(CFLAGS) $(FTRACE) -DIN_ECLIPSE $(IFLAGS) -c
LINK.c=$(CC) $(CFLAGS) $(IFLAGS) -DIN_ECLIPSE $(LDFLAGS)

.c:
	@($(LINK.c) -o $@ $< $(LDLIBS))
.c.o:
	@(echo "compiling $< ...")
	@($(COMPILE.c) -o $@ $<)

COMPF  = $(CFLAGS) $(IFLAGS) -DIN_ECLIPSE
LINKF  = $(LDFLAGS) $(LDLIBS) -DIN_ECLIPSE



#
# Eclipse directories
#

BINDIR = ../../bin


# Load compilation options (valid for all compiled sources hereafter) 
include $(ELIB)/../config.make


default: all

all: 	conica-lib \
		conica-recipes

clean:
	@(echo "cleaning CONICA object files...")
	@($(RM) */*.o)

veryclean:
	@(echo "cleaning CONICA recipes and object files...")
	@($(RM) */*.o lib/libconicap.a $(BINDIR)/conicap)

#
# CONICA library:
#

SRC_CO = common/utils.c \
		 common/filters.c \
		 common/prokeys.c 

OBJ_CO = $(SRC_CO:.c=.o)

conica-lib: lib/libconicap.a 

lib/libconicap.a: $(OBJ_CO)
	@(echo "Building conicap library...")
	@($(AR) $(ARFLAGS) lib/libconicap.a $(OBJ_CO))

#
# *************  CONICA recipes
#

conica-recipes:		recipe_conicap

SRC_CONICAP =		recipes/check-focus.c \
			   		recipes/dark.c \
					recipes/detlin.c \
					recipes/lampflat.c \
					recipes/qc-strehl.c \
					recipes/slitpos.c \
					recipes/twflat.c \
					recipes/zpoint.c

OBJ_CONICAP = $(SRC_CONICAP:.c=.o)

recipe_conicap:	$(BINDIR)/conicap

$(BINDIR)/conicap: conicap/conicap.o $(OBJ_CONICAP) $(OBJ_CO)
	@(echo "compiling $@ ...")
	@($(CC) $(COMPF) -o $(BINDIR)/conicap conicap/conicap.c \
	$(OBJ_CO) $(OBJ_CONICAP) $(LINKF))
	@($(STRIP) $(BINDIR)/conicap)


# end of file
