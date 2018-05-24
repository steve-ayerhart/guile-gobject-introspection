#ifndef __GUILE_GOBJECT_GPARAMETER_H__
#define __GUILE_GOBJECT_GPARAMETER_H__

#include "gtype.h"

G_BEGIN_DECLS

extern SCM scm_class_gparam;

#define SCM_GPARAMP(scm) \
  scm_c_gtype_instance_is_a_p (scm, G_TYPE_PARAM)

#define SCM_VALIDATE_GPARAM(pos, scm) \
  SCM_MAKE_VALIDATE (pos, scm, GPARAMP)

#define SCM_VALIDATE_GPARAM_COPY(pos, scm, cvar) \
  SCM_VALIDATE_GTYPE_INSTANCE_TYPE_COPY (pos, scm, G_TYPE_PARAM, cvar)

G_END_DECLS

#endif
