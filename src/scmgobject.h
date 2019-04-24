/* -*- Mode: C; c-basic-offset: 4 -*-
 * vim: tabstop=4 shiftwidth=4 expandtab
 */

#ifndef _SCMGOBJECT_H_
#define _SCMGOBJECT_H_

#include <libguile.h>

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

// GGClosure is a _private_ structure
typedef void (* ScmClosureExceptionHandler) (GValue *ret, guint n_param_values, const GValue *params);
typedef struct _ScmGClosure ScmGClosure;
typedef struct _ScmGObjectData ScmGObjectData;

struct _ScmGClosure {
    GClosure closure;
    SCM callback;
    SCM extra_args; // list of args to pass to callback
    SCM swap_data; // other object for gtk_signal_connect_object
    ScmClosureExceptionHandler exception_handler;
};

typedef enum {
              SCMGOBJECT_USING_TOGGLE_REF = 1 << 0,
              SCMGOBJECT_IS_FLOATING_REF = 1 << 1,
              SCMGOBJECT_GOBJECT_WAS_FLOATING = 1 << 2
} ScmGObjectFlags;

typedef struct {
    GObject *obj;
    SCM instance_hash;
    SCM weakref_list;
    ScmGObjectFlags private_flags;
} ScmGObject;

#define scmgobject_get(v) (((ScmGobject *)(v))->obj)
#define scmgobject_check(v,base) (ScmGobject_TypeCheck(v,base))

typedef struct {
    gpointer boxed;
    GType gtype;
    gboolean free_on_dealloc;
} ScmGBoxed;

#define scmg_boxed_get(v,t)          ((t *)((ScmGBoxed *)(v))->boxed)
#define scmg_boxed_get_ptr(v)        (((ScmGBoxed *)(v))->boxed)
#define scmg_boxed_set_ptr(v,p)      (((ScmGBoxed *)(v))->boxed = (gpointer)p)
#define scmg_boxed_check(v,typecode) (ScmGobject_TypeCheck(v, &ScmGBoxed_Type) && ((ScmGBoxed *)(v))->gtype == typecode)

typedef struct {
    gpointer pointer;
    GType gtype;
} ScmGPointer;

#define scmg_pointer_get(v,t)          ((t *)((ScmGPointer *)(v))->pointer)
#define scmg_pointer_get_ptr(v)        (((ScmGPointer *)(v))->pointer)
#define scmg_pointer_set_ptr(v,p)      (((ScmGPointer *)(v))->pointer = (gpointer)p)
#define scmg_pointer_check(v,typecode) (ScmGobject_TypeCheck(v, &ScmGPointer_Type) && ((ScmGPointer *)(v))->gtype == typecode)

typedef void (*ScmGFatalExceptionFunc) (void);
typedef void (*ScmGThreadBlockFunc) (void);

typedef struct {
    GParamSpec *pspec;
} ScmGParamSpec;

#define scmg_param_spec_get(v)   (((ScmGParamSpec *)v)->pspec)
#define scmg_param_spec_set(v,p) (((ScmGParamSpec *)v)->pspec = (GParamSpec*)p)
#define scmg_param_spec_check(v) (ScmGobject_TypeCheck(v, &ScmGParamSpec_Type))

typedef int (*ScmGClassInitFunc) (gpointer gclass, SCM *scmclass);
typedef SCM * (*ScmGTypeRegistrationFunction) (const gchar *name, gpointer data);

struct _ScmGObject_Functions {
    /* All field names in here are considered private
     * use the macros below instead, which provides stability
     */

    void (* register_class)(SCM hash,
                            const gchar *class_name,
                            GType gtype,
                            SCM *type,
                            SCM bases);

    void (* register_wrapper)(SCM self);
    SCM *(* lookup_class)(GType type);
    SCM (* newgobj)(GObject *obj);

    GClosure *(* closure_new)(SCM callback, SCM extra_args, SCM swap_data);
    void (* object_watch_closure)(SCM self, GClosure *closure);
    GDestroyNotify destroy_notify;

