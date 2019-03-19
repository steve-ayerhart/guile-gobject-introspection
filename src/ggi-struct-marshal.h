/* -*- Mode: C; c-basic-offset: 4 -*-
 * vim: tabstop=4 shiftwidth=4 expandtab
 */

#ifndef __GGI_STRUCT_MARHSAL_H__
#define __GGI_STRUCT_MARSHAL_H__

#include <girepository.h>
#include "ggi-cache.h"

G_BEGIN_DECLS

GGIArgCache *
ggi_arg_struct_new_from_info (GITypeInfo      *type_info,
                              GIArgInfo       *arg_info,
                              GITransfer       transfer,
                              GGIDirection     direction,
                              GIInterfaceInfo *iface_info);

gboolean
ggi_arg_gvalue_from_scm_marshal (SCM         scm_arg,
                                 GIArgument *arg,
                                 GITransfer  transfer,
                                 gboolean    is_allocated);

gboolean
ggi_arg_struct_from_scm_marshal (SCM          scm_arg,
                                 GIArgument  *arg,
                                 const gchar *arg_name,
                                 GIBaseInfo  *interface_info,
                                 GType        gtype,
                                 SCM          scm_type, // probably not needed
                                 GITransfer   transfer,
                                 gboolean     is_allocated,
                                 gboolean     is_foreign,
                                 gboolean     is_pointer);

SCM
ggi_arg_struct_to_scm_marshal (GIArgument *arg,
                               GIInterfaceInfo *interface_info,
                               GType gtype,
                               SCM scm_type, // again
                               GITransfer transfer,
                               gboolean is_allocated,
                               gboolean is_foreign);

G_END_DECLS

#endif
