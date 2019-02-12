(define-module (gobject)
  #:use-module ((srfi srfi-1) #:select (filter-map))
  #:use-module (oop goops)

  #:use-module (glib utils)
  #:use-module (gobject gtype)
  #:use-module (gobject gvalue)
  #:use-module (gobject gparameter)
  #:use-module (gobject gsignal)

  #:export (<gobject>
            <ginterface>
            <gparam-object>
            gtype-register-static
            get-property set-property
            gobject-class-get-properties gobject-class-find-property
            gobject-class-get-property-names
            gobject-get-property gobject-set-property))

(eval-when (expand load eval)
  (dynamic-call "scm_gobject_init"
                (dynamic-link "/home/steve/Source/guile-gobject-introspection/src/.libs/gobject-introspection")))

(define-class <gobject-class> (<gtype-class>))

(define-method (compute-slots (class <gobject-class>))
  (define (has-slot? name slots)
    (and (pair? slots)
         (or (eq? name (slot-definition-name (car slots)))
             (has-slot? name (cdr slots)))))
  (define (compute-extra-slots props slots)
    (filter-map (lambda (prop)
                  (and (not (has-slot? prop slots))
                       (if (defined? '<slot>)
                           (make <slot> #:name prop #:allocation #:gproperty)
                           `(,prop #:allocation #:gproperty))))
                props))
  (let* ((slots (next-method))
         (extra (compute-extra-slots
                 (gobject-class-get-property-names class) slots)))
    (with-accessors (direct-slots)
                    (set! (direct-slots class) (append (direct-slots class) extra)))
    (append slots extra)))

(define-method (compute-get-n-set (class <gobject-class>) slotdef)
  (let ((name (slot-definition-name slotdef)))
    (case (slot-definition-allocation slotdef)
      ((#:gproperty) (list (lambda (o) (gobject-get-property o name))
                            (lambda (o v) (gobject-set-property o name v))))
      (else (next-method)))))

(define-method (initialize (class <gobject-class>) initargs)
  (define (install-properties!)
    ;; To expose slots as gobject properties, <gobject> will process a
    ;; #:gparam slot option to create a new gobject property.
    (for-each
     (lambda (slot)
       (let ((pspec (get-keyword #:gparam (slot-definition-options slot) #f)))
         (if pspec
             (gobject-class-install-property
              class
              (apply make
                     (car pspec) #:name (slot-definition-name slot)
                     (cdr pspec))))))
     (class-direct-slots class)))

  (define (install-signals!)
    ;; We parse a #:gsignal initialization argument to install signals.
    (let loop ((args initargs))
      (if (not (null? args))
          (begin
            (if (eq? (car args) #:gsignal)
                (let ((signal (cadr args)))
                  (if (not (and (list? signal) (>= (length signal) 2)))
                      (gruntime-error "Invalid signal specification: ~A" signal))
                  (let* ((name (car signal))
                         (return-type (cadr signal))
                         (param-types (cddr signal))
                         (generic (gtype-class-create-signal class name return-type param-types)))
                    ;; Some magic to define the generic
                    (module-define! (current-module)
                                    (generic-function-name generic) generic))))
            (loop (cddr args))))))

  (define (first pred list)
    (cond ((null? list) #f)
          ((pred (car list)) (car list))
          (else (first pred (cdr list)))))
  (define (gobject-class? c)
    (memq <gobject> (class-precedence-list c)))

  ;; real work starts here

  (next-method
   class
   (cons*
    #:gtype-name
    (or (get-keyword #:gtype-name initargs #f)
        (gtype-register-static
         (class-name->gtype-name (get-keyword #:name initargs #f))
         (first gobject-class?
                (apply append
                       (map class-precedence-list
                            (get-keyword #:dsupers initargs '()))))))
    initargs))
  (install-properties!)
  (install-signals!))

(define-class/docs <gobject> (<gtype-instance>)
  "The base class for GLib's default object system.

@code{<gobject>}'s metaclass understands a new slot option,
@code{#:gparam}, which will export a slot as a @code{<gobject>}
property. The default implementation will set and access the value from
the slot, but you can customize this by writing your own methods for
@code{gobject:set-property} and @code{gobject:get-property}.

In addition, the metaclass also understands @code{#:gsignal} arguments,
which define signals on the class, and define the generics for the
default signal handler. See @code{gtype-class-define-signal} for more
information.

For example:
@lisp
 ;; deriving from <gobject>
 (define-class <test> (<gobject>)
  ;; a normal object slot
  my-data

  ;; an object slot exported as a gobject property
  (pub-data #:gparam (list <gparam-long> #:name 'test))

  ;; likewise, using non-default parameter settings
  (foo-data #:gparam (list <gparam-long> #:name 'foo
                           #:minimum -3 #:maximum 1000
                           #:default-value 42))

  ;; a signal with no arguments and no return value
  #:gsignal '(frobate #f)

  ;; a signal with arguments and a return value
  #:gsignal (list 'frobate <gboolean> <gint> <glong>))

 ;; deriving from <test> -- also inherits properties and signals
 (define-class <hungry> (<test>))
@end lisp

@code{<gobject>} classes also expose a slot for each GObject property
defined on the class, if such a slot is not already defined.
"
  ;; add a slot for signal generics instead of module-define! ?
  #:metaclass <gobject-class>
  #:gtype-name "GObject")

(define-class/docs <ginterface> (<gtype-instance>)
  "The base class for GLib's interface types. Not derivable in Scheme."
  #:metaclass <gobject-class>
  #:gtype-name "GInterface")

(define (class-is-a? x is-a)
  (memq is-a (class-precedence-list x)))

(define-class/docs <gparam-object> (<gparam>)
  "Parameter for @code{<gobject>} values."
  (object-type
   #:init-keyword #:object-type #:allocation #:checked
   #:pred (lambda (x) (is-a? x <gobject-class>)))
  #:value-type <gobject>
  #:gtype-name "GParamObject")


;;;
;;; {GObject Properties}
;;;

(define (gobject-class-find-property class name)
  "Returns a property named @var{name} (a symbol), belonging to
@var{class} or one of its parent classes, or @code{#f} if not found."
  (let ((propname name))
    (with-accessors (name)
                    (let lp ((props (gobject-class-get-properties class)))
                      (cond ((null? props) #f)
                            ((eq? (name (car props)) propname) (car props))
                            (else (lp (cdr props))))))))

(define-generic/docs gobject:set-property
  "Called to set a gobject property. Only properties directly belonging
to the object's class will come through this function; superclasses
handle their own properties.

Takes three arguments: the object, the property name, and the value.

Call @code{(next-method)} in your methods to invoke the default handler.")

(define-method (gobject:set-property (object <gobject>) (name <symbol>) value)
  "The default implementation of @code{gobject:set-property}, which sets
slots on the object."
  (if (class-slot-definition (class-of object) name)
      (slot-set! object name value)
      (gruntime-error "Properties added after object definition must be accessed via custom property methods: ~A" name)))

(define-generic/docs gobject:get-property
  "Called to get a gobject property. Only properties directly belonging
to the object's class will come through this function; superclasses
handle their own properties.

Takes two arguments: the object and the property name.

Call @code{(next-method)} in your methods to invoke the default handler")

(define-method (gobject:get-property (object <gobject>) (name <symbol>))
  "The default implementation of @code{gobject:get-property}, which
calls @code{(slot-ref obj name)}."
  (if (class-slot-definition (class-of object) name)
      (slot-ref object name)
      (gruntime-error "Properties added after object definition must be accessed via custom property methods: ~A" name)))

(eval-when (load eval)
  (gobject-object-post-init))
