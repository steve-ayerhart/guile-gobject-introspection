#include <stdio.h>
#include <string.h>

#include "gc.h"
#include "gparameter.h"
#include "gvalue.h"
#include "support.h"

SCM scm_class_gparam;

static gpointer scm_c_gparam_construct (SCM instance, SCM initargs);
static void scm_c_gparam_initialize_scm (SCM instance, gpointer pspec);

static const scm_t_gtype_instance_funcs gparamspec_funcs = {
  G_TYPE_PARAM,
  (scm_t_gtype_instance_ref)g_param_spec_ref,
  (scm_t_gtype_instance_unref)g_param_spec_unref,
  (scm_t_gtype_instance_get_qdata)g_param_spec_get_qdata,
  (scm_t_gtype_instance_set_qdata)g_param_spec_set_qdata,
  (scm_t_gtype_instance_construct)scm_c_gparam_construct,
  (scm_t_gtype_instance_initialize_scm)scm_c_gparam_initialize_scm
};

SCM_SYMBOL (sym_name, "name");
SCM_SYMBOL (sym_nick, "nick");
SCM_SYMBOL (sym_blurb, "blurb");
SCM_SYMBOL (sym_flags, "flags");
SCM_SYMBOL (sym_value_type, "value-type");
SCM_SYMBOL (sym_minimum, "minimum");
SCM_SYMBOL (sym_maximum, "maximum");
SCM_SYMBOL (sym_default_value, "default-value");
SCM_SYMBOL (sym_object_type, "object-type");
SCM_SYMBOL (sym_boxed_type, "boxed-type");
SCM_SYMBOL (sym_enum_type, "enum-type");
SCM_SYMBOL (sym_flags_type, "flags-type");
SCM_SYMBOL (sym_element_spec, "element-spec");
SCM_SYMBOL (sym_is_a_type, "is-a-type");
SCM_SYMBOL (sym_gruntime_error, "gruntime-error");

