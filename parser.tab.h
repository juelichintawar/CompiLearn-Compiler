/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_PARSER_TAB_H_INCLUDED
# define YY_YY_PARSER_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    TOKEN_IDENTIFIER = 258,        /* TOKEN_IDENTIFIER  */
    TOKEN_STRING_LIT = 259,        /* TOKEN_STRING_LIT  */
    TOKEN_INT_LIT = 260,           /* TOKEN_INT_LIT  */
    TOKEN_FLOAT_LIT = 261,         /* TOKEN_FLOAT_LIT  */
    TOKEN_INT = 262,               /* TOKEN_INT  */
    TOKEN_FLOAT = 263,             /* TOKEN_FLOAT  */
    TOKEN_STRING = 264,            /* TOKEN_STRING  */
    TOKEN_BOOL = 265,              /* TOKEN_BOOL  */
    TOKEN_IF = 266,                /* TOKEN_IF  */
    TOKEN_ELSE = 267,              /* TOKEN_ELSE  */
    TOKEN_WHILE = 268,             /* TOKEN_WHILE  */
    TOKEN_FOR = 269,               /* TOKEN_FOR  */
    TOKEN_PRINT = 270,             /* TOKEN_PRINT  */
    TOKEN_TRUE = 271,              /* TOKEN_TRUE  */
    TOKEN_FALSE = 272,             /* TOKEN_FALSE  */
    TOKEN_ASSIGN = 273,            /* TOKEN_ASSIGN  */
    TOKEN_SEMICOLON = 274,         /* TOKEN_SEMICOLON  */
    TOKEN_COMMA = 275,             /* TOKEN_COMMA  */
    TOKEN_LPAREN = 276,            /* TOKEN_LPAREN  */
    TOKEN_RPAREN = 277,            /* TOKEN_RPAREN  */
    TOKEN_LBRACE = 278,            /* TOKEN_LBRACE  */
    TOKEN_RBRACE = 279,            /* TOKEN_RBRACE  */
    TOKEN_ADD = 280,               /* TOKEN_ADD  */
    TOKEN_SUB = 281,               /* TOKEN_SUB  */
    TOKEN_MUL = 282,               /* TOKEN_MUL  */
    TOKEN_DIV = 283,               /* TOKEN_DIV  */
    TOKEN_MOD = 284,               /* TOKEN_MOD  */
    TOKEN_EQ = 285,                /* TOKEN_EQ  */
    TOKEN_NEQ = 286,               /* TOKEN_NEQ  */
    TOKEN_LT = 287,                /* TOKEN_LT  */
    TOKEN_LTE = 288,               /* TOKEN_LTE  */
    TOKEN_GT = 289,                /* TOKEN_GT  */
    TOKEN_GTE = 290,               /* TOKEN_GTE  */
    TOKEN_AND = 291,               /* TOKEN_AND  */
    TOKEN_OR = 292,                /* TOKEN_OR  */
    TOKEN_NOT = 293,               /* TOKEN_NOT  */
    LOWER_THAN_ELSE = 294,         /* LOWER_THAN_ELSE  */
    UMINUS = 295                   /* UMINUS  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 19 "parser.y"

    int int_val;
    double float_val;
    char *str_val;
    SemanticValue sem;
    struct {
        DataType data_type;
        ParseTreeNode *pt;
    } type_info;

#line 115 "parser.tab.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;


int yyparse (void);


#endif /* !YY_YY_PARSER_TAB_H_INCLUDED  */
