#include "ggi-info.h"
#include "gtype.h"
#include "gi-argument.h"

/*
 * Make a list from the common GI API pattern of having a function which
 * returns a count and an indexed GIBaseInfo in the range of 0 to count
 */
SCM
ggi_make_infos_list (SCM scm_info_class,
                     gint (*get_n_infos)(GIBaseInfo*),
                     GIBaseInfo* (*get_info)(GIBaseInfo*, gint))
{
  gint n_infos;
  SCM scm_infos;
  gint infos_index;

  n_infos = get_n_infos ((GIBaseInfo *) scm_foreign_object_signed_ref (scm_info_class, 0));

  scm_infos = SCM_EOL;

  for (infos_index = 0; infos_index < n_infos; infos_index++) {
    GIBaseInfo *info;
    SCM scm_info;

    info = (GIBaseInfo *) get_info ((GIBaseInfo *) scm_foreign_object_signed_ref (scm_info_class, 0),
                                    infos_index);
    g_assert (info != NULL);

    scm_info = ggi_make_info (info);

    scm_infos = scm_append (scm_list_2 (scm_infos, scm_list_1 (scm_info)));
  }

  return scm_infos;
}


/*
 * GIBaseInfo
 */

SCM_DEFINE (scm_g_base_info_get_type, "%g-base-info-get-type", 1, 0, 0,
            (SCM scm_base_info),
            "")
{
  return scm_from_uint (g_base_info_get_type (ggi_object_get_gi_info (scm_base_info)));
}

SCM_DEFINE (scm_g_base_info_get_name, "%g-base-info-get-name", 1, 0, 0,
            (SCM scm_base_info),
            "")
{
  GIBaseInfo *base_info;
  const gchar *name;

  base_info = ggi_object_get_gi_info (scm_base_info);
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

  base_info = ggi_object_get_gi_info (scm_base_info);
  namespace = g_base_info_get_name (base_info);

  return scm_from_locale_string (namespace);
}

SCM_DEFINE (scm_g_base_info_is_deprecated, "%g-base-info-is-deprecated", 1, 0, 0,
            (SCM scm_base_info),
            "")
{
  GIBaseInfo *base_info;

  base_info = ggi_object_get_gi_info (scm_base_info);
  return scm_from_bool (g_base_info_is_deprecated (base_info));
}

SCM_DEFINE (scm_g_base_info_get_container, "%g-base-info-get-container", 1, 0, 0,
            (SCM scm_base_info),
            "")
{
  GIBaseInfo *base_info;

  base_info = ggi_object_get_gi_info (scm_base_info);

  return ggi_make_info (g_base_info_get_container (base_info));
}

SCM_DEFINE (scm_g_base_info_get_attribute, "%g-base-info-get-attribute", 2, 0, 0,
            (SCM scm_base_info, SCM scm_name),
            "")
{
  GIBaseInfo *base_info;
  char *name;
  const char *value;

  base_info = ggi_object_get_gi_info (scm_base_info);

  scm_name = scm_symbol_to_string (scm_name);
  name = scm_to_locale_string (scm_name);

  value = g_base_info_get_attribute (base_info, name);
  g_free (name);

  if (value == NULL)
    return SCM_UNSPECIFIED;

  return scm_from_locale_symbol (value);
}

/*
 * GIRegisteredInfo
 */

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

/*
 * GIObjectInfo
 */

SCM_DEFINE (scm_g_object_info_get_methods, "%g-object-info-get-methods", 1, 0, 0,
            (SCM scm_object_info),
            ""
            )
{
  return ggi_make_infos_list (scm_object_info, g_object_info_get_n_methods, g_object_info_get_method);
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

  constant_info = (GIConstantInfo *) ggi_object_get_gi_info (scm_constant_info);

  GI_IS_CONSTANT_INFO (constant_info);

  if (g_constant_info_get_value (constant_info, &value) < 0) {
    return SCM_UNDEFINED;
  }

  type_info = g_constant_info_get_type (constant_info);

  return gi_arg_to_scm (type_info, GI_TRANSFER_NOTHING, value);
}

GIBaseInfo *
ggi_object_get_gi_info (SCM scm_object)
{
  return (GIBaseInfo *) scm_foreign_object_signed_ref (scm_object, 0);
}

static SCM
make_infos_list (SCM scm_info,
                 gint (*get_n_infos)(GIBaseInfo *),
                 GIBaseInfo * (*get_info)(GIBaseInfo *, gint))
{
  GIBaseInfo *base_info;
  gint n_infos;
  SCM scm_infos;
  gint i;

  base_info = ggi_object_get_gi_info (scm_info);

  n_infos = get_n_infos (base_info);

  scm_infos = SCM_EOL;
  for (i = 0; i < n_infos; i++) {
    GIBaseInfo *info;
    SCM scm_info;

    info = (GIBaseInfo *) get_info (base_info, i);
    g_assert (info != NULL);

    scm_info = ggi_make_info (info);

    // maybe? g_base_info_unref (info);
    scm_infos = scm_append (scm_list_2 (scm_infos, scm_list_1 (scm_info)));
  }

  return scm_infos;
}

void
ggi_base_info_init (void)
{
  scm_t_struct_finalize finalizer = ggi_finalize_object;
  scm_base_info_type = scm_make_foreign_object_type (scm_from_utf8_symbol ("<base-info>"),
                                                     scm_list_1 (scm_from_utf8_symbol ("info")),
                                                     finalizer);
  scm_c_define ("<base-info>", scm_base_info_type);
}

