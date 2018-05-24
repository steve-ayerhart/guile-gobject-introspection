#ifndef __GUILE_GOBJECT_GSIGNAL_H__
#define __GUILE_GOBJECT_GSIGNAL_H__

#include "gclosure.h"

G_BEGIN_DECLS

SCM scm_gtype_get_signals (SCM type);
SCM scm_gsignal_create (SCM signal, SCM closure);
SCM scm_gsignal_handler_block (SCM instance, SCM handler_id);
SCM scm_gsignal_handler_unblock (SCM instance, SCM handler_id);
SCM scm_gsignal_handler_disconnect (SCM instance, SCM handler_id);
SCM scm_gsignal_handler_connected_p (SCM instance, SCM handler_id);

G_END_DECLS

#endif
