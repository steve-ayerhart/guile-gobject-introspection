(define-module (gir g-object g-value)
  #:use-module (oop goops)

  #:use-module (gir g-object utils)
  #:use-module (gir g-object g-type)

  #:export (<g-value>
            <g-boolean> <g-char> <g-uchar> <g-int> <g-uint> <g-long>
            <g-ulong> <g-int64> <g-uint64> <g-float> <g-double>
            <g-chararray> <g-boxed> <g-boxed-scm> <g-value-array>
            <g-pointer>
            <g-enum> <g-flags>
            g-enum-register-static g-flags-register-static
            g-enum-class->value-table g-flags-class->value-table
            scm->g-value g-value->scm
            g-enum->symbol g-enum->name g-enum->value
            g-flags->value
            g-flags->symbol-list g-flags->name-list g-flags->value-list))

;;; generic values

(define-class <g-value-class> (<g-type-class>))

(define-class <g-value> ()
  (value #:class <read-only-slot>)
  #:g-type-name #t
  #:metaclass <g-value-class>)

(eval-when (expand load eval)
  (dynamic-call "scm_gobject_gvalue_init"
                (dynamic-link "/home/steve/Source/guile-gobject-introspection/src/.libs/gobject-introspection")))

(eval-when (load eval)
  (bless-g-value-class <g-value>))

(define-method (allocate-instance (class <g-value-class>) initargs)
  (let ((instance (next-method)))
    (allocate-g-value class instance)
    instance))

(define-method (initialize (instance <g-value>) initargs)
  (or (memq #:value initargs)
      (g-runtime-error "Missing #:value argument"))
  (g-value-set! instance (get-keyword #:value initargs 'foo)))

(define-method (write (obj <g-value>) file)
  (format file "#<~a ~a ~a>"
          (class-name (class-of obj))
          (number->string (object-address obj) 16)
          (let ((converted (g-value->scm obj)))
            ;; should be able to use eq?, but gvalue->scm always rips us
            ;; a fresh one. bugs, bugs..
            (if (is-a? converted <g-value>)
                "[native]"
                converted))))

;; simple classes

(define-class/docs <g-char> (<g-value>)
  "A @cod{<g-value>} class for signed 8-bit values"
  #:g-type-name "gchar")

(define-class/docs <g-uchar> (<g-value>)
  "A @code{<g-value>} class for unsigned 8-bit values."
  #:g-type-name "guchar")

(define-class/docs <g-boolean> (<g-value>)
  "A @code{<g-value>} class for boolean values."
  #:g-type-name "gboolean")

(define-class/docs <g-int> (<g-value>)
  "A @code{<g-value>} class for signed 32-bit values."
  #:g-type-name "gint")

(define-class/docs <g-uint> (<g-value>)
  "A @code{<g-value>} class for unsigned 32-bit values."
  #:g-type-name "guint")

(define-class/docs <g-long> (<g-value>)
  "A @code{<g-value>} class for signed ``long'' (32- or 64-bit)
values."
  #:g-type-name "glong")

(define-class/docs <g-ulong> (<g-value>)
  "A @code{<g-value>} class for unsigned ``long'' (32- or 64-bit)
values."
  #:g-type-name "gulong")

(define-class/docs <g-int64> (<g-value>)
  "A @code{<g-value>} class for signed 64-bit values."
  #:g-type-name "gint64")

(define-class/docs <g-uint64> (<g-value>)
  "A @code{<g-value>} class for unsigned 64-bit values."
  #:g-type-name "guint64")

(define-class/docs <g-float> (<g-value>)
  "A @code{<g-value>} class for 32-bit floating-point values."
  #:g-type-name "gfloat")

(define-class/docs <g-double> (<g-value>)
  "A @code{<g-value>} class for 64-bit floating-point values."
  #:g-type-name "gdouble")

(define-class/docs <g-chararray> (<g-value>)
  "A @code{<g-value>} class for arrays of 8-bit values (C strings)."
  #:g-type-name "gchararray")

(define-class/docs <g-boxed> (<g-value>)
  "A @code{<g-value>} class for ``boxed'' types, a way of wrapping
generic C structures. You won't see instances of this class, only of its
subclasses."
  #:g-type-name "GBoxed")

(define-class/docs <g-value-array> (<g-boxed>)
  "A @code{<g-value>} class for arrays of @code{<g-value>}."
  #:g-type-name "GValueArray")

(define-class/docs <g-boxed-scm> (<g-boxed>)
  "A @code{<g-boxed>} class for holding arbitrary Scheme objects."
  #:g-type-name "GBoxedSCM")

(define-class/docs <g-pointer> (<g-value>)
  "A @code{<g-value>} class for opaque pointers."
  #:g-type-name "gpointer")

;;; enums

(define (vtable-ref vtable keyfunc val)
  (let lp ((i (1- (vector-length vtable))))
    (cond ((< i 0) (g-runtime-error "No such value in ~A: ~A" vtable val))
          ((equal? (keyfunc (vector-ref vtable i)) val)
           (vector-ref vtable i))
          (else (lp (1- i))))))

(define vtable-symbol car)
(define vtable-name cadr)
(define vtable-index caddr)
(define (vtable-by-value vtable v)
  (vtable-ref vtable
              (cond ((integer? v) vtable-index)
                    ((symbol? v) vtable-symbol)
                    ((string? v) vtable-name)
                    (else (g-runtime-error "Wrong type argument: ~S" v)))
              v))

(define-class <g-enum-class> (<g-value-class>))

(define-class/docs <g-enum> (<g-value>)
  "A @code{<g-value>} base class for enumerated values. Users may define
new enumerated value types via subclssing from @code{<g-enum>}, passing
@code{#:vtable @var{table}} as an initarg, where @var{table} should be
in a format suitable for passing to @code{g-enum-register-static}."
  #:g-type-name "GEnum"
  #:metaclass <g-enum-class>)

(define (has-kw-arg? args key)
  (cond ((null? args) #f)
        ((eq? (car args) key) #t)
        (else (has-kw-arg? (cddr args) key))))

(define (supply-initarg-if-missing initargs key proc)
  (cond ((has-kw-arg? initargs key) initargs)
        (else (cons* key (proc initargs) initargs))))

(define (override-initarg initargs key val)
  (cons* key val initargs))

(define (kw-ref initargs key)
  (or (has-kw-arg? initargs key)
      (error "Missing required keyword argument:" key))
  (get-keyword key initargs #f))

(define-method (initialize (class <g-enum-class>) initargs)
  (next-method class
               (supply-initarg-if-missing
                initargs #:g-type-name
                (lambda (initargs)
                  (let ((name (class-name->g-type-name (kw-ref initargs #:name))))
                    (g-enum-register-static name (kw-ref initargs #:vtable))
                    name)))))

(define-method (initialize (instance <g-enum>) initargs)
  (next-method
   instance
   (override-initarg
    initargs #:value
    (vtable-index
     (vtable-by-value
      (g-enum-class->value-table (class-of instance))
      (kw-ref initargs #:value))))))

(define-method (write (obj <g-enum>) file)
  (format file "#<~a ~a ~a>"
          (class-name (class-of obj))
          (number->string (object-address obj) 16)
          (g-enum->symbol obj)))

(define (g-enum->enum obj)
  (vtable-by-value (g-enum-class->value-table (class-of obj))
                   (g-enum->value obj)))

(define (g-enum->symbol obj)
  "Convert the enumerated value @var{obj} from a @code{<g-value>} to its
symbol representation (its ``nickname'')."
  (vtable-symbol (g-enum->enum obj)))

(define (g-enum->name obj)
  "Convert the enumerated value @var{obj} from a @code{<g-value>} to its
representation as a string (its ``name'')."
  (vtable-name (g-enum->enum obj)))

;;; flags

(define-class <g-flags-class> (<g-value-class>))
(define-class/docs <g-flags> (<g-value>)
  "A @code{<g-value>} base class for flag values. Users may define new
flag value types via subclssing from @code{<g-flags>}, passing
@code{#:vtable @var{table}} as an initarg, where @var{table} should be
in a format suitable for passing to @code{g-flags-register-static}."
  #:g-type-name "GFlags"
  #:metaclass <g-flags-class>)

(define-method (initialize (class <g-flags-class>) initargs)
  (next-method class
               (supply-initarg-if-missing
                initargs #:g-type-name
                (lambda (initargs)
                  (let ((name (class-name->g-type-name (kw-ref initargs #:name))))
                    (g-flags-register-static name (kw-ref initargs #:vtable))
                    name)))))

(define-method (write (obj <g-flags>) file)
  (format file "#<~a ~a ~a>"
          (class-name (class-of obj))
          (number->string (object-address obj) 16)
          (g-flags->symbol-list obj)))

(define (g-flags->element-list obj)
  (let ((vtable (g-flags-class->value-table (class-of obj)))
        (value (g-flags->value obj)))
    (filter (lambda (v)
              (= (logand value (vtable-index v)) (vtable-index v)))
            (vector->list vtable))))

(define (g-flags->symbol-list obj)
  "Convert the flags value @var{obj} from a @code{<g-value>} to a list of
the symbols that it represents."
  (map vtable-symbol (g-flags->element-list obj)))

(define (g-flags->name-list obj)
  "Convert the flags value @var{obj} from a @code{<g-value>} to a list of
strings, the names of the values it represents."
  (map vtable-name (g-flags->element-list obj)))

(define (g-flags->value-list obj)
  "Convert the flags value @var{obj} from a @code{<g-value>} to a list of
integers, which when @code{logand}'d together yield the flags' value."
  (map vtable-index (g-flags->element-list obj)))
