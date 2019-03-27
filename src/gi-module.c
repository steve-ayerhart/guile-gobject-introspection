/* -*- Mode: C; c-base-offset: 4 -*-
 * vim: tabstop=4 shiftwidth=4 expandtab
 */

#include <libguile.h>
#include <glib-object.h>
#include <girepository.h>
#include <glib.h>

#include "gtype.h"
#include "gi-module.h"
#include "ggi-argument.h"
#include "ggi-invoke.h"
#include "ggi-infos.h"
#include "ggi-function.h"
#include "ggi-cache.h"
#include "gutil.h"



void
ggi_define_module_enum (GIBaseInfo *info)
{
  GIEnumInfo *enum_info;
  const char *name;
  GType gtype;
  SCM scm_enum;

  GI_IS_ENUM_INFO (info);

  gtype = g_registered_type_info_get_g_type ((GIRegisteredTypeInfo *) info);

  name = ggi_gname_to_scm_name (g_base_info_get_name (info));
  scm_enum = scm_c_gtype_to_class (gtype);

  scm_c_define (name, scm_enum);
  scm_c_export (name, NULL);
}

void
ggi_define_module_constant (GIBaseInfo *info)
{
  GIConstantInfo *constant_info;
  const char *name;
  GITypeInfo *type_info;
  GIArgument value = {0};
  SCM scm_value;

  GI_IS_CONSTANT_INFO (info);

  constant_info = (GIConstantInfo *) info;

  if (g_constant_info_get_value (constant_info, &value) < 0) {
    scm_value = SCM_UNDEFINED;
    return;
  }

  type_info = g_constant_info_get_type (constant_info);
  name = ggi_gname_to_scm_name (g_base_info_get_name (info));

  scm_value = gi_arg_to_scm (type_info, GI_TRANSFER_NOTHING, value);
  g_constant_info_free_value (constant_info, &value);

  scm_c_define (name, scm_value);
  scm_c_export (name, NULL);
}

void
ggi_namespace_init (void *namespace)
{
  g_debug ("ggi_namespace_init");

  for (int n = 0; n < g_irepository_get_n_infos (NULL, namespace); n++)
    {
      GIBaseInfo *info;

      info = g_irepository_get_info (NULL, namespace, n);

      if (g_base_info_is_deprecated (info))
        {
          g_base_info_unref (info);
          continue;
        }

      switch (g_base_info_get_type (info))
        {
        case GI_INFO_TYPE_CONSTANT:
          ggi_define_module_constant (info);
          g_base_info_unref (info);
          break;
        case GI_INFO_TYPE_ENUM:
        case GI_INFO_TYPE_FLAGS:
          ggi_define_module_enum (info);
          g_base_info_unref (info);
          break;
        case GI_INFO_TYPE_FUNCTION:
          ggi_define_module_function (info);
        default:
          break;
        }
    }
}


SCM_DEFINE (scm_ggi_create_gi_module, "create-gi-module", 1, 1, 0,
            (SCM scm_namespace, SCM scm_version),
            "")
{
  g_debug ("scm_ggi_create_gi_module");

  GITypelib *typelib;
  gchar *namespace;
  gchar *module_name;
  gchar *version;
  GError *error;
  SCM scm_module;

  scm_dynwind_begin (0);

  namespace = (gchar *) scm_to_locale_string (scm_symbol_to_string (scm_namespace));
  scm_dynwind_free (namespace);

  g_debug (" namespace");
  if (SCM_UNBNDP (scm_version))
    version = NULL;
  else
    {
      version = (gchar *) scm_to_locale_string (scm_symbol_to_string (scm_version));
      scm_dynwind_free (version);
    }

  g_debug (" version");
  error = NULL;

  typelib = g_irepository_require (NULL, namespace, version, 0, &error);

  g_debug (" required");

  if (typelib == NULL)
    {
      scm_dynwind_end ();
      return SCM_BOOL_F;
    }

  if (error != NULL)
    {
      g_critical ("Failed to load repository: %s", error->message);
      g_error_free (error);
      scm_dynwind_end ();

      return SCM_BOOL_F;
    }

  module_name = g_strjoin (" ", "gi", namespace, NULL);

  scm_module = scm_c_define_module (module_name, ggi_namespace_init, namespace);

  scm_dynwind_end ();

  return scm_module;
}


void
ggi_gi_constants_init (void)
{
  g_debug ("ggi_gi_constants_init");

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

  scm_c_define ("blahblah", scm_c_gtype_to_class (g_error_get_type ()));
}


void
gi_module_init (void)
{
  g_debug ("gi_module_init");

  ggi_cache_init ();

  ggi_gi_constants_init ();

#ifndef SCM_MAGIC_SNARFER
#include "gi-module.x"
#endif
}
