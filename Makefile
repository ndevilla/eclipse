
RM = rm -rf

default:	pkg_qfits pkg_pfits pkg_main pkg_ins pkg_lang pkg_dfs


pkg_qfits:
	@(if test -d ./qfits ; then\
	(cd qfits ; ./configure ; $(MAKE)); \
	else (true) fi)

pkg_pfits:
	@(if test -d ./ins/pfits ; then\
	(cd ins/pfits ; $(MAKE)); \
	else (true) fi)

pkg_main:
	@(if test -d ./src ; then\
	(cd src ; $(MAKE)); \
	else (true) fi)

pkg_ins:
	@(if test -d ./ins ; then\
	(cd ins ; $(MAKE)); \
	else (true) fi)

pkg_lang:
	@(if test -d ./lang ; then\
	(cd lang ; $(MAKE)); \
	else (true) fi)

pkg_dfs:
	@(if test -d ./dfs ; then\
	(cd dfs ; $(MAKE)); \
	else (true) fi)

clean veryclean:
	@(if test -d ./src ; then\
	(echo "cleaning library tree..." ; cd src ; $(MAKE) $@); \
	else (true) fi)
	@(if test -d ./ins ; then\
	(echo "cleaning instrument tree..." ; cd ins ; $(MAKE) $@); \
	else (true) fi)
	@(if test -d ./lang ; then\
	(echo "cleaning language tree..." ; cd lang ; $(MAKE) $@); \
	else (true) fi)
	@(if test -d ./dfs ; then\
	(echo "cleaning dfs tree..." ; cd dfs ; $(MAKE) $@); \
	else (true) fi)
	@(if test -d ./qfits ; then\
	(echo "cleaning qfits tree..." ; cd qfits ; $(MAKE) $@); \
	else (true) fi)

isaacp:
	$(RM) lang
	$(RM) ins/adonis ins/ccd ins/conica ins/wfi
