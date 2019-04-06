/* -*- Mode: C; c-basic-offset: 4 -*-
 */
#ifndef __GGI_VALUE_H__
#define __GGI_VALUE_H__

#include <libguile.h>
#include <glib-object.h>
#include <girepository.h>

G_BEGIN_DECLS

GIArgument
_ggi_argument_from_g_value (const GValue *value, GITypeInfo *type_info);

int
ggi_value_from_scmobject (GValue *value, SCM scm_obj);
int
ggi_value_from_scmobject_with_error (GValue *value, SCM scm_obj);
SCM
ggi_value_as_scmobject (const GValue *value, SCM scm_obj);
int
ggi_param_gvalue_from_scmobject (GValue *value, SCM scm_obj, const GParamSpec *pspec);
SCM
ggi_param_gvalue_as_scmobject (const GValue *gvalue, gboolean copy_boxed, const GParamSpec *pspec);
SCM
ggi_strv_from_gvalue (const GValue *value);
int
ggi_strv_to_gvalue (GValue *gvalue, SCM scm_object);
SCM
ggi_value_to_scm_basic_type (const GValue *value, GType fundamental, gboolean *handled);

G_END_DECLS

#endif
