#ifndef __GI_CALLABLE_H__
#define __GI_CALLABLE_H__

#include <glib.h>
#include <libguile.h>
#include <girepository.h>

extern scm_t_bits callable_info_t;
extern scm_t_bits callback_info_t;

G_GNUC_INTERNAL void
gi_callable_init (void);

#endif
