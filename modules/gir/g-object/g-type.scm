(define-module (gir g-object g-type)
  #:use-module (oop goops)

  #:use-module (gir g-object utils)

  #:export (<g-type-class>
            <g-type-instance>
            g-runtime-error
            g-type-instance-destroy!))

(eval-when (expand load eval)
  (load-extension "/home/steve/Source/guile-gobject-introspection/src/.libs/gobject-introspection"
                  "scm_gobject_gc_init")
  (load-extension "/home/steve/Source/guile-gobject-introspection/src/.libs/gobject-introspection"
                  "scm_gobject_gtype_init"))


;;; base class

(define-class <g-type-class> (<class>)
  (g-type #:class <foreign-slot>))

(define-method (initialize (class <g-type-class>) initargs)
  (let ((g-type-name (or (get-keyword #:g-type-name initargs #f)
                         (g-runtime-error "Need #:g-type-name initarg: ~a"
                                          (pk initargs)))))
    ;; allow g-type-name of #t for base classes without g-types (e.g. <g-type-instance>)
    (if (not (eq? g-type-name #t))
        (g-type-class-bind class g-type-name))
    (next-method)
    (g-type-class-inherit-magic class)))

(define-method (write (class <g-type-class>) p)
  (format p "#<~a ~a>" (class-name (class-of class)) (class-name class)))

(eval-when (expand load eval)
  (dynamic-call "scm_gobject_gtype_class_init"
                (dynamic-link "/home/steve/Source/guile-gobject-introspection/src/.libs/gobject-introspection")))

(define-class <g-type-instance> ()
  (g-type-instance #:class <read-only-slot>)
  #:g-type-name #t
  #:metaclass <g-type-class>)

(define-method (initialize (instance <g-type-instance>) initargs)
  (next-method)
  (g-type-instance-construct instance initargs))

(eval-when (expand load eval)
  (dynamic-call "scm_gobject_gtype_instance_init"
                (dynamic-link "/home/steve/Source/guile-gobject-introspection/src/.libs/gobject-introspection")))
