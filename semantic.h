#ifndef SEMANTIC_H
#define SEMANTIC_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "ast.h"
#include "symboltable.h"

typedef struct SemanticError {
    int line;
    char *message;
    struct SemanticError *next;
} SemanticError;

typedef struct SemanticAnalyzer {
    SymbolTable *symbol_table;
    SemanticError *errors;
    SemanticError *errors_tail;
    int error_count;
    int warning_count;
} SemanticAnalyzer;

SemanticAnalyzer *create_semantic_analyzer(void);
void free_semantic_analyzer(SemanticAnalyzer *sa);

void add_semantic_error(SemanticAnalyzer *sa, int line, const char *fmt, ...);
int analyze_semantics(SemanticAnalyzer *sa, ASTNode *root);
void print_semantic_errors(SemanticAnalyzer *sa);

#endif /* SEMANTIC_H */
