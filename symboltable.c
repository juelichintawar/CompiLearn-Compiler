#include "symboltable.h"

static Scope *create_scope(int level, const char *name, Scope *parent) {
    Scope *scope = (Scope *)calloc(1, sizeof(Scope));
    if (!scope) {
        fprintf(stderr, "Error: Out of memory in create_scope\n");
        exit(1);
    }
    scope->level = level;
    scope->name = name ? strdup(name) : strdup("anonymous");
    scope->parent = parent;
    scope->symbols = NULL;
    scope->children = NULL;
    scope->child_count = 0;
    scope->child_capacity = 0;
    return scope;
}

SymbolTable *create_symbol_table(void) {
    SymbolTable *st = (SymbolTable *)calloc(1, sizeof(SymbolTable));
    if (!st) {
        fprintf(stderr, "Error: Out of memory in create_symbol_table\n");
        exit(1);
    }
    st->root_scope = create_scope(0, "global", NULL);
    st->current_scope = st->root_scope;
    st->total_symbols = 0;
    st->next_mem_address = 0x1000;
    st->scope_counter = 0;
    return st;
}

Scope *enter_scope(SymbolTable *st, const char *prefix) {
    if (!st) return NULL;
    st->scope_counter++;
    char scope_name[64];
    snprintf(scope_name, sizeof(scope_name), "%s_%d", prefix ? prefix : "scope", st->scope_counter);

    Scope *new_scope = create_scope(st->current_scope->level + 1, scope_name, st->current_scope);

    /* Add to parent's children list */
    Scope *parent = st->current_scope;
    if (parent->child_count >= parent->child_capacity) {
        parent->child_capacity = (parent->child_capacity == 0) ? 4 : parent->child_capacity * 2;
        parent->children = (Scope **)realloc(parent->children, parent->child_capacity * sizeof(Scope *));
    }
    parent->children[parent->child_count++] = new_scope;

    st->current_scope = new_scope;
    return new_scope;
}

Scope *exit_scope(SymbolTable *st) {
    if (!st || !st->current_scope) return NULL;
    if (st->current_scope->parent) {
        st->current_scope = st->current_scope->parent;
    }
    return st->current_scope;
}

Symbol *insert_symbol(SymbolTable *st, const char *name, DataType type, int line) {
    if (!st || !st->current_scope || !name) return NULL;

    Symbol *sym = (Symbol *)calloc(1, sizeof(Symbol));
    if (!sym) {
        fprintf(stderr, "Error: Out of memory in insert_symbol\n");
        exit(1);
    }
    sym->name = strdup(name);
    sym->type = type;
    sym->scope_level = st->current_scope->level;
    sym->scope_name = strdup(st->current_scope->name);
    sym->line_declared = line;
    sym->mem_address = st->next_mem_address;
    st->next_mem_address += 4; /* 4 bytes per variable */
    sym->is_initialized = false;

    /* Append to scope symbols list */
    sym->next = st->current_scope->symbols;
    st->current_scope->symbols = sym;
    st->total_symbols++;

    return sym;
}

Symbol *lookup_current_scope(SymbolTable *st, const char *name) {
    if (!st || !st->current_scope || !name) return NULL;
    Symbol *cur = st->current_scope->symbols;
    while (cur) {
        if (strcmp(cur->name, name) == 0) {
            return cur;
        }
        cur = cur->next;
    }
    return NULL;
}

Symbol *lookup_all_scopes(SymbolTable *st, const char *name) {
    if (!st || !st->current_scope || !name) return NULL;
    Scope *s = st->current_scope;
    while (s) {
        Symbol *cur = s->symbols;
        while (cur) {
            if (strcmp(cur->name, name) == 0) {
                return cur;
            }
            cur = cur->next;
        }
        s = s->parent;
    }
    return NULL;
}

static void print_scope_symbols(Scope *scope) {
    if (!scope) return;

    /* Reverse order to display in declaration order */
    int count = 0;
    Symbol *cur = scope->symbols;
    while (cur) {
        count++;
        cur = cur->next;
    }

    if (count > 0) {
        Symbol **list = (Symbol **)malloc(count * sizeof(Symbol *));
        cur = scope->symbols;
        for (int i = count - 1; i >= 0; i--) {
            list[i] = cur;
            cur = cur->next;
        }
        for (int i = 0; i < count; i++) {
            printf("| %-18s | %-10s | %-12s (lvl %d) | 0x%04X   | %-6d |\n",
                   list[i]->name,
                   datatype_to_string(list[i]->type),
                   list[i]->scope_name,
                   list[i]->scope_level,
                   list[i]->mem_address,
                   list[i]->line_declared);
        }
        free(list);
    }

    /* Print child scopes */
    for (int i = 0; i < scope->child_count; i++) {
        print_scope_symbols(scope->children[i]);
    }
}

void print_symbol_table(SymbolTable *st) {
    if (!st || !st->root_scope) {
        printf("(Symbol table is empty)\n");
        return;
    }
    printf("\n+--------------------+------------+----------------------+----------+--------+\n");
    printf("| %-18s | %-10s | %-20s | %-8s | %-6s |\n", "Variable Name", "Type", "Scope", "Address", "Line");
    printf("+--------------------+------------+----------------------+----------+--------+\n");
    if (st->total_symbols == 0) {
        printf("| %-70s |\n", "                 (No symbols defined in table)                        ");
    } else {
        print_scope_symbols(st->root_scope);
    }
    printf("+--------------------+------------+----------------------+----------+--------+\n");
    printf("  Total Symbols: %d\n\n", st->total_symbols);
}

static void free_scope(Scope *scope) {
    if (!scope) return;
    Symbol *sym = scope->symbols;
    while (sym) {
        Symbol *tmp = sym->next;
        if (sym->name) free(sym->name);
        if (sym->scope_name) free(sym->scope_name);
        free(sym);
        sym = tmp;
    }
    for (int i = 0; i < scope->child_count; i++) {
        free_scope(scope->children[i]);
    }
    if (scope->children) free(scope->children);
    if (scope->name) free(scope->name);
    free(scope);
}

void free_symbol_table(SymbolTable *st) {
    if (!st) return;
    if (st->root_scope) {
        free_scope(st->root_scope);
    }
    free(st);
}
