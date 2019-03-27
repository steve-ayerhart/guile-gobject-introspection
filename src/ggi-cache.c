/* -*- Mode: C; c-basic-offset: 4 -*-
 * vim: tabstop=4 shiftwidth=4 expandtab
 */

#include <libguile.h>
#include <girffi.h>
#include <ffi.h>
#include <girepository.h>

#include "ggi-cache.h"
#include "ggi-infos.h"
#include "ggi-invoke.h"
#include "ggi-object.h"
#include "ggi-hashtable.h"
#include "ggi-error.h"
#include "ggi-array.h"
#include "ggi-closure.h"
#include "ggi-struct-marshal.h"
#include "ggi-enum-marshal.h"
#include "ggi-basic-types.h"

/* _arg_info_default_value
 * info:
 * arg: (out): GIArgument to fill in with default value.
 *
 * This is currently a place holder API which only supports "allow-none" pointer args.
 * Once defaults are part of the GI API, we can replace this with: g_arg_info_default_value
 * https://bugzilla.gnome.org/show_bug.cgi?id=558620
 *
 * Returns: TRUE if the given argument supports a default value and was filled in.
 */
static gboolean
_arg_info_default_value (GIArgInfo *info, GIArgument *arg)
{
    if (g_arg_info_may_be_null (info)) {
        arg->v_pointer = NULL;
        return TRUE;

    }

    return FALSE;
}

/* ggi_arg_base_setup:
 * arg_cache: argument cache to initialize
 * type_info: source for type related attributes to cache
 * arg_info: (allow-none): source for argument related attributes to cache
 * transfer: transfer mode to store in the argument cache
 * direction: marshaling direction to store in the cache
 *
 * Initializer for GGIArgCache
 *
 * Returns: TRUE on success and FALSE on failure
 */

gboolean
ggi_arg_base_setup (GGIArgCache *arg_cache,
                    GITypeInfo  *type_info,
                    GIArgInfo   *arg_info,
                    GITransfer   transfer,
                    GIDirection  direction)
{
    arg_cache->direction = direction;
    arg_cache->transfer = transfer;
    arg_cache->scm_arg_index = -1;
    arg_cache->c_arg_index = -1;

    if (type_info != NULL) {
        arg_cache->is_pointer = g_type_info_is_pointer (type_info);
        arg_cache->type_tag = g_type_info_get_tag (type_info);
        g_base_info_ref ((GIBaseInfo *) type_info);
        arg_cache->type_info = type_info;
    }

    if (arg_info != NULL) {
        if (!arg_cache->has_default) {
            /* It is possible has_default was set somewhere else */
            arg_cache->has_default = _arg_info_default_value (arg_info,
                                                              &arg_cache->default_value);
        }
        arg_cache->arg_name = g_base_info_get_name ((GIBaseInfo *) arg_info);
        arg_cache->allow_none = g_arg_info_may_be_null (arg_info);

        if (arg_cache->type_tag == GI_TYPE_TAG_INTERFACE || arg_cache->type_tag == GI_TYPE_TAG_ARRAY)
            arg_cache->is_caller_allocates = g_arg_info_is_caller_allocates (arg_info);
        else
            arg_cache->is_caller_allocates = FALSE;
    }

    return TRUE;
}

GGIArgCache *
ggi_arg_cache_alloc (void)
{
    return g_slice_new0 (GGIArgCache);
}

void
ggi_arg_cache_free (GGIArgCache *cache)
{
    if (cache == NULL)
        return;

    if (cache->type_info != NULL)
        g_base_info_unref ((GIBaseInfo *)cache->type_info);
    if (cache->destroy_notify)
        cache->destroy_notify (cache);
    else
        g_slice_free (GGIArgCache, cache);
}

/* GGIInterfaceCache */

