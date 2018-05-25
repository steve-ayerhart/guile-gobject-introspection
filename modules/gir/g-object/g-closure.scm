(define-module (gir g-object g-closure)
;;  #:use-module (g-object config)
  #:use-module (gir g-object utils)
  #:use-module (gir g-object g-type)
  #:use-module (gir g-object g-value)
  #:use-module (oop goops)

  #:export (<g-closure> g-closure-invoke))

(eval-when (expand load eval)
  (dynamic-call "scm_gobject_gclosure_init"
                (dynamic-link "/home/steve/Source/guile-gobject-introspection/src/.libs/gobject-introspection")))

(define-class/docs <g-closure> (<g-boxed>)
  "The Scheme representation of a GLib closure: a typed procedure
object that can be passed to other languages." ;; FIXME say something about initargs
  #:g-type-name "GClosure")

;;;
;;; {Instance Allocation and Initialization}
;;;

(define-method (initialize (closure <g-closure>) initargs)
  ;; don't chain up, we do our own init
  (let ((return-type (get-keyword #:return-type initargs #f))
        (param-types (get-keyword #:param-types initargs '()))
        (func (get-keyword #:func initargs #f)))
    (g-closure-construct closure return-type param-types func)))
