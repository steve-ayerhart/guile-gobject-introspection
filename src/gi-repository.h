#ifndef __GI_REPOSITORY_H__
#define __GI_REPOSITORY_H__

#include <glib.h>
#include <libguile.h>
#include <girepository.h>

static SCM scm_girepository_class;
static SCM scm_gitypelib_class;

void
gi_repository_init (void);

#endif
