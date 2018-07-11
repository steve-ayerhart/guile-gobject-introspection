#ifndef __GGI_CALLABLE_PROXY_H__
#define __GGI_CALLABLE_PROXY_H__

#include <libguile.h>
#include <girepository.h>
#include <girffi.h>

G_BEGIN_DECLS

typedef struct _GGIInvokeArgState
{
  GIArgument arg_value;
  GIArgument arg_pointer;

}  GGIInvokeArgState;

typedef struct _GGIInvokeState
{
  SCM scm_in_args;
  GIArgument **ffi_args;
  GGIInvokeArgState *args;
  GIArgument return_arg;
  GError *error;
  gboolean failed;
  gpointer user_data;
  gpointer function_ptr;

} GGIInvokeArgState;

G_END_DECLS

#endif
