#include <stdio.h>
#include <string.h>

#include "gc.h"
#include "gvalue.h"
#include "gutil.h"
#include "gobject.h"
#include "support.h"

typedef struct {
  SCM (*wrap) (const GValue*);
  void (*unwrap) (SCM, GValue*);
} wrap_funcs;

typedef struct {
  SCMGValueGetTypeInstanceFunc getter;
  SCMGValueSetTypeInstanceFunc setter;
} gtype_instance_wrap_funcs;

static guint scm_c_scm_to_flags_value (GFlagsClass *flags_class, SCM value);
static gint scm_c_scm_to_enum_value (GEnumClass *enum_class, SCM value);

SCM scm_class_gvalue;
static SCM scm_allocate_instance;

SCM_SYMBOL (sym_primitive_value, "primitive-value");
SCM_SYMBOL (sym_closure, "closure");
SCM_KEYWORD (k_value, "value");

static GHashTable *gvalue_wrappers = NULL;
static GHashTable *gtype_instance_wrappers = NULL;

static void
scm_gvalue_struct_free (SCM object)
{
  GValue *value = (GValue *) SCM_STRUCT_DATA_REF (object, 0);

  if (value) {
    //    DEBUG_ALLOC ("freeing GValue %p", value);
    g_value_unset (value);
    scm_gc_free (value, sizeof (GValue), "gvalue");
  }
}

SCM_DEFINE (scm_sys_bless_gvalue_class, "bless-gvalue-class", 1, 0, 0,
            (SCM class),
            "")
{
  scm_t_bits *slots = SCM_STRUCT_DATA (class);
  scm_class_gvalue = scm_permanent_object (class);
  slots[scm_vtable_index_instance_finalize] = (scm_t_bits)scm_gvalue_struct_free;
  return SCM_UNSPECIFIED;
}

SCM_DEFINE (scm_sys_allocate_gvalue, "allocate-gvalue", 2, 0, 0,
            (SCM class, SCM instance),
            "")
{
  GValue *value;

  value = scm_gc_malloc (sizeof (GValue), "gvalue");
  value->g_type = 0;
  SCM_STRUCT_DATA (instance)[0] = (scm_t_bits)value;
  if (class != scm_class_gvalue) {
    GType type = scm_c_gtype_class_to_gtype (class);
    g_value_init (value, type);
    //    DEBUG_ALLOC ("Bound SCM %p to GValue %p (%s)", instance, value, g_type_name (type));
  } else {
    //    DEBUG_ALLOC ("Bound SCM %p to generic GValue %p", instance, value);
  }

  return SCM_UNSPECIFIED;
}

SCM
scm_c_make_gvalue (GType c_gtype)
{
  SCM scm_gvalue, scm_class;

  scm_class = scm_c_gtype_to_class (c_gtype);
  /* it's not a <gvalue> class; use the generic code */
  if (scm_is_false (scm_memq (scm_class_gvalue,
                              scm_class_precedence_list (scm_class))))
    scm_class = scm_class_gvalue;

  scm_gvalue = scm_call_2 (scm_allocate_instance, scm_class, SCM_EOL);

  /* generic code needs a bit of help.. */
  if (scm_class == scm_class_gvalue)
    g_value_init (scm_c_gvalue_peek_value (scm_gvalue), c_gtype);

  /* no need to `initialize' */
  return scm_gvalue;
}

/* assume that something on the stack will reference scm */
GValue*
scm_c_gvalue_peek_value (SCM scm)
#define FUNC_NAME "gvalue-peek-value"
{
  SCM_VALIDATE_GVALUE (1, scm);
  return (GValue*)SCM_STRUCT_DATA (scm)[0];
}
#undef FUNC_NAME

// get/set primitives

