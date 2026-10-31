
#ifndef COMMON_H
#define COMMON_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX 200

typedef struct {
    char op[32], a[64], b[64], r[64];
} Quad;

static Quad q[MAX];
static int nq = 0, tempNo = 0;
static int tempTripleIndex[200];

static char *dupstr(const char *s) {
    char *p = malloc(strlen(s) + 1);
    if (!p) exit(1);
    strcpy(p, s);
    return p;
}

static char *newtemp(void) {
    char s[32];
    snprintf(s, sizeof(s), "t%d", ++tempNo);
    return dupstr(s);
}

static void emit(const char *op, const char *a,
                 const char *b, const char *r) {
    if (nq >= MAX) {
        fprintf(stderr, "Too many instructions\n");
        exit(1);
    }

    int i = nq++;
    snprintf(q[i].op, sizeof(q[i].op), "%s", op);
    snprintf(q[i].a, sizeof(q[i].a), "%s", a ? a : "");
    snprintf(q[i].b, sizeof(q[i].b), "%s", b ? b : "");
    snprintf(q[i].r, sizeof(q[i].r), "%s", r ? r : "");

    int t;
    if (r && sscanf(r, "t%d", &t) == 1 &&
        t > 0 && t < 200)
        tempTripleIndex[t] = i;
}

static char *trref(const char *s) {
    char out[64];
    int t;
    if (s && sscanf(s, "t%d", &t) == 1 &&
        t > 0 && t < 200) {
        snprintf(out, sizeof(out), "(%d)", tempTripleIndex[t]);
        return dupstr(out);
    }
    return dupstr(s && *s ? s : "-");
}

static void print_all(void) {
    int i;

    printf("\nTHREE-ADDRESS CODE (TAC)\n");
    for (i = 0; i < nq; i++) {
        if (!strcmp(q[i].op, "label"))
            printf("%s:\n", q[i].r);
        else if (!strcmp(q[i].op, "goto"))
            printf("goto %s\n", q[i].r);
        else if (!strncmp(q[i].op, "if ", 3))
            printf("if %s %s %s goto %s\n",
                   q[i].a, q[i].op + 3, q[i].b, q[i].r);
        else if (!strcmp(q[i].op, "="))
            printf("%s = %s\n", q[i].r, q[i].a);
        else
            printf("%s = %s %s %s\n",
                   q[i].r, q[i].a, q[i].op, q[i].b);
    }

    printf("\nQUADRUPLES\n");
    printf("+-----+------------+------------+------------+------------+\n");
    printf("| No. | Op         | Arg1       | Arg2       | Result     |\n");
    printf("+-----+------------+------------+------------+------------+\n");
    for (i = 0; i < nq; i++)
        printf("| %-3d | %-10s | %-10s | %-10s | %-10s |\n",
               i, q[i].op, *q[i].a ? q[i].a : "-",
               *q[i].b ? q[i].b : "-",
               *q[i].r ? q[i].r : "-");
    printf("+-----+------------+------------+------------+------------+\n");

    printf("\nTRIPLES\n");
    printf("+-----+----------------+------------+------------+\n");
    printf("| No. | Op             | Arg1       | Arg2       |\n");
    printf("+-----+----------------+------------+------------+\n");

    for (i = 0; i < nq; i++) {
        char op[64];
        char *a, *b;

        if (!strcmp(q[i].op, "label"))
            snprintf(op, sizeof(op), "%s:", q[i].r);
        else if (!strcmp(q[i].op, "goto"))
            snprintf(op, sizeof(op), "goto %s", q[i].r);
        else if (!strncmp(q[i].op, "if ", 3))
            snprintf(op, sizeof(op), "%s goto %s",
                     q[i].op, q[i].r);
        else
            snprintf(op, sizeof(op), "%s", q[i].op);

        if (!strcmp(q[i].op, "=")) {
            a = trref(q[i].a);
            b = trref(q[i].r);
        } else {
            a = trref(q[i].a);
            b = trref(q[i].b);
        }

        printf("| %-3d | %-14s | %-10s | %-10s |\n",
               i, op, a, b);
        free(a);
        free(b);
    }

    printf("+-----+----------------+------------+------------+\n");
}
#endif
