/* -*- Mode: C; c-basic-offset: 4 -*-
 */
#ifndef __GGI_MASHAL_CLEANUP_H__
#define __GGI_MASHAL_CLEANUP_H__

#include "ggi-struct.h"
#include "ggi-invoke-state-struct.h"
#include "ggi-cache.h"

G_BEGIN_DECLS

void
ggi_marshal_cleanup_args_from_scm_marshal_success (GGIInvokeState   *state,
                                                   GGICallableCache *cache);
void
ggi_marshal_cleanup_args_from_scm_parameter_fail (GGIInvokeState   *state,
                                                  GGICallableCache *cache,
                                                  gssize            failed_arg_index);
void
ggi_marshal_cleanup_args_to_scm_marshal_success (GGIInvokeState   *state,
                                                 GGICallableCache *cache);
void
ggi_marshal_cleanup_args_return_fail (GGIInvokeState   *state,
                                      GGICallableCache *cache);
void
ggi_marshal_cleanup_args_to_scm_parameter_fail (GGIInvokeState   *state,
                                                GGICallableCache *cache,
                                                gssize            failed_to_scm_arg_index);

G_END_DECLS
#endif