// not threadsafe
void
scm_c_register_gvalue_wrappers (GType type,
                                SCM (*wrap) (const GValue*),
                                void (*unwrap) (SCM, GValue*))
{
  wrap_funcs* w = g_new (wrap_funcs, 1);

  if (!gvalue_wrappers)
    gvalue_wrappers = g_hash_table_new (NULL, NULL);

  w->wrap = wrap;
  w->unwrap = unwrap;

  g_hash_table_insert (gvalue_wrappers, (gpointer)type, w);
}

void
scm_c_register_gtype_instance_gvalue_wrappers (GType type,
                                               SCMGValueGetTypeInstanceFunc getter,
                                               SCMGValueSetTypeInstanceFunc setter)
{
  gtype_instance_wrap_funcs* w = g_new (gtype_instance_wrap_funcs, 1);

  if (!gtype_instance_wrappers)
    gtype_instance_wrappers = g_hash_table_new (NULL, NULL);

  w->getter = getter;
  w->setter = setter;

  g_hash_table_insert (gtype_instance_wrappers, (gpointer)type, w);
}

SCM
scm_c_gvalue_ref (const GValue *gvalue)
#define FUNC_NAME "gvalue-ref"
{
  GType type, fundamental;

  type = G_VALUE_TYPE (gvalue);
  fundamental = G_TYPE_FUNDAMENTAL (type);

  switch (fundamental) {
  case G_TYPE_CHAR:
    return SCM_MAKE_CHAR (g_value_get_schar (gvalue));

  case G_TYPE_UCHAR:
    return SCM_MAKE_CHAR (g_value_get_uchar (gvalue));

  case G_TYPE_BOOLEAN:
    return SCM_BOOL (g_value_get_boolean (gvalue));

  case G_TYPE_INT:
    return scm_from_int (g_value_get_int (gvalue));

  case G_TYPE_UINT:
    return scm_from_uint (g_value_get_uint (gvalue));

  case G_TYPE_LONG:
    return scm_from_long (g_value_get_long (gvalue));

  case G_TYPE_ULONG:
    return scm_from_ulong (g_value_get_ulong (gvalue));

  case G_TYPE_INT64:
    return scm_from_long_long (g_value_get_int64 (gvalue));

  case G_TYPE_UINT64:
    return scm_from_ulong_long (g_value_get_uint64 (gvalue));

  case G_TYPE_FLOAT:
    return scm_from_double ((double) g_value_get_float (gvalue));

  case G_TYPE_DOUBLE:
    return scm_from_double (g_value_get_double (gvalue));

  case G_TYPE_STRING:
    {
      const char *s = g_value_get_string (gvalue);
      return s ? scm_from_locale_string (s) : SCM_BOOL_F;
    }

  default:
    {
      gtype_instance_wrap_funcs* w1;
      wrap_funcs* w2;
      if ((w1 = g_hash_table_lookup (gtype_instance_wrappers,
                                     (gpointer)fundamental))) {
        return scm_c_gtype_instance_to_scm (w1->getter (gvalue));
      } else if ((w2 = g_hash_table_lookup (gvalue_wrappers,
                                            (gpointer)type))) {
        return w2->wrap (gvalue);
      } else {
        SCM ret = scm_c_make_gvalue (type);

        /* Enums and flags are natively represented as GValues. Boxed
         * and pointer values also fall through here, unless there is a
         * custom wrapper registered. */
        g_value_copy (gvalue, scm_c_gvalue_peek_value (ret));
        return ret;
      }
    }
  }
}
#undef FUNC_NAME

