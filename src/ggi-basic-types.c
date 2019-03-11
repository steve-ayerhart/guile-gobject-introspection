/* -*- Mode: C; c-basic-offset: 4 -*-
 *
 */

#include <libguile.h>
#include "ggi-basic-types.h"
#include "ggi-argument.h"
#include "gtype.h"

#if defined(G_OS_WIN32)
#include <float.h>
static boolean
ggi_is_finite (gdouble value)
{
    return _finite(value);
}
#else
#include <math.h>
static gboolean
ggi_is_finite (double value)
{
    return isfinite (value);
}
#endif

gboolean
ggi_marshal_from_scm_basic_type_cache_adapter (GGIInvokeState   *state,
                                               GGICallableCache *callable_cache,
                                               GGIArgCache      *arg_cache,
                                               SCM               scm_arg,
                                               GIArgument       *arg,
                                               gpointer         *cleanup_data)
{
    return ggi_marshal_from_scm_basic_type (scm_arg,
                                            arg,
                                            arg_cache->type_tag,
                                            arg_cache->transfer,
                                            cleanup_data);
}

SCM
ggi_marshal_to_scm_basic_type_cache_adapter (GGIInvokeState *state,
                                             GGICallableCache *callable_cache,
                                             GGIArgCache *arg_cache,
                                             GIArgument *arg,
                                             gpointer *cleanup_data)
{
    return ggi_marshal_to_scm_basic_type (arg,
                                          arg_cache->type_tag);

}

gboolean
ggi_scm_to_utf8 (SCM scm_value, gchar **utf8)
{
    g_debug ("ggi_scm_to_utf8");

    gchar *string_;

    string_ = scm_to_locale_string (scm_value);

    *utf8 = string_;

    return TRUE;
}

static gboolean
ggi_gpointer_from_scm (SCM scm_arg, gpointer *gpointer_)
{
    if (scm_is_false (scm_arg))
        {
            *gpointer_ = NULL;
            return TRUE;
        }
    else if (scm_is_exact_integer (scm_arg))
        {
            *gpointer_ = (gpointer) scm_to_uintptr_t (scm_arg);
            return TRUE;
        }
    else if (scm_is_bytevector (scm_arg) && SCM_BYTEVECTOR_LENGTH (scm_arg) >= 8)
        {
            *gpointer_ =  SCM_BYTEVECTOR_CONTENTS (scm_arg);
            return TRUE;
        }
    else
        {
            scm_misc_error ("type marshal",
                            "cannot convert '~a' to gpointer",
                            scm_list_1 (scm_arg));
            return FALSE;
        }
}




static gboolean
marshal_from_scm_void (GGIInvokeState   *state,
                       GGICallableCache *callable_cache,
                       GGIArgCache      *arg_cache,
                       SCM               scm_arg,
                       GIArgument       *arg,
                       gpointer         *cleanup_data)
{
    g_warn_if_fail (arg_cache->transfer == GI_TRANSFER_NOTHING);

    if (ggi_gpointer_from_scm (scm_arg, &(arg->v_pointer)))
        {
            *cleanup_data = arg->v_pointer;
            return TRUE;
        }

    return FALSE;
}

static SCM
marshal_to_scm_void (GGIInvokeState *state,
                     GGICallableCache *callable_cache,
                     GGIArgCache      *arg_cache,
                     GIArgument       *arg,
                     gpointer         *cleanup_data)
{
    if (arg_cache->is_pointer)
        return SCM_UNSPECIFIED;
}

gboolean
ggi_scm_to_gdouble (SCM scm_value, gdouble *gdouble_)
{
    if (scm_is_false (scm_number_p (scm_value)))
        {
            scm_misc_error ("type marshal",
                            "Must be a number, not '~a'",
                            scm_list_1 (scm_value));
            return FALSE;
        }

    *gdouble_ = scm_to_double (scm_value);

    return TRUE;
}

gboolean
ggi_scm_to_gunichar (SCM scm_value, gunichar *gunichar_)
{
    // TODO: handle bytevector ?
    if (scm_is_true (scm_char_p (scm_value)))
        {
            *gunichar_ = (gunichar) scm_to_uint32 (scm_char_to_integer (scm_value));

            return TRUE;
        }
    else if (scm_is_true (scm_string_p (scm_value)))
        {
            SCM scm_char_list = scm_string_to_list (scm_value);

            if (1 == scm_to_long (scm_length (scm_char_list)))
                {
                    *gunichar_ = (gunichar) scm_to_uint32 (scm_char_to_integer (scm_list_ref (scm_value, 0)));

                    return TRUE;
                }
            else
                {
                    scm_misc_error ("type marshal",
                                    "Must be a 1 character string, not ~d",
                                    scm_list_1 (scm_length (scm_char_list)));

                    return FALSE;
                }
        }
    else if (scm_is_exact_integer (scm_value))
        {
            // TODO: assuming this tests code points. find better way?
            *gunichar_ = (gunichar) scm_to_uint32 (scm_char_to_integer (scm_integer_to_char (scm_value)));

            return TRUE;
        }
    else
        {
            scm_misc_error ("type marshal",
                            "cannot convert  '~a' to guinchar",
                            scm_list_1 (scm_value));

            return FALSE;
        }
}

