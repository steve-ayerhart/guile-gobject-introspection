#include <stdio.h>
#include <stdio.h>
#include <glib.h>

#include "gc.h"
#include "gobject.h"
#include "private.h"
#include "support.h"

SCM scm_class_gobject;
static SCM _initialize;
static SCM _gobject_set_property;
static SCM _gobject_get_property;
static SCM _in_construction_from_scheme;
static SCM _slot_definition_options;

static GQuark quark_guile_gtype_class = 0;

SCM_SYMBOL  (sym_gruntime_error,"gruntime-error");
SCM_KEYWORD (kw_init_keyword, "init-keyword");

/* #define DEBUG_PRINT */

#define DEBUG_REFCOUNTING

static gpointer scm_c_gobject_construct (SCM instance, SCM initargs);
//static void scm_c_gobject_initialize_scm (SCM instance, SCM initargs);

static const scm_t_gtype_instance_funcs gobject_funcs = {
  G_TYPE_OBJECT,
  (scm_t_gtype_instance_ref)g_object_ref,
  (scm_t_gtype_instance_unref)g_object_unref,
  (scm_t_gtype_instance_get_qdata)g_object_get_qdata,
  (scm_t_gtype_instance_set_qdata)g_object_set_qdata,
  (scm_t_gtype_instance_construct)scm_c_gobject_construct,
  NULL
  //    (scm_t_gtype_instance_initialize_scm)scm_c_gobject_initialize_scm
};

// FIXME
static inline void post_make_object (GObject *obj);

static void
scm_with_c_gobject_get_property (GObject *gobject, guint param_id,
                                 GValue *dest_gvalue, GParamSpec *pspec)
{
  scm_c_gvalue_set (dest_gvalue,
                    scm_call_2 (_gobject_get_property,
                                scm_c_gtype_instance_to_scm (gobject),
                                scm_from_locale_symbol (pspec->name)));
}

static void
scm_c_gobject_get_property (GObject *gobject, guint param_id, GValue *dest_gvalue, GParamSpec *pspec)
{
  return scm_dynwind_guile_v__p_u_p_p (scm_with_guile,
                                       scm_with_c_gobject_get_property,
                                       gobject, param_id, dest_gvalue, pspec);
}

static void
scm_with_c_gobject_set_property (GObject *gobject, guint param_id, const GValue *src_value, GParamSpec *pspec)
{
  scm_call_3 (_gobject_set_property,
              scm_c_gtype_instance_to_scm (gobject),
              scm_from_locale_symbol (pspec->name),
              scm_c_gvalue_to_scm (src_value));
}

static void
scm_c_gobject_set_property (GObject *gobject, guint param_id, const GValue *src_value, GParamSpec *pspec)
{
  return scm_dynwind_guile_v__p_u_c_p (scm_with_guile,
                                       scm_with_c_gobject_set_property,
                                       gobject, param_id, src_value, pspec);
}


static gboolean
is_init_keyword (SCM slots, SCM kw)
{
  SCM defs;

  for (; scm_is_pair (slots); slots = scm_cdr (slots))
    for (defs = scm_call_1 (_slot_definition_options, scm_car (slots));
         scm_is_pair (defs);
         defs = scm_cddr (defs))
      if (scm_is_eq (scm_car (defs), kw_init_keyword)
          && scm_is_eq (scm_cadr (defs), kw))
        return TRUE;

  return FALSE;
}

static void
push_in_construction_from_scheme (SCM instance)
{
  SCM stack = scm_fluid_ref (_in_construction_from_scheme);
  scm_fluid_set_x (_in_construction_from_scheme, scm_cons (instance, stack));
}

static void
pop_in_construction_from_scheme (void)
{
  SCM stack = scm_fluid_ref (_in_construction_from_scheme);
  scm_fluid_set_x (_in_construction_from_scheme, scm_cdr (stack));
}

static SCM
in_construction_from_scheme (void)
{
  SCM stack = scm_fluid_ref (_in_construction_from_scheme);
  return scm_is_null (stack) ? SCM_BOOL_F : scm_car (stack);
}