void
scm_c_gvalue_set (GValue *gvalue, SCM value)
#define FUNC_NAME "gvalue-set!"
{
  GType gtype, fundamental;

  gtype = G_VALUE_TYPE (gvalue);
  fundamental = G_TYPE_FUNDAMENTAL (gtype);

  if (SCM_GVALUEP (value)) {
    if (g_type_is_a (scm_c_gtype_class_to_gtype (scm_class_of (value)),
                     gtype)) {
      GValue *v = scm_c_gvalue_peek_value (value);
      g_value_copy (v, gvalue);
      return;
    } else {
      scm_c_gruntime_error (FUNC_NAME, "Can't make ~a into ~a",
                            SCM_LIST2 (value, scm_c_gtype_to_class (gtype)));
      return;
    }
  }

  switch (fundamental) {
  case G_TYPE_CHAR:
    if (SCM_CHARP (value))
      g_value_set_schar (gvalue, SCM_CHAR (value));
    else
      g_value_set_schar (gvalue, scm_to_int8 (value));
    break;

  case G_TYPE_UCHAR:
    if (SCM_CHARP (value))
      g_value_set_uchar (gvalue, SCM_CHAR (value));
    else
      g_value_set_uchar (gvalue, scm_to_uint8 (value));
    break;

  case G_TYPE_BOOLEAN:
    SCM_VALIDATE_BOOL (2, value);
    g_value_set_boolean (gvalue, SCM_NFALSEP (value));
    break;

  case G_TYPE_INT:
    g_value_set_int (gvalue, scm_to_int (value));
    break;

  case G_TYPE_UINT:
    g_value_set_uint (gvalue, scm_to_uint (value));
    break;

  case G_TYPE_LONG:
    g_value_set_long (gvalue, scm_to_long (value));
    break;

  case G_TYPE_ULONG:
    g_value_set_ulong (gvalue, scm_to_ulong (value));
    break;

  case G_TYPE_INT64:
    g_value_set_int64 (gvalue, scm_to_int64 (value));
    break;

  case G_TYPE_UINT64:
    g_value_set_uint64 (gvalue, scm_to_uint64 (value));
    break;

  case G_TYPE_FLOAT: {
    double x = scm_to_double (value);
    SCM_ASSERT_RANGE (2, value, (- G_MAXFLOAT < x) && (x < G_MAXFLOAT));
    g_value_set_float (gvalue, (float) x);
    break;
  }

  case G_TYPE_DOUBLE:
    g_value_set_double (gvalue, scm_to_double (value));
    break;

  case G_TYPE_STRING:
    SCM_ASSERT (scm_is_string (value) || SCM_FALSEP (value),
                value, SCM_ARG2, FUNC_NAME);
    if (SCM_FALSEP (value))
      g_value_set_string (gvalue, NULL);
    else
      g_value_take_string (gvalue, scm_to_locale_string (value));
    break;

  case G_TYPE_ENUM: {
    GEnumClass *enum_class = g_type_class_ref (G_VALUE_TYPE (gvalue));
    g_value_set_enum (gvalue, scm_c_scm_to_enum_value (enum_class, value));
    g_type_class_unref (enum_class);
    break;
  }

  case G_TYPE_FLAGS: {
    GFlagsClass *flags_class = g_type_class_ref (G_VALUE_TYPE (gvalue));
    g_value_set_flags (gvalue, scm_c_scm_to_flags_value (flags_class, value));
    g_type_class_unref (flags_class);
    break;
  }

  default:
    {
      gtype_instance_wrap_funcs *w;
      w = g_hash_table_lookup (gtype_instance_wrappers,
                               (gpointer)fundamental);

      if (w) {
        if (SCM_FALSEP (value)) {
          w->setter (gvalue, NULL);
        } else {
          gpointer ginstance;

          SCM_VALIDATE_GTYPE_INSTANCE_TYPE_COPY (2, value,
                                                 G_VALUE_TYPE (gvalue),
                                                 ginstance);

          w->setter (gvalue, ginstance);
        }
        break;
      }
    }
    {
      wrap_funcs *w;
      w = g_hash_table_lookup (gvalue_wrappers, (gpointer)gtype);

      if (w) {
        w->unwrap (value, gvalue);
        break;
      }
    }

    scm_c_gruntime_error (FUNC_NAME,
                          "Don't know how to make values of type ~A",
                          SCM_LIST1 (scm_c_gtype_to_class (gtype)));
  }
}
#undef FUNC_NAME
static gint
scm_c_scm_to_enum_value (GEnumClass *enum_class, SCM value)
#define FUNC_NAME "scm->enum-value"
{
  guint i;

#define ERROR(x)                                                    \
  scm_c_gruntime_error                                              \
    (FUNC_NAME, "Bad enum value for enumerated type `~a': ~a",      \
     SCM_LIST2 (scm_from_locale_string                              \
                (g_type_name (G_TYPE_FROM_CLASS (enum_class))), x))

  if (scm_is_signed_integer (value, SCM_T_INT32_MIN, SCM_T_INT32_MAX)) {
    gint v = scm_to_int (value);
    for (i = 0; i < enum_class->n_values; i++)
      if (enum_class->values[i].value == v)
        return v;
    ERROR (value);
  } else if (scm_is_symbol (value)) {
    char *v = scm_symbol_chars (value);
    for (i = 0; i < enum_class->n_values; i++)
      if (strcmp (enum_class->values[i].value_nick, v) == 0) {
        free (v);
        return enum_class->values[i].value;
      }
    free (v);
    ERROR (value);
  } else if (scm_is_string (value)) {
    char *v = scm_to_locale_string (value);
    for (i = 0; i < enum_class->n_values; i++)
      if (strcmp (enum_class->values[i].value_name, v) == 0) {
        free (v);
        return enum_class->values[i].value;
      }
    free (v);
    ERROR (value);
  }
  ERROR (value);
  return 0; /* not reached */
#undef ERROR
}
#undef FUNC_NAME

