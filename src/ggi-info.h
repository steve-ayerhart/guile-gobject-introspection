/* -*- Mode: C; c-base-ioffset: 4 -*-
 * vim: tabstop=4 shiftwidth=4 expandtab
*/

#ifndef __GGI_INFO_H__
#define __GGI_INFO_H__

#include <glib.h>
#include <libguile.h>
#include <girepository.h>

G_BEGIN_DECLS

static SCM scm_base_info_type;

static SCM scm_callable_info_type;
static SCM scm_callback_info_type;
static SCM scm_function_info_type;
static SCM scm_signal_info_type;
static SCM scm_v_func_info_type;

static SCM scm_registered_type_info_type;
static SCM scm_enum_info_type;
static SCM scm_interface_info_type;
static SCM scm_object_info_type;
static SCM scm_struct_info_type;
static SCM scm_union_info_type;

static SCM scm_arg_info_type;
static SCM scm_constant_info_type;
static SCM scm_field_info_type;
static SCM scm_property_info_type;
static SCM scm_value_info_type;
static SCM scm_type_info_type;

static void
ggi_finalize_object (SCM scm_info)
{
  GIBaseInfo *info;

  info = (GIBaseInfo *)scm_foreign_object_signed_ref (scm_info, 0);

  g_base_info_unref (info);
}

GIBaseInfo *
ggi_object_get_gi_info (SCM scm_object);

SCM
ggi_make_info (GIBaseInfo *info);

G_END_DECLS

#endif /* __GGI_INFO_H__ */
