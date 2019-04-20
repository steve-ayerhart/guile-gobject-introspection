/* -*- Mode: C; c-basic-offset: 4 -*-
 * vim: tabstop=4 shiftwidth=4 expandtab
 */

#include <libguile.h>

#include "ggi-basic-types.h"
#include "gvalue.h"

GIArgument
_ggi_argument_from_g_value (const GValue *value,
                            GITypeInfo *type_info)
{
  GIArgument arg = {0, };

  GITypeTag type_tag = g_type_info_get_tag (type_info);

  switch (type_tag)
    {
    case GI_TYPE_TAG_BOOLEAN:
      arg.v_boolean = g_value_get_boolean (value);
      break;
    case GI_TYPE_TAG_INT8:
      arg.v_int8 = g_value_get_schar (value);
      break;
    case GI_TYPE_TAG_INT16:
    case GI_TYPE_TAG_INT32:
      if (g_type_is_a (G_VALUE_TYPE (value), G_TYPE_LONG))
        arg.v_int32 = (gint32)g_value_get_long (value);
      else
        arg.v_int32 = (gint32)g_value_get_int (value);
      break;
    case GI_TYPE_TAG_INT64:
      if (g_type_is_a (G_VALUE_TYPE (value), G_TYPE_LONG))
        arg.v_int64 = g_value_get_long (value);
      else
        arg.v_int64 = g_value_get_int64 (value);
      break;
    case GI_TYPE_TAG_UINT8:
      arg.v_uint8 = g_value_get_uchar (value);
      break;
    case GI_TYPE_TAG_UINT16:
    case GI_TYPE_TAG_UINT32:
      if (g_type_is_a (G_VALUE_TYPE (value), G_TYPE_ULONG))
        arg.v_uint32 = (guint32)g_value_get_ulong (value);
      else
        arg.v_uint32 = (guint32)g_value_get_uint (value);
      break;
    case GI_TYPE_TAG_UINT64:
      if (g_type_is_a (G_VALUE_TYPE (value), G_TYPE_ULONG))
        arg.v_uint64 = g_value_get_ulong (value);
      else
        arg.v_uint64 = g_value_get_uint64 (value);
      break;
    case GI_TYPE_TAG_UNICHAR:
      arg.v_uint32 = g_value_get_schar (value);
      break;
    case GI_TYPE_TAG_FLOAT:
      arg.v_float = g_value_get_float (value);
      break;
    case GI_TYPE_TAG_DOUBLE:
      arg.v_double = g_value_get_double (value);
      break;
    case GI_TYPE_TAG_GTYPE:
      arg.v_size = g_value_get_gtype (value);
      break;
    case GI_TYPE_TAG_UTF8:
    case GI_TYPE_TAG_FILENAME:
      arg.v_string = (char *) g_value_get_string (value);
      break;
    case GI_TYPE_TAG_GLIST:
    case GI_TYPE_TAG_GSLIST:
    case GI_TYPE_TAG_ARRAY:
    case GI_TYPE_TAG_GHASH:
      if (G_VALUE_HOLDS_BOXED (value))
        arg.v_pointer = g_value_get_boxed (value);
      else
        /* e. g. GSettings::change-event */
        arg.v_pointer = g_value_get_pointer (value);
      break;
    case GI_TYPE_TAG_INTERFACE:
      {
        GIBaseInfo *info;
        GIInfoType info_type;

        info = g_type_info_get_interface (type_info);
        info_type = g_base_info_get_type (info);

        g_base_info_unref (info);

        switch (info_type)
          {
          case GI_INFO_TYPE_FLAGS:
            arg.v_uint = g_value_get_flags (value);
            break;
          case GI_INFO_TYPE_ENUM:
            arg.v_int = g_value_get_enum (value);
            break;
          case GI_INFO_TYPE_INTERFACE:
          case GI_INFO_TYPE_OBJECT:
            if (G_VALUE_HOLDS_PARAM (value))
              arg.v_pointer = g_value_get_param (value);
            else
              arg.v_pointer = g_value_get_object (value);
            break;
          case GI_INFO_TYPE_BOXED:
          case GI_INFO_TYPE_STRUCT:
          case GI_INFO_TYPE_UNION:
            if (G_VALUE_HOLDS (value, G_TYPE_BOXED)) {
              arg.v_pointer = g_value_get_boxed (value);
            } else if (G_VALUE_HOLDS (value, G_TYPE_VARIANT)) {
              arg.v_pointer = g_value_get_variant (value);
            } else if (G_VALUE_HOLDS (value, G_TYPE_POINTER)) {
              arg.v_pointer = g_value_get_pointer (value);
            } else {
              scm_misc_error ("not implemented",
                              "Converting GValue's of type '~s' is not implemented.",
                              scm_from_locale_string (g_type_name (G_VALUE_TYPE (value))));
            }
            break;
          default:
            scm_misc_error ("not implemented",
                            "Converting GValue's of type '~s' is not implemented.",
                            scm_from_locale_string (g_info_type_to_string (info_type)));
            break;
          }
        break;
      }
    case GI_TYPE_TAG_ERROR:
      arg.v_pointer = g_value_get_boxed (value);
      break;
    case GI_TYPE_TAG_VOID:
      arg.v_pointer = g_value_get_pointer (value);
      break;
    default:
      break;
    }

  return arg;
}


