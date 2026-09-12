#include "ast.h"

/* --- Parse Tree Implementation --- */

ParseTreeNode *pt_create_node(const char *name, const char *attribute, int line) {
    ParseTreeNode *node = (ParseTreeNode *)malloc(sizeof(ParseTreeNode));
    if (!node) {
        fprintf(stderr, "Error: Out of memory in pt_create_node\n");
        exit(1);
    }
    node->name = name ? strdup(name) : NULL;
    node->attribute = attribute ? strdup(attribute) : NULL;
    node->children = NULL;
    node->child_count = 0;
    node->child_capacity = 0;
    node->line = line;
    return node;
}

void pt_add_child(ParseTreeNode *parent, ParseTreeNode *child) {
    if (!parent || !child) return;
    if (parent->child_count >= parent->child_capacity) {
        parent->child_capacity = (parent->child_capacity == 0) ? 4 : parent->child_capacity * 2;
        parent->children = (ParseTreeNode **)realloc(parent->children, parent->child_capacity * sizeof(ParseTreeNode *));
        if (!parent->children) {
            fprintf(stderr, "Error: Out of memory in pt_add_child\n");
            exit(1);
        }
    }
    parent->children[parent->child_count++] = child;
}

static void print_pt_recursive(ParseTreeNode *node, const char *prefix, bool is_last) {
    if (!node) return;

    printf("%s", prefix);
    printf("%s", is_last ? "└── " : "├── ");

    if (node->attribute && strlen(node->attribute) > 0) {
        printf("\033[1;36m<%s>\033[0m: \033[1;33m\"%s\"\033[0m (line %d)\n",
               node->name, node->attribute, node->line);
    } else {
        printf("\033[1;32m<%s>\033[0m (line %d)\n", node->name, node->line);
    }

    char new_prefix[512];
    snprintf(new_prefix, sizeof(new_prefix), "%s%s", prefix, is_last ? "    " : "│   ");

    for (int i = 0; i < node->child_count; i++) {
        print_pt_recursive(node->children[i], new_prefix, i == (node->child_count - 1));
    }
}

void print_parse_tree(ParseTreeNode *root) {
    if (!root) {
        printf("(Parse tree is empty)\n");
        return;
    }
    printf("\n=======================================================\n");
    printf("              CONCRETE PARSE TREE (CST)                \n");
    printf("=======================================================\n");
    print_pt_recursive(root, "", true);
    printf("=======================================================\n\n");
}

void free_parse_tree(ParseTreeNode *root) {
    if (!root) return;
    for (int i = 0; i < root->child_count; i++) {
        free_parse_tree(root->children[i]);
    }
    if (root->children) free(root->children);
    if (root->name) free(root->name);
    if (root->attribute) free(root->attribute);
    free(root);
}

/* --- AST Implementation --- */

ASTNode *ast_create_node(ASTNodeType type, int line) {
    ASTNode *node = (ASTNode *)calloc(1, sizeof(ASTNode));
    if (!node) {
        fprintf(stderr, "Error: Out of memory in ast_create_node\n");
        exit(1);
    }
    node->type = type;
    node->data_type = DATA_TYPE_UNKNOWN;
    node->line = line;
    node->op = OP_NONE;
    return node;
}

ASTNode *ast_create_var_decl(DataType type, const char *name, ASTNode *init_expr, int line) {
    ASTNode *node = ast_create_node(NODE_VAR_DECL, line);
    node->data_type = type;
    node->str_value = name ? strdup(name) : NULL;
    node->init = init_expr;
    return node;
}

ASTNode *ast_create_assign(const char *name, ASTNode *expr, int line) {
    ASTNode *node = ast_create_node(NODE_ASSIGN, line);
    node->str_value = name ? strdup(name) : NULL;
    node->right = expr;
    return node;
}

ASTNode *ast_create_binary_op(OperatorType op, ASTNode *left, ASTNode *right, int line) {
    ASTNode *node = ast_create_node(NODE_BINARY_OP, line);
    node->op = op;
    node->left = left;
    node->right = right;
    return node;
}

ASTNode *ast_create_unary_op(OperatorType op, ASTNode *operand, int line) {
    ASTNode *node = ast_create_node(NODE_UNARY_OP, line);
    node->op = op;
    node->left = operand;
    return node;
}

ASTNode *ast_create_int(int val, int line) {
    ASTNode *node = ast_create_node(NODE_LITERAL_INT, line);
    node->data_type = DATA_TYPE_INT;
    node->int_value = val;
    return node;
}

