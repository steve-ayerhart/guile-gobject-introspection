#include <girffi.h>

#include "gtype.h"
#include "gvalue.h"
#include "gi-argument.h"
#include "gi-types.h"

#define DIRECTION_IN_SYMBOL "g-i-direction-in"
#define DIRECTION_OUT_SYMBOL "g-i-direction-out"
#define DIRECTION_INOUT_SYMBOL "g-i-direction-inout"

#define SCOPE_TYPE_INVALID_SYMBOL "g-i-scope-type-invalid"
#define SCOPE_TYPE_CALL_SYMBOL "g-i-scope-type-call"
#define SCOPE_TYPE_ASYNC_SYMBOL "g-i-scope-type-async"
#define SCOPE_TYPE_NOTIFIED_SYMBOL "g-i-scope-type-notified"

scm_t_bits arg_info_t;

typedef struct
{
  ffi_cif *cif;
  SCM scm_callback;

  GICallableInfo *callable_info;

  GIScopeType scope_type;
} CallbackData;

typedef struct
{
  ffi_cif *cif;
  CallbackData *callback_data;
} DestroyNotifyData;

static CallbackData *
callback_data_new (SCM scm_callback,
                   GICallableInfo *callable_info,
                   GIScopeType scope_type)
{
  CallbackData *data;

  data = g_slice_new0 (CallbackData);

  data->cif = g_slice_new0 (ffi_cif);
  data->scm_callback = scm_callback;
  data->callable_info = callable_info;
  data->scope_type = scope_type;

  if (scope_type == GI_SCOPE_TYPE_ASYNC ||
      scope_type == GI_SCOPE_TYPE_NOTIFIED)
    scm_gc_protect_object (scm_callback);

  return data;
}

static void
callback_data_free (CallbackData *data)
{
  scm_gc_unprotect_object (data->scm_callback);

  g_slice_free (ffi_cif, data->cif);
  g_slice_free (CallbackData, data);
}

static DestroyNotifyData *
destroy_notify_data_new (CallbackData *callback_data)
{
  DestroyNotifyData *data;

  data = g_slice_new0 (DestroyNotifyData);

  data->cif = g_slice_new0 (ffi_cif);
  data->callback_data = callback_data;

  return data;
}

static void
destroy_notify_data_free (DestroyNotifyData *data)
{
  callback_data_free (data->callback_data);

  g_slice_free (ffi_cif, data->cif);
  g_slice_free (DestroyNotifyData, data);
}

SCM
gi_return_value_to_scm (GICallableInfo *info,
                        GArgument return_value)
{
  GITypeInfo *return_type;
  GITransfer transfer_type;

  return_type = g_callable_info_get_return_type (info);
  transfer_type = g_callable_info_get_caller_owns (info);

  return gi_arg_to_scm (return_type, transfer_type, return_value);
}

static void
callback_closure (ffi_cif *cif,
                  void *result,
                  void **args,
                  void *data)
{
  CallbackData *callback_data;
  SCM *scm_args;
  int i, n_args;
  SCM scm_return;

  callback_data = (CallbackData *) data;

  n_args = g_callable_info_get_n_args (callback_data->callable_info);
  scm_args = (SCM *) g_new0 (SCM, n_args);
  for (i = 0; i < n_args; i++) {
    GIArgInfo *arg_info;
    GITypeInfo *arg_type;
    GITransfer transfer_type;

    arg_info = g_callable_info_get_arg (callback_data->callable_info,
                                        i);
    arg_type = g_arg_info_get_type (arg_info);
    transfer_type = g_arg_info_get_ownership_transfer (arg_info);

    scm_args[i] = gi_arg_to_scm (arg_type,
                                 transfer_type,
                                 *((GArgument *) args[i]));

    g_base_info_unref ((GIBaseInfo*) arg_info);
    g_base_info_unref ((GIBaseInfo*) arg_type);
  }

  scm_return = scm_call_n (callback_data->scm_callback, scm_args, n_args);

  scm_return_value_to_gi (scm_return,
                          callback_data->callable_info,
                          (GArgument *) result);

  g_free (scm_args);

  if (callback_data->scope_type == GI_SCOPE_TYPE_ASYNC)
    callback_data_free (callback_data);
}

static void
destroy_notify_callback (ffi_cif *cif,
                         void *result,
                         void **args,
                         void *data)
{
  destroy_notify_data_free ((DestroyNotifyData *) data);
}

