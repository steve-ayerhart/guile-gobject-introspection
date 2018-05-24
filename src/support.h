#ifndef __GUILE_SUPPORT_H__
#define __GUILE_SUPPORT_H__

#include <glib.h>
#include <libguile.h>

G_BEGIN_DECLS

char* scm_to_locale_string_dynwind (SCM s);
char* scm_to_utf8_stringn_dynwind (SCM s, size_t *lenp);
char* scm_symbol_chars (SCM s);
char* scm_symbol_chars_dynwind (SCM s);
char* scm_keyword_chars (SCM s);
char* scm_keyword_chars_dynwind (SCM s);
void scm_dynwind_guile_v__p_p (void* (*dynwind)(void*(*)(void*), void*), void *func,
                               void *arg1, void *arg2);
void scm_dynwind_guile_v__p_p_p_p_p (void* (*dynwind)(void*(*)(void*), void*), void *func,
                                     void *arg1, void *arg2, void *arg3,
                                     void *arg4, void *arg5);
void scm_dynwind_guile_v__p_u_p_p (void* (*dynwind)(void*(*)(void*), void*), void *func,
                                   void *arg1, unsigned int arg2, void *arg3,
                                   void *arg4);
void scm_dynwind_guile_v__p_u_c_p (void* (*dynwind)(void*(*)(void*), void*), void *func,
                                   void *arg1, unsigned int arg2, const void *arg3,
                                   void *arg4);

G_END_DECLS

#endif
