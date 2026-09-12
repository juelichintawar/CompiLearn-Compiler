#ifndef MENU_H
#define MENU_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "ast.h"
#include "symboltable.h"
#include "semantic.h"
#include "tac.h"
#include "optimizer.h"
#include "stackcode.h"

/* Compiler Pipeline State */
typedef struct CompilerState {
    char current_filepath[512];
    TokenStream *tokens;
    ParseTreeNode *parse_tree;
    ASTNode *ast;
    SemanticAnalyzer *semantic;
    TACList *tac_raw;
    TACList *tac_optimized;
    StackProgram *stack_prog;
    OptimizationStats opt_stats;
    bool lex_done;
    bool parse_done;
    bool sem_done;
    bool tac_done;
    bool opt_done;
    bool stack_done;
} CompilerState;

CompilerState *create_compiler_state(const char *filepath);
void free_compiler_state(CompilerState *cs);
bool load_file(CompilerState *cs, const char *filepath);

/* Individual Compiler Phase Runners */
bool run_lexical_analysis(CompilerState *cs);
void run_view_tokens(CompilerState *cs);
bool run_syntax_analysis(CompilerState *cs);
void run_display_parse_tree(CompilerState *cs);
void run_display_ast(CompilerState *cs);
void run_display_symbol_table(CompilerState *cs);
bool run_semantic_analysis(CompilerState *cs);
bool run_generate_tac(CompilerState *cs);
bool run_optimize_code(CompilerState *cs);
bool run_generate_stack_code(CompilerState *cs);
bool run_compile_complete(CompilerState *cs);

/* Interactive Menu */
void run_interactive_menu(const char *initial_file);

#endif /* MENU_H */
