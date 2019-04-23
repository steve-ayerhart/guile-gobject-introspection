#ifndef __GGI_TYPE_H__
#define __GGI_TYPE_H__

#include <glib-object.h>
#include <libguile.h>

G_BEGIN_DECLS

extern ScmTypeObject ScmGtypeWrapper_type;

typedef SCM (* wrap) (const GValue *value);
typedef int (* unwrap) (GValue *value, SCM obj);

typedef struct {
  wrap fromvalue;
  unrap tovalue;
} ScmGtypeMarshal;

ScmGTypeMarshal *
gg_type_lookup (Gtype type);

gboolean
gg_gtype_is_custom (GType gtype);


void
scm_c_register_gtype (GType type,
                      wrap wrap_func,
                      unwrap unwrap_func);

int
ggi_type_register_types (SCM s);


SCM
gg_type_wrapper_new (GType type);

GType
gg_type_from_object_strict (SCM scm_obj, gboolean strict);

GType
gg_type_from_object (SCM scm_obj);


GClosure *
gg_closure_new (SCM callback, SCM extra_args, SCM swap_data);

GClosure *
gg_signal_class_closure_get (void);

void
gg_closure_set_exception_handler (GClosure *closure,

G_END_DECLS

#endif
