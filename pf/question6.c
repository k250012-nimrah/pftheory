#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DATAFILE "members.dat"
#define NAME_LEN 100
#define BATCH_LEN 32
#define TYPE_LEN 16
#define DATE_LEN 11
#define INTEREST_LEN 16

typedef struct {
    int id;
    char name[NAME_LEN];
    char batch[BATCH_LEN];           // CS, SE, Cyber Security, AI
    char membershipType[TYPE_LEN];   // IEEE / ACM
    char registrationDate[DATE_LEN];
    char dob[DATE_LEN];
    char interest[INTEREST_LEN];     // IEEE / ACM / Both
} Student;

typedef struct {
    Student *arr;
    size_t size;
    size_t capacity;
} Database;

// ----- Memory helpers -----
void *xmalloc(size_t n) {
    void *p = malloc(n);
    if (!p) {
        perror("malloc failed");
        exit(1);
    }
    return p;
}

void *xrealloc(void *ptr, size_t n) {
    void *p = realloc(ptr, n);
    if (!p) {
        perror("realloc failed");
        exit(1);
    }
    return p;
}

void initDatabase(Database *db) {
    db->arr = NULL;
    db->size = 0;
    db->capacity = 0;
}

void freeDatabase(Database *db) {
    free(db->arr);
    db->arr = NULL;
    db->size = 0;
    db->capacity = 0;
}

void ensureCapacity(Database *db, size_t minCapacity) {
    if (db->capacity >= minCapacity) return;
    size_t newCap = db->capacity ? db->capacity * 2 : 8;
    while (newCap < minCapacity) newCap *= 2;
    db->arr = (Student *) xrealloc(db->arr, newCap * sizeof(Student));
    db->capacity = newCap;
}

// ----- File operations -----
int loadDatabase(Database *db, const char *filename) {
    FILE *f = fopen(filename, "rb");
    if (!f) return 0; // No file = empty DB
    fseek(f, 0, SEEK_END);
    long fileSize = ftell(f);
    rewind(f);
    size_t recCount = (size_t)(fileSize / sizeof(Student));
    if (recCount == 0) { fclose(f); return 0; }
    ensureCapacity(db, recCount);
    size_t read = fread(db->arr, sizeof(Student), recCount, f);
    db->size = read;
    fclose(f);
    return 0;
}

int saveDatabase(const Database *db, const char *filename) {
    FILE *f = fopen(filename, "wb");
    if (!f) { perror("Cannot open file for write"); return -1; }
    if (db->size > 0) {
        if (fwrite(db->arr, sizeof(Student), db->size, f) != db->size) {
            perror("Error writing to file");
            fclose(f);
            return -1;
        }
    }
    fclose(f);
    return 0;
}

// ----- In-memory operations -----
size_t findStudentIndex(const Database *db, int id) {
    for (size_t i = 0; i < db->size; ++i)
        if (db->arr[i].id == id) return (size_t)i;
    return -1;
}

int addStudent(Database *db, const Student *s) {
    if (findStudentIndex(db, s->id) != -1) return -1; // duplicate
    ensureCapacity(db, db->size + 1);
    db->arr[db->size++] = *s;
    return 0;
}

int deleteStudent(Database *db, int id) {
    size_t idx = findStudentIndex(db, id);
    if (idx < 0) return -1;
    size_t i = (size_t)idx;
    if (i + 1 < db->size)
        memmove(&db->arr[i], &db->arr[i + 1], (db->size - i - 1) * sizeof(Student));
    db->size--;
    return 0;
}

int updateStudent(Database *db, int id, const char *newBatch, const char *newMembership) {
    size_t idx = findStudentIndex(db, id);
    if (idx < 0) return -1;
    Student *s = &db->arr[idx];
    if (newBatch && newBatch[0] != '\0') strncpy(s->batch, newBatch, BATCH_LEN-1);
    if (newMembership && newMembership[0] != '\0') strncpy(s->membershipType, newMembership, TYPE_LEN-1);
    s->batch[BATCH_LEN-1]='\0';
    s->membershipType[TYPE_LEN-1]='\0';
    return 0;
}

