
%{
#include "parser.tab.h"
#include "../common.h"
%}

%%
[ \t\r\n]+                    ;
[A-Za-z_][A-Za-z0-9_]*        { yylval.s = dupstr(yytext); return ID; }
[0-9]+                        { yylval.s = dupstr(yytext); return NUM; }
.                             { return yytext[0]; }
%%
int yywrap(void) { return 1; }
