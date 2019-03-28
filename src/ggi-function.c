/* -*- Mode: C; c-basic-offset: 4 -*-
 * vim: tabstop=4 shiftwidth=4 expandtabo
 */

#include <libguile.h>
#include <ffi.h>
#include <glib.h>
#include <girepository.h>

#include "ggi-function.h"
#include "ggi-cache.h"

void
ggi_finalize_callable_cache (void *cache)
{
    g_debug ("ggi_fnalize_callable_cache");

    //    ggi_callable_cache_free (cache);
}

void
ggi_define_module_function (GIBaseInfo *info)
{
    g_debug ("ggi_define_module_function: %s", g_base_info_get_name (info));

    GGIFunctionCache *function_cache;
    GGICallableCache *callable_cache;
    GIFunctionInfo *function_info;
    const char *function_name;
    SCM scm_function_cache;
    const int args;
    const int opt_args;
    ggi_gsubr_t  *ggi_func;

    GI_IS_FUNCTION_INFO (info);

    function_name = g_base_info_get_name (info);

    function_cache = ggi_function_cache_new ((GICallableInfo *) info);
    callable_cache = (GGICallableCache *) function_cache;

    g_assert (function_cache != NULL);

    scm_c_define_gsubr (function_name,
                        callable_cache->n_scm_required_args,
                        callable_cache->n_scm_args - callable_cache->n_scm_required_args,
                        0,
                        function_cache->wrapper);


    //    scm_function_cache = scm_from_pointer (function_cache, ggi_finalize_callable_cache);

    /*
      scm_c_define_gsubr (function_name,
      callable_cache->n_scm_required_args,
      callable_cache->n_scm_args - callable_cache->n_scm_required_args,
      0,
      _wrap_ggi_function);

      scm_set_procedure_property_x (scm_variable_ref (scm_c_lookup (function_name)),
      scm_from_locale_symbol ("cache"),
      scm_function_cache);
    */
}
