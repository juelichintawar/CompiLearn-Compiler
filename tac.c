#include "tac.h"

TACList *create_tac_list(void) {
    TACList *list = (TACList *)calloc(1, sizeof(TACList));
    if (!list) {
        fprintf(stderr, "Error: Out of memory in create_tac_list\n");
        exit(1);
    }
    list->head = NULL;
    list->tail = NULL;
    list->count = 0;
    list->temp_counter = 0;
    list->label_counter = 0;
    return list;
}

void free_tac_list(TACList *list) {
    if (!list) return;
    TACInstr *cur = list->head;
    while (cur) {
        TACInstr *next = cur->next;
        if (cur->result) free(cur->result);
        if (cur->arg1) free(cur->arg1);
        if (cur->arg2) free(cur->arg2);
        free(cur);
        cur = next;
    }
    free(list);
}

char *tac_new_temp(TACList *list) {
    list->temp_counter++;
    char buf[32];
    snprintf(buf, sizeof(buf), "t%d", list->temp_counter);
    return strdup(buf);
}

char *tac_new_label(TACList *list) {
    list->label_counter++;
    char buf[32];
    snprintf(buf, sizeof(buf), "L%d", list->label_counter);
    return strdup(buf);
}

TACInstr *tac_append(TACList *list, TACOp op, const char *res, const char *arg1, const char *arg2, int line) {
    TACInstr *instr = (TACInstr *)calloc(1, sizeof(TACInstr));
    if (!instr) {
        fprintf(stderr, "Error: Out of memory in tac_append\n");
        exit(1);
    }
    instr->op = op;
    instr->result = res ? strdup(res) : NULL;
    instr->arg1 = arg1 ? strdup(arg1) : NULL;
    instr->arg2 = arg2 ? strdup(arg2) : NULL;
    instr->line = line;
    instr->prev = list->tail;
    instr->next = NULL;

    if (list->tail) {
        list->tail->next = instr;
    } else {
        list->head = instr;
    }
    list->tail = instr;
    list->count++;

    return instr;
}

void tac_remove_instr(TACList *list, TACInstr *instr) {
    if (!list || !instr) return;

    if (instr->prev) {
        instr->prev->next = instr->next;
    } else {
        list->head = instr->next;
    }

    if (instr->next) {
        instr->next->prev = instr->prev;
    } else {
        list->tail = instr->prev;
    }

    if (instr->result) free(instr->result);
    if (instr->arg1) free(instr->arg1);
    if (instr->arg2) free(instr->arg2);
    free(instr);
    list->count--;
}

TACList *clone_tac_list(TACList *src) {
    if (!src) return NULL;
    TACList *dst = create_tac_list();
    dst->temp_counter = src->temp_counter;
    dst->label_counter = src->label_counter;

    TACInstr *cur = src->head;
    while (cur) {
        tac_append(dst, cur->op, cur->result, cur->arg1, cur->arg2, cur->line);
        cur = cur->next;
    }
    return dst;
}

const char *tac_op_to_string(TACOp op) {
    switch (op) {
        case TAC_ADD: return "+";
        case TAC_SUB: return "-";
        case TAC_MUL: return "*";
        case TAC_DIV: return "/";
        case TAC_MOD: return "%";
        case TAC_NEG: return "-";
        case TAC_EQ:  return "==";
        case TAC_NEQ: return "!=";
        case TAC_LT:  return "<";
        case TAC_LTE: return "<=";
        case TAC_GT:  return ">";
        case TAC_GTE: return ">=";
        case TAC_AND: return "&&";
        case TAC_OR:  return "||";
        case TAC_NOT: return "!";
        default:      return "";
    }
}

