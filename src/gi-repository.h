#ifndef __GI_REPOSITORY_H__
#define __GI_REPOSITORY_H__

#include <glib.h>
#include <libguile.h>
#include <girepository.h>

static SCM scm_repository_class;
static SCM scm_typelib_class;

void
gi_repository_init (void);

#endif