gboolean
ggi_scm_to_gfloat (SCM scm_value, gfloat *gfloat_)
{
    gdouble double_;

    if (scm_is_false (scm_number_p (scm_value)))
        {
            scm_misc_error ("type marshal",
                            "Must be a number, not '~a'",
                            scm_list_1 (scm_value));
            return FALSE;
        }

    double_ = scm_to_double (scm_value);

    if (ggi_is_finite (double_) && (double_ < -G_MAXFLOAT || double_ > G_MAXFLOAT))
        {
            scm_misc_error ("type marshal",
                            "Cannot convert '~a' to gfloat, out of range",
                            scm_list_1 (scm_value));
            return FALSE;
        }

    *gfloat_ = (gfloat) double_;

    return TRUE;
}

gboolean
ggi_scm_to_gtype (SCM scm_value, GType *gtype_)
{
    if (!SCM_GTYPE_CLASSP (scm_value))
        {
            scm_misc_error ("type marshal",
                            "Must be a guile Gtype, not '~a'",
                            scm_list_1 (scm_value));
            return FALSE;
        }

    *gtype_ = scm_c_gtype_class_to_gtype (scm_value);

    return TRUE;
}

gboolean
ggi_scm_to_gboolean (SCM scm_value, gboolean *gboolean_)
{
    if (scm_is_bool (scm_value))
        {
            *gboolean_ = (gboolean) scm_to_bool (scm_value);

            return TRUE;
        }

    return FALSE;
}

gboolean
ggi_scm_to_gint8 (SCM scm_value, gint8 *gint8_)
{
    // TODO handle unicode?

    long long_int;

    // TODO: use exact integer?
    if (!scm_is_integer (scm_value))
        return FALSE;

    long_int = scm_to_long (scm_value);

    if (long_int < G_MININT8 || long_int > G_MAXINT8)
        {
            // TODO misc error
            return FALSE;
        }

    *gint8_ = (gint8) long_int;

    return TRUE;
}

gboolean
ggi_scm_to_guint8 (SCM scm_value, guint8 *guint_)
{
    unsigned long long_int;

    // TODO: use exact integer?
    if (!scm_is_integer (scm_value))
        return FALSE;

    long_int = scm_to_ulong (scm_value);

    if (long_int > G_MAXUINT8)
        {
            // TODO misc error
            return FALSE;
        }

    *guint_ = (guint) long_int;

    return TRUE;
}

gboolean
ggi_scm_to_gint16 (SCM scm_value, gint16 *gint16_)
{
    long long_int;

    // TODO: use exact integer?
    if (!scm_is_integer (scm_value))
        return FALSE;

    long_int = scm_to_long (scm_value);

    if (long_int < G_MININT16 || long_int > G_MAXINT16)
        {
            // TODO misc error
            return FALSE;
        }

    *gint16_ = (gint16) long_int;

    return TRUE;
}

gboolean
ggi_scm_to_guint16 (SCM scm_value, guint16 *guint16_)
{
    unsigned long long_int;

    // TODO: use exact integer?
    if (!scm_is_integer (scm_value))
        return FALSE;

    long_int = scm_to_ulong (scm_value);

    if (long_int > G_MAXUINT16)
        {
            // TODO misc error
            return FALSE;
        }

    *guint16_ = (guint) long_int;

    return TRUE;
}

gboolean
ggi_scm_to_gint32 (SCM scm_value, gint32 *gint32_)
{
    long long_int;

    // TODO: use exact integer?
    if (!scm_is_integer (scm_value))
        return FALSE;

    long_int = scm_to_long (scm_value);

    if (long_int < G_MININT32 || long_int > G_MAXINT32)
        {
            // TODO misc error
            return FALSE;
        }

    *gint32_ = (gint32) long_int;

    return TRUE;
}

gboolean
ggi_scm_to_guint32 (SCM scm_value, guint32 *guint32_)
{
    unsigned long long_int;

    // TODO: use exact integer?
    if (!scm_is_integer (scm_value))
        return FALSE;

    long_int = scm_to_ulong (scm_value);

    if (long_int > G_MAXUINT32)
        {
            // TODO misc error
            return FALSE;
        }

    *guint32_ = (guint32) long_int;

    return TRUE;
}

