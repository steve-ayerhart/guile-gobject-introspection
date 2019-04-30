(define-module (g-object g-type)
  #:use-module (oop goops)
  #:use-module (ice-9 format))

(eval-when (expand load eval)
  (dynamic-call "g_type_init"
                (dynamic-link "/home/steve/Source/guile-gobject-introspection/src/.libs/gobject-introspection")))


(define-method (initialize (self <g-type>) initargs)
  (next-method)
  (%g-type-initialize self initargs))

(define-method (write (self <g-type>) port)
  (%g-type-display self port))

(define-method (display (self <g-type>) port)
  (%g-type-display self port))
