/* -*- Mode: C; c-basic-offset: 4 -*-
 * vim: tabstop=4 shiftwidth=4 expandtab
 */

#include <libguile.h>
#include "ggi-list.h"
#include "ggi-argument.h"

typedef GGISequenceCache GGIArgGList;

// GList and GSList from Guile

static gboolean
_ggi_marshal_from_scm_glist (GGIInvokeState   *state,
                             GGICallableCache *callable_cache,
                             GGIArgCache      *arg_cache,
                             SCM               scm_arg,
                             GIArgument       *arg,
                             gpointer         *cleanup_data)
{
  GGIMarshalFromScmFunc from_scm_marshaller;
  ssize_t length;
  GList *list_ = NULL;
  GGISequenceCache *sequence_cache = (GGISequenceCache *) arg_cache;

  if (SCM_UNBNDP (scm_arg))
    {
      arg->v_pointer = NULL;
      return TRUE;
    }

  // TODO: check is a list

  length = scm_to_ssize_t (scm_length (scm_arg));
  if (length < 0)
    return FALSE;

  from_scm_marshaller = sequence_cache->item_cache->from_scm_marshaller;
  for (int i = 0; i < length; i++)
    {
      GIArgument item = {0};
      gpointer item_cleanup_data = NULL;
      SCM scm_item = scm_list_ref (scm_arg, scm_from_int (i));

      if (!from_scm_marshaller ( state,
                                 callable_cache,
                                 sequence_cache->item_cache,
                                 scm_item,
                                 &item,
                                 &item_cleanup_data))
        goto err;

      list_ = g_list_prepend (list_, _ggi_arg_to_hash_pointer (&item, sequence_cache->item_cache->type_info));
      continue;

    err:

      g_list_free (list_);
      return FALSE;
    }

  arg->v_pointer = g_list_reverse (list_);

  if (arg_cache->transfer == GI_TRANSFER_NOTHING)
    {
      *cleanup_data = arg->v_pointer;
    }
  else if (arg_cache->transfer == GI_TRANSFER_CONTAINER)
    {
      *cleanup_data = g_list_copy (arg->v_pointer);
    }
  else
    {
      *cleanup_data = NULL;
    }

  return TRUE;
}

static gboolean
_ggi_marshal_from_scm_gslist (GGIInvokeState   *state,
                             GGICallableCache *callable_cache,
                             GGIArgCache      *arg_cache,
                             SCM               scm_arg,
                             GIArgument       *arg,
                             gpointer         *cleanup_Data)
{
  GGIMarshalFromScmFunc from_scm_marshaller;
  int i;
  SCM scm_length_;
  GSList *list_ = NULL;
  GGISequenceCache *sequence_cache = (GGISequenceCache *) arg_cache;

  if (SCM_UNBNDP (scm_arg))
    {
      arg->v_pointer = NULL;
      return TRUE;
    }

  // TODO check is a list?

  scm_length_ = scm_length (scm_arg);

  from_scm_marshaller = sequence_cache->item_cache->from_scm_marshaller;

  // TODO FINISH;
}

static void
_ggi_marshal_cleanup_from_scm_glist (GGIInvokeState *state,
                                     GGIArgCache     *arg_cache,
                                     SCM             scm_arg,
                                     gpointer        data,
                                     gboolean        was_processed)
{
  // TODO
}


static SCM
_ggi_marshal_to_scm_glist (GGIInvokeState    *state,
                           GGICallableCache *callable_cache,
                           GGIArgCache      *arg_cache,
                           GIArgument       *arg,
                           gpointer         *cleanup_data)
{
  // TODO
}

static SCM
_ggi_marshal_to_scm_gslist (GGIInvokeState  *state,
                            GGICallableCache *callable_cache,
                            GGIArgCache      *arg_cache,
                            GIArgument       *arg,
                            gpointer         *cleanup_data)
{
  // TODO;
}

static void
_ggi_marshal_cleanup_to_scm_glist (GGIInvokeState *state,
                                      GGIArgCache     *arg_cache,
                                      SCM             scm_arg,
                                      gpointer        data,
                                      gboolean        was_processed)
{
  // TODO
}

static void
_arg_cache_from_scm_glist_setup (GGIArgCache *arg_cache,
                                 GITransfer   transfer)
{
  arg_cache->from_scm_marshaller = _ggi_marshal_from_scm_glist;
  //  arg_cache->from_scm_cleanup = _ggi_marshal_cleanup_from_scm_glist;
}

static void
_arg_cache_to_scm_glist_setup (GGIArgCache *arg_cache,
                               GITransfer   transfer)
{
  arg_cache->to_scm_marshaller = _ggi_marshal_to_scm_glist;
  //arg_cache->to_scm_cleanup = _ggi_marshal_cleanup_to_scm_glist;
}

static void
_arg_cache_from_scm_gslist_setup (GGIArgCache *arg_cache,
                                 GITransfer   transfer)
{
  arg_cache->from_scm_marshaller = _ggi_marshal_from_scm_gslist;
  //arg_cache->from_scm_cleanup = _ggi_marshal_cleanup_from_scm_glist;
}

static void
_arg_cache_to_scm_gslist_setup (GGIArgCache *arg_cache,
                               GITransfer   transfer)
{
  arg_cache->to_scm_marshaller = _ggi_marshal_to_scm_gslist;
  //arg_cache->to_scm_cleanup = _ggi_marshal_cleanup_to_scm_glist;
}

static gboolean
ggi_arg_glist_setup_from_info (GGIArgCache *arg_cache,
                               GITypeInfo  *type_info,
                               GIArgInfo   *arg_info,
                               GITransfer   transfer,
                               GGIDirection direction,
                               GGICallableCache *callable_cache)
{
  GITypeTag type_tag = g_type_info_get_tag (type_info);

  if (!ggi_arg_sequence_setup ((GGISequenceCache *) arg_cache,
                           type_info,
                           arg_info,
                           transfer,
                           direction,
                           callable_cache))
    return FALSE;

  switch (type_tag)
    {
    case GI_TYPE_TAG_GLIST:
      {
        if (direction & GGI_DIRECTION_FROM_SCM)
          _arg_cache_from_scm_glist_setup (arg_cache, transfer);

        if (direction & GGI_DIRECTION_TO_SCM)
          _arg_cache_to_scm_glist_setup (arg_cache, transfer);

        break;
      }
    case GI_TYPE_TAG_GSLIST:
      {
        if (direction & GGI_DIRECTION_FROM_SCM)
          _arg_cache_from_scm_gslist_setup (arg_cache, transfer);

        if (direction & GGI_DIRECTION_TO_SCM)
          _arg_cache_to_scm_gslist_setup (arg_cache, transfer);

        break;
      }
    default:
      g_assert_not_reached ();
    }

  return TRUE;
}

GGIArgCache *
ggi_arg_glist_new_from_info (GITypeInfo *type_info,
                             GIArgInfo   *arg_info,
                             GITransfer   transfer,
                             GGIDirection direction,
                             GGICallableCache *callable_cache)
{
  gboolean res = FALSE;

  GGIArgCache *arg_cache = (GGIArgCache *) g_slice_new0 (GGIArgGList);
  if (arg_cache == NULL)
    return NULL;

  res = ggi_arg_glist_setup_from_info (arg_cache,
                                       type_info,
                                       arg_info,
                                       transfer,
                                       direction,
                                       callable_cache);

  if (res)
    {
      return arg_cache;
    }
  else
    {
      ggi_arg_cache_free (arg_cache);
      return NULL;
    }
}
