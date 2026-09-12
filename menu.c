#include "menu.h"

extern FILE *yyin;
extern int yylex(void);
extern int yyparse(void);
extern void yyrestart(FILE *input_file);
extern int yylineno;
extern int g_column;
extern TokenStream *g_token_stream;
extern int lex_error_count;
extern int syntax_error_count;
extern ASTNode *g_ast_root;
extern ParseTreeNode *g_parse_tree_root;

CompilerState *create_compiler_state(const char *filepath) {
    CompilerState *cs = (CompilerState *)calloc(1, sizeof(CompilerState));
    if (!cs) {
        fprintf(stderr, "Error: Out of memory in create_compiler_state\n");
        exit(1);
    }
    if (filepath) {
        load_file(cs, filepath);
    }
    return cs;
}

void free_compiler_state(CompilerState *cs) {
    if (!cs) return;
    if (cs->tokens) free_token_stream(cs->tokens);
    if (cs->parse_tree) free_parse_tree(cs->parse_tree);
    if (cs->ast) free_ast(cs->ast);
    if (cs->semantic) free_semantic_analyzer(cs->semantic);
    if (cs->tac_raw) free_tac_list(cs->tac_raw);
    if (cs->tac_optimized) free_tac_list(cs->tac_optimized);
    if (cs->stack_prog) free_stack_program(cs->stack_prog);
    free(cs);
}

static void reset_phase_data(CompilerState *cs) {
    if (cs->tokens) { free_token_stream(cs->tokens); cs->tokens = NULL; }
    if (cs->parse_tree) { free_parse_tree(cs->parse_tree); cs->parse_tree = NULL; }
    if (cs->ast) { free_ast(cs->ast); cs->ast = NULL; }
    if (cs->semantic) { free_semantic_analyzer(cs->semantic); cs->semantic = NULL; }
    if (cs->tac_raw) { free_tac_list(cs->tac_raw); cs->tac_raw = NULL; }
    if (cs->tac_optimized) { free_tac_list(cs->tac_optimized); cs->tac_optimized = NULL; }
    if (cs->stack_prog) { free_stack_program(cs->stack_prog); cs->stack_prog = NULL; }
    cs->lex_done = false;
    cs->parse_done = false;
    cs->sem_done = false;
    cs->tac_done = false;
    cs->opt_done = false;
    cs->stack_done = false;
}

bool load_file(CompilerState *cs, const char *filepath) {
    if (!cs || !filepath) return false;
    FILE *f = fopen(filepath, "r");
    if (!f) {
        printf("\033[1;31mError: Cannot open source file '%s'\033[0m\n", filepath);
        return false;
    }
    fclose(f);
    strncpy(cs->current_filepath, filepath, sizeof(cs->current_filepath) - 1);
    cs->current_filepath[sizeof(cs->current_filepath) - 1] = '\0';
    reset_phase_data(cs);
    return true;
}

bool run_lexical_analysis(CompilerState *cs) {
    if (strlen(cs->current_filepath) == 0) {
        printf("\033[1;33mWarning: No source file currently loaded.\033[0m\n");
        return false;
    }

    FILE *f = fopen(cs->current_filepath, "r");
    if (!f) {
        printf("\033[1;31mError: Cannot open '%s'\033[0m\n", cs->current_filepath);
        return false;
    }

    if (cs->tokens) free_token_stream(cs->tokens);
    cs->tokens = create_token_stream();
    g_token_stream = cs->tokens;
    lex_error_count = 0;
    yylineno = 1;
    g_column = 1;

    yyrestart(f);
    while (yylex() != 0) {
        /* Scanning and recording tokens */
    }
    fclose(f);
    g_token_stream = NULL;
    cs->lex_done = true;

    printf("\n=======================================================\n");
    printf("                  LEXICAL ANALYSIS                     \n");
    printf("=======================================================\n");
    printf("  Source File       : %s\n", cs->current_filepath);
    printf("  Total Tokens Read : %d\n", cs->tokens->count);
    printf("  Lines Scanned     : %d\n", yylineno);
    printf("  Lexical Errors    : %d\n", lex_error_count);
    if (lex_error_count == 0) {
        printf("  Status            : \033[1;32mSUCCESS\033[0m\n");
    } else {
        printf("  Status            : \033[1;31mERRORS DETECTED\033[0m\n");
    }
    printf("=======================================================\n\n");
    return (lex_error_count == 0);
}

void run_view_tokens(CompilerState *cs) {
    if (!cs->lex_done) {
        if (!run_lexical_analysis(cs)) return;
    }
    printf("\n--- [TOKEN STREAM VIEW] ---\n");
    print_token_stream(cs->tokens);
}

