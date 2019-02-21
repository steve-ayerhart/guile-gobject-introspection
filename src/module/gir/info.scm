(define-module (gir info)
  #:use-module (glib utils)
  #:use-module (gobject gtype)

  #:use-module (oop goops)
  #:use-module (system foreign)
  #:use-module (system foreign-object)

  #:export (<gi-base-info>
            get-name
            <gi-callable-info>
            <gi-function-info>
            <gi-callback-info>
            <gi-signal-info>
            <gi-v-func-info>
            <gi-registered-type-info>
            get-gtype
            <gi-enum-info>
            <gi-interface-info>
            <gi-object-info>
            get-methods
            <gi-struct-info>
            <gi-union-info>
            <gi-arg-info>
            <gi-constant-info>
            get-value
            <gi-field-info>
            <gi-property-info>
            <gi-type-info>))

(eval-when (expand load eval)
  (dynamic-call "gi_infos_init"
                (dynamic-link "/home/steve/Source/guile-gobject-introspection/src/.libs/gobject-introspection")))

;; helper to generate the boilerplace for define-methods which take only the info
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
                                           "%"
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

;;;; BaseInfo

(define-info-methods gi-base-info
  (get-name))

;;;; RegisteredTypeInfo
(define-info-methods gi-registered-type-info
  (get-gtype))

;;;; ObjectInfo

(define-info-methods gi-object-info
  (get-methods))

;;;; ConstantInfo

(define-info-methods gi-constant-info
  (get-value))
