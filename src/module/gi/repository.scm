(define-module (gi repository)
  #:use-module (gi info)
  #:use-module (glib utils)
  #:use-module (gobject gtype)

  #:use-module (oop goops)
  #:use-module (system foreign)

  #:export (gi-repository-get-default
            <gi-repository>
            <gi-typelib>
            find-by-gtype
            find-by-name
            require
            get-infos))


(define gi-lib "/home/steve/Source/guile-gobject-introspection/src/.libs/gobject-introspection")

(eval-when (expand load eval)
  (dynamic-call "ggi_repository_init"
              (dynamic-link "/home/steve/Source/guile-gobject-introspection/src/.libs/gobject-introspection")))

(define-method (require (repository <gi-repository>) (namespace <symbol>))
  (%gi-repository-require repository namespace))

(define-method (get-infos (repository <gi-repository>) (namespace <symbol>))
  (%gi-repository-get-infos repository namespace))
(define-method (find-by-gtype (repository <gi-repository>) (gtype <gtype-class>))
  (%gi-repository-find-by-gtype repository gtype))
(define-method (find-by-name (repository <gi-repository>) (namespace <symbol>) (name <symbol>))
  (%gi-repository-find-by-name repository namespace name))
