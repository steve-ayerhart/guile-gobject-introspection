/* -*- Mode: C; c-basic-offset: 4 -*-
 */

#include "ggi-closure.h"
#include "ggi-error.h"
#include "ggi-marshal-cleanup.h"
#include "ggi-invoke.h"
#include "ggi-ccallback.h"
#include "ggi-infos.h"

extern SCM _GGIDefaultArgPlaceholder;

typedef struct _GGICallbackCache
{
  GGIArgCache arg_cache;
  gssize user_data_index;
  gssize destroy_notify_index;
  GIScopeType scope;
  GIInterfaceInfo *interface_info;
  GGIClosureCache *closure_cache;
} GGICallbackCache;

static GSList* async_free_list;

static void
_ggi_closure_assign_scmobj_to_retval (gpointer     retval,
                                      GIArgument  *arg,
                                      GGIArgCache *arg_cache)
{
  if (retval == NULL)
    return;

  switch (arg_cache->type_tag)
    {
    case GI_TYPE_TAG_BOOLEAN:
      *((ffi_sarg *) retval) = arg->v_boolean;
      break;
    case GI_TYPE_TAG_INT8:
      *((ffi_sarg *) retval) = arg->v_int8;
      break;
    case GI_TYPE_TAG_UINT8:
      *((ffi_arg *) retval) = arg->v_uint8;
      break;
    case GI_TYPE_TAG_INT16:
      *((ffi_sarg *) retval) = arg->v_int16;
      break;
    case GI_TYPE_TAG_UINT16:
      *((ffi_arg *) retval) = arg->v_uint16;
      break;
    case GI_TYPE_TAG_INT32:
      *((ffi_sarg *) retval) = arg->v_int32;
      break;
    case GI_TYPE_TAG_UINT32:
      *((ffi_arg *) retval) = arg->v_uint32;
      break;
    case GI_TYPE_TAG_INT64:
      *((ffi_sarg *) retval) = arg->v_int64;
      break;
    case GI_TYPE_TAG_UINT64:
      *((ffi_arg *) retval) = arg->v_uint64;
      break;
    case GI_TYPE_TAG_FLOAT:
      *((gfloat *) retval) = arg->v_float;
      break;
    case GI_TYPE_TAG_DOUBLE:
      *((gdouble *) retval) = arg->v_double;
      break;
    case GI_TYPE_TAG_GTYPE:
      *((ffi_arg *) retval) = arg->v_size;
      break;
    case GI_TYPE_TAG_UNICHAR:
      *((ffi_arg *) retval) = arg->v_uint32;
      break;
    case GI_TYPE_TAG_INTERFACE:
      {
        GIBaseInfo *interface_info;

        interface_info = ((GGIInterfaceCache *) arg_cache)->interface_info;

        switch (g_base_info_get_type (interface_info)) {
        case GI_INFO_TYPE_ENUM:
          *(ffi_sarg *) retval = arg->v_int;
          break;
        case GI_INFO_TYPE_FLAGS:
          *(ffi_arg *) retval = arg->v_uint;
          break;
        default:
          *(ffi_arg *) retval = (ffi_arg) arg->v_pointer;
          break;
        }

        break;
      }
    default:
      *(ffi_arg *) retval = (ffi_arg) arg->v_pointer;
      break;
    }
}

