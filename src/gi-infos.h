#ifndef __GI_INFOS_H__
#define __GI_INFOS_H__

#include <glib.h>
#include <libguile.h>
#include <girepository.h>

static SCM scm_base_info_class;

static SCM scm_callable_info_class;
static SCM scm_function_info_class;
static SCM scm_signal_info_class;
static SCM scm_v_func_info_class;

static SCM scm_registered_type_info_class;
static SCM scm_enum_info_class;
static SCM scm_interface_info_class;
static SCM scm_object_info_class;
static SCM scm_struct_info_class;
static SCM scm_union_info_class;

static SCM scm_arg_info_class;
static SCM scm_constant_info_class;
static SCM scm_field_info_class;
static SCM scm_property_info_class;
static SCM scm_type_info_class;

static void
finalize_gi_object (SCM scm_info)
{
  GIBaseInfo *info;

  info = (GIBaseInfo *)scm_foreign_object_signed_ref (scm_info, 0);

  g_base_info_unref (info);
}


void
gi_infos_init (void);

SCM
make_gi_info (GIBaseInfo *info);

#endif