static void scm_c_gparam_initialize_scm (SCM param, gpointer ppspec)
#define FUNC_NAME "gparam-initialize-scm"
{
  GParamSpec *pspec = ppspec;
  char *blurb;

#define SET(slot, val) scm_slot_set_x (param, sym_##slot, val)
  SET (name, scm_from_locale_symbol ((char *) g_param_spec_get_name (pspec)));
  SET (nick, scm_from_locale_string ((char *) g_param_spec_get_nick (pspec)));
  blurb = (char *) g_param_spec_get_blurb (pspec);
  SET (blurb, scm_from_locale_string (blurb ? blurb : ""));
  SET (flags, scm_from_uint (pspec->flags & ((1<<5)-1)));

  if (G_IS_PARAM_SPEC_BOOLEAN (pspec)) {
    GParamSpecBoolean *p = (GParamSpecBoolean *)pspec;
    SET (default_value, SCM_BOOL (p->default_value));
  }

  else if (G_IS_PARAM_SPEC_CHAR (pspec)) {
    GParamSpecChar *p = (GParamSpecChar *) pspec;
    SET (minimum, scm_from_char (p->minimum));
    SET (maximum, scm_from_char (p->maximum));
    SET (default_value, scm_from_char (p->default_value));
  }

  else if  (G_IS_PARAM_SPEC_UCHAR (pspec)) {
    GParamSpecUChar *p = (GParamSpecUChar *) pspec;
    SET (minimum, scm_from_uchar (p->minimum));
    SET (maximum, scm_from_uchar (p->maximum));
    SET (default_value, scm_from_uchar (p->default_value));
  }

  else if (G_IS_PARAM_SPEC_INT (pspec)) {
    GParamSpecInt *p = (GParamSpecInt *) pspec;
    SET (minimum, scm_from_int (p->minimum));
    SET (maximum, scm_from_int (p->maximum));
    SET (default_value, scm_from_int (p->default_value));
  }

  else if (G_IS_PARAM_SPEC_UINT (pspec)) {
    GParamSpecUInt *p = (GParamSpecUInt *) pspec;
    SET (minimum, scm_from_uint (p->minimum));
    SET (maximum, scm_from_uint (p->maximum));
    SET (default_value, scm_from_uint (p->default_value));
  }

  else if (G_IS_PARAM_SPEC_LONG (pspec)) {
    GParamSpecLong *p = (GParamSpecLong *) pspec;
    SET (minimum, scm_from_long (p->minimum));
    SET (maximum, scm_from_long (p->maximum));
    SET (default_value, scm_from_long (p->default_value));
  }

  else if (G_IS_PARAM_SPEC_ULONG (pspec)) {
    GParamSpecULong *p = (GParamSpecULong *) pspec;
    SET (minimum, scm_from_ulong (p->minimum));
    SET (maximum, scm_from_ulong (p->maximum));
    SET (default_value, scm_from_ulong (p->default_value));
  }

  else if (G_IS_PARAM_SPEC_INT64 (pspec)) {
    GParamSpecInt64 *p = (GParamSpecInt64 *) pspec;
    SET (minimum, scm_from_int64 (p->minimum));
    SET (maximum, scm_from_int64 (p->maximum));
    SET (default_value, scm_from_int64 (p->default_value));
  }

  else if (G_IS_PARAM_SPEC_UINT64 (pspec)) {
    GParamSpecUInt64 *p = (GParamSpecUInt64 *) pspec;
    SET (minimum, scm_from_uint64 (p->minimum));
    SET (maximum, scm_from_uint64 (p->maximum));
    SET (default_value, scm_from_uint64 (p->default_value));
  }

  else if (G_IS_PARAM_SPEC_FLOAT (pspec)) {
    GParamSpecFloat *p = (GParamSpecFloat *) pspec;
    SET (minimum, scm_from_double (p->minimum));
    SET (maximum, scm_from_double (p->maximum));
    SET (default_value, scm_from_double (p->default_value));
  }

  else if (G_IS_PARAM_SPEC_DOUBLE (pspec)) {
    GParamSpecDouble *p = (GParamSpecDouble *) pspec;
    SET (minimum, scm_from_double (p->minimum));
    SET (maximum, scm_from_double (p->maximum));
    SET (default_value, scm_from_double (p->default_value));
  }

  else if (G_IS_PARAM_SPEC_POINTER (pspec)) { }

  else if (G_IS_PARAM_SPEC_STRING (pspec)) {
    GParamSpecString *s = (GParamSpecString *) pspec;
    SET (default_value,
         s->default_value ? scm_from_locale_string (s->default_value) : SCM_BOOL_F);
  }

  else if (G_IS_PARAM_SPEC_OBJECT (pspec)) {
    SET (object_type, scm_c_gtype_to_class (pspec->value_type));
  }

  else if (G_IS_PARAM_SPEC_BOXED (pspec)) {
    SET (boxed_type, scm_c_gtype_to_class (pspec->value_type));
  }

  else if (G_IS_PARAM_SPEC_ENUM (pspec)) {
    GParamSpecEnum *e = (GParamSpecEnum *) pspec;
    GType enum_type = G_TYPE_FROM_CLASS (e->enum_class);

    SET (enum_type, scm_c_gtype_to_class (enum_type));
    SET (default_value, scm_from_uint (e->default_value));
  }

  else if (G_IS_PARAM_SPEC_FLAGS (pspec)) {
    GParamSpecFlags *f = (GParamSpecFlags *) pspec;
    GType flags_type = G_TYPE_FROM_CLASS (f->flags_class);

    SET (flags_type, scm_c_gtype_to_class (flags_type));
    SET (default_value, scm_from_uint (f->default_value));
  }

  else if (G_IS_PARAM_SPEC_VALUE_ARRAY (pspec)) {
    GParamSpecValueArray *va = (GParamSpecValueArray *) pspec;

    SET (element_spec, (va->element_spec
                        ? scm_c_gtype_instance_to_scm (va->element_spec)
                        : SCM_BOOL_F));
  }

  else if (G_IS_PARAM_SPEC_UNICHAR (pspec)) {
    GParamSpecUnichar *p = (GParamSpecUnichar *) pspec;
    /* borken! */
    SET (default_value, scm_from_uint (p->default_value));
  }

  else if (G_IS_PARAM_SPEC_GTYPE (pspec)) {
    GParamSpecGType *p = (GParamSpecGType *) pspec;
    SET (is_a_type, ((p->is_a_type == G_TYPE_NONE)
                     ? SCM_BOOL_F : scm_c_gtype_to_class (p->is_a_type)));
  }

  else {
    g_warning ("param type not implemented: %s",
               g_type_name (G_TYPE_FROM_INSTANCE (pspec)));
    /* SCM_ERROR_NOT_YET_IMPLEMENTED (SCM_BOOL_F); */
  }
#undef SET
}
#undef FUNC_NAME

