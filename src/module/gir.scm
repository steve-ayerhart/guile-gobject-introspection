(define-module (gir)
  #:use-module (gir repository)
  #:use-module (gir info)

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

(define repository (gi-repository-get-default))

(define (build-gir-module namespace)
  (when (require repository namespace)
    (let ((namespace-module (resolve-module `(gir ,namespace) #f)))
      (set-module-public-interface! namespace-module namespace-module)

      (module-use! namespace-module (resolve-module '(oop goops)))

      (save-module-excursion
       (λ ()
         (let ((gir-module (set-current-module namespace-module)))
           (let process-info ((infos (get-infos repository namespace)))
             (if (null? infos)
                 namespace-module
                 (let ((base-info (car infos)))
                   (build-gi-type! base-info)
                   (process-info (cdr infos)))))))))))


(define (build-gi-registered-type! info)
  (let ((class-name (gtype-name->class-name (get-name info))))
    (module-define! (current-module)
                    class-name
                    (get-gtype info))
    (when (is-a? info <gi-object-info>)
      (for-each
       (λ (method-info)
         (let ((method-name ((compose string->symbol gtype-name->scheme-name) (get-name method-info))))
           (module-define! (current-module)
                           method-name
                           (make <method>
                             #:dsupers `(,<gobject> <object>)
                             #:specializers `(,class-name)
                             #:procedure (λ (self) "BOO")))))
       (get-methods info)))))


(define (build-constant-type info)
  (let ((constant-name ((compose string->symbol camel-case->snake-case) (get-name info))))
    (module-define! (current-module) constant-name (get-value info))
    (export constant-name)))

(define (build-gi-type! info)
  (cond
   ((is-a? info <gi-registered-type-info>)
    (build-gi-registered-type! info))
   ((is-a? info <gi-constant-info>)
    (build-constant-type info))
   (else
    (module-define! (current-module)
                    ((compose string->symbol gtype-name->scheme-name) (get-name info))
                    "BUTTS"))))
