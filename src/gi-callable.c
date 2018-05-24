#include "gi-callable.h"
#include "gi-types.h"
#include "gi-argument.h"

#define TRANSFER_NOTHING_SYMBOL "g-i-transfer-nothing"
#define TRANSFER_CONTAINER_SYMBOL "g-i-transfer-container"
#define TRANSFER_EVERYTHING_SYMBOL "g-i-transfer-everything"

scm_t_bits callable_info_t;
scm_t_bits callback_info_t;

SCM_DEFINE (scm_g_callable_info_get_return_type, "g-callable-info-get-return-type", 1, 0, 0,
            (SCM scm_callable_info),
            ""
            )
{
  GICallableInfo *callable_info;
  GITypeInfo *type_info;
  SCM scm_type_info;

  callable_info = (GICallableInfo *) SCM_SMOB_DATA (scm_callable_info);
  type_info = g_callable_info_get_return_type (callable_info);

  if (type_info == NULL)
    return SCM_BOOL_F;
  else {
    scm_type_info = scm_make_smob (type_info_t);
    SCM_SET_SMOB_DATA (scm_type_info, type_info);
  }

  return scm_type_info;
}

SCM_DEFINE (scm_g_callable_info_may_return_null, "g-callable-info-may-return-null?", 1, 0, 0,
            (SCM scm_callable_info),
            ""
            )
{
  GICallableInfo *callable_info;

  callable_info = (GICallableInfo *) SCM_SMOB_DATA (scm_callable_info);

  return scm_from_int (g_callable_info_may_return_null (callable_info));
}

SCM_DEFINE (scm_g_callable_info_get_n_args, "g-callable-info-get-n-args", 1, 0, 0,
            (SCM scm_callable_info),
            ""
            )
{
  GICallableInfo *callable_info;

  callable_info = (GICallableInfo *) SCM_SMOB_DATA (scm_callable_info);

  return scm_from_int (g_callable_info_get_n_args (callable_info));
}

SCM_DEFINE (scm_g_callable_info_get_arg, "g-callable-info-get-arg", 2, 0, 0,
            (SCM scm_callable_info, SCM scm_n),
            ""
            )
{
  GICallableInfo *callable_info;
  GIArgInfo *arg_info;
  SCM scm_arg_info;

  callable_info = (GICallableInfo *) SCM_SMOB_DATA (scm_callable_info);
  arg_info = g_callable_info_get_arg (callable_info, scm_to_int (scm_n));

  if (arg_info == NULL)
    scm_arg_info = SCM_BOOL_F;
  else {
    scm_arg_info = scm_make_smob (arg_info_t);
    SCM_SET_SMOB_DATA (scm_arg_info, arg_info);
  }

  return scm_arg_info;
}

void
gi_callable_init (void)
{
  #ifndef SCM_MAGIC_SNARFER
  #include "gi-callable.x"
  #endif

  callable_info_t = scm_make_smob_type ("g-i-callable-info", 0);
  callback_info_t = scm_make_smob_type ("g-i-callback-info", 0);

  scm_c_define (TRANSFER_NOTHING_SYMBOL, scm_from_int (GI_TRANSFER_NOTHING));
  scm_c_define (TRANSFER_CONTAINER_SYMBOL, scm_from_int (GI_TRANSFER_CONTAINER));
  scm_c_define (TRANSFER_EVERYTHING_SYMBOL, scm_from_int (GI_TRANSFER_EVERYTHING));
}
