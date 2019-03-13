#include "gutil.h"

#define SCM_ERROR_NOT_YET_IMPLEMENTED(what) \
  scm_c_gruntime_error (FUNC_NAME, "Not yet implemented: file ~S line ~S: ~A", \
                        SCM_LIST3 (scm_makfrom0str (__FILE__), scm_from_uint (__LINE__), what))


char *
ggi_gname_to_scm_name (const char *gname)
{
  size_t len = strlen (gname);
  GString *name = g_string_new (NULL);
  gboolean was_lower = FALSE;

  for (size_t ch = 0; ch < len; ch++)
    {
      if (g_ascii_islower (gname[ch]))
        {
          g_string_append_c (name, gname[ch]);
          was_lower = TRUE;
        }
      else if (gname[ch] == '_' || gname[ch] == '-')
        {
          g_string_append_c (name, '-');
          was_lower = FALSE;
        }
      else if (g_ascii_isdigit (gname[ch]))
        {
          g_string_append_c (name, gname[ch]);
          was_lower = FALSE;
        }
      else if (g_ascii_isupper (gname[ch]))
        {
          if (was_lower)
            g_string_append_c (name, '-');

          g_string_append_c (name, g_ascii_tolower (gname[ch]));
          was_lower = FALSE;
        }
    }

  return g_string_free (name, FALSE);
}

SCM
scm_c_gerror_to_scm (GError *error)
{
  return scm_list_3 (scm_from_ulong (error->domain),
                     scm_from_ulong (error->code),
                     scm_from_locale_string (error->message));
}

void
scm_c_raise_gerror (GError *error)
{
  SCM scm_gerror = scm_c_gerror_to_scm (error);
  g_error_free (error);
  scm_throw (scm_from_locale_symbol ("gerror"), scm_gerror);
}
