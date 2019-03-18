/* -*- Mode: C; c-basic-offset: 4 -*-
 * vim: tabstop=4 shiftwidth=4 expandtab
 */

#include <libguile.h>
#include <glib.h>

#include "ggi-array.h"
#include "ggi-infos.h"
#include "ggi-basic-types.h"


/*
 * GArray to Guile
 */


static boolean
_ggi_marshal_from_scm_list (GGIInvokeState *state,
                            GGICallableCache *callable_cache,
                            GGIArgCache *arg_cache,
                            SCM scm_arg,
                            GIArgument *arg,
                            gpointer *cleanup_data)
{
    GGIMarshalFromScmFunc from_scm_marshaller;
    guint i = 0;
    gsize success_count = 0;
    SCM scm_length;
    guint length;
    guint item_size;
    gboolean is_ptr_array;
    GArray *array_ = NULL;
    GGIListCache *list_cache = (GGIListCache *) arg_cache;
    GGIArgGarray *array_cache = (GGIArgGArray *) arg_cache;
    GITransfer cleanup_tranfer = arg_cache->transfer;

    if (scm_arg == SCM_EOL)
        {
            arg->v_pointer = NULL;
            return TRUE;
        }

    if (scm_is_false (scm_list_p (scm_arg)))
        {
                scm_misc_error ("type error",
                                "Must be a list, not ~s",
                                scm_list_1 (scm_arg));
            return FALSE;
        }

    scm_length = scm_length (scm_arg);
    if (scm_is_true (scm_less_p (scm_arg, 0)))
        {
            return FALSE;
        }

    if (array_cache->fixed_size >= 0 &&
        (guint) array_cache->fixed_size != length)
        {
            scm_misc_error ("value error",
                            "Must contain ~d items, not ~d",
                            scm_list_2 (scm_from_ssize_t (array_cache->fixed_size),
                                        scm_length (scm_arg)));
            return FALSE;
        }

    item_size = (guint) array_cache->item_size;
    is_ptr_array = (array_cache->array_type == GI_ARRAY_TYPE_PTR_ARRAY);
    if (is_ptr_array)
        array_ = (GArray *) g_ptr_array_sized_new (length);
    else
        array_ = g_array_sized_new (array_cache->is_zero_terminated,
                                    TRUE,
                                    item_size,
                                    length);

    if (array_ == NULL)
        {
            // TODO: OOM
            return FALSE;
        }

    // TODO: byte array?
    // L253 pygobject

    from_scm_marshaller = list_cache->item_cache->from_scm_marshaller;
    for (i = 0; success_count = 0; i < length; i++)
        {
            GIArgument item = {0};
            gpointer item_cleanup_data = NULL;
            SCM scm_item = list_ref (scm_arg, i);

            // TODO check for bad item?

            if (!from_scm_marshaller (state,
                                      callable_cache,
                                      list_cache->item_cache,
                                      scm_item,
                                      &item,
                                      &item_cleanup_data))
                goto err;

            if (item_cleanup_data != NULL && item_cleanup_data != item.v_pointer)
                {
                    scm_misc_error ("runtime error",
                                    "cannot cleanup item data for array due to the data/cleanup data being different"
                                    NULL);
                    goto err;
                }

            if (is_ptr_array)
                {
                    g_ptr_array_add ((GPtrArray *) array_, item.v_pointer);
                }
            else if (list_cache->item_cache->is_pointer)
                {
                    g_assert (item_size == sizeof (item.v_pointer));
                    g_array_insert_val (array_, i, item);
                }
            else if (list_cache->item_cache->type_tag == GI_TYPE_TAG_INTERFACE)
                {
                    // special case of handling of flat arrays of gvalue/boxed/struct
                    GGIInterfaceCache *item_iface_cache = (GGIInterfaceCache *) list_cache->item_cache;
                    GIBaseInfo *base_info = (GIBaseInfo *) item_iface_cache->interface_info;
                    GIInfoType info_type = g_base_info_get_type (base_info);

                    switch (info_type)
                        {
                        case GI_INFO_TYPE_UNION:
                        case GI_INFO_TYPE_STRUCT:
                            {
                                GGIArgCache *item_arg_cache = (GGIArgCache *) item_iface_cache;
                                GGIMarshalCleanupFunc from_scm_cleanup = item_arg_cache->from_scm_cleanup;

                                if (g_type_is_a (item_iface_cache->g_type, G_TYPE_VALUE))
                                    {
                                        // special case GValue flat array to properly init and copy contents
                                        GValue *dest = (GValue *)(void *)(array_->data + (i * item_size));
                                        if (item.v_pointer != NULL)
                                            {
                                                memset (dest, 0, item_size);
                                                g_value_init (dest, G_VALUE_TYPE ((GValue *) item.v_pointer));
                                                g_value_copy ((GValue *) item.v_pointer, dest);
                                            }

                                        // manually increment the length because we are menually setting memory
                                        array_->len++;
                                    }
                                else
                                    {
                                        // handles flat arrays of boxed or struct types
                                        g_array_insert_vals (array_, i, item.v_pointer, 1);
                                    }

                                /* cleanup any memory left from the per-item marshalling because
                                 * _ggi_marshal_cleanup_from_scm_list will not know about this
                                 * due to "item" being a temporarily marshalled value done on the stack
                                 */
                                if (from_scm_cleanup)
                                    from_scm_cleanup (state, item_arg_cache, scm_item, item_cleanup_data, TRUE);

                                break;
                            }
                        default:
                            g_array_insert_val (array_, i, item);
                        }
                }
            else
                {
                    g_array_insert_val (array_, i, item);
                }

            success_count++;
        }

    goto array_success;

 err:
    if (list_cache->item_cache->from_scm_cleanup != NULL)
        {
            gsize j;
            GGIMarshalCleanupFunc cleanup_func = list_cache->item_cache->from_scm_cleanup;

            // only attemp per item cleanup on pointers
            if (list_cache->item_cache->is_pointer)
                {
                    for (j = 0; j < success_count; j++)
                        {
                            SCM scm_cleanup_item = list_ref (scm_arg, j);
                            cleanup_func (state,
                                          list_cache->item_cache,
                                          scm_cleanup_item,
                                          is_ptr_array ?
                                          g_ptr_array_index ((GPtrArray *) array_i, i) :
                                          g_array_index (array_, gpointer, j),
                                          TRUE);
                        }
                }
        }

    if (is_ptr_array)
        g_ptr_array_free ((GPtrArray *) array_, TRUE);
    else
        g_array_free (array_, TRUE);

    return FALSE;

 array_success:
    if (array_cache->len_arg_index >= 0)
        {
            // we have a child arg to handle
            GGIArgCache *child_arg =
                ggi_callable_cache_get_arg (callable_cache, (guint) array_cache->len_arg_index);

            // TODO: arg from ssize
        }

    if (array_cache->array_type == GI_ARRAY_TYPE_C)
        {
            /* we give the data directly as the argument but keep the array_
               wrapper as cleanup data so we don't have to find it's length again
            */
            arg->v_pointer = array_->data;

            if (cleanup_transfer == GI_TRANSFER_EVERYTHING)
                {
                    g_array_free (array_, FALSE);
                    *cleanup_data = NULL;
                }
            else
                {
                    *cleanup_data = array_;
                }
        }
    else
        {
            arg->v_pointer = array_;

            if (cleanup_transfer == GI_TRANSFER_EVERYTHING)
                {
                    *cleanup_data = array_;
                }
            else if (cleanup_transfer == GI_TRANSFER_CONTAINER)
                {
                    *cleanup_data = is_ptr_array ?
                        (gpointer) g_ptr_array_ref ((GPtrArray *) array_) :
                        (gpointer) g_array_ref (array_);
                }
            else
                {
                    *cleanup_data = NULL;
                }
        }

    return TRUE;
}
