/* -*- Mode: C; c-basic-offset: 4 -*-
 * vim: tabstop=4 shiftwidth=4 expandtabo
 */

#ifndef __GGI_INVOKE_H__
#define __GGI_INVOKE_H__

#include <libguile.h>

#include <girepository.h>

#include "ggi-infos.h"
#include "ggi-invoke-state-struct.h"

G_BEGIN_DECLS

SCM
ggi_invoke_c_callable (GGIFunctionCache *function_cache,
                       GGIInvokeState *state,
                       SCM scm_args,
                       SCM scm_kwargs);

SCM
ggi_callable_info_invoke (GIBaseInfo *info,
                          SCM scm_args,
                          SCM scm_kwargs,
                          GGICallableCache *cache,
                          gpointer user_data);

SCM
_wrap_g_callable_info_invoke (SCM scm_base_info,
                              SCM scm_args,
                              SCM scm_kwargs);

gboolean
_ggi_invoke_arg_state_init (GGIInvokeState *state);

void
_ggi_invoke_arg_state_free (GGIInvokeState *state);


G_END_DECLS
#endif
