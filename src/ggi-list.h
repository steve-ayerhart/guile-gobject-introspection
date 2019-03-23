/* -*- Mode: C; c-basic-offset: 4 -*-
 * vim: tabstop=4 shiftwidth=4 expandtab
 */

#ifndef __GGI_LIST_H__
#define __GGI_LIST_H__

#include <girepository.h>
#include "ggi-cache.h"

G_BEGIN_DECLS

GGIArgCache *
ggi_arg_glist_new_from_info (GITypeInfo       *type_info,
                             GIArgInfo        *arg_info,
                             GITransfer        transfer,
                             GGIDirection      direction,
                             GGICallableCache *callable_cache);

#define ggi_arg_gslist_new_from_info ggi_arg_glist_new_from_info

G_END_DECLS

#endif
