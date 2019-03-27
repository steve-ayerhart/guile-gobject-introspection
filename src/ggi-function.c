/* -*- Mode: C; c-basic-offset: 4 -*-
 * vim: tabstop=4 shiftwidth=4 expandtabo
 */

#include <libguile.h>
#include <ffi.h>
#include <glib.h>
#include <girepository.h>

#include "ggi-function.h"
#include "ggi-cache.h"

static void
_ggi_function_wrapper (ffi_cif *cif, void *ret, void **args, void *cache)
{
    g_debug ("ggi_function_wrapper");

    g_assert (cif != NULL);
    g_assert (ret != NULL);
    g_assert (args != NULL);
    g_assert (cache != NULL);

    *(ffi_arg *)ret = SCM_UNPACK (SCM_EOL);
}

static ggi_gsubr_t *
ggi_function_create_gsubr (GGICallableCache *callable_cache)
{
    g_debug ("ggi_function_create_gsubr");

    ffi_type **args = NULL;
    ffi_type *ret_type;
    ffi_closure *closure;
    ffi_status prep_ok;
    ffi_cif cif;
    void *ggi_subr;
    int n_total_args;
    int n_opt_args;

    closure = ffi_closure_alloc (sizeof (ffi_closure), &ggi_subr);

    g_return_val_if_fail (closure != NULL, NULL);
    g_return_val_if_fail (ggi_subr != NULL, NULL);

    n_opt_args = callable_cache->n_scm_args - callable_cache->n_scm_required_args;
    n_total_args = callable_cache->n_scm_required_args + n_opt_args;

    if (n_total_args > 0)
        args = g_new0 (ffi_type *, n_total_args);

    for (int i = 0; i < n_total_args; i++)
        {
            args[i] = &ffi_type_pointer;
        }

    ret_type = &ffi_type_pointer;

    prep_ok = ffi_prep_cif (&cif,
                            FFI_DEFAULT_ABI,
                            n_total_args,
                            ret_type,
                            args);

    if (prep_ok != FFI_OK)
        {
            scm_misc_error ("ggi_function_create_gsubr",
                            "failed to created closure",
                            NULL);
        }

    ffi_status closure_ok;
    closure_ok = ffi_prep_closure_loc (closure,
                                       &cif,
                                       _ggi_function_wrapper,
                                       callable_cache,
                                       ggi_subr);

    if (closure_ok != FFI_OK)
        {
            scm_misc_error ("ggi_function_create_gsubr",
                            "failed to created closure",
                            NULL);
        }

    return ggi_subr;
}

void
ggi_define_module_function (GIBaseInfo *info)
{
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

    ggi_func = ggi_function_create_gsubr (callable_cache);

    scm_c_define_gsubr (function_name,
                        callable_cache->n_scm_required_args,
                        callable_cache->n_scm_args - callable_cache->n_scm_required_args,
                        0,
                        ggi_func);

    /*
      scm_function_cache = scm_from_pointer (function_cache, ggi_finalize_callable_cache);
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
