
/* -*- Mode: C; c-basic-offset: 4 -*-
 * vim: tabstop=4 shiftwidth=4 expandtab
 */


// TODO: reorg

#include <libguile.h>

#include <string.h>
#include <time.h>

#include <girffi.h>

#include "ggi-argument.h"
#include "gtype.h"
#include "gvalue.h"
#include "ggi-value.h"
#include "ggi-infos.h"
#include "ggi-basic-types.h"

gboolean
ggi_argument_to_gssize (GIArgument *arg_in,
                        GITypeTag   type_tag,
                        gssize     *gssize_out)
{
  switch (type_tag)
    {
    case GI_TYPE_TAG_INT8:
      *gssize_out = arg_in->v_int8;
      return TRUE;
    case GI_TYPE_TAG_UINT8:
      *gssize_out = arg_in->v_uint8;
      return TRUE;
    case GI_TYPE_TAG_INT16:
      *gssize_out = arg_in->v_int16;
      return TRUE;
    case GI_TYPE_TAG_UINT16:
      *gssize_out = arg_in->v_uint16;
      return TRUE;
    case GI_TYPE_TAG_INT32:
      *gssize_out = arg_in->v_int32;
      return TRUE;
    case GI_TYPE_TAG_UINT32:
      *gssize_out = arg_in->v_uint32;
      return TRUE;
    case GI_TYPE_TAG_INT64:
      if (arg_in->v_int64 > G_MAXSSIZE || arg_in->v_int64 < G_MINSSIZE)
        {
          // TODO: error
          return FALSE;
        }
      *gssize_out = (gssize) arg_in->v_int64;
      return TRUE;
    case GI_TYPE_TAG_UINT64:
      if (arg_in->v_uint64 > G_MAXSSIZE || arg_in->v_uint64 < G_MINSSIZE)
        {
          // TODO: error
          return FALSE;
        }
      *gssize_out = (gssize) arg_in->v_uint64;
      return TRUE;
    default:
      // TODO error
      return FALSE;
    }
}

static GITypeTag
_ggi_get_storage_type (GITypeInfo *type_info)
{
  GITypeTag type_tag = g_type_info_get_tag (type_info);

  if (type_tag == GI_TYPE_TAG_INTERFACE)
    {
      GIBaseInfo *interface = g_type_info_get_interface (type_info);

      switch (g_base_info_get_type (interface))
        {
        case GI_INFO_TYPE_ENUM:
        case GI_INFO_TYPE_FLAGS:
          type_tag = g_enum_info_get_storage_type ((GIEnumInfo *) interface);
          break;
        default:
          // FIXME: other types?
          break;
        }

      g_base_info_unref (interface);
    }

  return type_tag;
}

void
_ggi_hash_pointer_to_arg (GIArgument *arg,
                          GITypeInfo *type_info)
{
  GITypeTag type_tag = _ggi_get_storage_type (type_info);

  switch (type_tag)
    {
    case GI_TYPE_TAG_INT8:
      arg->v_int8 = (gint8) GPOINTER_TO_INT (arg->v_pointer);
      break;
    case GI_TYPE_TAG_INT16:
      arg->v_int16 = (gint16) GPOINTER_TO_INT (arg->v_pointer);
      break;
    case GI_TYPE_TAG_INT32:
      arg->v_int16 = (gint32) GPOINTER_TO_INT (arg->v_pointer);
      break;
    case GI_TYPE_TAG_UINT8:
      arg->v_int16 = (guint8) GPOINTER_TO_INT (arg->v_pointer);
      break;
    case GI_TYPE_TAG_UINT16:
      arg->v_int16 = (guint16) GPOINTER_TO_INT (arg->v_pointer);
      break;
    case GI_TYPE_TAG_UINT32:
      arg->v_int16 = (guint32) GPOINTER_TO_INT (arg->v_pointer);
      break;
    case GI_TYPE_TAG_GTYPE:
      arg->v_size = GPOINTER_TO_SIZE (arg->v_pointer);
      break;
    case GI_TYPE_TAG_UTF8:
    case GI_TYPE_TAG_FILENAME:
    case GI_TYPE_TAG_INTERFACE:
    case GI_TYPE_TAG_ARRAY:
      break;
    default:
      g_critical ("Unsupport type %s", g_type_tag_to_string (type_tag));
    }
}

