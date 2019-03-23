/* -*- Mode: C; c-basic-offset: 4 -*-
 * vim: tabstop=4 shiftwidth=4 expandtab
 */

#include <libguile.h>
#include <glib.h>

#include "ggi-hashtable.h"
#include "ggi-argument.h"

typedef struct _GGIHashCache
{
  GGIArgCache arg_cache;
  GGIArgCache *key_cache;
  GGIArgCache *value_cache;
} GGIHashCache;

static void
_hash_cache_free_func (GGIHashCache *cache)
{
  if (cache != NULL)
    {
      ggi_arg_cache_free (cache->key_cache);
      ggi_arg_cache_free (cache->value_cache);
      g_slice_free (GGIHashCache, cache);
    }
}

static gboolean
_ggi_marshal_from_scm_ghash (GGIInvokeState *state,
                             GGICallableCache *callable_cache,
                             GGIArgCache *arg_cache,
                             SCM scm_arg,
                             GIArgument *arg,
                             gpointer *cleanup_data)
{
  GGIMarshalFromScmFunc key_from_scm_marshaller;
  GGIMarshalFromScmFunc value_from_scm_marshaller;

  return TRUE;
}

static void
_ggi_marshal_cleanup_from_scm_ghash (GGIInvokeState *state,
                                     GGIArgCache *arg_cache,
                                     SCM scm_arg,
                                     gpointer data,
                                     gboolean was_processed)
{
}

static SCM
_ggi_marshal_to_scm_ghash (GGIInvokeState *state,
                           GGICallableCache *callabel_cache,
                           GGIArgCache *arg_cache,
                           GIArgument *arg,
                           gpointer *cleanup_data)
{
  SCM scm_hash;

  return scm_hash;
}

static void
_ggi_marshal_cleanup_to_scm_ghash (GGIInvokeState *state,
                                   GGIArgCache *arg_cache,
                                   gpointer cleanup_data,
                                   gpointer data,
                                   gboolean was_processed)
{
}

static void
_arg_cache_from_scm_ghash_setup (GGIArgCache *arg_cache)
{
    arg_cache->from_scm_marshaller = _ggi_marshal_from_scm_ghash;
    arg_cache->from_scm_cleanup = _ggi_marshal_cleanup_from_scm_ghash;
}

static void
_arg_cache_to_scm_ghash_setup (GGIArgCache *arg_cache)
{
    arg_cache->to_scm_marshaller = _ggi_marshal_to_scm_ghash;
    arg_cache->to_scm_cleanup = _ggi_marshal_cleanup_to_scm_ghash;
}

static gboolean
ggi_arg_hash_table_setup_from_info (GGIHashCache *hc,
                                    GITypeInfo *type_info,
                                    GIArgInfo *arg_info,
                                    GITransfer transfer,
                                    GGIDirection direction,
                                    GGICallableCache *callable_cache)
{
    GITypeInfo *key_type_info;
    GITypeInfo *value_type_info;
    GITransfer item_transfer;

    if (!ggi_arg_base_setup ((GGIArgCache *) hc,
                             type_info,
                             arg_info,
                             transfer,
                             direction))
        return FALSE;

    ((GGIArgCache *) hc)->destroy_notify = (GDestroyNotify) _hash_cache_free_func;
    key_type_info = g_type_info_get_param_type (type_info, 0);
    value_type_info = g_type_info_get_param_type (type_info, 1);

    item_transfer =
        transfer == GI_TRANSFER_CONTAINER ?
        GI_TRANSFER_NOTHING :
        transfer;

    hc->key_cache = ggi_arg_cache_new (key_type_info,
                                       NULL,
                                       item_transfer,
                                       direction,
                                       callable_cache,
                                       0,
                                       0);

    if (hc->key_cache == NULL)
        return FALSE;

    hc->value_cache = ggi_arg_cache_new (value_type_info,
                                         NULL,
                                         item_transfer,
                                         direction,
                                         callable_cache,
                                         0,
                                         0);

    if (hc->value_cache == NULL)
        return FALSE;

    g_base_info_unref ((GIBaseInfo *) key_type_info);
    g_base_info_unref ((GIBaseInfo *) value_type_info);

    if (direction & GGI_DIRECTION_FROM_SCM)
        _arg_cache_from_scm_ghash_setup ((GGIArgCache *) hc);

    if (direction & GGI_DIRECTION_TO_SCM)
        _arg_cache_to_scm_ghash_setup ((GGIArgCache *) hc);

    return TRUE;
}

GGIArgCache *
ggi_arg_hash_table_new_from_info (GITypeInfo *type_info,
                                  GIArgInfo *arg_info,
                                  GITransfer transfer,
                                  GGIDirection direction,
                                  GGICallableCache *callable_cache)
{
    g_debug ("ggi_arg_hash_table_new_from_info");

    gboolean res = FALSE;
    GGIHashCache *hc = NULL;

    hc = g_slice_new0 (GGIHashCache);
    if (hc == NULL)
        return NULL;

    res = ggi_arg_hash_table_setup_from_info (hc,
                                              type_info,
                                              arg_info,
                                              transfer,
                                              direction,
                                              callable_cache);

    if (res)
        {
            return (GGIArgCache *) hc;
        }
    else
        {
            ggi_arg_cache_free ((GGIArgCache *) hc);
            return NULL;
        }
}
