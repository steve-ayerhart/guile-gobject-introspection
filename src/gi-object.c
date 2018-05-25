#include "gi-object.h"
#include "gi-types.h"
#include "gi-function.h"

scm_t_bits object_info_t;

SCM_DEFINE (scm_g_object_info_get_type_name, "g-object-info-get-type-name", 1, 0, 0,
            (SCM scm_object_info),
            ""
            )
{
  GIObjectInfo *object_info;
  const char *type_name;

  object_info = (GIObjectInfo *) SCM_SMOB_DATA (scm_object_info);
  type_name = g_object_info_get_type_name (object_info);

  if (type_name == NULL)
    return SCM_BOOL_F;

  return scm_from_locale_string (type_name);
}

SCM_DEFINE (scm_g_object_info_get_type_init, "g-object-info-get-type-init", 1, 0, 0,
            (SCM scm_object_info),
            ""
            )
{
  GIObjectInfo *object_info;
  const char *type_init;

  object_info = (GIObjectInfo *) SCM_SMOB_DATA (scm_object_info);
  type_init = g_object_info_get_type_init (object_info);

  if (type_init == NULL)
    return SCM_BOOL_F;

  return scm_from_locale_string (type_init);
}

SCM_DEFINE (scm_g_object_info_get_type_abstract, "g-object-info-get-type-abstract", 1, 0, 0,
            (SCM scm_object_info),
            ""
            )
{
  GIObjectInfo *object_info;

  object_info = (GIObjectInfo *) SCM_SMOB_DATA (scm_object_info);

  return scm_from_bool (g_object_info_get_abstract (object_info));
}

SCM_DEFINE (scm_g_object_info_get_parent, "g-object-info-get-parent", 1, 0, 0,
            (SCM scm_object_info),
            ""
            )
{
  GIObjectInfo *object_info;
  GIObjectInfo *parent_info;
  SCM scm_return;

  object_info = (GIObjectInfo *) SCM_SMOB_DATA (scm_object_info);
  parent_info = g_object_info_get_parent (object_info);

  if (parent_info == NULL)
    scm_return = SCM_BOOL_F;
  else {
    scm_return = scm_make_smob (object_info_t);
    SCM_SET_SMOB_DATA (scm_return, parent_info);
  }

  return scm_return;
}

SCM_DEFINE (scm_g_object_info_get_n_interfaces, "g-object-info-get-n-interfaces", 1, 0, 0,
            (SCM scm_object_info),
            ""
            )
{
  GIObjectInfo *object_info;

  object_info = (GIObjectInfo *) SCM_SMOB_DATA (scm_object_info);

  return scm_from_int (g_object_info_get_n_interfaces (object_info));
}

SCM_DEFINE (scm_g_object_info_get_interface, "g-object-info-get-interface", 2, 0, 0,
            (SCM scm_object_info, SCM scm_n),
            ""
            )
{
  GIObjectInfo *object_info;
  GIInterfaceInfo *interface_info;
  SCM scm_return;

  object_info = (GIObjectInfo *) SCM_SMOB_DATA (scm_object_info);
  interface_info = g_object_info_get_interface (object_info,
                                                scm_to_int (scm_n));
  if (interface_info == NULL)
    scm_return = SCM_BOOL_F;
  else {
    scm_return = scm_make_smob (interface_info_t);
    SCM_SET_SMOB_DATA (scm_return, interface_info);
  }

  return scm_return;
}

SCM_DEFINE (scm_g_object_info_get_n_fields, "g-object-info-get-n-fields", 1, 0, 0,
            (SCM scm_object_info),
            ""
            )
{
  GIObjectInfo *object_info;
  object_info = (GIObjectInfo *) SCM_SMOB_DATA (scm_object_info);

  return scm_from_int (g_object_info_get_n_fields (object_info));
}

