// Example 2: Arithmetic Expressions & Precedence
// Tests operator precedence (*, / before +, -), modulo, parentheses, and unary minus.

int a = 10;
int b = 20;
int c = 5;
int d = 2;

// Standard TAC example: a = b + c * d
int result1;
result1 = b + c * d;

// Parentheses override
int result2;
result2 = (b + c) * d;

// Complex expression with modulo and unary minus
int result3;
result3 = -a + (b / c) * 3 - (15 % 4);

print("Result 1 (20 + 5 * 2 = 30):");
print(result1);

print("Result 2 ((20 + 5) * 2 = 50):");
print(result2);

print("Result 3 (-10 + 4 * 3 - 3 = -1):");
print(result3);
