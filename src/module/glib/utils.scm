-(define-module (glib utils)
  #:use-module (system foreign)
  #:use-module (srfi srfi-42)
  #:use-module (oop goops)
  #:use-module (ice-9 list)

  #:export (camel-case->snake-case
            gtype-name->scheme-name
            gtype-name->class-name
            gtype-class-name->method-name
            class-name->gtype-name
            ggi-make-method  ggi-add-method!
            char**->list)

  #:export-syntax (define/docs define-generic/docs define-class/docs with-accessors
                    define-glib define-gobject define-gir
                    define-enumeration))

(define (ggi-make-method specializers procdure)
  (make <method>
    #:specializers specializers
    #:procedure procedure))

(define (ggi-add-method! generic method)
  (add-method! generic method))

(define-syntax define-enumeration
  (λ (stx)
    (syntax-case stx ()
      ((_ enumerator (name value) ...)
       #'(define-public enumerator
           (λ (lookup)
             (let ((enums '((name . value) ...)))
               (cond ((symbol? lookup)
                      (assq-ref enums lookup))
                     ((number? lookup)
                      (car (rassq lookup enums)))
                     (#t #f))))))
      ((_ enumerator (enum ...))
       #'(define-public enumerator
           (λ (lookup)
             (let ((enums (map cons
                               '(enum ...)
                               (list-ec (:range i (length '(enum ...))) i))))
               (cond ((symbol? lookup)
                      (assq-ref enums lookup))
                     ((number? lookup)
                      (car (rassq lookup enums)))
                     (#t #f)))))))))

;(define (foreign-func name)
;  (let ((lib (dynamic-link name)))
;    (λ (return-type function-name arg-types)
;      (pointer->procedure return-type
;                          (dynamic-func function-name lib)
;                          arg-types))))

;(define gir-func (foreign-func "girepository-1.0"))
;(define gobject-func (foreign-func "gobject-2.0"))
;(define glib-func (foreign-func "glib-2.0"))

(eval-when (expand load eval)
  (define-syntax define-macro/docs
    (lambda (x)
      "Define a defmacro with documentation."
      (syntax-case x ()
        ((_ (f . args) doc b0 b* ...)
         #'(define-macro (f . args)
             doc b0 b* ...)))))

  (define-syntax define-gnome-definer
    (λ (stx)
      (syntax-case stx ()
        ((_ lib)
         (with-syntax ((func-name (datum->syntax stx
                                                 (string->symbol (string-append
                                                                  (symbol->string (syntax->datum #'lib))
                                                                  "-func"))))
                       (syntax-name (datum->syntax stx
                                                   (string->symbol (string-append
                                                                    "define-"
                                                                    (symbol->string (syntax->datum #'lib)))))))
           #'(define-syntax syntax-name
               (λ (stx)
                 (syntax-case stx ()
                   ((_ return-type name arg-types)
                    #'(define-public name (func-name return-type (symbol->string 'name) arg-types))))))))))))

(define-gnome-definer gir)
(define-gnome-definer gobject)
(define-gnome-definer glib)

(define-macro/docs (define/docs name docs val)
  "Define @var{name} as @var{val}, documenting the value with
@var{docs}."
  `(begin
     (define ,name ,val)
     (set-object-property! ,name 'documentation ,docs)))

(define-macro/docs (define-generic/docs name documentation)
  "Define a generic named @var{name}, with documentation
@var{documentation}."
  `(define/docs ,name ,documentation
     (make-generic ',name)))

(define-macro/docs (define-class/docs name supers docs . rest)
  "Define a class named @var{name}, with superclasses @var{supers}, with
documentation @var{docs}."
  `(begin
     (define-class ,name ,supers ,@rest)
     (set-object-property! ,name 'documentation ,docs)))


(define (class-name->gtype-name class-name)
  "Convert the name of a class into a suitable name for a GType. For example:
@lisp
(class-name->gtype-name '<foo-bar>) @result{} \"FooBar\"
@end lisp"
(list->string
 (reverse!
  (let loop ((to-process (string->list (symbol->string class-name))) (ret '()) (caps? #t))
    (cond
     ((null? to-process)
      ret)
     ((char-alphabetic? (car to-process))
      (loop (cdr to-process)
            (cons (if caps? (char-upcase (car to-process)) (car to-process)) ret)
            #f))
     ((char-numeric? (car to-process))
      (loop (cdr to-process)
            (cons (car to-process) ret)
            #f))
     (else
      (loop (cdr to-process) ret #t)))))))

(define (camel-case->snake-case nstr)
  "Expand the StudlyCaps @var{nstr} to a more schemey-form, according to
the conventions of GLib libraries. For example:
@lisp
 (GStudlyCapsExpand \"GSource\") @result{} gsource
 (GStudlyCapsExpand \"GtkIMContext\") @result{} gtk-im-context
 (GStudlyCapsExpand \"GtkHBox\") @result{} gtk-hbox
@end lisp"
  (do ((idx (+ -1 (string-length nstr)) (+ -1 idx)))
      ((> 1 idx) (string-downcase nstr))
    (cond ((and (> idx 2)
                (char-lower-case? (string-ref nstr (+ -3 idx)))
                (char-upper-case? (string-ref nstr (+ -2 idx)))
                (char-upper-case? (string-ref nstr (+ -1 idx)))
                (char-lower-case? (string-ref nstr idx)))
           (set! idx (1- idx))
           (set! nstr
             (string-append (substring nstr 0 (+ -1 idx))
                            "-"
                            (substring nstr (+ -1 idx)
                                       (string-length nstr)))))
          ((and (> idx 1)
                (char-upper-case? (string-ref nstr (+ -1 idx)))
                (char-lower-case? (string-ref nstr idx)))
           (set! nstr
             (string-append (substring nstr 0 (+ -1 idx))
                            "-"
                            (substring nstr (+ -1 idx)
                                       (string-length nstr)))))
          ((and (char-lower-case? (string-ref nstr (+ -1 idx)))
                (char-upper-case? (string-ref nstr idx)))
           (set! nstr
             (string-append (substring nstr 0 idx)
                            "-"
                            (substring nstr idx
                                       (string-length nstr))))))))

(define (gtype-name->scheme-name type-name)
  "Transform a name of a @code{<gtype>}, such as \"GtkWindow\", to a
scheme form, such as @code{gtk-window}, taking into account the
exceptions in @code{gtype-name->scheme-name-alist}, and trimming
trailing dashes if any."
  (string-trim-right
   (camel-case->snake-case ;; only change _ to -, other characters are not valid in a type name
    (string-map (lambda (c) (if (eq? c #\_) #\- c)) type-name))
   #\-))

;; "GtkAccelGroup" => <gtk-accel-group>
;; "GSource*" => <g-source*>
(define (gtype-name->class-name type-name)
  "Transform a name of a @code{<gtype>}, such as \"GtkWindow\", to a
suitable name of a Scheme class, such as @code{<gtk-window>}. Uses
@code{gtype-name->scheme-name}."
  (string->symbol
   (string-append "<" (gtype-name->scheme-name type-name) ">")))

(define (gtype-class-name->method-name class-name name)
  "Generate the name of a method given the name of a @code{<gtype>} and
the name of the operation. For example:
@lisp
 (gtype-name->method-name \"GtkFoo\" \"bar\") @result{} gtk-foo:bar
@end lisp
Uses @code{gtype-name->scheme-name}."
  (let ((class-string (symbol->string class-name)))
    (string->symbol
     (string-append (substring class-string 1 (1- (string-length class-string)))
                    ":" (symbol->string name)))))

(define-macro (with-accessors names . body)
  `(let (,@(map (lambda (name)
                  ;; Ew, fixme.
                  `(,name (make-procedure-with-setter
                           (lambda (x) (slot-ref x ',name))
                           (lambda (x y) (slot-set! x ',name y)))))
                names))
     ,@body))

(define (char**->list char**)
  (let loop ((strings '())
             (offset 0))
    (let* ((bv (pointer->bytevector char** (sizeof '*) offset))
           (char* ((compose dereference-pointer bytevector->pointer) bv)))
      (if (null-pointer? char*)
          strings
        (loop (cons (pointer->string char*) strings)
              (+ (sizeof '*) offset))))))

(define (one? val)
  (= 1 val))
