#include "stackcode.h"
#include <ctype.h>

StackProgram *create_stack_program(void) {
    StackProgram *sp = (StackProgram *)calloc(1, sizeof(StackProgram));
    if (!sp) {
        fprintf(stderr, "Error: Out of memory in create_stack_program\n");
        exit(1);
    }
    sp->head = NULL;
    sp->tail = NULL;
    sp->count = 0;
    return sp;
}

void free_stack_program(StackProgram *sp) {
    if (!sp) return;
    StackInstr *cur = sp->head;
    while (cur) {
        StackInstr *next = cur->next;
        if (cur->arg) free(cur->arg);
        free(cur);
        cur = next;
    }
    free(sp);
}

static void append_stack_instr(StackProgram *sp, StackOp op, const char *arg, int line) {
    StackInstr *instr = (StackInstr *)calloc(1, sizeof(StackInstr));
    if (!instr) {
        fprintf(stderr, "Error: Out of memory in append_stack_instr\n");
        exit(1);
    }
    instr->op = op;
    instr->arg = arg ? strdup(arg) : NULL;
    instr->line = line;
    instr->next = NULL;

    if (sp->tail) {
        sp->tail->next = instr;
    } else {
        sp->head = instr;
    }
    sp->tail = instr;
    sp->count++;
}

static bool is_operand_literal(const char *arg) {
    if (!arg || *arg == '\0') return false;
    if (arg[0] == '"') return true;
    if (arg[0] == '-' || arg[0] == '+') {
        return isdigit((unsigned char)arg[1]);
    }
    return isdigit((unsigned char)arg[0]);
}

static void emit_operand_access(StackProgram *sp, const char *arg, int line) {
    if (!arg) return;
    if (is_operand_literal(arg)) {
        append_stack_instr(sp, STACK_PUSH, arg, line);
    } else {
        append_stack_instr(sp, STACK_LOAD, arg, line);
    }
}

StackProgram *generate_stack_code(TACList *tac) {
    if (!tac) return NULL;
    StackProgram *sp = create_stack_program();

    TACInstr *cur = tac->head;
    while (cur) {
        switch (cur->op) {
            case TAC_LABEL:
                append_stack_instr(sp, STACK_LABEL, cur->result, cur->line);
                break;

            case TAC_ASSIGN:
                emit_operand_access(sp, cur->arg1, cur->line);
                append_stack_instr(sp, STACK_STORE, cur->result, cur->line);
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
            case TAC_OR: {
                emit_operand_access(sp, cur->arg1, cur->line);
                emit_operand_access(sp, cur->arg2, cur->line);

                StackOp op = STACK_ADD;
                switch (cur->op) {
                    case TAC_ADD: op = STACK_ADD; break;
                    case TAC_SUB: op = STACK_SUB; break;
                    case TAC_MUL: op = STACK_MUL; break;
                    case TAC_DIV: op = STACK_DIV; break;
                    case TAC_MOD: op = STACK_MOD; break;
                    case TAC_EQ:  op = STACK_CMP_EQ; break;
                    case TAC_NEQ: op = STACK_CMP_NE; break;
                    case TAC_LT:  op = STACK_CMP_LT; break;
                    case TAC_LTE: op = STACK_CMP_LE; break;
                    case TAC_GT:  op = STACK_CMP_GT; break;
                    case TAC_GTE: op = STACK_CMP_GE; break;
                    case TAC_AND: op = STACK_AND; break;
                    case TAC_OR:  op = STACK_OR; break;
                    default: break;
                }
                append_stack_instr(sp, op, NULL, cur->line);
                append_stack_instr(sp, STACK_STORE, cur->result, cur->line);
                break;
            }

            case TAC_NEG:
                emit_operand_access(sp, cur->arg1, cur->line);
                append_stack_instr(sp, STACK_NEG, NULL, cur->line);
                append_stack_instr(sp, STACK_STORE, cur->result, cur->line);
                break;

            case TAC_NOT:
                emit_operand_access(sp, cur->arg1, cur->line);
                append_stack_instr(sp, STACK_NOT, NULL, cur->line);
                append_stack_instr(sp, STACK_STORE, cur->result, cur->line);
                break;

            case TAC_GOTO:
                append_stack_instr(sp, STACK_JMP, cur->result, cur->line);
                break;

            case TAC_IF_FALSE:
                emit_operand_access(sp, cur->arg1, cur->line);
                append_stack_instr(sp, STACK_JZ, cur->result, cur->line);
                break;

            case TAC_IF_TRUE:
                emit_operand_access(sp, cur->arg1, cur->line);
                append_stack_instr(sp, STACK_JNZ, cur->result, cur->line);
                break;

            case TAC_PRINT:
                emit_operand_access(sp, cur->arg1, cur->line);
                append_stack_instr(sp, STACK_PRINT, NULL, cur->line);
                break;

            case TAC_NOP:
                break;
        }
        cur = cur->next;
    }

    append_stack_instr(sp, STACK_HALT, NULL, 0);
    return sp;
}

