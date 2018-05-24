#include <string.h>

#include "support.h"

char*
scm_to_locale_string_dynwind (SCM s)
{
  char *ret = scm_to_locale_string (s);
  scm_dynwind_free (ret);
  return ret;
}

char*
scm_to_utf8_stringn_dynwind (SCM s, size_t *lenp)
{
  char *ret = scm_to_utf8_stringn (s, lenp);
  scm_dynwind_free (ret);
  return ret;
}

char*
scm_symbol_chars (SCM s)
{
  return scm_to_locale_string (scm_symbol_to_string (s));
}

char*
scm_symbol_chars_dynwind (SCM s)
{
  char *ret = scm_symbol_chars (s);
  scm_dynwind_free (ret);
  return ret;
}

char*
scm_keyword_chars (SCM s)
{
  return scm_symbol_chars (scm_keyword_to_symbol (s));
}

char*
scm_keyword_chars_dynwind (SCM s)
{
  return scm_symbol_chars_dynwind (scm_keyword_to_symbol (s));
}

typedef struct {
  void *func;
  void *p[5];
  unsigned int u[3];
  int d[3];
  const void *c[4];
} arg_data;

static void*
_invoke_v__p_p (void *p)
{
  arg_data *a = p;
  void (*func)(void*, void*) = a->func;
  func(a->p[0], a->p[1]);
  return NULL;
}

void
scm_dynwind_guile_v__p_p (void* (*dynwind)(void*(*)(void*), void*),
                          void *func, void *arg1, void *arg2)
{
  arg_data args = {func, {arg1, arg2,},};
  dynwind (_invoke_v__p_p, &args);
}

static void*
_invoke_v__p_p_p_p_p (void *p)
{
  arg_data *a = p;
  void (*func)(void*, void*, void*, void*, void*) = a->func;
  func(a->p[0], a->p[1], a->p[2], a->p[3], a->p[4]);
  return NULL;
}

void
scm_dynwind_guile_v__p_p_p_p_p (void* (*dynwind)(void*(*)(void*), void*),
                                void *func, void *arg1, void *arg2,
                                void *arg3, void *arg4, void *arg5)
{
  arg_data args = {func, {arg1, arg2, arg3, arg4, arg5},};
  dynwind (_invoke_v__p_p_p_p_p, &args);
}

static void*
_invoke_v__p_u_p_p (void *p)
{
  arg_data *a = p;
  void (*func)(void*, unsigned int, void*, void*) = a->func;
  func(a->p[0], a->u[0], a->p[1], a->p[2]);
  return NULL;
}

void
scm_dynwind_guile_v__p_u_p_p (void* (*dynwind)(void*(*)(void*), void*),
                              void *func, void *arg1, unsigned int arg2,
                              void *arg3, void *arg4)
{
  arg_data args = {func, {arg1, arg3, arg4,}, {arg2,},};
  dynwind (_invoke_v__p_u_p_p, &args);
}

static void*
_invoke_v__p_u_c_p (void *p)
{
  arg_data *a = p;
  void (*func)(void*, unsigned int, const void*, void*) = a->func;
  func(a->p[0], a->u[0], a->c[0], a->p[1]);
  return NULL;
}

void
scm_dynwind_guile_v__p_u_c_p (void* (*dynwind)(void*(*)(void*), void*),
                              void *func, void *arg1, unsigned int arg2,
                              const void *arg3, void *arg4)
{
  arg_data args = {func, {arg1, arg4,}, {arg2,}, {0,}, {arg3,}};
  dynwind (_invoke_v__p_u_c_p, &args);
}
