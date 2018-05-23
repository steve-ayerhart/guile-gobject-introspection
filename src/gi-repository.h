#ifndef __GI_REPOSITORY_H__
#define __GI_REPOSITORY_H__

#include <glib.h>
#include <libguile.h>
#include <girepository.h>

#include "gi-repository.h"
#include "gi-types.h"

extern scm_t_bits repository_t;
extern scm_t_bits typelib_t;

G_GNUC_INTERNAL void
gi_repository_init (void);

#endif
