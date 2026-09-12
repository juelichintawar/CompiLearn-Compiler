#ifndef STACKCODE_H
#define STACKCODE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "tac.h"

/* Stack Machine OpCodes */
typedef enum {
    STACK_PUSH,       /* PUSH <val> */
    STACK_LOAD,       /* LOAD <var> */
    STACK_STORE,      /* STORE <var> */
    STACK_ADD,        /* ADD */
    STACK_SUB,        /* SUB */
    STACK_MUL,        /* MUL */
    STACK_DIV,        /* DIV */
    STACK_MOD,        /* MOD */
    STACK_NEG,        /* NEG */
    STACK_CMP_EQ,     /* CMP_EQ */
    STACK_CMP_NE,     /* CMP_NE */
    STACK_CMP_LT,     /* CMP_LT */
    STACK_CMP_LE,     /* CMP_LE */
    STACK_CMP_GT,     /* CMP_GT */
    STACK_CMP_GE,     /* CMP_GE */
    STACK_AND,        /* AND */
    STACK_OR,         /* OR */
    STACK_NOT,        /* NOT */
    STACK_LABEL,      /* LABEL <name> */
    STACK_JMP,        /* JMP <label> */
    STACK_JZ,         /* JZ <label> */
    STACK_JNZ,        /* JNZ <label> */
    STACK_PRINT,      /* PRINT */
    STACK_HALT        /* HALT */
} StackOp;

typedef struct StackInstr {
    StackOp op;
    char *arg;        /* Variable name, literal, or label */
    int line;
    struct StackInstr *next;
} StackInstr;

typedef struct StackProgram {
    StackInstr *head;
    StackInstr *tail;
    int count;
} StackProgram;

StackProgram *create_stack_program(void);
void free_stack_program(StackProgram *sp);

StackProgram *generate_stack_code(TACList *tac);
void print_stack_code(StackProgram *sp);
int execute_stack_program(StackProgram *sp);

#endif /* STACKCODE_H */
