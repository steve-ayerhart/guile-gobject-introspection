#ifndef __GI_FUNCTION_H__
#define __GI_FUNCTION_H__

#include <glib.h>
#include <libguile.h>
#include <girepository.h>

extern scm_t_bits function_info_t;

G_GNUC_INTERNAL void
gi_function_init (void);

#endif
