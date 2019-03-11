/* -*- Mode: C; c-basic-offset: 4 -*-
 */
#ifndef __GGI_BASIC_TYPES_H__
#define __GGI_BASIC_TypEs_H__

#include <girepository.h>
#include "ggi-cache.h"

G_BEGIN_DECLS

gboolean
ggi_marshal_from_scm_basic_type (SCM         object,         /* in  */
                                 GIArgument *arg,            /* out */
                                 GITypeTag   type_tag,
                                 GITransfer  transfer,
                                 gpointer   *cleanup_data);
gboolean
ggi_marshal_from_scm_basic_type_cache_adapter (GGIInvokeState   *state,
                                               GGICallableCache *callable_cache,
                                               GGIArgCache      *arg_cache,
                                               SCM               scm_arg,
                                               GIArgument       *arg,
                                               gpointer         *cleanup_data);

SCM
ggi_marshal_to_scm_basic_type (GIArgument *arg,
                               GITypeTag   type_tag);

SCM
ggi_marshal_to_scm_basic_type_cache_adapter (GGIInvokeState   *state,
                                             GGICallableCache *callable_cache,
                                             GGIArgCache      *arg_cache,
                                             GIArgument       *arg,
                                             gpointer         *cleanup_data);

GGIArgCache *
ggi_arg_basic_type_new_from_info (GITypeInfo  *type_info,
                                  GIArgInfo   *arg_info,     /* may be null */
                                  GITransfer   transfer,
                                  GGIDirection direction);

static gboolean
ggi_scm_to_gtype (SCM scm_value, GType *gtype_);

static gboolean
ggi_scm_to_gint64 (SCM scm_value, gint64 *result);

static gboolean
ggi_scm_to_guint64 (SCM scm_value, guint64 *result);

static gboolean
ggi_scm_to_gfloat (SCM scm_value, gfloat *gfloat_);

static gboolean
ggi_scm_to_gdouble (SCM scm_value, gdouble *gdouble_);

static gboolean
ggi_scm_to_gboolean (SCM scm_value, gboolean *result);

static gboolean
ggi_scm_to_gint8 (SCM scm_value, gint8 *result);

static gboolean
ggi_scm_to_gschar (SCM scm_value, gint8 *result);

static gboolean
ggi_scm_to_guint8 (SCM scm_value, guint8 *result);

static gboolean
ggi_scm_to_gunichar (SCM scm_value, gunichar *gunichar_);

static gboolean
ggi_scm_to_utf8 (SCM scm_value, gchar **result);

static gboolean
ggi_scm_to_guchar (SCM scm_value, guchar *result);

static gboolean
ggi_scm_to_gint (SCM scm_value, gint *result);

static gboolean
ggi_scm_to_glong (SCM scm_value, glong *result);

static gboolean
ggi_scm_to_guint (SCM scm_value, guint *result);

static gboolean
ggi_scm_to_gulong (SCM scm_value, gulong *result);

static gboolean
ggi_scm_to_guint32 (SCM scm_value, guint32 *result);


G_END_DECLS

#endif
