#ifndef __GI_INFOS_H__
#define __GI_INFOS_H__

#include <glib.h>
#include <libguile.h>
#include <girepository.h>

static SCM base_info_class_t;

static void
finalize_gi_object (SCM scm_info)
{
  GIBaseInfo *info;

  info = (GIBaseInfo *)scm_foreign_object_signed_ref (scm_info, 0);

  g_base_info_unref (info);
}


G_GNUC_INTERNAL void
gi_infos_init (void);

#endif
