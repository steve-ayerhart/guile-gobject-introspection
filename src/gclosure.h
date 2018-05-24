#ifndef __GUILE_GOBJECT_GCLOSURE_H__
#define __GUILE_GOBJECT_GCLOSURE_H__

#include "gvalue.h"

G_BEGIN_DECLS

SCM scm_gclosure_invoke (SCM instance, SCM return_type, SCM args);

G_END_DECLS

#endif