SCM_DEFINE (scm_g_object_info_get_field, "g-object-info-get-field", 2, 0, 0,
            (SCM scm_object_info, SCM scm_n),
            ""
            )
{
  GIObjectInfo *object_info;
  GIFieldInfo *field_info;
  SCM scm_return;

  object_info = (GIObjectInfo *) SCM_SMOB_DATA (scm_object_info);
  field_info = g_object_info_get_field (object_info, scm_to_int (scm_n));

  if (field_info == NULL)
    scm_return = SCM_BOOL_F;
  else {
    scm_return = scm_make_smob (field_info_t);
    SCM_SET_SMOB_DATA (scm_return, field_info);
  }

  return scm_return;
}

SCM_DEFINE (scm_g_object_info_get_n_properties, "g-object-info-get-n-properties", 1, 0, 0,
            (SCM scm_object_info),
            ""
            )
{
  GIObjectInfo *object_info;

  object_info = (GIObjectInfo *) SCM_SMOB_DATA (scm_object_info);

  return scm_from_int (g_object_info_get_n_properties (object_info));
}

SCM_DEFINE (scm_g_object_info_get_property, "g-object-info-get-property", 2, 0, 0,
            (SCM scm_object_info, SCM scm_n),
            ""
            )
{
  GIObjectInfo *object_info;
  GIPropertyInfo *property_info;
  SCM scm_return;

  object_info = (GIObjectInfo *) SCM_SMOB_DATA (scm_object_info);
  property_info = g_object_info_get_property (object_info,
                                              scm_to_int (scm_n));

  if (property_info == NULL)
    scm_return = SCM_BOOL_F;
  else {
    scm_return = scm_make_smob (property_info_t);
    SCM_SET_SMOB_DATA (scm_return, property_info);
  }

  return scm_return;
}

SCM_DEFINE (scm_g_object_info_get_n_methods, "g-object-info-get-n-methods", 1, 0, 0,
            (SCM scm_object_info),
            ""
            )
{
  GIObjectInfo *object_info;

  object_info = (GIObjectInfo *) SCM_SMOB_DATA (scm_object_info);

  return scm_from_int (g_object_info_get_n_methods (object_info));
}

SCM_DEFINE (scm_g_object_info_get_method, "g-object-info-get-method", 2, 0, 0,
            (SCM scm_object_info, SCM scm_n),
            ""
            )
{
  GIObjectInfo *object_info;
  GIFunctionInfo *func_info;
  SCM scm_return;

  object_info = (GIObjectInfo *) SCM_SMOB_DATA (scm_object_info);
  func_info = g_object_info_get_method (object_info, scm_to_int (scm_n));

  if (func_info == NULL)
    scm_return = SCM_BOOL_F;
  else {
    scm_return = scm_make_smob (function_info_t);
    SCM_SET_SMOB_DATA (scm_return, func_info);
  }

  return scm_return;
}

SCM_DEFINE (scm_g_object_info_find_method, "g-object-info-find-method", 2, 0, 0,
            (SCM scm_object_info, SCM scm_name),
            ""
            )
{
  GIObjectInfo *object_info;
  GIFunctionInfo *func_info;
  SCM scm_return;

  object_info = (GIObjectInfo *) SCM_SMOB_DATA (scm_object_info);
  func_info = g_object_info_find_method (object_info,
                                         scm_to_locale_string (scm_name));

  if (func_info == NULL)
    scm_return = SCM_BOOL_F;
  else {
    scm_return = scm_make_smob (function_info_t);
    SCM_SET_SMOB_DATA (scm_return, func_info);
  }

  return scm_return;
}

SCM_DEFINE (scm_g_object_info_get_n_signals, "g-object-info-get-n-signals", 1, 0, 0,
            (SCM scm_object_info),
            ""
            )
{
  GIObjectInfo *object_info;

  object_info = (GIObjectInfo *) SCM_SMOB_DATA (scm_object_info);

  return scm_from_int (g_object_info_get_n_signals (object_info));
}

