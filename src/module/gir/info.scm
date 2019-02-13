(define-module (gir info)
  #:use-module (gobject gtype)

  #:use-module (oop goops)
  #:use-module (system foreign)
  #:use-module (system foreign-object)



;; helper to generate the boilerplace for definine methods which take only the info
;; example:
;; (define-info-methods base-info
;;  (get-name get-type))
;; ->
;; (begin
;;   (define-method (get-name (base-info <base-info>)
;;     (%g-base-info-get-name base-info)))
;;   (define-method (get-type (base-info <base-info>)
;;     (%g-base-info-get-type base-info))))
