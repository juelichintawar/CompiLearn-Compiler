
%{
#include "../common.h"
int yylex(void);
void yyerror(const char *s);

static void begin_if(char *a, char *rel, char *b) {
    char op[32];
    snprintf(op, sizeof(op), "if %s", rel);
    emit(op, a, b, "L1");
    emit("goto", "-", "-", "L2");
    emit("label", "-", "-", "L1");
}

static void start_else(void) {
    emit("goto", "-", "-", "L3");
    emit("label", "-", "-", "L2");
}

static void end_if(void) {
    emit("label", "-", "-", "L3");
}
%}

%union { char *s; }
%token <s> ID NUM RELOP
%token IF ELSE
%type <s> expr term factor

%left '+' '-'
%left '*' '/' '%'

%%
program:
    IF '(' ID RELOP ID ')'
    {
        begin_if($3, $4, $5);
    }
    assignment ELSE
    {
        start_else();
    }
    assignment
    {
        end_if();
        print_all();
    }
    ;

assignment:
    ID '=' expr ';'
    {
        emit("=", $3, "-", $1);
    }
    ;

expr:
    expr '+' term
    {
        $$ = newtemp();
        emit("+", $1, $3, $$);
    }
  | expr '-' term
    {
        $$ = newtemp();
        emit("-", $1, $3, $$);
    }
  | term { $$ = $1; }
  ;

term:
    term '*' factor
    {
        $$ = newtemp();
        emit("*", $1, $3, $$);
    }
  | term '/' factor
    {
        $$ = newtemp();
        emit("/", $1, $3, $$);
    }
  | term '%' factor
    {
        $$ = newtemp();
        emit("%", $1, $3, $$);
    }
  | factor { $$ = $1; }
  ;

factor:
    '(' expr ')' { $$ = $2; }
  | ID           { $$ = $1; }
  | NUM          { $$ = $1; }
  ;
%%

void yyerror(const char *s) {
    fprintf(stderr, "Syntax error: %s\n", s);
}

int main(void) {
    printf("Enter if-else statement:\n");
    return yyparse();
}
