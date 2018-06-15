#include "gi-registered-type.h"
#include "gtype.h"

scm_t_bits registered_type_info_t;

void
gi_registered_type_init ()
{
  #ifndef SCM_MAGIC_SNARFER
  #include "gi-registered-type.x"
  #endif
}
