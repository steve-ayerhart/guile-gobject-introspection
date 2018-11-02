(define-module (gir info)
  #:use-module (gir g-object g-type)

  #:use-module (oop goops)
  #:use-module (system foreign)
  #:use-module (system foreign-object)

  #:export (<base-info>
            get-name
            <callable-info>
            <function-info>
            <signal-info>
            <v-func-info>
            <registered-type-info>
            get-g-type
            <enum-info>
            <interface-info>
            <object-info>
            get-methods
            <struct-info>
            <union-info>
            <arg-info>
            <constant-info>
            get-value
            <field-info>
            <property-info>
            <type-info>))

(define ggi-lib "/home/steve/Source/guile-gobject-introspection/src/.libs/gobject-introspection")

(eval-when (expand load eval)
  (dynamic-call "ggi_base_info_init"
                (dynamic-link "/home/steve/Source/guile-gobject-introspection/src/.libs/gobject-introspection")))
,
(define-class <callable-info> (<base-info>))
(define-class <callback-info> (<callable-info>))
(define-class <function-info> (<callable-info>))
(define-class <signal-info> (<callable-info>))
(define-class <v-func-info> (<callable-info>))

(define-class <registered-type-info> (<base-info>))
(define-class <enum-info> (<registered-type-info>))
(define-class <interface-info> (<registered-type-info>))
(define-class <object-info> (<registered-type-info>))
(define-class <struct-info> (<registered-type-info>))
(define-class <union-info> (<registered-type-info>))

(define-class <arg-info> (<base-info>))
(define-class <constant-info> (<base-info>))
(define-class <field-info> (<base-info>))
(define-class <property-info> (<base-info>))
(define-class <type-info> (<base-info>))
(define-class <value-info> (<base-info>))

(eval-when (expand load eval)
  (dynamic-call "ggi_infos_init"
                (dynamic-link ggi-lib)))

;; helper to generate the boilerplace for definine methods which take only the info
;; example:
;; (define-info-methods base-info
;;  (get-name get-type))
;; ->
;; (begin
;;   (define-method (get-name (base-info <base-info>)
;;     (%g-base-info-get-name base-info)))
;;   (define-method (get-type (base-info <base-info>)
;;     (%g-base-info-get-type base-info))))
(define-syntax define-info-methods
  (λ (stx)
    (syntax-case stx ()
      ((_  info-name (method-name))
       (with-syntax
           ((class-name (datum->syntax stx (string->symbol
                                             (string-append
                                              "<"
                                               (symbol->string (syntax->datum #'info-name))
                                               ">"))))
            (ffi-name (datum->syntax stx (string->symbol
                                          (string-append
                                            "%g-"
                                            (symbol->string (syntax->datum #'info-name))
                                            "-"
                                            (symbol->string (syntax->datum #'method-name)))))))
         #'(begin
             (define-method (method-name (info-name class-name))
               (ffi-name info-name))
             (export method-name))))
      ((_ info-name (method-name0 method-name* ...))
       #'(begin
           (define-info-methods info-name (method-name0))
           (define-info-methods info-name (method-name* ...)))))))

;;; BaseInfo
(define-info-methods base-info
  (get-name))

;;; RegisteredTypeInfo
(define-info-methods registered-type-info
  (get-g-type))

;;; ObjectInfo

(define-info-methods object-info
  (get-methods))

;;; ConstantInfo

(define-info-methods constant-info
  (get-value))