bool run_syntax_analysis(CompilerState *cs) {
    if (strlen(cs->current_filepath) == 0) {
        printf("\033[1;33mWarning: No source file loaded.\033[0m\n");
        return false;
    }

    FILE *f = fopen(cs->current_filepath, "r");
    if (!f) {
        printf("\033[1;31mError: Cannot open '%s'\033[0m\n", cs->current_filepath);
        return false;
    }

    if (cs->parse_tree) { free_parse_tree(cs->parse_tree); cs->parse_tree = NULL; }
    if (cs->ast) { free_ast(cs->ast); cs->ast = NULL; }

    g_ast_root = NULL;
    g_parse_tree_root = NULL;
    syntax_error_count = 0;
    yylineno = 1;
    g_column = 1;
    g_token_stream = NULL;

    yyrestart(f);
    int res = yyparse();
    fclose(f);

    cs->ast = g_ast_root;
    cs->parse_tree = g_parse_tree_root;
    cs->parse_done = (res == 0 && syntax_error_count == 0);

    printf("\n=======================================================\n");
    printf("                  SYNTAX ANALYSIS                      \n");
    printf("=======================================================\n");
    printf("  Source File       : %s\n", cs->current_filepath);
    printf("  Syntax Errors     : %d\n", syntax_error_count);
    if (cs->parse_done) {
        printf("  Status            : \033[1;32mPARSING SUCCESSFUL\033[0m\n");
    } else {
        printf("  Status            : \033[1;31mPARSING FAILED\033[0m\n");
    }
    printf("=======================================================\n\n");

    return cs->parse_done;
}

void run_display_parse_tree(CompilerState *cs) {
    if (!cs->parse_done) {
        if (!run_syntax_analysis(cs)) {
            if (!cs->parse_tree) return;
        }
    }
    print_parse_tree(cs->parse_tree);
}

void run_display_ast(CompilerState *cs) {
    if (!cs->parse_done) {
        if (!run_syntax_analysis(cs)) {
            if (!cs->ast) return;
        }
    }
    print_ast(cs->ast);
}

bool run_semantic_analysis(CompilerState *cs) {
    if (!cs->parse_done) {
        if (!run_syntax_analysis(cs)) return false;
    }
    if (!cs->ast) {
        printf("\033[1;31mError: AST is null, cannot perform semantic analysis.\033[0m\n");
        return false;
    }

    if (cs->semantic) {
        free_semantic_analyzer(cs->semantic);
        cs->semantic = NULL;
    }
    cs->semantic = create_semantic_analyzer();
    int errors = analyze_semantics(cs->semantic, cs->ast);
    cs->sem_done = (errors == 0);

    print_semantic_errors(cs->semantic);
    return cs->sem_done;
}

void run_display_symbol_table(CompilerState *cs) {
    if (!cs->semantic) {
        run_semantic_analysis(cs);
    }
    if (cs->semantic && cs->semantic->symbol_table) {
        print_symbol_table(cs->semantic->symbol_table);
    } else {
        printf("(Symbol table unavailable)\n");
    }
}

bool run_generate_tac(CompilerState *cs) {
    if (!cs->sem_done) {
        if (!run_semantic_analysis(cs)) {
            printf("\033[1;33mWarning: Semantic errors detected. Generating TAC anyway for diagnostic purposes...\033[0m\n");
        }
    }
    if (!cs->ast) {
        printf("\033[1;31mError: No valid AST available to generate TAC.\033[0m\n");
        return false;
    }

    if (cs->tac_raw) free_tac_list(cs->tac_raw);
    cs->tac_raw = generate_tac(cs->ast);
    cs->tac_done = (cs->tac_raw != NULL);

    print_tac(cs->tac_raw);
    return cs->tac_done;
}

bool run_optimize_code(CompilerState *cs) {
    if (!cs->tac_done) {
        if (!run_generate_tac(cs)) return false;
    }
    if (!cs->tac_raw) {
        printf("\033[1;31mError: No TAC available to optimize.\033[0m\n");
        return false;
    }

    if (cs->tac_optimized) free_tac_list(cs->tac_optimized);
    cs->tac_optimized = clone_tac_list(cs->tac_raw);
    cs->opt_stats = optimize_tac(cs->tac_optimized);
    cs->opt_done = true;

    print_optimization_report(cs->tac_raw, cs->tac_optimized, cs->opt_stats);
    return true;
}

bool run_generate_stack_code(CompilerState *cs) {
    if (!cs->opt_done) {
        run_optimize_code(cs);
    }
    TACList *target = cs->tac_optimized ? cs->tac_optimized : cs->tac_raw;
    if (!target) {
        printf("\033[1;31mError: No TAC available for stack machine code generation.\033[0m\n");
        return false;
    }

    if (cs->stack_prog) free_stack_program(cs->stack_prog);
    cs->stack_prog = generate_stack_code(target);
    cs->stack_done = (cs->stack_prog != NULL);

    print_stack_code(cs->stack_prog);
    return cs->stack_done;
}

