%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

extern int yylex(void);
extern int yylineno;
extern char *yytext;
extern int g_column;

void yyerror(const char *s);

ASTNode *g_ast_root = NULL;
ParseTreeNode *g_parse_tree_root = NULL;
int syntax_error_count = 0;
%}

%union {
    int int_val;
    double float_val;
    char *str_val;
    SemanticValue sem;
    struct {
        DataType data_type;
        ParseTreeNode *pt;
    } type_info;
}

/* Terminals */
%token <str_val> TOKEN_IDENTIFIER
%token <str_val> TOKEN_STRING_LIT
%token <int_val> TOKEN_INT_LIT
%token <float_val> TOKEN_FLOAT_LIT

%token TOKEN_INT TOKEN_FLOAT TOKEN_STRING TOKEN_BOOL
%token TOKEN_IF TOKEN_ELSE TOKEN_WHILE TOKEN_FOR TOKEN_PRINT
%token TOKEN_TRUE TOKEN_FALSE

%token TOKEN_ASSIGN TOKEN_SEMICOLON TOKEN_COMMA
%token TOKEN_LPAREN TOKEN_RPAREN TOKEN_LBRACE TOKEN_RBRACE

%token TOKEN_ADD TOKEN_SUB TOKEN_MUL TOKEN_DIV TOKEN_MOD
%token TOKEN_EQ TOKEN_NEQ TOKEN_LT TOKEN_LTE TOKEN_GT TOKEN_GTE
%token TOKEN_AND TOKEN_OR TOKEN_NOT

/* Precedence and Associativity */
%nonassoc LOWER_THAN_ELSE
%nonassoc TOKEN_ELSE

%left TOKEN_OR
%left TOKEN_AND
%left TOKEN_EQ TOKEN_NEQ
%left TOKEN_LT TOKEN_LTE TOKEN_GT TOKEN_GTE
%left TOKEN_ADD TOKEN_SUB
%left TOKEN_MUL TOKEN_DIV TOKEN_MOD
%right TOKEN_NOT UMINUS

/* Non-terminals */
%type <sem> program stmt_list stmt var_decl assign_stmt if_stmt while_stmt for_stmt print_stmt block
%type <sem> expr opt_expr opt_for_init opt_for_update
%type <type_info> type_spec

%start program

%%

program:
    stmt_list
    {
        g_ast_root = ast_create_node(NODE_PROGRAM, 1);
        g_ast_root->body = $1.ast;
        g_parse_tree_root = pt_create_node("Program", NULL, 1);
        if ($1.pt) pt_add_child(g_parse_tree_root, $1.pt);
    }
    | /* empty */
    {
        g_ast_root = ast_create_node(NODE_PROGRAM, 1);
        g_parse_tree_root = pt_create_node("Program", "(Empty)", 1);
    }
    ;

stmt_list:
    stmt_list stmt
    {
        $$.ast = ast_append_stmt($1.ast, $2.ast);
        $$.pt = pt_create_node("StmtList", NULL, yylineno);
        if ($1.pt) pt_add_child($$.pt, $1.pt);
        if ($2.pt) pt_add_child($$.pt, $2.pt);
    }
    | stmt
    {
        $$.ast = $1.ast;
        $$.pt = pt_create_node("StmtList", NULL, yylineno);
        if ($1.pt) pt_add_child($$.pt, $1.pt);
    }
    ;

stmt:
    var_decl           { $$ = $1; }
    | assign_stmt      { $$ = $1; }
    | if_stmt          { $$ = $1; }
    | while_stmt       { $$ = $1; }
    | for_stmt         { $$ = $1; }
    | print_stmt       { $$ = $1; }
    | block            { $$ = $1; }
    | TOKEN_SEMICOLON
    {
        $$.ast = NULL;
        $$.pt = pt_create_node("EmptyStmt", ";", yylineno);
    }
    | error TOKEN_SEMICOLON
    {
        syntax_error_count++;
        $$.ast = NULL;
        $$.pt = pt_create_node("SyntaxError", "Recovered", yylineno);
        yyerrok;
    }
    ;

