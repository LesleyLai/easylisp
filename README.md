# Easylisp

This is a lisp interpreter written for a course project. It does not intend to be fast or feature rich, and the C++ code
is pretty hacky, but it can probably provide some educational value.

## Build and Run

A recent version of [Visual Studio](https://www.visualstudio.com/), [GCC](https://gcc.gnu.org/),
and [Clang](https://clang.llvm.org/) should be found on your system, they need to support C++20.

You also need to have [CMake](https://cmake.org/) and [Conan](https://conan.io/) package manager installed. You can
install them by

```sh
pip install cmake
pip install conan
```

After installing all the tools, you can build the code with the following commands:

```sh
mkdir build
cd build
cmake ..
make
```

## Usage

After building the project, there should be an `bin/easylisp` executable under the `build` (where you invoked CMake)
folder.

You can run the interpreter as a REPL:

```sh
$ easylisp
>> 1
1
>> (define square (lambda (x) (* x x)))
>> (map square (range 1 6))
(1 4 9 16 25)
>> (cons 1 (list 2 3 4))
(1 2 3 4)
```

Or you can use it to interpret a file:

```sh
$ easylisp file.easylisp
```

You can find some examples in the `scripts` folder. Those scripts will be automatically copied into the same folder of
executable after build.

## Language

### If/Else

``` scheme
(if (> 10 20) 20 10)
```

### Let bindings

`let` creates local variables. The following expression evaluates to the number `15`:

```scheme
(let ((x 10) (y 5)) (+ x y))
```

### Lambda

Lambda expression is the primary way to create new procedurals. The following lambda expression adds `1` to `x` every
time it gets applied:

```scheme
(lambda (x) (+ x 1))
```

The above lambda expression creates an anonymous procedural. To store it in a variable, we can combine it with the let
expression:

```scheme
(let ((+1 (lambda (x) (+ x 1)))) (+1 42))
```

The above code first creates procedural `+1`, and then we apply it with `42`. As a result, we should get `43`.

#### Closure

Like many more modern Lisp dialects, our language has Lexical scope, which means the lambda expression can refer to
variables in outer scope when it is created. For example:

```scheme
(((lambda (y) (lambda (x) (+ x y))) 42) 55)
```

The above expression will evaluate `97` even though the inner lambda expression does not contain a parameter y. The
above code is equivalent to:

```scheme
(((lambda (x) (+ x 42))) 55)
```

### Definition

`define` creates global variables. The following example evaluates to `101`.

```scheme
(define x 100)
(+ x 1)
```

We can combine `define` and `lambda` to create new procedurals. For example:

```scheme
(define cube (lambda (x) (* x x x)))
(cube 2)   ; 8
(cube 10)  ; 1000
(cube 100) ; 1000000
```

We can achieve recursion by using definition. For example:

```scheme
(define fact (lambda (x) (if (< x 2) 1 (* x (fact (- x 1))))))
(fact 0)   ; 1
(fact 1)   ; 1
(fact 2)   ; 2
(fact 3)   ; 6
(fact 10)  ; 3628800
```

### List/Pair

```scheme
(list 1 2 3) ;; (1 2 3)
(cons 1 (list 2 3)) ;; (1 2 3)
(cons 1 2) ;; (1 . 2) - This is a pair instead of a list
```

### builtin constants and procedural

#### Boolean

- `boolean?`
- `true`, `false`
- `and`, `or`, `not`

#### Comparison

- `(eq? x y)`
    - Referential equality comparison
    - examples:
      ```scheme
      (eq? 1 2)                        ; false
      (eq? 1 1)                        ; true
      (eq? (list 1) (list 1))          ; false
      (let ((x (cons 1 2))) (eq? x x)) ; true
      ```
- `(equal? x y)`
    - Value equality comparison
    - examples:
      ```scheme
      (equal? 1 2)                     ; false
      (equal? 1 1)                     ; true
      (equal? (list 1) (list 1))       ; true
      (let ((x (cons 1 2))) (eq? x x)) ; true
      ```

#### Number

- `number?`
- `+`, `-`, `*`, `/`
- `<`, `<=`, `>`, `>=`

#### Procedural

- `procedural?`

#### List

- `null` Empty list
- `(null? v)`
    - Returns `true` if `v` is `null`, `false` otherwise
    - examples:
      ```scheme
      (null? 1)          ; false
      (null? (list))     ; true
      (null? null)       ; true
      (null? (list 1))   ; false
      (null? (cons 1 2)) ; false
      ```
- `(pair? v)`
    - Returns `true` if `v` is a pair, `false` otherwise
    - examples:
      ```scheme
      (pair? 1)          ; false
      (pair? null)       ; false
      (pair? (list 1))   ; true
      (pair? (cons 1 2)) ; true
      ```
- `(list? v)`
    - Returns `true` if `v` is a list, `false` otherwise
    - examples:
      ```scheme
      (list? 1)          ; false
      (list? (list))     ; true
      (list? (list 1))   ; true
      (list? (cons 1 2)) ; false
      ```
- `(list ...)`
    - Constructs a list
    - examples:
      ```scheme
      (list)     ; null
      (list 1 2) ; (1 2)
      ```
- `(range lower upper)`
    - `lower: number?`
    - `upper: number?`
    - Constructs a list of integers from range `[lower, upper)`
    - examples:
      ```scheme
      (range 0 5) ; (0 1 2 3 4)
      (range 5 0) ; ()
      ```
- `(cons v1 v2)`
    - Constructs a pair
    - examples:
      ```scheme
      (cons 1 2)       ; (1 . 2)
      (cons 1 (list 2) ; (1 2)
      ```
- `(car p)`
    - `p: pair?`
    - Returns the first element of a pair
    - examples:
      ```scheme
      (car (list 1 2)) ; 1
      (car (cons 1 2)) ; 1
      ```
- `(cdr p)`
    - `p: pair?`
    - Returns the second element of a pair
    - examples:
      ```scheme
      (cdr (list 1 2)) ; (2)
      (cdr (cons 1 2)) ; 2
      ```
- `(map proc l)`
    - `proc: procedural?`
    - `l: list?`
    - Creates a new list by applying `proc` to each element of `l`
    - examples:
      ```scheme
      (map (lambda (x) (+ x 1)) (list 1 2 3 4 5))
      ;; (2 3 4 5 6)
      ```
- `(filter pred l)`
    - `pred: procedural?`
    - `l: list?`
    - Returns a list with the elements of `l` for which `pred` produces `true`.
    - examples:
      ```scheme
      (filter (lambda (x) (> x 0)) (list 1 -1 2 -2 3))
      ;; (1 2 3)
      ```
- `(foldl proc init l)`
    - `proc: procedural?`
    - `l: list?`
    - Folds from left to right
    - examples:
      ```scheme
      (foldl * 1 (range 1 5)) ;; 24
      (foldl cons (list 42) (range 0 5))
      ;; (4 3 2 1 0 42)
      ```
- `(foldr proc init l)`
    - `proc: procedural?`
    - `l: list?`
    - Folds from right to left
    - examples:
      ```scheme
      (foldr * 1 (range 1 5)) ;; 24
      (foldr cons (list 42) (range 0 5))
      ;; (0 1 2 3 4 42)
      ```

#### Printing

- `(print v)`
    - Prints the value `v` and an endline

## License

Copyright 2021 Lesley Lai & Yu Li & Valyria Mcfarland

This repository is released under the MIT license. See [License](file:License) for more information.
