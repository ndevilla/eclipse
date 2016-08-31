#
# qfits tools Makefile
#

#
# General definitions
#

include ../config.make

RM      = rm -f
CP      = cp
MV      = mv
BINDIR  = ../bin

COMPF = $(CFLAGS) -I../include
LINKF = $(LFLAGS) -L../lib -lqfits

default: all


PROGS= 	$(BINDIR)/dtfits \
		$(BINDIR)/flipx \
		$(BINDIR)/dfits \
		$(BINDIR)/frameq \
		$(BINDIR)/replacekey \
		$(BINDIR)/qextract


all: $(PROGS)

veryclean:
	@(echo "cleaning qfits programs...")
	@($(RM) $(PROGS))


$(BINDIR)/dfits: dfits.c
	$(CC) $(COMPF) -o $(BINDIR)/dfits dfits.c $(LINKF)

$(BINDIR)/dtfits: dtfits.c
	$(CC) $(COMPF) -o $(BINDIR)/dtfits dtfits.c $(LINKF)
    
$(BINDIR)/flipx: flipx.c
	$(CC) $(COMPF) -o $(BINDIR)/flipx flipx.c $(LINKF)

$(BINDIR)/frameq: frameq.c
	$(CC) $(COMPF) -o $(BINDIR)/frameq frameq.c $(LINKF)

$(BINDIR)/replacekey: replacekey.c
	$(CC) $(COMPF) -o $(BINDIR)/replacekey replacekey.c $(LINKF)

$(BINDIR)/qextract: qextract.c
	$(CC) $(COMPF) -o $(BINDIR)/qextract qextract.c $(LINKF)


