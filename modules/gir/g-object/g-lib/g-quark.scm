(define-module (gir g-object g-lib g-quark)
  #:use-module (system foreign)
  #:use-module (ice-9 format)

  #:use-module (gir g-object utils)

  #:export (g-quark?
            unwrap-g-quark wrap-g-quark
            g-quark-from-string
            string->g-quark
            g-quark-to-string
            g-quark->string
            g-quark-try-string))

(define-g-lib int32 g_quark_from_string (list '*))
(define-g-lib '* g_quark_to_string (list uint32))
(define-g-lib int32 g_quark_try_string (list '*))

(define-wrapped-pointer-type g-quark
  g-quark? wrap-g-quark unwrap-g-quark
  (λ (q p)
    (format p  "#<g-quark ~d>" (unwrap-g-quark q))))

(define (g-quark-from-string str)
  (wrap-g-quark (g_quark_from_string (string->pointer str))))

(define string->g-quark g-quark-from-string)

(define (g-quark-to-string quark)
  (if (g-quark? quark)
      (let ((maybe-null (g_quark_to_string (unwrap-g-quark quark))))
        (if (null-pointer? maybe-null)
            ""
            (pointer->string (g_quark_to_string (unwrap-g-quark quark)))))
      #f))

(define g-quark->string g-quark-to-string)

(define (g-quark-try-string str)
  (let ((maybe-quark (g_quark_try_string (string->pointer str))))
    (if(= 0 maybe-quark)
       #f
       maybe-quark)))
