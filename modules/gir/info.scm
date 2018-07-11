(define-module (gir info)
  #:use-module (oop goops)
  #:use-module (system foreign)
  #:use-module (system foreign-object)

  #:export (<base-info>
            <callable-info>
            <function-info>
            <signal-info>
            <v-func-info>
            <registered-type-info>
            <enum-info>
            <interface-info>
            <object-info>
            <struct-info>
            <union-info>
            <arg-info>
            <constant-info>
            <field-info>
            <property-info>
            <type-info>

            base-info:get-name
            registered-type-info:get-g-type
            constant-info:get-value
            object-info:get-methods))

(define ggi-lib "/home/steve/Source/guile-gobject-introspection/src/.libs/gobject-introspection")

(eval-when (expand load eval)
  (dynamic-call "ggi_base_info_init"
                (dynamic-link ggi-lib)))

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

;;; BaseInfo
(define-method (base-info:get-name (base-info <base-info>))
  (%g-base-info-get-name base-info))

(define-method (registered-type-info:get-g-type (registered-type-info <registered-type-info>))
  (%g-registered-type-info-get-g-type registered-type-info))

;;; ObjectInfo

(define-method (object-info:get-methods (object-info <object-info>))
  (%g-object-info-get-methods object-info))

;;; ConstantInfo

(define-method (constant-info:get-value (constant-info <constant-info>))
  (%g-constant-info-get-value constant-info))
