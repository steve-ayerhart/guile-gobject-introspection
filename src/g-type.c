/* -*- Mode: C; c-basic-offset: 4 -*-
 * vim: tabstop=4 shiftwidth=4 expandtab
 */

#include "g-type.h"

SCM_GLOBAL_SYMBOL (scm_sym_gtype, "gtype");

SCM scm_class_g_type_class;

SCM_DEFINE (scm_g_type_initialize, "%g-type-initialize", 2, 0, 0,
            (SCM scm_g_type, SCM scm_initargs),
            "")
#define FUNC_NAME s_scm_g_type_initialize
{
    GType gtype;
    gchar *gtype_name;
    SCM k_g_type_name = scm_from_utf8_keyword ("g-type-name");
    SCM scm_g_type_name = SCM_UNDEFINED;


    scm_c_bind_keyword_arguments ("initialize", scm_initargs, 0,
                                  k_g_type_name, &scm_g_type_name,
                                  SCM_UNDEFINED);

    if (SCM_UNBNDP (scm_g_type_name))
        scm_misc_error ("initialize", "Need #:g-type-name initarg", SCM_EOL);

    SCM_VALIDATE_STRING (1, scm_g_type_name);

    scm_dynwind_begin (0);
    gtype_name = scm_to_locale_string (scm_g_type_name);
    scm_dynwind_free (gtype_name);

    gtype = g_type_from_name (gtype_name);
    if (!gtype)
        scm_misc_error ("scm_g_type_initialize",
                        "No GType registered with name ~a",
                        scm_list_1 (scm_g_type_name));

    scm_dynwind_end ();

    scm_foreign_object_set_x (scm_g_type, 0, (void *) gtype);

    return SCM_UNSPECIFIED;
}
#undef FUNC_NAME

SCM_DEFINE (scm_g_type_display, "%g-type-display", 2, 0, 0,
            (SCM scm_g_type, SCM scm_port),
            "")
{
    GType gtype;

    gtype = (GType) scm_foreign_object_ref (scm_g_type, 0);
    const gchar *name = g_type_name (gtype);

    return scm_simple_format (scm_port,
                              scm_from_locale_string ("<g-type ~a (~a)>"),
                              scm_list_2 (scm_from_locale_string (name ? name : "invalid"),
                                          scm_from_ulong (gtype)));
}

void
g_type_init ()
{
#ifndef SCM_MAGIC_SNARFER
#include "g-type.x"
#endif
    scm_class_g_type_class =
        scm_make_foreign_object_type (scm_sym_gtype,
                                      scm_list_1(scm_from_utf8_symbol ("type")),
                                      NULL);

    scm_c_define ("<g-type>", scm_class_g_type_class);
}