static gpointer
scm_c_gparam_construct (SCM instance, SCM initargs)
{
  GParamSpec *pspec = NULL;
  GParamFlags flags;
  GType param_type, param_gtype;
  char *name, *nick, *blurb;

  param_type = scm_c_gtype_class_to_gtype (scm_class_of (instance));

  scm_dynwind_begin (0);
#define REF(what) scm_slot_ref (instance, sym_##what)

  name = scm_to_locale_string (scm_symbol_to_string (REF (name)));
  if (SCM_NFALSEP (scm_slot_bound_p (instance, sym_nick)))
    nick = scm_to_locale_string (REF (nick));
  else
    nick = g_strdup (name);
  if (SCM_NFALSEP (scm_slot_bound_p (instance, sym_blurb)))
    blurb = scm_to_locale_string (REF (blurb));
  else
    blurb = g_strdup (nick);
  if (SCM_NFALSEP (scm_slot_bound_p (instance, sym_flags)))
    flags = scm_to_uint (REF (flags));
  else
    flags = G_PARAM_READWRITE;
  /* FIXME: free these? */

  if (param_type == G_TYPE_PARAM_BOOLEAN) {
    pspec = g_param_spec_boolean (name, nick, blurb,
                                  SCM_NFALSEP (REF (default_value)),
                                  flags);
  }

  else if (param_type == G_TYPE_PARAM_CHAR) {
    pspec = g_param_spec_char (name, nick, blurb,
                               scm_to_char (REF (minimum)),
                               scm_to_char (REF (maximum)),
                               scm_to_char (REF (default_value)),
                               flags);
  }

  else if  (param_type == G_TYPE_PARAM_UCHAR) {
    pspec = g_param_spec_uchar (name, nick, blurb,
                                scm_to_uchar (REF (minimum)),
                                scm_to_uchar (REF (maximum)),
                                scm_to_uchar (REF (default_value)),
                                flags);
  }

  else if (param_type == G_TYPE_PARAM_INT) {
    pspec = g_param_spec_int (name, nick, blurb,
                              scm_to_int (REF (minimum)),
                              scm_to_int (REF (maximum)),
                              scm_to_int (REF (default_value)),
                              flags);
  }

  else if (param_type == G_TYPE_PARAM_UINT) {
    pspec = g_param_spec_uint (name, nick, blurb,
                               scm_to_uint (REF (minimum)),
                               scm_to_uint (REF (maximum)),
                               scm_to_uint (REF (default_value)),
                               flags);
  }

  else if (param_type == G_TYPE_PARAM_LONG) {
    pspec = g_param_spec_long (name, nick, blurb,
                               scm_to_long (REF (minimum)),
                               scm_to_long (REF (maximum)),
                               scm_to_long (REF (default_value)),
                               flags);
  }

  else if (param_type == G_TYPE_PARAM_ULONG) {
    pspec = g_param_spec_ulong (name, nick, blurb,
                                scm_to_ulong (REF (minimum)),
                                scm_to_ulong (REF (maximum)),
                                scm_to_ulong (REF (default_value)),
                                flags);
  }

  else if (param_type == G_TYPE_PARAM_INT64) {
    pspec = g_param_spec_int64 (name, nick, blurb,
                                scm_to_int64 (REF (minimum)),
                                scm_to_int64 (REF (maximum)),
                                scm_to_int64 (REF (default_value)),
                                flags);
  }

  else if (param_type == G_TYPE_PARAM_UINT64) {
    pspec = g_param_spec_uint64 (name, nick, blurb,
                                 scm_to_uint64 (REF (minimum)),
                                 scm_to_uint64 (REF (maximum)),
                                 scm_to_uint64 (REF (default_value)),
                                 flags);
  }

  else if (param_type == G_TYPE_PARAM_FLOAT) {
    pspec = g_param_spec_float (name, nick, blurb,
                                scm_to_double (REF (minimum)),
                                scm_to_double (REF (maximum)),
                                scm_to_double (REF (default_value)),
                                flags);
  }

  else if (param_type == G_TYPE_PARAM_DOUBLE) {
    pspec = g_param_spec_double (name, nick, blurb,
                                 scm_to_double (REF (minimum)),
                                 scm_to_double (REF (maximum)),
                                 scm_to_double (REF (default_value)),
                                 flags);
  }

  else if (param_type == G_TYPE_PARAM_POINTER) {
    pspec = g_param_spec_pointer (name, nick, blurb, flags);
  }

  else if (param_type == G_TYPE_PARAM_STRING) {
    SCM val = REF (default_value);

    pspec = g_param_spec_string (name, nick, blurb,
                                 SCM_FALSEP (val) ? NULL : scm_to_locale_string (val),
                                 flags);
  }

  else if (param_type == G_TYPE_PARAM_OBJECT) {
    pspec = g_param_spec_object (name, nick, blurb,
                                 scm_c_gtype_class_to_gtype (REF (object_type)),
                                 flags);
  }

  else if (param_type == G_TYPE_PARAM_BOXED) {
    pspec = g_param_spec_boxed (name, nick, blurb,
                                scm_c_gtype_class_to_gtype (REF (boxed_type)),
                                flags);
  }

  else if (param_type == G_TYPE_PARAM_ENUM) {
    pspec = g_param_spec_enum (name, nick, blurb,
                               scm_c_gtype_class_to_gtype (REF (enum_type)),
                               scm_to_int (REF (default_value)),
                               flags);
  }

  else if (param_type == G_TYPE_PARAM_FLAGS) {
    pspec = g_param_spec_flags (name, nick, blurb,
                                scm_c_gtype_class_to_gtype (REF (flags_type)),
                                scm_to_int (REF (default_value)),
                                flags);
  }
  else if (param_type == G_TYPE_PARAM_VALUE_ARRAY) {
    pspec = g_param_spec_value_array (name, nick, blurb,
                                      scm_c_scm_to_gtype_instance_typed
                                      (REF (element_spec), G_TYPE_PARAM),
                                      flags);
  }
  else if (param_type == G_TYPE_PARAM_UNICHAR) {
    pspec = g_param_spec_unichar (name, nick, blurb,
                                  scm_to_uint (REF (default_value)),
                                  flags);
  }
  else if (param_type == G_TYPE_PARAM_GTYPE) {
    SCM type = REF (is_a_type);
    if (scm_is_true (type))
      param_gtype = scm_c_gtype_class_to_gtype (type);
    else
      param_gtype = G_TYPE_NONE;
    pspec = g_param_spec_gtype (name, nick, blurb, param_gtype, flags);
  }
  else {
    scm_c_gruntime_error ("gparam-construct",
                          "Can't create instance of ~A from initargs: ~A",
                          SCM_LIST2 (scm_class_of (instance), initargs));
  }

  //  DEBUG_ALLOC ("new guile-owned param spec %p of type %s",
  //            pspec, g_type_name (G_TYPE_FROM_INSTANCE (pspec)));

  scm_dynwind_end ();

  return pspec;
}

