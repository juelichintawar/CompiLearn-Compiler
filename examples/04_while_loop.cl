// Example 4: While Loop (Factorial Calculation)
// Tests while loop structure, condition check, and loop variable update.

int n = 5;
int factorial = 1;
int i = 1;

while (i <= n) {
    factorial = factorial * i;
    i = i + 1;
}

print("Factorial of 5 is:");
print(factorial);
