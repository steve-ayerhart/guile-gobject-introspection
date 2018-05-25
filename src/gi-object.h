#ifndef __GI_OBJECT_H__
#define __GI_OBJECT_H__

#include <glib.h>
#include <libguile.h>
#include <girepository.h>

extern scm_t_bits object_info_t;

G_GNUC_INTERNAL void
gi_object_init (void);

#endif