    GType (* type_from_object)(SCM obj);
    SCM *(* type_wrapper_new)(GType type);

    gint (* enum_get_value)(GType enum_type, SCM obj, gint *val);
    gint (* flags_get_value)(GType flag_type, SCM obj, guint *val);
    void (* register_gtype_custom)(GType gtype,
                                   SCM (* wrap)(const GValue *value),
                                   int (* unwrap)(GValue *value, SCM obj));
    int (* value_from_scmobject)(GValue *value, SCM obj);
    SCM (* value_as_scmobject)(const GValue *value, gboolean copy_boxed);

    void (* register_interface)(SCM hash,
                                const gchar *class_name,
                                GType gtype,
                                SCM *type);

    SCM *boxed_type;
    void (* register_boxed)(SCM hash,
                            const gchar *class_name,
                            GType boxed_type,
                            SCM *type);
    SCM (* boxed_new)(GType boxed_type,
                      gpointer boxed,
                      gboolean copy_boxed,
                      gboolean own_ref);

    SCM *pointer_type;
    void (* register_pointer)(SCM hash,
                              const gchar *class_name,
                              GType pointer_type,
                              SCM *type);
    SCM (* pointer_new)(GType pointer_type, gpointer pointer);

    void (* enum_add_constants)(SCM module,
                                GType enum_type,
                                const gchar *strip_prefix);
    void (* flags_add_constants)(SCM module,
                                 GType flags_type,
                                 const gchar *strip_prefix);

    const gchar *(* constant_strip_prefix)(const gchar *name,
                                           const gchar *strip_prefix);

    gboolean (* error_check)(GError **error);

    // FIXME: GDK threads to cooperate with guile threading?

    SCM *paramspec_type;
    SCM (* paramspec_new)(GParamSpec *spec);
    GParamSpec *(* paramspec_get)(SCM scm_list);

    gboolean (*parse_constructor_args)(GType obj_type,
                                       char **arg_names,
                                       char **prop_names,
                                       GParameter *params,
                                       guint *nparams,
                                       SCM scm_args);
    SCM (* param_gvalue_as_scm) (const GValue *gvalue,
                                 gboolean copy_boxed,
                                 const GParamSpec *spec);
    int (* gvalue_from_param_scm) (GValue *value,
                                   SCM scm_obj,
                                   const GParamSpec *spec);

    SCM *enum_type;
    SCM (* enum_add)(SCM module,
                     const char *type_name_,
                     const char *strip_prefix,
                     GType gtype);
    SCM (* flags_from_gtype)(GType gtype, guint value);

    void (* register_class_init) (GType gtype, ScmGClassInitFunc class_init);
    void (* register_interface_info) (GType gtype, const GInterfaceInfo *info);
    void (* closure_set_exception_handler) (GClosure *closure, SCM handler);
    void (* add_warning_redirection) (const char *domain,
                                      SCM warning);
    void (* disable_warning_redirections) (void);

    gboolean (* gerror_exceoption_check) (GError **error);
    SCM (* option_group_new) (GOptionGroup *group);
    GType (* type_from_object_strict) (SCM obj, gboolean strict);

    SCM (* newgobj_full)(GObject *obj, gboolean steal, gpointer g_class);
    SCM object_type;
    int (* value_from_scmobject_with_error) (GValue *value, SCM obj);
};

#ifndef _INSIDE_SCMGOBJECT_

#if defined(NO_IMPORT) || defined(NO_IMPORT_SCMGOBJECT)
extern struct _ScmGObjectFunctions *_ScmGObjectAPI;
#else
struct _ScmGObjectFunctions *_ScmGObject_API;
#endif

