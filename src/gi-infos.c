#include "gi-infos.h"
#include "gtype.h"
#include "gi-argument.h"

// BASE INFO
SCM_DEFINE (scm_g_base_info_get_name, "%g-base-info-get-name", 1, 0, 0,
            (SCM scm_base_info),
            "")
{
  GIBaseInfo *base_info;
  const gchar *name;

  base_info = (GIBaseInfo *) scm_foreign_object_signed_ref (scm_base_info, 0);
  name = g_base_info_get_name (base_info);

  // TODO: need to escape any names?

  return scm_from_locale_string (name);
}

SCM_DEFINE (scm_g_base_info_get_namespace, "%g-base-info-get-namespace", 1, 0, 0,
            (SCM scm_base_info),
            "")
{
  GIBaseInfo *base_info;
  const gchar *namespace;

  base_info = (GIBaseInfo *) scm_foreign_object_signed_ref (scm_base_info, 0);
  namespace = g_base_info_get_name (base_info);

  return scm_from_locale_string (namespace);
}

SCM_DEFINE (scm_g_base_info_is_deprecated, "%g-base-info-is-deprecated", 1, 0, 0,
            (SCM scm_base_info),
            "")
{
  GIBaseInfo *base_info;

  base_info = (GIBaseInfo *) scm_foreign_object_signed_ref (scm_base_info, 0);
  return scm_from_bool (g_base_info_is_deprecated (base_info));
}

SCM_DEFINE (scm_g_base_info_get_container, "%g-base-info-get-container", 1, 0, 0,
            (SCM scm_base_info),
            "")
{
  GIBaseInfo *base_info;

  base_info = (GIBaseInfo *) scm_foreign_object_signed_ref (scm_base_info, 0);

  return make_gi_info (g_base_info_get_container (base_info));
}

// REGISTERED TYPE

SCM_DEFINE (scm_g_registered_type_info_get_g_type, "%g-registered-type-info-get-g-type", 1, 0, 0,
            (SCM scm_registered_type_info),
            ""
            )
{
  GIRegisteredTypeInfo *registered_type_info;
  GType gtype;


  registered_type_info = (GIRegisteredTypeInfo *) scm_foreign_object_signed_ref (scm_registered_type_info, 0);
  gtype = g_registered_type_info_get_g_type (registered_type_info);

  if (gtype == G_TYPE_INVALID)
    return SCM_BOOL_F;

  return scm_c_gtype_to_class (gtype);
}

// CONSTANT INFO

SCM_DEFINE (scm_g_constant_info_get_value, "%g-constant-info-get-value", 1, 0, 0,
            (SCM scm_constant_info),
            "")
{
  GITypeInfo *type_info;
  GIConstantInfo *constant_info;
  GIArgument value = {0};
  SCM scm_value;

  constant_info = (GIConstantInfo *) ggir_object_get_gi_info (scm_constant_info);

  GI_IS_CONSTANT_INFO (constant_info);

  if (g_constant_info_get_value (constant_info, &value) < 0) {
    return SCM_UNDEFINED;
  }

  type_info = g_constant_info_get_type (constant_info);

  return gi_arg_to_scm (type_info, GI_TRANSFER_NOTHING, value);
}

GIBaseInfo *
ggir_object_get_gi_info (SCM scm_object)
{
  return (GIBaseInfo *) scm_foreign_object_signed_ref (scm_object, 0);
}

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
  scm_callback_info_class = scm_permanent_object (SCM_VARIABLE_REF (scm_c_lookup ("<g-i-callback-info>")));
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
  scm_value_info_class = scm_permanent_object (SCM_VARIABLE_REF (scm_c_lookup ("<g-i-value-info>")));
  scm_type_info_class = scm_permanent_object (SCM_VARIABLE_REF (scm_c_lookup ("<g-i-type-info>")));
}

SCM
make_gi_info (GIBaseInfo *info)
{
  GIInfoType info_type;
  SCM scm_info_class;

  info_type = g_base_info_get_type (info);

  switch (info_type) {
  case GI_INFO_TYPE_INVALID:
    g_critical ("Invalid info type");
    return SCM_UNSPECIFIED;
  case GI_INFO_TYPE_FUNCTION:
    scm_info_class = scm_make_foreign_object_1 (scm_function_info_class, (void *) info);
    break;
  case GI_INFO_TYPE_CALLBACK:
    scm_info_class = scm_make_foreign_object_1 (scm_callback_info_class, (void *) info);
    break;
  case GI_INFO_TYPE_STRUCT:
  case GI_INFO_TYPE_BOXED:
    scm_info_class = scm_make_foreign_object_1 (scm_struct_info_class, (void *) info);
    break;
  case GI_INFO_TYPE_ENUM:
  case GI_INFO_TYPE_FLAGS:
    scm_info_class = scm_make_foreign_object_1 (scm_enum_info_class, (void *) info);
    break;
  case GI_INFO_TYPE_OBJECT:
    scm_info_class = scm_make_foreign_object_1 (scm_object_info_class, (void *) info);
    break;
  case GI_INFO_TYPE_INTERFACE:
    scm_info_class = scm_make_foreign_object_1 (scm_interface_info_class, (void *) info);
    break;
  case GI_INFO_TYPE_CONSTANT:
    scm_info_class = scm_make_foreign_object_1 (scm_constant_info_class, (void *) info);
    break;
  case GI_INFO_TYPE_UNION:
    scm_info_class = scm_make_foreign_object_1 (scm_union_info_class, (void *) info);
    break;
  case GI_INFO_TYPE_VALUE:
    scm_info_class = scm_make_foreign_object_1 (scm_value_info_class, (void *) info);
    break;
  case GI_INFO_TYPE_SIGNAL:
    scm_info_class = scm_make_foreign_object_1 (scm_signal_info_class, (void *) info);
    break;
  case GI_INFO_TYPE_VFUNC:
    scm_info_class = scm_make_foreign_object_1 (scm_v_func_info_class, (void *) info);
    break;
  case GI_INFO_TYPE_PROPERTY:
    scm_info_class = scm_make_foreign_object_1 (scm_property_info_class, (void *) info);
    break;
  case GI_INFO_TYPE_FIELD:
    scm_info_class = scm_make_foreign_object_1 (scm_field_info_class, (void *) info);
    break;
  case GI_INFO_TYPE_ARG:
    scm_info_class = scm_make_foreign_object_1 (scm_arg_info_class, (void *) info);
    break;
  case GI_INFO_TYPE_TYPE:
    scm_info_class = scm_make_foreign_object_1 (scm_type_info_class, (void *) info);
    break;
  default:
    g_assert_not_reached ();
    return SCM_UNSPECIFIED;
  }

  return scm_info_class;
}