/* These next two functions are exactly equivalent to struct-ref and struct-set!
 * from guile, except that they don't check permissions or allocation, because
 * we know them, and, more crucially, they understand how to calculate n_fields
 * for the so-called "light structs". Should be unnecessary once the patches
 * submitted to guile-devel on 10 April 2008 are accepted. */
SCM_DEFINE (scm_sys_hacky_struct_ref, "hacky-struct-ref",
            2, 0, 0, (SCM handle, SCM pos),
            "")
#define FUNC_NAME s_scm_sys_hacky_struct_ref
{
  scm_t_bits * data;
  SCM layout;
  size_t layout_len;
  size_t p;
  scm_t_bits n_fields;

  SCM_VALIDATE_STRUCT (1, handle);

  layout = SCM_STRUCT_LAYOUT (handle);
  data = SCM_STRUCT_DATA (handle);
  p = scm_to_size_t (pos);

  layout_len = scm_c_symbol_length (layout);
  n_fields = layout_len / 2;

  SCM_ASSERT_RANGE(1, pos, p < n_fields);

  return SCM_PACK (data[p]);
}
#undef FUNC_NAME

SCM_DEFINE (scm_sys_hacky_struct_set_x, "hacky-struct-set!",
            3, 0, 0, (SCM handle, SCM pos, SCM val),
            "")
