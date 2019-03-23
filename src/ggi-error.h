/* -*- Mode: C; c-basic-offset: 4 -*-
 * vim: tabstop=4 shiftwidth=4 expandtab
 */

#ifndef __GGI_ERROR_H__
#define __GGI_ERROR_H__

#include <girepository.h>
#include "ggi-cache.h"

G_BEGIN_DECLS

gboolean
ggi_error_check (GError **error);

SCM
ggi_error_marshal_to_scm (GError **error);

gboolean
ggi_error_marshal_from_scm (SCM      scm_error,
                            GError **error);

gboolean
ggi_error_exception_check (GError **error);

GGIArgCache *
ggi_arg_gerror_new_from_info (GITypeInfo  *type_info,
                              GIArgInfo   *arg_info,
                              GITransfer   transfer,
                              GGIDirection direction);


G_END_DECLS

#endif