static GGIArgCache *
_arg_cache_new_for_interface (GIInterfaceInfo *iface_info,
                              GITypeInfo       *type_info,
                              GIArgInfo        *arg_info,
                              GITransfer        transfer,
                              GGIDirection      direction,
                              GGICallableCache *callable_cache)
{
    GIInfoType info_type;

    info_type = g_base_info_get_type ((GIBaseInfo *) iface_info);

    switch (info_type)
        {
        case GI_INFO_TYPE_CALLBACK:
            return ggi_arg_callback_new_from_info (type_info,
                                                   arg_info,
                                                   transfer,
                                                   direction,
                                                   iface_info,
                                                   callable_cache);
        case GI_INFO_TYPE_OBJECT:
        case GI_INFO_TYPE_INTERFACE:
            return ggi_arg_gobject_new_from_info (type_info,
                                                  arg_info,
                                                  transfer,
                                                  direction,
                                                  iface_info,
                                                  callable_cache);
        case GI_INFO_TYPE_BOXED:
        case GI_INFO_TYPE_STRUCT:
        case GI_INFO_TYPE_UNION:
            return ggi_arg_struct_new_from_info (type_info,
                                                 arg_info,
                                                 transfer,
                                                 direction,
                                                 iface_info);
        case GI_INFO_TYPE_ENUM:
            return ggi_arg_enum_new_from_info (type_info,
                                               arg_info,
                                               transfer,
                                               direction,
                                               iface_info);
        case GI_INFO_TYPE_FLAGS:
            return ggi_arg_flags_new_from_info (type_info,
                                                arg_info,
                                                transfer,
                                                direction,
                                                iface_info);
        default:
            g_assert_not_reached ();
        }

    return NULL;
}


static void
_interface_cache_free_func (GGIInterfaceCache *cache)
{
    if (cache != NULL) {
        if (cache->type_name != NULL)
            g_free (cache->type_name);
        if (cache->interface_info != NULL)
            g_base_info_unref ((GIBaseInfo *) cache->interface_info);
        g_slice_free (GGIInterfaceCache, cache);
    }
}

gboolean
ggi_arg_interface_setup (GGIInterfaceCache *iface_cache,
                         GITypeInfo        *type_info,
                         GIArgInfo         *arg_info,    /* may be NULL for return arguments */
                         GITransfer         transfer,
                         GGIDirection       direction,
                         GIInterfaceInfo   *iface_info)
{
    if (!ggi_arg_base_setup ((GGIArgCache *) iface_cache,
                             type_info,
                             arg_info,
                             transfer,
                             direction)) {
        return FALSE;
    }

    ((GGIArgCache *) iface_cache)->destroy_notify = (GDestroyNotify)_interface_cache_free_func;

    g_base_info_ref ((GIBaseInfo *)iface_info);
    iface_cache->interface_info = iface_info;
    iface_cache->arg_cache.type_tag = GI_TYPE_TAG_INTERFACE;
    iface_cache->type_name = (gchar*)scm_from_utf8_string ("TYPE!!!!");
    iface_cache->type_name = _ggi_g_base_info_get_fullname (iface_info);
    iface_cache->g_type = g_registered_type_info_get_g_type ((GIRegisteredTypeInfo *) iface_info);
    iface_cache->scm_type = scm_from_int (1);
    //    iface_cache->scm_type = ggi_type_import_by_gi_info ((GIBaseInfo *) iface_info);

    if (iface_cache->scm_type == NULL) {
        return FALSE;
    }

    return TRUE;
}


