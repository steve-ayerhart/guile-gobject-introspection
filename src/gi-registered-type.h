#ifndef __GI_REGISTERED_TYPE_H__
#define __GI_REGISTERED_TYPE_H__

#include <glib.h>
#include <libguile.h>
#include <girepository.h>

extern scm_t_bits registered_type_info_t;

G_GNUC_INTERNAL void
gi_registered_type_init (void);

#endif
