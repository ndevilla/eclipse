#
# eclipse/isaac Makefile
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
LDFLAGS = -L$(ELIB)/lib -L./lib -L$(QFITSDIR)/lib -L$(PFITSLIB)/lib \
		  $(LFLAGS) 
LDLIBS  = -lisaacp -leclipse -lpfits -lqfits -lm $(LDLIBSLOCAL)
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

all: 	isaac-lib \
		isaac-recipes \
		isaac-spjitter

clean:
	@(echo "cleaning ISAAC object files...")
	@($(RM) */*.o)
	@(cd ./spjitter ; $(MAKE) clean ; cd ..)

veryclean:
	@(echo "cleaning ISAAC recipes and object files...")
	@($(RM) */*.o lib/libisaacp.a $(BINDIR)/isaacp)
	@(cd ./spjitter ; $(MAKE) veryclean ; cd ..)

#
# ISAAC library
#

SRC_IS = common/classif.c \
		 common/deghost.c \
		 common/distortion.c \
		 common/filters.c \
		 common/lwload.c \
		 common/prokeys.c \
		 common/utils.c \
		 common/wavelength.c

OBJ_IS = $(SRC_IS:.c=.o)

isaac-lib: lib/libisaacp.a

lib/libisaacp.a: $(OBJ_IS)
	@(echo "Building isaacp library...")
	@($(AR) $(ARFLAGS) lib/libisaacp.a $(OBJ_IS))

#
# *************  ISAAC recipes
#

isaac-recipes:		recipe_isaacp

SRC_ISAACP =		recipes/arc.c \
			recipes/dark.c \
			recipes/ghost.c \
			recipes/illum.c \
			recipes/detlin.c \
			recipes/rename.c \
			recipes/respfunc.c \
			recipes/skybg.c \
			recipes/slitpos.c \
			recipes/sp_flat.c\
			recipes/startrace.c \
			recipes/twflat.c \
			recipes/wavecal.c \
			recipes/wltest.c \
			recipes/zpoint.c

OBJ_ISAACP = $(SRC_ISAACP:.c=.o)

recipe_isaacp:	$(BINDIR)/isaacp

$(BINDIR)/isaacp: isaacp/isaacp.o $(OBJ_ISAACP) lib/libisaacp.a
	@(echo "compiling $@ ...")
	@($(CC) $(COMPF) -o $(BINDIR)/isaacp isaacp/isaacp.c \
	$(OBJ_ISAACP) $(LINKF))
	@($(STRIP) $(BINDIR)/isaacp)

#
# ************* ISAAC spjitter recipe
# 

isaac-spjitter:
	@(if test -d ./spjitter ; then\
	(cd ./spjitter ; $(MAKE))\
	else (true) fi)

# end of file
