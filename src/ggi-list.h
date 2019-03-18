/* -*- Mode: C; c-basic-offset: 4 -*-
 * vim: tabstop=4 shiftwidth=4 expandtab
 */

#ifndef __GGI_LIST_H__
#define __GGI_LIST_H__

#include <girepository.h>
#include "ggi-cache.h"

G_BEGIN_DECLS

GGIArgCache *ggi_arg_garray_new_from_info  (GITypeInfo        *type_info,
                                            GIArgInfo         *arg_info,   /* may be null */
                                            GITransfer         transfer,
                                            GGIDirection      direction,
                                            GGICallableCache *callable_cache);

GGIArgCache *ggi_arg_garray_len_arg_setup  (GGIArgCache      *arg_cache,
                                            GITypeInfo        *type_info,
                                            GGICallableCache *callable_cache,
                                            GGIDirection      direction,
                                            gssize             arg_index,
                                            gssize            *py_arg_index);

G_END_DECLS

#endif
