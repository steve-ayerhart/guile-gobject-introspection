/* -*- Mode: C; c-basic-offset: 4 -*-
 * vim: tabstop=4 shiftwidth=4 expandtab
 */

#ifndef __GGI_STRUCT_H__
#define __GGI_STRUCT_H__

#include <libguile.h>

G_BEGIN_DECLS

SCM
ggi_struct_new_from_g_type (GType    gtype,
                            gpointer pointer,
                            gboolean free_on_alloc);

int ggi_struct_register_types (SCM s);

G_END_DECLS

#endif
