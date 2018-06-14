#include "gi-infos.h"

void
gi_base_info_init (void)
{
  #ifndef SCM_MAGIC_SNARFER
  #include "gi-infos.x"
  #endif
  SCM scm_make_class;

  scm_t_struct_finalize finalizer = finalize_gi_object;

  scm_make_class = scm_permanent_object (SCM_VARIABLE_REF (scm_c_lookup ("make-class")));

  scm_base_info_class = scm_make_foreign_object_type (scm_from_utf8_symbol ("<g-i-base-info>"),
                                                      scm_list_1 (scm_from_utf8_symbol ("ptr")),
                                                      finalizer);
  scm_c_define ("<g-i-base-info>", scm_base_info_class);
}

void
gi_infos_init (void)
{
  scm_callable_info_class = scm_permanent_object (SCM_VARIABLE_REF (scm_c_lookup ("<g-i-callable-info>")));
  scm_function_info_class = scm_permanent_object (SCM_VARIABLE_REF (scm_c_lookup ("<g-i-function-info>")));
  scm_signal_info_class = scm_permanent_object (SCM_VARIABLE_REF (scm_c_lookup ("<g-i-signal-info>")));
  scm_v_func_info_class = scm_permanent_object (SCM_VARIABLE_REF (scm_c_lookup ("<g-i-v-func-info>")));
  scm_registered_type_info_class =
    scm_permanent_object (SCM_VARIABLE_REF (scm_c_lookup ("<g-i-registered-type-info>")));
  scm_enum_info_class = scm_permanent_object (SCM_VARIABLE_REF (scm_c_lookup ("<g-i-enum-info>")));
  scm_interface_info_class = scm_permanent_object (SCM_VARIABLE_REF (scm_c_lookup ("<g-i-interface-info>")));
  scm_object_info_class = scm_permanent_object (SCM_VARIABLE_REF (scm_c_lookup ("<g-i-object-info>")));
  scm_struct_info_class = scm_permanent_object (SCM_VARIABLE_REF (scm_c_lookup ("<g-i-struct-info>")));
  scm_union_info_class = scm_permanent_object (SCM_VARIABLE_REF (scm_c_lookup ("<g-i-union-info>")));
  scm_arg_info_class = scm_permanent_object (SCM_VARIABLE_REF (scm_c_lookup ("<g-i-arg-info>")));
  scm_constant_info_class = scm_permanent_object (SCM_VARIABLE_REF (scm_c_lookup ("<g-i-constant-info>")));
  scm_field_info_class = scm_permanent_object (SCM_VARIABLE_REF (scm_c_lookup ("<g-i-field-info>")));
  scm_property_info_class = scm_permanent_object (SCM_VARIABLE_REF (scm_c_lookup ("<g-i-property-info>")));
  scm_type_info_class = scm_permanent_object (SCM_VARIABLE_REF (scm_c_lookup ("<g-i-type-info>")));
}

SCM
make_gi_info (GIBaseInfo *info)
{
  return scm_make_foreign_object_1 (scm_base_info_class, (void *) info);
}
