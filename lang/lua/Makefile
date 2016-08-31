# makefile for Lua
# Heavily modified for embedding into eclipse
# N. Devillard - 01 dec 2000

# Path to eclipse lib
ELIB=../../src

include $(ELIB)/../config.make

IFLAGS  = -DPOPEN -I./src -I./include


SUFFIXES = .o .c .h .a

AR      = ar
ARFLAGS = rcvs

RM		= rm -f

COMPILE.c=$(CC) $(CFLAGS) $(IFLAGS) -c

.c.o:
	@(echo "compiling $< ...")
	@($(COMPILE.c) -o $@ $<)


SRCS=	\
	src/lapi.c \
	src/lcode.c \
	src/ldebug.c \
	src/ldo.c \
	src/lfunc.c \
	src/lgc.c \
	src/llex.c \
	src/lmem.c \
	src/lobject.c \
	src/lparser.c \
	src/lstate.c \
	src/lstring.c \
	src/ltable.c \
	src/ltests.c \
	src/ltm.c \
	src/lundump.c \
	src/lvm.c \
	src/lzio.c \
	src/lauxlib.c \
	src/lbaselib.c \
	src/ldblib.c \
	src/liolib.c \
	src/lmathlib.c \
	src/lstrlib.c \
	src/lextra.c

OBJS = $(SRCS:.c=.o)

all: lualib

lualib: $(ELIB)/lib/liblua.a

$(ELIB)/lib/liblua.a: $(OBJS)
	@(echo "Building lua library file ...")
	@($(AR) $(ARFLAGS) $(ELIB)/lib/liblua.a $(OBJS))

clean:
	@(echo "cleaning Lua object files...")
	@($(RM) $(OBJS))

veryclean:
	@(echo "cleaning Lua libraries and object files...")
	@($(RM) $(OBJS) $(ELIB)/lib/liblua.a)