void
ggi_infos_init (void)
{
#ifndef SCM_MAGIC_SNARFER
#include "ggi-info.x"
#endif

  scm_callable_info_type = scm_permanent_object (SCM_VARIABLE_REF (scm_c_lookup ("<callable-info>")));
  scm_function_info_type = scm_permanent_object (SCM_VARIABLE_REF (scm_c_lookup ("<function-info>")));
  scm_signal_info_type = scm_permanent_object (SCM_VARIABLE_REF (scm_c_lookup ("<signal-info>")));
  scm_v_func_info_type = scm_permanent_object (SCM_VARIABLE_REF (scm_c_lookup ("<v-func-info>")));
  scm_callback_info_type = scm_permanent_object (SCM_VARIABLE_REF (scm_c_lookup ("<callback-info>")));
  scm_registered_type_info_type =
    scm_permanent_object (SCM_VARIABLE_REF (scm_c_lookup ("<registered-type-info>")));
  scm_enum_info_type = scm_permanent_object (SCM_VARIABLE_REF (scm_c_lookup ("<enum-info>")));
  scm_interface_info_type = scm_permanent_object (SCM_VARIABLE_REF (scm_c_lookup ("<interface-info>")));
  scm_object_info_type = scm_permanent_object (SCM_VARIABLE_REF (scm_c_lookup ("<object-info>")));
  scm_struct_info_type = scm_permanent_object (SCM_VARIABLE_REF (scm_c_lookup ("<struct-info>")));
  scm_union_info_type = scm_permanent_object (SCM_VARIABLE_REF (scm_c_lookup ("<union-info>")));
  scm_arg_info_type = scm_permanent_object (SCM_VARIABLE_REF (scm_c_lookup ("<arg-info>")));
  scm_constant_info_type = scm_permanent_object (SCM_VARIABLE_REF (scm_c_lookup ("<constant-info>")));
  scm_field_info_type = scm_permanent_object (SCM_VARIABLE_REF (scm_c_lookup ("<field-info>")));
  scm_property_info_type = scm_permanent_object (SCM_VARIABLE_REF (scm_c_lookup ("<property-info>")));
  scm_value_info_type = scm_permanent_object (SCM_VARIABLE_REF (scm_c_lookup ("<value-info>")));
  scm_type_info_type = scm_permanent_object (SCM_VARIABLE_REF (scm_c_lookup ("<type-info>")));
}

SCM
ggi_make_info (GIBaseInfo *info)
{
  GIInfoType info_type;
  SCM scm_info_type;

  info_type = g_base_info_get_type (info);

  switch (info_type) {
  case GI_INFO_TYPE_INVALID:
    g_critical ("Invalid info type");
    return SCM_UNSPECIFIED;
  case GI_INFO_TYPE_FUNCTION:
    scm_info_type = scm_make_foreign_object_1 (scm_function_info_type, (void *) info);
    break;
  case GI_INFO_TYPE_CALLBACK:
    scm_info_type = scm_make_foreign_object_1 (scm_callback_info_type, (void *) info);
    break;
  case GI_INFO_TYPE_STRUCT:
  case GI_INFO_TYPE_BOXED:
    scm_info_type = scm_make_foreign_object_1 (scm_struct_info_type, (void *) info);
    break;
  case GI_INFO_TYPE_ENUM:
  case GI_INFO_TYPE_FLAGS:
    scm_info_type = scm_make_foreign_object_1 (scm_enum_info_type, (void *) info);
    break;
  case GI_INFO_TYPE_OBJECT:
    scm_info_type = scm_make_foreign_object_1 (scm_object_info_type, (void *) info);
    break;
  case GI_INFO_TYPE_INTERFACE:
    scm_info_type = scm_make_foreign_object_1 (scm_interface_info_type, (void *) info);
    break;
  case GI_INFO_TYPE_CONSTANT:
    scm_info_type = scm_make_foreign_object_1 (scm_constant_info_type, (void *) info);
    break;
  case GI_INFO_TYPE_UNION:
    scm_info_type = scm_make_foreign_object_1 (scm_union_info_type, (void *) info);
    break;
  case GI_INFO_TYPE_VALUE:
    scm_info_type = scm_make_foreign_object_1 (scm_value_info_type, (void *) info);
    break;
  case GI_INFO_TYPE_SIGNAL:
    scm_info_type = scm_make_foreign_object_1 (scm_signal_info_type, (void *) info);
    break;
  case GI_INFO_TYPE_VFUNC:
    scm_info_type = scm_make_foreign_object_1 (scm_v_func_info_type, (void *) info);
    break;
  case GI_INFO_TYPE_PROPERTY:
    scm_info_type = scm_make_foreign_object_1 (scm_property_info_type, (void *) info);
    break;
  case GI_INFO_TYPE_FIELD:
    scm_info_type = scm_make_foreign_object_1 (scm_field_info_type, (void *) info);
    break;
  case GI_INFO_TYPE_ARG:
    scm_info_type = scm_make_foreign_object_1 (scm_arg_info_type, (void *) info);
    break;
  case GI_INFO_TYPE_TYPE:
    scm_info_type = scm_make_foreign_object_1 (scm_type_info_type, (void *) info);
    break;
  default:
    g_assert_not_reached ();
    return SCM_UNSPECIFIED;
  }

  return scm_info_type;
}

