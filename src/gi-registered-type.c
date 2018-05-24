#include "gi-registered-type.h"
#include "gtype.h"

scm_t_bits registered_type_info_t;

SCM_DEFINE (scm_g_registered_type_info_get_g_type, "g-registered-type-get-g-type", 1, 0, 0,
            (SCM scm_info),
            ""
            )
{
  GIRegisteredTypeInfo *info;
  GType gtype;

  info = (GIRegisteredTypeInfo *) SCM_SMOB_DATA (scm_info);
  gtype = g_registered_type_info_get_g_type (info);

  if (gtype == G_TYPE_INVALID)
    return SCM_BOOL_F;

  return scm_c_gtype_to_class (gtype);
}
