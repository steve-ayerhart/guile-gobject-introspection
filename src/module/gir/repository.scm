(define-module (gir repository)
  #:use-module (glib utils)
  #:use-module (gobject gtype)

  #:use-module (oop goops)
  #:use-module (system foreign)

  #:export (get-default
            find-by-g-type
            require
            get-infos
            <gi-base-info>
            get-name
            <gi-callable-info>
            <gi-function-info>
            <gi-callback-info>
            <gi-signal-info>
            <gi-v-func-info>
            <gi-registered-type-info>
            get-g-type
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

(define gi-lib "/home/steve/Source/guile-gobject-introspection/src/.libs/gobject-introspection")

(eval-when (expand load eval)
  (dynamic-call "gi_base_info_init"
                (dynamic-link "/home/steve/Source/guile-gobject-introspection/src/.libs/gobject-introspection")))

(define-class <gi-callable-info> (<gi-base-info>))
(define-class <gi-callback-info> (<gi-callable-info>))
(define-class <gi-function-info> (<gi-callable-info>))
(define-class <gi-signal-info> (<gi-callable-info>))
(define-class <gi-v-func-info> (<gi-callable-info>))

(define-class <gi-registered-type-info> (<gi-base-info>))
(define-class <gi-enum-info> (<gi-registered-type-info>))
(define-class <gi-interface-info> (<gi-registered-type-info>))
(define-class <gi-object-info> (<gi-registered-type-info>))
(define-class <gi-struct-info> (<gi-registered-type-info>))
(define-class <gi-union-info> (<gi-registered-type-info>))

(define-class <gi-arg-info> (<gi-base-info>))
(define-class <gi-constant-info> (<gi-base-info>))
(define-class <gi-field-info> (<gi-base-info>))
(define-class <gi-property-info> (<gi-base-info>))
(define-class <gi-type-info> (<gi-base-info>))
(define-class <gi-value-info> (<gi-base-info>))

(eval-when (expand load eval)
  (dynamic-call "gi_infos_init"
                (dynamic-link "/home/steve/Source/guile-gobject-introspection/src/.libs/gobject-introspection")))

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

;;; GIRepository

(eval-when (expand load eval)
  (dynamic-call "gi_repository_init"
                (dynamic-link "/home/steve/Source/guile-gobject-introspection/src/.libs/gobject-introspection"))) (define-method (require (repository <repository>) (namespace <symbol>))
  (%g-irepository-require repository namespace))

(define-method (get-infos (repository <repository>) (namespace <symbol>))
  (%g-irepository-get-infos repository namespace))

(define-method (find-by-g-type (repository <repository>) (g-type <real>))
  (%g-irepository-find-by-g-type repository g-type))