SCM_DEFINE (scm_genum_to_value, "genum->value", 1, 0, 0,
            (SCM value),
            "Convert the enumerated value @var{obj} from a @code{<gvalue>} to "
            "its representation as an integer.")
#define FUNC_NAME s_scm_genum_to_value
{
  SCM_ASSERT (scm_c_gvalue_holds (value, G_TYPE_ENUM), value, SCM_ARG1,
              FUNC_NAME);

  return scm_from_int (g_value_get_enum (scm_c_gvalue_peek_value (value)));
}
#undef FUNC_NAME

static guint
scm_c_scm_to_flags_value (GFlagsClass *flags_class, SCM value)
#define FUNC_NAME "scm->flags-value"
{
#define ERROR(x)                                                      \
  scm_c_gruntime_error                                                \
    (FUNC_NAME, "Bad value for flags type `~a': ~a",                  \
     SCM_LIST2 (scm_from_locale_string                                \
                (g_type_name (G_TYPE_FROM_CLASS (flags_class))), x))

  if (scm_is_unsigned_integer (value, 0, SCM_T_UINT32_MAX)) {
    guint v = scm_to_uint (value);
    if ((v & flags_class->mask) == v)
      return v;
    ERROR (value);
    return 0; /* not reached */
  } else {
    guint ret = 0;
    guint i;
    SCM s;

    if (!scm_is_true (scm_list_p (value))) {
      if (scm_is_symbol (value) || scm_is_string (value))
        value = scm_list_1 (value);
      else
        ERROR (value);
    }

    for (; !scm_is_null (value); value = scm_cdr (value)) {
      s = scm_car (value);
      if (scm_is_unsigned_integer (s, 0, SCM_T_UINT32_MAX)) {
        guint v = scm_to_uint (s);
        for (i = 0; i < flags_class->n_values; i++)
          if (flags_class->values[i].value == v) {
            ret |= v;
            break;
          }
        if (i == flags_class->n_values)
          ERROR (s);
      } else if (scm_is_symbol (s)) {
        char *v = scm_symbol_chars (s);
        for (i = 0; i < flags_class->n_values; i++)
          if (strcmp (flags_class->values[i].value_nick, v) == 0) {
            ret |= flags_class->values[i].value;
            break;
          }
        free (v);
        if (i == flags_class->n_values)
          ERROR (s);
      } else if (scm_is_string (s)) {
        char *v = scm_to_locale_string (s);
        for (i = 0; i < flags_class->n_values; i++)
          if (strcmp (flags_class->values[i].value_name, v) == 0) {
            ret |= flags_class->values[i].value;
            break;
          }
        free (v);
        if (i == flags_class->n_values)
          ERROR (s);
      } else {
        ERROR (s);
      }
    }
    return ret;
  }
#undef ERROR
}
#undef FUNC_NAME

