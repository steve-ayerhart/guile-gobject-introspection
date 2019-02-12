#ifndef __GUILE_GOBJECT_GUTIL_H__
#define __GUILE_GOBJECT_GUTIL_H__

#include <glib.h>
#include <libguile.h>

G_BEGIN_DECLS

#ifdef DEBUG_PRINT
#define DEBUG_ALLOC(str, args...) g_print ("I: " str "\n", ##args)
#else
#define DEBUG_ALLOC(str, args...)
#endif

#define SCM_TO_GPOINTER(scm) ((gpointer) SCM_UNPACK (scm))
#define GPOINTER_TO_SCM(ptr) (SCM_PACK ((scm_t_bits) (ptr)))

SCM scm_c_gerror_to_scm (GError *error);
void scm_c_raise_gerror (GError *error);

G_END_DECLS

#endif
