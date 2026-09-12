%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "parser.tab.h"

int g_column = 1;
TokenStream *g_token_stream = NULL;
int lex_error_count = 0;

static int return_tok(int token_type, const char *type_name) {
    if (g_token_stream) {
        token_stream_add(g_token_stream, token_type, type_name, yytext, yylineno, g_column);
    }
    g_column += yyleng;
    return token_type;
}
%}

%option noyywrap
%option yylineno
%option nounput
%option noinput

DIGIT       [0-9]
INT_NUM     {DIGIT}+
FLOAT_NUM   {DIGIT}+\.{DIGIT}+
IDENT       [a-zA-Z_][a-zA-Z0-9_]*
STRING_LIT  \"([^"\\]|\\.)*\"

%%

"//".*                  { /* Single-line comment: ignore, but update column */ g_column += yyleng; }
"/*"([^*]|\*+[^*/])*\*+"/" {
    /* Multi-line comment: update line and column */
    for (int i = 0; i < yyleng; i++) {
        if (yytext[i] == '\n') {
            g_column = 1;
        } else {
            g_column++;
        }
    }
}

[ \t\r]+                { g_column += yyleng; /* ignore whitespace */ }
\n                      { g_column = 1; }

"int"                   { return return_tok(TOKEN_INT, "KEYWORD_INT"); }
"float"                 { return return_tok(TOKEN_FLOAT, "KEYWORD_FLOAT"); }
"string"                { return return_tok(TOKEN_STRING, "KEYWORD_STRING"); }
"bool"                  { return return_tok(TOKEN_BOOL, "KEYWORD_BOOL"); }
"if"                    { return return_tok(TOKEN_IF, "KEYWORD_IF"); }
"else"                  { return return_tok(TOKEN_ELSE, "KEYWORD_ELSE"); }
"while"                 { return return_tok(TOKEN_WHILE, "KEYWORD_WHILE"); }
"for"                   { return return_tok(TOKEN_FOR, "KEYWORD_FOR"); }
"print"                 { return return_tok(TOKEN_PRINT, "KEYWORD_PRINT"); }
"true"                  { return return_tok(TOKEN_TRUE, "KEYWORD_TRUE"); }
"false"                 { return return_tok(TOKEN_FALSE, "KEYWORD_FALSE"); }

{FLOAT_NUM} {
    yylval.float_val = atof(yytext);
    return return_tok(TOKEN_FLOAT_LIT, "LITERAL_FLOAT");
}

{INT_NUM} {
    yylval.int_val = atoi(yytext);
    return return_tok(TOKEN_INT_LIT, "LITERAL_INT");
}

{STRING_LIT} {
    /* Strip leading and trailing quotes */
    char *clean = (char *)malloc(yyleng - 1);
    strncpy(clean, yytext + 1, yyleng - 2);
    clean[yyleng - 2] = '\0';
    yylval.str_val = clean;
    return return_tok(TOKEN_STRING_LIT, "LITERAL_STRING");
}

{IDENT} {
    yylval.str_val = strdup(yytext);
    return return_tok(TOKEN_IDENTIFIER, "IDENTIFIER");
}

"="                     { return return_tok(TOKEN_ASSIGN, "OP_ASSIGN"); }
"=="                    { return return_tok(TOKEN_EQ, "OP_EQ"); }
"!="                    { return return_tok(TOKEN_NEQ, "OP_NEQ"); }
"<="                    { return return_tok(TOKEN_LTE, "OP_LTE"); }
">="                    { return return_tok(TOKEN_GTE, "OP_GTE"); }
"<"                     { return return_tok(TOKEN_LT, "OP_LT"); }
">"                     { return return_tok(TOKEN_GT, "OP_GT"); }
"&&"                    { return return_tok(TOKEN_AND, "OP_AND"); }
"||"                    { return return_tok(TOKEN_OR, "OP_OR"); }
"!"                     { return return_tok(TOKEN_NOT, "OP_NOT"); }

"+"                     { return return_tok(TOKEN_ADD, "OP_ADD"); }
"-"                     { return return_tok(TOKEN_SUB, "OP_SUB"); }
"*"                     { return return_tok(TOKEN_MUL, "OP_MUL"); }
"/"                     { return return_tok(TOKEN_DIV, "OP_DIV"); }
"%"                     { return return_tok(TOKEN_MOD, "OP_MOD"); }

";"                     { return return_tok(TOKEN_SEMICOLON, "DELIM_SEMICOLON"); }
","                     { return return_tok(TOKEN_COMMA, "DELIM_COMMA"); }
"("                     { return return_tok(TOKEN_LPAREN, "DELIM_LPAREN"); }
")"                     { return return_tok(TOKEN_RPAREN, "DELIM_RPAREN"); }
"{"                     { return return_tok(TOKEN_LBRACE, "DELIM_LBRACE"); }
"}"                     { return return_tok(TOKEN_RBRACE, "DELIM_RBRACE"); }

. {
    lex_error_count++;
    fprintf(stderr, "\033[1;31m[Lexical Error]\033[0m Line %d, Column %d: Unrecognized character '%s'\n",
            yylineno, g_column, yytext);
    g_column += yyleng;
}

%%