GGIArgCache *
ggi_arg_cache_new (GITypeInfo *type_info,
                   GIArgInfo *arg_info,
                   GITransfer transfer,
                   GIDirection direction,
                   GGICallableCache *callable_cache,
                   gssize c_arg_index,
                   gssize scm_arg_index)
{
    g_debug ("ggi_arg_cache_new");

    GGIArgCache *arg_cache = NULL;
    GITypeTag type_tag;

    type_tag = g_type_info_get_tag (type_info);

    switch (type_tag)
        {
        case GI_TYPE_TAG_VOID:
        case GI_TYPE_TAG_BOOLEAN:
        case GI_TYPE_TAG_INT8:
        case GI_TYPE_TAG_UINT8:
        case GI_TYPE_TAG_INT16:
        case GI_TYPE_TAG_UINT16:
        case GI_TYPE_TAG_INT32:
        case GI_TYPE_TAG_UINT32:
        case GI_TYPE_TAG_INT64:
        case GI_TYPE_TAG_UINT64:
        case GI_TYPE_TAG_FLOAT:
        case GI_TYPE_TAG_DOUBLE:
        case GI_TYPE_TAG_UNICHAR:
        case GI_TYPE_TAG_GTYPE:
        case GI_TYPE_TAG_UTF8:
        case GI_TYPE_TAG_FILENAME:
            arg_cache = ggi_arg_basic_type_new_from_info (type_info,
                                                          arg_info,
                                                          transfer,
                                                          direction);
            break;

        case GI_TYPE_TAG_ARRAY:
            arg_cache = ggi_arg_garray_new_from_info (type_info,
                                                      arg_info,
                                                      transfer,
                                                      direction,
                                                      callable_cache);

            if (arg_cache == NULL)
                return NULL;

            ggi_arg_garray_len_arg_setup (arg_cache,
                                          type_info,
                                          callable_cache,
                                          direction,
                                          c_arg_index,
                                          &scm_arg_index);
            break;
        case GI_TYPE_TAG_GSLIST:
            g_critical ("GSLIST NOT IMPLEMENTED");
            // TODO
            break;
        case GI_TYPE_TAG_GHASH:
            arg_cache = ggi_arg_hash_table_new_from_info (type_info,
                                              arg_info,
                                              transfer,
                                              direction,
                                              callable_cache);
            break;
        case GI_TYPE_TAG_INTERFACE:
            arg_cache = _arg_cache_new_for_interface ((GIInterfaceInfo *) g_type_info_get_interface (type_info),
                                                      type_info,
                                                      arg_info,
                                                      transfer,
                                                      direction,
                                                      callable_cache);
            //            g_base_info_unref ((GIBaseInfo *) interface_info);
            break;
        case GI_TYPE_TAG_ERROR:
            arg_cache = ggi_arg_gerror_new_from_info (type_info,
                                                      arg_info,
                                                      transfer,
                                                      direction);
            break;
        default:
            break;
        }

    if (arg_cache!= NULL)
        {
            arg_cache->scm_arg_index = scm_arg_index;
            arg_cache->c_arg_index = c_arg_index;
        }

    return arg_cache;
}
static GGIDirection
_ggi_get_direction (GGICallableCache *callable_cache, GIDirection gi_direction)
{
    if (gi_direction == GI_DIRECTION_INOUT)
        return GGI_DIRECTION_BIDIRECTIONAL;
    else if (gi_direction == GI_DIRECTION_IN)
        {
            if (callable_cache->calling_context != GGI_CALLING_CONTEXT_IS_FROM_SCM)
                return GGI_DIRECTION_TO_SCM;
            return GGI_DIRECTION_FROM_SCM;
        }
    else
        {
            if (callable_cache->calling_context !=GGI_CALLING_CONTEXT_IS_FROM_SCM)
                return GGI_DIRECTION_FROM_SCM;
            return GGI_DIRECTION_TO_SCM;
        }
}

void
ggi_callable_cache_free (GGICallableCache *cache)
{
    cache->deinit (cache);
    g_free (cache);
}

