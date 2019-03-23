(define-module (glib gerror)
  #:use-module (system foreign)
  #:use-module (rnrs bytevectors)
  #:use-module (ice-9 format)
  #:use-module (srfi srfi-9)

  #:use-module (glib utils)
  #:use-module (glib gquark)

  #:export-syntax (with-gerror)

  #:export (make-gerror gerror-domain gerror-code gerror-message
            gerror-new
            gerror-free
            gerror? unwrap-gerror wrap-gerror))

(define-record-type <gerror>
  (make-gerror domain code message)
  gerror?
  (domain gerror-domain)
  (code gerror-code)
  (message gerror-message))

(define-glib '* g_error_new (list uint32 int '*))
(define-glib void g_error_free (list '*))

(define-wrapped-pointer-type gerror
  gerror? wrap-gerror unwrap-gerror
  (λ (e p)
    (let ((gerror (parse-c-struct (unwrap-gerror e) (list uint32 int '*))))
      (format p "#<gerror ~s ~d>"
              (gquark->string (wrap-gquark (car gerror)))
              (cadr gerror)))))

(define (gerror-new quark code message)
  (wrap-gerror
   (g_error_new (unwrap-gquark quark)
                code
                (string->pointer message))))

(define (gerror-free gerror)
  (g_error_free (unwrap-gerror gerror)))

(define-syntax-rule (with-gerror (gerror) body ...)
  (let ((gerror (wrap-gerror (bytevector->pointer (make-bytevector 16)))))
    body ...))
