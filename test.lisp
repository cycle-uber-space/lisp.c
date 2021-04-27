
;;(def std (make-env *env*))
;;
;;(with-env std
;;  (load-file "std.lisp"))
;;
;;(println 'pi: (with-env std pi))

(load-file "std.lisp")

(defmacro test (exp)
  `(println ',exp '=> ,exp))

(test nil)
(test t)

(test 'foo)

(test (if nil 'a 'b))
(test (if 'eq 'a 'b))
