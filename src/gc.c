#include "support.h"
#include "gc.h"
#include "gutil.h"

static GMutex glib_gc_marker_hash_lock;
static GHashTable *glib_gc_marker_hash = NULL;

static SCM scm_sys_glib_gc_marker;
static scm_t_bits scm_tc16_glib_gc_marker;

gpointer
scm_glib_gc_protect_object (SCM obj)
{
  gpointer key = SCM_TO_GPOINTER (obj);
  gpointer val;

  g_mutex_lock (&glib_gc_marker_hash_lock);
  val = g_hash_table_lookup (glib_gc_marker_hash, key);
  g_hash_table_insert (glib_gc_marker_hash, key,
                       GINT_TO_POINTER (GPOINTER_TO_INT (val)+1));
  g_mutex_unlock (&glib_gc_marker_hash_lock);

  return key;
}

void
scm_glib_gc_unprotect_object (gpointer key)
{
  gpointer val;

  g_mutex_lock (&glib_gc_marker_hash_lock);
  val = g_hash_table_lookup (glib_gc_marker_hash, key);
  /* FIXME: is this right? */
  if (val)
    g_hash_table_insert (glib_gc_marker_hash, key,
                         GINT_TO_POINTER (GPOINTER_TO_INT (val)-1));
  else
    g_hash_table_remove (glib_gc_marker_hash, key);
  g_mutex_unlock (&glib_gc_marker_hash_lock);
}

static void
mark (gpointer key, gpointer val, gpointer user_data)
{
  scm_gc_mark (GPOINTER_TO_SCM (key));
}

static SCM
glib_gc_marker_mark (SCM smob)
{
  g_mutex_lock (&glib_gc_marker_hash_lock);
  g_hash_table_foreach (glib_gc_marker_hash, mark, NULL);
  g_mutex_unlock (&glib_gc_marker_hash_lock);

  return SCM_BOOL_F;
}

static int
glib_gc_marker_print (SCM smob, SCM port, scm_print_state *print_state)
{
  scm_puts ("#<g-lib-gc-marker>", port);
  return 1;
}

void
scm_gobject_gc_init (void)
{
  scm_tc16_glib_gc_marker = scm_make_smob_type ("g-lib-gc-marker", 0);
  scm_set_smob_mark (scm_tc16_glib_gc_marker, glib_gc_marker_mark);
  scm_set_smob_print (scm_tc16_glib_gc_marker, glib_gc_marker_print);

  glib_gc_marker_hash = g_hash_table_new (NULL, NULL);

  SCM_NEWSMOB (scm_sys_glib_gc_marker, scm_tc16_glib_gc_marker, NULL);
  scm_permanent_object (scm_sys_glib_gc_marker);
}
