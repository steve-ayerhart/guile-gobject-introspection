(define-module (gir g-object)
  #:use-module ((srfi srfi-1) #:select (filter-map))
  #:use-module (oop goops)

  #:use-module (gir g-object utils)
  #:use-module (gir g-object g-type)
  #:use-module (gir g-object g-value)
  #:use-module (gir g-object g-parameter)
  #:use-module (gir g-object g-signal)

  #:export (<g-object>
            <g-interface>
            <g-param-object>
            g-type-register-static
            g-object:get-property g-object-set-property
            g-object-class-get-properties g-object-class-find-property
            g-object-class-get-property-names
            g-object-get-property g-object-set-property))

(eval-when (expand load eval)
  (dynamic-call "scm_gobject_init"
                (dynamic-link "/home/steve/Source/guile-g-object/src/.libs/g-object")))

(define-class <g-object-class> (<g-type-class>))

(define-method (compute-slots (class <g-object-class>))
  (define (has-slot? name slots)
    (and (pair? slots)
         (or (eq? name (slot-definition-name (car slots)))
             (has-slot? name (cdr slots)))))
  (define (compute-extra-slots props slots)
    (filter-map (lambda (prop)
                  (and (not (has-slot? prop slots))
                       (if (defined? '<slot>)
                           (make <slot> #:name prop #:allocation #:g-property)
                           `(,prop #:allocation #:g-property))))
                props))
  (let* ((slots (next-method))
         (extra (compute-extra-slots
                 (g-object-class-get-property-names class) slots)))
    (with-accessors (direct-slots)
                    (set! (direct-slots class) (append (direct-slots class) extra)))
    (append slots extra)))

(define-method (compute-get-n-set (class <g-object-class>) slotdef)
  (let ((name (slot-definition-name slotdef)))
    (case (slot-definition-allocation slotdef)
      ((#:g-property) (list (lambda (o) (g-object-get-property o name))
                           (lambda (o v) (g-object-set-property o name v))))
      (else (next-method)))))

(define-method (initialize (class <g-object-class>) initargs)
  (define (install-properties!)
    ;; To expose slots as gobject properties, <g-object> will process a
    ;; #:gparam slot option to create a new gobject property.
    (for-each
     (lambda (slot)
       (let ((pspec (get-keyword #:g-param (slot-definition-options slot) #f)))
         (if pspec
             (g-object-class-install-property
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
            (if (eq? (car args) #:g-signal)
                (let ((signal (cadr args)))
                  (if (not (and (list? signal) (>= (length signal) 2)))
                      (g-runtime-error "Invalid signal specification: ~A" signal))
                  (let* ((name (car signal))
                         (return-type (cadr signal))
                         (param-types (cddr signal))
                         (generic (g-type-class-create-signal class name return-type param-types)))
                    ;; Some magic to define the generic
                    (module-define! (current-module)
                                    (generic-function-name generic) generic))))
            (loop (cddr args))))))

  (define (first pred list)
    (cond ((null? list) #f)
          ((pred (car list)) (car list))
          (else (first pred (cdr list)))))
  (define (g-object-class? c)
    (memq <g-object> (class-precedence-list c)))

  ;; real work starts here

  (next-method
   class
   (cons*
    #:g-type-name
    (or (get-keyword #:g-type-name initargs #f)
        (g-type-register-static
         (class-name->g-type-name (get-keyword #:name initargs #f))
         (first g-object-class?
                (apply append
                       (map class-precedence-list
                            (get-keyword #:dsupers initargs '()))))))
    initargs))
  (install-properties!)
  (install-signals!))

(define-class/docs <g-object> (<g-type-instance>)
  "The base class for GLib's default object system.

@code{<g-object>}'s metaclass understands a new slot option,
@code{#:g-param}, which will export a slot as a @code{<g-object>}
property. The default implementation will set and access the value from
the slot, but you can customize this by writing your own methods for
@code{g-object:set-property} and @code{g-object:get-property}.

In addition, the metaclass also understands @code{#:g-signal} arguments,
which define signals on the class, and define the generics for the
default signal handler. See @code{g-type-class-define-signal} for more
information.

For example:
@lisp
 ;; deriving from <g-object>
 (define-class <test> (<g-object>)
  ;; a normal object slot
  my-data

  ;; an object slot exported as a g-object property
  (pub-data #:g-param (list <g-param-long> #:name 'test))

  ;; likewise, using non-default parameter settings
  (foo-data #:g-param (list <g-param-long> #:name 'foo
                           #:minimum -3 #:maximum 1000
                           #:default-value 42))

  ;; a signal with no arguments and no return value
  #:g-signal '(frobate #f)

  ;; a signal with arguments and a return value
  #:g-signal (list 'frobate <g-boolean> <g-int> <g-long>))

 ;; deriving from <test> -- also inherits properties and signals
 (define-class <hungry> (<test>))
@end lisp

@code{<g-object>} classes also expose a slot for each GObject property
defined on the class, if such a slot is not already defined.
"
  ;; add a slot for signal generics instead of module-define! ?
  #:metaclass <g-object-class>
  #:g-type-name "GObject")

(define-class/docs <g-interface> (<g-type-instance>)
  "The base class for GLib's interface types. Not derivable in Scheme."
  #:metaclass <g-object-class>
  #:g-type-name "GInterface")

(define (class-is-a? x is-a)
  (memq is-a (class-precedence-list x)))

(define-class/docs <g-param-object> (<g-param>)
  "Parameter for @code{<g-object>} values."
  (object-type
   #:init-keyword #:object-type #:allocation #:checked
   #:pred (lambda (x) (is-a? x <g-object-class>)))
  #:value-type <g-object>
  #:g-type-name "GParamObject")


;;;
;;; {GObject Properties}
;;;

(define (g-object-class-find-property class name)
  "Returns a property named @var{name} (a symbol), belonging to
@var{class} or one of its parent classes, or @code{#f} if not found."
  (let ((propname name))
    (with-accessors (name)
      (let lp ((props (g-object-class-get-properties class)))
        (cond ((null? props) #f)
              ((eq? (name (car props)) propname) (car props))
              (else (lp (cdr props))))))))

(define-generic/docs g-object:set-property
  "Called to set a gobject property. Only properties directly belonging
to the object's class will come through this function; superclasses
handle their own properties.

Takes three arguments: the object, the property name, and the value.

Call @code{(next-method)} in your methods to invoke the default handler.")

(define-method (g-object:set-property (object <g-object>) (name <symbol>) value)
  "The default implementation of @code{g-object:set-property}, which sets
slots on the object."
  (if (class-slot-definition (class-of object) name)
      (slot-set! object name value)
      (g-runtime-error "Properties added after object definition must be accessed via custom property methods: ~A" name)))

(define-generic/docs g-object:get-property
  "Called to get a gobject property. Only properties directly belonging
to the object's class will come through this function; superclasses
handle their own properties.

Takes two arguments: the object and the property name.

Call @code{(next-method)} in your methods to invoke the default handler")

(define-method (g-object:get-property (object <g-object>) (name <symbol>))
  "The default implementation of @code{g-object:get-property}, which
calls @code{(slot-ref obj name)}."
  (if (class-slot-definition (class-of object) name)
      (slot-ref object name)
      (g-runtime-error "Properties added after object definition must be accessed via custom property methods: ~A" name)))

(eval-when (load eval)
  (g-object-object-post-init))
