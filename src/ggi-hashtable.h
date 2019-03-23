/* -*- Mode: C; c-basic-offset: 4 -*-
 * vim: tabstop=4 shiftwidth=4 expandtab
 */

#ifndef __GGI_HASHTABLE_H__
#define __GGI_HASHTABLE_H__

#include <girepository.h>
#include "ggi-cache.h"

G_BEGIN_DECLS

GGIArgCache *
ggi_arg_hash_table_new_from_info (GITypeInfo *type_info,
                                  GIArgInfo *arg_info,
                                  GITransfer transfer,
                                  GGIDirection direction,
                                  GGICallableCache *callable_cache);

G_END_DECLS

#endif
