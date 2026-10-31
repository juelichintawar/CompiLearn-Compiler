
%{
#include "../common.h"
int yylex(void);
void yyerror(const char *s);
%}

%union { char *s; }
%token <s> ID NUM
%type <s> expr term factor

%left '+' '-'
%left '*' '/' '%'

%%
input:
    ID '=' expr ';'
    {
        emit("=", $3, "-", $1);
        print_all();
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
    printf("Enter assignment (example: x = (a+b)*c-d;):\n");
    return yyparse();
}
