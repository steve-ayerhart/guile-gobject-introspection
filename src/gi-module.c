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

static SCM scm_gtype_name_to_scheme_name;
static SCM scm_gtype_name_to_class_name;

void
ggi_finalize_callable_cache (void *cache)
{
  g_debug ("ggi_fnalize_callable_cache");

  //ggi_callable_cache_free (cache);
}


void
ggi_define_module_enum (GIBaseInfo *info)
{
  g_debug ("ggi_define_module_enum: %s", g_base_info_get_name (info));

  GIEnumInfo *enum_info;
  const char *name;
  GType gtype;
  SCM scm_enum;

  GI_IS_ENUM_INFO (info);

  gtype = g_registered_type_info_get_g_type ((GIRegisteredTypeInfo *) info);

  name = scm_to_locale_string (scm_symbol_to_string (scm_call_1 (scm_gtype_name_to_class_name,
                                                                 scm_from_locale_string (g_type_name (gtype)))));

  scm_enum = scm_c_gtype_to_class (gtype);

  scm_c_define (name, scm_enum);
  scm_c_export (name, NULL);
}

void
ggi_define_module_constant (GIBaseInfo *info)
{
  g_debug ("ggi_define_module_constant: %s", g_base_info_get_name (info));

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
ggi_define_module_object_method (GGICallableCache *callable_cache, GICallableInfo *callable_info)
{
  g_debug ("ggi_define_module_object_method");

  SCM scm_callable_cache;
  char *method_name;

  method_name = ggi_gname_to_scm_function_name (callable_cache->name, callable_info);


  scm_c_define_gsubr (method_name,
                      callable_cache->n_scm_required_args,
                      callable_cache->n_scm_args - callable_cache->n_scm_required_args,
                      0,
                      ((GGIFunctionCache *) callable_cache)->wrapper);


  scm_callable_cache = scm_from_pointer (callable_cache, ggi_finalize_callable_cache);
  scm_set_procedure_property_x (scm_variable_ref (scm_c_lookup (method_name)),
                                scm_from_locale_symbol ("cache"),
                                scm_callable_cache);
}

void
ggi_define_module_object_methods (GIBaseInfo *info)
{
  g_debug ("ggi_define_module_object_methods");

  GIObjectInfo *object_info;

  object_info = (GIObjectInfo *) info;

  for (size_t n = 0; n < g_object_info_get_n_methods (object_info); n++)
    {
      GIFunctionInfo *function_info;
      GGICallableCache *callable_cache;
      GGIFunctionCache *function_cache;

      function_info = g_object_info_get_method (object_info, n);

      switch (g_function_info_get_flags (function_info))
        {
        case GI_FUNCTION_IS_CONSTRUCTOR:
          function_cache = ggi_constructor_cache_new ((GICallableInfo *) function_info);
          break;
        case GI_FUNCTION_IS_METHOD:
          function_cache = ggi_method_cache_new ((GICallableInfo *) function_info);
          break;
        case GI_FUNCTION_WRAPS_VFUNC:
          function_cache = ggi_vfunc_cache_new ((GICallableInfo *) function_info);
          break;
        case GI_FUNCTION_IS_GETTER:
        case GI_FUNCTION_IS_SETTER:
          return;
        }

      ggi_define_module_object_method ((GGICallableCache *) function_cache,
                                       (GICallableInfo *) function_info);
    }
}

void
ggi_define_module_object (GIBaseInfo *info)
{
  GIObjectInfo *object_info;
  char *class_name;
  GType gtype;
  SCM scm_class;
  SCM scm_gtype_name;

  GI_IS_REGISTERED_TYPE_INFO (info);
  GI_IS_OBJECT_INFO (info);

  object_info = (GIObjectInfo *) info;
  gtype = g_registered_type_info_get_g_type ((GIRegisteredTypeInfo *) info);

  scm_dynwind_begin (0);

  class_name = scm_to_locale_string (scm_symbol_to_string (scm_call_1 (scm_gtype_name_to_class_name,
                                                                       scm_from_locale_string (g_type_name (gtype)))));
  scm_dynwind_free (class_name);

  scm_class = scm_c_gtype_to_class (gtype);


  scm_c_define (class_name, scm_class);
  scm_c_export (class_name, NULL);

  scm_dynwind_end ();

  ggi_define_module_object_methods (info);
}

void
ggi_define_module_function (GIBaseInfo *info)
{
    g_debug ("ggi_define_module_function:");
    g_debug (" %s", g_base_info_get_name (info));

    GGIFunctionCache *function_cache;
    GGICallableCache *callable_cache;
    GIFunctionInfo *function_info;
    const char *function_name;
    SCM scm_callable_cache;
    const int args;
    const int opt_args;
    ggi_gsubr_t  *ggi_func;

    GI_IS_FUNCTION_INFO (info);

    function_cache = ggi_function_cache_new ((GICallableInfo *) info);
    callable_cache = (GGICallableCache *) function_cache;

    g_assert (function_cache != NULL);

    function_name = ggi_gname_to_scm_function_name (callable_cache->name,
                                                    (GICallableInfo *) info);


    g_debug (" n_args: %d, req_args: ",
             callable_cache->n_scm_args,
             callable_cache->n_scm_required_args);

    scm_c_define_gsubr (function_name,
                        callable_cache->n_scm_required_args,
                        callable_cache->n_scm_args - callable_cache->n_scm_required_args,
                        0,
                        function_cache->wrapper);


    scm_callable_cache = scm_from_pointer (callable_cache, ggi_finalize_callable_cache);
    scm_set_procedure_property_x (scm_variable_ref (scm_c_lookup (function_name)),
                                  scm_from_locale_symbol ("cache"),
                                  scm_callable_cache);
}



void
ggi_namespace_init (void *namespace)
{
  g_debug ("ggi_namespace_init");

  scm_c_use_module ("oop goops");
  scm_c_use_module ("glib utils");
  scm_c_use_module ("gobject");
  scm_c_use_module ("gobject gtype");
  scm_c_use_module ("gobject gvalue");

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
        case GI_INFO_TYPE_OBJECT:
          ggi_define_module_object (info);
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

  if (SCM_UNBNDP (scm_version))
    version = NULL;
  else
    {
      version = (gchar *) scm_to_locale_string (scm_symbol_to_string (scm_version));
      scm_dynwind_free (version);
    }

  error = NULL;

  typelib = g_irepository_require (NULL, namespace, version, 0, &error);

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

  scm_gtype_name_to_scheme_name =
    scm_permanent_object (scm_variable_ref (scm_c_lookup ("gtype-name->scheme-name")));
  scm_gtype_name_to_class_name =
    scm_permanent_object (scm_variable_ref (scm_c_lookup ("gtype-name->class-name")));
}


void
gi_module_init (void)
{
  g_debug ("gi_module_init");

  //  ggi_cache_init ();

  ggi_gi_constants_init ();

#ifndef SCM_MAGIC_SNARFER
#include "gi-module.x"
#endif
}
