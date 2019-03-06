/* -*- Mode: C; c-basic-offset: 4 -*-
 */
#ifndef __GGI_CACHE_H__
#define __GGI_CACHE_H__

#include <libguile.h>
#include <girepository.h>
#include <girffi.h>


#include "ggi-invoke-state-struct.h"

G_BEGIN_DECLS

typedef struct _GGIArgCache GGIArgCache;
typedef struct _GGICallableCache GGICallableCache;
typedef struct _GGIFunctionCache GGIFunctionCache;
typedef struct _GGIVFuncCache GGIVFuncCache;

typedef GGIFunctionCache GGICallbackCache;
typedef GGIFunctionCache GGIConstructorCache;
typedef GGIFunctionCache GGIFunctionWithInstanceCache;
typedef GGIFunctionCache GGIMethodCache;
typedef GGICallableCache GGIClosureCache;

typedef gboolean (*GGIMarshalFromScmProc) (GGIInvokeState   *state,
                                           GGICallableCache *callable_cache,
                                           GGIArgCache      *arg_cache,
                                           SCM              scm_arg,
                                           GIArgument       *arg,
                                           gpointer         *cleanup_data);

typedef SCM (*GGIMarshalToScmProc) (GGIInvokeState   *state,
                                    GGICallableCache *callable_cache,
                                    GGIArgCache      *arg_cache,
                                    GIArgument       *arg,
                                    gpointer         *cleanup_data);


typedef enum {
              GGI_META_ARG_TYPE_PARENT,
              GGI_META_ARG_TYPE_CHILD,
              GGI_META_ARG_TYPE_CHLD_WITH_SCMARG,
              GGI_META_ARG_TYPE_CLOSURE,
} GGIMetaArgType;

typedef enum {
              GGI_DIRECTION_TO_SCM   = 1 << 0,
              GGI_DIRECTION_FROM_SCM = 1 << 1,
              GGI_DIRECTION_BIDIRECTIONAL = GGI_DIRECTION_TO_SCM | GGI_DIRECTION_FROM_SCM
} GGIDirection;


typedef enum {
              GGI_CALLING_CONTEXT_IS_FROM_C,
              GGI_CALLING_CONTEXT_IS_FROM_SCM
} GGICallingContext;


struct _GGIArgCache
{
    const gchar *arg_name;

    GGIMetaArgType meta_type;
    gboolean is_pointer;
    gboolean is_caller_allocates;
    gboolean is_skipped;
    gboolean is_allow_none;
    gboolean has_default;

    GGIDirection direction;
    GITransfer transfer;
    GITypeTag type_tag;
    GITypeInfo *type_info;

    GDestroyNotify destroy_notify;

    gssize c_arg_index;
    gssize scm_arg_index;

    GIArgument default_value;
};


typedef struct _GGIListCache {
    GGIArgCache arg_cache;
    GGIArgCache *item_cache;
} GGIListCache;


struct _GGIArgGArray
{
    GGIListCache list_cache;
    gssize fixed_size;
    gssize len_arg_index;
    gboolean is_zero_terminated;
    gsize item_size;
    GIArrayType array_type;

} GGIArgGArray;

typedef struct _GGIInterfaceCache
{
    GGIArgCache arg_cache;
    gboolean is_foreign;
    GType g_type;
    SCM scm_type;
    GIInterfaceInfo *interface_info;
    gchar *type_name;
} GGIInterfaceCache;

struct _GGICallableCache
{
    const gchar *name;
    const gchar *container_name;
    const gchar *namespace;

    GGICallingContext calling_context;

    GGIArgCache *return_cache;
    GPtrArray *args_cache;
    GSList *to_py_args;
    GSList *arg_name_list; /* for keyword arg matching */
    GHashTable *arg_name_hash;
    gboolean throws;

    /* Index of user_data arg passed to a callable. */
    gssize user_data_index;

    /* Index of user_data arg that can eat variable args passed to a callable. */
    gssize user_data_varargs_index;

    /* Number of args already added */
    gssize args_offset;

    /* Number of out args passed to g_function_info_invoke.
     * This is used for the length of GGIInvokeState.out_values */
    gssize n_to_scm_args;

    /* If the callable return value gets used */
    gboolean has_return;

    /* The type used for returning multiple values or NULL */
    SCM resultlist;

