/* -*- Mode: C; c-basic-offset: 4 -*-
 * vim: tabstop=4 shiftwidth=4 expandtabo
 */

#ifndef __GGI_FUNCTION_H__
#define __GGI_FUNCTION_H__

#include <libguile.h>

#include <girepository.h>

extern SCM ggi_funtion_type;
typedef SCM (*ggi_gsubr_t)(void);

G_BEGIN_DECLS

void
ggi_define_module_function (GIBaseInfo *info);

G_END_DECLS

#endif
