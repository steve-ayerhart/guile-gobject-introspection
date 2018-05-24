#ifndef __GUILE_GOBJECT_GTYPE_H__
#define __GUILE_GOBJECT_GTYPE_H__

#include <glib-object.h>
#include <libguile.h>

G_BEGIN_DECLS

typedef gpointer (*scm_t_gtype_instance_ref)(gpointer instance);
typedef void (*scm_t_gtype_instance_unref)(gpointer instance);
typedef gpointer (*scm_t_gtype_instance_get_qdata)(gpointer instance, GQuark quark);
typedef void (*scm_t_gtype_instance_set_qdata)(gpointer instance, GQuark quark, gpointer data);
typedef void* (*scm_t_gtype_instance_construct)(SCM object, SCM initargs);
typedef void (*scm_t_gtype_instance_initialize_scm)(SCM object, gpointer instance);
typedef struct {
  GType type;
  scm_t_gtype_instance_ref ref;
  scm_t_gtype_instance_unref unref;
  scm_t_gtype_instance_get_qdata get_qdata;
  scm_t_gtype_instance_set_qdata set_qdata;
  scm_t_gtype_instance_construct construct;
  scm_t_gtype_instance_initialize_scm initialize_scm;
} scm_t_gtype_instance_funcs;

extern SCM scm_class_gtype_class;
extern SCM scm_class_gtype_instance;
extern SCM scm_sys_gtype_to_class;

// helper macros
#define SCM_GTYPE_CLASSP(scm) \
  SCM_NFALSEP (scm_memq (scm_class_gtype_class, scm_class_precedence_list (scm_class_of ((scm)))))
#define SCM_GTYPE_INSTANCEP(scm) SCM_IS_A_P (scm, scm_class_gtype_instance)

#define SCM_VALIDATE_GTYPE_CLASS(pos, scm) SCM_MAKE_VALIDATE (pos, scm, GTYPE_CLASSP)
#define SCM_VALIDATE_GTYPE_INSTANCE(pos, scm) SCM_MAKE_VALIDATE (pos, scm,GTYPE_INSTANCEP)

#define SCM_VALIDATE_GTYPE_CLASS_COPY(pos, scm, cvar) \
  do { \
  SCM_VALIDATE_GTYPE_CLASS (pos, scm); \
  cvar = scm_c_gtype_class_to_gtype (scm); \
  } while (0)

#define SCM_VALIDATE_GTYPE_CLASS_IS_A(pos, scm, is_a, cvar) \
  do { \
  SCM_VALIDATE_GTYPE_CLASS_COPY (pos, scm, cvar); \
  SCM_ASSERT (g_type_is_a (cvar, is_a), scm, pos, FUNC_NAME); \
  } while (0)

#define SCM_VALIDATE_GTYPE_INSTANCE_COPY(pos, value, cvar) \
  do { \
  SCM_VALIDATE_GTYPE_INSTANCE (pos, value); \
  cvar = scm_c_scm_to_gtype_instance (value); \
  } while (0)

#define SCM_VALIDATE_GTYPE_INSTANCE_TYPE_COPY(pos, value, type, cvar) \
  do { \
  SCM_VALIDATE_GTYPE_INSTANCE (pos, value); \
  cvar = scm_c_scm_to_gtype_instance_typed (value, type); \
  SCM_ASSERT (cvar != NULL, value, pos, FUNC_NAME); \
  } while (0)

// C API

GType gboxed_scm_get_type (void);
#define G_TYPE_BOXED_SCM (gboxed_scm_get_type ())
GType garray_scm_get_type (void);
#define G_TYPE_GARRAY_SCM (garray_scm_get_type ())

// GTypeClass
SCM scm_c_gtype_lookup_class (GType gtype);
SCM scm_c_gtype_to_class (GType gtype);
gboolean scm_c_gtype_class_is_a_p (SCM instance, GType gtype);
GType scm_c_gtype_class_to_gtype (SCM klass);

// GTypeInstance implementations
void scm_c_gruntime_error (const char *function_name, const char *message, SCM args);
void scm_register_gtype_instance_funcs (const scm_t_gtype_instance_funcs *funcs);
void scm_register_gtype_instance_sinkfunc (GType type, void (*sinkfunc) (gpointer));

// GTypeInstance

// SCM
SCM scm_gtype_instance_destroy_x (SCM instance);

// GTypeInstance use
gpointer scm_c_gtype_instance_ref (gpointer instance);
void scm_c_gtype_instance_unref (gpointer gtype);
gboolean scm_c_gtype_instance_is_a_p (SCM instance, GType gtype);
gpointer scm_c_scm_to_gtype_instance (SCM instance);
gpointer scm_c_scm_to_gtype_instance_typed (SCM instance, GType gtype);
SCM scm_c_gtype_instance_to_scm (gpointer ginstance);

G_END_DECLS

#endif
