#include "gi-types.h"

scm_t_bits base_info_t;
scm_t_bits struct_info_t;
scm_t_bits union_info_t;
scm_t_bits enum_info_t;
scm_t_bits interface_info_t;
scm_t_bits constant_info_t;
scm_t_bits value_info_t;
scm_t_bits signal_info_t;
scm_t_bits v_func_info_t;
scm_t_bits property_info_t;
scm_t_bits field_info_t;
scm_t_bits type_info_t;
scm_t_bits error_domain_info_t;
scm_t_bits unresolved_info_t;

void
gi_types_init (void)
{
  #ifndef SCM_MAGIC_SNARFER
  #include "gi-types.x"
  #endif

  base_info_t = scm_make_smob_type ("g-i-base-info", 0);
  struct_info_t = scm_make_smob_type ("g-i-struct-info", 0);
  union_info_t = scm_make_smob_type ("g-i-union-info", 0);
  enum_info_t = scm_make_smob_type ("g-i-enum-info", 0);
  interface_info_t = scm_make_smob_type ("g-i-interface-info", 0);
  constant_info_t = scm_make_smob_type ("g-i-constant-info", 0);
  value_info_t = scm_make_smob_type ("g-i-value-info", 0);
  signal_info_t = scm_make_smob_type ("g-i-signal-info", 0);
  v_func_info_t = scm_make_smob_type ("g-i-v-func-info", 0);
  property_info_t = scm_make_smob_type ("g-i-property-info", 0);
  field_info_t = scm_make_smob_type ("g-i-field-info", 0);
  type_info_t = scm_make_smob_type ("g-i-type-info", 0);
  error_domain_info_t = scm_make_smob_type ("g-i-error-domain-info", 0);
  unresolved_info_t = scm_make_smob_type ("g-i-unresolved-info", 0);
}
