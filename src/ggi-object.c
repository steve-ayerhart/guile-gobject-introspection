/* -*- Mode: C; c-basic-offset: 4 -*-
 * vim: tabstop=4 shiftwidth=4 expandtab
 */

#include <libguile.h>
#include <glib.h>
#include <glib-object.h>

#include "ggi-object.h"
#include "gobject.h"

typedef gboolean (*GGIObjectMarshalFromScmFunc) (SCM         scm_arg,
                                                 GIArgument *arg,
                                                 GITransfer  transfer);


static gboolean
_ggi_marshal_from_scm_gobject (SCM         scm_arg,
                               GIArgument *arg,
                               GITransfer  transfer)
#define FUNC_NAME "_ggi_marshal_from_scm_gobject"
{
    g_debug ("_ggi_marshal_from_scm_gobject");

    GObject *gobject;

    if (SCM_UNBNDP (scm_arg))
        {
            arg->v_pointer = NULL;
            return TRUE;
        }

    SCM_VALIDATE_GOBJECT_COPY (1, scm_arg, gobject);

    if (transfer == GI_TRANSFER_EVERYTHING)
        g_object_unref (gobject);

    arg->v_pointer = gobject;

    return TRUE;
}
#undef FUNC_NAME

gboolean
ggi_arg_gobject_out_arg_from_scm (SCM         scm_arg,
                                  GIArgument *arg,
                                  GITransfer  transfer)
{
    g_debug ("ggi_arg_gobject_out_arg_from_scm");

    GObject *gobject;

    if (!_ggi_marshal_from_scm_gobject (scm_arg, arg, transfer))
        return FALSE;

    // vfunc stuff
    //gobject = arg->v_pointer;
    return TRUE;
}

static gboolean
_ggi_marshal_from_scm_interface_object (GGIInvokeState              *state,
                                        GGICallableCache            *callable_cache,
                                        GGIArgCache                 *arg_cache,
                                        SCM                          scm_arg,
                                        GIArgument                  *arg,
                                        gpointer                    *cleanup_data,
                                        GGIObjectMarshalFromScmFunc  func)
#define FUNC_NAME "_ggi_marshal_from_scm_interface_object"
{
    g_debug ("_ggi_marshal_from_scm_interface_object");

    GObject *gobject;
    GGIInterfaceCache *iface_cache = (GGIInterfaceCache *) arg_cache;

    if (SCM_UNBNDP (scm_arg))
        {
            arg->v_pointer = NULL;
            return TRUE;
        }

    SCM_VALIDATE_GOBJECT_COPY (1, scm_arg, gobject);
    if (g_type_is_a (G_OBJECT_TYPE (gobject), iface_cache->g_type))
        {
            gboolean res;
            res = func (scm_arg, arg, arg_cache->transfer);
            *cleanup_data = arg->v_pointer;
            return res;
        }
    else
        {
            return FALSE;
        }
}
#undef FUNC_NAME

SCM
ggi_arg_gobject_to_scm_called_from_c (GIArgument *arg,
                                      GITransfer  transfer)
{
    SCM scm_object;
    // TODO: explin hack 254 pygobject

    if (arg->v_pointer != NULL &&
        transfer == GI_TRANSFER_NOTHING &&
        !G_PARAM_SPEC (arg->v_pointer) &&
        g_object_is_floating (arg->v_pointer))
        {
            g_object_ref (arg->v_pointer);
            scm_object = ggi_arg_gobject_to_scm (arg, GI_TRANSFER_EVERYTHING);
            g_object_force_floating (arg->v_pointer);
        }
    else
        {
            scm_object = ggi_arg_gobject_to_scm (arg, transfer);
        }

    return scm_object;
}

static SCM
_ggi_marshal_to_scm_called_from_c_interface_object_cache_adapter (GGIInvokeState   *state,
                                                                  GGICallableCache *callable_cache,
                                                                  GGIArgCache      *arg_cache,
                                                                  GIArgument       *arg,
                                                                  gpointer         *cleanup_data)
{
    return ggi_arg_gobject_to_scm_called_from_c (arg, arg_cache->transfer);
}

static SCM
_ggi_marshal_to_scm_called_from_scm_interface_object_cache_adapter (GGIInvokeState   *state,
                                                                    GGICallableCache *callable_cache,
                                                                    GGIArgCache      *arg_cache,
                                                                    GIArgument       *arg,
                                                                    gpointer         *cleanup_data)
{
    return ggi_arg_gobject_to_scm (arg, arg_cache->transfer);
}

