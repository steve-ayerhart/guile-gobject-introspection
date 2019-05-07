#ifndef __GGI_ARGUMENT_H__
#define __GGI_ARGUMENT_H__

#include <girffi.h>
#include <glib.h>
#include <libguile.h>
#include <girepository.h>

G_BEGIN_DECLS

typedef gssize (*GGIArgArrayLengthPolicy) (gsize item_index,
                                           void *user_data1,
                                           void *user_data2);

gssize
_ggi_argument_array_length_marshal (gsize  length_arg_index,
                                    void  *user_data1,
                                    void  *user_data2);

gpointer
_ggi_arg_to_hash_pointer (const GIArgument *arg,
                          GITypeInfo       *type_info);

void
_ggi_hash_pointer_to_arg (GIArgument *arg,
                          GITypeInfo *type_info);

GArray *
_ggi_argument_to_array (GIArgument              *arg,
                        GGIArgArrayLengthPolicy  array_length_policy,
                        void                    *user_data1,
                        void                    *user_data2,
                        GITypeInfo              *type_info,
                        gboolean                *out_free_array);

GIArgument
_ggi_argument_from_object (SCM         scm_object,
                           GITypeInfo *type_info,
                           GITransfer  transfer);

SCM
_ggi_argument_to_object (GIArgument *arg,
                         GITypeInfo *type_info,
                         GITransfer  transfer);

void
_ggi_argument_release (GIArgument  *arg,
                       GITypeInfo  *type_info,
                       GITransfer   transfer,
                       GIDirection  direction);

gboolean
ggi_argument_to_gssize (GIArgument *arg_in,
                        GITypeTag   type_tag,
                        gssize     *gssize_out);
//


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


G_GNUC_INTERNAL void
gi_argument_init (void);

G_END_DECLS

#endif
