#include "gi-repository.h"
#include "gi-types.h"
#include "gi-registered-type.h"
#include "gi-object.h"
#include "gi-function.h"
#include "gi-callable.h"
#include "gi-argument.h"

void
gir_init ()
{
  gi_repository_init ();
  gi_types_init ();
  gi_registered_type_init ();
  gi_object_init ();
  gi_function_init ();
  gi_callable_init ();
  gi_argument_init ();
}
