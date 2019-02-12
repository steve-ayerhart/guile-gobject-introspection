#include <stdio.h>
#include <string.h>
#include "gc.h"
#include "gutil.h"
#include "gvalue.h"
#include "gclosure.h"

typedef struct _GuileGClosure GuileGClosure;

struct _GuileGClosure {
  GClosure closure;

  SCM func;
  GType rtype;
  guint n_params;
  GType *ptypes;
};

typedef struct {
  GClosure *closure;
  GValue *return_value;
  guint n_param_values;
  const GValue *param_values;
  gpointer invocation_hint;
  gpointer marshal_data;
} closure_data;

static void*
scm_gclosure_marshal_with_guile (const closure_data *d)
#define FUNC_NAME "scm-gclosure-marshal"
{
  GuileGClosure *gclosure = (GuileGClosure *) d->closure;
  SCM params = SCM_EOL, retval;
  guint i;

  for (i = 0; i < d->n_param_values; i++)
    params = scm_cons (scm_c_gvalue_ref (&d->param_values[i]),
                       params);
  params = scm_reverse_x (params, SCM_EOL);

  retval = scm_apply (gclosure->func, params, SCM_EOL);

  if (d->return_value
      && G_VALUE_TYPE (d->return_value) != G_TYPE_NONE
      && G_VALUE_TYPE (d->return_value) != G_TYPE_INVALID)
    scm_c_gvalue_set (d->return_value, retval);

  return NULL;
}
#undef FUNC_NAME


static void
scm_gclosure_marshal (GClosure *closure, GValue *return_value,
                      guint n_param_values, const GValue *param_values,
                      gpointer invocation_hint, gpointer marshal_data)
{
  closure_data data = {
    closure, return_value, n_param_values, param_values,
    invocation_hint, marshal_data
  };
  /* GThreadFunc is void* (*func)(void*), just like we need */
  scm_with_guile ((GThreadFunc)scm_gclosure_marshal_with_guile, &data);
}

static void
free_closure (gpointer data, GClosure *closure)
{
  GuileGClosure *gclosure = (GuileGClosure*)closure;
  //  DEBUG_ALLOC ("  unprotecting closure %p of GuileGClosure %p",
  //            gclosure->func, gclosure);

  if (gclosure->ptypes)
    g_free (gclosure->ptypes);
  gclosure->ptypes = NULL;

  scm_glib_gc_unprotect_object
    (SCM_TO_GPOINTER (((GuileGClosure *) closure)->func));
  ((GuileGClosure *) closure)->func = SCM_UNDEFINED;
}

SCM_DEFINE (scm_sys_gclosure_construct, "gclosure-construct", 4, 0, 0,
            (SCM closure, SCM return_type, SCM param_types, SCM func),
            "")
#define FUNC_NAME s_scm_sys_gclosure_construct
{
  GClosure *gclosure;
  GuileGClosure *ggclosure;
  GValue *value;
  GType rtype = G_TYPE_NONE;
  guint i;
  SCM walk;

  SCM_VALIDATE_GVALUE_TYPE_COPY (1, closure, G_TYPE_CLOSURE, value);
  if (SCM_NFALSEP (return_type))
    SCM_VALIDATE_GTYPE_CLASS_COPY (2, return_type, rtype);
  SCM_VALIDATE_LIST (3, param_types);
  SCM_VALIDATE_PROC (4, func);

  gclosure = g_closure_new_simple (sizeof (GuileGClosure), NULL);
  ggclosure = (GuileGClosure*)gclosure;

  ggclosure->rtype = rtype;
  ggclosure->ptypes = g_new (GType, scm_ilength (param_types));

  for (i = 0, walk = param_types; SCM_CONSP (walk);
       walk = scm_cdr (walk), i++)
    ggclosure->ptypes[i] = scm_c_gtype_class_to_gtype (scm_car (walk));

  //  DEBUG_ALLOC ("  protecting new closure %p of GuileGClosure %p", func, closure);
  scm_glib_gc_protect_object (func);
  ggclosure->func = func;

  g_closure_ref (gclosure);
  g_closure_sink (gclosure);

  g_closure_set_marshal (gclosure, scm_gclosure_marshal);
  g_closure_add_invalidate_notifier (gclosure, NULL, free_closure);
  g_value_take_boxed (value, gclosure);
  /* closure->ref_count is 1, and is not floating */

  return SCM_UNSPECIFIED;
}
#undef FUNC_NAME

static void*
scm_closure_primitive_invoke_without_guile (closure_data *args)
{
  g_closure_invoke (args->closure, args->return_value, args->n_param_values,
                    args->param_values, NULL);
  return NULL;
}

SCM_DEFINE (scm_gclosure_invoke, "gclosure-invoke", 2, 0, 1,
            (SCM closure, SCM return_type, SCM args),
            "Invoke a closure.\n\n"
            "A @code{<gclosure>} in GLib's abstraction for a callable object. "
            "This abstraction carries no type information, so the caller must "
            "supply all arguments as typed <g-value> instances, which may be "
            "obtained by the scheme procedure, @code{scm->g-value}.\n\n"
            "As you can see, this is a low-level function. In fact, it is "
            "not used internally by the @code{guile-gobject} bindings.")
#define FUNC_NAME s_scm_gclosure_invoke
{
  GClosure *gclosure;
  GType return_gtype = G_TYPE_NONE;
  SCM values = SCM_EOL;
  SCM retval = SCM_UNSPECIFIED;
  GValue *gvalue, *params, retval_param = { 0, };
  long n_params = 0, i;

  SCM_VALIDATE_GVALUE_TYPE_COPY (1, closure, G_TYPE_CLOSURE, gvalue);
  gclosure = g_value_get_boxed (gvalue);

  n_params = scm_ilength (args);
  params = g_new0 (GValue, n_params);
  for (i = 0; SCM_CONSP (args); args = scm_cdr (args), i++) {
    const GValue *peeked;
    SCM_ASSERT (SCM_GVALUEP (scm_car (args)), scm_car (args), 1+i,
                FUNC_NAME);
    peeked = scm_c_gvalue_peek_value (scm_car (args));
    g_value_init (&params[i], G_VALUE_TYPE (peeked));
    g_value_copy (peeked, &params[i]);
  }

  if (SCM_NFALSEP (return_type)) {
    return_gtype = scm_c_gtype_class_to_gtype (return_type);
    g_value_init (&retval_param, return_gtype);
  }

  {
    closure_data cdata = { gclosure, (G_VALUE_TYPE (&retval_param)
                                      ? &retval_param : NULL),
                           n_params, params, NULL, NULL };
    scm_without_guile
      ((GThreadFunc)scm_closure_primitive_invoke_without_guile, &cdata);
  }

  if (G_VALUE_TYPE (&retval_param)) {
    retval = scm_c_gvalue_ref (&retval_param);
    g_value_unset (&retval_param);
  }

  for (i = 0; i < n_params; i++)
    g_value_unset (&params[i]);
  g_free (params);

  scm_remember_upto_here_1 (values);

  return retval;
}
#undef FUNC_NAME

volatile static GType gcc_please_look_the_other_way;
void
scm_gobject_gclosure_init (void)
{
#ifndef SCM_MAGIC_SNARFER
#include "gclosure.x"
#endif
  /* make sure the GClosure type is registered */
  gcc_please_look_the_other_way = g_closure_get_type ();
}