void
scm_return_value_to_gi (SCM scm_return,
                        GICallableInfo *info,
                        GArgument *return_value)
{
  GITypeInfo *return_type;
  GITransfer transfer_type;

  return_type = g_callable_info_get_return_type (info);
  transfer_type = g_callable_info_get_caller_owns (info);

  scm_to_gi_arg (scm_return,
                 return_type,
                 transfer_type,
                 GI_SCOPE_TYPE_INVALID,
                 NULL,
                 return_value,
                 NULL);
}

SCM
gi_arg_to_scm (GITypeInfo *arg_type,
               GITransfer transfer_type,
               GArgument arg)
{
  switch (g_type_info_get_tag (arg_type)) {
  case GI_TYPE_TAG_VOID:
    return PTR2SCM (arg.v_pointer);
  case GI_TYPE_TAG_BOOLEAN:
    return scm_from_bool (arg.v_boolean);
  case GI_TYPE_TAG_INT8:
    return scm_from_int8 (arg.v_int8);
  case GI_TYPE_TAG_UINT8:
    return scm_from_uint8 (arg.v_uint8);
  case GI_TYPE_TAG_INT16:
    return scm_from_int16 (arg.v_int16);
  case GI_TYPE_TAG_UINT16:
    return scm_from_uint16 (arg.v_uint16);
  case GI_TYPE_TAG_INT32:
    return scm_from_int32 (arg.v_int32);
  case GI_TYPE_TAG_UINT32:
    return scm_from_uint32 (arg.v_uint32);
  case GI_TYPE_TAG_INT64:
    return scm_from_int64 (arg.v_int64);
  case GI_TYPE_TAG_UINT64:
    return scm_from_uint64 (arg.v_uint64);
  case GI_TYPE_TAG_GTYPE:
    return scm_from_size_t (arg.v_size);
  case GI_TYPE_TAG_FLOAT:
    return scm_from_double (arg.v_float);
  case GI_TYPE_TAG_DOUBLE:
    return scm_from_double (arg.v_double);
  case GI_TYPE_TAG_FILENAME:
  case GI_TYPE_TAG_UTF8:
    return scm_from_locale_string (arg.v_string);
  case GI_TYPE_TAG_INTERFACE:
    return gi_interface_to_scm (arg_type,
                                transfer_type,
                                arg);
  default:
    return SCM_UNSPECIFIED;
  }
}

