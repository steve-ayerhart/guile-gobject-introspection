#include "gi-infos.h"
#include "gtype.h"
#include "gi-argument.h"

/*
 * Make a list from the common GI API pattern of having a function which
 * returns a count and an indexed GIBaseInfo in the range of 0 to count
 */

SCM
gi_make_infos_list (SCM scm_info_class,
                    gint (*get_n_infos)(GIBaseInfo*),
                    GIBaseInfo* (*get_info)(GIBaseInfo*, gint))
{
  gint n_infos;
  SCM scm_infos;
  gint infos_index;

  n_infos = get_n_infos ((GIBaseInfo *) scm_to_pointer (scm_foreign_object_ref (scm_info_class, 0)));

  scm_infos = SCM_EOL;

  for (infos_index = 0; infos_index < n_infos; infos_index++) {
    GIBaseInfo *info;
    SCM scm_info;

    info = (GIBaseInfo *) get_info ((GIBaseInfo *) scm_to_pointer (scm_foreign_object_ref (scm_info_class, 0)),
                                    infos_index);
    g_assert (info != NULL);

    scm_info = gi_make_info (info);

    scm_infos = scm_append (scm_list_2 (scm_infos, scm_list_1 (scm_info)));
  }

  return scm_infos;
}

SCM
gi_make_info (GIBaseInfo *info)
{
  GIInfoType info_type;
  scm_t_pointer_finalizer finalizer = gi_finalize_pointer;

  SCM scm_info_type;

  info_type = g_base_info_get_type (info);

  switch (info_type) {
  case GI_INFO_TYPE_INVALID:
    g_critical ("Invalid info type");
    return SCM_UNSPECIFIED;
  case GI_INFO_TYPE_FUNCTION:
    scm_info_type = scm_gifunction_info_type;
    break;
  case GI_INFO_TYPE_CALLBACK:
    scm_info_type = scm_gicallback_info_type;
    break;
  case GI_INFO_TYPE_STRUCT:
  case GI_INFO_TYPE_BOXED:
    scm_info_type = scm_gistruct_info_type;
    break;
  case GI_INFO_TYPE_ENUM:
  case GI_INFO_TYPE_FLAGS:
    scm_info_type = scm_gienum_info_type;
    break;
  case GI_INFO_TYPE_OBJECT:
    scm_info_type = scm_giobject_info_type;
    break;
  case GI_INFO_TYPE_INTERFACE:
    scm_info_type = scm_giinterface_info_type;
    break;
  case GI_INFO_TYPE_CONSTANT:
    scm_info_type = scm_giconstant_info_type;
    break;
  case GI_INFO_TYPE_UNION:
    scm_info_type = scm_giunion_info_type;
    break;
  case GI_INFO_TYPE_VALUE:
    scm_info_type = scm_givalue_info_type;
    break;
  case GI_INFO_TYPE_SIGNAL:
    scm_info_type = scm_gisignal_info_type;
    break;
  case GI_INFO_TYPE_VFUNC:
    scm_info_type = scm_giv_func_info_type;
    break;
  case GI_INFO_TYPE_PROPERTY:
    scm_info_type = scm_giproperty_info_type;
    break;
  case GI_INFO_TYPE_FIELD:
    scm_info_type = scm_gifield_info_type;
    break;
  case GI_INFO_TYPE_ARG:
    scm_info_type = scm_giarg_info_type;
    break;
  case GI_INFO_TYPE_TYPE:
    scm_info_type = scm_gitype_info_type;
    break;
  default:
    g_assert_not_reached ();
    return SCM_UNSPECIFIED;
  }

  return scm_make_foreign_object_1 (scm_info_type,
                                    scm_from_pointer (g_base_info_ref (info), finalizer));
}

GIBaseInfo *
gi_object_get_gi_info (SCM scm_object)
{
  return (GIBaseInfo *) scm_to_pointer (scm_foreign_object_ref (scm_object, 0));
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

  base_info = gi_object_get_gi_info (scm_info);

  n_infos = get_n_infos (base_info);

  scm_infos = SCM_EOL;
  for (i = 0; i < n_infos; i++) {
    GIBaseInfo *info;
    SCM scm_info;

    info = (GIBaseInfo *) get_info (base_info, i);
    g_assert (info != NULL);

    scm_info = gi_make_info (info);

    scm_infos = scm_append (scm_list_2 (scm_infos, scm_list_1 (scm_info)));
  }

  return scm_infos;
}


/*
 * GIBaseInfo
 */

