(define-module (gobject gvalue)
  #:use-module (oop goops)

  #:use-module (glib utils)
  #:use-module (gobject gtype)

  #:export (<gvalue>
            <gboolean> <gchar> <guchar> <gint> <guint> <glong>
            <gulong> <gint64> <guint64> <gfloat> <gdouble>
            <gchar-array> <gboxed> <gboxed-scm> <gvalue-array>
            <gpointer>
            <genum> <gflags> <gerror>
            genum-register-static gflags-register-static
            genum-class->value-table gflags-class->value-table
            scm->gvalue gvalue->scm
            genum->symbol genum->name genum->value genum->enum
            gflags->value
            gflags->symbol-list gflags->name-list gflags->value-list))

;;; generic values

(define-class <gvalue-class> (<gtype-class>))

(define-class <gvalue> ()
  (value #:class <read-only-slot>)
  #:gtype-name #t
  #:metaclass <gvalue-class>)

(eval-when (expand load eval)
  (dynamic-call "scm_gobject_gvalue_init"
                (dynamic-link "/home/steve/Source/guile-gobject-introspection/src/.libs/gobject-introspection")))

(eval-when (load eval)
  (bless-gvalue-class <gvalue>))

(define-method (allocate-instance (class <gvalue-class>) initargs)
  (let ((instance (next-method)))
    (allocate-gvalue class instance)
    instance))

(define-method (initialize (instance <gvalue>) initargs)
  (or (memq #:value initargs)
      (error "Missing #:value argument"))
  (gvalue-set! instance (get-keyword #:value initargs 'foo)))

(define-method (write (obj <gvalue>) file)
  (format file "#<~a ~a ~a>"
          (class-name (class-of obj))
          (number->string (object-address obj) 16)
          (let ((converted (gvalue->scm obj)))
            ;; should be able to use eq?, but gvalue->scm always rips us
            ;; a fresh one. bugs, bugs..
            (if (is-a? converted <gvalue>)
                "[native]"
                converted))))

;; simple classes

(define-class/docs <gchar> (<gvalue>)
  "A @cod{<gvalue>} class for signed 8-bit values"
  #:gtype-name "gchar")

(define-class/docs <guchar> (<gvalue>)
  "A @code{<gvalue>} class for unsigned 8-bit values."
  #:gtype-name "guchar")

(define-class/docs <gboolean> (<gvalue>)
  "A @code{<gvalue>} class for boolean values."
  #:gtype-name "gboolean")

(define-class/docs <gint> (<gvalue>)
  "A @code{<gvalue>} class for signed 32-bit values."
  #:gtype-name "gint")

(define-class/docs <guint> (<gvalue>)
  "A @code{<gvalue>} class for unsigned 32-bit values."
  #:gtype-name "guint")

(define-class/docs <glong> (<gvalue>)
  "A @code{<gvalue>} class for signed ``long'' (32- or 64-bit)
values."
  #:gtype-name "glong")

(define-class/docs <gulong> (<gvalue>)
  "A @code{<gvalue>} class for unsigned ``long'' (32- or 64-bit)
values."
  #:gtype-name "gulong")

(define-class/docs <gint64> (<gvalue>)
  "A @code{<gvalue>} class for signed 64-bit values."
  #:gtype-name "gint64")

(define-class/docs <guint64> (<gvalue>)
  "A @code{<gvalue>} class for unsigned 64-bit values."
  #:gtype-name "guint64")

(define-class/docs <gfloat> (<gvalue>)
  "A @code{<gvalue>} class for 32-bit floating-point values."
  #:gtype-name "gfloat")

(define-class/docs <gdouble> (<gvalue>)
  "A @code{<gvalue>} class for 64-bit floating-point values."
  #:gtype-name "gdouble")

(define-class/docs <gchar-array> (<gvalue>)
  "A @code{<gvalue>} class for arrays of 8-bit values (C strings)."
  #:gtype-name "gchararray")

(define-class/docs <gboxed> (<gvalue>)
  "A @code{<gvalue>} class for ``boxed'' types, a way of wrapping
generic C structures. You won't see instances of this class, only of its
subclasses."
  #:gtype-name "GBoxed")


(define-class/docs <gvalue-array> (<gboxed>)
  "A @code{<gvalue>} class for arrays of @code{<gvalue>}."
  #:gtype-name "GValueArray")

(define-class/docs <gboxed-scm> (<gboxed>)
  "A @code{<gboxed>} class for holding arbitrary Scheme objects."
  #:gtype-name "GBoxedSCM")

(define-class/docs <gpointer> (<gvalue>)
  "A @code{<gvalue>} class for opaque pointers."
  #:gtype-name "gpointer")

;;; enums

(define (vtable-ref vtable keyfunc val)
  (let lp ((i (1- (vector-length vtable))))
    (cond ((< i 0) (gruntime-error "No such value in ~A: ~A" vtable val))
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
                    (else (gruntime-error "Wrong type argument: ~S" v)))
              v))

(define-class <genum-class> (<gvalue-class>))

(define-class/docs <genum> (<gvalue>)
  "A @code{<gvalue>} base class for enumerated values. Users may define
new enumerated value types via subclssing from @code{<genum>}, passing
@code{#:vtable @var{table}} as an initarg, where @var{table} should be
in a format suitable for passing to @code{genum-register-static}."
  #:gtype-name "GEnum"
  #:metaclass <genum-class>)

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

(define-method (initialize (class <genum-class>) initargs)
  (next-method class
               (supply-initarg-if-missing
                initargs #:gtype-name
                (lambda (initargs)
                  (let ((name (class-name->gtype-name (kw-ref initargs #:name))))
                    (genum-register-static name (kw-ref initargs #:vtable))
                    name)))))

(define-method (initialize (instance <genum>) initargs)
  (next-method
   instance
   (override-initarg
    initargs #:value
    (vtable-index
     (vtable-by-value
      (genum-class->value-table (class-of instance))
      (kw-ref initargs #:value))))))

(define-method (write (obj <genum>) file)
  (format file "#<~a ~a ~a>"
          (class-name (class-of obj))
          (number->string (object-address obj) 16)
          (genum->symbol obj)))

(define (genum->enum obj)
  (vtable-by-value (genum-class->value-table (class-of obj))
                   (genum->value obj)))

(define (genum->symbol obj)
  "Convert the enumerated value @var{obj} from a @code{<gvalue>} to its
symbol representation (its ``nickname'')."
  (vtable-symbol (genum->enum obj)))

(define (genum->name obj)
  "Convert the enumerated value @var{obj} from a @code{<gvalue>} to its
representation as a string (its ``name'')."
  (vtable-name (genum->enum obj)))

;;; flags

(define-class <gflags-class> (<gvalue-class>))
(define-class/docs <gflags> (<gvalue>)
  "A @code{<gvalue>} base class for flag values. Users may define new
flag value types via subclssing from @code{<gflags>}, passing
@code{#:vtable @var{table}} as an initarg, where @var{table} should be
in a format suitable for passing to @code{gflags-register-static}."
  #:gtype-name "GFlags"
  #:metaclass <gflags-class>)

(define-method (initialize (class <gflags-class>) initargs)
  (next-method class
               (supply-initarg-if-missing
                initargs #:gtype-name
                (lambda (initargs)
                  (let ((name (class-name->gtype-name (kw-ref initargs #:name))))
                    (gflags-register-static name (kw-ref initargs #:vtable))
                    name)))))

(define-method (write (obj <gflags>) file)
  (format file "#<~a ~a ~a>"
          (class-name (class-of obj))
          (number->string (object-address obj) 16)
          (gflags->symbol-list obj)))

(define (gflags->element-list obj)
  (let ((vtable (gflags-class->value-table (class-of obj)))
        (value (gflags->value obj)))
    (filter (lambda (v)
              (= (logand value (vtable-index v)) (vtable-index v)))
            (vector->list vtable))))

(define (gflags->symbol-list obj)
  "Convert the flags value @var{obj} from a @code{<gvalue>} to a list of
the symbols that it represents."
  (map vtable-symbol (gflags->element-list obj)))

(define (gflags->name-list obj)
  "Convert the flags value @var{obj} from a @code{<gvalue>} to a list of
strings, the names of the values it represents."
  (map vtable-name (gflags->element-list obj)))

(define (gflags->value-list obj)
  "Convert the flags value @var{obj} from a @code{<gvalue>} to a list of
integers, which when @code{logand}'d together yield the flags' value."
  (map vtable-index (gflags->element-list obj)))