static void
_ggi_closure_assign_scmobj_to_out_argument (gpointer     out_arg,
                                            GIArgument  *arg,
                                            GGIArgCache *arg_cache)
{
  if (out_arg == NULL)
    return;

  switch (arg_cache->type_tag)
    {
    case GI_TYPE_TAG_BOOLEAN:
      *((gboolean *) out_arg) = arg->v_boolean;
      break;
    case GI_TYPE_TAG_INT8:
      *((gint8 *) out_arg) = arg->v_int8;
      break;
    case GI_TYPE_TAG_UINT8:
      *((guint8 *) out_arg) = arg->v_uint8;
      break;
    case GI_TYPE_TAG_INT16:
      *((gint16 *) out_arg) = arg->v_int16;
      break;
    case GI_TYPE_TAG_UINT16:
      *((guint16 *) out_arg) = arg->v_uint16;
      break;
    case GI_TYPE_TAG_INT32:
      *((gint32 *) out_arg) = arg->v_int32;
      break;
    case GI_TYPE_TAG_UINT32:
      *((guint32 *) out_arg) = arg->v_uint32;
      break;
    case GI_TYPE_TAG_INT64:
      *((gint64 *) out_arg) = arg->v_int64;
      break;
    case GI_TYPE_TAG_UINT64:
      *((guint64 *) out_arg) = arg->v_uint64;
      break;
    case GI_TYPE_TAG_FLOAT:
      *((gfloat *) out_arg) = arg->v_float;
      break;
    case GI_TYPE_TAG_DOUBLE:
      *((gdouble *) out_arg) = arg->v_double;
      break;
    case GI_TYPE_TAG_GTYPE:
      *((GType *) out_arg) = arg->v_size;
      break;
    case GI_TYPE_TAG_UNICHAR:
      *((guint32 *) out_arg) = arg->v_uint32;
      break;
    case GI_TYPE_TAG_INTERFACE:
      {
        GIBaseInfo *interface_info;

        interface_info = ((GGIInterfaceCache *) arg_cache)->interface_info;

        switch (g_base_info_get_type (interface_info)) {
        case GI_INFO_TYPE_ENUM:
          *(gint *) out_arg = arg->v_int;
          break;
        case GI_INFO_TYPE_FLAGS:
          *(guint *) out_arg = arg->v_uint;
          break;
        case GI_INFO_TYPE_STRUCT:
          if (!arg_cache->is_pointer) {
            if (arg->v_pointer != NULL) {
              gsize item_size = _ggi_g_type_info_size (arg_cache->type_info);
              memcpy (out_arg, arg->v_pointer, item_size);
            }
            break;
          }
          *((gpointer *) out_arg) = arg->v_pointer;
          break;
        default:
          *((gpointer *) out_arg) = arg->v_pointer;
          break;
        }
        break;
      }

    default:
      *((gpointer *) out_arg) = arg->v_pointer;
      break;

    }
}

static void
_ggi_closure_convert_ffi_arguments (GGIInvokeArgState *state,
                                    GGICallableCache  *cache,
                                    void             **args)
{
  guint i;

  for (i = 0; i < _ggi_callable_cache_args_len (cache); i++) {
    GGIArgCache *arg_cache = g_ptr_array_index (cache->args_cache, i);
    gpointer arg_pointer;

    if (arg_cache->direction & GGI_DIRECTION_FROM_SCM) {
      state[i].arg_value.v_pointer = * (gpointer *) args[i];

      if (state[i].arg_value.v_pointer == NULL)
        continue;

      state[i].arg_pointer.v_pointer = state[i].arg_value.v_pointer;
      arg_pointer = state[i].arg_value.v_pointer;
    } else {
      arg_pointer = args[i];
    }

    switch (arg_cache->type_tag) {
    case GI_TYPE_TAG_BOOLEAN:
      state[i].arg_value.v_boolean = * (gboolean *) arg_pointer;
      break;
    case GI_TYPE_TAG_INT8:
      state[i].arg_value.v_int8 = * (gint8 *) arg_pointer;
      break;
    case GI_TYPE_TAG_UINT8:
      state[i].arg_value.v_uint8 = * (guint8 *) arg_pointer;
      break;
    case GI_TYPE_TAG_INT16:
      state[i].arg_value.v_int16 = * (gint16 *) arg_pointer;
      break;
    case GI_TYPE_TAG_UINT16:
      state[i].arg_value.v_uint16 = * (guint16 *) arg_pointer;
      break;
    case GI_TYPE_TAG_INT32:
      state[i].arg_value.v_int32 = * (gint32 *) arg_pointer;
      break;
    case GI_TYPE_TAG_UINT32:
      state[i].arg_value.v_uint32 = * (guint32 *) arg_pointer;
      break;
    case GI_TYPE_TAG_INT64:
      state[i].arg_value.v_int64 = * (gint64 *) arg_pointer;
      break;
    case GI_TYPE_TAG_UINT64:
      state[i].arg_value.v_uint64 = * (guint64 *) arg_pointer;
      break;
    case GI_TYPE_TAG_FLOAT:
      state[i].arg_value.v_float = * (gfloat *) arg_pointer;
      break;
    case GI_TYPE_TAG_DOUBLE:
      state[i].arg_value.v_double = * (gdouble *) arg_pointer;
      break;
    case GI_TYPE_TAG_UTF8:
      state[i].arg_value.v_string = * (gchar **) arg_pointer;
      break;
    case GI_TYPE_TAG_INTERFACE:
      {
        GIBaseInfo *interface;
        GIInfoType interface_type;

        interface = ((GGIInterfaceCache *) arg_cache)->interface_info;
        interface_type = g_base_info_get_type (interface);

        if (interface_type == GI_INFO_TYPE_ENUM) {
          state[i].arg_value.v_int = * (gint *) arg_pointer;
        } else if (interface_type == GI_INFO_TYPE_FLAGS) {
          state[i].arg_value.v_uint = * (guint *) arg_pointer;
        } else {
          state[i].arg_value.v_pointer = * (gpointer *) arg_pointer;
        }
        break;
      }
    case GI_TYPE_TAG_UNICHAR:
      state[i].arg_value.v_uint32 = * (guint32 *) arg_pointer;
      break;
    case GI_TYPE_TAG_ERROR:
    case GI_TYPE_TAG_GHASH:
    case GI_TYPE_TAG_GLIST:
    case GI_TYPE_TAG_GSLIST:
    case GI_TYPE_TAG_ARRAY:
    case GI_TYPE_TAG_VOID:
      state[i].arg_value.v_pointer = * (gpointer *) arg_pointer;
      break;
    default:
      g_warning ("Unhandled type tag %s",
                 g_type_tag_to_string (arg_cache->type_tag));
      state[i].arg_value.v_pointer = 0;
    }
  }

  if (cache->throws) {
    gssize error_index = _ggi_callable_cache_args_len (cache);

    state[error_index].arg_value.v_pointer = * (gpointer *) args[error_index];

  }
}