static gpointer
scm_c_gobject_construct (SCM instance, SCM initargs)
#define FUNC_NAME "gobject-construct"
{
  GObject *gobject;
  GObjectClass *propclass;
  GType gtype;
  const char **param_names; const char *current_name;
  GValue *param_values; GValue *current_value;

  GParamSpec *pspec;
  long nparams, i;
  SCM class, slots, kw, propname, val;

  SCM_VALIDATE_INSTANCE (1, instance);

  scm_dynwind_begin (0);

  class = scm_class_of (instance);
  gtype = scm_c_gtype_class_to_gtype (class);
  slots = scm_class_slots (class);
  nparams = scm_ilength (initargs) / 2; // a maximum length

  param_names = g_malloc (nparams * sizeof (char *));
  param_values = g_malloc (nparams * sizeof (GValue *));

  scm_dynwind_unwind_handler ((gpointer)g_free, param_names, SCM_F_WIND_EXPLICITLY);
  scm_dynwind_unwind_handler ((gpointer)g_free, param_values, SCM_F_WIND_EXPLICITLY);

  for (i = 0; SCM_CONSP (initargs); initargs = scm_cddr (initargs)) {
    kw = scm_car (initargs);
    SCM_ASSERT (scm_is_keyword (kw), kw, 2, FUNC_NAME);
    SCM_ASSERT (SCM_CONSP (scm_cdr (initargs)), initargs, 2, FUNC_NAME);
    propname = scm_keyword_to_symbol (kw);
    val = scm_cadr (initargs);

    if (is_init_keyword (slots, kw))
      continue;

    current_name = scm_keyword_chars_dynwind (kw);
    param_names[i] = current_name;

    propclass = g_type_class_ref (gtype);
    pspec = g_object_class_find_property (propclass, current_name);
    g_type_class_unref (propclass);

    if (!pspec)
      scm_c_gruntime_error (FUNC_NAME,
                            "No property named ~S in object ~A",
                            SCM_LIST2 (propname, instance));

    #pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
    g_value_init (current_value, G_PARAM_SPEC_VALUE_TYPE (pspec));
    scm_c_gvalue_set (current_value, val);

    param_values[i] = *current_value;

    i++;
  }

  push_in_construction_from_scheme (instance);
  gobject = g_object_new_with_properties (gtype, nparams, param_names, param_values);
  pop_in_construction_from_scheme ();

  /* GtkWindow's first ref is owned by GTK. */
  post_make_object (gobject);

  for (i--; i>=0; i--)
    g_value_unset (&param_values[i]);

  g_free (param_names);
  g_free (param_values);

  scm_dynwind_end ();

  return gobject;
}
#undef FUNC_NAME

static void
scm_with_c_gtype_instance_instance_init (GTypeInstance *g_instance,
                                         gpointer g_class)
{
  GType type;
  SCM class;

  type = G_TYPE_FROM_CLASS (g_class);

  /* make sure we know about the class */
  class = scm_c_gtype_lookup_class (type);
  g_assert (SCM_NFALSEP (class));

  /* It seems that as an object is initialized, the g_class argument to the
   * init function is the same for each level of inherited classes. However --
   * and this shit tripped me up for a while -- _the class of the instance
   * changes for each level of the init process_. Thus if you want to know the
   * real type of the object, use G_TYPE_FROM_CLASS (g_class). If you want to
   * know which derived class is being initialized (as in a gobject class
   * doubly-specialized on the scheme side), use G_TYPE_FROM_INSTANCE
   * (g_instance). Fucked up! */

  switch (G_TYPE_FUNDAMENTAL (type)) {
  case G_TYPE_OBJECT: {
    GuileGTypeClass *guile_class;
    SCM under_construction = in_construction_from_scheme ();

    guile_class = g_type_get_qdata (type, quark_guile_gtype_class);
    guile_class->first_instance_created = TRUE;

    if (scm_is_false (under_construction))
      /* not strictly necessary from the pov of c code, but we want to
         make sure that g_object_new () causes `initialize' to be called
         on a new scheme object -- hence this call that just serves to
         associate a scheme object with the instance as long as the
         instance is alive */
      scm_c_gtype_instance_to_scm_typed (g_instance, type);
    else
      scm_c_gtype_instance_bind_to_object (g_instance, under_construction);

    break;
  }

  default:
    break;
  }
}

static void
scm_c_gtype_instance_instance_init (GTypeInstance *g_instance,
                                    gpointer g_class)
{
  scm_dynwind_guile_v__p_p (scm_with_guile,
                            scm_with_c_gtype_instance_instance_init,
                            g_instance, g_class);
}

