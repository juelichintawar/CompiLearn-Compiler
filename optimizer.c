#include "optimizer.h"

static bool is_integer_constant(const char *str) {
    if (!str || *str == '\0') return false;
    if (*str == '-' || *str == '+') str++;
    if (*str == '\0') return false;
    while (*str) {
        if (!isdigit((unsigned char)*str)) return false;
        str++;
    }
    return true;
}

static bool is_float_constant(const char *str) {
    if (!str || *str == '\0') return false;
    if (*str == '-' || *str == '+') str++;
    bool has_dot = false;
    bool has_digit = false;
    while (*str) {
        if (*str == '.') {
            if (has_dot) return false;
            has_dot = true;
        } else if (isdigit((unsigned char)*str)) {
            has_digit = true;
        } else {
            return false;
        }
        str++;
    }
    return has_dot && has_digit;
}

static bool is_constant(const char *str) {
    if (!str) return false;
    if (is_integer_constant(str)) return true;
    if (is_float_constant(str)) return true;
    if (str[0] == '"' && str[strlen(str) - 1] == '"') return true;
    return false;
}

static bool is_temporary(const char *str) {
    if (!str || str[0] != 't') return false;
    for (int i = 1; str[i]; i++) {
        if (!isdigit((unsigned char)str[i])) return false;
    }
    return true;
}

/* 1. Constant Folding Pass */
static int pass_constant_folding(TACList *list) {
    int folded = 0;
    TACInstr *cur = list->head;

    while (cur) {
        if (cur->arg1 && cur->arg2 && is_constant(cur->arg1) && is_constant(cur->arg2)) {
            bool is_float = is_float_constant(cur->arg1) || is_float_constant(cur->arg2);
            char res_buf[64];
            bool can_fold = false;

            if (is_float) {
                double a = atof(cur->arg1);
                double b = atof(cur->arg2);
                double r = 0;
                switch (cur->op) {
                    case TAC_ADD: r = a + b; can_fold = true; break;
                    case TAC_SUB: r = a - b; can_fold = true; break;
                    case TAC_MUL: r = a * b; can_fold = true; break;
                    case TAC_DIV: if (b != 0) { r = a / b; can_fold = true; } break;
                    case TAC_EQ:  snprintf(res_buf, sizeof(res_buf), "%d", a == b); can_fold = true; break;
                    case TAC_NEQ: snprintf(res_buf, sizeof(res_buf), "%d", a != b); can_fold = true; break;
                    case TAC_LT:  snprintf(res_buf, sizeof(res_buf), "%d", a < b);  can_fold = true; break;
                    case TAC_LTE: snprintf(res_buf, sizeof(res_buf), "%d", a <= b); can_fold = true; break;
                    case TAC_GT:  snprintf(res_buf, sizeof(res_buf), "%d", a > b);  can_fold = true; break;
                    case TAC_GTE: snprintf(res_buf, sizeof(res_buf), "%d", a >= b); can_fold = true; break;
                    default: break;
                }
                if (can_fold && cur->op != TAC_EQ && cur->op != TAC_NEQ && cur->op != TAC_LT &&
                    cur->op != TAC_LTE && cur->op != TAC_GT && cur->op != TAC_GTE) {
                    snprintf(res_buf, sizeof(res_buf), "%g", r);
                }
            } else if (is_integer_constant(cur->arg1) && is_integer_constant(cur->arg2)) {
                long a = strtol(cur->arg1, NULL, 10);
                long b = strtol(cur->arg2, NULL, 10);
                long r = 0;
                switch (cur->op) {
                    case TAC_ADD: r = a + b; can_fold = true; break;
                    case TAC_SUB: r = a - b; can_fold = true; break;
                    case TAC_MUL: r = a * b; can_fold = true; break;
                    case TAC_DIV: if (b != 0) { r = a / b; can_fold = true; } break;
                    case TAC_MOD: if (b != 0) { r = a % b; can_fold = true; } break;
                    case TAC_EQ:  r = (a == b); can_fold = true; break;
                    case TAC_NEQ: r = (a != b); can_fold = true; break;
                    case TAC_LT:  r = (a < b);  can_fold = true; break;
                    case TAC_LTE: r = (a <= b); can_fold = true; break;
                    case TAC_GT:  r = (a > b);  can_fold = true; break;
                    case TAC_GTE: r = (a >= b); can_fold = true; break;
                    case TAC_AND: r = (a && b); can_fold = true; break;
                    case TAC_OR:  r = (a || b); can_fold = true; break;
                    default: break;
                }
                if (can_fold) {
                    snprintf(res_buf, sizeof(res_buf), "%ld", r);
                }
            }

            if (can_fold) {
                cur->op = TAC_ASSIGN;
                free(cur->arg1);
                cur->arg1 = strdup(res_buf);
                if (cur->arg2) {
                    free(cur->arg2);
                    cur->arg2 = NULL;
                }
                folded++;
            }
        } else if (cur->arg1 && !cur->arg2 && is_constant(cur->arg1)) {
            /* Unary folding */
            char res_buf[64];
            bool can_fold = false;
            if (cur->op == TAC_NEG) {
                if (is_integer_constant(cur->arg1)) {
                    long a = strtol(cur->arg1, NULL, 10);
                    snprintf(res_buf, sizeof(res_buf), "%ld", -a);
                    can_fold = true;
                } else if (is_float_constant(cur->arg1)) {
                    double a = atof(cur->arg1);
                    snprintf(res_buf, sizeof(res_buf), "%g", -a);
                    can_fold = true;
                }
            } else if (cur->op == TAC_NOT && is_integer_constant(cur->arg1)) {
                long a = strtol(cur->arg1, NULL, 10);
                snprintf(res_buf, sizeof(res_buf), "%d", !a);
                can_fold = true;
            }

            if (can_fold) {
                cur->op = TAC_ASSIGN;
                free(cur->arg1);
                cur->arg1 = strdup(res_buf);
                folded++;
            }
        }
        cur = cur->next;
    }
    return folded;
}

