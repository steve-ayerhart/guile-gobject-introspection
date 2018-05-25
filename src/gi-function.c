#include "gi-function.h"
#include "gi-callable.h"
#include "gi-argument.h"
#include "gi-types.h"

#define FUNCTION_IS_METHOD "g-i-function-is-method"
#define FUNCTION_IS_CONSTRUCTOR "g-i-function-is-constructor"
#define FUNCTION_IS_GETTER "g-i-function-is-getter"
#define FUNCTION_IS_SETTER "g-i-function-is-setter"
#define FUNCTION_WRAPS_VFUNC "g-i-function-wraps-vfunc"
#define FUNCTION_THROWS "g-i-function-throws"

scm_t_bits function_info_t;

static GArgument *
construct_in_args (GICallableInfo *callable_info,
                   SCM scm_in_args,
                   int *n_in_args);

static GArgument *
construct_out_args (GICallableInfo *callable_info,
                    GArgument **real_out_args,
                    int *n_out_args,
                    int **out_arg_indices);

static SCM
construct_return_value (GICallableInfo *callable_info,
                        GArgument return_value,
                        GArgument *out_args,
                        int n_out_args,
                        int *out_arg_indices);

static GArgument *
construct_in_args (GICallableInfo *callable_info,
                   SCM scm_in_args,
                   int *n_in_args)
{
  GArgument *in_args;
  int n_args;
  int i;
  int destroy;
  int closure;

  in_args = NULL;
  *n_in_args = 0;
  destroy = -1;
  closure = -1;
  n_args = g_callable_info_get_n_args (callable_info);

  /* FIXME: We are allocating array for all arguments although it only
   *        needs to be big enough to fit 'in' arguments.
   */
  if ((g_function_info_get_flags ((GIFunctionInfo *) callable_info) &
       GI_FUNCTION_IS_METHOD) != 0) {
    GIBaseInfo *container;
    GIInfoType type;
    SCM arg;

    in_args = g_malloc0 (sizeof (GArgument) * (n_args + 1));

    container = g_base_info_get_container (
                                           (GIBaseInfo *) callable_info);
    type = g_base_info_get_type (container);

    arg = scm_list_ref (scm_in_args, scm_from_int (*n_in_args));
    scm_to_gi_interface (arg,
                         type,
                         GI_TRANSFER_NOTHING,
                         GI_SCOPE_TYPE_INVALID,
                         container,
                         NULL,
                         &in_args[*n_in_args],
                         NULL);

    (*n_in_args)++;
  } else
    in_args = g_malloc0 (sizeof (GArgument) * n_args);

  for (i = 0; i < n_args; i++) {
    GIArgInfo *arg_info;
    GIArgInfo *destroy_info;
    GITypeInfo *arg_type;
    GIDirection direction;
    GITransfer transfer;
    GIScopeType scope;
    GArgument *destroy_arg;
    SCM arg;

    /* FIXME: We are assuming that the DestroyNotify and closure are
     *        always immediately after the callback argument.
     */
    if (i == destroy) {
      (*n_in_args)++;
      destroy = -1;

      continue;
    }

    if (i == closure) {
      (*n_in_args)++;
      closure = -1;

      continue;
    }

    arg_info = g_callable_info_get_arg (callable_info, i);
    direction = g_arg_info_get_direction (arg_info);
    if (direction != GI_DIRECTION_IN &&
        direction != GI_DIRECTION_INOUT)
      continue;
    arg_type = g_arg_info_get_type (arg_info);
    transfer = g_arg_info_get_ownership_transfer (arg_info);
    scope = g_arg_info_get_scope (arg_info);
    destroy = g_arg_info_get_destroy (arg_info);
    closure = g_arg_info_get_closure (arg_info);

    if (destroy >= 0) {
      destroy_info = g_callable_info_get_arg (callable_info,
                                              destroy);
      destroy_arg = &in_args[destroy];
    } else {
      destroy_info = NULL;
      destroy_arg = NULL;
    }

    arg = scm_list_ref (scm_in_args, scm_from_int (*n_in_args));
    scm_to_gi_arg (arg,
                   arg_type,
                   transfer,
                   scope,
                   (GICallableInfo *) destroy_info,
                   &in_args[*n_in_args],
                   destroy_arg);

    (*n_in_args)++;
  }

  return in_args;
}

