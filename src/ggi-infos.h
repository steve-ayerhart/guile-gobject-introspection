/* -*- Mode: C; c-base-ioffset: 4 -*-
 * vim: tabstop=4 shiftwidth=4 expandtab
*/

#ifndef __GGI_INFOS_H__
#define __GGI_INFOS_H__

#include <glib.h>
#include <libguile.h>
#include <girepository.h>

#include "ggi-cache.h"

G_BEGIN_DECLS

static SCM scm_gibase_info_type;

static SCM scm_gicallable_info_type;
static SCM scm_gicallback_info_type;
static SCM scm_gifunction_info_type;
static SCM scm_gisignal_info_type;
static SCM scm_giv_func_info_type;

static SCM scm_giregistered_type_info_type;
static SCM scm_gienum_info_type;
static SCM scm_giinterface_info_type;
static SCM scm_giobject_info_type;
static SCM scm_gistruct_info_type;
static SCM scm_giunion_info_type;

static SCM scm_giarg_info_type;
static SCM scm_giconstant_info_type;
static SCM scm_gifield_info_type;
static SCM scm_giproperty_info_type;
static SCM scm_givalue_info_type;
static SCM scm_gitype_info_type;

static void
ggi_finalize_object (SCM scm_info)
{
  g_print ("FINALIZING OBJ\n");
  GIBaseInfo *info;

  info = (GIBaseInfo *) scm_to_pointer (scm_foreign_object_ref (scm_info, 0));

  g_base_info_unref (info);
}

static void
ggi_finalize_pointer (void *info)
{
  g_print ("FINALIZING PROP\n");

  g_base_info_unref (info);
}

GIBaseInfo *
ggi_object_get_gi_info (SCM scm_object);

SCM
ggi_make_info (GIBaseInfo *info);

G_END_DECLS

#endif /* __GGI_INFO_H__ */
