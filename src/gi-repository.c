#include "gi-repository.h"
#include "gi-types.h"

scm_t_bits repository_t;
scm_t_bits typelib_t;

SCM_DEFINE (scm_g_irepository_get_default, "g-i-repository-get-default", 0, 0, 0,
            (),
            "")
{
  SCM scm_repository = scm_make_smob (repository_t);
  SCM_SET_SMOB_DATA (scm_repository, g_irepository_get_default ());

  return scm_repository;
}

// FIXME: scm_Flags is currently ignored
SCM_DEFINE (scm_g_irepository_require, "g-i-repository-require", 1, 4, 0,
            (SCM scm_namespace, SCM scm_repository, SCM scm_version, SCM scm_flags),
            ""
            )
{
  GIRepository *repo;
  GTypelib *typelib;
  GError *error;
  const char *version;
  const char *namespace;
  SCM scm_typelib;

  repo = (GIRepository *) SCM_SMOB_DATA (scm_repository);

  if (SCM_UNBNDP (scm_repository))
    repo = NULL;
  else
    repo = g_irepository_get_default ();

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

  scm_typelib = scm_make_smob (typelib_t);
  SCM_SET_SMOB_DATA (scm_typelib, typelib);

  return scm_typelib;
}

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



void
gi_repository_init (void)
{
  #ifndef SCM_MAGIC_SNARFER
  #include "gi-repository.x"
  #endif

  repository_t = scm_make_smob_type ("g-i-repository", 0);
  typelib_t = scm_make_smob_type ("g-typelib", 0);
}