static void
scm_with_c_gtype_instance_class_init (gpointer g_class, gpointer class_data)
{
  GuileGTypeClass *guile_class;
  SCM class;

  class = scm_c_gtype_lookup_class (G_TYPE_FROM_CLASS (g_class));
  if (SCM_FALSEP (class)) {
    /* this can happen for scheme-defined classes */
    class = scm_c_gtype_to_class (G_TYPE_FROM_CLASS (g_class));
  }
  g_assert (SCM_NFALSEP (class));

  guile_class = g_type_get_qdata (G_TYPE_FROM_CLASS (g_class), quark_guile_gtype_class);
  g_assert (guile_class != NULL);

  //DEBUG_ALLOC ("  protecting class %p of %s gclass %p", class,
  //            g_type_name (G_TYPE_FROM_CLASS (g_class)), class);

  scm_glib_gc_protect_object (class);
  guile_class->class = class;

  /* Not calling a class-init generic will prevent GOOPS classes that are
   * subclassed on the scheme side from being initialized, but that's a corner
   * case. Perhaps we should support it, but I'm removing it for now. */
  /* NOTE: The proper way for supporting class-init is to override initialize
   * for gtype-instance-class. */

  if (G_TYPE_IS_OBJECT (G_TYPE_FROM_CLASS (g_class))) {
    ((GObjectClass *) g_class)->get_property = scm_c_gobject_get_property;
    ((GObjectClass *) g_class)->set_property = scm_c_gobject_set_property;
  }
}

static void
scm_c_gtype_instance_class_init (gpointer g_class, gpointer class_data)
{
  scm_dynwind_guile_v__p_p (scm_with_guile,
                            scm_with_c_gtype_instance_class_init,
                            g_class, class_data);
}

SCM_DEFINE (scm_scheme_gclass_p, "scheme-gclass?", 1, 0, 0,
            (SCM class),
            "")
#define FUNC_NAME s_scm_scheme_gclass_p
{
  GType gtype;
  GObjectClass *gclass;

  SCM_VALIDATE_GOBJECT_CLASS_COPY (1, class, gtype);

  gclass = g_type_class_ref (gtype);
  return SCM_BOOL (gclass->get_property == scm_c_gobject_get_property);
}
#undef FUNC_NAME

// FIXME: remove?
SCM_DEFINE (scm_gtype_register_static, "gtype-register-static", 2, 0, 0,
            (SCM name, SCM parent_class),
            "Derive a new type named @var{name} from @var{parent_class}. "
            "Returns the new @code{<gtype-class>}. This function is called "
            "when deriving from @code{<gobject>}; users do not normally "
            "call this function directly.")
#define FUNC_NAME s_scm_gtype_register_static
{
  GType gtype_parent, gtype;
  GTypeInfo gtype_info;
  GTypeQuery gtype_query;
  GuileGTypeClass *guile_class;
  char *utf8;

  SCM_VALIDATE_STRING (1, name);
  SCM_VALIDATE_GTYPE_CLASS_COPY (2, parent_class, gtype_parent);

  scm_dynwind_begin (0);

  utf8 = scm_to_locale_string_dynwind (name);
  gtype = g_type_from_name (utf8);

  if (gtype)
    scm_c_gruntime_error (FUNC_NAME,
                          "There is already a type with this name: ~S",
                          SCM_LIST1 (name));

  if (!G_TYPE_IS_DERIVABLE (gtype_parent))
    scm_c_gruntime_error (FUNC_NAME,
                          "Cannot derive ~S from non-derivable parent type: ~S",
                          SCM_LIST2 (name, parent_class));

  if (!G_TYPE_IS_FUNDAMENTAL (gtype_parent) && !G_TYPE_IS_DEEP_DERIVABLE (gtype_parent))
    scm_c_gruntime_error (FUNC_NAME,
                          "Cannot derive ~S from non-fundamental parent type: ~S",
                          SCM_LIST2 (name, parent_class));

  g_type_query (gtype_parent, &gtype_query);

  memset (&gtype_info, 0, sizeof (gtype_info));
  gtype_info.class_size = gtype_query.class_size;
  gtype_info.instance_size = gtype_query.instance_size;
  gtype_info.class_init = scm_c_gtype_instance_class_init;
  gtype_info.instance_init = scm_c_gtype_instance_instance_init;

  gtype = g_type_register_static (gtype_parent, utf8, &gtype_info, 0);

  guile_class = g_new0 (GuileGTypeClass, 1);
  guile_class->properties_hash = g_hash_table_new (NULL, NULL);

  g_type_set_qdata (gtype, quark_guile_gtype_class, guile_class);

  scm_dynwind_end ();

  return scm_from_locale_string (g_type_name (gtype));
}
#undef FUNC_NAME

