(define-module (gir g-object g-lib g-error)
  #:use-module (system foreign)
  #:use-module (rnrs bytevectors)
  #:use-module (ice-9 format)

  #:use-module (gir g-object utils)
  #:use-module (gir g-object g-lib g-quark)

  #:export-syntax (with-g-error)

  #:export (g-error-new
            g-error-free
            g-error? unwrap-g-error wrap-g-error))

(define-g-lib '* g_error_new (list uint32 int '*))
(define-g-lib void g_error_free (list '*))

(define-wrapped-pointer-type g-error
  g-error? wrap-g-error unwrap-g-error
  (λ (e p)
    (let ((g-error (parse-c-struct (unwrap-g-error e) (list uint32 int '*))))
      (format p "#<g-error ~s ~d>"
              (g-quark->string (wrap-g-quark (car g-error)))
              (cadr g-error)))))

(define (g-error-new quark code message)
  (wrap-g-error
   (g_error_new (unwrap-g-quark quark)
                code
                (string->pointer message))))

(define (g-error-free g-error)
  (g_error_free (unwrap-g-error g-error)))

(define-syntax-rule (with-g-error (g-error) body ...)
  (let ((g-error (wrap-g-error (bytevector->pointer (make-bytevector 16)))))
    body ...))