type_spec:
    TOKEN_INT
    {
        $$.data_type = DATA_TYPE_INT;
        $$.pt = pt_create_node("TypeSpec", "int", yylineno);
    }
    | TOKEN_FLOAT
    {
        $$.data_type = DATA_TYPE_FLOAT;
        $$.pt = pt_create_node("TypeSpec", "float", yylineno);
    }
    | TOKEN_STRING
    {
        $$.data_type = DATA_TYPE_STRING;
        $$.pt = pt_create_node("TypeSpec", "string", yylineno);
    }
    | TOKEN_BOOL
    {
        $$.data_type = DATA_TYPE_BOOL;
        $$.pt = pt_create_node("TypeSpec", "bool", yylineno);
    }
    ;

var_decl:
    type_spec TOKEN_IDENTIFIER TOKEN_SEMICOLON
    {
        $$.ast = ast_create_var_decl($1.data_type, $2, NULL, yylineno);
        $$.pt = pt_create_node("VarDecl", $2, yylineno);
        pt_add_child($$.pt, $1.pt);
        pt_add_child($$.pt, pt_create_node("Identifier", $2, yylineno));
        free($2);
    }
    | type_spec TOKEN_IDENTIFIER TOKEN_ASSIGN expr TOKEN_SEMICOLON
    {
        $$.ast = ast_create_var_decl($1.data_type, $2, $4.ast, yylineno);
        $$.pt = pt_create_node("VarDeclInit", $2, yylineno);
        pt_add_child($$.pt, $1.pt);
        pt_add_child($$.pt, pt_create_node("Identifier", $2, yylineno));
        pt_add_child($$.pt, pt_create_node("AssignOp", "=", yylineno));
        pt_add_child($$.pt, $4.pt);
        free($2);
    }
    ;

assign_stmt:
    TOKEN_IDENTIFIER TOKEN_ASSIGN expr TOKEN_SEMICOLON
    {
        $$.ast = ast_create_assign($1, $3.ast, yylineno);
        $$.pt = pt_create_node("AssignStmt", $1, yylineno);
        pt_add_child($$.pt, pt_create_node("Identifier", $1, yylineno));
        pt_add_child($$.pt, pt_create_node("AssignOp", "=", yylineno));
        pt_add_child($$.pt, $3.pt);
        free($1);
    }
    ;

if_stmt:
    TOKEN_IF TOKEN_LPAREN expr TOKEN_RPAREN stmt %prec LOWER_THAN_ELSE
    {
        $$.ast = ast_create_if($3.ast, $5.ast, NULL, yylineno);
        $$.pt = pt_create_node("IfStmt", NULL, yylineno);
        pt_add_child($$.pt, $3.pt);
        pt_add_child($$.pt, $5.pt);
    }
    | TOKEN_IF TOKEN_LPAREN expr TOKEN_RPAREN stmt TOKEN_ELSE stmt
    {
        $$.ast = ast_create_if($3.ast, $5.ast, $7.ast, yylineno);
        $$.pt = pt_create_node("IfElseStmt", NULL, yylineno);
        pt_add_child($$.pt, $3.pt);
        pt_add_child($$.pt, $5.pt);
        pt_add_child($$.pt, $7.pt);
    }
    ;

while_stmt:
    TOKEN_WHILE TOKEN_LPAREN expr TOKEN_RPAREN stmt
    {
        $$.ast = ast_create_while($3.ast, $5.ast, yylineno);
        $$.pt = pt_create_node("WhileStmt", NULL, yylineno);
        pt_add_child($$.pt, $3.pt);
        pt_add_child($$.pt, $5.pt);
    }
    ;

for_stmt:
    TOKEN_FOR TOKEN_LPAREN opt_for_init TOKEN_SEMICOLON opt_expr TOKEN_SEMICOLON opt_for_update TOKEN_RPAREN stmt
    {
        $$.ast = ast_create_for($3.ast, $5.ast, $7.ast, $9.ast, yylineno);
        $$.pt = pt_create_node("ForStmt", NULL, yylineno);
        if ($3.pt) pt_add_child($$.pt, $3.pt);
        if ($5.pt) pt_add_child($$.pt, $5.pt);
        if ($7.pt) pt_add_child($$.pt, $7.pt);
        if ($9.pt) pt_add_child($$.pt, $9.pt);
    }
    ;

