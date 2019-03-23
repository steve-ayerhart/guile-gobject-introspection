/* -*- Mode: C; c-basic-offset: 4 -*-
 * vim: tabstop=4 shiftwidth=4 expandtab
 */

#ifndef __GGI_CCLOSURE_H__
#define __GGI_CCLOSURE_H__

#include <libguile.h>
#include "ggi-cache.h"

G_BEGIN_DECLS

typedef struct {
  GCallback callback;
  GIFunctionInfo *info;
  gpointer user_data;
  GIScopeType scope;
  GDestroyNotify destroy_notify_func;
  GGICallableCache *cache;
} GGICCallback;

extern SCM GGICallback_Type;

SCM
_ggi_callback_new (GCallback callback,
                   gpointer user_data,
                   GIScopeType scope,
                   GIFunctionInfo *info,
                   GDestroyNotify destroy_notify);

int
ggi_callback_register_types (SCM m);

G_END_DECLS

#endif
