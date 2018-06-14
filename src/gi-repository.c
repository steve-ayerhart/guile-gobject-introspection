#include "gi-repository.h"
#include "gi-infos.h"

SCM_DEFINE (scm_g_irepository_get_default, "g-i-repository-get-default", 0, 0, 0,
            (),
            "")
{
  return scm_make_foreign_object_1 (repository_t, (void *) g_irepository_get_default());
}

// FIXME: scm_Flags is currently ignored
SCM_DEFINE (scm_g_irepository_require, "%g-i-repository-require", 2, 1, 0,
            (SCM scm_repository, SCM scm_namespace, SCM scm_version),
            ""
            )
{
  GIRepository *repo;
  GTypelib *typelib;
  GError *error;
  const char *version;
  const char *namespace;
  SCM scm_typelib;

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

  return scm_make_foreign_object_1 (typelib_t, (void *) typelib);
}

SCM_DEFINE (scm_g_irepository_get_infos, "%g-i-repository-get-infos", 2, 0, 0,
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

    scm_info = make_gi_info (info);

    // maybe? g_base_info_unref (info);
    scm_append (scm_list_2 (scm_infos, scm_list_1 (scm_info)));
  }

  return scm_infos;
}

/*
SCM_DEFINE (scm_g_irepository_get_n_infos, "g-i-repository-get-n-infos", 1, 1, 0,
            (SCM scm_namespace, SCM scm_repository),
            ""
            )
{
  GIRepository *repo;

  repo = (GIRepository *) SCM_SMOB_DATA (scm_repository);

  if (SCM_UNBNDP (scm_repository))
    repo = NULL;
  else
    repo = g_irepository_get_default ();

  if (scm_is_symbol (scm_namespace))
    scm_namespace = scm_symbol_to_string (scm_namespace);

  return scm_from_int (g_irepository_get_n_infos (repo,
                                                  scm_to_locale_string (scm_namespace)));
}

SCM_DEFINE (scm_g_irepository_get_info, "g-i-repository-get-info", 2, 1, 0,
            (SCM scm_namespace, SCM scm_index, SCM scm_repository),
            ""
            )
{
  GIRepository *repo;
  GIBaseInfo *info;
  GError *error;
  SCM scm_info;

  repo = (GIRepository *) SCM_SMOB_DATA (scm_repository);

  error = NULL;

  if (SCM_UNBNDP (scm_repository))
    repo = NULL;
  else
    repo = g_irepository_get_default ();

  if (scm_is_symbol (scm_namespace))
    scm_namespace = scm_symbol_to_string (scm_namespace);

  info = g_irepository_get_info (repo,
                                 scm_to_locale_string (scm_namespace),
                                 scm_to_int (scm_index));

  if (info == NULL)
    return SCM_UNSPECIFIED;

  scm_info = scm_make_smob (base_info_t);
  SCM_SET_SMOB_DATA (scm_info, info);

  return scm_info;
}

SCM_DEFINE (scm_g_irepository_find_by_name, "g-i-repository-find-by-name", 2, 1, 0,
           (SCM scm_namespace, SCM scm_name, SCM scm_repository),
           ""
           )
{
  GIRepository *repo;
  GIBaseInfo *info;
  GError *error;
  SCM scm_info;

  repo = (GIRepository *) SCM_SMOB_DATA (scm_repository);

  error = NULL;

  if (SCM_UNBNDP (scm_repository))
    repo = NULL;
  else
    repo = g_irepository_get_default ();

  if (scm_is_symbol (scm_namespace))
    scm_namespace = scm_symbol_to_string (scm_namespace);

  if (scm_is_symbol (scm_name))
    scm_name = scm_symbol_to_string (scm_name);

  info = g_irepository_find_by_name (repo,
                                     scm_to_locale_string (scm_namespace),
                                     scm_to_locale_string (scm_name));

  if (info == NULL)
    return SCM_UNSPECIFIED;

  scm_info = scm_make_smob (base_info_t);
  SCM_SET_SMOB_DATA (scm_info, info);

  return scm_info;
}
*/


void
gi_repository_init (void)
{
  #ifndef SCM_MAGIC_SNARFER
  #include "gi-repository.x"
  #endif

  scm_t_struct_finalize finalizer = finalize_gi_object;

  repository_t = scm_make_foreign_object_type (scm_from_utf8_symbol ("g-i-repository"),
                                               scm_list_1 (scm_from_utf8_symbol ("ptr")),
                                               finalizer);

  typelib_t = scm_make_foreign_object_type (scm_from_utf8_symbol ("g-i-typelib"),
                                            scm_list_1 (scm_from_utf8_symbol ("ptr")),
                                            finalizer);
}
