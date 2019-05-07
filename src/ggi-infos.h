/* -*- Mode: C; c-base-offset: 4 -*-
 * vim: tabstop=4 shiftwidth=4 expandtab
 */

#ifndef __GGI_INFOS_H__
#define __GGI_INFOS_H__

#include <glib.h>
#include <libguile.h>
#include <girepository.h>

#include "ggi-cache.h"

G_BEGIN_DECLS

void ggi_finalize_object (SCM scm_info);

void ggi_finalize_pointer (void *info);

gsize
_ggi_g_type_tag_size (GITypeTag type_tag);
gsize
_ggi_g_type_info_size (GITypeInfo *type_info);

GIBaseInfo *
ggi_object_get_gi_info (SCM scm_object);

gchar *
_ggi_g_base_info_get_fullname (GIBaseInfo *info);

SCM
ggi_make_info (GIBaseInfo *info);

G_END_DECLS

#endif /* __GGI_INFO_H__ */