static gboolean
_invoke_state_init_from_cache (GGIInvokeState   *state,
                               GGIClosureCache  *closure_cache,
                               void            **args)
{
  GGICallableCache *cache = (GGICallableCache *) closure_cache;

  state->n_args = _ggi_callable_cache_args_len (cache);
  state->n_scm_in_args = state->n_args;

  if (cache->throws)
    state->n_args++;

  state->scm_in_args = SCM_EOL;

  state->args = NULL;
  state->error = NULL;

  if (!_ggi_invoke_arg_state_init (state))
    return FALSE;

  state->ffi_args = NULL;

  _ggi_closure_convert_ffi_arguments (state->args, cache, args);

  return TRUE;
}

static void
_invoke_state_clear (GGIInvokeState *state)
{
  _ggi_invoke_arg_state_free (state);
}

static gboolean
_ggi_closure_convert_arguments (GGIInvokeState  *state,
                                GGIClosureCache *closure_cache)
{
  GGICallableCache *cache = (GGICallableCache *) closure_cache;
  gssize n_in_args = 0;
  gssize i;

  for (i = 0; (gsize) i < _ggi_callable_cache_args_len (cache); i++)
    {
      GGIArgCache *arg_cache;

      arg_cache = g_ptr_array_index (cache->args_cache, i);

      if (arg_cache->direction & GGI_DIRECTION_TO_SCM)
        {
          SCM scm_value;

          if (cache->user_data_index == i)
            {
              if (state->user_data == SCM_UNSPECIFIED)
                {
                  
                  scm_value = SCM_UNSPECIFIED;
                }
              else
                {
                  if (scm_is_false (scm_list_p (state->user_data)))
                    {
                      scm_misc_error ("type error",
                                      "expected list for callback user_data",
                                      NULL);
                      return FALSE;
                    }

                  state->scm_in_args = state->user_data;

                  continue;
                }
            }
          else if (arg_cache->meta_type != GGI_META_ARG_TYPE_PARENT)
            {
              continue;
            }
          else
            {
              gpointer cleanup_data = NULL;

              scm_value = arg_cache->to_scm_marshaller (state,
                                                        cache,
                                                        arg_cache,
                                                        &state->args[i].arg_value,
                                                        &cleanup_data);
              state->args[i].to_scm_arg_cleanup_data = cleanup_data;

              if (scm_value == SCM_UNSPECIFIED)
                {
                  ggi_marshal_cleanup_args_to_scm_parameter_fail (state,
                                                                  cache,
                                                                  i);
                  return FALSE;
                }
            }

          state->scm_in_args = scm_append (scm_list_2 (state->scm_in_args,
                                                       scm_value));
          n_in_args++;
        }
    }

  return TRUE;
}

