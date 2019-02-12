(define-module (gobject gparameter)
  #:use-module (oop goops)

  #:use-module (glib utils)
  #:use-module (gobject gtype)
  #:use-module (gobject gvalue)

  #:export (<gparam>
            <gparam-char> <gparam-uchar> <gparam-boolean> <gparam-int>
            <gparam-uint> <gparam-long> <gparam-ulong> <gparam-int64>
            <gparam-uint64> <gparam-float> <gparam-double>
            <gparam-unichar> <gparam-pointer> <gparam-string>
            <gparam-boxed> <gparam-enum> <gparam-flags> <gparam-gtype>
            ;; Helper class
            <gparam-spec-flags>
            ;; Limits
            gparameter:uint-max gparameter:int-min gparameter:int-max
            gparameter:ulong-max gparameter:long-min
            gparameter:long-max gparameter:uint64-max
            gparameter:int64-min gparameter:int64-max
            gparameter:float-max gparameter:float-min
            gparameter:double-max gparameter:double-min
            gparameter:byte-order))

(define-class/docs <gparam-spec-flags> (<gflags>)
  "A @code{<gflags>} type for the flags allowable on a @code{<gparam>}:
@code{read}, @code{write}, @code{construct}, @code{construct-only}, and
@code{lax-validation}."
  #:vtable
  #((read "Readable" 1)
    (write "Writable" 2)
    (construct "Set on object construction" 4)
    (construct-only "Only set on object construction" 8)
    (lax-validation "Don't require strict validation on parameter conversion" 16)))

