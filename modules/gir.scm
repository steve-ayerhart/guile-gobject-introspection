(define-module (gir)
  #:use-module (gir infos)
  #:use-module (gir g-object utils)
  #:use-module (gir g-object)
  #:use-module (gir g-object g-type)
  #:use-module (gir g-object g-value)
  #:use-module (gir g-object g-parameter)
  #:use-module (gir g-object g-closure)
  #:use-module (gir g-object g-signal)

  #:use-module (oop goops)
  #:use-module (system foreign)
  #:use-module (ice-9 receive)
  #:use-module (ice-9 match))

(eval-when (expand load eval)
  (dynamic-call "gir_init"
                (dynamic-link "/home/steve/Source/guile-gobject-introspection/src/.libs/gobject-introspection")))

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

(define (build-gi-type base-info)
  base-info)