static char *gen_expr_tac(TACList *list, ASTNode *node) {
    if (!node) return NULL;

    switch (node->type) {
        case NODE_LITERAL_INT: {
            char buf[32];
            snprintf(buf, sizeof(buf), "%d", node->int_value);
            return strdup(buf);
        }
        case NODE_LITERAL_FLOAT: {
            char buf[32];
            snprintf(buf, sizeof(buf), "%g", node->float_value);
            return strdup(buf);
        }
        case NODE_LITERAL_STRING: {
            char buf[512];
            snprintf(buf, sizeof(buf), "\"%s\"", node->str_value ? node->str_value : "");
            return strdup(buf);
        }
        case NODE_LITERAL_BOOL: {
            return strdup(node->int_value ? "1" : "0");
        }
        case NODE_IDENTIFIER: {
            return strdup(node->str_value ? node->str_value : "id");
        }
        case NODE_UNARY_OP: {
            char *arg1 = gen_expr_tac(list, node->left);
            char *res = tac_new_temp(list);
            TACOp op = (node->op == OP_NOT) ? TAC_NOT : TAC_NEG;
            tac_append(list, op, res, arg1, NULL, node->line);
            free(arg1);
            return res;
        }
        case NODE_BINARY_OP: {
            char *arg1 = gen_expr_tac(list, node->left);
            char *arg2 = gen_expr_tac(list, node->right);
            char *res = tac_new_temp(list);

            TACOp op = TAC_ADD;
            switch (node->op) {
                case OP_ADD: op = TAC_ADD; break;
                case OP_SUB: op = TAC_SUB; break;
                case OP_MUL: op = TAC_MUL; break;
                case OP_DIV: op = TAC_DIV; break;
                case OP_MOD: op = TAC_MOD; break;
                case OP_EQ:  op = TAC_EQ;  break;
                case OP_NEQ: op = TAC_NEQ; break;
                case OP_LT:  op = TAC_LT;  break;
                case OP_LTE: op = TAC_LTE; break;
                case OP_GT:  op = TAC_GT;  break;
                case OP_GTE: op = TAC_GTE; break;
                case OP_AND: op = TAC_AND; break;
                case OP_OR:  op = TAC_OR;  break;
                default:     op = TAC_ADD; break;
            }

            tac_append(list, op, res, arg1, arg2, node->line);
            free(arg1);
            free(arg2);
            return res;
        }
        default:
            return NULL;
    }
}

static void gen_stmt_tac(TACList *list, ASTNode *node) {
    if (!node) return;

    switch (node->type) {
        case NODE_PROGRAM:
        case NODE_BLOCK: {
            ASTNode *cur = node->body;
            while (cur) {
                gen_stmt_tac(list, cur);
                cur = cur->next;
            }
            break;
        }
        case NODE_VAR_DECL: {
            if (node->init) {
                char *val = gen_expr_tac(list, node->init);
                tac_append(list, TAC_ASSIGN, node->str_value, val, NULL, node->line);
                free(val);
            }
            break;
        }
        case NODE_ASSIGN: {
            char *val = gen_expr_tac(list, node->right);
            tac_append(list, TAC_ASSIGN, node->str_value, val, NULL, node->line);
            free(val);
            break;
        }
        case NODE_PRINT: {
            char *val = gen_expr_tac(list, node->left);
            tac_append(list, TAC_PRINT, NULL, val, NULL, node->line);
            free(val);
            break;
        }
        case NODE_IF: {
            char *cond = gen_expr_tac(list, node->cond);
            char *l_else = tac_new_label(list);
            char *l_end = tac_new_label(list);

            if (node->else_body) {
                tac_append(list, TAC_IF_FALSE, l_else, cond, NULL, node->line);
                gen_stmt_tac(list, node->body);
                tac_append(list, TAC_GOTO, l_end, NULL, NULL, node->line);
                tac_append(list, TAC_LABEL, l_else, NULL, NULL, node->line);
                gen_stmt_tac(list, node->else_body);
                tac_append(list, TAC_LABEL, l_end, NULL, NULL, node->line);
            } else {
                tac_append(list, TAC_IF_FALSE, l_end, cond, NULL, node->line);
                gen_stmt_tac(list, node->body);
                tac_append(list, TAC_LABEL, l_end, NULL, NULL, node->line);
            }
            free(cond);
            free(l_else);
            free(l_end);
            break;
        }
        case NODE_WHILE: {
            char *l_start = tac_new_label(list);
            char *l_end = tac_new_label(list);

            tac_append(list, TAC_LABEL, l_start, NULL, NULL, node->line);
            char *cond = gen_expr_tac(list, node->cond);
            tac_append(list, TAC_IF_FALSE, l_end, cond, NULL, node->line);
            gen_stmt_tac(list, node->body);
            tac_append(list, TAC_GOTO, l_start, NULL, NULL, node->line);
            tac_append(list, TAC_LABEL, l_end, NULL, NULL, node->line);

            free(cond);
            free(l_start);
            free(l_end);
            break;
        }
        case NODE_FOR: {
            if (node->init) {
                gen_stmt_tac(list, node->init);
            }
            char *l_start = tac_new_label(list);
            char *l_end = tac_new_label(list);

            tac_append(list, TAC_LABEL, l_start, NULL, NULL, node->line);
            if (node->cond) {
                char *cond = gen_expr_tac(list, node->cond);
                tac_append(list, TAC_IF_FALSE, l_end, cond, NULL, node->line);
                free(cond);
            }
            gen_stmt_tac(list, node->body);
            if (node->update) {
                gen_stmt_tac(list, node->update);
            }
            tac_append(list, TAC_GOTO, l_start, NULL, NULL, node->line);
            tac_append(list, TAC_LABEL, l_end, NULL, NULL, node->line);

            free(l_start);
            free(l_end);
            break;
        }
        default:
            break;
    }
}

