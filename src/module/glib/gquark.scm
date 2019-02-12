(define-module (glib gquark)
  #:use-module (system foreign)
  #:use-module (ice-9 format)

  #:use-module (glib utils)

  #:export (gquark?
            unwrap-gquark wrap-gquark
            gquark-from-string
            string->gquark
            gquark-to-string
            gquark->string
            gquark-try-string))

(define-glib int32 g_quark_from_string (list '*))
(define-glib '* g_quark_to_string (list uint32))
(define-glib int32 g_quark_try_string (list '*))

(define-wrapped-pointer-type gquark
  gquark? wrap-gquark unwrap-gquark
  (λ (q p)
    (format p  "#<gquark ~d>" (unwrap-gquark q))))

(define (gquark-from-string str)
  (wrap-gquark (g_quark_from_string (string->pointer str))))

(define string->gquark gquark-from-string)

(define (gquark-to-string quark)
  (if (gquark? quark)
      (let ((maybe-null (g_quark_to_string (unwrap-gquark quark))))
        (if (null-pointer? maybe-null)
            ""
            (pointer->string (g_quark_to_string (unwrap-gquark quark)))))
      #f))

(define gquark->string gquark-to-string)

(define (gquark-try-string str)
  (let ((maybe-quark (g_quark_try_string (string->pointer str))))
    (if(= 0 maybe-quark)
       #f
       maybe-quark)))