/* 2. Constant & Copy Propagation Pass within Basic Blocks */
static int pass_propagation(TACList *list, int *const_props, int *copy_props) {
    int changes = 0;
    TACInstr *block_start = list->head;

    while (block_start) {
        /* Find block boundary: end at label or jump */
        TACInstr *block_end = block_start;
        while (block_end && block_end->op != TAC_LABEL &&
               block_end->op != TAC_GOTO && block_end->op != TAC_IF_FALSE &&
               block_end->op != TAC_IF_TRUE) {
            block_end = block_end->next;
        }

        /* Forward propagation inside [block_start, block_end) */
        TACInstr *def = block_start;
        while (def && def != block_end) {
            if (def->op == TAC_ASSIGN && def->result && def->arg1) {
                const char *var = def->result;
                const char *val = def->arg1;
                bool is_const_val = is_constant(val);

                /* Look at subsequent instructions in the block */
                TACInstr *use = def->next;
                while (use && use != block_end) {
                    /* If val is modified, stop copy propagation for this val */
                    if (!is_const_val && use->result && strcmp(use->result, val) == 0) {
                        break;
                    }

                    /* Replace in arg1 */
                    if (use->arg1 && strcmp(use->arg1, var) == 0) {
                        free(use->arg1);
                        use->arg1 = strdup(val);
                        changes++;
                        if (is_const_val) (*const_props)++;
                        else (*copy_props)++;
                    }
                    /* Replace in arg2 */
                    if (use->arg2 && strcmp(use->arg2, var) == 0) {
                        free(use->arg2);
                        use->arg2 = strdup(val);
                        changes++;
                        if (is_const_val) (*const_props)++;
                        else (*copy_props)++;
                    }

                    /* If var itself is redefined, stop propagating this definition */
                    if (use->result && strcmp(use->result, var) == 0) {
                        break;
                    }

                    use = use->next;
                }
            }
            def = def->next;
        }

        block_start = block_end ? block_end->next : NULL;
    }

    return changes;
}