static gboolean
_callable_cache_generate_args_cache_real (GGICallableCache *callable_cache,
                                          GICallableInfo   *callable_info)
{
    g_debug ("callable_cache_generate_args_cache_real");

    gint i;
    guint arg_index;
    GITypeInfo *return_info;
    GITransfer return_transfer;
    GGIArgCache *return_cache;
    GGIDirection return_direction;
    gssize last_explicit_arg_index;
    SCM values_names;
    GSList *arg_cache_item;

    return_direction = _ggi_get_direction (callable_cache, GI_DIRECTION_OUT);

    return_info = g_callable_info_get_return_type (callable_info);
    return_transfer = g_callable_info_get_caller_owns (callable_info);
    return_cache = ggi_arg_cache_new (return_info,
                                      NULL,
                                      return_transfer,
                                      return_direction,
                                      callable_cache,
                                      -1,
                                      -1);

    if (return_cache == NULL)
        return FALSE;

    return_cache->is_skipped = g_callable_info_skip_return (callable_info);
    callable_cache->return_cache = return_cache;

    g_base_info_unref (return_info);

    callable_cache->user_data_index = -1;

    for (i = 0, arg_index = (guint)callable_cache->args_offset;
         arg_index < _ggi_callable_cache_args_len (callable_cache);
         i++, arg_index++)
        {
            GGIArgCache *arg_cache = NULL;
            GIArgInfo *arg_info;
            GGIDirection direction;

            arg_info = g_callable_info_get_arg (callable_info, i);

            // only happens when dealing with callbacks
            if (g_arg_info_get_closure (arg_info) == i)
                {
                    // TODO: handle
                }
            else
                {
                    GITypeInfo *type_info;

                    direction = _ggi_get_direction (callable_cache,
                                                    g_arg_info_get_direction (arg_info));
                    type_info = g_arg_info_get_type (arg_info);

                    /* must be a child arg filled in by its owner
                     * and continue
                     * fill in it's c_arg_index, add to the in count
                     */
                    arg_cache = _ggi_callable_cache_get_arg (callable_cache, arg_index);
                    if (arg_cache != NULL)
                        {

                            /* ensure c_arg_index always alsign with callable_cache->args_cache
                             * and all of the various GGIInvokeState arrays. */
                            arg_cache->c_arg_index = arg_index;

                            if (arg_cache->meta_type == GGI_META_ARG_TYPE_CHILD_WITH_SCMARG)
                                {
                                    arg_cache->scm_arg_index = callable_cache->n_scm_args;
                                    callable_cache->n_scm_args++;
                                }

                            if (direction & GGI_DIRECTION_TO_SCM)
                                callable_cache->n_to_scm_args++;

                            arg_cache->type_tag = g_type_info_get_tag (type_info);
                        }
                    else
                        {

                            GITransfer transfer;
                            gssize scm_arg_index = -1;

                            transfer = g_arg_info_get_ownership_transfer (arg_info);

                            if (direction & GGI_DIRECTION_FROM_SCM)
                                {
                                    scm_arg_index = callable_cache->n_scm_args;
                                    callable_cache->n_scm_args++;
                                }

                            arg_cache = ggi_arg_cache_new (type_info,
                                                           arg_info,
                                                           transfer,
                                                           direction,
                                                           callable_cache,
                                                           arg_index,
                                                           scm_arg_index);

                            if (arg_cache == NULL)
                                {
                                    g_base_info_unref ((GIBaseInfo *) type_info);
                                    g_base_info_unref ((GIBaseInfo *) arg_info);
                                    return FALSE;
                                }

                            if (direction & GGI_DIRECTION_TO_SCM)
                                {
                                    callable_cache->n_to_scm_args++;

                                    callable_cache->to_scm_args = g_slist_append (callable_cache->to_scm_args,
                                                                                  arg_cache);
                                }

                            _ggi_callable_cache_set_arg (callable_cache, arg_index, arg_cache);
                        }

                    g_base_info_unref (type_info);
                }

            // ensure arguments always have name when available

            arg_cache->arg_name = g_base_info_get_name ((GIBaseInfo *) arg_info);

            g_base_info_unref ((GIBaseInfo *) arg_info);
        }

    if (callable_cache->arg_name_hash == NULL)
        callable_cache->arg_name_hash = g_hash_table_new (g_str_hash, g_str_equal);
    else
        g_hash_table_remove_all (callable_cache->arg_name_hash);

    callable_cache->n_scm_required_args = 0;
    callable_cache->user_data_varargs_index = -1;

    last_explicit_arg_index = -1;

    //reverse loop through all the arguments to setup arg_name_list/hash
    //and find the number of required arguments
    for (i = (_ggi_callable_cache_args_len (callable_cache)) - 1; i >= 0; i--)
        {
            GGIArgCache *arg_cache = _ggi_callable_cache_get_arg (callable_cache, i);

            if (arg_cache->meta_type != GGI_META_ARG_TYPE_CHILD &&
                arg_cache->meta_type != GGI_META_ARG_TYPE_CLOSURE &&
                arg_cache->direction & GGI_DIRECTION_FROM_SCM)
                {
                    gpointer arg_name = (gpointer) arg_cache->arg_name;
                    callable_cache->arg_name_list = g_slist_prepend (callable_cache->arg_name_list,
                                                                     arg_name);

                    if (arg_name != NULL)
                        g_hash_table_insert (callable_cache->arg_name_hash,
                                             arg_name,
                                             GINT_TO_POINTER(i));

                    if (callable_cache->n_scm_required_args > 0)
                        {
                            arg_cache->has_default = FALSE;
                            callable_cache->n_scm_required_args += 1;
                        }
                    else if (!arg_cache->has_default)
                        callable_cache->n_scm_required_args += 1;

                    if (last_explicit_arg_index == -1)
                        {
                            last_explicit_arg_index = i;

                            if (arg_cache->meta_type == GGI_META_ARG_TYPE_CHILD_WITH_SCMARG)
                                callable_cache->user_data_varargs_index = i;
                        }
                }
        }

    if (!return_cache->is_skipped && return_cache->type_tag != GI_TYPE_TAG_VOID)
        callable_cache->has_return = TRUE;

    return TRUE;
}

