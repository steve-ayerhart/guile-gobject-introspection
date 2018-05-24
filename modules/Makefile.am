GOBJECTS = $(SOURCES:%.scm=%.go)

nobase_mod_DATA = $(SOURCES) $(NOCOMP_SOURCES)
nobase_go_DATA = $(GOBJECTS)

guile_install_go_files = install_nobase_goDATA
$(guile_install_go_files): install_nobase_modDATA

CLEANFILES = $(GOBJECTS)
EXTRA_DIST = $(SOURCES) $(NOCOMP_SOURCES)
GUILE_WARNINGS = -Wunbound-variable -Warity-mismatch -Wformat
SUFFIXES = .scm .go

.scm.go:
	$(AM_V_GEN)$(top_builddir)/modules/pre-install-env $(GUILE_TOOLS) compile $(GUILE_WARNINGS) -o "$@" "$<"

moddir=$(prefix)/share/guile/site/$(GUILE_EFFECTIVE_VERSION)
godir=$(libdir)/guile/$(GUILE_EFFECTIVE_VERSION)/ccache

SOURCES = \
	gir.scm \
	gir/g-object/g-lib/g-quark.scm \
	gir/g-object/g-lib/g-error.scm \
	gir/g-object/utils.scm \
	gir/g-object/g-type.scm \
	gir/g-object/g-value.scm \
	gir/g-object/g-parameter.scm \
	gir/g-object/g-closure.scm \
	gir/g-object/g-signal.scm \
	gir/g-object.scm
