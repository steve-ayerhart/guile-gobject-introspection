#ifndef __GUILE_GOBJECT_H__
#define __GUILE_GOBJECT_H__

#include "gtype.h"
#include "gparameter.h"
#include "gvalue.h"
#include "gsignal.h"
#include "gutil.h"

G_BEGIN_DECLS

#define SCM_GOBJECT_CLASSP(scm) \
  scm_c_gtype_class_is_a_p (scm, G_TYPE_OBJECT)

#define SCM_VALIDATE_GOBJECT_CLASS(pos, scm) \
  SCM_MAKE_VALIDATE (pos, scm, GOBJECT_CLASSP)

#define SCM_VALIDATE_GOBJECT_CLASS_COPY(pos, scm, cvar) \
  do { \
    SCM_VALIDATE_GOBJECT_CLASS (pos, scm); \
    SCM_VALIDATE_GTYPE_CLASS_COPY (pos, scm, cvar); \
  } while (0)

#define SCM_GOBJECTP(scm) \
  scm_c_gtype_instance_is_a_p (scm, G_TYPE_OBJECT)

#define SCM_VALIDATE_GOBJECT(pos, scm) \
  SCM_MAKE_VALIDATE (pos, scm, GOBJECTP)

#define SCM_VALIDATE_GOBJECT_COPY(pos, scm, cvar) \
  SCM_VALIDATE_GTYPE_INSTANCE_TYPE_COPY (pos, scm, G_TYPE_OBJECT, cvar)

void scm_register_gobject_postmakefunc (GType type, gpointer (*postmakefunc) (gpointer));

G_END_DECLS

#endif