/* 3. Dead Code Elimination Pass */
static int pass_dead_code_elimination(TACList *list) {
    int eliminated = 0;

    /* A. Eliminate unreachable code after unconditional jump */
    TACInstr *cur = list->head;
    while (cur) {
        if (cur->op == TAC_GOTO) {
            TACInstr *unreachable = cur->next;
            while (unreachable && unreachable->op != TAC_LABEL) {
                TACInstr *to_del = unreachable;
                unreachable = unreachable->next;
                tac_remove_instr(list, to_del);
                eliminated++;
            }
        }
        cur = cur->next;
    }

    /* B. Eliminate unused temporary variables (t1, t2...) */
    cur = list->head;
    while (cur) {
        TACInstr *next_instr = cur->next;
        if (cur->result && is_temporary(cur->result) &&
            (cur->op == TAC_ASSIGN || cur->op == TAC_ADD || cur->op == TAC_SUB ||
             cur->op == TAC_MUL || cur->op == TAC_DIV || cur->op == TAC_MOD ||
             cur->op == TAC_NEG || cur->op == TAC_NOT || cur->op == TAC_EQ ||
             cur->op == TAC_NEQ || cur->op == TAC_LT || cur->op == TAC_LTE ||
             cur->op == TAC_GT || cur->op == TAC_GTE || cur->op == TAC_AND ||
             cur->op == TAC_OR)) {

            /* Check if cur->result is used anywhere in the TAC list */
            bool used = false;
            TACInstr *scan = list->head;
            while (scan) {
                if (scan != cur) {
                    if ((scan->arg1 && strcmp(scan->arg1, cur->result) == 0) ||
                        (scan->arg2 && strcmp(scan->arg2, cur->result) == 0)) {
                        used = true;
                        break;
                    }
                }
                scan = scan->next;
            }

            if (!used) {
                tac_remove_instr(list, cur);
                eliminated++;
            }
        }
        cur = next_instr;
    }

    return eliminated;
}

OptimizationStats optimize_tac(TACList *list) {
    OptimizationStats stats = {0, 0, 0, 0, 0};
    if (!list || !list->head) return stats;

    bool progress = true;
    while (progress && stats.total_passes < 15) {
        progress = false;
        stats.total_passes++;

        int folds = pass_constant_folding(list);
        if (folds > 0) {
            stats.constant_folds += folds;
            progress = true;
        }

        int c_prop = 0, cp_prop = 0;
        int props = pass_propagation(list, &c_prop, &cp_prop);
        if (props > 0) {
            stats.constant_propagations += c_prop;
            stats.copy_propagations += cp_prop;
            progress = true;
        }

        int dce = pass_dead_code_elimination(list);
        if (dce > 0) {
            stats.dead_code_eliminations += dce;
            progress = true;
        }
    }

    return stats;
}

void print_optimization_report(TACList *before, TACList *after, OptimizationStats stats) {
    printf("\n=======================================================\n");
    printf("                TAC OPTIMIZATION REPORT                \n");
    printf("=======================================================\n");
    printf("  Optimization Passes Run : %d\n", stats.total_passes);
    printf("  Constant Folds          : %d\n", stats.constant_folds);
    printf("  Constant Propagations   : %d\n", stats.constant_propagations);
    printf("  Copy Propagations       : %d\n", stats.copy_propagations);
    printf("  Dead Code Eliminations  : %d\n", stats.dead_code_eliminations);
    printf("  -----------------------------------------------------\n");
    printf("  Instructions Before     : %d\n", before ? before->count : 0);
    printf("  Instructions After      : %d\n", after ? after->count : 0);
    if (before && before->count > 0) {
        int reduction = before->count - (after ? after->count : 0);
        printf("  Instruction Reduction   : %d (%.1f%% reduction)\n",
               reduction, (double)reduction / before->count * 100.0);
    }
    printf("=======================================================\n\n");

    printf("--- [OPTIMIZED CODE LISTING] ---\n");
    print_tac(after);
}