gboolean
ggi_scm_to_gint (SCM scm_value, gint *gint_)
{
    long long_int;

    // TODO: use exact integer?
    if (!scm_is_integer (scm_value))
        return FALSE;

    long_int = scm_to_long (scm_value);

    if (long_int < G_MININT || long_int > G_MAXINT)
        {
            // TODO misc error
            return FALSE;
        }

    *gint_ = (gint) long_int;

    return TRUE;
}

gboolean
ggi_scm_to_guint (SCM scm_value, guint *guint_)
{
    unsigned long long_int;

    // TODO: use exact integer?
    if (!scm_is_integer (scm_value))
        return FALSE;

    long_int = scm_to_ulong (scm_value);

    if (long_int > G_MAXUINT)
        {
            // TODO misc error
            return FALSE;
        }

    *guint_ = (guint) long_int;

    return TRUE;
}

gboolean
ggi_scm_to_glong (SCM scm_value, glong *glong_)
{
    long long_int;

    *glong_ = (glong) scm_to_long (scm_value);

    return TRUE;
}

gboolean
ggi_scm_to_gulong (SCM scm_value, gulong *gulong_)
{
    long long_int;

    *gulong_ = (gulong) scm_to_ulong (scm_value);

    return TRUE;
}

gboolean
ggi_marshal_from_scm_basic_type (SCM scm_value,
                                 GIArgument *arg,
                                 GITypeTag type_tag,
                                 GITransfer transfer,
                                 gpointer *cleanup_data)
{
    switch (type_tag)
        {
        case GI_TYPE_TAG_VOID:
            g_warn_if_fail (transfer == GI_TRANSFER_NOTHING);
            if (ggi_gpointer_from_scm (scm_value, &(arg->v_pointer)))
                {
                    *cleanup_data = arg->v_pointer;
                    return TRUE;
                }
            return FALSE;

        case GI_TYPE_TAG_DOUBLE:
            return ggi_scm_to_gdouble (scm_value, &(arg->v_double));

        case GI_TYPE_TAG_INT8:
            return ggi_scm_to_gint8 (scm_value, &(arg->v_int8));

        case GI_TYPE_TAG_UINT8:
            return ggi_scm_to_guint8 (scm_value, &(arg->v_uint8));

        case GI_TYPE_TAG_INT16:
            return ggi_scm_to_gint16 (scm_value, &(arg->v_int16));

        case GI_TYPE_TAG_UINT16:
            return ggi_scm_to_guint16 (scm_value, &(arg->v_uint16));

        case GI_TYPE_TAG_INT32:
            return ggi_scm_to_gint32 (scm_value, &(arg->v_int32));

        case GI_TYPE_TAG_UINT32:
            return ggi_scm_to_guint32 (scm_value, &(arg->v_uint32));

        case GI_TYPE_TAG_INT64:
            return ggi_scm_to_gint64 (scm_value, &(arg->v_int64));

        case GI_TYPE_TAG_UINT64:
            return ggi_scm_to_guint64 (scm_value, &(arg->v_uint64));

        case GI_TYPE_TAG_BOOLEAN:
            return ggi_scm_to_gboolean (scm_value, &(arg->v_boolean));

        case GI_TYPE_TAG_FLOAT:
            return ggi_scm_to_gfloat (scm_value, &(arg->v_float));

        case GI_TYPE_TAG_GTYPE:
            return ggi_scm_to_gtype (scm_value, &(arg->v_size));

        case GI_TYPE_TAG_UNICHAR:
            return ggi_scm_to_gunichar (scm_value, &(arg->v_uint32));

        case GI_TYPE_TAG_UTF8:
            if (ggi_scm_to_utf8 (scm_value, &(arg->v_string))) {
                *cleanup_data = arg->v_string;
                return TRUE;
            }
            return FALSE;

        case GI_TYPE_TAG_FILENAME:
            if (ggi_scm_to_utf8 (scm_value, &(arg->v_string))) {
                *cleanup_data = arg->v_string;
                return TRUE;
            }
            return FALSE;

        default:
            // todo scm error
            return FALSE;
        }

    return TRUE;
}