SCM
gi_interface_to_scm (GITypeInfo *arg_type,
                     GITransfer transfer_type,
                     GArgument arg)
{
  GIBaseInfo *base_info;
  gpointer c_instance;
  SCM scm_instance;

  base_info = g_type_info_get_interface (arg_type);
  c_instance = arg.v_pointer;

  switch (g_base_info_get_type (base_info)) {
  case GI_INFO_TYPE_OBJECT:
  case GI_INFO_TYPE_INTERFACE:
    scm_instance = scm_c_gtype_instance_to_scm (c_instance);
    if (transfer_type == GI_TRANSFER_EVERYTHING)
      scm_c_gtype_instance_unref (c_instance);

    break;
  case GI_INFO_TYPE_BOXED:
    {
      GIRegisteredTypeInfo *reg_type;
      GType gtype;

      reg_type = (GIRegisteredTypeInfo *) arg_type;
      gtype = g_registered_type_info_get_g_type (reg_type);

      if (transfer_type == GI_TRANSFER_EVERYTHING)
        scm_instance = scm_c_gvalue_new_take_boxed
          (gtype,
           c_instance);
      else
        scm_instance = scm_c_gvalue_new_from_boxed
          (gtype,
           c_instance);

      break;
    }
  default:
    scm_instance = SCM_UNSPECIFIED;
  }

  return scm_instance;
}
void
scm_to_gi_arg (SCM scm_arg,
               GITypeInfo *arg_type,
               GITransfer transfer_type,
               GIScopeType scope_type,
               GICallableInfo *destroy_info,
               GArgument *arg,
               GArgument *destroy_arg)
{
  switch (g_type_info_get_tag (arg_type)) {
  case GI_TYPE_TAG_VOID:
    arg->v_pointer = SCM2PTR (scm_arg);
    break;
  case GI_TYPE_TAG_BOOLEAN:
    arg->v_boolean = scm_to_bool (scm_arg);
    break;
  case GI_TYPE_TAG_INT8:
    arg->v_int8 = scm_to_int8 (scm_arg);
    break;
  case GI_TYPE_TAG_UINT8:
    arg->v_uint8 = scm_to_uint8 (scm_arg);
    break;
  case GI_TYPE_TAG_INT16:
    arg->v_int16 = scm_to_int16 (scm_arg);
    break;
  case GI_TYPE_TAG_UINT16:
    arg->v_uint16 = scm_to_uint16 (scm_arg);
    break;
  case GI_TYPE_TAG_INT32:
    arg->v_int = scm_to_int32 (scm_arg);
    break;
  case GI_TYPE_TAG_UINT32:
    arg->v_int = scm_to_int32 (scm_arg);
    break;
  case GI_TYPE_TAG_INT64:
    arg->v_int64 = scm_to_int64 (scm_arg);
    break;
  case GI_TYPE_TAG_UINT64:
    arg->v_uint64 = scm_to_uint64 (scm_arg);
    break;
  case GI_TYPE_TAG_GTYPE:
    arg->v_ssize = scm_to_ssize_t (scm_arg);
    break;
  case GI_TYPE_TAG_FLOAT:
    arg->v_float = scm_to_double (scm_arg);
    break;
  case GI_TYPE_TAG_DOUBLE:
    arg->v_double = scm_to_double (scm_arg);
    break;
  case GI_TYPE_TAG_UTF8:
  case GI_TYPE_TAG_FILENAME:
    arg->v_string = scm_to_locale_string (scm_arg);
    break;
  case GI_TYPE_TAG_INTERFACE:
    {
      GIBaseInfo *base_info;

      base_info = g_type_info_get_interface (arg_type);

      scm_to_gi_interface (scm_arg,
                           g_base_info_get_type (base_info),
                           transfer_type,
                           scope_type,
                           base_info,
                           destroy_info,
                           arg,
                           destroy_arg);
      break;
    }
  default:
    break;
  }
}
void
scm_to_gi_interface (SCM scm_arg,
                     GIInfoType arg_type,
                     GITransfer transfer_type,
                     GIScopeType scope_type,
                     GIBaseInfo *info,
                     GICallableInfo *destroy_info,
                     GArgument *arg,
                     GArgument *destroy_arg)
{
  gpointer *c_instance;

  c_instance = &(arg->v_pointer);

  switch (arg_type) {
  case GI_INFO_TYPE_OBJECT:
  case GI_INFO_TYPE_INTERFACE:
    if (transfer_type == GI_TRANSFER_EVERYTHING)
      scm_c_gtype_instance_ref (*c_instance);

    *c_instance = scm_c_scm_to_gtype_instance (scm_arg);

    break;
  case GI_INFO_TYPE_BOXED:
    {
      GIRegisteredTypeInfo *reg_type;
      GType gtype;

      reg_type = (GIRegisteredTypeInfo *) arg_type;
      gtype = g_registered_type_info_get_g_type (reg_type);

      if (transfer_type == GI_TRANSFER_EVERYTHING)
        *c_instance = scm_c_gvalue_dup_boxed (scm_arg);
      else
        *c_instance = scm_c_gvalue_peek_boxed (scm_arg);

      break;
    }
  case GI_INFO_TYPE_CALLBACK:
    {
      CallbackData *data;

      data = callback_data_new (scm_arg,
                                (GICallableInfo *) info,
                                scope_type);

      *c_instance = g_callable_info_prepare_closure (
                                                     data->callable_info,
                                                     data->cif,
                                                     callback_closure,
                                                     data);

      if (scope_type == GI_SCOPE_TYPE_NOTIFIED) {
        DestroyNotifyData *destroy_data;
        GITypeInfo *destroy_type;
        GIBaseInfo *base_info;

        destroy_type = g_arg_info_get_type (
                                            destroy_info);
        base_info = g_type_info_get_interface (
                                               destroy_type);

        destroy_data = destroy_notify_data_new (data);

        destroy_arg->v_pointer =
          g_callable_info_prepare_closure (
                                           base_info,
                                           destroy_data->cif,
                                           destroy_notify_callback,
                                           destroy_data);
      }
      break;
    }
  default:
    break;
  }
}

SCM_DEFINE (scm_g_arg_info_get_direction, "g-arg-info-get-direction", 1, 0, 0,
            (SCM scm_arg_info),
            ""
            )
{
  GIArgInfo *arg_info;

  arg_info = (GIArgInfo *) SCM_SMOB_DATA (scm_arg_info);

  return scm_from_int (g_arg_info_get_direction (arg_info));
}

