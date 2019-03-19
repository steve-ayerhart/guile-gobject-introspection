/* -*- Mode: C; c-basic-offset: 4 -*-
 * vim: tabstop=4 shiftwidth=4 expandtab
 */

#ifndef __GGI_ENUM_MARHSAL_H__
#define __GGI_ENUM_MARSHAL_H__

#include <girepository.h>
#include "ggi-cache.h"

GGIArgCache *
ggi_arg_enum_new_from_info (GITypeInfo      *type_info,
                            GIArgInfo       *arg_info,
                            GITransfer       transfer,
                            GGIDirection     direction,
                            GIInterfaceInfo *iface_info);

GGIArgCache *
ggi_arg_flags_new_from_info (GITypeInfo      *type_info,
                             GIArgInfo       *arg_info,
                             GITransfer       transfer,
                             GGIDirection     direction,
                             GIInterfaceInfo *iface_info);

G_BEGIN_DECLS


#endif