static gboolean
_callable_cache_init (GGICallableCache *cache, GICallableInfo *callable_info)
{
    g_debug ("callable_cache_init");

    gint n_args;
    GIBaseInfo *container;

    // TODO: deinit

    if (cache->generate_args_cache == NULL)
        cache->generate_args_cache = _callable_cache_generate_args_cache_real;

    cache->name = g_base_info_get_name ((GIBaseInfo *) callable_info);
    cache->namespace = g_base_info_get_namespace ((GIBaseInfo *) callable_info);
    container = g_base_info_get_container ((GIBaseInfo *) callable_info);

    if (container != NULL && g_base_info_get_type (container) != GI_INFO_TYPE_TYPE) {
        cache->container_name = g_base_info_get_name (container);
    }
    cache->throws = g_callable_info_can_throw_gerror ((GIBaseInfo *) callable_info);

    // TODO: handle deprecated

    n_args = (gint) cache->args_offset + g_callable_info_get_n_args (callable_info);
    g_debug (" name: %s", cache->name);
    g_debug (" n_args: %d", n_args);

    if (n_args >= 0)
        {
            cache->args_cache = g_ptr_array_new_full (n_args, (GDestroyNotify) ggi_arg_cache_free);
            g_ptr_array_set_size (cache->args_cache, n_args);
        }

    g_debug ( "generate_args_cache");
    if (!cache->generate_args_cache (cache, callable_info)) {
        //        _callable_cache_deinit_real (cache);
        return FALSE;
    }

    g_debug ("callable_cache_init: success");
    return TRUE;
}

static SCM
_function_cache_invoke_real (GGIFunctionCache *function_cache,
                             GGIInvokeState *state,
                             SCM scm_args,
                             SCM scm_kwargs)
{
    return ggi_invoke_c_callable (function_cache, state, scm_args, scm_kwargs);
}

static void
_ggi_function_wrapper (ffi_cif *cif, void *ret, void **args, void *function_cache)
{
    g_debug ("ggi_function_wrapper");

    g_assert (cif != NULL);
    g_assert (ret != NULL);
    g_assert (args != NULL);
    g_assert (function_cache != NULL);

    SCM scm_args, scm_optargs;
    unsigned int n_args = cif->nargs;
    unsigned int n_optargs;
    GGICallableCache *callable_cache;

    callable_cache = (GGICallableCache *) function_cache;

    scm_args = SCM_EOL;
    for (unsigned int i = 0; i < callable_cache->n_scm_required_args; i++)
        {
            SCM scm_arg;

            scm_arg = SCM_PACK (*(scm_t_bits *)(args[i]));

            scm_args = scm_append (scm_list_2 (scm_args,
                                               scm_list_1 (scm_arg)));
        }

    scm_optargs = SCM_EOL;
    n_optargs = callable_cache->n_scm_args - callable_cache->n_scm_required_args;
    for (unsigned int i = 0; i < n_optargs; i++)
        {
            SCM scm_arg;

            scm_arg = SCM_PACK (*(scm_t_bits *)(args[i]));

            scm_optargs = scm_append (scm_list_2 (scm_args,
                                                  scm_list_1 (scm_arg)));
        }


    *(ffi_arg *)ret = SCM_UNPACK (ggi_function_cache_invoke (function_cache,
                                                             scm_args,
                                                             scm_optargs));
}

// TODO: don't need both args
static gboolean
_function_cache_wrapper_init (GGIFunctionCache *function_cache,
                              GGICallableCache *callable_cache)
{
    g_debug ("_funtion_cache_wrapper_init");

    ffi_type **args = NULL;
    ffi_type *ret_type;
    ffi_status prep_ok;

    int n_total_args;
    int n_opt_args;

    function_cache->wrapper_closure = ffi_closure_alloc (sizeof (ffi_closure),
                                                         &(function_cache->wrapper));

    g_return_val_if_fail (function_cache->wrapper_closure != NULL, FALSE);
    g_return_val_if_fail (function_cache->wrapper != NULL, FALSE);

    n_opt_args = callable_cache->n_scm_args - callable_cache->n_scm_required_args;
    n_total_args = callable_cache->n_scm_required_args + n_opt_args;

    if (n_total_args > 0)
        args = g_new0 (ffi_type *, n_total_args);

    for (int i = 0; i < n_total_args; i++)
        {
            args[i] = &ffi_type_pointer;
        }

    ret_type = &ffi_type_pointer;

    prep_ok = ffi_prep_cif (&(function_cache->wrapper_cif),
                            FFI_DEFAULT_ABI,
                            n_total_args,
                            ret_type,
                            args);

    if (prep_ok != FFI_OK)
        {
            scm_misc_error ("ggi_function_create_gsubr",
                            "failed to created closure",
                            NULL);
            return FALSE;
        }

    prep_ok = ffi_prep_closure_loc (function_cache->wrapper_closure,
                                   &(function_cache->wrapper_cif),
                                   _ggi_function_wrapper,
                                   function_cache,
                                   function_cache->wrapper);

    if (prep_ok != FFI_OK)
        {
            scm_misc_error ("ggi_function_create_gsubr",
                            "failed to created closure",
                            NULL);
            return FALSE;
        }

    return TRUE;
}


