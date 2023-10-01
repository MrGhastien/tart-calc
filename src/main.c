#include "darray.h"
#include "interpreter.h"
#include "string-builder.h"
#include "token.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {

    initOperators();

    char *line = null;
    u64 size;
    while (true) {
        getline(&line, &size, stdin);
        printf("Input : %s", line);

        Token **tokens = tokenize(line);
        printf("Identified %zu tokens.\n", darrayLength(tokens));
        for (u64 i = 0; i < darrayLength(tokens); i++) {
            printf("%s", tokens[i]->symbol);
        }
        printf("%s", "\n");
        EvalNode *node = parse(tokens);

        printf("Build tree :\n");
        printTree(node);
        printf("\n==== End of tree ====\n");

        double n = treeEval(node);
        printf("Reuslt : %g\n", n);

        free(line);
    }

    return 0;
}