SCM_DEFINE (scm_gflags_to_value, "gflags->value", 1, 0, 0,
            (SCM value),
            "Convert the flags value @var{obj} from a @code{<gvalue>} to "
            "its representation as an integer.")
#define FUNC_NAME s_scm_gflags_to_value
{
  SCM_ASSERT (scm_c_gvalue_holds (value, G_TYPE_FLAGS), value, SCM_ARG1, FUNC_NAME);

  return scm_from_int (g_value_get_flags (scm_c_gvalue_peek_value (value)));
}
#undef FUNC_NAME

SCM_DEFINE (scm_sys_gvalue_set_x, "gvalue-set!", 2, 0, 0,
            (SCM instance, SCM value), "")
{
  scm_c_gvalue_set (scm_c_gvalue_peek_value (instance), value);

  return SCM_UNSPECIFIED;
}

// scm <-> GValue*
SCM
scm_c_gvalue_to_scm (const GValue *gvalue)
{
  return scm_c_gvalue_ref (gvalue);
}

GValue*
scm_c_scm_to_gvalue (GType gtype, SCM scm)
{
  GValue *new = g_new0 (GValue, 1);

  g_value_init (new, gtype);
  scm_c_gvalue_set (new, scm);
  return new;
}

SCM_DEFINE (scm_gvalue_to_scm, "gvalue->scm", 1, 0, 0,
            (SCM value),
            "Convert a @code{<gvalue>} into it normal scheme representation, "
            "for example unboxing characters into Scheme characters. Note "
            "that the Scheme form for some values is the @code{<gvalue>} "
            "form, for example with boxed or enumerated values.")
{
  /* FIXME: needlessly creates a new value in the e.g. boxed case */
  GValue *v = scm_c_gvalue_peek_value (value);
  return scm_c_gvalue_ref (v);
}

SCM_DEFINE (scm_scm_to_gvalue, "scm->gvalue", 2, 0, 0,
            (SCM class, SCM scm),
            "Convert a Scheme value into a @code{<gvalue>} of type "
            "@var{class}. If the conversion is not possible, raise a "
            "@code{gruntime-error}.")
{
  SCM ret;
  GValue *gvalue;

  /* fixme the noop case */
  ret = scm_c_make_gvalue (scm_c_gtype_class_to_gtype (class));
  gvalue = scm_c_gvalue_peek_value (ret);
  scm_c_gvalue_set (gvalue, scm);

  return ret;
}

// scm wrappers for some boxed types
static gpointer
copy_gboxed_scm (gpointer boxed)
{
  //DEBUG_ALLOC (G_STRLOC ": copying gboxed %p", boxed);
  scm_glib_gc_protect_object ((SCM) boxed);
  return boxed;
}

static void
free_gboxed_scm (gpointer boxed)
{
  //  DEBUG_ALLOC (G_STRLOC ": freeing gboxed %p", boxed);
  scm_glib_gc_unprotect_object (boxed);
}

GType
gboxed_scm_get_type (void)
{
  static GType boxed_type = 0;

  if (!boxed_type)
    boxed_type = g_boxed_type_register_static ("GBoxedSCM",
                                               copy_gboxed_scm,
                                               free_gboxed_scm);

  return boxed_type;
}

SCM
wrap_boxed_scm (const GValue *value)
{
  gpointer val = g_value_get_boxed (value);
  return val ? GPOINTER_TO_SCM (g_value_get_boxed (value)) : SCM_UNSPECIFIED;
}

void
unwrap_boxed_scm (SCM scm, GValue *value)
{
  g_value_set_boxed (value, SCM_TO_GPOINTER (scm));
}

