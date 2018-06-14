#ifndef __GI_REPOSITORY_H__
#define __GI_REPOSITORY_H__

#include <glib.h>
#include <libguile.h>
#include <girepository.h>

static SCM repository_t;
static SCM typelib_t;

G_GNUC_INTERNAL void
gi_repository_init (void);

#endif