ASTNode *ast_create_float(double val, int line) {
    ASTNode *node = ast_create_node(NODE_LITERAL_FLOAT, line);
    node->data_type = DATA_TYPE_FLOAT;
    node->float_value = val;
    return node;
}

ASTNode *ast_create_string(const char *val, int line) {
    ASTNode *node = ast_create_node(NODE_LITERAL_STRING, line);
    node->data_type = DATA_TYPE_STRING;
    node->str_value = val ? strdup(val) : NULL;
    return node;
}

ASTNode *ast_create_bool(int val, int line) {
    ASTNode *node = ast_create_node(NODE_LITERAL_BOOL, line);
    node->data_type = DATA_TYPE_BOOL;
    node->int_value = val ? 1 : 0;
    return node;
}

ASTNode *ast_create_identifier(const char *name, int line) {
    ASTNode *node = ast_create_node(NODE_IDENTIFIER, line);
    node->str_value = name ? strdup(name) : NULL;
    return node;
}

ASTNode *ast_create_if(ASTNode *cond, ASTNode *then_branch, ASTNode *else_branch, int line) {
    ASTNode *node = ast_create_node(NODE_IF, line);
    node->cond = cond;
    node->body = then_branch;
    node->else_body = else_branch;
    return node;
}

ASTNode *ast_create_while(ASTNode *cond, ASTNode *body, int line) {
    ASTNode *node = ast_create_node(NODE_WHILE, line);
    node->cond = cond;
    node->body = body;
    return node;
}

ASTNode *ast_create_for(ASTNode *init, ASTNode *cond, ASTNode *update, ASTNode *body, int line) {
    ASTNode *node = ast_create_node(NODE_FOR, line);
    node->init = init;
    node->cond = cond;
    node->update = update;
    node->body = body;
    return node;
}

ASTNode *ast_create_block(ASTNode *stmts, int line) {
    ASTNode *node = ast_create_node(NODE_BLOCK, line);
    node->body = stmts;
    return node;
}

ASTNode *ast_create_print(ASTNode *expr, int line) {
    ASTNode *node = ast_create_node(NODE_PRINT, line);
    node->left = expr;
    return node;
}

ASTNode *ast_append_stmt(ASTNode *list, ASTNode *stmt) {
    if (!stmt) return list;
    if (!list) return stmt;
    ASTNode *cur = list;
    while (cur->next) {
        cur = cur->next;
    }
    cur->next = stmt;
    return list;
}

const char *datatype_to_string(DataType type) {
    switch (type) {
        case DATA_TYPE_INT:    return "int";
        case DATA_TYPE_FLOAT:  return "float";
        case DATA_TYPE_STRING: return "string";
        case DATA_TYPE_BOOL:   return "bool";
        case DATA_TYPE_VOID:   return "void";
        default:               return "unknown";
    }
}

DataType string_to_datatype(const char *str) {
    if (!str) return DATA_TYPE_UNKNOWN;
    if (strcmp(str, "int") == 0) return DATA_TYPE_INT;
    if (strcmp(str, "float") == 0) return DATA_TYPE_FLOAT;
    if (strcmp(str, "string") == 0) return DATA_TYPE_STRING;
    if (strcmp(str, "bool") == 0) return DATA_TYPE_BOOL;
    return DATA_TYPE_UNKNOWN;
}

const char *op_to_string(OperatorType op) {
    switch (op) {
        case OP_ADD: return "+";
        case OP_SUB: return "-";
        case OP_MUL: return "*";
        case OP_DIV: return "/";
        case OP_MOD: return "%";
        case OP_EQ:  return "==";
        case OP_NEQ: return "!=";
        case OP_LT:  return "<";
        case OP_LTE: return "<=";
        case OP_GT:  return ">";
        case OP_GTE: return ">=";
        case OP_AND: return "&&";
        case OP_OR:  return "||";
        case OP_NOT: return "!";
        case OP_NEG: return "unary -";
        default:     return "?";
    }
}

const char *ast_node_type_to_string(ASTNodeType type) {
    switch (type) {
        case NODE_PROGRAM:        return "Program";
        case NODE_BLOCK:          return "Block";
        case NODE_VAR_DECL:       return "VarDecl";
        case NODE_ASSIGN:         return "Assign";
        case NODE_BINARY_OP:      return "BinaryOp";
        case NODE_UNARY_OP:       return "UnaryOp";
        case NODE_LITERAL_INT:    return "IntLiteral";
        case NODE_LITERAL_FLOAT:  return "FloatLiteral";
        case NODE_LITERAL_STRING: return "StringLiteral";
        case NODE_LITERAL_BOOL:   return "BoolLiteral";
        case NODE_IDENTIFIER:     return "Identifier";
        case NODE_IF:             return "If";
        case NODE_WHILE:          return "While";
        case NODE_FOR:            return "For";
        case NODE_PRINT:          return "Print";
        case NODE_EMPTY:          return "Empty";
        default:                  return "UnknownNode";
    }
}

