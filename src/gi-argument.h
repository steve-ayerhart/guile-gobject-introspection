#ifndef __GI_ARGUMENT_H__
#define __GI_ARGUMENT_H__

#include <girffi.h>
#include <glib.h>
#include <libguile.h>
#include <girepository.h>

G_GNUC_INTERNAL SCM
gi_return_value_to_scm (GICallableInfo *info,
                        GArgument return_value);

G_GNUC_INTERNAL void
scm_return_value_to_gi (SCM scm_return,
                        GICallableInfo *info,
                        GArgument *return_value);

G_GNUC_INTERNAL SCM
gi_arg_to_scm (GITypeInfo *arg_type,
               GITransfer transfer_type,
               GArgument arg);

G_GNUC_INTERNAL void
scm_to_gi_arg (SCM scm_arg,
               GITypeInfo *arg_type,
               GITransfer transfer_type,
               GIScopeType scope_type,
               GICallableInfo *destroy_info,
               GArgument *arg,
               GArgument *destroy_arg);

G_GNUC_INTERNAL SCM
gi_interface_to_scm (GITypeInfo *arg_type,
                     GITransfer transfer_type,
                     GArgument arg);

G_GNUC_INTERNAL void
scm_to_gi_interface (SCM scm_arg,
                     GIInfoType arg_type,
                     GITransfer transfer_type,
                     GIScopeType scope_type,
                     GIBaseInfo *info,
                     GICallableInfo *destroy_info,
                     GArgument *arg,
                     GArgument *destroy_arg);

static void
callback_closure (ffi_cif *cif,
                  void *result,
                  void **args,
                  void *data);

static void
destroy_notify_callback (ffi_cif *cif,
                         void *result,
                         void **args,
                         void *data);

G_GNUC_INTERNAL void
gi_argument_init (void);

#endif