static void
_ggi_marshal_cleanup_from_scm_interface_object (GGIInvokeState *state,
                                                GGIArgCache    *arg_cache,
                                                SCM             scm_arg,
                                                gpointer        data,
                                                gboolean        was_processed)
{
    if (was_processed && state->failed && data != NULL && arg_cache->transfer != GI_TRANSFER_EVERYTHING)
        g_object_unref (G_OBJECT (data));
}

static gboolean
_ggi_marshal_from_scm_called_from_c_interface_object (GGIInvokeState   *state,
                                                      GGICallableCache *callable_cache,
                                                      GGIArgCache      *arg_cache,
                                                      SCM               scm_arg,
                                                      GIArgument       *arg,
                                                      gpointer         *cleanup_data)
{
    return _ggi_marshal_from_scm_interface_object (state,
                                                   callable_cache,
                                                   arg_cache,
                                                   scm_arg,
                                                   arg,
                                                   cleanup_data,
                                                   ggi_arg_gobject_out_arg_from_scm);
}

static gboolean
_ggi_marshal_from_scm_called_from_scm_interface_object (GGIInvokeState *state,
                                                        GGICallableCache *callable_cache,
                                                        GGIArgCache      *arg_cache,
                                                        SCM               scm_arg,
                                                        GIArgument       *arg,
                                                        gpointer         *cleanup_data)
{
    return _ggi_marshal_from_scm_interface_object (state,
                                                   callable_cache,
                                                   arg_cache,
                                                   scm_arg,
                                                   arg,
                                                   cleanup_data,
                                                   _ggi_marshal_from_scm_gobject);
}

static void
_ggi_marshal_cleanup_to_scm_interface_object (GGIInvokeState *state,
                                              GGIArgCache    *arg_cache,
                                              gpointer        cleanup_data,
                                              gpointer        data,
                                              gboolean        was_processed)
{
    if (!was_processed&& arg_cache->transfer == GI_TRANSFER_EVERYTHING)
        g_object_unref (G_OBJECT (data));
}

static gboolean
ggi_arg_gobject_setup_from_info (GGIArgCache      *arg_cache,
                                 GITypeInfo       *type_info,
                                 GIArgInfo        *arg_info,
                                 GITransfer        transfer,
                                 GGIDirection      direction,
                                 GGICallableCache *callable_cache)
{
    if (direction & GGI_DIRECTION_FROM_SCM)
        {
            if (callable_cache->calling_context == GGI_CALLING_CONTEXT_IS_FROM_C)
                arg_cache->from_scm_marshaller = _ggi_marshal_from_scm_called_from_c_interface_object;
            else
                arg_cache->from_scm_marshaller = _ggi_marshal_from_scm_called_from_scm_interface_object;

            arg_cache->from_scm_cleanup = _ggi_marshal_cleanup_from_scm_interface_object;
        }

    if (direction & GGI_DIRECTION_TO_SCM)
        {
            if (callable_cache->calling_context == GGI_CALLING_CONTEXT_IS_FROM_C)
                arg_cache->to_scm_marshaller = _ggi_marshal_to_scm_called_from_c_interface_object_cache_adapter;
            else
                arg_cache->to_scm_marshaller = _ggi_marshal_to_scm_called_from_scm_interface_object_cache_adapter;

            arg_cache->to_scm_cleanup = _ggi_marshal_cleanup_to_scm_interface_object;
        }

    return TRUE;
}

GGIArgCache *
ggi_arg_gobject_new_from_info (GITypeInfo       *type_info,
                               GIArgInfo        *arg_info,
                               GITransfer        transfer,
                               GGIDirection      direction,
                               GIInterfaceInfo  *iface_info,
                               GGICallableCache *callable_cache)
{
    g_debug ("ggi_arg_gobject_new_from_info");

    gboolean res = FALSE;
    GGIArgCache *cache = NULL;

    cache = ggi_arg_interface_new_from_info (type_info,
                                             arg_info,
                                             transfer,
                                             direction,
                                             iface_info);

    if (cache == NULL)
        {
            return NULL;
        }

    res = ggi_arg_gobject_setup_from_info (cache,
                                           type_info,
                                           arg_info,
                                           transfer,
                                           direction,
                                           callable_cache);

    if (res)
        {
            return cache;
        }
    else
        {
            ggi_arg_cache_free (cache);
            return NULL;
        }
}