SCM_DEFINE (scm_g_object_info_get_signal, "g-object-info-get-signal", 2, 0, 0,
            (SCM scm_object_info, SCM scm_n),
            ""
            )
{
  GIObjectInfo *object_info;
  GISignalInfo *signal_info;
  SCM scm_return;

  object_info = (GIObjectInfo *) SCM_SMOB_DATA (scm_object_info);
  signal_info = g_object_info_get_signal (object_info,
                                          scm_to_int (scm_n));

  if (signal_info == NULL)
    scm_return = SCM_BOOL_F;
  else {
    scm_return = scm_make_smob (signal_info_t);
    SCM_SET_SMOB_DATA (scm_return, signal_info);
  }

  return scm_return;
}

SCM_DEFINE (scm_g_object_info_get_n_vfuncs, "g-object-info-get-n-vfuncs", 1, 0, 0,
            (SCM scm_object_info),
            ""
            )
{
  GIObjectInfo *object_info;

  object_info = (GIObjectInfo *) SCM_SMOB_DATA (scm_object_info);

  return scm_from_int (g_object_info_get_n_vfuncs (object_info));
}

SCM_DEFINE (scm_g_object_info_get_vfunc, "g-object-info-get-vfunc", 2, 0, 0,
            (SCM scm_object_info, SCM scm_n),
            ""
            )
{
  GIObjectInfo *object_info;
  GIVFuncInfo *vfunc_info;
  SCM scm_return;

  object_info = (GIObjectInfo *) SCM_SMOB_DATA (scm_object_info);
  vfunc_info = g_object_info_get_vfunc (object_info, scm_to_int (scm_n));

  if (vfunc_info == NULL)
    scm_return = SCM_BOOL_F;
  else {
    scm_return = scm_make_smob (v_func_info_t);
    SCM_SET_SMOB_DATA (scm_return, vfunc_info);
  }

  return scm_return;
}

SCM_DEFINE (scm_g_object_info_get_n_constants, "g-object-info-get-n-constants", 1, 0, 0,
            (SCM scm_object_info),
            ""
            )
{
  GIObjectInfo *object_info;

  object_info = (GIObjectInfo *) SCM_SMOB_DATA (scm_object_info);

  return scm_from_int (g_object_info_get_n_constants (object_info));

}

SCM_DEFINE (scm_g_object_info_get_constant, "g-object-info-get-constant", 2, 0, 0,
            (SCM scm_object_info, SCM scm_n),
            ""
            )
{
  GIObjectInfo *object_info;
  GIConstantInfo *constant_info;
  SCM scm_return;

  object_info = (GIObjectInfo *) SCM_SMOB_DATA (scm_object_info);
  constant_info = g_object_info_get_constant (object_info,
                                              scm_to_int (scm_n));

  if (constant_info == NULL)
    scm_return = SCM_BOOL_F;
  else {
    scm_return = scm_make_smob (constant_info_t);
    SCM_SET_SMOB_DATA (scm_return, constant_info);
  }

  return scm_return;
}

SCM_DEFINE (scm_g_object_info_get_class_struct, "g-object-info-get-class_struct", 1, 0, 0,
            (SCM scm_object_info),
            ""
            )
{
  GIObjectInfo *object_info;
  GIStructInfo *struct_info;
  SCM scm_return;

  object_info = (GIObjectInfo *) SCM_SMOB_DATA (scm_object_info);
  struct_info = g_object_info_get_class_struct (object_info);

  if (struct_info == NULL)
    return SCM_BOOL_F;
  else {
    scm_return = scm_make_smob (struct_info_t);
    SCM_SET_SMOB_DATA (scm_return, struct_info);
  }

  return scm_return;
}

void
gi_object_init (void)
{
  #ifndef SCM_MAGIC_SNARFER
  #include "gi-object.x"
  #endif

  object_info_t = scm_make_smob_type ("g-i-object-info", 0);
}
