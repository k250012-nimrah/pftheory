
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/* ---- Helper: safe allocation wrappers ---- */

static void *xmalloc(size_t n) {
    void *p = malloc(n);
    if (!p) {
        perror("malloc failed");
        fprintf(stderr, "Fatal: allocation of %zu bytes failed\n", n);
        exit(EXIT_FAILURE);
    }
    return p;
}

static void *xrealloc(void *ptr, size_t n) {
    void *p = realloc(ptr, n);
    if (!p) {
        perror("realloc failed");
        fprintf(stderr, "Fatal: reallocation to %zu bytes failed\n", n);
        exit(EXIT_FAILURE);
    }
    return p;
}

/* ---- Dynamic buffer structure ---- */

typedef struct {
    char **lines;      /* dynamic array of pointers to C-strings */
    size_t size;       /* number of stored lines */
    size_t capacity;   /* allocated slots in lines[] */
} LineBuffer;

/* initialize buffer with initial capacity */
void initBuffer(LineBuffer *buf, size_t initialCapacity) {
    if (initialCapacity == 0) initialCapacity = 4;
    buf->lines = (char **) xmalloc(initialCapacity * sizeof(char *));
    buf->size = 0;
    buf->capacity = initialCapacity;
    for (size_t i = 0; i < buf->capacity; ++i) buf->lines[i] = NULL;
}

/* free all strings and the pointer array */
void freeAll(LineBuffer *buf) {
    if (!buf) return;
    for (size_t i = 0; i < buf->size; ++i) {
        free(buf->lines[i]); /* free each string */
        buf->lines[i] = NULL;
    }
    free(buf->lines);
    buf->lines = NULL;
    buf->size = 0;
    buf->capacity = 0;
}

/* ensure capacity >= minCapacity */
void ensureCapacity(LineBuffer *buf, size_t minCapacity) {
    if (buf->capacity >= minCapacity) return;
    size_t newCap = buf->capacity ? buf->capacity : 4;
    while (newCap < minCapacity) newCap *= 2;
    buf->lines = (char **) xrealloc(buf->lines, newCap * sizeof(char *));
    /* initialize new slots to NULL so deletion/free logic remains simple */
    for (size_t i = buf->capacity; i < newCap; ++i) buf->lines[i] = NULL;
    buf->capacity = newCap;
}

/* shrink pointer array to exactly fit the number of stored lines */
void shrinkToFit(LineBuffer *buf) {
    if (buf->capacity == buf->size) return; /* already fits */
    if (buf->size == 0) {
        free(buf->lines);
        buf->lines = NULL;
        buf->capacity = 0;
        return;
    }
    buf->lines = (char **) xrealloc(buf->lines, buf->size * sizeof(char *));
    buf->capacity = buf->size;
}

/* ---- Safe line input: read an entire line of arbitrary length ----
 * Returns pointer to malloc'ed string (without trailing newline, unless
 * line was empty and contained only newline -> returns empty string "").
 * Caller must free() returned pointer.
 * On EOF (no characters read), returns NULL.
 */
char *readLineSafe(void) {
    size_t bufSize = 128;
    size_t pos = 0;
    char *buffer = (char *) xmalloc(bufSize);
    int c;

    while ((c = getchar()) != EOF) {
        if (c == '\n') break;
        /* ensure space for c and terminating NUL */
        if (pos + 2 > bufSize) {
            bufSize *= 2;
            buffer = (char *) xrealloc(buffer, bufSize);
        }
        buffer[pos++] = (char) c;
    }

    if (c == EOF && pos == 0) {
        /* nothing read and EOF -> signal EOF to caller */
        free(buffer);
        return NULL;
    }

    /* terminate string */
    buffer[pos] = '\0';

    /* shrink to exact size (pos + 1) */
    buffer = (char *) xrealloc(buffer, pos + 1);
    return buffer;
}

/* Insert a line at index (0..size). If index == size -> append.
 * The function takes ownership of 'text' pointer (caller must not free it).
 * If text is NULL -> treat as empty string (makes a copy).
 */
void insertLine(LineBuffer *buf, size_t index, const char *text) {
    if (!buf) return;
    if (index > buf->size) {
        fprintf(stderr, "insertLine: index %zu out of bounds (size=%zu)\n", index, buf->size);
        return;
    }

    /* ensure we have room for one more pointer */
    ensureCapacity(buf, buf->size + 1);

    /* shift pointers to the right to make room (shift number = size - index) */
    if (index < buf->size) {
        memmove(&buf->lines[index + 1], &buf->lines[index],
                (buf->size - index) * sizeof(char *));
    }

    /* store exact-sized copy of text */
    if (!text) text = "";
    size_t len = strlen(text);
    char *copy = (char *) xmalloc(len + 1);
    memcpy(copy, text, len + 1);

    buf->lines[index] = copy;
    buf->size += 1;
}

