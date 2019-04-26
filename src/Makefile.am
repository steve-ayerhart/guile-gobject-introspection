lib_LTLIBRARIES = gobject-introspection.la

AM_CFALGS = -I. $(WARN_CFLAGS) $(DEBUG_FLAGS)

C_FILES = \
  scmgobject-object.c \
  scmgi-type.c \
	gc.c \
	gutil.c \
	gtype.c \
	gvalue.c \
	gobject.c \
	gparameter.c \
	gclosure.c \
	gsignal.c \
  gi-module.c \
  ggi-basic-types.c \
  ggi-list.c \
  ggi-hashtable.c \
  ggi-array.c \
  ggi-object.c \
  ggi-closure.c \
  ggi-function.c \
  ggi-ccallback.c \
  ggi-argument.c \
  ggi-error.c \
  ggi-cache.c \
  ggi-value.c \
  ggi-enum-marshal.c \
  ggi-struct.c \
  ggi-struct-marshal.c \
  ggi-invoke.c \
  ggi-infos.c \
  ggi-repository.c

H_FILES = $(C_FILES:.c=.h)
X_FILES = $(C_FILES:.c=.x)

gobject_introspection_la_SOURCES = $(C_FILES) $(H_FILES) support.c support.h private.h ggi-invoke-state-struct.h scmgobject.h scmgobject-internal.h

gobject_introspection_la_CFLAGS = $(GUILE_CFLAGS) $(GIR_CFLAGS) $(GOBJECT_CFLAGS) $(AM_CFLAGS)
gobject_introspection_la_LIBADD = $(GUILE_LIBS) $(GIR_LIBS) $(GOBJECT_LIBS)
gobject_introspection_la_LDFLAGS = -module -avoid-version

MAINTERCLEANFILES = Makefile.in

SUFFIXES = .x .c

.c.x:
	guile-snarf $(gobject_introspection_la_CFLAGS) $< > $@ || { rm $@; false; }

BUILT_SOURCES = $(X_FILES)

CLEANFILES = $(X_FILES)
