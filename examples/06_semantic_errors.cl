// Example 6: Semantic Errors Demonstration
// This file intentionally contains multiple semantic violations to demonstrate
// line-numbered semantic error diagnostics in CompiLearn.

// 1. Valid declarations
int validX = 10;
string message = "hello";

// 2. Error: Redeclaration in the same scope
int validX = 20;

// 3. Error: Undeclared variable usage
undeclaredVar = 50;

// 4. Error: Type mismatch in assignment (cannot assign string to int)
int num = "this is not an int";

// 5. Error: Incompatible arithmetic operands (cannot multiply string with int)
int badMath = message * 5;

// 6. Error: Non-boolean/numeric condition in if statement
if ("string condition") {
    print("Should not be allowed");
}

// 7. Error: Scope violation (accessing variable declared in an inner block)
{
    int secret = 999;
}
int leaked = secret;
