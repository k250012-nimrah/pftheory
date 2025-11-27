#include <stdio.h>
#include <string.h>

// Structure to store employee details
struct Employee {
    int id;
    char name[50];
    char designation[50];
    float salary;
};

// Function prototypes
void displayEmployees(struct Employee emp[], int n);
void findHighestSalary(struct Employee emp[], int n);
void searchEmployee(struct Employee emp[], int n);
void giveBonus(struct Employee emp[], int n, float threshold); // for explanation


int main() {
    int n;

    printf("Enter number of employees: ");
    scanf("%d", &n);

    struct Employee emp[n];

    // Input employee records
    for (int i = 0; i < n; i++) {
        printf("\nEnter details for Employee %d:\n", i + 1);
        printf("ID: ");
        scanf("%d", &emp[i].id);

        printf("Name: ");
        scanf("%s", emp[i].name);

        printf("Designation: ");
        scanf("%s", emp[i].designation);

        printf("Salary: ");
        scanf("%f", &emp[i].salary);
    }

    int choice;

    do {
        printf("\n===== Employee Management Menu =====\n");
        printf("1. Display All Employees\n");
        printf("2. Find Employee with Highest Salary\n");
        printf("3. Search Employee (ID or Name)\n");
        printf("4. Give Bonus to Low Salary Employees\n");
        printf("5. Exit\n");
        printf("Enter choice: ");
        scanf("%d", &choice);

        switch(choice) {
            case 1:
                displayEmployees(emp, n);
                break;

            case 2:
                findHighestSalary(emp, n);
                break;

            case 3:
                searchEmployee(emp, n);
                break;

            case 4:
                giveBonus(emp, n, 50000);  // 10% bonus to salary < 50,000
                break;

            case 5:
                printf("Exiting program...\n");
                break;

            default:
                printf("Invalid choice! Try again.\n");
        }

    } while (choice != 5);

    return 0;
}



// ------------------------------------------------------------
// Function 1: Display all employees
// ------------------------------------------------------------
void displayEmployees(struct Employee emp[], int n) {
    printf("\n%-10s %-20s %-20s %-10s\n", "ID", "Name", "Designation", "Salary");
    printf("---------------------------------------------------------------\n");

    for (int i = 0; i < n; i++) {
        printf("%-10d %-20s %-20s %-10.2f\n",
               emp[i].id, emp[i].name, emp[i].designation, emp[i].salary);
    }
}



// ------------------------------------------------------------
// Function 2: Find employee with highest salary
// ------------------------------------------------------------
void findHighestSalary(struct Employee emp[], int n) {
    int index = 0;

    for (int i = 1; i < n; i++) {
        if (emp[i].salary > emp[index].salary) {
            index = i;
        }
    }

    printf("\nEmployee with Highest Salary:\n");
    printf("ID: %d\n", emp[index].id);
    printf("Name: %s\n", emp[index].name);
    printf("Designation: %s\n", emp[index].designation);
    printf("Salary: %.2f\n", emp[index].salary);
}



// ------------------------------------------------------------
// Function 3: Search employee by ID or Name
// ------------------------------------------------------------
void searchEmployee(struct Employee emp[], int n) {
    int choice;
    printf("\nSearch By:\n1. ID\n2. Name\nEnter choice: ");
    scanf("%d", &choice);

    if (choice == 1) {
        int id;
        printf("Enter ID to search: ");
        scanf("%d", &id);

        for (int i = 0; i < n; i++) {
            if (emp[i].id == id) {
                printf("\nEmployee Found:\n");
                printf("ID: %d\nName: %s\nDesignation: %s\nSalary: %.2f\n",
                       emp[i].id, emp[i].name, emp[i].designation, emp[i].salary);
                return;
            }
        }
        printf("No employee found with ID %d.\n", id);
    }

    else if (choice == 2) {
        char name[50];
        printf("Enter Name to search: ");
        scanf("%s", name);

        for (int i = 0; i < n; i++) {
            if (strcmp(emp[i].name, name) == 0) {
                printf("\nEmployee Found:\n");
                printf("ID: %d\nName: %s\nDesignation: %s\nSalary: %.2f\n",
                       emp[i].id, emp[i].name, emp[i].designation, emp[i].salary);
                return;
            }
        }
        printf("No employee found with Name %s.\n", name);
    }

    else {
        printf("Invalid search option.\n");
    }
}



// ------------------------------------------------------------
// Function 4 (Optional): Give 10% bonus to employees below threshold
// ------------------------------------------------------------
void giveBonus(struct Employee emp[], int n, float threshold) {
    printf("\nApplying 10%% bonus to employees below %.2f...\n", threshold);

    for (int i = 0; i < n; i++) {
        if (emp[i].salary < threshold) {
            emp[i].salary += emp[i].salary * 0.10;  // increase by 10%
        }
    }

    printf("Bonus applied successfully.\n");
}
