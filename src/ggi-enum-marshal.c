/* -*- Mode: C; c-basic-offset: 4 -*-
 * vim: tabstop=4 shiftwidth=4 expandtab
 */

#include <libguile.h>
#include <glib.h>

#include "ggi-enum-marshal.h"
#include "gvalue.h"

static gboolean
gi_argument_from_c_long (GIArgument *arg_out,
                         long        c_long_in,
                         GITypeTag   type_tag)
{
    switch (type_tag)
        {
        case GI_TYPE_TAG_INT8:
            arg_out->v_int8 = (gint8) c_long_in;
            return TRUE;
        case GI_TYPE_TAG_UINT8:
            arg_out->v_uint8 = (guint8) c_long_in;
            return TRUE;
        case GI_TYPE_TAG_INT16:
            arg_out->v_int16 = (gint16) c_long_in;
            return TRUE;
        case GI_TYPE_TAG_UINT16:
            arg_out->v_uint16 = (guint16) c_long_in;
            return TRUE;
        case GI_TYPE_TAG_INT32:
            arg_out->v_int32 = (gint32) c_long_in;
            return TRUE;
        case GI_TYPE_TAG_UINT32:
            arg_out->v_uint32 = (guint32) c_long_in;
            return TRUE;
        case GI_TYPE_TAG_INT64:
            arg_out->v_int64 = (gint64) c_long_in;
            return TRUE;
        case GI_TYPE_TAG_UINT64:
            arg_out->v_uint64 = (guint64) c_long_in;
            return TRUE;
        default:
            scm_misc_error ("type error",
                            "Unable to marshal c long ~d to ~s",
                            scm_list_2 (scm_from_long (c_long_in),
                                        scm_from_locale_string (g_type_tag_to_string (type_tag))));
            return FALSE;

        }
}

static gboolean
gi_argument_to_c_long (GIArgument *arg_in,
                       long       *c_long_out,
                       GITypeTag   type_tag)
{
    switch (type_tag)
        {
        case GI_TYPE_TAG_INT8:
            *c_long_out = arg_in->v_int8;
            return TRUE;
        case GI_TYPE_TAG_UINT8:
            *c_long_out = arg_in->v_uint8;
            return TRUE;
        case GI_TYPE_TAG_INT16:
            *c_long_out = arg_in->v_int16;
            return TRUE;
        case GI_TYPE_TAG_UINT16:
            *c_long_out = arg_in->v_uint16;
            return TRUE;
        case GI_TYPE_TAG_INT32:
            *c_long_out = arg_in->v_int32;
            return TRUE;
        case GI_TYPE_TAG_UINT32:
            *c_long_out = arg_in->v_uint32;
            return TRUE;
        case GI_TYPE_TAG_INT64:
            if (arg_in->v_int64 > G_MAXLONG || arg_in->v_int64 < G_MINLONG) {
                scm_misc_error ("type error",
                                "Unable to marshal ~s to C long",
                                scm_from_locale_string (g_type_tag_to_string (type_tag)));
                return FALSE;
            }
            *c_long_out = (glong)arg_in->v_int64;
            return TRUE;
        case GI_TYPE_TAG_UINT64:
            if (arg_in->v_uint64 > G_MAXLONG) {
                scm_misc_error ("type error",
                                "Unable to marshal ~s to C long",
                                scm_from_locale_string (g_type_tag_to_string (type_tag)));
            }
            *c_long_out = (glong)arg_in->v_uint64;
            return TRUE;
        default:
            scm_misc_error ("type error",
                            "Unable to marshal ~s to C long",
                            scm_from_locale_string (g_type_tag_to_string (type_tag)));
            return FALSE;
        }
}

static gboolean
_ggi_marshal_from_scm_interface_enum (GGIInvokeState *state,
                                      GGICallableCache *callable_cache,
                                      GGIArgCache *arg_cache,
                                      SCM          scm_arg,
                                      GIArgument *arg,
                                      gpointer *cleanup_data)
{
    gint enum_value;
    GGIInterfaceCache *iface_cache = (GGIInterfaceCache *) arg_cache;
    GIBaseInfo *interface = NULL;

    // TODO: check to make sure it's a goops enum

    enum_value = g_value_get_enum (scm_c_gvalue_peek_value (scm_arg));

    interface = g_type_info_get_interface (arg_cache->type_info);
    g_assert (g_base_info_get_type (interface) == GI_INFO_TYPE_ENUM);

    if (!gi_argument_from_c_long (arg,
                                  (long) enum_value,
                                  g_enum_info_get_storage_type ((GIEnumInfo *) interface)))
        {
            g_assert_not_reached ();
            g_base_info_unref (interface);
            return FALSE;
        }

    // TODO: if it's not an goops enum we need to make sure the value is in


    g_base_info_unref (interface);
    return TRUE;
}