/* Delete a line at index (0..size-1). Free the string and compact the array. */
void deleteLine(LineBuffer *buf, size_t index) {
    if (!buf) return;
    if (index >= buf->size) {
        fprintf(stderr, "deleteLine: index %zu out of bounds (size=%zu)\n", index, buf->size);
        return;
    }

    free(buf->lines[index]);             /* free the string memory */
    /* shift pointers left to fill the hole */
    if (index + 1 < buf->size) {
        memmove(&buf->lines[index], &buf->lines[index + 1],
                (buf->size - index - 1) * sizeof(char *));
    }
    buf->size -= 1;
    buf->lines[buf->size] = NULL; /* optional, keep invariant */
}

/* Replace line at index with new text (free old string and store exact new copy) */
void replaceLine(LineBuffer *buf, size_t index, const char *text) {
    if (!buf) return;
    if (index >= buf->size) {
        fprintf(stderr, "replaceLine: index %zu out of bounds (size=%zu)\n", index, buf->size);
        return;
    }
    free(buf->lines[index]);
    size_t len = strlen(text);
    char *copy = (char *) xmalloc(len + 1);
    memcpy(copy, text, len + 1);
    buf->lines[index] = copy;
}

/* Print all lines with 1-based numbers for readability */
void printAllLines(const LineBuffer *buf) {
    if (!buf) return;
    printf("---- Buffer: %zu line(s) ----\n", buf->size);
    for (size_t i = 0; i < buf->size; ++i) {
        printf("%4zu: %s\n", i + 1, buf->lines[i]);
    }
    printf("---- end ----\n");
}

/* Save buffer to file (one line per file line). Returns 0 on success, -1 on error. */
int saveToFile(const LineBuffer *buf, const char *filename) {
    if (!buf || !filename) return -1;
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("fopen for write failed");
        return -1;
    }
    for (size_t i = 0; i < buf->size; ++i) {
        if (fprintf(f, "%s\n", buf->lines[i]) < 0) {
            perror("fprintf failed");
            fclose(f);
            return -1;
        }
    }
    if (fclose(f) == EOF) {
        perror("fclose failed");
        return -1;
    }
    return 0;
}

/* Load buffer from file. This frees existing buffer contents and rebuilds from file.
 * Returns 0 on success, -1 on error.
 */
int loadFromFile(LineBuffer *buf, const char *filename) {
    if (!buf || !filename) return -1;
    FILE *f = fopen(filename, "r");
    if (!f) {
        perror("fopen for read failed");
        return -1;
    }

    /* free existing contents */
    for (size_t i = 0; i < buf->size; ++i) {
        free(buf->lines[i]);
        buf->lines[i] = NULL;
    }
    buf->size = 0;

    /* We'll read char-by-char building a dynamic buffer for each line */
    int c;
    char *line = NULL;
    size_t lineCap = 0;
    size_t pos = 0;

    while ((c = fgetc(f)) != EOF) {
        /* ensure buffer */
        if (pos + 2 > lineCap) {
            size_t newcap = lineCap ? lineCap * 2 : 128;
            line = (char *) xrealloc(line, newcap);
            lineCap = newcap;
        }
        if (c == '\n') {
            line[pos] = '\0';
            /* shrink to exact size then insert */
            line = (char *) xrealloc(line, pos + 1);
            insertLine(buf, buf->size, line); /* insert copies the string */
            free(line); /* insertLine allocated its own copy */
            line = NULL;
            lineCap = 0;
            pos = 0;
        } else {
            line[pos++] = (char) c;
        }
    }

    /* handle last line if file didn't end with newline */
    if (line && pos > 0) {
        line[pos] = '\0';
        line = (char *) xrealloc(line, pos + 1);
        insertLine(buf, buf->size, line);
        free(line);
    } else {
        free(line);
    }

    if (fclose(f) == EOF) {
        perror("fclose failed");
        return -1;
    }
    return 0;
}

/* ---- Simple interactive demo menu ----
 * The demo reads commands from stdin. The commands:
 *  i <index>   -> insert a new line at position index (1-based). After this command, user types the new line and presses Enter.
 *  a           -> append (insert at end). Then type line.
 *  d <index>   -> delete line at index (1-based)
 *  r <index>   -> replace line at index; then type new line
 *  p           -> print all lines
 *  s <file>    -> save to file
 *  l <file>    -> load from file (replaces buffer)
 *  f           -> shrinkToFit
 *  q           -> quit
 *
 * Example:
 *   a
 *   Hello world
 *   a
 *   Another line
 *   p
 *   s myfile.txt
 *   q
 */
void printHelp(void) {
    puts("Commands:");
    puts("  i <index>   - insert line at index (1-based), then type line text and press Enter");
    puts("  a           - append (add at end), then type line text and press Enter");
    puts("  d <index>   - delete line at index (1-based)");
    puts("  r <index>   - replace line at index (1-based), then type new text and press Enter");
    puts("  p           - print all lines");
    puts("  s <file>    - save to file");
    puts("  l <file>    - load from file (rebuilds buffer)");
    puts("  f           - shrinkToFit (reduce memory to fit exactly number of lines)");
    puts("  h           - help");
    puts("  q           - quit");
}