gpointer
_ggi_arg_to_hash_pointer (const GIArgument *arg,
                          GITypeInfo       *type_info)
{
  GITypeTag type_tag = _ggi_get_storage_type (type_info);

  switch (type_tag)
    {
    case GI_TYPE_TAG_INT8:
      return GINT_TO_POINTER (arg->v_int8);
    case GI_TYPE_TAG_UINT8:
      return GINT_TO_POINTER (arg->v_uint8);
    case GI_TYPE_TAG_INT16:
      return GINT_TO_POINTER (arg->v_int16);
    case GI_TYPE_TAG_UINT16:
      return GINT_TO_POINTER (arg->v_uint16);
    case GI_TYPE_TAG_INT32:
      return GINT_TO_POINTER (arg->v_int32);
    case GI_TYPE_TAG_UINT32:
      return GINT_TO_POINTER (arg->v_uint32);
    case GI_TYPE_TAG_GTYPE:
      return GSIZE_TO_POINTER (arg->v_size);
    case GI_TYPE_TAG_UTF8:
    case GI_TYPE_TAG_FILENAME:
    case GI_TYPE_TAG_INTERFACE:
    case GI_TYPE_TAG_ARRAY:
      return arg->v_pointer;
    default:
      g_critical ("Unsupported type %s", g_type_tag_to_string(type_tag));
      return arg->v_pointer;
    }
}

gssize
_ggi_argument_array_length_marshal (gsize length_arg_index,
                                    void *user_data1,
                                    void *user_data2)
{
  GIArgInfo length_arg_info;
  GITypeInfo length_type_info;
  GIArgument length_arg;
  gssize array_len = -1;
  GValue *values = (GValue *) user_data1;
  GICallableInfo *callable_info = (GICallableInfo *) user_data2;

  g_callable_info_load_arg (callable_info, (gint) length_arg_index, &length_arg_info);
  g_arg_info_load_type (&length_arg_info, &length_type_info);

  length_arg = _ggi_argument_from_g_value (&(values[length_arg_index]),
                                           &length_type_info);

  if (!ggi_argument_to_gssize (&length_arg,
                               g_type_info_get_tag (&length_type_info),
                               &array_len))
    return -1;

  return array_len;
}

