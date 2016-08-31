# General definitions
#

include ../config.make

RM      = rm -f
CP      = cp
MV      = mv
BINDIR  = ../bin

default: all


PROGS =		$(BINDIR)/fitsort \
			$(BINDIR)/hierarch28 \
			$(BINDIR)/iofits \
			$(BINDIR)/fitsmd5

all: $(PROGS)

veryclean:
	@(echo "cleaning saft programs...")
	@($(RM) $(PROGS))


$(BINDIR)/fitsort:	fitsort.c
	@(echo "building $@  ...")
	@($(CC) $(CFLAGS) $(LFLAGS) -o $(BINDIR)/fitsort fitsort.c)

$(BINDIR)/hierarch28:	hierarch28.c
	@(echo "building $@  ...")
	@($(CC) $(CFLAGS) $(LFLAGS) -o $(BINDIR)/hierarch28 hierarch28.c)

$(BINDIR)/iofits:	iofits.c
	@(echo "building $@  ...")
	@($(CC) $(CFLAGS) $(LFLAGS) -o $(BINDIR)/iofits iofits.c)

$(BINDIR)/fitsmd5:	fitsmd5.c
	@(echo "building $@  ...")
	@($(CC) $(CFLAGS) $(LFLAGS) -o $(BINDIR)/fitsmd5 fitsmd5.c)