SCM_DEFINE (scm_gi_base_info_get_type, "%gi-base-info-get-type", 1, 0, 0,
            (SCM scm_base_info),
            "")
{
  return scm_from_uint (g_base_info_get_type (gi_object_get_gi_info (scm_base_info)));
}

SCM_DEFINE (scm_gi_base_info_get_name, "%gi-base-info-get-name", 1, 0, 0,
            (SCM scm_base_info),
            "")
{
  GIBaseInfo *base_info;
  const gchar *name;

  base_info = gi_object_get_gi_info (scm_base_info);

  if (g_base_info_get_type (base_info) == GI_INFO_TYPE_TYPE)
    {
      name = "type_type_instance";
    }
  else
    {
      name = g_base_info_get_name (base_info);
    }

  // TODO: need to escape any names?

  return scm_from_locale_string (name);
}

SCM_DEFINE (scm_gi_base_info_get_namespace, "%gi-base-info-get-namespace", 1, 0, 0,
            (SCM scm_base_info),
            "")
{
  GIBaseInfo *base_info;
  const gchar *namespace;

  base_info = gi_object_get_gi_info (scm_base_info);
  namespace = g_base_info_get_name (base_info);

  return scm_from_locale_string (namespace);
}

SCM_DEFINE (scm_gi_base_info_is_deprecated, "%gi-base-info-is-deprecated", 1, 0, 0,
            (SCM scm_base_info),
            "")
{
  GIBaseInfo *base_info;

  base_info = gi_object_get_gi_info (scm_base_info);
  return scm_from_bool (g_base_info_is_deprecated (base_info));
}

SCM_DEFINE (scm_gi_base_info_get_container, "%gi-base-info-get-container", 1, 0, 0,
            (SCM scm_base_info),
            "")
{
  GIBaseInfo *base_info;

  base_info = gi_object_get_gi_info (scm_base_info);

  return gi_make_info (g_base_info_get_container (base_info));
}