#define FUNC_NAME s_scm_sys_hacky_struct_set_x
{
  scm_t_bits * data;
  SCM layout;
  size_t layout_len;
  size_t p;
  scm_t_bits n_fields;

  SCM_VALIDATE_STRUCT (1, handle);

  layout = SCM_STRUCT_LAYOUT (handle);
  data = SCM_STRUCT_DATA (handle);
  p = scm_to_size_t (pos);

  layout_len = scm_c_symbol_length (layout);
  n_fields = layout_len / 2;

  SCM_ASSERT_RANGE(1, pos, p < n_fields);

  data[p] = SCM_UNPACK (val);

  return SCM_UNDEFINED;
}
#undef FUNC_NAME

void
scm_gobject_gparameter_init (void)
{
#ifndef SCM_MAGIC_SNARFER
#include "gparameter.x"
#endif

  scm_register_gtype_instance_funcs (&gparamspec_funcs);
  scm_c_register_gtype_instance_gvalue_wrappers (G_TYPE_PARAM,
                                                 (SCMGValueGetTypeInstanceFunc)g_value_get_param,
                                                 (SCMGValueSetTypeInstanceFunc)g_value_set_param);

  /* fixme: these names suck */
  scm_c_define ("gparameter:uint-max", scm_from_uint (G_MAXUINT));
  scm_c_define ("gparameter:int-min", scm_from_int (G_MININT));
  scm_c_define ("gparameter:int-max", scm_from_int (G_MAXINT));
  scm_c_define ("gparameter:ulong-max", scm_from_ulong (G_MAXULONG));
  scm_c_define ("gparameter:long-min", scm_from_long (G_MINLONG));
  scm_c_define ("gparameter:long-max", scm_from_long (G_MAXLONG));
  scm_c_define ("gparameter:uint64-max", scm_from_ulong_long (G_MAXUINT64));
  scm_c_define ("gparameter:int64-min", scm_from_long_long (G_MININT64));
  scm_c_define ("gparameter:int64-max", scm_from_long_long (G_MAXINT64));
  scm_c_define ("gparameter:float-max", scm_from_double (G_MAXFLOAT));
  scm_c_define ("gparameter:float-min", scm_from_double (G_MINFLOAT));
  scm_c_define ("gparameter:double-max", scm_from_double (G_MAXDOUBLE));
  scm_c_define ("gparameter:double-min", scm_from_double (G_MINDOUBLE));
  scm_c_define ("gparameter:byte-order", scm_from_uint (G_BYTE_ORDER));

}