static void print_ast_recursive(ASTNode *node, const char *prefix, bool is_last, const char *role) {
    if (!node) return;

    printf("%s%s", prefix, is_last ? "└── " : "├── ");

    if (role && strlen(role) > 0) {
        printf("\033[0;35m[%s] \033[0m", role);
    }

    switch (node->type) {
        case NODE_PROGRAM:
            printf("\033[1;32mProgram\033[0m\n");
            break;
        case NODE_BLOCK:
            printf("\033[1;34mBlock\033[0m (line %d)\n", node->line);
            break;
        case NODE_VAR_DECL:
            printf("\033[1;33mVarDecl\033[0m: \033[1m%s %s\033[0m (line %d)\n",
                   datatype_to_string(node->data_type), node->str_value, node->line);
            break;
        case NODE_ASSIGN:
            printf("\033[1;33mAssign\033[0m: \033[1m%s =\033[0m (line %d)\n",
                   node->str_value, node->line);
            break;
        case NODE_BINARY_OP:
            printf("\033[1;36mBinaryOp (%s)\033[0m (line %d, type: %s)\n",
                   op_to_string(node->op), node->line, datatype_to_string(node->data_type));
            break;
        case NODE_UNARY_OP:
            printf("\033[1;36mUnaryOp (%s)\033[0m (line %d, type: %s)\n",
                   op_to_string(node->op), node->line, datatype_to_string(node->data_type));
            break;
        case NODE_LITERAL_INT:
            printf("\033[1;32mIntLiteral\033[0m: %d (line %d)\n", node->int_value, node->line);
            break;
        case NODE_LITERAL_FLOAT:
            printf("\033[1;32mFloatLiteral\033[0m: %g (line %d)\n", node->float_value, node->line);
            break;
        case NODE_LITERAL_STRING:
            printf("\033[1;32mStringLiteral\033[0m: \"%s\" (line %d)\n", node->str_value, node->line);
            break;
        case NODE_LITERAL_BOOL:
            printf("\033[1;32mBoolLiteral\033[0m: %s (line %d)\n",
                   node->int_value ? "true" : "false", node->line);
            break;
        case NODE_IDENTIFIER:
            printf("\033[1;37mIdentifier\033[0m: %s (line %d, type: %s)\n",
                   node->str_value, node->line, datatype_to_string(node->data_type));
            break;
        case NODE_IF:
            printf("\033[1;35mIfStatement\033[0m (line %d)\n", node->line);
            break;
        case NODE_WHILE:
            printf("\033[1;35mWhileLoop\033[0m (line %d)\n", node->line);
            break;
        case NODE_FOR:
            printf("\033[1;35mForLoop\033[0m (line %d)\n", node->line);
            break;
        case NODE_PRINT:
            printf("\033[1;35mPrint\033[0m (line %d)\n", node->line);
            break;
        default:
            printf("Node (%d, line %d)\n", node->type, node->line);
            break;
    }

    char new_prefix[512];
    snprintf(new_prefix, sizeof(new_prefix), "%s%s", prefix, is_last ? "    " : "│   ");

    /* Collect all child subtrees to print sequentially */
    struct ChildEntry {
        ASTNode *node;
        const char *role;
    } children[8];
    int child_cnt = 0;

    if (node->type == NODE_VAR_DECL && node->init) {
        children[child_cnt++] = (struct ChildEntry){node->init, "init"};
    } else if (node->type == NODE_ASSIGN && node->right) {
        children[child_cnt++] = (struct ChildEntry){node->right, "value"};
    } else if (node->type == NODE_BINARY_OP) {
        if (node->left) children[child_cnt++] = (struct ChildEntry){node->left, "left"};
        if (node->right) children[child_cnt++] = (struct ChildEntry){node->right, "right"};
    } else if (node->type == NODE_UNARY_OP) {
        if (node->left) children[child_cnt++] = (struct ChildEntry){node->left, "operand"};
    } else if (node->type == NODE_IF) {
        if (node->cond) children[child_cnt++] = (struct ChildEntry){node->cond, "cond"};
        if (node->body) children[child_cnt++] = (struct ChildEntry){node->body, "then"};
        if (node->else_body) children[child_cnt++] = (struct ChildEntry){node->else_body, "else"};
    } else if (node->type == NODE_WHILE) {
        if (node->cond) children[child_cnt++] = (struct ChildEntry){node->cond, "cond"};
        if (node->body) children[child_cnt++] = (struct ChildEntry){node->body, "body"};
    } else if (node->type == NODE_FOR) {
        if (node->init) children[child_cnt++] = (struct ChildEntry){node->init, "init"};
        if (node->cond) children[child_cnt++] = (struct ChildEntry){node->cond, "cond"};
        if (node->update) children[child_cnt++] = (struct ChildEntry){node->update, "update"};
        if (node->body) children[child_cnt++] = (struct ChildEntry){node->body, "body"};
    } else if (node->type == NODE_PRINT) {
        if (node->left) children[child_cnt++] = (struct ChildEntry){node->left, "expr"};
    } else if (node->type == NODE_PROGRAM || node->type == NODE_BLOCK) {
        ASTNode *stmt = node->body;
        while (stmt) {
            bool has_more = (stmt->next != NULL);
            print_ast_recursive(stmt, new_prefix, !has_more, "stmt");
            stmt = stmt->next;
        }
        return;
    }

    for (int i = 0; i < child_cnt; i++) {
        print_ast_recursive(children[i].node, new_prefix, i == (child_cnt - 1), children[i].role);
    }
}