SCM_DEFINE (scm_gobject_class_get_properties, "gobject-class-get-properties", 1, 0, 0,
            (SCM class),
            "")
#define FUNC_NAME s_scm_gobject_class_get_properties
{
  gpointer gclass = 0;
  GParamSpec **properties;
  guint n_properties, i;
  GType gtype;
  SCM ret = SCM_EOL;

  SCM_VALIDATE_GTYPE_CLASS_COPY (1, class, gtype);

  if (G_TYPE_FUNDAMENTAL (gtype) == G_TYPE_OBJECT) {
    gclass = G_OBJECT_CLASS (g_type_class_ref (gtype));
    properties = g_object_class_list_properties (gclass, &n_properties);
  } else if (G_TYPE_FUNDAMENTAL (gtype) == G_TYPE_INTERFACE) {
    if (G_TYPE_IS_FUNDAMENTAL (gtype)) {
      properties = NULL;
      n_properties = 0;
    } else {
      gclass = g_type_default_interface_ref (gtype);
      properties = g_object_interface_list_properties (gclass, &n_properties);
    }
  } else {
    scm_wrong_type_arg (FUNC_NAME, 1, class);
  }

  for (i = n_properties; i > 0; i--)
    ret = scm_cons (scm_c_gtype_instance_to_scm (properties[i-1]),
                    ret);

  if (G_TYPE_FUNDAMENTAL (gtype) == G_TYPE_OBJECT)
    g_type_class_unref (gclass);
  else if (G_TYPE_FUNDAMENTAL (gtype) == G_TYPE_INTERFACE
           && !G_TYPE_IS_FUNDAMENTAL (gtype))
    g_type_default_interface_unref (gclass);

  g_free (properties);

  return ret;
}
#undef FUNC_NAME

SCM_DEFINE (scm_gobject_class_get_property_names, "gobject-class-get-property-names", 1, 0, 0,
            (SCM class),
            "")
#define FUNC_NAME s_scm_gobject_class_get_property_names
{
  gpointer gclass = 0;
  GParamSpec **properties;
  guint n_properties, i;
  GType gtype;
  SCM ret = SCM_EOL;

  SCM_VALIDATE_GTYPE_CLASS_COPY (1, class, gtype);

  g_debug ("gtype: %s", g_type_name (gtype));

  if (G_TYPE_FUNDAMENTAL (gtype) == G_TYPE_OBJECT) {
    gclass = G_OBJECT_CLASS (g_type_class_ref (gtype));
    properties = g_object_class_list_properties (gclass, &n_properties);
  } else if (G_TYPE_FUNDAMENTAL (gtype) == G_TYPE_INTERFACE) {
    if (G_TYPE_IS_FUNDAMENTAL (gtype)) {
      properties = NULL;
      n_properties = 0;
    } else {
      gclass = g_type_default_interface_ref (gtype);
      properties = g_object_interface_list_properties (gclass, &n_properties);
    }
  } else {
    scm_wrong_type_arg (FUNC_NAME, 1, class);
  }

  for (i = n_properties; i > 0; i--)
    ret = scm_cons (scm_from_locale_symbol (properties[i-1]->name), ret);

  if (G_TYPE_FUNDAMENTAL (gtype) == G_TYPE_OBJECT)
    g_type_class_unref (gclass);
  else if (G_TYPE_FUNDAMENTAL (gtype) == G_TYPE_INTERFACE
           && !G_TYPE_IS_FUNDAMENTAL (gtype))
    g_type_default_interface_unref (gclass);

  g_free (properties);

  return ret;
}
#undef FUNC_NAME

SCM_DEFINE (scm_gobject_class_install_property, "gobject-class-install-property", 2, 0, 0,
            (SCM class, SCM param),
            "")
