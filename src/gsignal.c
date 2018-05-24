#include <stdio.h>
#include <string.h>
#include "gsignal.h"
#include "private.h"
#include "support.h"

static SCM scm_scm_make;
static SCM scm_class_gsignal;
SCM_KEYWORD (k_id, "id");
SCM_KEYWORD (k_name, "name");
SCM_KEYWORD (k_interface_type, "interface-type");
SCM_KEYWORD (k_return_type, "return-type");
SCM_KEYWORD (k_param_types, "param-types");
SCM_SYMBOL (sym_name, "name");
SCM_SYMBOL (sym_interface_type, "interface-type");
SCM_SYMBOL (sym_return_type, "return-type");
SCM_SYMBOL (sym_param_types, "param-types");

static SCM
scm_c_gsignal_query (guint id)
{
  SCM args, param_types = SCM_EOL;
  GSignalQuery q;
  guint i;

  g_signal_query (id, &q);

  for (i = q.n_params; i > 0; i--)
    param_types = scm_cons (scm_c_gtype_to_class (q.param_types [i-1]),
                            param_types);

  args = scm_list_n (scm_class_gsignal,
                     k_id, scm_from_uint (q.signal_id),
                     k_name, scm_from_locale_string (q.signal_name),
                     k_interface_type, scm_c_gtype_to_class (q.itype),
                     k_return_type, q.return_type == G_TYPE_NONE
                     ? SCM_BOOL_F : scm_c_gtype_to_class (q.return_type),
                     k_param_types, param_types,
                     SCM_UNDEFINED);
  return scm_apply_0 (scm_scm_make, args);
}

SCM_DEFINE (scm_gsignal_query, "g-signal-query", 2, 0, 0,
            (SCM class, SCM name),
            "")
#define FUNC_NAME s_scm_gsignal_query
{
  GType type;
  guint id;
  char *cname;

  SCM_VALIDATE_GTYPE_CLASS_COPY (1, class, type);
  SCM_VALIDATE_SYMBOL (2, name);

  cname = scm_symbol_chars (name);
  id = g_signal_lookup (cname, type);
  free (cname);
  if (!id)
    scm_c_gruntime_error (FUNC_NAME, "Unknown signal ~A on class ~A",
                          SCM_LIST2 (name, class));

  return scm_c_gsignal_query (id);
}
#undef FUNC_NAME

SCM_DEFINE (scm_gtype_class_get_signals, "g-type-class-get-signals", 1, 1, 0,
            (SCM class, SCM tail),
            "Returns a list of signals belonging to @var{class} and all "
            "parent types.")
#define FUNC_NAME s_scm_gtype_class_get_signals
{
  GType type;
  SCM supers;
  guint *ids, n_ids, i;

  SCM_VALIDATE_GTYPE_CLASS_COPY (1, class, type);
  if (SCM_UNBNDP (tail))
    tail = SCM_EOL;

  if (!type)
    return tail;

  if (!(G_TYPE_IS_INSTANTIATABLE (type) || G_TYPE_IS_INTERFACE (type)))
    return tail;

  ids = g_signal_list_ids (type, &n_ids);

  for (i = n_ids; i > 0; i--)
    tail = scm_cons (scm_c_gsignal_query (ids[i-1]), tail);

  g_free (ids);

  for (supers = scm_class_direct_supers (class); SCM_CONSP (supers);
       supers = scm_cdr (supers))
    if (SCM_GTYPE_CLASSP (scm_car (supers)))
      tail = scm_gtype_class_get_signals (scm_car (supers), tail);

  return tail;
}
#undef FUNC_NAME

SCM_DEFINE (scm_gsignal_create, "g-signal-create", 2, 0, 0,
            (SCM signal, SCM closure),
            "")
