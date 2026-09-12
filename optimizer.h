#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include "tac.h"

typedef struct OptimizationStats {
    int constant_folds;
    int constant_propagations;
    int copy_propagations;
    int dead_code_eliminations;
    int total_passes;
} OptimizationStats;

OptimizationStats optimize_tac(TACList *list);
void print_optimization_report(TACList *before, TACList *after, OptimizationStats stats);

#endif /* OPTIMIZER_H */
