#
# qfits Makefile
#
include config.make

AR	    = ar
ARFLAGS = rcs
RM      = rm -f
CP      = cp -f

# Implicit rules
 
SUFFIXES = .o .c .h .a .so .sl
 
COMPILE.c=$(CC) $(CFLAGS) -Iinclude $(RELOC) -c
.c.o:
	@(echo "compiling $< ...")
	@($(COMPILE.c) -o $@ $<)
 
SRCS = src/byteswap.c \
	   src/cache.c \
       src/expkey.c \
       src/fits_h.c \
       src/fits_md5.c \
       src/fits_p.c \
       src/fits_rw.c \
       src/get_name.c \
	   src/ieeefp-compat.c \
       src/md5.c \
	   src/pafs.c \
       src/pixio.c \
       src/qerror.c \
       src/simple.c \
       src/t_iso8601.c \
       src/tfits.c \
	   src/version.c \
	   src/xmemory.c

 
OBJS = $(SRCS:.c=.o)
 
default:    all
 
all:        $(TARGETS) progs

check:
	@(if test -d ./test ; then\
	(echo "Executing test programs..." ; cd test ; $(MAKE) ; \
	./test_qfits ;\
	./test_pixio ;\
	./test_tfits ;) ; \
	else (true) fi)

static: lib/libqfits.a

dynamic: lib/libqfits.$(DYNSUF)

lib/libqfits.a: $(OBJS)
	@(echo "Building static library...")
	@($(AR) $(ARFLAGS) lib/libqfits.a $(OBJS))

lib/libqfits.$(DYNSUF): $(OBJS)
	@(echo "Building shared library...")
	@(ld $(SHARED) -o lib/libqfits.$(DYNSUF) $(OBJS) -lc)

install:
	@(echo "Installing library in " $(prefix))
	mkdir -p $(prefix)
	mkdir -p $(prefix)/lib
	mkdir -p $(prefix)/include
	mkdir -p $(prefix)/bin

	@(if test -f ./include/qfits.h ; then\
	($(CP) include/qfits.h $(prefix)/include); \
	else (true) fi)
	
	@(if test -f ./include/xmemory.h ; then\
	($(CP) include/xmemory.h $(prefix)/include); \
	else (true) fi)

	@(if test -f ./lib/libqfits.a ; then\
	($(CP) lib/libqfits.a $(prefix)/lib); \
	else (true) fi)

	@(if test -f ./lib/libqfits.$(DYNSUF) ; then\
	($(CP) lib/libqfits.$(DYNSUF) $(prefix)/lib); \
	else (true) fi)

	@(if test -f ./bin/dfits ; then\
	($(CP) bin/dfits $(prefix)/bin); \
	else (true) fi)
	
	@(if test -f ./bin/dtfits ; then\
	($(CP) bin/dtfits $(prefix)/bin); \
	else (true) fi)
	
	@(if test -f ./bin/fitsmd5 ; then\
	($(CP) bin/fitsmd5 $(prefix)/bin); \
	else (true) fi)
	
	@(if test -f ./bin/fitsort ; then\
	($(CP) bin/fitsort $(prefix)/bin); \
	else (true) fi)
	
	@(if test -f ./bin/flipx ; then\
	($(CP) bin/flipx $(prefix)/bin); \
	else (true) fi)
	
	@(if test -f ./bin/hierarch28 ; then\
	($(CP) bin/hierarch28 $(prefix)/bin); \
	else (true) fi)
	
	@(if test -f ./bin/iofits ; then\
	($(CP) bin/iofits $(prefix)/bin); \
	else (true) fi)
	
	@(if test -f ./bin/replacekey ; then\
	($(CP) bin/replacekey $(prefix)/bin); \
	else (true) fi)
	
	@(if test -f ./bin/qextract ; then\
	($(CP) bin/qextract $(prefix)/bin); \
	else (true) fi)
	
	@(if test -f ./bin/frameq ; then\
	($(CP) bin/frameq $(prefix)/bin); \
	else (true) fi)
	
clean:
	$(RM) $(OBJS) 
 
veryclean:
	$(RM) $(OBJS) lib/libqfits.a lib/libqfits.$(DYNSUF)
	$(RM) -r build
	
	@(if test -d ./test ; then \
	(cd test ; $(MAKE) veryclean ; cd ..) \
	else (true) fi)
	
	cd main ; $(MAKE) veryclean ; cd ..
	cd saft ; $(MAKE) veryclean ; cd ..
	$(RM) config.make

#
# Build example and stand-alone programs
#

progs:
	cd main ; $(MAKE) ; cd ..
	cd saft ; $(MAKE) ; cd ..

#
# Building qfits as a Python module.
#
# type:
# % python setup.py build
# % python setup.py install
#

docs:
	@(echo "Building HTML documentation...")
	@(cd doc ; $(MAKE))
	
