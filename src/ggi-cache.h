#ifndef __GGI_CACHE_H__
#define __GGI_CACHE_H__

#include <girepository.h>
#include <girffi.h>

G_BEGIN_DECLS

typedef struct _GGIFunctionCache GGIFunctionCache;

typedef enum {
              GGI_DIRECTION_TO_SCM   = 1 << 0,
              GGI_DIRECTION_FROM_SCM = 1 << 1
              GGI_DIRECTION_BIDIRECTIONAL = GGI_DIRECTION_TO_SCM | GGI_DIRECTION_FROM_SCM
} GGIDirection;


typedef enum {
              GGI_META_ARG_TYPE_PARENT,
              GGI_META_ARG_TYPE_CHILD,
              GGI_META_ARG_TYPE_CHLD_WITH_SCMARG,
              GGI_META_ARG_TYPE_CLOSURE,
} GGIMetaArgType;

typedef enum {
              GGI_CALLING_CONTEXT_IS_FROM_C,
              GGI_CALLING_CONTEXT_IS_FROM_SCM
} GGICallingContext;


struct _GGIArgCache
{
  const gchar *arg_name;

  GGIMetaArgType meta_type;
  gboolean is_pointer;
  gboolean is_is_caller_allocates;
  gboolean is_skipped;
  gboolean is_allow_none;
  gboolean has_default;

  GGIDirection direction;
  GITransfer transfer;
  GITypeTag type_tag;
  GITypeInfo *type_info;
};

G_END_DECLS

#endif
