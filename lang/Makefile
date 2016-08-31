
default:	pkg_lua pkg_python


pkg_lua:
	@(if test -d ./lua ; then\
	(cd lua ; $(MAKE)); \
	else (true) fi)
	@(if test -d ./lua-dev ; then\
	(cd lua-dev ; $(MAKE)); \
	else (true) fi)

pkg_python:
	@(if test -d ./python ; then\
	(cd python ; $(MAKE)); \
	else (true) fi)

clean veryclean:
	@(if test -d ./lua ; then\
	(cd lua ; $(MAKE) $@); \
	else (true) fi)
	@(if test -d ./lua-dev ; then\
	(cd lua-dev ; $(MAKE) $@); \
	else (true) fi)
	@(if test -d ./python ; then\
	(cd python ; $(MAKE) $@); \
	else (true) fi)
