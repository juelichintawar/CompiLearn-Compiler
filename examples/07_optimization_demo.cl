// Example 7: Optimizer Demonstration
// Showcases Constant Folding, Constant Propagation, Copy Propagation, and Dead Code Elimination.

int a = 5;
int b = 10;

// Constant folding opportunities:
int c = 10 + 20 * 2;   // 10 + 40 -> 50
int d = (15 - 5) / 2;   // 10 / 2 -> 5

// Propagation:
int x = c + d;

// Unused computation that should be optimized out:
int unused1 = 100 * 3;

print("Computed result x:");
print(x);
