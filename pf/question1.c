#include <stdio.h>

double calculateRepayment(double loan, double interestRate, int years, double installment) {

    // Base case
    if (loan <= 0 || years == 0) {
        return 0;
    }

    // Apply interest
    loan = loan + (loan * interestRate);

    // Subtract yearly installment
    loan -= installment;

    // Print status
    printf("Year %d: Remaining loan = %.2f\n", years, loan);

    // Recursive case
    return installment + calculateRepayment(loan, interestRate, years - 1, installment);
}

int main() {
    double loan = 100000;
    double interestRate = 0.05;
    int years = 3;
    double installment = 40000; // fixed yearly payment

    printf("Loan Repayment Schedule:\n");
    double totalPaid = calculateRepayment(loan, interestRate, years, installment);

    printf("\nTotal repayment over %d years = %.2f\n", years, totalPaid);

    return 0;
}
