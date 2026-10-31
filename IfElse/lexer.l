
%{
#include "parser.tab.h"
#include "../common.h"
%}

%%
"if"                          { return IF; }
"else"                        { return ELSE; }
"<="|">="|"=="|"!="|"<"|">"  { yylval.s = dupstr(yytext); return RELOP; }
[ \t\r\n]+                    ;
[A-Za-z_][A-Za-z0-9_]*        { yylval.s = dupstr(yytext); return ID; }
[0-9]+                        { yylval.s = dupstr(yytext); return NUM; }
.                             { return yytext[0]; }
%%
int yywrap(void) { return 1; }