void print_ast(ASTNode *root) {
    if (!root) {
        printf("(AST is empty)\n");
        return;
    }
    printf("\n=======================================================\n");
    printf("              ABSTRACT SYNTAX TREE (AST)               \n");
    printf("=======================================================\n");
    print_ast_recursive(root, "", true, NULL);
    printf("=======================================================\n\n");
}

void free_ast(ASTNode *root) {
    if (!root) return;

    if (root->str_value) free(root->str_value);

    free_ast(root->left);
    free_ast(root->right);
    free_ast(root->cond);
    free_ast(root->body);
    free_ast(root->else_body);
    free_ast(root->init);
    free_ast(root->update);
    free_ast(root->next);

    free(root);
}

/* --- Token Stream Implementation --- */

TokenStream *create_token_stream(void) {
    TokenStream *ts = (TokenStream *)calloc(1, sizeof(TokenStream));
    if (!ts) {
        fprintf(stderr, "Error: Out of memory in create_token_stream\n");
        exit(1);
    }
    return ts;
}

void free_token_stream(TokenStream *ts) {
    if (!ts) return;
    TokenInfo *cur = ts->head;
    while (cur) {
        TokenInfo *tmp = cur->next;
        if (cur->type_name) free(cur->type_name);
        if (cur->lexeme) free(cur->lexeme);
        free(cur);
        cur = tmp;
    }
    free(ts);
}

void token_stream_add(TokenStream *ts, int type, const char *type_name, const char *lexeme, int line, int col) {
    if (!ts) return;
    TokenInfo *tok = (TokenInfo *)calloc(1, sizeof(TokenInfo));
    if (!tok) {
        fprintf(stderr, "Error: Out of memory in token_stream_add\n");
        exit(1);
    }
    tok->token_type = type;
    tok->type_name = type_name ? strdup(type_name) : strdup("UNKNOWN");
    tok->lexeme = lexeme ? strdup(lexeme) : strdup("");
    tok->line = line;
    tok->column = col;
    tok->next = NULL;

    if (ts->tail) {
        ts->tail->next = tok;
    } else {
        ts->head = tok;
    }
    ts->tail = tok;
    ts->count++;
}

void print_token_stream(TokenStream *ts) {
    if (!ts || !ts->head) {
        printf("(Token stream is empty)\n");
        return;
    }

    printf("\n+------+--------+----------------------+--------------------------------+\n");
    printf("| %-4s | %-6s | %-20s | %-30s |\n", "Line", "Column", "Token Type", "Lexeme");
    printf("+------+--------+----------------------+--------------------------------+\n");

    TokenInfo *cur = ts->head;
    while (cur) {
        char display_lexeme[31];
        strncpy(display_lexeme, cur->lexeme, 30);
        display_lexeme[30] = '\0';

        printf("| %4d | %6d | %-20s | %-30s |\n",
               cur->line, cur->column, cur->type_name, display_lexeme);
        cur = cur->next;
    }
    printf("+------+--------+----------------------+--------------------------------+\n");
    printf("  Total Tokens: %d\n\n", ts->count);
}
