(define-module (gir g-object g-parameter)
  #:use-module (oop goops)

  #:use-module (gir g-object utils)
  #:use-module (gir g-object g-type)
  #:use-module (gir g-object g-value)

  #:export (<g-param>
            <g-param-char> <g-param-uchar> <g-param-boolean> <g-param-int>
            <g-param-uint> <g-param-long> <g-param-ulong> <g-param-int64>
            <g-param-uint64> <g-param-float> <g-param-double>
            <g-param-unichar> <g-param-pointer> <g-param-string>
            <g-param-boxed> <g-param-enum> <g-param-flags> <g-param-gtype>
            ;; Helper class
            <g-param-spec-flags>
            ;; Limits
            g-parameter:uint-max g-parameter:int-min g-parameter:int-max
            g-parameter:ulong-max g-parameter:long-min
            g-parameter:long-max g-parameter:uint64-max
            g-parameter:int64-min g-parameter:int64-max
            g-parameter:float-max g-parameter:float-min
            g-parameter:double-max g-parameter:double-min
            g-parameter:byte-order))

(define-class/docs <g-param-spec-flags> (<g-flags>)
  "A @code{<g-flags>} type for the flags allowable on a @code{<g-param>}:
@code{read}, @code{write}, @code{construct}, @code{construct-only}, and
@code{lax-validation}."
  #:vtable
  #((read "Readable" 1)
    (write "Writable" 2)
    (construct "Set on object construction" 4)
    (construct-only "Only set on object construction" 8)
    (lax-validation "Don't require strict validation on parameter conversion" 16)))

