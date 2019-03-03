#include "ggi-repository.h"
#include "ggi-infos.h"
#include "gtype.h"

SCM_DEFINE (scm_g_irepository_get_default, "gi-repository-get-default", 0, 0, 0,
            (),
            "")
{
  GIRepository *repository;
  SCM scm_repository;

  repository = g_irepository_get_default ();
  scm_repository = scm_from_pointer (repository, NULL);

  return scm_make_foreign_object_1 (scm_girepository_class, scm_repository);
}

SCM_DEFINE (scm_g_irepository_require, "%gi-repository-require", 2, 2, 0,
            (SCM scm_repository, SCM scm_namespace, SCM scm_version, SCM scm_lazy),
            ""
            )
{
  GIRepository *repo;
  GIRepositoryLoadFlags flags = 0;
  GError *error;
  char *version;
  char *namespace;

  if (!SCM_UNBNDP (scm_lazy) && scm_is_true (scm_lazy)) {
    flags |= G_IREPOSITORY_LOAD_FLAG_LAZY;
  }

  repo = (GIRepository *) scm_to_pointer (scm_foreign_object_ref (scm_repository, 0));

  scm_dynwind_begin (0);
  scm_dynwind_free (version);

  if (SCM_UNBNDP (scm_version))
    version = NULL;
  else
    version = scm_to_locale_string (scm_version);

  error = NULL;

  scm_namespace = scm_symbol_to_string (scm_namespace);
  namespace = scm_to_locale_string (scm_namespace);
  scm_dynwind_free (namespace);

  g_irepository_require (repo, namespace, version, flags, &error);

  scm_dynwind_end ();

  if (error != NULL) {
    g_critical ("Failed to load typelib: %s", error->message);
    g_error_free (error);

    return SCM_BOOL_F;
  }

  return SCM_BOOL_T;
}

SCM_DEFINE (scm_g_irepository_get_infos, "%gi-repository-get-infos", 2, 0, 0,
            (SCM scm_repository, SCM scm_namespace),
            ""
            )
{
  GIRepository *repository;
  GIBaseInfo *info;
  char *namespace;
  gssize n_infos;
  SCM scm_infos;
  gint i;

  repository = (GIRepository *) scm_to_pointer (scm_foreign_object_ref (scm_repository, 0));

  scm_namespace = scm_symbol_to_string (scm_namespace);

  scm_dynwind_begin (0);
  scm_dynwind_free (namespace);

  namespace = scm_to_locale_string (scm_namespace);

  n_infos = g_irepository_get_n_infos (repository, namespace);

  if (n_infos <0) {
    g_critical ("Namespace '%s' not loaded", namespace);
    return SCM_UNSPECIFIED;
  }

  scm_infos = SCM_EOL;

  for(i = 0; i < n_infos; i++) {
    SCM scm_info;

    info = g_irepository_get_info (repository, namespace, i);
    g_assert (info != NULL);

    scm_info = ggi_make_info (info);

    scm_infos = scm_append (scm_list_2 (scm_infos, scm_list_1 (scm_info)));

  }

  scm_dynwind_end ();

  return scm_infos;
}

SCM_DEFINE (scm_g_irepository_find_by_name, "%gi-repository-find-by-name", 3, 0, 0,
            (SCM scm_repository, SCM scm_namespace, SCM scm_name),
            ""
            )
{
  scm_t_pointer_finalizer finalizer = ggi_finalize_pointer;
  GIRepository *repo;
  GIBaseInfo *info;
  GError *error;
  gchar *namespace;
  gchar *name;

  repo = (GIRepository *) scm_to_pointer (scm_foreign_object_ref (scm_repository, 0));

  error = NULL;

  scm_dynwind_begin (0);
  scm_namespace = scm_symbol_to_string (scm_namespace);
  scm_name = scm_symbol_to_string (scm_name);

  namespace = scm_to_locale_string (scm_namespace);
  scm_dynwind_free (namespace);
  name = scm_to_locale_string (scm_name);
  scm_dynwind_free (name);

  info = g_irepository_find_by_name (repo, namespace, name);

  scm_dynwind_end ();

  if (info == NULL)
    return SCM_BOOL_F;

  return scm_make_foreign_object_1 (scm_variable_ref (scm_c_lookup ("<gi-base-info>")),
                                    scm_from_pointer (g_base_info_ref (info), finalizer));
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
ggi_repository_init (void)
{
#ifndef SCM_MAGIC_SNARFER
#include "ggi-repository.x"
#endif

  scm_girepository_class = scm_make_foreign_object_type (scm_from_utf8_symbol ("<gi-repository>"),
                                                         scm_list_1 (scm_from_utf8_symbol ("repository")),
                                                         NULL);
  scm_c_define ("<gi-repository>", scm_girepository_class);

  scm_gitypelib_class = scm_make_foreign_object_type (scm_from_utf8_symbol ("<gi-typelib>"),
                                                      scm_list_1 (scm_from_utf8_symbol ("ptr")),
                                                      NULL);
  scm_c_define ("<gi-typelib>", scm_gitypelib_class);
}
