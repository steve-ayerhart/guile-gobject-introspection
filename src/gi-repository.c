#include "gi-repository.h"
#include "gi-types.h"

scm_t_bits repository_t;
scm_t_bits typelib_t;

SCM_DEFINE (scm_g_irepository_get_default, "g-i-repository-get-default", 0, 0, 0,
            (),
            "")
{
  SCM repository = scm_make_smob (repository_t);
  SCM_SET_SMOB_DATA (repository, g_irepository_get_default ());

  return repository;
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

void
gi_repository_init (void)
{
  #ifndef SCM_MAGIC_SNARFER
  #include "gi-repository.x"
  #endif
}
