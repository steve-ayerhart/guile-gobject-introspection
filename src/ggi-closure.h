/* -*- Mode: C; c-basic-offset: 4 -*-
 * vim: tabstop=4 shiftwidth=4 expandtab
 */

#ifndef __GGI_CLOSURE_H__
#define __GGI_CLOSURE_H__

#include <libguile.h>
#include <girffi.h>
#include <ffi.h>

#include "ggi-cache.h"

G_BEGIN_DECLS

typedef struct _GGICCLOSURE
{
  GICallableInfo *info;
  SCM function;

  ffi_closure *closure;
  ffi_cif cif;

  GIScopeType scope;

  SCM user_data;

  GGIClosureCache *cache;
} GGICClosure;

void
_ggi_closure_handle (ffi_cif *cif,
                     void    *result,
                     void   **args,
                     void    *userdata);

void
_ggi_invoke_closure_free (gpointer user_data);

GGICClosure *
_ggi_make_native_closure (GICallableInfo  *info,
                          GGIClosureCache *cache,
                          GIScopeType      scope,
                          SCM              function,
                          gpointer         user_data);

GGIArgCache *
ggi_arg_callback_new_from_info (GITypeInfo       *type_info,
                                GIArgInfo        *arg_info,
                                GITransfer        transfer,
                                GGIDirection      direction,
                                GIInterfaceInfo  *iface_info,
                                GGICallableCache *callable_cache);

G_END_DECLS

#endif
