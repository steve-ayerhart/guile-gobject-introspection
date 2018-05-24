#ifndef __GUILE_GOBJECT_GC_H__
#define _GUILE_GOBJECT_GC_H__

#include <glib.h>
#include <libguile.h>

G_BEGIN_DECLS

gpointer scm_glib_gc_protect_object (SCM obj);
void scm_glib_gc_unprotect_object (gpointer key);

void scm_gobject_gc_init (void);

G_END_DECLS
#endif