const char *stack_op_to_string(StackOp op) {
    switch (op) {
        case STACK_PUSH:   return "PUSH";
        case STACK_LOAD:   return "LOAD";
        case STACK_STORE:  return "STORE";
        case STACK_ADD:    return "ADD";
        case STACK_SUB:    return "SUB";
        case STACK_MUL:    return "MUL";
        case STACK_DIV:    return "DIV";
        case STACK_MOD:    return "MOD";
        case STACK_NEG:    return "NEG";
        case STACK_CMP_EQ: return "CMP_EQ";
        case STACK_CMP_NE: return "CMP_NE";
        case STACK_CMP_LT: return "CMP_LT";
        case STACK_CMP_LE: return "CMP_LE";
        case STACK_CMP_GT: return "CMP_GT";
        case STACK_CMP_GE: return "CMP_GE";
        case STACK_AND:    return "AND";
        case STACK_OR:     return "OR";
        case STACK_NOT:    return "NOT";
        case STACK_LABEL:  return "LABEL";
        case STACK_JMP:    return "JMP";
        case STACK_JZ:     return "JZ";
        case STACK_JNZ:    return "JNZ";
        case STACK_PRINT:  return "PRINT";
        case STACK_HALT:   return "HALT";
        default:           return "UNKNOWN";
    }
}

void print_stack_code(StackProgram *sp) {
    if (!sp || !sp->head) {
        printf("(Stack program is empty)\n");
        return;
    }

    printf("\n=======================================================\n");
    printf("             STACK MACHINE CODE (BYTECODE)             \n");
    printf("=======================================================\n");

    StackInstr *cur = sp->head;
    int addr = 0;
    while (cur) {
        if (cur->op == STACK_LABEL) {
            printf("\033[1;33m%s:\033[0m\n", cur->arg);
        } else {
            printf("[%04d]   %-8s", addr++, stack_op_to_string(cur->op));
            if (cur->arg) {
                printf(" %s", cur->arg);
            }
            printf("\n");
        }
        cur = cur->next;
    }
    printf("=======================================================\n");
    printf("  Total Instructions: %d\n\n", sp->count);
}

/* --- Built-in Stack Virtual Machine (VM) Runtime --- */

typedef enum {
    VAL_INT,
    VAL_FLOAT,
    VAL_STR
} ValType;

typedef struct VMVal {
    ValType type;
    union {
        long i;
        double f;
        char *s;
    };
} VMVal;

typedef struct VMVar {
    char *name;
    VMVal val;
    struct VMVar *next;
} VMVar;

static VMVal make_int_val(long v) {
    VMVal val; val.type = VAL_INT; val.i = v; return val;
}
static VMVal make_float_val(double v) {
    VMVal val; val.type = VAL_FLOAT; val.f = v; return val;
}
static VMVal make_str_val(const char *s) {
    VMVal val; val.type = VAL_STR; val.s = s ? strdup(s) : strdup(""); return val;
}
static void free_vm_val(VMVal v) {
    if (v.type == VAL_STR && v.s) free(v.s);
}

static VMVal clone_vm_val(VMVal v) {
    if (v.type == VAL_STR) return make_str_val(v.s);
    return v;
}

static VMVar *find_var(VMVar *vars, const char *name) {
    VMVar *v = vars;
    while (v) {
        if (strcmp(v->name, name) == 0) return v;
        v = v->next;
    }
    return NULL;
}

static void set_var(VMVar **vars, const char *name, VMVal val) {
    VMVar *v = find_var(*vars, name);
    if (v) {
        free_vm_val(v->val);
        v->val = clone_vm_val(val);
    } else {
        VMVar *new_v = (VMVar *)malloc(sizeof(VMVar));
        new_v->name = strdup(name);
        new_v->val = clone_vm_val(val);
        new_v->next = *vars;
        *vars = new_v;
    }
}

static int find_label_pc(StackInstr **instructions, int count, const char *lbl) {
    for (int i = 0; i < count; i++) {
        if (instructions[i]->op == STACK_LABEL &&
            instructions[i]->arg && strcmp(instructions[i]->arg, lbl) == 0) {
            return i;
        }
    }
    return -1;
}

