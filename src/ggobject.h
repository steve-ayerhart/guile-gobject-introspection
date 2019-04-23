/* -*- Mode: C; c-basic-offset: 4 -*-
 * vim: tabstop=4 shiftwidth=4 expandtab
 */
#ifndef _GGOBJECT_H_
#define _GGOBJECT_H_

#include <libguile.h>

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

// GGClosure is a _private_ structure
typedef void (* GGClosureExceptionHandler) (GValue *ret, guint n_param_values, const GValue *params);
typedef struct _GGClosure GGClosure;
typedef struct _GGObjectData GGobjectData;

struct _GGClosure {
  GClosure closure;
  SCM callback;
  SCM extra_args; // list of args to pass to callback
  SCM swap_data; // other object for gtk_signal_connect_object
  GGClosureExecptionHandler exception_handler;
};

typedef enum {
              GGOBJECT_USING_TOGGLE_REF = 1 << 0,
              GGOBJECT_IS_FLOATING_REF = 1 << 1,
              GGOBJECT_GOBJECT_WAS_FLOATING = 1 << 2
} GGObjectFlags;

G_END_DECLS

#endif
