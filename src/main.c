#include "context.h"
#include "darray.h"
#include "interpreter.h"
#include "string-builder.h"
#include "token.h"
#include "error.h"

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static Context context;

void handleOptions(int argc, char **argv) {
    int r;
    while ((r = getopt(argc, argv, "v")) != -1) {
        char c = r;
        if (c == '?') {
            err(ERRCODE_UNKNOWN_OPTION, "Unknown option '%c%c'.", '-', optopt);
        }
        switch (c) {
        case 'v':
            context.verbose = 1;
        }
    }
}

int main(int argc, char **argv) {

    initErrorSystem();

    handleOptions(argc, argv);

    initTokens();
    initOperators();

    char *line = null;
    u64 size;
    while (getline(&line, &size, stdin) > 0) {

        if (context.verbose) {
            darray *tokens = tokenize(line);
            printf("Identified %zu tokens.\n", darrayLength(tokens));
            for (u64 i = 0; i < darrayLength(tokens); i++) {
                Token *t;
                darrayGet(tokens, i, &t);
                printf("%s", t->symbol);
            }
            printf("%s", "\n");
            EvalNode *node = parse(tokens);

            printf("Build tree :\n");
            printTree(node);
            printf("\n==== End of tree ====\n");

            double n = treeEval(node);
            printf("Reuslt : %g\n", n);
        } else {
            double result = evaluate(line);
            if (getErrorCount()) {
                puts("\e[31mFailed to compute result.\e[0m");
            } else {
                printf("\e[32m=> %g\e[0m\n\n", result);
            }
        }
    }
    free(line);
    shutTokens();
    shutErrorSystem();
    return 0;
}

void darrayPrintTokenPtr(darray *array) {
    Token** raw = array->a;
    for (u64 i = 0; i < darrayLength(array); i++) {
        Token *t = raw[i];
        printf("{id=%i, symbol='%s', value={number=%g, op={priority=%i, rightAssoc=%i}}, func={ptr=%p, arity=%u}",
               t->identifier, t->symbol, t->value.number, t->value.operator.priority ,t->value.operator.rightAssociative, t->function.ptr, t->function.arity);
        putchar('\n');
    }
}