static gboolean
_ggi_closure_set_out_arguments (GGIInvokeState   *state,
                                GGICallableCache *cache,
                                SCM               scm_retval,
                                void             *resp)
{
  gssize i;
  gssize i_scm_retval = 0;
  gboolean success;

  if (cache->return_cache->type_tag != GI_TYPE_TAG_VOID)
    {
      SCM scm_item = scm_retval;

      if (scm_is_true (scm_list_p (scm_retval)))
        scm_item = scm_list_ref (scm_retval, 0);

      success = cache->return_cache->from_scm_marshaller (state,
                                                          cache,
                                                          cache->return_cache,
                                                          scm_item,
                                                          &state->return_arg,
                                                          &state->args[0].arg_cleanup_data);

      if (!success)
        {
          ggi_marshal_cleanup_args_return_fail (state,
                                                cache);
          return FALSE;
        }

      _ggi_closure_assign_scmobj_to_retval (resp,
                                            &state->return_arg,
                                            cache->return_cache);

      i_scm_retval++;
    }

  for (i = 0; (gsize) i < _ggi_callable_cache_args_len (cache); i++)
    {
      GGIArgCache *arg_cache = g_ptr_array_index (cache->args_cache, i);

      if (arg_cache->direction & GGI_DIRECTION_FROM_SCM)
        {
          SCM scm_item = scm_retval;

          if (arg_cache->type_tag == GI_TYPE_TAG_ERROR)
            {
              * (GError **) state->args[i].arg_pointer.v_pointer = NULL;
              continue;
            }

          if (scm_is_true (scm_list_p (scm_retval)))
            {
              scm_item = scm_list_ref (scm_retval, scm_from_int (i_scm_retval));
            }
          else if (i_scm_retval != 0)
            {
              ggi_marshal_cleanup_args_to_scm_parameter_fail (state,
                                                              cache,
                                                              i_scm_retval);
              return FALSE;
            }

          success = arg_cache->from_scm_marshaller (state,
                                                    cache,
                                                    arg_cache,
                                                    scm_item,
                                                    &state->args[i].arg_value,
                                                    &state->args[i_scm_retval].arg_cleanup_data);

          if (!success)
            {
              ggi_marshal_cleanup_args_to_scm_parameter_fail (state,
                                                              cache,
                                                              i_scm_retval);
              return FALSE;
            }

          _ggi_closure_assign_scmobj_to_out_argument (state->args[i].arg_pointer.v_pointer,
                                                         &state->args[i].arg_value,
                                                         arg_cache);

          i_scm_retval++;
        }
    }

  return TRUE;
}

static void
_ggi_closure_clear_retvals (GGIInvokeState   *state,
                            GGICallableCache *cache,
                            gpointer          resp)
{
  gsize i;
  GIArgument arg = { 0, };

  if (cache->return_cache->type_tag != GI_TYPE_TAG_VOID)
    _ggi_closure_assign_scmobj_to_retval (resp,
                                          &arg,
                                          cache->return_cache);

  for (i = 0; i < _ggi_callable_cache_args_len (cache); i++)
    {
      GGIArgCache *arg_cache = g_ptr_array_index (cache->args_cache, i);

      if (arg_cache->direction & GGI_DIRECTION_FROM_SCM)
        _ggi_closure_assign_scmobj_to_out_argument (state->args[i].arg_pointer.v_pointer,
                                                    &arg,
                                                    arg_cache);

      if (cache->throws)
        {
          // TODO: really do stuff
        }
    }
}

void
_gg_closure_handle (ffi_cif  *cif,
                    void     *result,
                    void    **args,
                    void     *data)
{
  GGICClosure *closure = data;
  SCM retval;
  gboolean success;
  GGIInvokeState state = { 0, };
  // TODO
}

static gboolean
ggi_arg_callback_setup_from_info (GGICallbackCache *arg_cache,
                                  GITypeInfo       *type_info,
                                  GIArgInfo        *arg_info,
                                  GITransfer        transfer,
                                  GGIDirection      direction,
                                  GIInterfaceInfo  *iface_info,
                                  GGICallableCache *callable_cache)
{
  // TODO: implement
  return TRUE;
}

GGIArgCache *
ggi_arg_callback_new_from_info (GITypeInfo       *type_info,
                                GIArgInfo        *arg_info,
                                GITransfer        transfer,
                                GGIDirection      direction,
                                GIInterfaceInfo  *iface_info,
                                GGICallableCache *callable_cache)
{
  g_debug ("ggi_arg_callback_new_from_info");

  gboolean res = FALSE;
  GGICallbackCache *callback_cache;

  callback_cache = g_slice_new0 (GGICallbackCache);
  if (callback_cache == NULL)
    return NULL;

  res = ggi_arg_callback_setup_from_info (callback_cache,
                                          type_info,
                                          arg_info,
                                          transfer,
                                          direction,
                                          iface_info,
                                          callable_cache);

  if (res)
    {
      return (GGIArgCache *) callback_cache;
    }
  else
    {
      ggi_arg_cache_free ((GGIArgCache *) callback_cache);
      return NULL;
    }
}