GType
garray_scm_get_type (void)
{
  static GType boxed_type =0;

  if (!boxed_type)
    boxed_type = g_boxed_type_register_static ("GArraySCM",
                                               copy_gboxed_scm,
                                               free_gboxed_scm);
  return boxed_type;
}

SCM
wrap_gvalue_array (const GValue *value)
{
  GValueArray *arr = g_value_get_boxed (value);
  gint i = arr ? arr->n_values : 0;
  SCM l = SCM_EOL;

  while (i--)
    l = scm_cons (scm_c_gvalue_ref (&arr->values[i]), l);
  return l;
}

void
unwrap_gvalue_array (SCM scm, GValue *value)
#define FUNC_NAME "unwrap-gvalue-array"
{
  GArray *arr;
  gint len;

  SCM_ASSERT (SCM_BOOL (scm_list_p (scm)), scm, SCM_ARG2, FUNC_NAME);

  len = scm_ilength (scm);
  arr = g_array_sized_new (FALSE, FALSE, sizeof (GValue *), len);
  while (len--) {
    GType value_type;
    SCM v;

    v = SCM_CAR (scm);

    if (SCM_GVALUEP (v))
      value_type = G_VALUE_TYPE (scm_c_gvalue_peek_value (v));
    if (scm_is_string (v))
      value_type = G_TYPE_STRING;
    else if (SCM_BOOLP (v))
      value_type = G_TYPE_BOOLEAN;
    else if (scm_is_signed_integer (v, SCM_T_INT32_MIN, SCM_T_INT32_MAX))
      value_type = G_TYPE_INT;
    else if (SCM_REALP (v))
      value_type = G_TYPE_DOUBLE;
    else if (SCM_CHARP (v))
      value_type = G_TYPE_CHAR;
    else if (SCM_GOBJECTP (v)) {
      GObject *gobject;
      SCM_VALIDATE_GOBJECT_COPY (1, v, gobject);
      value_type = G_OBJECT_TYPE (gobject);
    }
    else if (SCM_BOOL (scm_list_p (v)))
      value_type = G_TYPE_BOXED;
    else
      scm_wrong_type_arg (FUNC_NAME, SCM_ARG2, v);

    {
      GValue tmp = { 0, };
      g_value_init (&tmp, value_type);
      scm_c_gvalue_set (&tmp, v);
      /* copies the val */
      g_array_append_val (arr, tmp);
      g_value_unset (&tmp);
    }
    scm = SCM_CDR (scm);
  }

  g_value_take_boxed (value, arr);
}
#undef FUNC_NAME

// enum and flags

SCM_DEFINE (scm_genum_register_static, "genum-register-static", 2, 0, 0,
            (SCM name, SCM vtable),
            "Creates and registers a new enumerated type with name @var{name} with the C runtime. "
            "There must be no type with name @var{name} when this function is called.\n\n"
            "The new type can be accessed by using @code{gtype-name->class}.\n\n"
            "@var{vtable} is a vector describing the new enum type. Each vector element describes "
            "one enum element and must be a list of 3 elements: the element's nick name as a symbol, "
            "its name as a string, and its integer value.\n\n"
            "@lisp\n"
            "(genum-register-static \"Test\"\n"
            "  #((foo \"Foo\" 1) (bar \"Bar\" 2) (baz \"Long name of baz\" 4)))\n"
            "@end lisp\n")
