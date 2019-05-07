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
	$(AM_V_GEN)$(top_builddir)/src/module/pre-install-env $(GUILE_TOOLS) compile $(GUILE_WARNINGS) -o "$@" "$<"

moddir=$(prefix)/share/guile/site/$(GUILE_EFFECTIVE_VERSION)
godir=$(libdir)/guile/$(GUILE_EFFECTIVE_VERSION)/ccache

SOURCES = \
	glib/utils.scm \
	gobject/gtype.scm \
	gobject/gvalue.scm \
	gobject/gparameter.scm \
	gobject/gclosure.scm \
	gobject/gsignal.scm \
	gobject.scm \
  gi.scm