#define FUNC_NAME s_scm_gobject_class_install_property
{
  GType gtype;
  GParamSpec *gparam;
  GObjectClass *gclass;
  GuileGTypeClass *guile_class;
  guint id;

  SCM_VALIDATE_GOBJECT_CLASS_COPY (1, class, gtype);
  SCM_VALIDATE_GPARAM_COPY (2, param, gparam);

  gclass = g_type_class_ref (gtype);
  if (g_object_class_find_property (gclass, gparam->name))
    scm_error (sym_gruntime_error, FUNC_NAME,
               "There is already a property with this name in class ~S: ~S",
               SCM_LIST2 (class, scm_from_locale_string (gparam->name)),
               SCM_EOL);

  guile_class = g_type_get_qdata (gtype, quark_guile_gtype_class);
  if (!guile_class)
    scm_error (sym_gruntime_error, FUNC_NAME,
               "Can't add properties to non-derived type: ~S",
               SCM_LIST1 (class), SCM_EOL);

  if (guile_class->first_instance_created)
    scm_error (sym_gruntime_error, FUNC_NAME,
               "Can't add properties after intances have been created: ~S",
               SCM_LIST1 (class), SCM_EOL);

  id = ++guile_class->last_property_id;
  g_object_class_install_property (gclass, id, gparam);

  //  DEBUG_ALLOC ("  protecting param %p of %s gparam %p", param,
  //            g_type_name (G_TYPE_FROM_INSTANCE (gparam)), gparam);
  g_hash_table_insert (guile_class->properties_hash, GINT_TO_POINTER (id),
                       scm_glib_gc_protect_object (param));

  return SCM_UNSPECIFIED;
}
#undef FUNC_NAME

SCM_DEFINE (scm_gobject_get_property, "get-property", 2, 0, 0,
            (SCM object, SCM name),
            "Gets a the property named @var{name} (a symbol) from @var{object}.")
#define FUNC_NAME s_scm_gobject_get_property
{
  GObject *gobject;
  GParamSpec *pspec;
  SCM retval;
  GValue value = { 0, };

  SCM_VALIDATE_GOBJECT_COPY (1, object, gobject);
  SCM_VALIDATE_SYMBOL (2, name);

  scm_dynwind_begin (0);

  pspec = g_object_class_find_property (G_OBJECT_GET_CLASS (gobject),
                                        scm_symbol_chars_dynwind (name));

  if (!pspec)
    scm_error (sym_gruntime_error, FUNC_NAME,
               "No such property ~S in class ~S",
               SCM_LIST2 (name, scm_class_of (object)), SCM_EOL);

  g_value_init (&value, pspec->value_type);
  g_object_get_property (gobject, pspec->name, &value);
  retval = scm_c_gvalue_ref (&value);
  g_value_unset (&value);

  scm_dynwind_end ();

  return retval;
}
#undef FUNC_NAME



SCM_DEFINE (scm_gobject_set_property, "set-property!", 3, 0, 0,
            (SCM object, SCM name, SCM value),
            "Sets the property named @var{name} (a symbol) on @var{object} to "
            "@var{init-value}.")
#define FUNC_NAME s_scm_gobject_set_property
{
  GObject *gobject;
  GParamSpec *pspec;
  GValue *gvalue;

  SCM_VALIDATE_GOBJECT_COPY (1, object, gobject);
  SCM_VALIDATE_SYMBOL (2, name);

  scm_dynwind_begin (0);

  pspec = g_object_class_find_property (G_OBJECT_GET_CLASS (gobject),
                                        scm_symbol_chars_dynwind (name));
  if (!pspec)
    scm_error (sym_gruntime_error, FUNC_NAME,
               "No such property ~S in class ~S",
               SCM_LIST2 (name, scm_class_of (object)), SCM_EOL);

  gvalue = scm_c_scm_to_gvalue (pspec->value_type, value);
  g_object_set_property (gobject, pspec->name, gvalue);
  g_value_unset (gvalue);

  scm_dynwind_end ();

  return SCM_UNSPECIFIED;
}
#undef FUNC_NAME

typedef struct {
  GType type;
  gpointer (* postmakefunc)(gpointer object);
} PostMakeFunc;

