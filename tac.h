#ifndef TAC_H
#define TAC_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "ast.h"

/* Three Address Code Operations */
typedef enum {
    TAC_LABEL,          /* LABEL L1: */
    TAC_ASSIGN,         /* res = arg1 */
    TAC_ADD,            /* res = arg1 + arg2 */
    TAC_SUB,            /* res = arg1 - arg2 */
    TAC_MUL,            /* res = arg1 * arg2 */
    TAC_DIV,            /* res = arg1 / arg2 */
    TAC_MOD,            /* res = arg1 % arg2 */
    TAC_NEG,            /* res = -arg1 */
    TAC_EQ,             /* res = arg1 == arg2 */
    TAC_NEQ,            /* res = arg1 != arg2 */
    TAC_LT,             /* res = arg1 < arg2 */
    TAC_LTE,            /* res = arg1 <= arg2 */
    TAC_GT,             /* res = arg1 > arg2 */
    TAC_GTE,            /* res = arg1 >= arg2 */
    TAC_AND,            /* res = arg1 && arg2 */
    TAC_OR,             /* res = arg1 || arg2 */
    TAC_NOT,            /* res = !arg1 */
    TAC_GOTO,           /* goto res */
    TAC_IF_FALSE,       /* if_false arg1 goto res */
    TAC_IF_TRUE,        /* if arg1 goto res */
    TAC_PRINT,          /* print arg1 */
    TAC_NOP
} TACOp;

/* Single TAC Quadruple / Instruction */
typedef struct TACInstr {
    TACOp op;
    char *result;
    char *arg1;
    char *arg2;
    int line;
    struct TACInstr *prev;
    struct TACInstr *next;
} TACInstr;

/* Doubly linked list of TAC Instructions */
typedef struct TACList {
    TACInstr *head;
    TACInstr *tail;
    int count;
    int temp_counter;
    int label_counter;
} TACList;

TACList *create_tac_list(void);
void free_tac_list(TACList *list);

char *tac_new_temp(TACList *list);
char *tac_new_label(TACList *list);

TACInstr *tac_append(TACList *list, TACOp op, const char *res, const char *arg1, const char *arg2, int line);
void tac_remove_instr(TACList *list, TACInstr *instr);

TACList *generate_tac(ASTNode *root);
void print_tac(TACList *list);

const char *tac_op_to_string(TACOp op);
TACList *clone_tac_list(TACList *src);

#endif /* TAC_H */