static GArgument *
construct_out_args (GICallableInfo *callable_info,
                    GArgument     **real_out_args,
                    int            *n_out_args,
                    int           **out_arg_indices)
{
  GArgument *out_args;
  int n_args;
  int i;

  out_args = NULL;
  *n_out_args = 0;
  n_args = g_callable_info_get_n_args (callable_info);

  if (n_args < 0)
    return NULL;

  /* FIXME: We are allocating array for all arguments although it only
   *        needs to be big enough to fit 'out' arguments.
   */
  out_args = g_malloc0 (sizeof (GArgument) * n_args);
  *real_out_args = g_malloc0 (sizeof (GArgument) * n_args);
  *out_arg_indices = g_malloc (sizeof (int) * n_args);

  for (i = 0; i < n_args; i++) {
    GIArgInfo *arg_info;
    GIDirection direction;

    arg_info = g_callable_info_get_arg (callable_info, i);
    direction = g_arg_info_get_direction (arg_info);
    if (direction != GI_DIRECTION_OUT &&
        direction != GI_DIRECTION_INOUT)
      continue;

    out_args[*n_out_args].v_pointer =
      &(*real_out_args)[*n_out_args];
    (*out_arg_indices)[*n_out_args] = i;

    (*n_out_args)++;
  }

  return out_args;
}
static SCM
construct_return_value (GICallableInfo *callable_info,
                        GArgument       return_value,
                        GArgument      *out_args,
                        int             n_out_args,
                        int            *out_arg_indices)
{
  SCM scm_return;
  SCM scm_return_value;

  scm_return = SCM_EOL;
  scm_return_value = gi_return_value_to_scm (callable_info, return_value);
  if (scm_return_value != SCM_UNSPECIFIED)
    scm_return = scm_cons (scm_return_value, scm_return);

  if (n_out_args > 0) {
    int i;

    for (i = 0; i < n_out_args; i++) {
      GIArgInfo *arg_info;
      GITypeInfo *type;
      GITransfer transfer_type;
      GArgument *argument;
      SCM scm_arg;

      arg_info = g_callable_info_get_arg (callable_info,
                                          out_arg_indices[i]);
      type = g_arg_info_get_type (arg_info);
      transfer_type = g_arg_info_get_ownership_transfer
        (arg_info);
      argument = (GArgument *) out_args[i].v_pointer;
      scm_arg = gi_arg_to_scm (type,
                               transfer_type,
                               *argument);

      if (scm_arg != SCM_UNSPECIFIED)
        scm_return = scm_cons (scm_arg, scm_return);
    }
  }

  if (scm_return != SCM_EOL)
    return scm_values (scm_reverse (scm_return));
  else
    return SCM_UNSPECIFIED;
}

SCM_DEFINE (scm_g_function_info_get_symbol, "g-function-info-get-symbol", 1, 0, 0,
            (SCM scm_function_info),
            ""
            )
{
  GIFunctionInfo *function_info;
  const gchar *symbol;

  function_info = (GIFunctionInfo *) SCM_SMOB_DATA (scm_function_info);
  symbol = g_function_info_get_symbol (function_info);

  if (symbol == NULL)
    return SCM_BOOL_F;

  return scm_from_locale_string (symbol);
}

SCM_DEFINE (scm_g_function_info_get_flags, "g-function-info-get-flags", 1, 0, 0,
            (SCM scm_function_info),
            ""
            )
{
  GIFunctionInfo *function_info;

  function_info = (GIFunctionInfo *) SCM_SMOB_DATA (scm_function_info);

  return scm_from_int (g_function_info_get_flags (function_info));
}