// marshalling to scheme is relatively straight forward.
SCM
ggi_marshal_to_scm_basic_type (GIArgument *arg,
                               GITypeTag   type_tag)
{
    switch (type_tag)
        {
        case GI_TYPE_TAG_BOOLEAN:
            return scm_from_bool (arg->v_boolean);

        case GI_TYPE_TAG_INT8:
            return scm_from_int8 (arg->v_int8);

        case GI_TYPE_TAG_UINT8:
            return scm_from_uint8 (arg->v_uint8);

        case GI_TYPE_TAG_INT16:
            return scm_from_int16 (arg->v_int16);

        case GI_TYPE_TAG_UINT16:
            return scm_from_uint16 (arg->v_uint16);

        case GI_TYPE_TAG_INT32:
            return scm_from_int32 (arg->v_int32);

        case GI_TYPE_TAG_UINT32:
            return scm_from_uint32 (arg->v_uint32);

        case GI_TYPE_TAG_INT64:
            return scm_from_int64 (arg->v_int64);

        case GI_TYPE_TAG_UINT64:
            return scm_from_uint64 (arg->v_uint64);

        case GI_TYPE_TAG_FLOAT:
            return scm_from_double ((double) arg->v_float);

        case GI_TYPE_TAG_DOUBLE:
            return scm_from_double (arg->v_double);

        case GI_TYPE_TAG_GTYPE:
            return scm_c_gtype_lookup_class (arg->v_size);

        case GI_TYPE_TAG_UNICHAR:
            return scm_integer_to_char (scm_from_uint32 (arg->v_uint32));

        case GI_TYPE_TAG_UTF8:
            if (!arg->v_string)
                return scm_c_make_string(0, SCM_MAKE_CHAR(0));
            else
                return scm_from_utf8_string (arg->v_string);

        case GI_TYPE_TAG_FILENAME:
            if (!arg->v_string)
                return scm_c_make_string(0, SCM_MAKE_CHAR(0));
            else
                return scm_from_locale_string (arg->v_string);

        default:
            scm_misc_error("argument marshal",
                           "Type tag %d not supported",
                           scm_list_1 (scm_from_size_t (type_tag)));
            return SCM_UNSPECIFIED;
        }
}

static gboolean
arg_basic_type_setup_from_info (GGIArgCache *arg_cache,
                                GITypeInfo  *type_info,
                                GIArgInfo   *arg_info,
                                GITransfer   transfer,
                                GGIDirection direction)
{
    GITypeTag type_tag = g_type_info_get_tag (type_info);

    if (!ggi_arg_base_setup (arg_cache, type_info, arg_info, transfer, direction))
        return FALSE;

    switch (type_tag)
        {
        case GI_TYPE_TAG_VOID:
            if (direction & GGI_DIRECTION_FROM_SCM)
                arg_cache->from_scm_marshaller = marshal_from_scm_void;

            if (direction & GGI_DIRECTION_TO_SCM)
                arg_cache->to_scm_marshaller = marshal_to_scm_void;

            break;
        case GI_TYPE_TAG_BOOLEAN:
            arg_cache->allow_none = TRUE;
            // keep on casing
        case GI_TYPE_TAG_INT8:
        case GI_TYPE_TAG_UINT8:
        case GI_TYPE_TAG_INT16:
        case GI_TYPE_TAG_UINT16:
        case GI_TYPE_TAG_INT32:
        case GI_TYPE_TAG_UINT32:
        case GI_TYPE_TAG_INT64:
        case GI_TYPE_TAG_UINT64:
        case GI_TYPE_TAG_FLOAT:
        case GI_TYPE_TAG_DOUBLE:
        case GI_TYPE_TAG_UNICHAR:
        case GI_TYPE_TAG_GTYPE:
            if (direction & GGI_DIRECTION_FROM_SCM)
                arg_cache->from_scm_marshaller = ggi_marshal_from_scm_basic_type_cache_adapter;

            if (direction & GGI_DIRECTION_TO_SCM)
                arg_cache->to_scm_marshaller = ggi_marshal_to_scm_basic_type_cache_adapter;

            break;
        case GI_TYPE_TAG_UTF8:
        case GI_TYPE_TAG_FILENAME:
            if (direction & GGI_DIRECTION_FROM_SCM)
                {
                    arg_cache->from_scm_marshaller = ggi_marshal_from_scm_basic_type_cache_adapter;
                    // TODO: cleanup
                }

            if (direction & GGI_DIRECTION_TO_SCM)
                {
                    arg_cache->to_scm_marshaller = ggi_marshal_to_scm_basic_type_cache_adapter;
                    // TODO: cleanup
                }

            break;
        default:
            g_assert_not_reached ();
        }

    return TRUE;
}

GGIArgCache *
ggi_arg_basic_type_new_from_info (GITypeInfo  *type_info,
                                  GIArgInfo   *arg_info,
                                  GITransfer   transfer,
                                  GGIDirection direction)
{
    gboolean result = FALSE;
    GGIArgCache *arg_cache = ggi_arg_cache_alloc ();

    result = arg_basic_type_setup_from_info (arg_cache,
                                             type_info,
                                             arg_info,
                                             transfer,
                                             direction);

    if (result)
        return arg_cache;

    ggi_arg_cache_free (arg_cache);
    return NULL;
}