#define FUNC_NAME s_scm_genum_register_static
{
  size_t length, i;
  GEnumValue *values;
  GType type;

  SCM_VALIDATE_STRING (1, name);
  SCM_VALIDATE_VECTOR (2, vtable);

  scm_dynwind_begin (0);

  type = g_type_from_name (scm_to_locale_string_dynwind (name));

  if (type)
    scm_c_gruntime_error (FUNC_NAME,
                          "There is already a type with this name: ~S",
                          SCM_LIST1 (name));

  length = scm_c_vector_length (vtable);

  for (i = 0; i < length; i++) {
    SCM this = scm_c_vector_ref (vtable, i);

    SCM_ASSERT ((scm_ilength (this) == 3) &&
                SCM_SYMBOLP (scm_car (this)) &&
                scm_is_string (scm_cadr (this)) &&
                scm_is_signed_integer (scm_caddr (this),
                                       SCM_T_INT32_MIN, SCM_T_INT32_MAX),
                vtable, SCM_ARG2, FUNC_NAME);
  }

  values = g_new0 (GEnumValue, length + 1);

  for (i = 0; i < length; i++) {
    SCM this = scm_c_vector_ref (vtable, i);

    values [i].value_nick  = scm_symbol_chars (scm_car (this));
    values [i].value_name  = scm_to_locale_string (scm_cadr (this));
    values [i].value       = scm_to_int (scm_caddr (this));
  }

  type = g_enum_register_static (scm_to_locale_string_dynwind (name), values);

  scm_dynwind_end ();

  return SCM_UNSPECIFIED;
}
#undef FUNC_NAME

SCM_DEFINE (scm_gflags_register_static, "gflags-register-static", 2, 0, 0,
            (SCM name, SCM vtable),
            "Creates and registers a new flags @code{<gtype-class>} with name "
            "@var{name} with the C runtime.\n\n"
            "The @var{vtable} should be in the format described in the "
            "documentation for @code{genum-register-static}.")
#define FUNC_NAME s_scm_gflags_register_static
{
  size_t length, i;
  GFlagsValue *values;
  GType type;

  SCM_VALIDATE_STRING (1, name);
  SCM_VALIDATE_VECTOR (2, vtable);

  scm_dynwind_begin (0);

  type = g_type_from_name (scm_to_locale_string_dynwind (name));
  if (type)
    scm_c_gruntime_error (FUNC_NAME,
                          "There is already a type with this name: ~S",
                          SCM_LIST1 (name));

  length = scm_c_vector_length (vtable);

  for (i = 0; i < length; i++) {
    SCM this = scm_c_vector_ref (vtable, i);

    SCM_ASSERT ((scm_ilength (this) == 3) &&
                SCM_SYMBOLP (scm_car (this)) &&
                scm_is_string (scm_cadr (this)) &&
                scm_is_unsigned_integer (scm_caddr (this),
                                         0, SCM_T_UINT32_MAX),
                vtable, SCM_ARG2, FUNC_NAME);
  }

  values = g_new0 (GFlagsValue, length + 1);

  for (i = 0; i < length; i++) {
    SCM this = scm_c_vector_ref (vtable, i);

    values [i].value_nick  = scm_symbol_chars (scm_car (this));
    values [i].value_name  = scm_to_locale_string (scm_cadr (this));
    values [i].value       = scm_to_uint (scm_caddr (this));
  }

  type = g_flags_register_static (scm_to_locale_string_dynwind (name), values);

  scm_dynwind_end ();

  return SCM_UNSPECIFIED;
}
#undef FUNC_NAME
SCM_DEFINE (scm_genum_class_to_value_table, "genum-class->value-table", 1, 0, 0,
            (SCM class),
            "Return a table of the values supported by the enumerated "
            "@code{<gtype-class>} @var{class}. The return value will be in the "
            "format described in @code{genum-register-static}.")
#define FUNC_NAME s_scm_genum_class_to_value_table
{
  GType gtype;
  GEnumClass *enum_class;
  SCM vector;
  guint i;

  SCM_VALIDATE_GTYPE_CLASS_IS_A (1, class, G_TYPE_ENUM, gtype);

  enum_class = g_type_class_ref (gtype);

  vector = scm_c_make_vector (enum_class->n_values, SCM_UNDEFINED);

  for (i = 0; i < enum_class->n_values; i++) {
    GEnumValue *current = &enum_class->values [i];
    SCM this;

    this = scm_list_3 (scm_from_locale_symbol (current->value_nick),
                       scm_from_locale_string (current->value_name),
                       scm_from_int (current->value));

    scm_c_vector_set_x (vector, i, this);
  }

  g_type_class_unref (enum_class);

  return vector;
}
#undef FUNC_NAME