;; The C code needs to reference <g-param> for use in its predicates.
;; Define it now before loading the library.
(define-class <g-param-class> (<g-type-class>)
  (value-type #:init-keyword #:value-type))

(define-method (compute-get-n-set (class <g-param-class>) s)
  (case (slot-definition-allocation s)
    ((#:checked)
     (let ((already-allocated (slot-ref class 'nfields))
           (pred (get-keyword #:pred (slot-definition-options s) #f))
           (trans (get-keyword #:trans (slot-definition-options s) #f)))
       (or pred (g-runtime-error "Missing #:pred for #:checked slot"))
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
                     (g-runtime-error
                      "Bad value for slot ~A on instance ~A: ~A"
                      (slot-definition-name s) instance value)))))))

    (else (next-method))))

(define-class/docs <g-param> (<g-type-instance>)
  "The base class for GLib parameter objects. (Doc slots)"
  (name #:init-keyword #:name #:allocation #:checked #:pred symbol?)
  (nick #:init-keyword #:nick #:allocation #:checked #:pred string?)
  (blurb #:init-keyword #:blurb #:allocation #:checked #:pred string?)
  (flags #:init-keyword #:flags #:init-value '(read write)
         #:allocation #:checked #:pred number?
         #:trans (lambda (x)
                   (apply + (g-flags->value-list
                             (make <g-param-spec-flags> #:value x)))))
  #:g-type-name "GParam"
  #:metaclass <g-param-class>)


(eval-when (expand load eval)
  (dynamic-call "scm_gobject_gparameter_init"
                (dynamic-link "/home/steve/Source/guile-gobject-introspection/src/.libs/gobject-introspection")))

(define-class/docs <g-param-char> (<g-param>)
  "Parameter for @code{<g-char>} values."
  (minimum
   #:init-keyword #:minimum #:init-value (integer->char 0)
   #:allocation #:checked #:pred (lambda (x) (or (char? x) (integer? x))))
  (maximum
   #:init-keyword #:maximum #:init-value (integer->char 127)
   #:allocation #:checked #:pred (lambda (x) (or (char? x) (integer? x))))
  (default-value
   #:init-keyword #:default-value #:init-value (integer->char 127)
   #:allocation #:checked #:pred (lambda (x) (or (char? x) (integer? x))))
  #:value-type <g-char>
  #:g-type-name "GParamChar")

(define-class/docs <g-param-uchar> (<g-param>)
  "Parameter for @code{<g-uchar>} values."
  (minimum
   #:init-keyword #:minimum #:init-value (integer->char 0)
   #:allocation #:checked #:pred (lambda (x) (or (char? x) (integer? x))))
  (maximum
   #:init-keyword #:maximum #:init-value (integer->char 255)
   #:allocation #:checked #:pred (lambda (x) (or (char? x) (integer? x))))
  (default-value
   #:init-keyword #:default-value #:init-value (integer->char 255)
   #:allocation #:checked #:pred (lambda (x) (or (char? x) (integer? x))))
  #:value-type <g-uchar>
  #:g-type-name "GParamUChar")

(define-class/docs <g-param-boolean> (<g-param>)
  "Parameter for @code{<g-boolean>} values."
  (default-value
   #:init-keyword #:default-value #:init-value #f
   #:allocation #:checked #:pred boolean?)
  #:value-type <g-boolean>
  #:g-type-name "GParamBoolean")

(define-class/docs <g-param-int> (<g-param>)
  "Parameter for @code{<g-int>} values."
  (minimum
   #:init-keyword #:minimum #:init-value g-parameter:int-min
   #:allocation #:checked #:pred integer?)
  (maximum
   #:init-keyword #:maximum #:init-value g-parameter:int-max
   #:allocation #:checked #:pred integer?)
  (default-value
   #:init-keyword #:default-value #:init-value 0
   #:allocation #:checked #:pred integer?)
  #:value-type <g-int>
  #:g-type-name "GParamInt")

(define-class/docs <g-param-uint> (<g-param>)
  "Parameter for @code{<g-uint>} values."
  (minimum
   #:init-keyword #:minimum #:init-value 0
   #:allocation #:checked #:pred integer?)
  (maximum
   #:init-keyword #:maximum #:init-value g-parameter:uint-max
   #:allocation #:checked #:pred integer?)
  (default-value
   #:init-keyword #:default-value #:init-value 0
   #:allocation #:checked #:pred integer?)
  #:value-type <g-uint>
  #:g-type-name "GParamUInt")

(define-class/docs <g-param-unichar> (<g-param>)
  "Parameter for Unicode codepoints, represented as @code{<g-uint>}
values."
  (default-value
   #:init-keyword #:default-value #:init-value 0
   #:allocation #:checked #:pred integer?)
  #:value-type <g-uint>
  #:g-type-name "GParamUnichar")

(define-class/docs <g-param-long> (<g-param>)
  "Parameter for @code{<g-long>} values."
  (minimum
   #:init-keyword #:minimum #:init-value g-parameter:long-min
   #:allocation #:checked #:pred integer?)
  (maximum
   #:init-keyword #:maximum #:init-value g-parameter:long-max
   #:allocation #:checked #:pred integer?)
  (default-value
   #:init-keyword #:default-value #:init-value 0
   #:allocation #:checked #:pred integer?)
  #:value-type <g-long>
  #:g-type-name "GParamLong")

(define-class/docs <g-param-ulong> (<g-param>)
  "Parameter for @code{<g-ulong>} values."
  (minimum
   #:init-keyword #:minimum #:init-value 0
   #:allocation #:checked #:pred integer?)
  (maximum
   #:init-keyword #:maximum #:init-value g-parameter:ulong-max
   #:allocation #:checked #:pred integer?)
  (default-value
   #:init-keyword #:default-value #:init-value 0
   #:allocation #:checked #:pred integer?)
  #:value-type <g-ulong>
  #:g-type-name "GParamULong")

(define-class/docs <g-param-int64> (<g-param>)
  "Parameter for @code{<g-int64>} values."
  (minimum
   #:init-keyword #:minimum #:init-value g-parameter:int64-min
   #:allocation #:checked #:pred integer?)
  (maximum
   #:init-keyword #:maximum #:init-value g-parameter:int64-max
   #:allocation #:checked #:pred integer?)
  (default-value
   #:init-keyword #:default-value #:init-value 0
   #:allocation #:checked #:pred integer?)
  #:value-type <g-int64>
  #:g-type-name "GParamInt64")

(define-class/docs <g-param-uint64> (<g-param>)
  "Parameter for @code{<g-uint64>} values."
  (minimum
   #:init-keyword #:minimum #:init-value 0
   #:allocation #:checked #:pred integer?)
  (maximum
   #:init-keyword #:maximum #:init-value g-parameter:uint64-max
   #:allocation #:checked #:pred integer?)
  (default-value
   #:init-keyword #:default-value #:init-value 0
   #:allocation #:checked #:pred integer?)
  #:value-type <g-uint64>
  #:g-type-name "GParamUInt64")

(define-class/docs <g-param-float> (<g-param>)
  "Parameter for @code{<g-float>} values."
  (minimum
   #:init-keyword #:minimum #:init-value (- g-parameter:float-max)
   #:allocation #:checked #:pred real?)
  (maximum
   #:init-keyword #:maximum #:init-value g-parameter:float-max
   #:allocation #:checked #:pred real?)
  (default-value
   #:init-keyword #:default-value #:init-value 0.0
   #:allocation #:checked #:pred real?)
  #:value-type <g-float>
  #:g-type-name "GParamFloat")

(define-class/docs <g-param-double> (<g-param>)
  "Parameter for @code{<g-double>} values."
  (minimum
   #:init-keyword #:minimum #:init-value (- g-parameter:double-max)
   #:allocation #:checked #:pred real?)
  (maximum
   #:init-keyword #:maximum #:init-value g-parameter:double-max
   #:allocation #:checked #:pred real?)
  (default-value
   #:init-keyword #:default-value #:init-value 0.0
   #:allocation #:checked #:pred real?)
  #:value-type <g-double>
  #:g-type-name "GParamDouble")

(define-class/docs <g-param-pointer> (<g-param>)
  "Parameter for @code{<g-pointer>} values."
  #:value-type <g-pointer>
  #:g-type-name "GParamPointer")

(define-class/docs <g-param-string> (<g-param>)
  "Parameter for @code{<g-chararray>} values."
  (default-value
   #:init-keyword #:default-value #:init-value ""
   #:allocation #:checked #:pred (lambda (x) (or (not x) (string? x))))
  #:value-type <g-chararray>
  #:g-type-name "GParamString")

(define (class-is-a? x is-a)
  (memq is-a (class-precedence-list x)))

(define-class/docs <g-param-boxed> (<g-param>)
  "Parameter for @code{<g-boxed>} values."
  (boxed-type
   #:init-keyword #:boxed-type #:allocation #:checked
   #:pred (lambda (x) (class-is-a? x <g-boxed>)))
  #:value-type <g-boxed>
  #:g-type-name "GParamBoxed")

(define-class/docs <g-param-enum> (<g-param>)
  "Parameter for @code{<g-enum>} values."
  (enum-type
   #:init-keyword #:enum-type #:allocation #:checked
   #:pred (lambda (x) (class-is-a? x <g-enum>)))
  (default-value
   #:init-keyword #:default-value #:init-value 0
   #:allocation #:checked #:pred number?)
  #:value-type <g-enum>
  #:g-type-name "GParamEnum")

(define-class/docs <g-param-flags> (<g-param>)
  "Parameter for @code{<g-flags>} values."
  (flags-type
   #:init-keyword #:flags-type #:allocation #:checked
   #:pred (lambda (x) (class-is-a? x <g-flags>)))
  (default-value
   #:init-keyword #:default-value #:init-value 0
   #:allocation #:checked #:pred number?)
  #:value-type <g-flags>
  #:g-type-name "GParamFlags")

(define-class/docs <g-param-value-array> (<g-param>)
  "Parameter for @code{<g-value-array>} values."
  (element-spec
   #:init-keyword #:element-spec #:allocation #:checked
   #:pred (lambda (x) (or (not x) (is-a? x <g-param>))))
  #:value-type <g-value-array>
  #:g-type-name "GParamValueArray")

(define-class/docs <g-param-gtype> (<g-param>)
  "Parameter for @code{<g-type>} values."
  (is-a-type
   #:init-keyword #:is-a-type #:allocation #:checked
   #:pred (lambda (x) (or (not x) (is-a? x <g-type-class>))))
  #:value-type <g-type-class>
  #:g-type-name "GParamGType")

;;;
;;; {Instance Initialization}
;;;

;; fixme, make me more useful
(define-method (write (param <g-param>) file)
  (let ((class (class-of param))
        (loc (number->string (object-address param) 16)))
    (if (slot-bound? class 'name)
        (with-accessors (name)
          (format file "<~a ~a ~a>" (class-name class) (name param) loc))
        (format file "<~a (uninitialized) ~a>" (class-name class) loc))))