    /* Number of out args for g_function_info_invoke that will be skipped
     * when marshaling to scheme due to them being implicitly available
     * (list/array length).
     */
    gssize n_to_scm_child_args;

    /* Number of scheme arguments expected for invoking the gi function. */
    gssize n_scm_args;

    /* Minimum number of args required to call the callable from scheme.
     * This count does not include args with defaults. */
    gssize n_scm_required_args;

    void     (*deinit)              (GGICallableCache *callable_cache);

    gboolean (*generate_args_cache) (GGICallableCache *callable_cache,
                                     GICallableInfo *callable_info);
};

struct _GGIFunctionCache {
    GGICallableCache callable_cache;

    /* An invoker with ffi_cif already setup */
    GIFunctionInvoker invoker;

    SCM (*invoke) (GGIFunctionCache *function_cache,
                   GGIInvokeState *state,
                   SCM scm_args,
                   SCM scm_kwargs);
};

struct _GGIVFuncCache {
    GGIFunctionWithInstanceCache fwi_cache;

    GIBaseInfo *info;
};


gboolean
ggi_arg_base_setup (GGIArgCache *arg_cache,
                    GITypeInfo  *type_info,
                    GIArgInfo   *arg_info,
                    GITransfer   transfer,
                    GIDirection  direction);

gboolean
ggi_arg_interface_setup (GGIInterfaceCache *iface_cache,
                         GITypeInfo        *type_info,
                         GIArgInfo         *arg_info,  /* may be NULL for return arguments */
                         GITransfer         transfer,
                         GGIDirection       direction,
                         GIInterfaceInfo   *iface_info);

gboolean
ggi_arg_list_setup  (GGIListCache      *list_cache,
                     GITypeInfo        *type_info,
                     GIArgInfo         *arg_info,    /* may be NULL for return arguments */
                     GITransfer         transfer,
                     GGIDirection       direction,
                     GGICallableCache  *callable_cache);

GGIArgCache *
ggi_arg_interface_new_from_info (GITypeInfo       *type_info,
                                 GIArgInfo        *arg_info,     /* may be NULL for return arguments */
                                 GITransfer        transfer,
                                 GGIDirection      direction,
                                 GIInterfaceInfo  *iface_info);

GGIArgCache *
ggi_arg_cache_alloc (void);

GGIArgCache *
ggi_arg_cache_new (GITypeInfo       *type_info,
                   GIArgInfo        *arg_info,
                   GITransfer        transfer,
                   GGIDirection      direction,
                   GGICallableCache *callable_cache,
                   /* will be removed */
                   gssize            c_arg_index,
                   gssize            scm_arg_index);

void
ggi_arg_cache_free      (GGIArgCache *cache);

void
ggi_callable_cache_free    (GGICallableCache *cache);

gchar *
ggi_callable_cache_get_full_name (GGICallableCache *cache);

GGIFunctionCache *
ggi_function_cache_new     (GICallableInfo *info);

SCM
ggi_function_cache_invoke  (GGIFunctionCache *function_cache,
                            SCM scm_args,
                            SCM scm_kwargs);

GGIFunctionCache *
ggi_ccallback_cache_new    (GICallableInfo *info,
                            GCallback function_ptr);

SCM
ggi_ccallback_cache_invoke (GGIFunctionCache *function_cache,
                            SCM scm_args,
                            SCM scm_kwargs,
                            gpointer user_data);

GGIFunctionCache *
ggi_constructor_cache_new  (GICallableInfo *info);

GGIFunctionCache *
ggi_method_cache_new       (GICallableInfo *info);

GGIFunctionCache *
ggi_vfunc_cache_new        (GICallableInfo *info);

GGIClosureCache *
ggi_closure_cache_new      (GICallableInfo *info);

inline static guint
_ggi_callable_cache_args_len (GGICallableCache *cache) {
    return ((cache)->args_cache)->len;

}

inline static GGIArgCache *
_ggi_callable_cache_get_arg (GGICallableCache *cache, guint index) {
    return (GGIArgCache *) g_ptr_array_index (cache->args_cache, index);

}

inline static void
_ggi_callable_cache_set_arg (GGICallableCache *cache, guint index, GGIArgCache *arg_cache) {
    cache->args_cache->pdata[index] = arg_cache;

}


G_END_DECLS

#endif