#define FUNC_NAME s_scm_gsignal_create
{
  GClosure *gclosure;
  GValue *gvalue;
  gulong i, length;
  GType *param_types;
  SCM params, rtype;
  guint id;

#define REF(slot) scm_slot_ref (signal, sym_##slot)

  SCM_VALIDATE_GVALUE_TYPE_COPY (2, closure, G_TYPE_CLOSURE, gvalue);
  gclosure = g_value_get_boxed (gvalue);

  params = REF(param_types);
  length = scm_ilength (params);
  param_types = g_new0 (GType, length);
  for (i = 0; i < length; i++, params = scm_cdr (params))
    param_types[i] = scm_c_gtype_class_to_gtype (scm_car (params));
  rtype = REF (return_type);

  scm_dynwind_begin (0);

  id = g_signal_newv (scm_symbol_chars_dynwind (REF (name)),
                      scm_c_gtype_class_to_gtype (REF (interface_type)),
                      G_SIGNAL_RUN_LAST,
                      gclosure,
                      NULL, NULL, NULL,
                      SCM_FALSEP (rtype)
                      ? G_TYPE_NONE : scm_c_gtype_class_to_gtype (rtype),
                      length, param_types);

  scm_dynwind_end ();

  return scm_from_uint (id);
#undef REF
}
#undef FUNC_NAME

SCM_DEFINE (scm_gtype_instance_signal_emit, "g-type-instance-signal-emit", 2, 0, 1,
            (SCM object, SCM name, SCM args),
            "")
#define FUNC_NAME s_scm_gtype_instance_signal_emit
{
  GValue *params;
  GType gtype;
  SCM walk, retval;
  GTypeInstance *instance;
  GValue ret = { 0, };
  GSignalQuery query;
  guint i, id;
  char *cname;

  SCM_VALIDATE_GTYPE_INSTANCE_COPY (1, object, instance);
  SCM_VALIDATE_SYMBOL (2, name);

  gtype = G_TYPE_FROM_INSTANCE (instance);
  cname = scm_symbol_chars (name);
  id = g_signal_lookup (cname, gtype);
  free (cname);

  if (!id)
    scm_c_gruntime_error (FUNC_NAME, "Unknown signal ~A on object ~A",
                          SCM_LIST2 (name, object));

  g_signal_query (id, &query);

  params = g_new0 (GValue, query.n_params + 1);
  g_value_init (&params[0], gtype);
  scm_c_gvalue_set (&params[0], object);

  for (walk = args, i = 0; i < query.n_params && SCM_CONSP (walk);
       i++, walk = scm_cdr (walk)) {
    g_value_init (&params[i+1],
                  query.param_types[i] & ~G_SIGNAL_TYPE_STATIC_SCOPE);
    scm_c_gvalue_set (&params[i+1], scm_car (walk));
  }
  SCM_ASSERT (i == query.n_params && SCM_NULLP (walk), args, 3, FUNC_NAME);

  if (query.return_type != G_TYPE_NONE) {
    g_value_init (&ret, query.return_type & ~G_SIGNAL_TYPE_STATIC_SCOPE);
    g_signal_emitv (params, id, 0, &ret);
    retval = scm_c_gvalue_ref (&ret);
    g_value_unset (&ret);
  } else {
    g_signal_emitv (params, id, 0, NULL);
    retval = SCM_UNSPECIFIED;
  }

  for (i = 0; i < query.n_params + 1; i++)
    g_value_unset (&params[i]);
  g_free(params);

  return retval;
}
#undef FUNC_NAME

SCM_DEFINE (scm_gtype_instance_signal_connect_closure,
            "g-type-instance-signal-connect-closure", 4, 1, 0,
            (SCM object, SCM id, SCM closure, SCM after, SCM detail),
            "")