int execute_stack_program(StackProgram *sp) {
    if (!sp || !sp->head) return 0;

    /* Build instruction array for indexed execution */
    int instr_count = sp->count;
    StackInstr **instructions = (StackInstr **)malloc(instr_count * sizeof(StackInstr *));
    StackInstr *cur = sp->head;
    int n = 0;
    while (cur) {
        instructions[n++] = cur;
        cur = cur->next;
    }

    /* Stack */
    #define MAX_STACK 2048
    VMVal stack[MAX_STACK];
    int sp_top = -1;

    /* Variable environment */
    VMVar *vars = NULL;

    printf("\n=======================================================\n");
    printf("             STACK MACHINE EXECUTION (VM)              \n");
    printf("=======================================================\n");

    int pc = 0;
    int steps = 0;
    const int MAX_STEPS = 500000;

    while (pc >= 0 && pc < instr_count && steps < MAX_STEPS) {
        StackInstr *in = instructions[pc];
        steps++;

        switch (in->op) {
            case STACK_LABEL:
                /* No-op during execution */
                pc++;
                break;

            case STACK_PUSH: {
                if (sp_top >= MAX_STACK - 1) {
                    fprintf(stderr, "VM Error: Stack overflow\n");
                    goto cleanup;
                }
                const char *arg = in->arg;
                if (arg[0] == '"') {
                    /* Strip surrounding quotes */
                    size_t len = strlen(arg);
                    char *clean = (char *)malloc(len);
                    if (len >= 2) {
                        strncpy(clean, arg + 1, len - 2);
                        clean[len - 2] = '\0';
                    } else {
                        clean[0] = '\0';
                    }
                    stack[++sp_top] = make_str_val(clean);
                    free(clean);
                } else if (strchr(arg, '.')) {
                    stack[++sp_top] = make_float_val(atof(arg));
                } else {
                    stack[++sp_top] = make_int_val(strtol(arg, NULL, 10));
                }
                pc++;
                break;
            }

            case STACK_LOAD: {
                if (sp_top >= MAX_STACK - 1) {
                    fprintf(stderr, "VM Error: Stack overflow\n");
                    goto cleanup;
                }
                VMVar *v = find_var(vars, in->arg);
                if (!v) {
                    /* Default uninitialized var to 0 */
                    stack[++sp_top] = make_int_val(0);
                } else {
                    stack[++sp_top] = clone_vm_val(v->val);
                }
                pc++;
                break;
            }

            case STACK_STORE: {
                if (sp_top < 0) {
                    fprintf(stderr, "VM Error: Stack underflow in STORE\n");
                    goto cleanup;
                }
                VMVal v = stack[sp_top--];
                set_var(&vars, in->arg, v);
                free_vm_val(v);
                pc++;
                break;
            }

            case STACK_ADD:
            case STACK_SUB:
            case STACK_MUL:
            case STACK_DIV:
            case STACK_MOD:
            case STACK_CMP_EQ:
            case STACK_CMP_NE:
            case STACK_CMP_LT:
            case STACK_CMP_LE:
            case STACK_CMP_GT:
            case STACK_CMP_GE:
            case STACK_AND:
            case STACK_OR: {
                if (sp_top < 1) {
                    fprintf(stderr, "VM Error: Stack underflow in binary op\n");
                    goto cleanup;
                }
                VMVal r = stack[sp_top--];
                VMVal l = stack[sp_top--];

                bool is_float = (l.type == VAL_FLOAT || r.type == VAL_FLOAT);
                double l_num = (l.type == VAL_FLOAT) ? l.f : l.i;
                double r_num = (r.type == VAL_FLOAT) ? r.f : r.i;

                VMVal res;
                switch (in->op) {
                    case STACK_ADD:
                        if (l.type == VAL_STR && r.type == VAL_STR) {
                            char *cat = (char *)malloc(strlen(l.s) + strlen(r.s) + 1);
                            strcpy(cat, l.s);
                            strcat(cat, r.s);
                            res = make_str_val(cat);
                            free(cat);
                        } else if (is_float) {
                            res = make_float_val(l_num + r_num);
                        } else {
                            res = make_int_val((long)l_num + (long)r_num);
                        }
                        break;
                    case STACK_SUB:
                        res = is_float ? make_float_val(l_num - r_num) : make_int_val((long)l_num - (long)r_num);
                        break;
                    case STACK_MUL:
                        res = is_float ? make_float_val(l_num * r_num) : make_int_val((long)l_num * (long)r_num);
                        break;
                    case STACK_DIV:
                        if (r_num == 0) {
                            fprintf(stderr, "VM Runtime Error: Division by zero\n");
                            free_vm_val(l); free_vm_val(r);
                            goto cleanup;
                        }
                        res = is_float ? make_float_val(l_num / r_num) : make_int_val((long)l_num / (long)r_num);
                        break;
                    case STACK_MOD:
                        if ((long)r_num == 0) {
                            fprintf(stderr, "VM Runtime Error: Modulo by zero\n");
                            free_vm_val(l); free_vm_val(r);
                            goto cleanup;
                        }
                        res = make_int_val((long)l_num % (long)r_num);
                        break;
                    case STACK_CMP_EQ: res = make_int_val(l_num == r_num); break;
                    case STACK_CMP_NE: res = make_int_val(l_num != r_num); break;
                    case STACK_CMP_LT: res = make_int_val(l_num < r_num); break;
                    case STACK_CMP_LE: res = make_int_val(l_num <= r_num); break;
                    case STACK_CMP_GT: res = make_int_val(l_num > r_num); break;
                    case STACK_CMP_GE: res = make_int_val(l_num >= r_num); break;
                    case STACK_AND:    res = make_int_val(((long)l_num != 0) && ((long)r_num != 0)); break;
                    case STACK_OR:     res = make_int_val(((long)l_num != 0) || ((long)r_num != 0)); break;
                    default:           res = make_int_val(0); break;
                }
                free_vm_val(l);
                free_vm_val(r);
                stack[++sp_top] = res;
                pc++;
                break;
            }

            case STACK_NEG: {
                if (sp_top < 0) {
                    fprintf(stderr, "VM Error: Stack underflow in NEG\n");
                    goto cleanup;
                }
                if (stack[sp_top].type == VAL_FLOAT) {
                    stack[sp_top].f = -stack[sp_top].f;
                } else if (stack[sp_top].type == VAL_INT) {
                    stack[sp_top].i = -stack[sp_top].i;
                }
                pc++;
                break;
            }

            case STACK_NOT: {
                if (sp_top < 0) {
                    fprintf(stderr, "VM Error: Stack underflow in NOT\n");
                    goto cleanup;
                }
                long v = (stack[sp_top].type == VAL_FLOAT) ? (stack[sp_top].f != 0) : stack[sp_top].i;
                free_vm_val(stack[sp_top]);
                stack[sp_top] = make_int_val(!v);
                pc++;
                break;
            }

            case STACK_JMP: {
                int target = find_label_pc(instructions, instr_count, in->arg);
                if (target < 0) {
                    fprintf(stderr, "VM Error: Unknown jump label '%s'\n", in->arg);
                    goto cleanup;
                }
                pc = target;
                break;
            }

            case STACK_JZ: {
                if (sp_top < 0) {
                    fprintf(stderr, "VM Error: Stack underflow in JZ\n");
                    goto cleanup;
                }
                VMVal v = stack[sp_top--];
                bool is_zero = false;
                if (v.type == VAL_INT) is_zero = (v.i == 0);
                else if (v.type == VAL_FLOAT) is_zero = (v.f == 0.0);
                free_vm_val(v);

                if (is_zero) {
                    int target = find_label_pc(instructions, instr_count, in->arg);
                    if (target < 0) {
                        fprintf(stderr, "VM Error: Unknown jump label '%s'\n", in->arg);
                        goto cleanup;
                    }
                    pc = target;
                } else {
                    pc++;
                }
                break;
            }

            case STACK_JNZ: {
                if (sp_top < 0) {
                    fprintf(stderr, "VM Error: Stack underflow in JNZ\n");
                    goto cleanup;
                }
                VMVal v = stack[sp_top--];
                bool not_zero = false;
                if (v.type == VAL_INT) not_zero = (v.i != 0);
                else if (v.type == VAL_FLOAT) not_zero = (v.f != 0.0);
                free_vm_val(v);

                if (not_zero) {
                    int target = find_label_pc(instructions, instr_count, in->arg);
                    if (target < 0) {
                        fprintf(stderr, "VM Error: Unknown jump label '%s'\n", in->arg);
                        goto cleanup;
                    }
                    pc = target;
                } else {
                    pc++;
                }
                break;
            }

            case STACK_PRINT: {
                if (sp_top < 0) {
                    fprintf(stderr, "VM Error: Stack underflow in PRINT\n");
                    goto cleanup;
                }
                VMVal v = stack[sp_top--];
                printf("\033[1;32m[PROGRAM OUTPUT]\033[0m ");
                if (v.type == VAL_INT) {
                    printf("%ld\n", v.i);
                } else if (v.type == VAL_FLOAT) {
                    printf("%g\n", v.f);
                } else if (v.type == VAL_STR) {
                    printf("%s\n", v.s);
                }
                free_vm_val(v);
                pc++;
                break;
            }

            case STACK_HALT:
                goto normal_halt;
        }
    }

normal_halt:
    if (steps >= MAX_STEPS) {
        printf("\n\033[1;31m[VM Warning] Execution limit reached (possible infinite loop)\033[0m\n");
    } else {
        printf("\n\033[1;32m[VM Complete] Execution halted successfully in %d steps.\033[0m\n", steps);
    }

cleanup:
    while (sp_top >= 0) {
        free_vm_val(stack[sp_top--]);
    }
    while (vars) {
        VMVar *tmp = vars->next;
        if (vars->name) free(vars->name);
        free_vm_val(vars->val);
        free(vars);
        vars = tmp;
    }
    free(instructions);
    printf("=======================================================\n\n");
    return 0;
}
