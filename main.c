#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "menu.h"

static void print_usage(const char *prog_name) {
    printf("CompiLearn - Educational Compiler Construction Platform\n\n");
    printf("Usage:\n");
    printf("  Interactive Menu Mode:\n");
    printf("    %s [source_file]\n\n", prog_name);
    printf("  Batch / Non-Interactive Phase Modes:\n");
    printf("    %s --lex     <file>         Run Lexical Analysis\n", prog_name);
    printf("    %s --tokens  <file>         Display Token Stream Table\n", prog_name);
    printf("    %s --parse   <file>         Run Syntax Analysis (Bison Parser)\n", prog_name);
    printf("    %s --pt      <file>         Display Concrete Parse Tree\n", prog_name);
    printf("    %s --ast     <file>         Display Abstract Syntax Tree\n", prog_name);
    printf("    %s --sym     <file>         Display Scoped Symbol Table\n", prog_name);
    printf("    %s --sem     <file>         Run Semantic Analysis & Type Checking\n", prog_name);
    printf("    %s --tac     <file>         Generate Three Address Code (TAC)\n", prog_name);
    printf("    %s --opt     <file>         Run TAC Optimization & Show Report\n", prog_name);
    printf("    %s --stack   <file>         Generate Stack Machine Bytecode\n", prog_name);
    printf("    %s --run     <file>         Execute Program on Stack VM\n", prog_name);
    printf("    %s --all     <file>         Run Full Pipeline (Phases 1-6 + VM Execution)\n\n", prog_name);
    printf("  Alternative Named Arguments:\n");
    printf("    %s --file <file> --phase <phase>\n", prog_name);
    printf("    Phases: lex, tokens, parse, tree, ast, symbols, semantic, tac, optimize, stack, run, all\n\n");
    printf("    %s --help                   Show this help message\n", prog_name);
}

static int execute_phase_action(CompilerState *cs, const char *phase) {
    if (strcmp(phase, "lex") == 0 || strcmp(phase, "--lex") == 0) {
        return run_lexical_analysis(cs) ? 0 : 1;
    } else if (strcmp(phase, "tokens") == 0 || strcmp(phase, "--tokens") == 0) {
        run_view_tokens(cs);
        return 0;
    } else if (strcmp(phase, "parse") == 0 || strcmp(phase, "--parse") == 0) {
        return run_syntax_analysis(cs) ? 0 : 1;
    } else if (strcmp(phase, "tree") == 0 || strcmp(phase, "pt") == 0 || strcmp(phase, "--pt") == 0) {
        run_display_parse_tree(cs);
        return 0;
    } else if (strcmp(phase, "ast") == 0 || strcmp(phase, "--ast") == 0) {
        run_display_ast(cs);
        return 0;
    } else if (strcmp(phase, "symbols") == 0 || strcmp(phase, "sym") == 0 || strcmp(phase, "--sym") == 0) {
        run_display_symbol_table(cs);
        return 0;
    } else if (strcmp(phase, "semantic") == 0 || strcmp(phase, "sem") == 0 || strcmp(phase, "--sem") == 0) {
        return run_semantic_analysis(cs) ? 0 : 1;
    } else if (strcmp(phase, "tac") == 0 || strcmp(phase, "--tac") == 0) {
        return run_generate_tac(cs) ? 0 : 1;
    } else if (strcmp(phase, "optimize") == 0 || strcmp(phase, "opt") == 0 || strcmp(phase, "--opt") == 0) {
        return run_optimize_code(cs) ? 0 : 1;
    } else if (strcmp(phase, "stack") == 0 || strcmp(phase, "--stack") == 0) {
        return run_generate_stack_code(cs) ? 0 : 1;
    } else if (strcmp(phase, "run") == 0 || strcmp(phase, "--run") == 0) {
        run_generate_stack_code(cs);
        return execute_stack_program(cs->stack_prog);
    } else if (strcmp(phase, "all") == 0 || strcmp(phase, "--all") == 0) {
        return run_compile_complete(cs) ? 0 : 1;
    } else {
        fprintf(stderr, "Unknown phase: %s\n", phase);
        return 1;
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        /* Default: launch interactive menu */
        run_interactive_menu(NULL);
        return 0;
    }

    if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
        print_usage(argv[0]);
        return 0;
    }

    /* Check for named flags: --file <file> --phase <phase> or --phase <phase> --file <file> */
    const char *file_arg = NULL;
    const char *phase_arg = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--file") == 0 && i + 1 < argc) {
            file_arg = argv[++i];
        } else if (strcmp(argv[i], "--phase") == 0 && i + 1 < argc) {
            phase_arg = argv[++i];
        }
    }

    if (file_arg && phase_arg) {
        CompilerState *cs = create_compiler_state(file_arg);
        int ret = execute_phase_action(cs, phase_arg);
        free_compiler_state(cs);
        return ret;
    }

    /* Check standard single-flag format: <prog> --<flag> <file> */
    if (argv[1][0] == '-') {
        if (argc < 3) {
            fprintf(stderr, "Error: Flag '%s' requires a source file argument.\n", argv[1]);
            print_usage(argv[0]);
            return 1;
        }

        const char *flag = argv[1];
        const char *file = argv[2];
        CompilerState *cs = create_compiler_state(file);
        int ret = execute_phase_action(cs, flag);
        free_compiler_state(cs);
        return ret;
    }

    /* Otherwise, user provided a source file name to start the interactive menu with */
    run_interactive_menu(argv[1]);
    return 0;
}
