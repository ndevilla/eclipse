#
# eclipse/adonis Makefile
# N. Devillard
#

# Path to eclipse library directory
ELIB=../../src

#
# General definitions are all here
#

IFLAGS  = -I$(ELIB)/include -I./recipes -I$(QFITSDIR)/include

LD      = ld
LDFLAGS = -L$(ELIB)/lib -L$(QFITSDIR)/lib $(LFLAGS)
LDLIBS  = -leclipse -lqfits -lm $(LDLIBSLOCAL)
RM      = rm -f
RRM     = rm -rf
CP      = cp
MV      = mv

COMPILE.c=$(CC) $(CFLAGS) $(IFLAGS) -c
LINK.c=$(CC) $(CFLAGS) $(IFLAGS) $(LDFLAGS)

.c:
	@($(LINK.c) -o $@ $< $(LDLIBS))
.c.o:
	@(echo "compiling $< ...")
	@($(COMPILE.c) -o $@ $<)


COMPF  = $(CFLAGS) $(IFLAGS)
LINKF  = $(LDFLAGS) $(LDLIBS)


BINDIR = ../../bin


# Load compilation options (valid for all compiled sources hereafter) 
include $(ELIB)/../config.make


default: all

all: 	adonis-recipes

clean:
	@(echo "cleaning Adonis object files...")
	@($(RM) recipes/*.o)

veryclean:
	@(echo "cleaning Adonis object files and recipes...")
	@($(RM) recipes/*.o $(ADOBIN))

#
# Adonis related routines
#

ADOBIN = $(BINDIR)/ado_pl \
		 $(BINDIR)/ado_refits \
		 $(BINDIR)/wdat

adonis-recipes:		$(ADOBIN) recipes/ado_utils.o

#
# General utilities for Adonis-related routines:
#
OBJ_ADO = recipes/ado_utils.o


recipes/ado_utils.o:	recipes/ado_utils.c recipes/ado_utils.h
	@(echo "compiling $@ ...")
	@($(CC) $(COMPF) -c -o recipes/ado_utils.o recipes/ado_utils.c)

#
# Adonis recipes
#

# Adonis pipeline

OBJ_ADOPL = recipes/ado_pl.o

$(BINDIR)/ado_pl: $(OBJ_ADOPL) $(OBJ_ADO)
	@(echo "compiling $@ ...")
	@($(CC) -o $(BINDIR)/ado_pl $(OBJ_ADOPL) $(OBJ_ADO) $(LINKF))

recipes/ado_pl.o: recipes/ado_pl.c
	@($(CC) $(COMPF) recipes/ado_pl.c -c -o recipes/ado_pl.o)

# Adonis re-FITS command

OBJ_REFITS = recipes/ado_refits.o

$(BINDIR)/ado_refits: $(OBJ_REFITS) $(OBJ_ADO)
	@(echo "compiling $@ ...")
	@($(CC) -o $(BINDIR)/ado_refits $(OBJ_REFITS) $(OBJ_ADO) $(LINKF))

recipes/ado_refits.o: recipes/ado_refits.c
	@($(CC) $(COMPF) recipes/ado_refits.c -c -o recipes/ado_refits.o)

# wdat command
$(BINDIR)/wdat: recipes/wdat.c
	@(echo "compiling $@ ...")
	@($(CC) -o $(BINDIR)/wdat $(COMPF) $(LFLAGS) recipes/wdat.c)



# end of file
