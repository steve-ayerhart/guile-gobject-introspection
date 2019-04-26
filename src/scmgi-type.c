/* -*- Mode: C; c-basic-offset: 4 -*-
 * vim: tabstop=4 shiftwidth=4 expandtab
 */

//#include <config.h>

#include "scmgobject-object.h"
#include "scmgi-type.h"

// __gtype__ objects

SCM_GLOBAL_SYMBOL (scm_sym_gtype, "g-type");

typedef struct {
    GType type;
} ScmGTypeWrapper;

static SCM scm_g_type_class;
static SCM scm_unknown_class;

// TODO: move these to the right files
extern GQuark scmginterface_type_key;
extern GQuark scmgenum_class_key;
extern GQuark scmgflags_class_key;
extern GQuark scmgpointer_class_key;
extern GQuark scmgboxed_class_key;
extern GQuark scmgobject_class_key;

GQuark scmginterface_type_key;
GQuark scmgenum_class_key;
GQuark scmgflags_class_key;
GQuark scmgpointer_class_key;
GQuark scmgboxed_class_key;
GQuark scmgobject_class_key;

static GQuark
_scmgi_type_key (GType type)
{
    GQuark key;

    if (g_type_is_a (type, G_TYPE_INTERFACE))
        key = scmginterface_type_key;
    else if (g_type_is_a (type, G_TYPE_ENUM))
        key = scmgenum_class_key;
    else if (g_type_is_a (type, G_TYPE_FLAGS))
        key = scmgflags_class_key;
    else if (g_type_is_a (type, G_TYPE_POINTER))
        key = scmgpointer_class_key;
    else if (g_type_is_a (type, G_TYPE_BOXED))
        key = scmgboxed_class_key;
    else
        key = scmgobject_class_key;

    return key;
}

SCM_DEFINE (scm_g_type_display, "%g-type-display", 1, 0, 0,
            (SCM scm_gtype),
            "")
{
    SCM scm_display;
    GType gtype;

    gtype = ((ScmGTypeWrapper *) scm_foreign_object_ref (scm_gtype, 0))->type;
    const gchar *name = g_type_name (gtype);

    return scm_simple_format (SCM_BOOL_F,
                              scm_from_locale_string ("<g-type ~s (~d)>"),
                              scm_list_2 (scm_from_locale_string (name ? name : "invalid"),
                                          scm_from_ulong (gtype)));
}

SCM_DEFINE (scmgi_g_type_get_goopstype, "%g-type-get-goops-type", 1, 0, 0,
            (SCM scm_gtype),
            "")
{
    GQuark type_key;
    SCM scm_type;
    GType gtype;

    gtype = ((ScmGTypeWrapper *) scm_foreign_object_ref (scm_gtype, 0))->type;

    type_key = _scmgi_type_key (gtype);

    scm_type = g_type_get_qdata (gtype, type_key);
    if (!scm_type)
        scm_type = SCM_UNDEFINED;

    return scm_type;
}
/*

SCM_DEFINE (scmgi_g_type_set_goopstype, "%g-type-set-goops-type", 2, 0, 0,
            (SCM scm_gtype, SCM scm_value),
            "")
{
    GQuark type_key;
    SCM scm_type;
    GType gtype;

    gtype = ((ScmGTypeWrapper *) scm_foreign_object_ref (scm_gtype, 0))->type;

    type_key = _scmg_type_key (gtype);

    scm_type = g_type_get_qdata (gtype, type_key);
    //    if (scm_value == )
}



*/
void
scmgi_type_init ()
{
#ifndef SCM_MAGIC_SNARFER
#include "scmgi-type.x"
#endif
    scm_g_type_class = scm_make_foreign_object_type (scm_sym_gtype,
                                                     scm_from_utf8_symbol ("type"),
                                                     NULL);
}
