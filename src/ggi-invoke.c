/* -*- Mode: C; c-basic-offset: 4 -*-
 */

#include "ggi-invoke.h"

extern SCM _GGIDefaultArgPlaceholder;

static

/* To reduce calls to g_slice_*() we (1) allocate all the memory depended on
 * the argument count in one go and (2) keep one version per argument count
 * around for faster reuse.
 */

#define GGI_INVOKE_ARG_STATE_SIZE(n)   (n * (sizeof (GGIInvokeArgState) + sizeof (GIArgument *)))
#define GGI_INVOKE_ARG_STATE_N_MAX     10
static gpointer free_arg_state[GGI_INVOKE_ARG_STATE_N_MAX];

gboolean
_ggi_invoke_arg_state_init (GGIInvokeState *state)
{
  gpointer mem;
}

SCM_DEFINE (scm_wrap_g_callable_info_invoke, "%wrap-callable-info-invoke", 3, 0, 0,
            (SCM scm_callable_info, SCM scm_args, SCM scm_kwargs),
            "")
{
  if (SCM_UNBNDP (scm_foreign_object_ref (scm_callable_info, 1))) {
    return SCM_UNDEFINED;
  }
  return SCM_UNDEFINED;
}

