#include "gutil.h"

#define SCM_ERROR_NOT_YET_IMPLEMENTED(what) \
  scm_c_gruntime_error (FUNC_NAME, "Not yet implemented: file ~S line ~S: ~A", \
                        SCM_LIST3 (scm_makfrom0str (__FILE__), scm_from_uint (__LINE__), what))

#ifdef DEBUG_PRINT
#define DEBUG_ALLOC(str, args...) g_print ("I: " str "\n", ##args)
#else
#define DEBUG_ALLOC(str, args...)
#endif

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
  scm_throw (scm_from_locale_symbol ("g-error"), scm_gerror);
}