;; The C code needs to reference <gparam> for use in its predicates.
;; Define it now before loading the library.
(define-class <gparam-class> (<gtype-class>)
  (value-type #:init-keyword #:value-type))

(define-method (compute-get-n-set (class <gparam-class>) s)
  (case (slot-definition-allocation s)
    ((#:checked)
     (let ((already-allocated (slot-ref class 'nfields))
           (pred (get-keyword #:pred (slot-definition-options s) #f))
           (trans (get-keyword #:trans (slot-definition-options s) #f)))
       (or pred (gruntime-error "Missing #:pred for #:checked slot"))
       ;; allocate a field in the struct
       (slot-set! class 'nfields (+ already-allocated 1))
       ;; struct-ref and struct-set! don't work on the structs that back
       ;; GOOPS objects, because they are "light structs", without the
       ;; hidden word that says how many fields are in the struct.
       ;; Patches submitted to guile-devel on 10 April 2008. Until then,
       ;; use our own struct accessors.
       (list (lambda (instance)
               (hacky-struct-ref instance already-allocated))
             (lambda (instance value)
               (let ((value (if trans (trans value) value)))
                 (if (pred value)
                     (hacky-struct-set! instance already-allocated value)
                     (gruntime-error
                      "Bad value for slot ~A on instance ~A: ~A"
                      (slot-definition-name s) instance value)))))))

    (else (next-method))))

(define-class/docs <gparam> (<gtype-instance>)
  "The base class for GLib parameter objects. (Doc slots)"
  (name #:init-keyword #:name #:allocation #:checked #:pred symbol?)
  (nick #:init-keyword #:nick #:allocation #:checked #:pred string?)
  (blurb #:init-keyword #:blurb #:allocation #:checked #:pred string?)
  (flags #:init-keyword #:flags #:init-value '(read write)
         #:allocation #:checked #:pred number?
         #:trans (lambda (x)
                   (apply + (gflags->value-list
                             (make <gparam-spec-flags> #:value x)))))
  #:gtype-name "GParam"
  #:metaclass <gparam-class>)


(eval-when (expand load eval)
  (dynamic-call "scm_gobject_gparameter_init"
                (dynamic-link "/home/steve/Source/guile-gobject-introspection/src/.libs/gobject-introspection")))

(define-class/docs <gparam-char> (<gparam>)
  "Parameter for @code{<gchar>} values."
  (minimum
   #:init-keyword #:minimum #:init-value (integer->char 0)
   #:allocation #:checked #:pred (lambda (x) (or (char? x) (integer? x))))
  (maximum
   #:init-keyword #:maximum #:init-value (integer->char 127)
   #:allocation #:checked #:pred (lambda (x) (or (char? x) (integer? x))))
  (default-value
   #:init-keyword #:default-value #:init-value (integer->char 127)
   #:allocation #:checked #:pred (lambda (x) (or (char? x) (integer? x))))
  #:value-type <gchar>
  #:gtype-name "GParamChar")

(define-class/docs <gparam-uchar> (<gparam>)
  "Parameter for @code{<guchar>} values."
  (minimum
   #:init-keyword #:minimum #:init-value (integer->char 0)
   #:allocation #:checked #:pred (lambda (x) (or (char? x) (integer? x))))
  (maximum
   #:init-keyword #:maximum #:init-value (integer->char 255)
   #:allocation #:checked #:pred (lambda (x) (or (char? x) (integer? x))))
  (default-value
   #:init-keyword #:default-value #:init-value (integer->char 255)
   #:allocation #:checked #:pred (lambda (x) (or (char? x) (integer? x))))
  #:value-type <guchar>
  #:gtype-name "GParamUChar")

(define-class/docs <gparam-boolean> (<gparam>)
  "Parameter for @code{<gboolean>} values."
  (default-value
   #:init-keyword #:default-value #:init-value #f
   #:allocation #:checked #:pred boolean?)
  #:value-type <gboolean>
  #:gtype-name "GParamBoolean")

(define-class/docs <gparam-int> (<gparam>)
  "Parameter for @code{<gint>} values."
  (minimum
   #:init-keyword #:minimum #:init-value gparameter:int-min
   #:allocation #:checked #:pred integer?)
  (maximum
   #:init-keyword #:maximum #:init-value gparameter:int-max
   #:allocation #:checked #:pred integer?)
  (default-value
   #:init-keyword #:default-value #:init-value 0
   #:allocation #:checked #:pred integer?)
  #:value-type <gint>
  #:gtype-name "GParamInt")

(define-class/docs <gparam-uint> (<gparam>)
  "Parameter for @code{<guint>} values."
  (minimum
   #:init-keyword #:minimum #:init-value 0
   #:allocation #:checked #:pred integer?)
  (maximum
   #:init-keyword #:maximum #:init-value gparameter:uint-max
   #:allocation #:checked #:pred integer?)
  (default-value
   #:init-keyword #:default-value #:init-value 0
   #:allocation #:checked #:pred integer?)
  #:value-type <guint>
  #:gtype-name "GParamUInt")

(define-class/docs <gparam-unichar> (<gparam>)
  "Parameter for Unicode codepoints, represented as @code{<guint>}
values."
  (default-value
   #:init-keyword #:default-value #:init-value 0
   #:allocation #:checked #:pred integer?)
  #:value-type <guint>
  #:gtype-name "GParamUnichar")

(define-class/docs <gparam-long> (<gparam>)
  "Parameter for @code{<glong>} values."
  (minimum
   #:init-keyword #:minimum #:init-value gparameter:long-min
   #:allocation #:checked #:pred integer?)
  (maximum
   #:init-keyword #:maximum #:init-value gparameter:long-max
   #:allocation #:checked #:pred integer?)
  (default-value
   #:init-keyword #:default-value #:init-value 0
   #:allocation #:checked #:pred integer?)
  #:value-type <glong>
  #:gtype-name "GParamLong")

(define-class/docs <gparam-ulong> (<gparam>)
  "Parameter for @code{<gulong>} values."
  (minimum
   #:init-keyword #:minimum #:init-value 0
   #:allocation #:checked #:pred integer?)
  (maximum
   #:init-keyword #:maximum #:init-value gparameter:ulong-max
   #:allocation #:checked #:pred integer?)
  (default-value
   #:init-keyword #:default-value #:init-value 0
   #:allocation #:checked #:pred integer?)
  #:value-type <gulong>
  #:gtype-name "GParamULong")

(define-class/docs <gparam-int64> (<gparam>)
  "Parameter for @code{<gint64>} values."
  (minimum
   #:init-keyword #:minimum #:init-value gparameter:int64-min
   #:allocation #:checked #:pred integer?)
  (maximum
   #:init-keyword #:maximum #:init-value gparameter:int64-max
   #:allocation #:checked #:pred integer?)
  (default-value
   #:init-keyword #:default-value #:init-value 0
   #:allocation #:checked #:pred integer?)
  #:value-type <gint64>
  #:gtype-name "GParamInt64")

(define-class/docs <gparam-uint64> (<gparam>)
  "Parameter for @code{<guint64>} values."
  (minimum
   #:init-keyword #:minimum #:init-value 0
   #:allocation #:checked #:pred integer?)
  (maximum
   #:init-keyword #:maximum #:init-value gparameter:uint64-max
   #:allocation #:checked #:pred integer?)
  (default-value
   #:init-keyword #:default-value #:init-value 0
   #:allocation #:checked #:pred integer?)
  #:value-type <guint64>
  #:gtype-name "GParamUInt64")

(define-class/docs <gparam-float> (<gparam>)
  "Parameter for @code{<gfloat>} values."
  (minimum
   #:init-keyword #:minimum #:init-value (- gparameter:float-max)
   #:allocation #:checked #:pred real?)
  (maximum
   #:init-keyword #:maximum #:init-value gparameter:float-max
   #:allocation #:checked #:pred real?)
  (default-value
   #:init-keyword #:default-value #:init-value 0.0
   #:allocation #:checked #:pred real?)
  #:value-type <gfloat>
  #:gtype-name "GParamFloat")

(define-class/docs <gparam-double> (<gparam>)
  "Parameter for @code{<gdouble>} values."
  (minimum
   #:init-keyword #:minimum #:init-value (- gparameter:double-max)
   #:allocation #:checked #:pred real?)
  (maximum
   #:init-keyword #:maximum #:init-value gparameter:double-max
   #:allocation #:checked #:pred real?)
  (default-value
   #:init-keyword #:default-value #:init-value 0.0
   #:allocation #:checked #:pred real?)
  #:value-type <gdouble>
  #:gtype-name "GParamDouble")

(define-class/docs <gparam-pointer> (<gparam>)
  "Parameter for @code{<gpointer>} values."
  #:value-type <gpointer>
  #:gtype-name "GParamPointer")

(define-class/docs <gparam-string> (<gparam>)
  "Parameter for @code{<gchar-array>} values."
  (default-value
   #:init-keyword #:default-value #:init-value ""
   #:allocation #:checked #:pred (lambda (x) (or (not x) (string? x))))
  #:value-type <gchar-array>
  #:gtype-name "GParamString")

(define (class-is-a? x is-a)
  (memq is-a (class-precedence-list x)))

(define-class/docs <gparam-boxed> (<gparam>)
  "Parameter for @code{<gboxed>} values."
  (boxed-type
   #:init-keyword #:boxed-type #:allocation #:checked
   #:pred (lambda (x) (class-is-a? x <gboxed>)))
  #:value-type <gboxed>
  #:gtype-name "GParamBoxed")

(define-class/docs <gparam-enum> (<gparam>)
  "Parameter for @code{<genum>} values."
  (enum-type
   #:init-keyword #:enum-type #:allocation #:checked
   #:pred (lambda (x) (class-is-a? x <genum>)))
  (default-value
   #:init-keyword #:default-value #:init-value 0
   #:allocation #:checked #:pred number?)
  #:value-type <genum>
  #:gtype-name "GParamEnum")

(define-class/docs <gparam-flags> (<gparam>)
  "Parameter for @code{<gflags>} values."
  (flags-type
   #:init-keyword #:flags-type #:allocation #:checked
   #:pred (lambda (x) (class-is-a? x <gflags>)))
  (default-value
   #:init-keyword #:default-value #:init-value 0
   #:allocation #:checked #:pred number?)
  #:value-type <gflags>
  #:gtype-name "GParamFlags")

(define-class/docs <gparam-value-array> (<gparam>)
  "Parameter for @code{<gvalue-array>} values."
  (element-spec
   #:init-keyword #:element-spec #:allocation #:checked
   #:pred (lambda (x) (or (not x) (is-a? x <gparam>))))
  #:value-type <gvalue-array>
  #:gtype-name "GParamValueArray")

(define-class/docs <gparam-gtype> (<gparam>)
  "Parameter for @code{<gtype>} values."
  (is-a-type
   #:init-keyword #:is-a-type #:allocation #:checked
   #:pred (lambda (x) (or (not x) (is-a? x <gtype-class>))))
  #:value-type <gtype-class>
  #:gtype-name "GParamGType")

;;;
;;; {Instance Initialization}
;;;

;; fixme, make me more useful
(define-method (write (param <gparam>) file)
  (let ((class (class-of param))
        (loc (number->string (object-address param) 16)))
    (if (slot-bound? class 'name)
        (with-accessors (name)
          (format file "<~a ~a ~a>" (class-name class) (name param) loc))
        (format file "<~a (uninitialized) ~a>" (class-name class) loc))))
