/* -*- Mode: C; c-base-offset: 4 -*-
 * vim: tabstop=4 shiftwidth=4 expandtab
 */

#include <libguile.h>
#include <glib-object.h>

#include "gi-module.h"

void
ggi_register_constants (void)
{
  scm_c_define ("g-minfloat", scm_from_double ((double) G_MINFLOAT));
  scm_c_define ("g-maxfloat", scm_from_double ((double) G_MAXFLOAT));
  scm_c_define ("g-mindouble", scm_from_double (G_MINDOUBLE));
  scm_c_define ("g-maxdouble", scm_from_double (G_MAXDOUBLE));
  scm_c_define ("g-minshort", scm_from_short (G_MINSHORT));
  scm_c_define ("g-maxshort", scm_from_short (G_MAXSHORT));
  scm_c_define ("g-maxushort", scm_from_ushort (G_MAXUSHORT));
  scm_c_define ("g-minint", scm_from_int (G_MININT));
  scm_c_define ("g-maxint", scm_from_int (G_MAXINT));
  scm_c_define ("g-maxuint", scm_from_uint (G_MAXUINT));
  scm_c_define ("g-minlong", scm_from_long (G_MINLONG));
  scm_c_define ("g-maxlong", scm_from_long (G_MAXLONG));
  scm_c_define ("g-maxulong", scm_from_ulong (G_MAXULONG));
  scm_c_define ("g-maxsize", scm_from_size_t (G_MAXSIZE));
  scm_c_define ("g-minssize", scm_from_size_t (G_MINSSIZE));
  scm_c_define ("g-maxssize", scm_from_ssize_t (G_MAXSSIZE));
  scm_c_define ("g-minoffset", scm_from_int64 (G_MINOFFSET));
  scm_c_define ("g-maxoffset", scm_from_int64 (G_MAXOFFSET));

  scm_c_export ("g-minfloat", "g-maxfloat", "g-mindouble", "g-maxdouble",
                "g-minshort", "g-maxshort", "g-maxushort", "g-minint",
                "g-maxint", "g-maxuint", "g-minlong", "g-maxlong",
                "g-maxulong", "g-maxsize", "g-minssize", "g-maxssize",
                "g-minoffset", "g-maxoffset", NULL);
}

void
gi_module_init (void)
{
  ggi_register_constants ();

#ifndef SCM_MAGIC_SNARFER
#include "gi-module.x"
#endif
}
