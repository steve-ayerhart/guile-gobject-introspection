/* -*- Mode: C; c-basic-offset: 4 -*-
 * vim: tabstop=4 shiftwidth=4 expandtab
 */

#include <libguile.h>
#include <girepository.h>

#include "ggi-cache.h"
#include "ggi-infos.h"
#include "ggi-invoke.h"

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
ggi_arg_base_setup (GGIArgCache  *arg_cache,
                    GITypeInfo   *type_info,
                    GIArgInfo    *arg_info,  /* may be NULL for return arguments */
                    GITransfer    transfer,
                    GGIDirection  direction)
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
        arg_cache->is_allow_none = g_arg_info_may_be_null (arg_info);

        if (arg_cache->type_tag == GI_TYPE_TAG_INTERFACE || arg_cache->type_tag == GI_TYPE_TAG_ARRAY)
            arg_cache->is_caller_allocates = g_arg_info_is_caller_allocates (arg_info);
        else
            arg_cache->is_caller_allocates = FALSE;
    }
    return TRUE;
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
    if (!ggi_arg_base_setup ((GGIArgCache *)iface_cache,
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
    iface_cache->type_name = scm_from_utf8_string ("TYPE!!!!");
    //    iface_cache->type_name = _ggi_g_base_info_get_fullname (iface_info);
    iface_cache->g_type = g_registered_type_info_get_g_type ((GIRegisteredTypeInfo *) iface_info);
    iface_cache->scm_type = scm_from_int (1);
    //    iface_cache->scm_type = ggi_type_import_by_gi_info ((GIBaseInfo *) iface_info);

    if (iface_cache->scm_type == NULL) {
        return FALSE;
    }

    return TRUE;
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
