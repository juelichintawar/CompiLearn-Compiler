#ifndef AST_H
#define AST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/* Data Types supported by the CompiLearn language */
typedef enum {
    DATA_TYPE_UNKNOWN,
    DATA_TYPE_INT,
    DATA_TYPE_FLOAT,
    DATA_TYPE_STRING,
    DATA_TYPE_BOOL,
    DATA_TYPE_VOID
} DataType;

/* Concrete Parse Tree Node */
typedef struct ParseTreeNode {
    char *name;                     /* Production or token name */
    char *attribute;                /* Optional value or lexeme */
    struct ParseTreeNode **children;
    int child_count;
    int child_capacity;
    int line;
} ParseTreeNode;

ParseTreeNode *pt_create_node(const char *name, const char *attribute, int line);
void pt_add_child(ParseTreeNode *parent, ParseTreeNode *child);
void print_parse_tree(ParseTreeNode *root);
void free_parse_tree(ParseTreeNode *root);

/* AST Node Types */
typedef enum {
    NODE_PROGRAM,
    NODE_BLOCK,
    NODE_VAR_DECL,
    NODE_ASSIGN,
    NODE_BINARY_OP,
    NODE_UNARY_OP,
    NODE_LITERAL_INT,
    NODE_LITERAL_FLOAT,
    NODE_LITERAL_STRING,
    NODE_LITERAL_BOOL,
    NODE_IDENTIFIER,
    NODE_IF,
    NODE_WHILE,
    NODE_FOR,
    NODE_PRINT,
    NODE_EMPTY
} ASTNodeType;

/* Binary and Unary Operators */
typedef enum {
    OP_NONE,
    /* Arithmetic */
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_MOD,
    /* Relational */
    OP_EQ,
    OP_NEQ,
    OP_LT,
    OP_LTE,
    OP_GT,
    OP_GTE,
    /* Logical */
    OP_AND,
    OP_OR,
    OP_NOT,
    /* Unary */
    OP_NEG
} OperatorType;

/* Abstract Syntax Tree Node */
typedef struct ASTNode {
    ASTNodeType type;
    DataType data_type;      /* Inferred or declared type */
    int line;

    char *str_value;         /* For identifier names, string literals */
    int int_value;           /* For integer and boolean literals */
    double float_value;      /* For float literals */
    OperatorType op;         /* Operator type */

    /* Subtrees */
    struct ASTNode *left;
    struct ASTNode *right;
    struct ASTNode *cond;
    struct ASTNode *body;
    struct ASTNode *else_body;
    struct ASTNode *init;
    struct ASTNode *update;

    /* Next statement in block/program */
    struct ASTNode *next;
} ASTNode;

/* AST Node constructors */
ASTNode *ast_create_node(ASTNodeType type, int line);
ASTNode *ast_create_var_decl(DataType type, const char *name, ASTNode *init_expr, int line);
ASTNode *ast_create_assign(const char *name, ASTNode *expr, int line);
ASTNode *ast_create_binary_op(OperatorType op, ASTNode *left, ASTNode *right, int line);
ASTNode *ast_create_unary_op(OperatorType op, ASTNode *operand, int line);
ASTNode *ast_create_int(int val, int line);
ASTNode *ast_create_float(double val, int line);
ASTNode *ast_create_string(const char *val, int line);
ASTNode *ast_create_bool(int val, int line);
ASTNode *ast_create_identifier(const char *name, int line);
ASTNode *ast_create_if(ASTNode *cond, ASTNode *then_branch, ASTNode *else_branch, int line);
ASTNode *ast_create_while(ASTNode *cond, ASTNode *body, int line);
ASTNode *ast_create_for(ASTNode *init, ASTNode *cond, ASTNode *update, ASTNode *body, int line);
ASTNode *ast_create_block(ASTNode *stmts, int line);
ASTNode *ast_create_print(ASTNode *expr, int line);
ASTNode *ast_append_stmt(ASTNode *list, ASTNode *stmt);

/* AST Utility Functions */
void print_ast(ASTNode *root);
void free_ast(ASTNode *root);
const char *datatype_to_string(DataType type);
DataType string_to_datatype(const char *str);
const char *op_to_string(OperatorType op);
const char *ast_node_type_to_string(ASTNodeType type);

/* Unified Semantic Value for Bison parser */
typedef struct SemanticValue {
    ASTNode *ast;
    ParseTreeNode *pt;
} SemanticValue;

/* Token Recording for Lexical Analysis & Token Viewer */
typedef struct TokenInfo {
    int token_type;
    char *type_name;
    char *lexeme;
    int line;
    int column;
    struct TokenInfo *next;
} TokenInfo;

typedef struct TokenStream {
    TokenInfo *head;
    TokenInfo *tail;
    int count;
} TokenStream;

TokenStream *create_token_stream(void);
void free_token_stream(TokenStream *ts);
void token_stream_add(TokenStream *ts, int type, const char *type_name, const char *lexeme, int line, int col);
void print_token_stream(TokenStream *ts);

#endif /* AST_H */
