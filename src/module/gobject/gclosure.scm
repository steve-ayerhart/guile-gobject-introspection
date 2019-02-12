(define-module (gobject gclosure)
  #:use-module (glib utils)
  #:use-module (gobject gtype)
  #:use-module (gobject gvalue)
  #:use-module (oop goops)

  #:export (<gclosure> gclosure-invoke))

(eval-when (expand load eval)
  (dynamic-call "scm_gobject_gclosure_init"
                (dynamic-link "/home/steve/Source/guile-gobject-introspection/src/.libs/gobject-introspection")))

(define-class/docs <gclosure> (<gboxed>)
  "The Scheme representation of a GLib closure: a typed procedure
object that can be passed to other languages." ;; FIXME say something about initargs
  #:gtype-name "GClosure")

;;;
;;; {Instance Allocation and Initialization}
;;;

(define-method (initialize (closure <gclosure>) initargs)
  ;; don't chain up, we do our own init
  (let ((return-type (get-keyword #:return-type initargs #f))
        (param-types (get-keyword #:param-types initargs '()))
        (func (get-keyword #:func initargs #f)))
    (gclosure-construct closure return-type param-types func)))