opt_for_init:
    /* empty */
    {
        $$.ast = NULL;
        $$.pt = NULL;
    }
    | type_spec TOKEN_IDENTIFIER TOKEN_ASSIGN expr
    {
        $$.ast = ast_create_var_decl($1.data_type, $2, $4.ast, yylineno);
        $$.pt = pt_create_node("ForInitDecl", $2, yylineno);
        pt_add_child($$.pt, $1.pt);
        pt_add_child($$.pt, pt_create_node("Identifier", $2, yylineno));
        pt_add_child($$.pt, $4.pt);
        free($2);
    }
    | TOKEN_IDENTIFIER TOKEN_ASSIGN expr
    {
        $$.ast = ast_create_assign($1, $3.ast, yylineno);
        $$.pt = pt_create_node("ForInitAssign", $1, yylineno);
        pt_add_child($$.pt, pt_create_node("Identifier", $1, yylineno));
        pt_add_child($$.pt, $3.pt);
        free($1);
    }
    ;

opt_expr:
    /* empty */
    {
        $$.ast = NULL;
        $$.pt = NULL;
    }
    | expr
    {
        $$ = $1;
    }
    ;

opt_for_update:
    /* empty */
    {
        $$.ast = NULL;
        $$.pt = NULL;
    }
    | TOKEN_IDENTIFIER TOKEN_ASSIGN expr
    {
        $$.ast = ast_create_assign($1, $3.ast, yylineno);
        $$.pt = pt_create_node("ForUpdateAssign", $1, yylineno);
        pt_add_child($$.pt, pt_create_node("Identifier", $1, yylineno));
        pt_add_child($$.pt, $3.pt);
        free($1);
    }
    ;

print_stmt:
    TOKEN_PRINT expr TOKEN_SEMICOLON
    {
        $$.ast = ast_create_print($2.ast, yylineno);
        $$.pt = pt_create_node("PrintStmt", NULL, yylineno);
        pt_add_child($$.pt, $2.pt);
    }
    ;

block:
    TOKEN_LBRACE stmt_list TOKEN_RBRACE
    {
        $$.ast = ast_create_block($2.ast, yylineno);
        $$.pt = pt_create_node("Block", NULL, yylineno);
        pt_add_child($$.pt, $2.pt);
    }
    | TOKEN_LBRACE TOKEN_RBRACE
    {
        $$.ast = ast_create_block(NULL, yylineno);
        $$.pt = pt_create_node("EmptyBlock", "{}", yylineno);
    }
    ;

