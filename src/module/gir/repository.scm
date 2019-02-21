(define-module (gir repository)
  #:use-module (gir info)
  #:use-module (glib utils)
  #:use-module (gobject gtype)

  #:use-module (oop goops)
  #:use-module (system foreign)

  #:export (find-by-g-type
            require
            get-infos))

(define gi-lib "/home/steve/Source/guile-gobject-introspection/src/.libs/gobject-introspection")

(eval-when (expand load eval)
  (dynamic-call "gi_repository_init"
              (dynamic-link "/home/steve/Source/guile-gobject-introspection/src/.libs/gobject-introspection")))

(define-method (require (repository <gi-repository>) (namespace <symbol>))
  (%gi-repository-require repository namespace))

(define-method (get-infos (repository <gi-repository>) (namespace <symbol>))
  (%gi-repository-get-infos repository namespace))
(define-method (find-by-g-type (repository <gi-repository>) (gtype <gtype-class>))
  (%gi-repository-find-by-gtype repository gtype))
