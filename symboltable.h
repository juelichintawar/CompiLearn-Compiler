#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "ast.h"

/* Symbol Table Entry */
typedef struct Symbol {
    char *name;
    DataType type;
    int scope_level;
    char *scope_name;
    int line_declared;
    int mem_address;          /* e.g., 0x1000 + 4 * offset */
    bool is_initialized;
    struct Symbol *next;      /* In current scope's symbol list */
} Symbol;

/* Scoped Environment */
typedef struct Scope {
    int level;
    char *name;
    Symbol *symbols;
    struct Scope *parent;
    struct Scope **children;
    int child_count;
    int child_capacity;
} Scope;

/* Overall Symbol Table Manager */
typedef struct SymbolTable {
    Scope *root_scope;
    Scope *current_scope;
    int total_symbols;
    int next_mem_address;
    int scope_counter;
} SymbolTable;

SymbolTable *create_symbol_table(void);
void free_symbol_table(SymbolTable *st);

Scope *enter_scope(SymbolTable *st, const char *prefix);
Scope *exit_scope(SymbolTable *st);

Symbol *insert_symbol(SymbolTable *st, const char *name, DataType type, int line);
Symbol *lookup_current_scope(SymbolTable *st, const char *name);
Symbol *lookup_all_scopes(SymbolTable *st, const char *name);

void print_symbol_table(SymbolTable *st);

#endif /* SYMBOLTABLE_H */
