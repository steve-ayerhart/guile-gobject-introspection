/* -*- Mode: C; c-basic-offset: 4 -*-
 *
 */

#include <libguile.h>
#include "ggi-basic-types.h"
#include "ggi-argument.h"

#if defined(G_OS_WIN32)
#include <float.h>
static boolean
ggi_is_finite (gdouble value)
{
    return _finite(value);
}
#else
#include <math.h>
static gboolean
ggi_is_finite (double value)
{
    return isfinite (value);
}
#endif

static gboolean
marshal_from_scm_void (GGIInvokeState   *state,
                       GGICallableCache *callable_cache,
                       GGIArgCache      *arg_cache,
                       SCM               scm_arg,
                       GIArgument       *arg,
                       gpointer         *cleanup_data)
{
    g_warn_if_fail (arg_cache->transfer == GI_TRANSFER_NOTHING);

    if (ggi_gpointer_from_scm (scm_arg, &(arg->v_pointer)))
        {
            *cleanup_data = arg->v_pointer;
            return TRUE;
        }

    return FALSE;
}

static gpointer
ggi_scm_to_gpointer (SCM scm_arg)
{
    if (scm_is_false (scm_arg))
        {
            return NULL;
        }
    else if (scm_is_exact_integer (scm_arg))
        {
            return (gpointer) scm_to_uintptr_t (scm_arg);
        }
    else if (scm_is_bytevector (scm_arg) && SCM_BYTEVECTOR_LENGTH (scm_arg) >= 8)
        {
            return SCM_BYTEVECTOR_CONTENTS (scm_arg);
        }
    else
        {
            scm_misc_error (NULL,
                            "cannot convert '~a' to gpointer",
                            scm_list_1 (scm_arg));
        }
}
