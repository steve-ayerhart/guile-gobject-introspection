(define-module (gobject gtype)
  #:use-module (oop goops)

  #:use-module (glib utils)

  #:export (<gtype-class>
            <gtype-instance>
            gruntime-error
            gtype-instance-destroy!))

(eval-when (expand load eval)
  (load-extension "/home/steve/Source/guile-gobject-introspection/src/.libs/gobject-introspection"
                  "scm_gobject_gc_init")
  (load-extension "/home/steve/Source/guile-gobject-introspection/src/.libs/gobject-introspection"
                  "scm_gobject_gtype_init"))


;;; base class

(define-class <gtype-class> (<class>)
  (gtype #:class <foreign-slot>))

;(define-method (initialize (class <gtype-class>) initargs)
;  (let ((gtype-name (or (get-keyword #:gtype-name initargs #f)
;                         (gruntime-error "Need #:gtype-name initarg: ~a"
;                                          (pk initargs)))))
    ;; allow gtype-name of #t for base classes without gtypes (e.g. <gtype-instance>)
;    (if (not (eq? gtype-name #t))
;        (gtype-class-bind class gtype-name))
;    (next-method)
;    (gtype-class-inherit-magic class)))

(define-method (initialize (class <gtype-class>) initargs)
  (next-method)
  (%gtype-initialize class initargs))

(define-method (write (class <gtype-class>) p)
  (format p "#<~a ~a>" (class-name (class-of class)) (class-name class)))

(eval-when (expand load eval)
  (dynamic-call "scm_gobject_gtype_class_init"
                (dynamic-link "/home/steve/Source/guile-gobject-introspection/src/.libs/gobject-introspection")))

(define-class <gtype-instance> ()
  (gtype-instance #:class <read-only-slot>)
  #:gtype-name #t
  #:metaclass <gtype-class>)

(define-method (initialize (instance <gtype-instance>) initargs)
  (next-method)
  (gtype-instance-construct instance initargs))

(eval-when (expand load eval)
  (dynamic-call "scm_gobject_gtype_instance_init"
                (dynamic-link "/home/steve/Source/guile-gobject-introspection/src/.libs/gobject-introspection")))
