# Definition of Function Semantics Used in the C++ Standards Specification

(Copied from section 16.3.2.4 of the C++26 Standard Specification)

Descriptions of function semantics contain the following elements (as appropriate) (see Note 135)
- **Constraints**: the conditions for the function’s participation in overload resolution (12.2).
  [Note 1 : Failure to meet such a condition results in the function’s silent non-viability. —end note]
  [Example 1 : An implementation can express such a condition via a constraint-expression (13.5.3). —end
  example]
- **Mandates**: the conditions that, if not met, render the program ill-formed.
  [Example 2 : An implementation can express such a condition via the constant-expression in a static_assert declaration
  (9.1). If the diagnostic is to be emitted only after the function has been selected by overload
  resolution, an implementation can express such a condition via a constraint-expression (13.5.3) and also define
  the function as deleted. —end example]
- **Constant When**: the conditions that are required for a call to the function to be a constant subexpression.
- **Preconditions**: conditions that the function assumes to hold whenever it is called; violation of any
  preconditions results in undefined behavior.
  [Example 3 : An implementation can express some such conditions via the use of a contract assertion, such as a
  precondition assertion (9.4.1). —end example]
- **Hardened preconditions**: conditions that the function assumes to hold whenever it is called.
  - When invoking the function in a hardened implementation, prior to any other observable side
    effects of the function, one or more contract assertions whose predicates are as described in the
    hardened precondition are evaluated with a checking semantic (6.11.2). If any of these assertions
    is evaluated with a non-terminating semantic and the contract-violation handler returns, the
    program has undefined behavior.
  - When invoking the function in a non-hardened implementation, if any hardened precondition is violated, 
    the program has undefined behavior.
- **Effects**: the actions performed by the function.
- **Synchronization**: the synchronization operations (6.10.2) applicable to the function.
- **Postconditions**: the conditions (sometimes termed observable results) established by the function.
  [Example 4 : An implementation can express some such conditions via the use of a contract assertion, such as a
  postcondition assertion (9.4.1). —end example]
- **Result**: for a typename-specifier, a description of the named type; for an expression, a description of the
  type and value category of the expression; the expression is an lvalue if the type is an lvalue reference
  type, an xvalue if the type is an rvalue reference type, and a prvalue otherwise.
- **Returns**: a description of the value(s) returned by the function.
- **Throws**: any exceptions thrown by the function, and the conditions that would cause the exception.
- **Complexity**: the time and/or space complexity of the function.
- **Remarks**: additional semantic constraints on the function.
- **Error conditions**: the error conditions for error codes reported by the function.

Whenever the Effects element specifies that the semantics of some function F are Equivalent to some code
sequence, then the various elements are interpreted as follows. If F’s semantics specifies any Constraints
or Mandates elements, then those requirements are logically imposed prior to the equivalent-to semantics.
Next, the semantics of the code sequence are determined by the Constraints, Mandates, Constant When,
Preconditions, Hardened preconditions, Effects, Synchronization, Postconditions, Returns, Throws, Complexity,
Remarks, and Error conditions specified for the function invocations contained in the code sequence. The
value returned from F is specified by F’s Returns element, or if F has no Returns element, a non-void return
from F is specified by the return statements (8.8.4) in the code sequence. If F’s semantics contains a Throws,
Postconditions, or Complexity element, then that supersedes any occurrences of that element in the code
sequence.

For non-reserved replacement and handler functions, Clause 17 specifies two behaviors for the functions in
question: their required and default behavior. The default behavior describes a function definition provided
by the implementation. The required behavior describes the semantics of a function definition provided by
either the implementation or a C++ program. Where no distinction is explicitly made in the description, the
behavior described is the required behavior.

If the formulation of a complexity requirement calls for a negative number of operations, the actual requirement
is zero operations.136

Complexity requirements specified in the library clauses are upper bounds, and implementations that provide
better complexity guarantees meet the requirements.

Error conditions specify conditions where a function may fail. The conditions are listed, together with a
suitable explanation, as the enum class errc constants (19.5).

Note 135) To save space, elements that do not apply to a function are omitted. For example, if a function specifies no preconditions,
there will be no Preconditions: element.
