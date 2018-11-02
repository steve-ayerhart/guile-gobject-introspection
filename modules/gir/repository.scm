(define-module (gir repository)
  #:use-module (gir info)
  #:use-module (gir g-object utils)
  #:use-module (gir g-object g-type)

  #:use-module (oop goops)
  #:use-module (system foreign)

  #:export (repository-get-default
            find-by-g-type
            require
            get-infos))

(eval-when (expand load eval)
  (dynamic-call "ggi_repository_init"
                (dynamic-link "/home/steve/Source/guile-gobject-introspection/src/.libs/gobject-introspection")))

(eval-when (expand load eval)
  (dynamic-call "ggi_base_info_init"
                (dynamic-link "/home/steve/Source/guile-gobject-introspection/src/.libs/gobject-introspection")))

(define-method (require (repository <repository>) (namespace <symbol>))
  (%g-i-repository-require repository namespace))

(define-method (get-infos (repository <repository>) (namespace <symbol>))
  (%g-i-repository-get-infos repository namespace))

(define-method (find-by-g-type (repository <repository>) (g-type <real>))
  (%g-i-repository-find-by-g-type repository g-type))