#define scmgobject_register_class    (_ScmGObject_API->register_class)
#define scmgobject_register_wrapper  (_ScmGObject_API->register_wrapper)
#define scmgobject_lookup_class      (_ScmGObject_API->lookup_class)
#define scmgobject_new               (_ScmGObject_API->newgobj)
#define scmgobject_new_full          (_ScmGObject_API->newgobj_full)
#define ScmGObject_Type              (*_ScmGObject_API->object_type)
#define scmg_closure_new             (_ScmGObject_API->closure_new)
#define scmgobject_watch_closure     (_ScmGObject_API->object_watch_closure)
#define scmg_closure_set_exception_handler (_ScmGObject_API->closure_set_exception_handler)
#define scmg_destroy_notify          (_ScmGObject_API->destroy_notify)
#define scmg_type_from_object_strict   (_ScmGObject_API->type_from_object_strict)
#define scmg_type_from_object        (_ScmGObject_API->type_from_object)
#define scmg_type_wrapper_new        (_ScmGObject_API->type_wrapper_new)
#define scmg_enum_get_value          (_ScmGObject_API->enum_get_value)
#define scmg_flags_get_value         (_ScmGObject_API->flags_get_value)
#define scmg_register_gtype_custom   (_ScmGObject_API->register_gtype_custom)
#define scmg_value_from_scmobject     (_ScmGObject_API->value_from_scmobject)
#define scmg_value_from_scmobject_with_error (_ScmGObject_API->value_from_scmobject_with_error)
#define scmg_value_as_scmobject       (_ScmGObject_API->value_as_scmobject)
#define scmg_register_interface      (_ScmGObject_API->register_interface)
#define ScmGBoxed_Type               (*_ScmGObject_API->boxed_type)
#define scmg_register_boxed          (_ScmGObject_API->register_boxed)
#define scmg_boxed_new               (_ScmGObject_API->boxed_new)
#define ScmGPointer_Type             (*_ScmGObject_API->pointer_type)
#define scmg_register_pointer        (_ScmGObject_API->register_pointer)
#define scmg_pointer_new             (_ScmGObject_API->pointer_new)
#define scmg_enum_add_constants      (_ScmGObject_API->enum_add_constants)
#define scmg_flags_add_constants     (_ScmGObject_API->flags_add_constants)
#define scmg_constant_strip_prefix   (_ScmGObject_API->constant_strip_prefix)
#define scmg_error_check             (_ScmGObject_API->error_check)
#define ScmGParamSpec_Type           (*_ScmGObject_API->paramspec_type)
#define scmg_param_spec_new          (_ScmGObject_API->paramspec_new)
#define scmg_param_spec_from_object  (_ScmGObject_API->paramspec_get)
#define scmg_scmobj_to_unichar_conv   (_ScmGObject_API->scmobj_to_unichar_conv)
#define scmg_parse_constructor_args  (_ScmGObject_API->parse_constructor_args)
#define scmg_param_gvalue_as_scmobject   (_ScmGObject_API->value_as_scmobject)
#define scmg_param_gvalue_from_scmobject (_ScmGObject_API->gvalue_from_param_scmobject)
#define ScmGEnum_Type                (*_ScmGObject_API->enum_type)
#define scmg_enum_add                (_ScmGObject_API->enum_add)
#define scmg_enum_from_gtype         (_ScmGObject_API->enum_from_gtype)
#define ScmGFlags_Type               (*_ScmGObject_API->flags_type)
#define scmg_flags_add               (_ScmGObject_API->flags_add)
#define scmg_flags_from_gtype        (_ScmGObject_API->flags_from_gtype)
#define scmg_register_class_init     (_ScmGObject_API->register_class_init)
#define scmg_register_interface_info (_ScmGObject_API->register_interface_info)
#define scmg_add_warning_redirection   (_ScmGObject_API->add_warning_redirection)
#define scmg_disable_warning_redirections (_ScmGObject_API->disable_warning_redirections)
#define scmg_gerror_exception_check (_ScmGObject_API->gerror_exception_check)
#define scmg_option_group_new       (_ScmGObject_API->option_group_new)

void
scmgobject_init (void);

#endif /* !_INSIDE_SCMGOBJECT_ */

G_END_DECLS

#endif /* !_SCMGOBJECT_H_ */
