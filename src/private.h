#ifndef __GUILE_GOBJECT_PRIVATE_H__
#define __GUILE_GOBJECT_PRIVATE_H__

#define SCM_DEFINE_STATIC(FNAME, PRIMNAME, REQ, OPT, VAR, ARGLIST)      \
  SCM_SNARF_HERE(static const char s_ ## FNAME [] = PRIMNAME; static SCM FNAME ARGLIST) \
  SCM_SNARF_INIT(scm_c_define_gsubr (s_ ## FNAME, REQ, OPT, VAR, (SCM_FUNC_CAST_ARBITRARY_ARGS) FNAME);) \
  SCM_SNARF_DOCS(primitive, FNAME, PRIMNAME, ARGLIST, REQ, OPT, VAR, "")

typedef struct _GuileGTypeClass GuileGTypeClass;
struct _GuileGTypeClass {
  /*< private >*/
  GHashTable *properties_hash;

  guint last_property_id;
  gboolean first_instance_created;

  SCM class;
};

SCM scm_c_gtype_instance_to_scm_typed (gpointer ginstance, GType type);
void scm_c_gtype_instance_bind_to_object (gpointer ginstance, SCM object);

G_END_DECLS

#endif