static gboolean
_ggi_marshal_from_scm_interface_flags (GGIInvokeState *state,
                                       GGICallableCache *callable_cache,
                                       GGIArgCache *arg_cache,
                                       SCM scm_arg,
                                       GIArgument *arg,
                                       gpointer *cleanup_data)
{
    return TRUE;
}

static SCM
_ggi_marshal_to_scm_interface_enum (GGIInvokeState *state,
                                    GGICallableCache *callable_cache,
                                    GGIArgCache *arg_cache,
                                    GIArgument *arg,
                                    gpointer *cleanup_data)
{
    SCM scm_obj = SCM_UNSPECIFIED;

    return scm_obj;
}

static SCM
_ggi_marshal_to_scm_interface_flags (GGIInvokeState *state,
                                    GGICallableCache *callable_cache,
                                    GGIArgCache *arg_cache,
                                    GIArgument *arg,
                                    gpointer *cleanup_data)
{
    SCM scm_obj = SCM_UNSPECIFIED;

    return scm_obj;
}

static gboolean
ggi_arg_enum_setup_from_info (GGIArgCache *arg_cache,
                              GITypeInfo *type_info,
                              GIArgInfo *arg_info,
                              GITransfer transfer,
                              GGIDirection direction)
{
    if (direction & GGI_DIRECTION_FROM_SCM)
        arg_cache->from_scm_marshaller = _ggi_marshal_from_scm_interface_enum;

    if (direction & GGI_DIRECTION_TO_SCM)
        arg_cache->to_scm_marshaller = _ggi_marshal_to_scm_interface_enum;

    return TRUE;
}

GGIArgCache *
ggi_arg_enum_new_from_info (GITypeInfo *type_info,
                            GIArgInfo *arg_info,
                            GITransfer transfer,
                            GGIDirection direction,
                            GIInterfaceInfo *iface_info)
{
    g_debug ("ggi_arg_enum_new_from_info");

    gboolean res = FALSE;
    GGIArgCache *cache = NULL;

    cache = ggi_arg_interface_new_from_info (type_info,
                                             arg_info,
                                             transfer,
                                             direction,
                                             iface_info);

    if (cache == NULL)
        return NULL;

    res = ggi_arg_enum_setup_from_info (cache,
                                        type_info,
                                        arg_info,
                                        transfer,
                                        direction);

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

static gboolean
ggi_arg_flags_setup_from_info (GGIArgCache *arg_cache,
                               GITypeInfo *type_info,
                               GIArgInfo *arg_info,
                               GITransfer transfer,
                               GGIDirection direction)
{
    if (direction & GGI_DIRECTION_FROM_SCM)
        arg_cache->from_scm_marshaller = _ggi_marshal_from_scm_interface_flags;

    if (direction & GGI_DIRECTION_TO_SCM)
        arg_cache->to_scm_marshaller = _ggi_marshal_to_scm_interface_flags;

    return TRUE;
}

GGIArgCache *
ggi_arg_flags_new_from_info (GITypeInfo *type_info,
                             GIArgInfo *arg_info,
                             GITransfer transfer,
                             GGIDirection direction,
                             GIInterfaceInfo *iface_info)
{
    g_debug ("ggi_arg_flags_new_from_info");

    gboolean res = FALSE;
    GGIArgCache *cache = NULL;

    cache = ggi_arg_interface_new_from_info (type_info,
                                             arg_info,
                                             transfer,
                                             direction,
                                             iface_info);

    if (cache == NULL)
        return NULL;

    res = ggi_arg_flags_setup_from_info (cache,
                                        type_info,
                                        arg_info,
                                        transfer,
                                        direction);

    if (res)
        {
            return cache;
        }
    else
        {
            g_debug ("NULL");
            ggi_arg_cache_free (cache);
            return NULL;
        }
}
