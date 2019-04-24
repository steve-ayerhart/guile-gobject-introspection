#ifndef __SCMGI_TYPE_H__
#define __SCMGI_TYPE_H__

#include <libguile.h>
#include <glib-object.h>
#include <girepository.h>

#include "scmgobject-internal.h"

G_BEGIN_DECLS

extern SCM ScmGTypeWrapper_type;

typedef SCM (* wrap) (const GValue *value);
typedef int (* unwrap) (GValue *value, SCM obj);

typedef struct {
  wrap fromvalue;
  unwrap tovalue;
} ScmGTypeMarshal;

ScmGTypeMarshal *
scmg_type_lookup (GType type);

gboolean
scmg_gtype_is_custom (GType gtype);


void
scm_c_register_gtype (GType type,
                      wrap wrap_func,
                      unwrap unwrap_func);

int
scmgi_type_register_types (SCM s);


SCM
scmg_type_wrapper_new (GType type);

GType
scmg_type_from_object_strict (SCM scm_obj, gboolean strict);

GType
scmg_type_from_object (SCM scm_obj);


GClosure *
scmg_closure_new (SCM callback, SCM extra_args, SCM swap_data);

GClosure *
scmg_signal_class_closure_get (void);

void
scmg_closure_set_exception_handler (GClosure *closure,
                                    SCM handler);

SCM scmgi_type_import_by_g_type (GType gtype);
SCM scmgi_type_import_by_name (const char *namespace_, const char* name);
SCM scmgi_type_import_by_gi_info (GIBaseInfo *info);
SCM scmgi_type_get_from_g_type (GType gtype);

void
scmgi_type_init (void);

G_END_DECLS

#endif
