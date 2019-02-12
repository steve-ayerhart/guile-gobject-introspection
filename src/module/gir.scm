(define-module (gir)
;  #:use-module (gir repository)
;  #:use-module (gir info)
  #:use-module (glib utils)
  #:use-module (gobject)
  #:use-module (gobject gtype)
  #:use-module (gobject gvalue)
  #:use-module (gobject gparameter)
  #:use-module (gobject gclosure)
  #:use-module (gobject gsignal)

  #:use-module (oop goops)
  #:use-module (system foreign)
  #:use-module (ice-9 receive)
  #:use-module (ice-9 match))

;(eval-when (expand load eval)
;  (dynamic-call "gir_init"
;                (dynamic-link "/home/steve/Source/guile-gobject-introspection/src/.libs/gobject-introspection")))

;(define repository (repository-get-default))
;
;(define (make-gir-module namespace)
;  (let ((gir-module (resolve-module `(gir ,namespace)))
;        (typelib (require repository namespace)))
;    (set-module-public-interface! gir-module gir-module)
;
;    (let process-info ((infos (get-infos repository namespace)))
;      (if (null? infos)
;          gir-module
;          (let ((base-info (car infos)))
;            (build-gi-type! gir-module base-info)
;            (process-info (cdr infos)))))))

;(define (build-gir-module namespace)
;  (let ((gir-module (resolve-module `(gir ,namespace)))
;        (c-namespace (camel-case->scheme-case namespace)))
;
;    (g-i-repository-require c-namespace)
;    (set-module-public-interface! gir-module gir-module)
;
;    (let ((n-infos (g-i-repository-get-n-infos c-namespace)))
;      (let read-info ((index 0))
;        (if (= n-infos index)
;            gir-module
;            (let ((base-info (g-i-repository-get-info c-namespace index)))
;              (receive (type-name type-value)
;                  (build-gi-type base-info)
;                (module-define! gir-module type-name type-value))
;              (read-info (+ index 1))))))))

;(define (function-info->scm-procedure function-info)
;  '())
;
;(define (build-gi-registered-type gir-module info)
;  (define class-name (g-type-name->class-name (get-name info)))
;  (module-define! gir-module
;                  class-name
;                  (get-g-type info))
;  (when (is-a? info <object-info>)
;    (for-each (lambda (method-info)
;                (module-define! gir-module
;                                (g-type-class-name->method-name class-name
;                                                                (string->symbol
;                                                                 (g-type-name->scheme-name (get-name method-info))))
;                                '()))
;              (get-methods info))))
;
;(define (build-gi-type! gir-module info)
;  (cond
;   ((is-a? info <registered-type-info>)
;    (build-gi-registered-type gir-module info))
;   ((is-a? info <constant-info>)
;    (module-define! gir-module
;                    ((compose string->symbol camel-case->snake-case) (get-name info))
;                    (get-value info)))
;     (else
;      (module-define! gir-module
;                      ((compose string->symbol g-type-name->scheme-name) (get-name info))
;                      "BUTTS"))))
