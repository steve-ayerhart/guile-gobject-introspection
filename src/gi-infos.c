#include "gi-infos.h"

void
gi_infos_init (void)
{
  #ifndef SCM_MAGIC_SNARFER
  #include "gi-infos.x"
  #endif

  scm_t_struct_finalize finalizer = finalize_gi_object;

  base_info_class_t = scm_make_foreign_object_type (scm_from_utf8_symbol ("base-info"),
                                                    scm_list_1 (scm_from_utf8_symbol ("ptr")),
                                                    finalizer);
}
