#ifndef __GGI_INVOKE_STATE_STRUCT_H__
#define __GGI_INVOKE_STATE_STRUCT_H__

#include <libguile.h>

#include <girepository.h>

G_BEGIN_DECLS

typedef struct _GGIInvokeArgState
{
  /* Holds memory for the C value of arguments marshaled "to" or "from" scm. */
  GIArgument arg_value;

  /* Holds pointers to values in arg_values or a caller allocated chunk of
   * memory via arg_pointer.v_pointer.
   */
  GIArgument arg_pointer;

  /* Holds from_scm marshaler cleanup data. */
  gpointer arg_cleanup_data;

  /* Holds to_scm marshaler cleanup data. */
  gpointer to_scm_arg_cleanup_data;

} GGIInvokeArgState;

typedef struct _GGIInvokeState
{
  SCM scm_in_args;
  gssize n_scm_in_args;

  /* Number of arguments the ffi wrapped C function takes. Used as the exact
   * count for argument related arrays held in this struct.
   */
  gssize n_args;

  /* List of arguments passed to ffi. Elements can point directly to values held in
   * arg_values for "in/from scm" or indirectly via arg_pointers for
   * "out/inout/to scm". In the latter case, the args[x].arg_pointer.v_pointer
   * member points to memory for the value storage.
   */
  GIArgument **ffi_args;

  /* Array of size n_args containing per argument state */
  GGIInvokeArgState *args;

  /* Memory to receive the result of the C ffi function call. */
  GIArgument return_arg;
  gpointer to_scm_return_arg_cleanup_data;

  /* A GError exception which is indirectly bound into the last position of
   * the "args" array if the callable caches "throws" member is set.
   */
  GError *error;

  gboolean failed;

  gpointer user_data;

  /* Function pointer to call with ffi. */
  gpointer function_ptr;

} GGIInvokeState;

G_END_DECLS

#endif