TACList *generate_tac(ASTNode *root) {
    if (!root) return NULL;
    TACList *list = create_tac_list();
    gen_stmt_tac(list, root);
    return list;
}

void print_tac(TACList *list) {
    if (!list || !list->head) {
        printf("(TAC list is empty)\n");
        return;
    }

    printf("\n=======================================================\n");
    printf("              THREE ADDRESS CODE (TAC)                 \n");
    printf("=======================================================\n");

    TACInstr *cur = list->head;
    int idx = 1;
    while (cur) {
        printf("%03d: ", idx++);
        switch (cur->op) {
            case TAC_LABEL:
                printf("\033[1;33m%s:\033[0m\n", cur->result);
                break;
            case TAC_ASSIGN:
                printf("    %-6s = %s\n", cur->result, cur->arg1);
                break;
            case TAC_ADD:
            case TAC_SUB:
            case TAC_MUL:
            case TAC_DIV:
            case TAC_MOD:
            case TAC_EQ:
            case TAC_NEQ:
            case TAC_LT:
            case TAC_LTE:
            case TAC_GT:
            case TAC_GTE:
            case TAC_AND:
            case TAC_OR:
                printf("    %-6s = %s %s %s\n", cur->result, cur->arg1, tac_op_to_string(cur->op), cur->arg2);
                break;
            case TAC_NEG:
                printf("    %-6s = -%s\n", cur->result, cur->arg1);
                break;
            case TAC_NOT:
                printf("    %-6s = !%s\n", cur->result, cur->arg1);
                break;
            case TAC_GOTO:
                printf("    \033[1;35mgoto %s\033[0m\n", cur->result);
                break;
            case TAC_IF_FALSE:
                printf("    \033[1;35mif_false %s goto %s\033[0m\n", cur->arg1, cur->result);
                break;
            case TAC_IF_TRUE:
                printf("    \033[1;35mif %s goto %s\033[0m\n", cur->arg1, cur->result);
                break;
            case TAC_PRINT:
                printf("    \033[1;36mprint %s\033[0m\n", cur->arg1);
                break;
            case TAC_NOP:
                printf("    nop\n");
                break;
        }
        cur = cur->next;
    }
    printf("=======================================================\n");
    printf("  Total Instructions: %d\n\n", list->count);
}