#define FUNC_NAME s_scm_gtype_instance_signal_connect_closure
{
  GClosure *gclosure;
  GValue *gvalue;
  GTypeInstance *instance;
  GSignalQuery query;
  GType gtype;
  gulong signal_id, handler_id;
  GQuark detail_quark = 0;
#ifdef DEBUG_PRINT
  guint old_ref_count;
#endif

  SCM_VALIDATE_GTYPE_INSTANCE_COPY (1, object, instance);
  SCM_VALIDATE_UINT_COPY (2, id, signal_id);
  SCM_VALIDATE_GVALUE_TYPE_COPY (3, closure, G_TYPE_CLOSURE, gvalue);
  SCM_VALIDATE_BOOL (4, after);
  if (!SCM_UNBNDP (detail) && SCM_NFALSEP (detail)) {
    SCM_VALIDATE_SYMBOL (5, detail);
    detail_quark = g_quark_from_string (scm_symbol_chars (detail));
  }

  gtype = G_TYPE_FROM_INSTANCE (instance);
  gclosure = g_value_get_boxed (gvalue);

  g_signal_query (signal_id, &query);
  SCM_ASSERT (g_type_is_a (gtype, query.itype), object, SCM_ARG1, FUNC_NAME);

#ifdef DEBUG_PRINT
  old_ref_count = gclosure->ref_count;
#endif
  handler_id = g_signal_connect_closure_by_id (instance, signal_id,
                                               detail_quark, gclosure,
                                               SCM_NFALSEP (after));
  //  DEBUG_ALLOC ("GClosure %p connecting: %u->%u",
  //            gclosure, old_ref_count, gclosure->ref_count);

  return scm_from_ulong (handler_id);
}
#undef FUNC_NAME

SCM_DEFINE (scm_gsignal_handler_block, "g-signal-handler-block", 2, 0, 0,
            (SCM instance, SCM handler_id),
            "")
#define FUNC_NAME s_scm_gsignal_handler_block
{
  GTypeInstance *ginstance;
  gulong id;

  SCM_VALIDATE_GTYPE_INSTANCE_COPY (1, instance, ginstance);
  SCM_VALIDATE_ULONG_COPY (2, handler_id, id);

  g_signal_handler_block (ginstance, id);

  return SCM_UNSPECIFIED;
}
#undef FUNC_NAME

SCM_DEFINE (scm_gsignal_handler_unblock, "g-signal-handler-unblock", 2, 0, 0,
            (SCM instance, SCM handler_id),
            "")
#define FUNC_NAME s_scm_gsignal_handler_unblock
{
  GTypeInstance *ginstance;
  gulong id;

  SCM_VALIDATE_GTYPE_INSTANCE_COPY (1, instance, ginstance);
  SCM_VALIDATE_ULONG_COPY (2, handler_id, id);

  g_signal_handler_unblock (ginstance, id);

  return SCM_UNSPECIFIED;
}
#undef FUNC_NAME

SCM_DEFINE (scm_gsignal_handler_disconnect, "g-signal-handler-disconnect", 2, 0, 0,
            (SCM instance, SCM handler_id),
            "")
#define FUNC_NAME s_scm_gsignal_handler_disconnect
{
  GTypeInstance *ginstance;
  gulong id;

  SCM_VALIDATE_GTYPE_INSTANCE_COPY (1, instance, ginstance);
  SCM_VALIDATE_ULONG_COPY (2, handler_id, id);

  g_signal_handler_disconnect (ginstance, id);

  return SCM_UNSPECIFIED;
}
#undef FUNC_NAME

SCM_DEFINE (scm_gsignal_handler_connected_p, "g-signal-handler-connected?", 2, 0, 0,
            (SCM instance, SCM handler_id),
            "")
#define FUNC_NAME s_scm_gsignal_handler_connected_p
{
  GTypeInstance *ginstance;
  gulong id;

  SCM_VALIDATE_GTYPE_INSTANCE_COPY (1, instance, ginstance);
  SCM_VALIDATE_ULONG_COPY (2, handler_id, id);

  return g_signal_handler_is_connected (ginstance, id) ? SCM_BOOL_T : SCM_BOOL_F;
}
#undef FUNC_NAME


void
scm_gobject_gsignal_init (void)
{
#ifndef SCM_MAGIC_SNARFER
#include "gsignal.x"
#endif
  scm_class_gsignal = scm_permanent_object (SCM_VARIABLE_REF (scm_c_lookup ("<g-signal>")));
  scm_scm_make = scm_permanent_object (SCM_VARIABLE_REF (scm_c_lookup ("make")));
}