// ----- Input helpers -----
void readString(const char *prompt, char *buf, size_t size) {
    printf("%s", prompt);
    if (fgets(buf, (int)size, stdin)) {
        size_t len = strlen(buf);
        if (len && buf[len-1]=='\n') buf[len-1]='\0';
    }
}

int readInt(const char *prompt) {
    char buf[32];
    readString(prompt, buf, sizeof(buf));
    return atoi(buf);
}

// ----- Display functions -----
void displayAll(const Database *db) {
    printf("\nID\tName\tBatch\tMembership\tRegDate\tDOB\tInterest\n");
    printf("---------------------------------------------------------------\n");
    for (size_t i=0;i<db->size;i++) {
        Student *s=&db->arr[i];
        printf("%d\t%s\t%s\t%s\t%s\t%s\t%s\n",
               s->id, s->name, s->batch, s->membershipType,
               s->registrationDate, s->dob, s->interest);
    }
}

void displayBatchReport(const Database *db, const char *batch, const char *membership) {
    printf("\nReport for batch=%s, membership=%s:\n", batch, membership);
    printf("ID\tName\tRegDate\tDOB\tInterest\n");
    printf("------------------------------------------\n");
    for (size_t i=0;i<db->size;i++) {
        Student *s=&db->arr[i];
        if (strcmp(s->batch,batch)==0 && (strcmp(s->membershipType,membership)==0 || strcmp(s->interest,membership)==0 || strcmp(s->interest,"Both")==0))
            printf("%d\t%s\t%s\t%s\t%s\n", s->id,s->name,s->registrationDate,s->dob,s->interest);
    }
}

// ----- Menu -----
void menu() {
    printf("\n--- IEEE / ACM Membership Manager ---\n");
    printf("1. Register new student\n");
    printf("2. Update student\n");
    printf("3. Delete student\n");
    printf("4. View all students\n");
    printf("5. Batch-wise report\n");
    printf("6. Exit\n");
}

int main() {
    Database db;
    initDatabase(&db);
    loadDatabase(&db, DATAFILE);

    int choice;
    while (1) {
        menu();
        choice = readInt("Enter choice: ");
        if (choice==1) {
            Student s;
            s.id = readInt("Enter Student ID: ");
            if (findStudentIndex(&db,s.id)!=-1) { printf("Duplicate ID!\n"); continue; }
            readString("Full Name: ", s.name, NAME_LEN);
            readString("Batch (CS/SE/Cyber Security/AI): ", s.batch, BATCH_LEN);
            readString("Membership Type (IEEE/ACM): ", s.membershipType, TYPE_LEN);
            readString("Registration Date (YYYY-MM-DD): ", s.registrationDate, DATE_LEN);
            readString("Date of Birth (YYYY-MM-DD): ", s.dob, DATE_LEN);
            readString("Interest (IEEE/ACM/Both): ", s.interest, INTEREST_LEN);
            if (addStudent(&db,&s)==0) saveDatabase(&db, DATAFILE);
        }
        else if (choice==2) {
            int id = readInt("Enter Student ID to update: ");
            char batch[BATCH_LEN], membership[TYPE_LEN];
            readString("New Batch (leave blank to keep): ", batch, BATCH_LEN);
            readString("New Membership Type (leave blank to keep): ", membership, TYPE_LEN);
            if (updateStudent(&db,id,batch,membership)==0) saveDatabase(&db, DATAFILE);
            else printf("Student not found.\n");
        }
        else if (choice==3) {
            int id = readInt("Enter Student ID to delete: ");
            if (deleteStudent(&db,id)==0) saveDatabase(&db, DATAFILE);
            else printf("Student not found.\n");
        }
        else if (choice==4) {
            displayAll(&db);
        }
        else if (choice==5) {
            char batch[BATCH_LEN], membership[TYPE_LEN];
            readString("Enter batch: ", batch, BATCH_LEN);
            readString("Enter membership (IEEE/ACM/Both): ", membership, TYPE_LEN);
            displayBatchReport(&db,batch,membership);
        }
        else if (choice==6) break;
        else printf("Invalid choice!\n");
    }

    freeDatabase(&db);
    printf("Exiting program.\n");
    return 0;
}
