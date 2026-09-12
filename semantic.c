#include "semantic.h"
#include <stdarg.h>

SemanticAnalyzer *create_semantic_analyzer(void) {
    SemanticAnalyzer *sa = (SemanticAnalyzer *)calloc(1, sizeof(SemanticAnalyzer));
    if (!sa) {
        fprintf(stderr, "Error: Out of memory in create_semantic_analyzer\n");
        exit(1);
    }
    sa->symbol_table = create_symbol_table();
    sa->errors = NULL;
    sa->errors_tail = NULL;
    sa->error_count = 0;
    sa->warning_count = 0;
    return sa;
}

void free_semantic_analyzer(SemanticAnalyzer *sa) {
    if (!sa) return;
    if (sa->symbol_table) {
        free_symbol_table(sa->symbol_table);
    }
    SemanticError *err = sa->errors;
    while (err) {
        SemanticError *tmp = err->next;
        if (err->message) free(err->message);
        free(err);
        err = tmp;
    }
    free(sa);
}

void add_semantic_error(SemanticAnalyzer *sa, int line, const char *fmt, ...) {
    if (!sa) return;
    char buffer[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    SemanticError *err = (SemanticError *)malloc(sizeof(SemanticError));
    err->line = line;
    err->message = strdup(buffer);
    err->next = NULL;

    if (!sa->errors) {
        sa->errors = err;
        sa->errors_tail = err;
    } else {
        sa->errors_tail->next = err;
        sa->errors_tail = err;
    }
    sa->error_count++;
}

static bool types_are_compatible(DataType target, DataType source) {
    if (target == source) return true;
    /* Allow implicit promotion from INT to FLOAT */
    if (target == DATA_TYPE_FLOAT && source == DATA_TYPE_INT) return true;
    /* Allow boolean and int in conditions / flags */
    if (target == DATA_TYPE_BOOL && source == DATA_TYPE_INT) return true;
    if (target == DATA_TYPE_INT && source == DATA_TYPE_BOOL) return true;
    return false;
}

static DataType analyze_expr(SemanticAnalyzer *sa, ASTNode *node) {
    if (!node) return DATA_TYPE_UNKNOWN;

    switch (node->type) {
        case NODE_LITERAL_INT:
            node->data_type = DATA_TYPE_INT;
            return DATA_TYPE_INT;

        case NODE_LITERAL_FLOAT:
            node->data_type = DATA_TYPE_FLOAT;
            return DATA_TYPE_FLOAT;

        case NODE_LITERAL_STRING:
            node->data_type = DATA_TYPE_STRING;
            return DATA_TYPE_STRING;

        case NODE_LITERAL_BOOL:
            node->data_type = DATA_TYPE_BOOL;
            return DATA_TYPE_BOOL;

        case NODE_IDENTIFIER: {
            Symbol *sym = lookup_all_scopes(sa->symbol_table, node->str_value);
            if (!sym) {
                add_semantic_error(sa, node->line,
                    "Undeclared identifier '%s': variable used without declaration in this scope",
                    node->str_value);
                node->data_type = DATA_TYPE_UNKNOWN;
                return DATA_TYPE_UNKNOWN;
            }
            node->data_type = sym->type;
            return sym->type;
        }

        case NODE_UNARY_OP: {
            DataType t = analyze_expr(sa, node->left);
            if (node->op == OP_NEG) {
                if (t != DATA_TYPE_INT && t != DATA_TYPE_FLOAT) {
                    add_semantic_error(sa, node->line,
                        "Type mismatch in unary minus: expected numeric operand, got '%s'",
                        datatype_to_string(t));
                    node->data_type = DATA_TYPE_UNKNOWN;
                    return DATA_TYPE_UNKNOWN;
                }
                node->data_type = t;
                return t;
            } else if (node->op == OP_NOT) {
                if (t != DATA_TYPE_BOOL && t != DATA_TYPE_INT) {
                    add_semantic_error(sa, node->line,
                        "Type mismatch in logical NOT: expected boolean or integer, got '%s'",
                        datatype_to_string(t));
                }
                node->data_type = DATA_TYPE_BOOL;
                return DATA_TYPE_BOOL;
            }
            node->data_type = t;
            return t;
        }

        case NODE_BINARY_OP: {
            DataType left_type = analyze_expr(sa, node->left);
            DataType right_type = analyze_expr(sa, node->right);

            if (left_type == DATA_TYPE_UNKNOWN || right_type == DATA_TYPE_UNKNOWN) {
                node->data_type = DATA_TYPE_UNKNOWN;
                return DATA_TYPE_UNKNOWN;
            }

            switch (node->op) {
                case OP_ADD:
                    /* String concatenation support */
                    if (left_type == DATA_TYPE_STRING && right_type == DATA_TYPE_STRING) {
                        node->data_type = DATA_TYPE_STRING;
                        return DATA_TYPE_STRING;
                    }
                    /* Arithmetic addition: fall through to common arithmetic check */
                    __attribute__((fallthrough));
                case OP_SUB:
                case OP_MUL:
                case OP_DIV:
                case OP_MOD:
                    if ((left_type != DATA_TYPE_INT && left_type != DATA_TYPE_FLOAT) ||
                        (right_type != DATA_TYPE_INT && right_type != DATA_TYPE_FLOAT)) {
                        add_semantic_error(sa, node->line,
                            "Invalid operands for arithmetic operator '%s': incompatible types '%s' and '%s'",
                            op_to_string(node->op),
                            datatype_to_string(left_type),
                            datatype_to_string(right_type));
                        node->data_type = DATA_TYPE_UNKNOWN;
                        return DATA_TYPE_UNKNOWN;
                    }
                    if (node->op == OP_MOD && (left_type != DATA_TYPE_INT || right_type != DATA_TYPE_INT)) {
                        add_semantic_error(sa, node->line,
                            "Operator '%%' is only defined for integer types, got '%s' and '%s'",
                            datatype_to_string(left_type), datatype_to_string(right_type));
                        node->data_type = DATA_TYPE_INT;
                        return DATA_TYPE_INT;
                    }
                    if (left_type == DATA_TYPE_FLOAT || right_type == DATA_TYPE_FLOAT) {
                        node->data_type = DATA_TYPE_FLOAT;
                        return DATA_TYPE_FLOAT;
                    }
                    node->data_type = DATA_TYPE_INT;
                    return DATA_TYPE_INT;

                case OP_EQ:
                case OP_NEQ:
                    if (!types_are_compatible(left_type, right_type) && !types_are_compatible(right_type, left_type)) {
                        add_semantic_error(sa, node->line,
                            "Comparison operator '%s' used on mismatched types '%s' and '%s'",
                            op_to_string(node->op),
                            datatype_to_string(left_type),
                            datatype_to_string(right_type));
                    }
                    node->data_type = DATA_TYPE_BOOL;
                    return DATA_TYPE_BOOL;

                case OP_LT:
                case OP_LTE:
                case OP_GT:
                case OP_GTE:
                    if ((left_type != DATA_TYPE_INT && left_type != DATA_TYPE_FLOAT) ||
                        (right_type != DATA_TYPE_INT && right_type != DATA_TYPE_FLOAT)) {
                        add_semantic_error(sa, node->line,
                            "Relational operator '%s' requires numeric types, got '%s' and '%s'",
                            op_to_string(node->op),
                            datatype_to_string(left_type),
                            datatype_to_string(right_type));
                    }
                    node->data_type = DATA_TYPE_BOOL;
                    return DATA_TYPE_BOOL;

                case OP_AND:
                case OP_OR:
                    if ((left_type != DATA_TYPE_BOOL && left_type != DATA_TYPE_INT) ||
                        (right_type != DATA_TYPE_BOOL && right_type != DATA_TYPE_INT)) {
                        add_semantic_error(sa, node->line,
                            "Logical operator '%s' requires boolean operands, got '%s' and '%s'",
                            op_to_string(node->op),
                            datatype_to_string(left_type),
                            datatype_to_string(right_type));
                    }
                    node->data_type = DATA_TYPE_BOOL;
                    return DATA_TYPE_BOOL;

                default:
                    node->data_type = DATA_TYPE_UNKNOWN;
                    return DATA_TYPE_UNKNOWN;
            }
        }

        default:
            return DATA_TYPE_UNKNOWN;
    }
}

static void analyze_stmt(SemanticAnalyzer *sa, ASTNode *node) {
    if (!node) return;

    switch (node->type) {
        case NODE_VAR_DECL: {
            Symbol *existing = lookup_current_scope(sa->symbol_table, node->str_value);
            if (existing) {
                add_semantic_error(sa, node->line,
                    "Redeclaration error: variable '%s' already declared in scope '%s' at line %d",
                    node->str_value, sa->symbol_table->current_scope->name, existing->line_declared);
            } else {
                Symbol *sym = insert_symbol(sa->symbol_table, node->str_value, node->data_type, node->line);
                if (node->init) {
                    DataType init_type = analyze_expr(sa, node->init);
                    if (init_type != DATA_TYPE_UNKNOWN && !types_are_compatible(node->data_type, init_type)) {
                        add_semantic_error(sa, node->line,
                            "Type mismatch in declaration of '%s': cannot initialize '%s' with expression of type '%s'",
                            node->str_value,
                            datatype_to_string(node->data_type),
                            datatype_to_string(init_type));
                    }
                    if (sym) sym->is_initialized = true;
                }
            }
            break;
        }

        case NODE_ASSIGN: {
            Symbol *sym = lookup_all_scopes(sa->symbol_table, node->str_value);
            if (!sym) {
                add_semantic_error(sa, node->line,
                    "Undeclared identifier in assignment: variable '%s' used without declaration",
                    node->str_value);
                analyze_expr(sa, node->right);
            } else {
                DataType val_type = analyze_expr(sa, node->right);
                if (val_type != DATA_TYPE_UNKNOWN && !types_are_compatible(sym->type, val_type)) {
                    add_semantic_error(sa, node->line,
                        "Type mismatch in assignment to '%s': cannot assign '%s' to variable of type '%s'",
                        node->str_value,
                        datatype_to_string(val_type),
                        datatype_to_string(sym->type));
                }
                sym->is_initialized = true;
            }
            break;
        }

        case NODE_IF: {
            DataType cond_type = analyze_expr(sa, node->cond);
            if (cond_type != DATA_TYPE_UNKNOWN && cond_type != DATA_TYPE_BOOL && cond_type != DATA_TYPE_INT) {
                add_semantic_error(sa, node->line,
                    "Type mismatch in 'if' condition: expected boolean or integer, got '%s'",
                    datatype_to_string(cond_type));
            }
            analyze_stmt(sa, node->body);
            if (node->else_body) {
                analyze_stmt(sa, node->else_body);
            }
            break;
        }

        case NODE_WHILE: {
            DataType cond_type = analyze_expr(sa, node->cond);
            if (cond_type != DATA_TYPE_UNKNOWN && cond_type != DATA_TYPE_BOOL && cond_type != DATA_TYPE_INT) {
                add_semantic_error(sa, node->line,
                    "Type mismatch in 'while' condition: expected boolean or integer, got '%s'",
                    datatype_to_string(cond_type));
            }
            analyze_stmt(sa, node->body);
            break;
        }

        case NODE_FOR: {
            enter_scope(sa->symbol_table, "for_scope");
            if (node->init) analyze_stmt(sa, node->init);
            if (node->cond) {
                DataType cond_type = analyze_expr(sa, node->cond);
                if (cond_type != DATA_TYPE_UNKNOWN && cond_type != DATA_TYPE_BOOL && cond_type != DATA_TYPE_INT) {
                    add_semantic_error(sa, node->line,
                        "Type mismatch in 'for' condition: expected boolean or integer, got '%s'",
                        datatype_to_string(cond_type));
                }
            }
            if (node->update) analyze_stmt(sa, node->update);
            analyze_stmt(sa, node->body);
            exit_scope(sa->symbol_table);
            break;
        }

        case NODE_BLOCK: {
            enter_scope(sa->symbol_table, "local_block");
            ASTNode *cur = node->body;
            while (cur) {
                analyze_stmt(sa, cur);
                cur = cur->next;
            }
            exit_scope(sa->symbol_table);
            break;
        }

        case NODE_PRINT: {
            analyze_expr(sa, node->left);
            break;
        }

        case NODE_PROGRAM: {
            ASTNode *cur = node->body;
            while (cur) {
                analyze_stmt(sa, cur);
                cur = cur->next;
            }
            break;
        }

        case NODE_BINARY_OP:
        case NODE_UNARY_OP:
        case NODE_LITERAL_INT:
        case NODE_LITERAL_FLOAT:
        case NODE_LITERAL_STRING:
        case NODE_LITERAL_BOOL:
        case NODE_IDENTIFIER:
            analyze_expr(sa, node);
            break;

        case NODE_EMPTY:
            break;
    }
}

int analyze_semantics(SemanticAnalyzer *sa, ASTNode *root) {
    if (!sa || !root) return 0;
    analyze_stmt(sa, root);
    return sa->error_count;
}

void print_semantic_errors(SemanticAnalyzer *sa) {
    if (!sa) return;
    printf("\n=======================================================\n");
    printf("                  SEMANTIC ANALYSIS                    \n");
    printf("=======================================================\n");
    if (sa->error_count == 0) {
        printf("\033[1;32m[PASS] Semantic analysis passed successfully!\033[0m\n");
        printf("       Zero semantic errors detected.\n");
    } else {
        printf("\033[1;31m[FAIL] Semantic analysis found %d error(s):\033[0m\n\n", sa->error_count);
        SemanticError *err = sa->errors;
        int idx = 1;
        while (err) {
            printf("  \033[1;31m[%d]\033[0m \033[1mLine %-3d:\033[0m %s\n", idx++, err->line, err->message);
            err = err->next;
        }
    }
    printf("=======================================================\n\n");
}
