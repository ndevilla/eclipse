#
# eclipse/wfi Makefile
# N. Devillard
#

# Path to eclipse library
ELIB = ../../src

#
# General definitions are all here
#

SUFFIXES = .o .c .h
OUTPUT_OPTION= -o $@
IFLAGS  = -I$(ELIB)/include -I./common -I$(QFITSDIR)/include

LD      = ld
LDFLAGS = -L$(ELIB)/lib -L$(QFITSDIR)/lib $(LFLAGS)
LDLIBS  = -leclipse -lqfits -lm $(LDLIBSLOCAL)
RM      = rm -f
RRM     = rm -rf
CP      = cp
MV      = mv

#
# Redefinition of implicit rules
#

SUFFIXES = .o .c .h .a .so .sl

COMPILE.c=$(CC) $(CFLAGS) $(IFLAGS) -c
LINK.c=$(CC) $(CFLAGS) $(IFLAGS) $(LDFLAGS)

.c:
	@($(LINK.c) -o $@ $< $(LDLIBS))
.c.o:
	@(echo "compiling $< ...")
	@($(COMPILE.c) -o $@ $<)

COMPF  = $(CFLAGS) $(IFLAGS)
LINKF  = $(LDFLAGS) $(LDLIBS)



#
# Eclipse directories
#

BINDIR = ../../bin


# Load compilation options (valid for all compiled sources hereafter) 
include $(ELIB)/../config.make


default: all

all: 	wfi-common wfi

clean:
	@(echo "cleaning WFI object files...")
	@($(RM) common/*.o recipes/*.o)

veryclean:
	@(echo "cleaning WFI recipes and object files...")
	@($(RM) common/*.o recipes/*.o $(WFIBINS))


#
# ************* WFI general routines
#

wfi-common:	common/wfip_lib.o

SRC_WFICOM = common/wfip_lib.c
OBJ_WFICOM = $(SRC_WFICOM:.c=.o)

common/wfip_lib.o:	common/wfip_lib.c
	@(echo "compiling $@ ...")
	@($(CC) $(COMPF) -c -o common/wfip_lib.o common/wfip_lib.c)

#
# ************* WFI recipes
#

WFIBINS =	$(BINDIR)/wfi_ff \
			$(BINDIR)/wfi_masterbias \
			$(BINDIR)/wfi_overscan \
			$(BINDIR)/wfi_prep \
			$(BINDIR)/wfi_split

wfi:	$(WFIBINS)


$(BINDIR)/wfi_ff:	recipes/wfi_ff.c common/wfip_lib.o
	@(echo "compiling $@ ...")
	@($(CC) $(COMPF) -o $(BINDIR)/wfi_ff recipes/wfi_ff.c \
	$(OBJ_WFICOM) $(LINKF))

$(BINDIR)/wfi_masterbias:	recipes/wfi_masterbias.c common/wfip_lib.o
	@(echo "compiling $@ ...")
	@($(CC) $(COMPF) -o $(BINDIR)/wfi_masterbias recipes/wfi_masterbias.c \
	$(OBJ_WFICOM) $(LINKF))

$(BINDIR)/wfi_overscan:	recipes/wfi_overscan.c common/wfip_lib.o
	@(echo "compiling $@ ...")
	@($(CC) $(COMPF) -o $(BINDIR)/wfi_overscan recipes/wfi_overscan.c \
	$(OBJ_WFICOM) $(LINKF))

$(BINDIR)/wfi_prep:	recipes/wfi_prep.c common/wfip_lib.o
	@(echo "compiling $@ ...")
	@($(CC) $(COMPF) -o $(BINDIR)/wfi_prep recipes/wfi_prep.c \
	$(OBJ_WFICOM) $(LINKF))

$(BINDIR)/wfi_split:	recipes/wfi_split.c common/wfip_lib.o
	@(echo "compiling $@ ...")
	@($(CC) $(COMPF) -o $(BINDIR)/wfi_split recipes/wfi_split.c \
	$(OBJ_WFICOM) $(LINKF))


# end of file
