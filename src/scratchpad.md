# Static Semantics

### Scope Rules
- [x] Only usable in scope they were declared in
- [x] Cannot be used before declared
- [ ] Must have a unique name (i.e. cannot be redeclared within the same block)
- [ ] Can be [shadowed](https://en.wikipedia.org/wiki/Variable_shadowing) in later scopes (with the exception of specific routines like `read()`, `write()`, `writeln()`, `int2real()`, `real2int()`, `int2string()`, and `real2string()`)

### Declarations
- [x] Constants must be initialized on declaration
- [x] Variables cannot be initialized during declaration
- [x] Blocks with an identifier after the `end` keyword must be the same as the name of the program, function, or procedure the block is a part of
- [x] Functions can only have value parameters

### Statements
- [x] The left-hand side of an assignment (i.e. the identifier being assigned to) must be an declared, mutable variable or parameter.
- [x] Only functions and procedures can be called (e.g. if foo is a variable, you cannot write `foo()`)
- [ ] Only integer and real variables and reference parameters can be passed to `read()`
- [ ] Only string, integer, and integer expressions can be passed to `write()` and `writeln()`
- [ ] All return statements in a function must be of the type of the return type of the function
- [ ] Return statements without an expression are legal in procedures and in the main program, but in a function
- [ ] The iterator variable of a for-loop is declared by the for loop declaration
- [ ] The iterator varable of the for-loop is effectively an integer variable, but of kind `lille_kind::for_ident` instead of kind `lille_kind::variable` so it cannot be assiged to like a variable

### Expressions
- [ ]  Identifiers in expressions (variables, constants, for-loop control variables, functions, or procedures) MUST be defined, or else the type of the expression is unknown

# Test Programs

### Correct Programs
- [x] 1. Simple assignment
- [x] 2. For-loop
- [x] 3. Multiple levels of scope
- [x] 4. Pass function call result to value parameter of another function
- [x] 5. Masking variable for-loop control variable
- [x] 6. Passing loop control variable to value parameter
- [x] 7. Pass ref parameter to a value parameter
- [x] 8. Recursive function call

### Incorrect Programs
- [x] 1. Assignment to for-loop control variable
- [x] 2. Assignment to undeclared variable
- [x] 3. Violation of type rules during assignment
- [ ] 4. Procedure/function call with mismatched types
- [x] 5. Procedure call with too few parameters
- [x] 6. Procedure call with too many parameters
- [ ] 7. Call to a procedure passing a function with matching return type