bool run_compile_complete(CompilerState *cs) {
    printf("\n=======================================================\n");
    printf("         COMPILEARN: FULL PIPELINE COMPILATION         \n");
    printf("=======================================================\n");
    printf("Compiling source: %s\n\n", cs->current_filepath);

    printf("[Phase 1/6] Lexical Analysis...\n");
    if (!run_lexical_analysis(cs)) {
        printf("\033[1;31m[Compilation Aborted] Lexical errors encountered.\033[0m\n");
        return false;
    }

    printf("[Phase 2/6] Syntax Analysis & CST/AST Construction...\n");
    if (!run_syntax_analysis(cs)) {
        printf("\033[1;31m[Compilation Aborted] Syntax errors encountered.\033[0m\n");
        return false;
    }

    printf("[Phase 3/6] Semantic Analysis & Scoped Symbol Resolution...\n");
    if (!run_semantic_analysis(cs)) {
        printf("\033[1;31m[Compilation Aborted] Semantic errors encountered.\033[0m\n");
        return false;
    }

    printf("[Phase 4/6] Intermediate Representation (Three Address Code)...\n");
    run_generate_tac(cs);

    printf("[Phase 5/6] Intermediate Code Optimization...\n");
    run_optimize_code(cs);

    printf("[Phase 6/6] Target Code Generation (Stack Machine Code)...\n");
    run_generate_stack_code(cs);

    printf("=======================================================\n");
    printf("         EXECUTING COMPILED PROGRAM ON STACK VM        \n");
    printf("=======================================================\n");
    execute_stack_program(cs->stack_prog);

    return true;
}

static void display_menu_header(const CompilerState *cs) {
    printf("\n================================\n");
    printf("          COMPILEARN            \n");
    printf("================================\n");
    if (strlen(cs->current_filepath) > 0) {
        printf("Current File: \033[1;36m%s\033[0m\n", cs->current_filepath);
    } else {
        printf("Current File: \033[1;33m(None loaded)\033[0m\n");
    }
    printf("--------------------------------\n");
    printf("1.  Lexical Analysis\n");
    printf("2.  View Tokens\n");
    printf("3.  Parse Source Code\n");
    printf("4.  Display Parse Tree\n");
    printf("5.  Display AST\n");
    printf("6.  Display Symbol Table\n");
    printf("7.  Semantic Analysis\n");
    printf("8.  Generate TAC\n");
    printf("9.  Optimize Code\n");
    printf("10. Generate Stack Code\n");
    printf("11. Compile Complete Program\n");
    printf("12. Exit\n");
    printf("--------------------------------\n");
    printf("C.  Change Source File\n");
    printf("================================\n");
    printf("Enter choice (1-12 or C): ");
}

void run_interactive_menu(const char *initial_file) {
    CompilerState *cs = create_compiler_state(initial_file);

    if (strlen(cs->current_filepath) == 0) {
        char buf[512];
        printf("Enter source file to load: ");
        if (fgets(buf, sizeof(buf), stdin)) {
            buf[strcspn(buf, "\r\n")] = '\0';
            if (strlen(buf) > 0) {
                load_file(cs, buf);
            }
        }
    }

    char choice[64];
    while (1) {
        display_menu_header(cs);
        if (!fgets(choice, sizeof(choice), stdin)) {
            break;
        }
        choice[strcspn(choice, "\r\n")] = '\0';

        if (strcmp(choice, "1") == 0) {
            run_lexical_analysis(cs);
        } else if (strcmp(choice, "2") == 0) {
            run_view_tokens(cs);
        } else if (strcmp(choice, "3") == 0) {
            run_syntax_analysis(cs);
        } else if (strcmp(choice, "4") == 0) {
            run_display_parse_tree(cs);
        } else if (strcmp(choice, "5") == 0) {
            run_display_ast(cs);
        } else if (strcmp(choice, "6") == 0) {
            run_display_symbol_table(cs);
        } else if (strcmp(choice, "7") == 0) {
            run_semantic_analysis(cs);
        } else if (strcmp(choice, "8") == 0) {
            run_generate_tac(cs);
        } else if (strcmp(choice, "9") == 0) {
            run_optimize_code(cs);
        } else if (strcmp(choice, "10") == 0) {
            run_generate_stack_code(cs);
        } else if (strcmp(choice, "11") == 0) {
            run_compile_complete(cs);
        } else if (strcmp(choice, "12") == 0) {
            printf("\nExiting CompiLearn. Happy compiling!\n");
            break;
        } else if (choice[0] == 'c' || choice[0] == 'C') {
            char new_file[512];
            printf("Enter new source file path: ");
            if (fgets(new_file, sizeof(new_file), stdin)) {
                new_file[strcspn(new_file, "\r\n")] = '\0';
                if (strlen(new_file) > 0) {
                    load_file(cs, new_file);
                }
            }
        } else {
            printf("\033[1;31mInvalid option '%s'. Please select 1-12 or C.\033[0m\n", choice);
        }
    }

    free_compiler_state(cs);
}