static GArray *post_make_funcs = NULL;

static inline void
post_make_object (GObject *obj)
{
  if (post_make_funcs) {
    gint i;

    for (i = 0; i < post_make_funcs->len; i++) {
      if (g_type_is_a (G_OBJECT_TYPE (obj),
                       g_array_index (post_make_funcs, PostMakeFunc, i).type)) {
        g_array_index (post_make_funcs, PostMakeFunc, i).postmakefunc (obj);
        //        DEBUG_ALLOC ("post-made gobject (%p) of type %s, ->%u",
        //            obj, g_type_name (G_TYPE_FROM_INSTANCE (obj)),
        //            obj->ref_count);
        break;
      }
    }
  }
}

/**
 * guile_gobject_register_postmakefunc:
 * type: the GType the sink function applies to.
 * postmakefunc: a function to remove the floating reference on an object.
 *
 * The lengths we go to to deal with broken APIs... This function is here to
 * deal with the case where the ref you get from g_object_new is not actually
 * your own, eg GtkWindow and GtkInvisible.
 *
 * The postmakefunc should be able to remove the floating reference on
 * instances of the given type, or any subclasses.
 */
void
scm_register_gobject_postmakefunc (GType type, gpointer (*postmakefunc) (gpointer))
{
  PostMakeFunc pmf;

  if (!post_make_funcs)
    post_make_funcs = g_array_new (FALSE, FALSE, sizeof(PostMakeFunc));

  pmf.type = type;
  pmf.postmakefunc = postmakefunc;
  g_array_append_val (post_make_funcs, pmf);
}

#ifdef DEBUG_REFCOUNTING
SCM_DEFINE (scm_sys_gobject_get_refcount, "gobject-get-refcount", 1, 0, 0,
            (SCM object),
            "Get the refcount of an object (for debugging purposes)")
#define FUNC_NAME s_scm_sys_gobject_get_refcount
{
  GObject *gobject;

  SCM_VALIDATE_GOBJECT_COPY (1, object, gobject);

  return scm_from_uint (gobject->ref_count);
}
#undef FUNC_NAME
#endif

SCM_DEFINE (scm_sys_gnome_gobject_object_post_init,
            "gobject-object-post-init", 0, 0, 0,
            (),
            "")
#define FUNC_NAME s_scm_sys_gnome_gobject_object_post_init
{
  _initialize = scm_permanent_object (SCM_VARIABLE_REF (scm_c_lookup ("initialize")));
  _gobject_get_property = scm_permanent_object (SCM_VARIABLE_REF (scm_c_lookup ("get-property")));
  _gobject_set_property = scm_permanent_object (SCM_VARIABLE_REF (scm_c_lookup ("set-property!")));
  scm_class_gobject = scm_permanent_object (SCM_VARIABLE_REF (scm_c_lookup ("<gobject>")));
  return SCM_UNSPECIFIED;
}
#undef FUNC_NAME

static void
sink_initially_unowned (gpointer i)
{
  GObject *object = i;
  if (g_object_is_floating (object))
    g_object_ref_sink (object);
}

void
scm_gobject_init (void)
{
#ifndef SCM_MAGIC_SNARFER
#include "gobject.x"
#endif
  scm_register_gtype_instance_funcs (&gobject_funcs);
  scm_c_register_gtype_instance_gvalue_wrappers (G_TYPE_OBJECT,
                                                 (SCMGValueGetTypeInstanceFunc) g_value_get_object,
                                                 (SCMGValueSetTypeInstanceFunc) g_value_set_object);
  scm_c_register_gtype_instance_gvalue_wrappers (G_TYPE_INTERFACE,
                                                 (SCMGValueGetTypeInstanceFunc) g_value_get_object,
                                                 (SCMGValueSetTypeInstanceFunc) g_value_set_object);

  _in_construction_from_scheme = scm_permanent_object (scm_make_fluid ());
  scm_fluid_set_x (_in_construction_from_scheme, SCM_EOL);

  scm_register_gtype_instance_sinkfunc (G_TYPE_INITIALLY_UNOWNED,
                                        sink_initially_unowned);

  _slot_definition_options =
    scm_variable_ref (scm_c_lookup ("slot-definition-options"));

  quark_guile_gtype_class = g_quark_from_static_string ("scm-guile-gtype-class");
}
