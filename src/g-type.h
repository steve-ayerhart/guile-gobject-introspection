#ifndef __SCMG_TYPE_H__
#define __SCMG_TYPE_H__

#include <libguile.h>
#include <glib-object.h>
#include <girepository.h>

G_BEGIN_DECLS

extern SCM scm_class_g_type_class;

void
g_type_init (void);

G_END_DECLS

#endif
