#ifndef __GI_TYPES_H__
#define __GI_TYPES_H__

#include <glib.h>
#include <libguile.h>
#include <girepository.h>

extern scm_t_bits base_info_t;
extern scm_t_bits struct_info_t;
extern scm_t_bits union_info_t;
extern scm_t_bits enum_info_t;
extern scm_t_bits interface_info_t;
extern scm_t_bits constant_info_t;
extern scm_t_bits value_info_t;
extern scm_t_bits signal_info_t;
extern scm_t_bits v_func_info_t;
extern scm_t_bits property_info_t;
extern scm_t_bits field_info_t;
extern scm_t_bits type_info_t;
extern scm_t_bits error_domain_info_t;
extern scm_t_bits unresolved_info_t;

G_GNUC_INTERNAL void
gi_types_init (void);


#endif
