#include "gi-repository.h"
#include "gi-infos.h"
#include "gtype.h"

SCM_DEFINE (scm_g_irepository_get_default, "gi-repository-get-default", 0, 0, 0,
            (),
            "")
{
  return scm_make_foreign_object_1 (scm_girepository_class, (void *) g_irepository_get_default());
}

// FIXME: scm_Flags is currently ignored
SCM_DEFINE (scm_g_irepository_require, "%gi-repository-require", 2, 1, 0,
            (SCM scm_repository, SCM scm_namespace, SCM scm_version),
            ""
            )
{
  GIRepository *repo;
  GTypelib *typelib;
  GError *error;
  const char *version;
  const char *namespace;

  repo = (GIRepository *) scm_foreign_object_signed_ref (scm_repository, 0);

  if (SCM_UNBNDP (scm_version))
    version = NULL;
  else
    version = scm_to_locale_string (scm_version);

  error = NULL;

  if (scm_is_symbol (scm_namespace))
    scm_namespace = scm_symbol_to_string (scm_namespace);

  typelib = g_irepository_require (repo,
                                   scm_to_locale_string (scm_namespace),
                                   version,
                                   0,
                                   &error);

  if (error) {
    g_critical ("Failed to load typelib: %s", error->message);
    return SCM_UNSPECIFIED;
  }

  return scm_make_foreign_object_1 (scm_gitypelib_class, (void *) typelib);
}

SCM_DEFINE (scm_g_irepository_get_infos, "%gi-repository-get-infos", 2, 0, 0,
            (SCM scm_repository, SCM scm_namespace),
            ""
            )
{
  GIRepository *repository;
  GIBaseInfo *info;
  const char *namespace;
  gssize n_infos;
  SCM scm_infos;
  gint i;

  repository = (GIRepository *) scm_foreign_object_signed_ref (scm_repository, 0);

  if (scm_is_symbol (scm_namespace))
    scm_namespace = scm_symbol_to_string (scm_namespace);

  namespace = scm_to_locale_string (scm_namespace);

  n_infos = g_irepository_get_n_infos (repository, namespace);

  if (n_infos <0) {
    g_critical ("Namespace '%s' not loaded", namespace);
    return SCM_UNSPECIFIED;
  }

  scm_infos = SCM_EOL;

  for(i = 0; i < n_infos; i++) {
    GIBaseInfo *info;
    SCM scm_info;

    info = g_irepository_get_info (repository, namespace, i);
    g_assert (info != NULL);

    scm_info = gi_make_info (info);

    scm_infos = scm_append (scm_list_2 (scm_infos, scm_list_1 (scm_info)));
  }

  return scm_infos;
}

SCM_DEFINE (scm_g_irepository_find_by_name, "%gi-repository-find-by-name", 3, 0, 0,
            (SCM scm_repository, SCM scm_namespace, SCM scm_name),
            ""
            )
{
  GIRepository *repo;
  GIBaseInfo *info;
  GError *error;

  repo = (GIRepository *) scm_foreign_object_signed_ref (scm_repository, 0);

  error = NULL;


  if (scm_is_symbol (scm_namespace))
    scm_namespace = scm_symbol_to_string (scm_namespace);

  if (scm_is_symbol (scm_name))
    scm_name = scm_symbol_to_string (scm_name);

  info = g_irepository_find_by_name (repo,
                                     scm_to_locale_string (scm_namespace),
                                     scm_to_locale_string (scm_name));

  if (info == NULL)
    return SCM_UNSPECIFIED;

  return scm_make_foreign_object_1 (scm_gibase_info_type, (void *) info);
}

SCM_DEFINE (scm_g_irepository_find_by_gtype, "%gi-repository-find-by-gtype", 2, 0, 0,
            (SCM scm_repository, SCM scm_gtype),
            ""
            )
{
  GIRepository *repository;
  GIBaseInfo *info;
  GType gtype;
  GError *error;

  repository = (GIRepository *) scm_foreign_object_signed_ref (scm_repository, 0);
  gtype = scm_to_ulong (scm_gtype);

  error = NULL;

  info = g_irepository_find_by_gtype (repository, gtype);

  return scm_make_foreign_object_1 (scm_variable_ref (scm_c_lookup ("<gi-base-info>")),
                                    (void *) info);
}

void
gi_repository_init (void)
{
#ifndef SCM_MAGIC_SNARFER
#include "gi-repository.x"
#endif

  scm_t_struct_finalize finalizer = gi_finalize_object;

  scm_girepository_class = scm_make_foreign_object_type (scm_from_utf8_symbol ("<gi-repository>"),
                                                         scm_list_1 (scm_from_utf8_symbol ("ptr")),
                                                         finalizer);
  scm_c_define ("<gi-repository>", scm_girepository_class);

  scm_gitypelib_class = scm_make_foreign_object_type (scm_from_utf8_symbol ("<gi-typelib>"),
                                                      scm_list_1 (scm_from_utf8_symbol ("ptr")),
                                                      finalizer);
  scm_c_define ("<gi-typelib>", scm_gitypelib_class);
}