SCM_DEFINE (scm_g_function_info_get_property, "g-function-info-get-property", 1, 0, 0,
            (SCM scm_function_info),
            ""
            )
{
  GIFunctionInfo *function_info;
  GIPropertyInfo *property_info;
  SCM scm_property_info;

  function_info = (GIFunctionInfo *) SCM_SMOB_DATA (scm_function_info);
  property_info = g_function_info_get_property (function_info);

  if (property_info == NULL)
    scm_property_info = SCM_BOOL_F;
  else {
    scm_property_info = scm_make_smob (property_info_t);
    SCM_SET_SMOB_DATA (scm_property_info, property_info);
  }

  return scm_property_info;
}

SCM_DEFINE (scm_g_function_info_get_vfunc, "g-function-info-get-vfunc", 1, 0, 0,
            (SCM scm_function_info),
            ""
            )
{
  GIFunctionInfo *function_info;
  GIVFuncInfo *vfunc_info;
  SCM scm_vfunc_info;

  function_info = (GIFunctionInfo *) SCM_SMOB_DATA (scm_function_info);
  vfunc_info = g_function_info_get_vfunc (function_info);

  if (vfunc_info == NULL)
    scm_vfunc_info = SCM_BOOL_F;
  else {
    scm_vfunc_info = scm_make_smob (v_func_info_t);
    SCM_SET_SMOB_DATA (scm_vfunc_info, vfunc_info);
  }

  return scm_vfunc_info;
}


SCM_DEFINE (scm_g_function_info_invoke, "g-function-info-invoke", 2, 0, 0,
            (SCM scm_function_info, SCM scm_in_args),
            ""
            )
{
  GIFunctionInfo *info;
  GICallableInfo *callable_info;
  int n_in_args;
  int n_out_args;
  int *out_arg_indices;
  GArgument *in_args;
  GArgument *out_args;
  GArgument *real_out_args;
  GArgument return_value;
  GError *error;
  SCM scm_return;

  info = (GIFunctionInfo *) SCM_SMOB_DATA (scm_function_info);
  callable_info = (GICallableInfo *) info;
  out_arg_indices = NULL;
  real_out_args = NULL;

  in_args = construct_in_args (callable_info, scm_in_args, &n_in_args);
  out_args = construct_out_args (callable_info,
                                 &real_out_args,
                                 &n_out_args,
                                 &out_arg_indices);

  error = NULL;
  g_function_info_invoke (info,
                          in_args,
                          n_in_args,
                          out_args,
                          n_out_args,
                          &return_value,
                          &error);
  if (error) {
    /* FIXME: Throw Guile error here */
    g_critical ("Failed to invoke function: %s", error->message);

    g_error_free (error);

    return SCM_UNSPECIFIED;
  }

  scm_return = construct_return_value (callable_info,
                                       return_value,
                                       out_args,
                                       n_out_args,
                                       out_arg_indices);

  g_free (in_args);
  g_free (out_args);
  g_free (out_arg_indices);
  g_free (real_out_args);

  return scm_return;
}

void
gi_function_init (void)
{
  #ifndef SCM_MAGIC_SNARFER
  #include "gi-function.x"
  #endif

  function_info_t = scm_make_smob_type ("g-i-function-info", 0);

  scm_c_define (FUNCTION_IS_METHOD, scm_from_int (GI_FUNCTION_IS_METHOD));
  scm_c_define (FUNCTION_IS_CONSTRUCTOR, scm_from_int (GI_FUNCTION_IS_CONSTRUCTOR));
  scm_c_define (FUNCTION_IS_GETTER, scm_from_int (GI_FUNCTION_IS_GETTER));
  scm_c_define (FUNCTION_IS_SETTER, scm_from_int (GI_FUNCTION_IS_SETTER));
  scm_c_define (FUNCTION_WRAPS_VFUNC, scm_from_int (GI_FUNCTION_WRAPS_VFUNC));
  scm_c_define (FUNCTION_THROWS, scm_from_int (GI_FUNCTION_THROWS));
}