expr:
    expr TOKEN_OR expr
    {
        $$.ast = ast_create_binary_op(OP_OR, $1.ast, $3.ast, yylineno);
        $$.pt = pt_create_node("BinaryExpr", "||", yylineno);
        pt_add_child($$.pt, $1.pt); pt_add_child($$.pt, $3.pt);
    }
    | expr TOKEN_AND expr
    {
        $$.ast = ast_create_binary_op(OP_AND, $1.ast, $3.ast, yylineno);
        $$.pt = pt_create_node("BinaryExpr", "&&", yylineno);
        pt_add_child($$.pt, $1.pt); pt_add_child($$.pt, $3.pt);
    }
    | expr TOKEN_EQ expr
    {
        $$.ast = ast_create_binary_op(OP_EQ, $1.ast, $3.ast, yylineno);
        $$.pt = pt_create_node("BinaryExpr", "==", yylineno);
        pt_add_child($$.pt, $1.pt); pt_add_child($$.pt, $3.pt);
    }
    | expr TOKEN_NEQ expr
    {
        $$.ast = ast_create_binary_op(OP_NEQ, $1.ast, $3.ast, yylineno);
        $$.pt = pt_create_node("BinaryExpr", "!=", yylineno);
        pt_add_child($$.pt, $1.pt); pt_add_child($$.pt, $3.pt);
    }
    | expr TOKEN_LT expr
    {
        $$.ast = ast_create_binary_op(OP_LT, $1.ast, $3.ast, yylineno);
        $$.pt = pt_create_node("BinaryExpr", "<", yylineno);
        pt_add_child($$.pt, $1.pt); pt_add_child($$.pt, $3.pt);
    }
    | expr TOKEN_LTE expr
    {
        $$.ast = ast_create_binary_op(OP_LTE, $1.ast, $3.ast, yylineno);
        $$.pt = pt_create_node("BinaryExpr", "<=", yylineno);
        pt_add_child($$.pt, $1.pt); pt_add_child($$.pt, $3.pt);
    }
    | expr TOKEN_GT expr
    {
        $$.ast = ast_create_binary_op(OP_GT, $1.ast, $3.ast, yylineno);
        $$.pt = pt_create_node("BinaryExpr", ">", yylineno);
        pt_add_child($$.pt, $1.pt); pt_add_child($$.pt, $3.pt);
    }
    | expr TOKEN_GTE expr
    {
        $$.ast = ast_create_binary_op(OP_GTE, $1.ast, $3.ast, yylineno);
        $$.pt = pt_create_node("BinaryExpr", ">=", yylineno);
        pt_add_child($$.pt, $1.pt); pt_add_child($$.pt, $3.pt);
    }
    | expr TOKEN_ADD expr
    {
        $$.ast = ast_create_binary_op(OP_ADD, $1.ast, $3.ast, yylineno);
        $$.pt = pt_create_node("BinaryExpr", "+", yylineno);
        pt_add_child($$.pt, $1.pt); pt_add_child($$.pt, $3.pt);
    }
    | expr TOKEN_SUB expr
    {
        $$.ast = ast_create_binary_op(OP_SUB, $1.ast, $3.ast, yylineno);
        $$.pt = pt_create_node("BinaryExpr", "-", yylineno);
        pt_add_child($$.pt, $1.pt); pt_add_child($$.pt, $3.pt);
    }
    | expr TOKEN_MUL expr
    {
        $$.ast = ast_create_binary_op(OP_MUL, $1.ast, $3.ast, yylineno);
        $$.pt = pt_create_node("BinaryExpr", "*", yylineno);
        pt_add_child($$.pt, $1.pt); pt_add_child($$.pt, $3.pt);
    }
    | expr TOKEN_DIV expr
    {
        $$.ast = ast_create_binary_op(OP_DIV, $1.ast, $3.ast, yylineno);
        $$.pt = pt_create_node("BinaryExpr", "/", yylineno);
        pt_add_child($$.pt, $1.pt); pt_add_child($$.pt, $3.pt);
    }
    | expr TOKEN_MOD expr
    {
        $$.ast = ast_create_binary_op(OP_MOD, $1.ast, $3.ast, yylineno);
        $$.pt = pt_create_node("BinaryExpr", "%", yylineno);
        pt_add_child($$.pt, $1.pt); pt_add_child($$.pt, $3.pt);
    }
    | TOKEN_SUB expr %prec UMINUS
    {
        $$.ast = ast_create_unary_op(OP_NEG, $2.ast, yylineno);
        $$.pt = pt_create_node("UnaryExpr", "-", yylineno);
        pt_add_child($$.pt, $2.pt);
    }
    | TOKEN_NOT expr
    {
        $$.ast = ast_create_unary_op(OP_NOT, $2.ast, yylineno);
        $$.pt = pt_create_node("UnaryExpr", "!", yylineno);
        pt_add_child($$.pt, $2.pt);
    }
    | TOKEN_LPAREN expr TOKEN_RPAREN
    {
        $$.ast = $2.ast;
        $$.pt = pt_create_node("ParenExpr", "()", yylineno);
        pt_add_child($$.pt, $2.pt);
    }
    | TOKEN_IDENTIFIER
    {
        $$.ast = ast_create_identifier($1, yylineno);
        $$.pt = pt_create_node("Identifier", $1, yylineno);
        free($1);
    }
    | TOKEN_INT_LIT
    {
        $$.ast = ast_create_int($1, yylineno);
        char buf[32]; snprintf(buf, sizeof(buf), "%d", $1);
        $$.pt = pt_create_node("IntLiteral", buf, yylineno);
    }
    | TOKEN_FLOAT_LIT
    {
        $$.ast = ast_create_float($1, yylineno);
        char buf[32]; snprintf(buf, sizeof(buf), "%g", $1);
        $$.pt = pt_create_node("FloatLiteral", buf, yylineno);
    }
    | TOKEN_STRING_LIT
    {
        $$.ast = ast_create_string($1, yylineno);
        $$.pt = pt_create_node("StringLiteral", $1, yylineno);
        free($1);
    }
    | TOKEN_TRUE
    {
        $$.ast = ast_create_bool(1, yylineno);
        $$.pt = pt_create_node("BoolLiteral", "true", yylineno);
    }
    | TOKEN_FALSE
    {
        $$.ast = ast_create_bool(0, yylineno);
        $$.pt = pt_create_node("BoolLiteral", "false", yylineno);
    }
    ;

%%

void yyerror(const char *s) {
    syntax_error_count++;
    fprintf(stderr, "\033[1;31m[Syntax Error]\033[0m Line %d, near '%s': %s\n", yylineno, yytext ? yytext : "", s);
}