GArray *
_gg_argument_to_array (GIArgument              *arg,
                       GGIArgArrayLengthPolicy  array_length_policy,
                       void                    *user_data1,
                       void                    *user_data2,
                       GITypeInfo              *type_info,
                       gboolean                *out_free_array)
{
  GITypeInfo *item_type_info;
  gboolean is_zero_terminated;
  gsize item_size;
  gssize length;
  GArray *g_array;

  g_return_val_if_fail (g_type_info_get_tag (type_info) == GI_TYPE_TAG_ARRAY, NULL);

  if (arg->v_pointer == NULL)
    return NULL;

  switch (g_type_info_get_array_type (type_info))
    {
    case GI_ARRAY_TYPE_C:
      is_zero_terminated = g_type_info_is_zero_terminated (type_info);
      item_type_info = g_type_info_get_param_type (type_info, 0);

      item_size = _ggi_g_type_info_size (item_type_info);

      g_base_info_unref ((GIBaseInfo *) item_type_info);

      if (is_zero_terminated)
        {
          length = g_strv_length (arg->v_pointer);
        }
      else
        {
          length = g_type_info_get_array_fixed_size (type_info);
          if (length < 0)
            {
              gint length_arg_pos;

              if (G_UNLIKELY (array_length_policy == NULL))
                {
                  g_critical ("Unable to determine array_length for %p", arg->v_pointer);
                  g_array = g_array_new (is_zero_terminated, FALSE, (guint) item_size);
                  *out_free_array = TRUE;
                  return g_array;
                }

              length_arg_pos = g_type_info_get_array_length (type_info);
              g_assert (length_arg_pos >= 0);

              length = array_length_policy (length_arg_pos, user_data1, user_data2);
              if (length < 0)
                return NULL;
            }
        }
      g_assert (length >= 0);

      g_array = g_array_new (is_zero_terminated, FALSE, (guint) item_size);

      g_free (g_array->data);
      g_array->data = arg->v_pointer;
      g_array->len = (guint) length;
      *out_free_array = TRUE;
      break;
    case GI_ARRAY_TYPE_ARRAY:
    case GI_ARRAY_TYPE_BYTE_ARRAY:
      g_array = arg->v_pointer;
      *out_free_array = FALSE;
      break;
    case GI_ARRAY_TYPE_PTR_ARRAY:
      {
        GPtrArray *ptr_array = (GPtrArray *) arg->v_pointer;
        g_array = g_array_sized_new (FALSE,
                                     FALSE,
                                     sizeof (gpointer),
                                     ptr_array->len);
        g_array->data = (char *) ptr_array->pdata;
        g_array->len = ptr_array->len;
        *out_free_array = TRUE;
        break;
      }
    default:
      g_critical ("Unexpected array type %u", g_type_info_get_array_type (type_info));
      g_array = NULL;
      break;
    }

  return g_array;
}

GIArgument
_ggi_argument_from_object (SCM         scm_obj,
                           GITypeInfo *type_info,
                           GITransfer  transfer)
{
  g_debug ("_ggi_argument_from_object");

  /*
  GIArgument arg;
  GITypeTag type_tag;
  gpointer cleanup_data = NULL;

  memset (&arg, 0, sizeof (GIArgument));
  type_tag = g_type_info_get_tag (type_info);

  switch (type_tag)
    {
    case GI_TYPE_TAG_ARRAY:
      {
        SCM scm_list_length;
        guint length, i;
        gboolean is_zero_terminated;
        GITypeInfo *item_type_info;
        gsize item_size;
        GArray *array;
        GITransfer item_transfer;

        if (SCM_UNBNDP (scm_obj))
          {
            arg.v_pointer = NULL;
            break;
          }

        // TODO check is a array?
        // FINISH
      }
    }
  */
}

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
                        GIArgument return_value)
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
                                 *((GIArgument *) args[i]));

    g_base_info_unref ((GIBaseInfo*) arg_info);
    g_base_info_unref ((GIBaseInfo*) arg_type);
  }

  scm_return = scm_call_n (callback_data->scm_callback, scm_args, n_args);

  scm_return_value_to_gi (scm_return,
                          callback_data->callable_info,
                          (GIArgument *) result);

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
                        GIArgument *return_value)
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
               GIArgument arg)
{
  switch (g_type_info_get_tag (arg_type))
    {
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
      return SCM_UNDEFINED;
    }
}

SCM
gi_interface_to_scm (GITypeInfo *arg_type,
                     GITransfer transfer_type,
                     GIArgument arg)
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
scm_to_gi_arg (SCM             scm_arg,
               GITypeInfo     *arg_type,
               GITransfer      transfer_type,
               GIScopeType     scope_type,
               GICallableInfo *destroy_info,
               GIArgument     *arg,
               GIArgument     *destroy_arg)
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
                     GIArgument *arg,
                     GIArgument *destroy_arg)
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
      //GIRegisteredTypeInfo *reg_type;
      //GType gtype;

      //reg_type = (GIRegisteredTypeInfo *) arg_type;
      //gtype = g_registered_type_info_get_g_type (reg_type);

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

void
gi_argument_init (void)
{
#ifndef SCM_MAGIC_SNARFER
#include "ggi-argument.x"
#endif
}