static gboolean
_function_cache_init (GGIFunctionCache *function_cache,
                      GICallableInfo *callable_info)
{
    g_debug ("ggi_function_cache_init");

    GGICallableCache *callable_cache = (GGICallableCache *) function_cache;
    GIFunctionInvoker *invoker = &function_cache->invoker;
    GError *error = NULL;

    callable_cache->calling_context = GGI_CALLING_CONTEXT_IS_FROM_SCM;

    // TODO: deinit

    if (function_cache->invoke == NULL)
        function_cache->invoke = _function_cache_invoke_real;

    if (!_callable_cache_init (callable_cache, callable_info))
        return FALSE;

    if (!_function_cache_wrapper_init (function_cache, callable_cache))
        return FALSE;

    if (invoker->native_address == NULL)
        {
            if (g_function_info_prep_invoker ((GIFunctionInfo *) callable_info,
                                              invoker,
                                              &error))
                return TRUE;
        }
    else
        {
            if (g_function_invoker_new_for_address (invoker->native_address,
                                                    (GIFunctionInfo *) callable_info,
                                                    invoker,
                                                    &error))
                return TRUE;
        }

    // TODO: deinit real

    return FALSE;
}


GGIArgCache *
ggi_arg_interface_new_from_info (GITypeInfo       *type_info,
                                 GIArgInfo       *arg_info,    /* may be NULL for return arguments */
                                 GITransfer       transfer,
                                 GGIDirection     direction,
                                 GIInterfaceInfo *iface_info)
{
    GGIInterfaceCache *ic;

    ic = g_slice_new0 (GGIInterfaceCache);
    if (!ggi_arg_interface_setup (ic,
                                  type_info,
                                  arg_info,
                                  transfer,
                                  direction,
                                  iface_info)) {
        ggi_arg_cache_free ((GGIArgCache *)ic);
        return NULL;
    }

    return (GGIArgCache *) ic;

}

GGIFunctionCache *
ggi_function_cache_new (GICallableInfo *info)
{
    g_debug ("ggi_function_cache_new");

    GGIFunctionCache *function_cache;

    function_cache = g_new0 (GGIFunctionCache, 1);

    if (!_function_cache_init (function_cache, info))
        {
            g_free (function_cache);
            return NULL;
        }

    return function_cache;
}

SCM
ggi_function_cache_invoke (GGIFunctionCache *function_cache,
                           SCM               scm_args,
                           SCM               scm_kwargs)
{
    g_debug ("ggi_function_cache_invoke");

    GGIInvokeState state = { 0, };

    return function_cache->invoke (function_cache, &state, scm_args, scm_kwargs);
}

// GGICCallbackCache

GGIFunctionCache *
ggi_ccallback_cache_new (GICallableInfo *info,
                         GCallback       function_ptr)
{
    GGICCallbackCache *ccallback_cache;
    GGIFunctionCache *function_cache;

    ccallback_cache = g_new0 (GGICCallbackCache, 1);
    function_cache = (GGIFunctionCache *) ccallback_cache;

    function_cache->invoker.native_address = function_ptr;

    if (!_function_cache_init (function_cache, info))
        {
            g_free (ccallback_cache);
            return NULL;
        }

    return function_cache;
}

SCM
ggi_ccallback_cache_invoke (GGICCallbackCache *ccallback_cache,
                            SCM                scm_args,
                            SCM                scm_optargs,
                            gpointer           user_data)
{
    GGIFunctionCache *function_cache = (GGIFunctionCache *) ccallback_cache;
    GGIInvokeState state = { 0, };

    state.user_data = user_data;

    return function_cache->invoke (function_cache,
                                   &state,
                                   scm_args,
                                   scm_optargs);
}

void
ggi_cache_init (void)
{
    g_debug ("ggi_cache_init");

    scm_ggi_function_cache_t = scm_make_foreign_object_type (scm_from_utf8_symbol ("<ggi-function-cache>"),
                                                             scm_list_1 (scm_from_utf8_symbol ("ptr")),
                                                             NULL);
}
