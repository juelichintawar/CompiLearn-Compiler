// Example 3: Conditional Branching with If / Else
// Demonstrates relational and logical operators with nested branching.

int score = 85;
int passing = 60;
int honors = 80;

if (score >= honors) {
    print("Grade: Distinction!");
} else {
    if (score >= passing) {
        print("Grade: Pass");
    } else {
        print("Grade: Needs Improvement");
    }
}

bool hasBonus = true;
if (score > 80 && hasBonus) {
    print("Eligible for scholarship bonus");
}