SCM_DEFINE (scm_g_arg_info_is_return_value, "g-arg-info-is-return-value?", 1, 0, 0,
            (SCM scm_arg_info),
            ""
            )
{
  GIArgInfo *arg_info;

  arg_info = (GIArgInfo *) SCM_SMOB_DATA (scm_arg_info);

  return scm_from_bool (g_arg_info_is_return_value (arg_info));
}

SCM_DEFINE (scm_g_arg_info_is_optional, "g-arg-info-is-optional?", 1, 0, 0,
            (SCM scm_arg_info),
            ""
            )
{
  GIArgInfo *arg_info;

  arg_info = (GIArgInfo *) SCM_SMOB_DATA (scm_arg_info);

  return scm_from_bool (g_arg_info_is_optional (arg_info));
}

SCM_DEFINE (scm_g_arg_info_may_be_null, "g-arg-info-may-be-null?", 1, 0, 0,
            (SCM scm_arg_info),
            ""
            )
{
  GIArgInfo *arg_info;

  arg_info = (GIArgInfo *) SCM_SMOB_DATA (scm_arg_info);

  return scm_from_bool (g_arg_info_may_be_null (arg_info));
}

SCM_DEFINE (scm_g_arg_info_get_ownership_transfer, "g-arg-info-get-ownership-transfer", 1, 0, 0,
            (SCM scm_arg_info),
            ""
            )
{
  GIArgInfo *arg_info;

  arg_info = (GIArgInfo *) SCM_SMOB_DATA (scm_arg_info);

  return scm_from_int (g_arg_info_get_ownership_transfer (arg_info));
}

SCM_DEFINE (scm_g_arg_info_get_scope, "g-arg-info-get-scope", 1, 0, 0,
            (SCM scm_arg_info),
            ""
            )
{
  GIArgInfo *arg_info;

  arg_info = (GIArgInfo *) SCM_SMOB_DATA (scm_arg_info);

  return scm_from_int (g_arg_info_get_scope (arg_info));
}

SCM_DEFINE (scm_g_arg_info_get_closure, "g-arg-info-get-closure", 1, 0, 0,
            (SCM scm_arg_info),
            ""
            )
{
  GIArgInfo *arg_info;

  arg_info = (GIArgInfo *) SCM_SMOB_DATA (scm_arg_info);

  return scm_from_int (g_arg_info_get_closure (arg_info));
}

SCM_DEFINE (scm_g_arg_info_get_destroy, "g-arg-info-get-destroy", 1, 0, 0,
            (SCM scm_arg_info),
            ""
            )
{
  GIArgInfo *arg_info;

  arg_info = (GIArgInfo *) SCM_SMOB_DATA (scm_arg_info);

  return scm_from_int (g_arg_info_get_destroy (arg_info));
}

SCM_DEFINE (scm_g_arg_info_get_type, "g-arg-info-get-type", 1, 0, 0,
            (SCM scm_arg_info),
            ""
            )
{
  GIArgInfo *arg_info;
  GITypeInfo *type_info;
  SCM scm_type;

  arg_info = (GIArgInfo *) SCM_SMOB_DATA (scm_arg_info);
  type_info = g_arg_info_get_type (arg_info);

  if (type_info == NULL)
    scm_type = SCM_BOOL_F;
  else {
    scm_type = scm_make_smob (type_info_t);
    SCM_SET_SMOB_DATA (scm_type, type_info);
  }

  return scm_type;
}

void
gi_argument_init (void)
{
  #ifndef SCM_MAGIC_SNARFER
  #include "gi-argument.x"
  #endif

  arg_info_t = scm_make_smob_type ("g-i-arg-info", 0);

  scm_c_define (DIRECTION_IN_SYMBOL, scm_from_int (GI_DIRECTION_IN));
  scm_c_define (DIRECTION_OUT_SYMBOL, scm_from_int (GI_DIRECTION_OUT));
  scm_c_define (DIRECTION_INOUT_SYMBOL, scm_from_int (GI_DIRECTION_INOUT));

  scm_c_define (SCOPE_TYPE_INVALID_SYMBOL, scm_from_int (GI_SCOPE_TYPE_INVALID));
  scm_c_define (SCOPE_TYPE_CALL_SYMBOL, scm_from_int (GI_SCOPE_TYPE_CALL));
  scm_c_define (SCOPE_TYPE_ASYNC_SYMBOL, scm_from_int (GI_SCOPE_TYPE_ASYNC));
  scm_c_define (SCOPE_TYPE_NOTIFIED_SYMBOL, scm_from_int (GI_SCOPE_TYPE_NOTIFIED));
}