/* read a single token (word) from stdin (skips whitespace) */
int readToken(char *buf, size_t buflen) {
    int c;
    size_t p = 0;
    /* skip whitespace */
    do {
        c = getchar();
        if (c == EOF) return -1;
    } while (c == ' ' || c == '\t' || c == '\n' || c == '\r');

    while (c != EOF && c != ' ' && c != '\t' && c != '\n' && c != '\r') {
        if (p + 1 < buflen) buf[p++] = (char) c;
        c = getchar();
    }
    buf[p] = '\0';
    return 0;
}

int main(void) {
    LineBuffer buf;
    initBuffer(&buf, 4);

    printf("Minimal Line-Based Editor (demo). Type 'h' for help.\n");
    printHelp();

    for (;;) {
        printf("\n> ");
        fflush(stdout);

        int c = getchar();
        if (c == EOF) break;
        /* consume any extra whitespace until token or newline */
        while (c == ' ' || c == '\t' || c == '\r') c = getchar();
        if (c == '\n') continue;

        if (c == 'i') { /* insert */
            int index;
            if (scanf("%d", &index) != 1) {
                fprintf(stderr, "invalid index\n");
                /* flush rest of line */
                while ((c = getchar()) != EOF && c != '\n');
                continue;
            }
            /* remove trailing newline before reading text line */
            while ((c = getchar()) != EOF && c != '\n');
            printf("Enter line to insert at %d:\n", index);
            char *line = readLineSafe();
            if (!line) {
                printf("No line entered (EOF)\n");
                continue;
            }
            /* convert 1-based index to 0-based but clamp */
            size_t idx0 = (index > 0) ? (size_t)(index - 1) : 0;
            if (idx0 > buf.size) idx0 = buf.size;
            insertLine(&buf, idx0, line);
            free(line);
            printf("Inserted.\n");
        }
        else if (c == 'a') { /* append */
            /* consume rest of token until newline */
            while ((c = getchar()) != EOF && c != '\n');
            printf("Enter line to append:\n");
            char *line = readLineSafe();
            if (!line) { printf("No line entered (EOF)\n"); continue; }
            insertLine(&buf, buf.size, line);
            free(line);
            printf("Appended.\n");
        }
        else if (c == 'd') { /* delete */
            int index;
            if (scanf("%d", &index) != 1) {
                fprintf(stderr, "invalid index for delete\n");
                while ((c = getchar()) != EOF && c != '\n');
                continue;
            }
            while ((c = getchar()) != EOF && c != '\n');
            if (index < 1 || (size_t)index > buf.size) {
                fprintf(stderr, "delete: index out of range\n");
                continue;
            }
            deleteLine(&buf, (size_t)index - 1);
            printf("Deleted.\n");
        }
        else if (c == 'r') { /* replace */
            int index;
            if (scanf("%d", &index) != 1) {
                fprintf(stderr, "invalid index for replace\n");
                while ((c = getchar()) != EOF && c != '\n');
                continue;
            }
            while ((c = getchar()) != EOF && c != '\n');
            if (index < 1 || (size_t)index > buf.size) {
                fprintf(stderr, "replace: index out of range\n");
                continue;
            }
            printf("Enter new text for line %d:\n", index);
            char *line = readLineSafe();
            if (!line) { printf("No line entered (EOF)\n"); continue; }
            replaceLine(&buf, (size_t)index - 1, line);
            free(line);
            printf("Replaced.\n");
        }
        else if (c == 'p') { /* print */
            while ((c = getchar()) != EOF && c != '\n'); /* consume rest of line */
            printAllLines(&buf);
        }
        else if (c == 's') { /* save */
            char filename[512];
            if (readToken(filename, sizeof(filename)) == -1) break;
            while ((c = getchar()) != EOF && c != '\n');
            if (saveToFile(&buf, filename) == 0) {
                printf("Saved to %s\n", filename);
            } else {
                fprintf(stderr, "Failed to save to %s\n", filename);
            }
        }
        else if (c == 'l') { /* load */
            char filename[512];
            if (readToken(filename, sizeof(filename)) == -1) break;
            while ((c = getchar()) != EOF && c != '\n');
            if (loadFromFile(&buf, filename) == 0) {
                printf("Loaded from %s\n", filename);
            } else {
                fprintf(stderr, "Failed to load from %s\n", filename);
            }
        }
        else if (c == 'f') { /* shrinkToFit */
            while ((c = getchar()) != EOF && c != '\n');
            shrinkToFit(&buf);
            printf("Shrink-to-fit done. capacity == %zu\n", buf.capacity);
        }
        else if (c == 'h') {
            while ((c = getchar()) != EOF && c != '\n');
            printHelp();
        }
        else if (c == 'q') {
            while ((c = getchar()) != EOF && c != '\n');
            break;
        }
        else {
            /* unknown command: consume rest of line and prompt */
            while ((c = getchar()) != EOF && c != '\n');
            printf("Unknown command. Type 'h' for help.\n");
        }
    }

    freeAll(&buf);
    printf("Exiting editor. Memory freed.\n");
    return 0;
}
