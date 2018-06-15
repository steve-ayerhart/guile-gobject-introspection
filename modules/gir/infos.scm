(define-module (gir infos)
  #:use-module (oop goops)
  #:use-module (system foreign)

  #:export (<g-i-base-info>
            <g-i-callable-info>
            <g-i-function-info>
            <g-i-signal-info>
            <g-i-v-func-info>
            <g-i-registered-type-info>
            <g-i-enum-info>
            <g-i-interface-info>
            <g-i-object-info>
            <g-i-struct-info>
            <g-i-union-info>
            <g-i-arg-info>
            <g-i-constant-info>
            <g-i-field-info>
            <g-i-property-info>
            <g-i-type-info>

            get-name))

(eval-when (expand load eval)
  (dynamic-call "gi_base_info_init"
                (dynamic-link "/home/steve/Source/guile-gobject-introspection/src/.libs/gobject-introspection")))

(define-class <g-i-callable-info> (<g-i-base-info>))
(define-class <g-i-callback-info> (<g-i-callable-info>))
(define-class <g-i-function-info> (<g-i-callable-info>))
(define-class <g-i-signal-info> (<g-i-callable-info>))
(define-class <g-i-v-func-info> (<g-i-callable-info>))

(define-class <g-i-registered-type-info> (<g-i-base-info>))
(define-class <g-i-enum-info> (<g-i-callable-info>))
(define-class <g-i-interface-info> (<g-i-callable-info>))
(define-class <g-i-object-info> (<g-i-callable-info>))
(define-class <g-i-struct-info> (<g-i-callable-info>))
(define-class <g-i-union-info> (<g-i-callable-info>))

(define-class <g-i-arg-info> (<g-i-base-info>))
(define-class <g-i-constant-info> (<g-i-base-info>))
(define-class <g-i-field-info> (<g-i-base-info>))
(define-class <g-i-property-info> (<g-i-base-info>))
(define-class <g-i-type-info> (<g-i-base-info>))
(define-class <g-i-value-info> (<g-i-base-info>))

(eval-when (expand load eval)
  (dynamic-call "gi_infos_init"
                (dynamic-link "/home/steve/Source/guile-gobject-introspection/src/.libs/gobject-introspection")))


(define-method (get-name (base-info <g-i-base-info>))
  (%g-base-info-get-name base-info))