SCM_DEFINE (scm_gi_base_info_get_attribute, "%gi-base-info-get-attribute", 2, 0, 0,
            (SCM scm_base_info, SCM scm_name),
            "")
{
  GIBaseInfo *base_info;
  char *name;
  const char *value;

  base_info = gi_object_get_gi_info (scm_base_info);

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

SCM_DEFINE (scm_gi_registered_type_info_get_g_type, "%gi-registered-type-info-get-gtype", 1, 0, 0,
            (SCM scm_registered_type_info),
            ""
            )
{
  GIRegisteredTypeInfo *registered_type_info;
  GType gtype;

  registered_type_info = (GIRegisteredTypeInfo *) gi_object_get_gi_info (scm_registered_type_info);
  gtype = g_registered_type_info_get_g_type (registered_type_info);

  if (gtype == G_TYPE_INVALID)
    return SCM_BOOL_F;


  return scm_c_gtype_to_class (gtype);
}

/*
 * GIObjectInfo
 */

SCM_DEFINE (scm_gi_object_info_get_methods, "%gi-object-info-get-methods", 1, 0, 0,
            (SCM scm_object_info),
            ""
            )
{
  return gi_make_infos_list (scm_object_info, g_object_info_get_n_methods, g_object_info_get_method);
}

/*
 * GICallableInfo
 */

/*
 * GIArgInfo
 */

#define DIRECTION_IN_SYMBOL "gi-direction-in"
#define DIRECTION_OUT_SYMBOL "gi-direction-out"
#define DIRECTION_INOUT_SYMBOL "gi-direction-inout"

#define SCOPE_TYPE_INVALID_SYMBOL "gi-scope-type-invalid"
#define SCOPE_TYPE_CALL_SYMBOL "gi-scope-type-call"
#define SCOPE_TYPE_ASYNC_SYMBOL "gi-scope-type-async"
#define SCOPE_TYPE_NOTIFIED_SYMBOL "gi-scope-type-notified"


SCM_DEFINE (scm_g_arg_info_get_direction, "%gi-arg-info-get-direction", 1, 0, 0,
            (SCM scm_arg_info),
            ""
            )
{
  GIArgInfo *arg_info;

  arg_info = (GIArgInfo *) gi_object_get_gi_info (scm_arg_info);

  return scm_from_int (g_arg_info_get_direction (arg_info));
}

SCM_DEFINE (scm_g_arg_info_is_return_value, "%gi-arg-info-is-return-value?", 1, 0, 0,
            (SCM scm_arg_info),
            ""
            )
{
  GIArgInfo *arg_info;

  arg_info = (GIArgInfo *) gi_object_get_gi_info (scm_arg_info);

  return scm_from_bool (g_arg_info_is_return_value (arg_info));
}

SCM_DEFINE (scm_g_arg_info_is_optional, "%gi-arg-info-is-optional?", 1, 0, 0,
            (SCM scm_arg_info),
            ""
            )
{
  GIArgInfo *arg_info;

  arg_info = (GIArgInfo *) gi_object_get_gi_info (scm_arg_info);

  return scm_from_bool (g_arg_info_is_optional (arg_info));
}

SCM_DEFINE (scm_g_arg_info_may_be_null, "%gi-arg-info-may-be-null?", 1, 0, 0,
            (SCM scm_arg_info),
            ""
            )
{
  GIArgInfo *arg_info;

  arg_info = (GIArgInfo *) gi_object_get_gi_info (scm_arg_info);

  return scm_from_bool (g_arg_info_may_be_null (arg_info));
}

SCM_DEFINE (scm_g_arg_info_get_ownership_transfer, "%gi-arg-info-get-ownership-transfer", 1, 0, 0,
            (SCM scm_arg_info),
            ""
            )
{
  GIArgInfo *arg_info;

  arg_info = (GIArgInfo *) gi_object_get_gi_info (scm_arg_info);

  return scm_from_int (g_arg_info_get_ownership_transfer (arg_info));
}

SCM_DEFINE (scm_g_arg_info_get_scope, "%gi-arg-info-get-scope", 1, 0, 0,
            (SCM scm_arg_info),
            ""
            )
{
  GIArgInfo *arg_info;

  arg_info = (GIArgInfo *) gi_object_get_gi_info (scm_arg_info);

  return scm_from_int (g_arg_info_get_scope (arg_info));
}

SCM_DEFINE (scm_g_arg_info_get_closure, "%gi-arg-info-get-closure", 1, 0, 0,
            (SCM scm_arg_info),
            ""
            )
{
  GIArgInfo *arg_info;

  arg_info = (GIArgInfo *) gi_object_get_gi_info (scm_arg_info);

  return scm_from_int (g_arg_info_get_closure (arg_info));
}

SCM_DEFINE (scm_g_arg_info_get_destroy, "%gi-arg-info-get-destroy", 1, 0, 0,
            (SCM scm_arg_info),
            ""
            )
{
  GIArgInfo *arg_info;

  arg_info = (GIArgInfo *) gi_object_get_gi_info (scm_arg_info);

  return scm_from_int (g_arg_info_get_destroy (arg_info));
}

SCM_DEFINE (scm_g_arg_info_get_type, "%gi-arg-info-get-type", 1, 0, 0,
            (SCM scm_arg_info),
            ""
            )
{
  GIArgInfo *arg_info;
  GITypeInfo *type_info;
  SCM scm_type;

  arg_info = (GIArgInfo *) gi_object_get_gi_info (scm_arg_info);
  type_info = g_arg_info_get_type (arg_info);

  if (type_info == NULL)
    scm_type = SCM_BOOL_F;
  else {
    scm_type = gi_make_info (type_info);
  }

  scm_remember_upto_here_1 (type_info);

  return scm_type;
}

/*
 * GIConstantInfo
 */

SCM_DEFINE (scm_gi_constant_info_get_value, "%gi-constant-info-get-value", 1, 0, 0,
            (SCM scm_constant_info),
            "")
{
  GITypeInfo *type_info;
  GIConstantInfo *constant_info;
  GIArgument value = {0};
  SCM scm_value;

  constant_info = (GIConstantInfo *) gi_object_get_gi_info (scm_constant_info);

  GI_IS_CONSTANT_INFO (constant_info);

  if (g_constant_info_get_value (constant_info, &value) < 0) {
    return SCM_UNDEFINED;
  }

  type_info = g_constant_info_get_type (constant_info);

  scm_value = gi_arg_to_scm (type_info, GI_TRANSFER_NOTHING, value);
  g_constant_info_free_value (constant_info, &value);

  scm_remember_upto_here_1 (constant_info);

  return scm_value;
}

/*
 * init
 */

void
gi_infos_init (void)
{
#ifndef SCM_MAGIC_SNARFER
#include "gi-infos.x"
#endif
  scm_t_struct_finalize finalizer = gi_finalize_object;
  scm_gibase_info_type = scm_make_foreign_object_type (scm_from_utf8_symbol ("<gi-base-info>"),
                                                       scm_list_1 (scm_from_utf8_symbol ("info")),
                                                       finalizer);
  scm_c_define ("<gi-base-info>", scm_gibase_info_type);

  scm_c_define (DIRECTION_IN_SYMBOL, scm_from_int (GI_DIRECTION_IN));
  scm_c_define (DIRECTION_OUT_SYMBOL, scm_from_int (GI_DIRECTION_OUT));
  scm_c_define (DIRECTION_INOUT_SYMBOL, scm_from_int (GI_DIRECTION_INOUT));
  scm_c_define (SCOPE_TYPE_INVALID_SYMBOL, scm_from_int (GI_SCOPE_TYPE_INVALID));
  scm_c_define (SCOPE_TYPE_CALL_SYMBOL, scm_from_int (GI_SCOPE_TYPE_CALL));
  scm_c_define (SCOPE_TYPE_ASYNC_SYMBOL, scm_from_int (GI_SCOPE_TYPE_ASYNC));
  scm_c_define (SCOPE_TYPE_NOTIFIED_SYMBOL, scm_from_int (GI_SCOPE_TYPE_NOTIFIED));

  // TODO: ugh
  scm_c_use_module ("oop goops");

  scm_c_eval_string("(define-class <gi-callable-info> (<gi-base-info>))");
  scm_c_eval_string("(define-class <gi-callback-info> (<gi-callable-info>))");
  scm_c_eval_string("(define-class <gi-function-info> (<gi-callable-info>))");
  scm_c_eval_string("(define-class <gi-signal-info> (<gi-callable-info>))");
  scm_c_eval_string("(define-class <gi-v-func-info> (<gi-callable-info>))");

  scm_c_eval_string("(define-class <gi-registered-type-info> (<gi-base-info>))");
  scm_c_eval_string("(define-class <gi-enum-info> (<gi-registered-type-info>))");
  scm_c_eval_string("(define-class <gi-interface-info> (<gi-registered-type-info>))");
  scm_c_eval_string("(define-class <gi-object-info> (<gi-registered-type-info>))");
  scm_c_eval_string("(define-class <gi-struct-info> (<gi-registered-type-info>))");
  scm_c_eval_string("(define-class <gi-union-info> (<gi-registered-type-info>))");

  scm_c_eval_string("(define-class <gi-arg-info> (<gi-base-info>))");
  scm_c_eval_string("(define-class <gi-constant-info> (<gi-base-info>))");
  scm_c_eval_string("(define-class <gi-field-info> (<gi-base-info>))");
  scm_c_eval_string("(define-class <gi-property-info> (<gi-base-info>))");
  scm_c_eval_string("(define-class <gi-type-info> (<gi-base-info>))");
  scm_c_eval_string("(define-class <gi-value-info> (<gi-base-info>))");

  scm_gicallable_info_type = scm_permanent_object (scm_variable_ref (scm_c_lookup("<gi-callable-info>")));
  scm_gicallback_info_type = scm_permanent_object (scm_variable_ref (scm_c_lookup("<gi-callback-info>")));
  scm_gifunction_info_type = scm_permanent_object (scm_variable_ref (scm_c_lookup("<gi-function-info>")));
  scm_gisignal_info_type = scm_permanent_object (scm_variable_ref (scm_c_lookup("<gi-signal-info>")));
  scm_giv_func_info_type = scm_permanent_object (scm_variable_ref (scm_c_lookup("<gi-v-func-info>")));

  scm_giregistered_type_info_type =
    scm_permanent_object (scm_variable_ref (scm_c_lookup("<gi-registered-type-info>")));
  scm_gienum_info_type = scm_permanent_object (scm_variable_ref (scm_c_lookup("<gi-enum-info>")));
  scm_giinterface_info_type = scm_permanent_object (scm_variable_ref (scm_c_lookup("<gi-interface-info>")));
  scm_giobject_info_type = scm_permanent_object (scm_variable_ref (scm_c_lookup("<gi-object-info>")));
  scm_gistruct_info_type = scm_permanent_object (scm_variable_ref (scm_c_lookup("<gi-struct-info>")));
  scm_giunion_info_type = scm_permanent_object (scm_variable_ref (scm_c_lookup("<gi-union-info>")));

  scm_giarg_info_type = scm_permanent_object (scm_variable_ref (scm_c_lookup("<gi-arg-info>")));
  scm_giconstant_info_type = scm_permanent_object (scm_variable_ref (scm_c_lookup("<gi-constant-info>")));
  scm_gifield_info_type = scm_permanent_object (scm_variable_ref (scm_c_lookup("<gi-field-info>")));
  scm_giproperty_info_type = scm_permanent_object (scm_variable_ref (scm_c_lookup("<gi-property-info>")));
  scm_gitype_info_type = scm_permanent_object (scm_variable_ref (scm_c_lookup("<gi-type-info>")));
  scm_givalue_info_type = scm_permanent_object (scm_variable_ref (scm_c_lookup("<gi-value-info>")));
}

