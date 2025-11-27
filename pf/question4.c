#include <stdio.h>

struct Book {
    int id;
    int pop;
    int lastAccess;   // Used to determine least recently accessed
};

// Function to find a book
int findBook(struct Book shelf[], int capacity, int id) {
    for (int i = 0; i < capacity; i++) {
        if (shelf[i].id == id)
            return i;
    }
    return -1;
}

// Function to find an empty slot
int findEmpty(struct Book shelf[], int capacity) {
    for (int i = 0; i < capacity; i++) {
        if (shelf[i].id == -1)
            return i;
    }
    return -1; // no empty slot
}

// Function to remove LRA (Least Recently Accessed) book
int findLRA(struct Book shelf[], int capacity) {
    int idx = 0;
    for (int i = 1; i < capacity; i++) {
        if (shelf[i].lastAccess < shelf[idx].lastAccess)
            idx = i;
    }
    return idx;
}

int main() {
    int capacity, Q;
    scanf("%d %d", &capacity, &Q);

    struct Book shelf[capacity];

    // Initialize shelf as empty
    for (int i = 0; i < capacity; i++) {
        shelf[i].id = -1;
        shelf[i].pop = 0;
        shelf[i].lastAccess = 0;
    }

    int timeCounter = 1;

    while (Q--) {
        char op[10];
        scanf("%s", op);

        if (op[0] == 'A' && op[1] == 'D') { 
            // ADD operation
            int x, y;
            scanf("%d %d", &x, &y);

            int idx = findBook(shelf, capacity, x);

            if (idx != -1) {
                // Book exists → update popularity and access time
                shelf[idx].pop = y;
                shelf[idx].lastAccess = timeCounter++;
            }
            else {
                // Need to add new book
                int empty = findEmpty(shelf, capacity);

                if (empty != -1) {
                    // Empty slot found
                    shelf[empty].id = x;
                    shelf[empty].pop = y;
                    shelf[empty].lastAccess = timeCounter++;
                }
                else {
                    // Shelf full → remove LRA
                    int removeIdx = findLRA(shelf, capacity);
                    shelf[removeIdx].id = x;
                    shelf[removeIdx].pop = y;
                    shelf[removeIdx].lastAccess = timeCounter++;
                }
            }
        }

        else if (op[0] == 'A' && op[1] == 'C') { 
            // ACCESS operation
            int x;
            scanf("%d", &x);

            int idx = findBook(shelf, capacity, x);

            if (idx != -1) {
                // Update lastAccess time because it's accessed
                shelf[idx].lastAccess = timeCounter++;
                printf("%d\n", shelf[idx].pop);
            } else {
                printf("-1\n");
            }
        }
    }

    return 0;
}
