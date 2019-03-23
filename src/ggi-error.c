#include "ggi-error.h"
#include "ggi-cache.h"
#include "gtype.h"

#include <libguile.h>
#include <glib.h>

SCM
ggi_error_marshal_to_scm (GError **error)
{
  const char *domain = NULL;

  g_return_val_if_fail (error != NULL, SCM_UNSPECIFIED);

  if (*error == NULL)
    return SCM_UNSPECIFIED;

  if ((*error)->domain)
    domain = g_quark_to_string ((*error)->domain);

  return scm_call_3 (scm_c_public_ref ("glib gerror", "make-gerror"),
                     scm_from_utf8_string (domain),
                     scm_from_int ((*error)->code),
                     scm_from_utf8_string ((*error)->message));
}

gboolean
ggi_error_check (GError **error)
{
  SCM scmgerror;

  g_return_val_if_fail (error != NULL, FALSE);
  if (*error == NULL)
    return FALSE;

  scmgerror = ggi_error_marshal_to_scm (error);
  scm_throw (scm_from_locale_symbol ("glib"),
             scmgerror);

  g_clear_error (error);

  return TRUE;
}

gboolean
ggi_error_marshal_from_scm (SCM scmgerror, GError **error)
{
  int code;
  char *message;
  char *domain;
  gboolean res = FALSE;

  // TODO CHECK is GOOPS

  scm_dynwind_begin (0);

  message = scm_to_locale_string (scm_call_1 (scm_c_public_ref ("glib gerror", "gerror-message"),
                                              scmgerror));
  scm_dynwind_free (message);

  domain = scm_to_locale_string (scm_call_1 (scm_c_public_ref ("glib gerror", "gerror-domain"),
                                             scmgerror));
  scm_dynwind_free (domain);

  code = scm_to_int (scm_call_1 (scm_c_public_ref ("glib gerror", "gerror-code"),
                                 scmgerror));

  res = TRUE;
  g_set_error_literal (error,
                       g_quark_from_string ((gchar *) domain),
                       (gint) code,
                       (gchar *) message);

  scm_dynwind_end ();

  return res;
}

gboolean
ggi_gerror_raised_check (GError **error)
{
  int res = -1;
  return res;
}

static gboolean
_ggi_marshal_from_scm_gerror (GGIInvokeState   *state,
                              GGICallableCache *callable_cache,
                              GGIArgCache      *arg_cache,
                              SCM               scm_arg,
                              GIArgument       *arg,
                              gpointer         *cleanup_data)
{
  GError *error = NULL;

  if (ggi_error_marshal_from_scm (scm_arg, &error))
    {
      arg->v_pointer = error;
      *cleanup_data = error;
      return TRUE;
    }
  else
    {
      return FALSE;
    }
}
static void
_ggi_marshal_from_scm_gerror_cleanup (GGIInvokeState *state,
                                      GGIArgCache    *arg_cache,
                                      SCM             scm_arg,
                                      gpointer        data,
                                      gboolean        was_processed)
{
  if (was_processed)
    g_error_free ((GError *) data);
}

static SCM
_ggi_marshal_to_scm_gerror (GGIInvokeState *state,
                            GGICallableCache *callable_cache,
                            GGIArgCache      *arg_cache,
                            GIArgument       *arg,
                            gpointer         *cleanup_data)
{
  GError *error = arg->v_pointer;
  SCM scmgerror;

  scmgerror = ggi_error_marshal_to_scm (&error);

  if (arg_cache->transfer == GI_TRANSFER_EVERYTHING && error != NULL)
    g_error_free (error);

  return scmgerror;
}

static gboolean
ggi_arg_gerror_setup_from_info (GGIArgCache *arg_cache,
                                GITypeInfo  *type_info,
                                GIArgInfo   *arg_info,
                                GITransfer   transfer,
                                GGIDirection direction)
{
  if (!ggi_arg_base_setup (arg_cache, type_info, arg_info, transfer, direction))
    return FALSE;

  if (direction & GGI_DIRECTION_FROM_SCM)
    {
      arg_cache->from_scm_marshaller = _ggi_marshal_from_scm_gerror;

      if (arg_cache->transfer == GI_TRANSFER_NOTHING)
        arg_cache->from_scm_cleanup = _ggi_marshal_from_scm_gerror_cleanup;
    }

  if (direction & GGI_DIRECTION_TO_SCM)
    {
      arg_cache->to_scm_marshaller = _ggi_marshal_to_scm_gerror;
      arg_cache->meta_type = GGI_META_ARG_TYPE_PARENT;
    }

  return TRUE;
}

GGIArgCache *
ggi_arg_gerror_new_from_info (GITypeInfo  *type_info,
                              GIArgInfo   *arg_info,
                              GITransfer   transfer,
                              GGIDirection direction)
{
  gboolean setup_result;
  GGIArgCache *arg_cache;

  arg_cache = ggi_arg_cache_alloc ();

  setup_result = ggi_arg_gerror_setup_from_info (arg_cache,
                                                 type_info,
                                                 arg_info,
                                                 transfer,
                                                 direction);

  if (setup_result)
    {
      return arg_cache;
    }
  else
    {
      ggi_arg_cache_free (arg_cache);
      return NULL;
    }
}

static SCM
scmgerror_from_gvalue (const GValue *value)
{
  GError *gerror = (GError *) g_value_get_boxed (value);
  SCM scmgerror = ggi_error_marshal_to_scm (&gerror);

  if (SCM_UNBNDP (scmgerror))
    return SCM_UNSPECIFIED;
  else
    return scmgerror;
}

static int
scmgerror_to_gvalue (GValue *value, SCM scmgerror)
{
  GError *gerror = NULL;

  if (ggi_error_marshal_from_scm (scmgerror, &gerror))
    {
      g_value_take_boxed (value, gerror);
      return 0;
    }

  return -1;
}

void
ggi_error_init (void)
{
  g_debug ("ggi_error_init");
#ifndef SCM_MAGIC_SNARFER
#include "ggi-error.x"
#endif
}
