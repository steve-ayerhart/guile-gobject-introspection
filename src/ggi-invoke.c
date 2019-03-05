/* -*- Mode: C; c-basic-offset: 4 -*-
 */

#include "ggi-invoke.h"
#include "ggi-infos.h"

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

SCM
_wrap_g_callable_info_invoke (SCM callable_info,
                              SCM scm_args,
                              SCM scm_kwargs)
{

  /*
    TODO: cache
  if (SCM_UNBNDP (scm_foreign_object_ref (scm_callable_info, 1))) {
    return SCM_UNDEFINED;
  }
  */

  GIBaseInfo *base_info = ggi_object_get_gi_info (callable_info);
  GIInfoType type = g_base_info_get_type ()

}