SCM_DEFINE (scm_gflags_class_to_value_table, "gflags-class->value-table", 1, 0, 0,
            (SCM class),
            "Return a table of the values supported by the flag "
            "@code{<gtype-class>} @var{class}. The return value will be in the "
            "format described in @code{gflags-register-static}.")
#define FUNC_NAME s_scm_gflags_class_to_value_table
{
  GType gtype;
  GFlagsClass *flags_class;
  SCM vector;
  guint i;

  SCM_VALIDATE_GTYPE_CLASS_IS_A (1, class, G_TYPE_FLAGS, gtype);

  flags_class = g_type_class_ref (gtype);

  vector = scm_c_make_vector (flags_class->n_values, SCM_UNDEFINED);

  for (i = 0; i < flags_class->n_values; i++) {
    GFlagsValue *current = &flags_class->values [i];
    SCM this;

    this = scm_list_3 (scm_from_locale_symbol (current->value_nick),
                       scm_from_locale_string (current->value_name),
                       scm_from_uint (current->value));

    scm_c_vector_set_x (vector, i, this);
  }

  g_type_class_unref (flags_class);

  return vector;
}
#undef FUNC_NAME

gboolean
scm_c_gvalue_holds (SCM maybe_gvalue, GType type)
{
  return (SCM_GVALUEP (maybe_gvalue)
          && G_TYPE_CHECK_VALUE_TYPE (scm_c_gvalue_peek_value (maybe_gvalue),
                                      type));
}

SCM
scm_c_gvalue_new_from_boxed (GType type, const gpointer boxed)
{
  SCM ret = scm_c_make_gvalue (type);
  g_value_set_boxed (scm_c_gvalue_peek_value (ret), boxed);
  return ret;
}

SCM
scm_c_gvalue_new_take_boxed (GType type, gpointer boxed)
{
  SCM ret = scm_c_make_gvalue (type);
  g_value_take_boxed (scm_c_gvalue_peek_value (ret), boxed);
  return ret;
}

gpointer
scm_c_gvalue_peek_boxed (SCM value)
{
  return g_value_get_boxed (scm_c_gvalue_peek_value (value));
}

gpointer
scm_c_gvalue_dup_boxed (SCM value)
{
  return g_value_dup_boxed (scm_c_gvalue_peek_value (value));
}

GValue*
scm_c_gvalue_dup_value (SCM scm)
{
  GValue *ret = g_new0 (GValue, 1);
  g_value_init (ret, G_VALUE_TYPE (scm_c_gvalue_peek_value (scm)));
  g_value_copy (scm_c_gvalue_peek_value (scm), ret);
  return ret;
}

SCM
scm_c_gvalue_from_value (const GValue *value)
{
  SCM ret = scm_c_make_gvalue (G_VALUE_TYPE (value));
  g_value_copy (value, scm_c_gvalue_peek_value (ret));
  return ret;
}

SCM
scm_c_gvalue_take_value (GValue *value)
{
  /* erm... suboptimal :) */
  SCM ret;
  GValue *trash;
  ret = scm_c_make_gvalue (G_VALUE_TYPE (value));
  g_return_val_if_fail (value != NULL, ret);
  trash = (GValue*)SCM_STRUCT_DATA (ret)[0];
  SCM_STRUCT_DATA (ret)[0] = (scm_t_bits)value;
  g_free (trash);
  return ret;
}

void
scm_gobject_gvalue_init (void)
{
#ifndef SCM_MAGIC_SNARFER
#include "gvalue.x"
#endif
  scm_allocate_instance = scm_permanent_object (SCM_VARIABLE_REF (scm_c_lookup ("allocate-instance")));

  scm_c_register_gvalue_wrappers (G_TYPE_BOXED_SCM, wrap_boxed_scm, unwrap_boxed_scm);
  scm_c_register_gvalue_wrappers (G_TYPE_GARRAY_SCM, wrap_gvalue_array, unwrap_gvalue_array);
}
