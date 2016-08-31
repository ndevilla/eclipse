
default:	pkg_pfits pkg_adonis pkg_conica pkg_isaac pkg_wfi

pkg_pfits:
	@(if test -d ./pfits ; then\
	(cd pfits ; $(MAKE)); \
	else (true) fi)

pkg_adonis:
	@(if test -d ./adonis ; then\
	(cd adonis ; $(MAKE)); \
	else (true) fi)

pkg_conica:
	@(if test -d ./conica ; then\
	(cd conica ; $(MAKE)); \
	else (true) fi)

pkg_isaac:
	@(if test -d ./isaac ; then\
	(cd isaac ; $(MAKE)); \
	else (true) fi)

pkg_wfi:
	@(if test -d ./wfi ; then\
	(cd wfi ; $(MAKE)); \
	else (true) fi)

clean veryclean:
	@(if test -d ./pfits ; then\
	(cd pfits ; $(MAKE) $@); \
	else (true) fi)
	@(if test -d ./adonis ; then\
	(cd adonis ; $(MAKE) $@); \
	else (true) fi)
	@(if test -d ./conica ; then\
	(cd conica ; $(MAKE) $@); \
	else (true) fi)
	@(if test -d ./isaac ; then\
	(cd isaac ; $(MAKE) $@); \
	else (true) fi)
	@(if test -d ./wfi ; then\
	(cd wfi ; $(MAKE) $@); \
	else (true) fi)
	@(if test -d ./ccd ; then\
	(cd ccd ; $(MAKE) $@); \
	else (true) fi)
