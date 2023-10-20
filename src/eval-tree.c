#include "eval-tree.h"
#include "error.h"
#include "var-handler.h"

#include <stdio.h>
#include <stdlib.h>

EvalNode* treeCreate(Token* token) {
    EvalNode* node = malloc(sizeof *node);
    if (node == null)
        return null;

    node->children = darrayCreate(2, sizeof(EvalNode*));
    node->parent = null;
    node->token = token;
    node->function = token->function.ptr; // TODO
    node->arity = token->function.arity;
    return node;
}

void treeAddChild(EvalNode* parent, EvalNode* child) {
    darrayAdd(parent->children, child);
    child->parent = parent;
}

static void printTreeRec(EvalNode* tree, u64 level) {
    for (u64 i = 0; i < level; i++) {
        printf("%s", "  ");
    }
    printf("%s\n", tree->token->symbol);
    for (u64 i = 0; i < tree->arity; i++) {
        EvalNode* node;
        darrayGet(tree->children, i, &node);
        printTreeRec(node, level + 1);
    }
}

void printTree(EvalNode* tree) {
    printTreeRec(tree, 0);
}

static double evalAssign(EvalNode* tree, double* outResult) {
    EvalNode* var;
    EvalNode* expr;
    darrayGet(tree->children, 0, &var);
    darrayGet(tree->children, 1, &expr);
    if (!treeEval(expr, outResult)) {
        return false;
    }
    setVariable(var->token->symbol, *outResult);
    return true;
}

static double evalOperator(EvalNode* tree, double* outResult) {
    double args[tree->arity];
    for (u64 i = 0; i < tree->arity; i++) {
        if (getErrorCount() > 0)
            return 0;
        EvalNode* node;
        darrayGet(tree->children, i, &node);
        if (!treeEval(node, args + i))
            return false;
    }
    *outResult = tree->function(args);
    return true;
}

bool treeEval(EvalNode* tree, double* outResult) {
    if (getErrorCount() > 0)
        return false;
    switch (tree->token->identifier) {
    case NUMBER:
         *outResult = tree->token->value.number;
         return true;
    case ALPHA: {
        if (!getVariable(tree->token->symbol, outResult)) {
            signalError(ERR_UNDEFINED_VAR, tree->token);
            return false;
        }
        return true;
    }
    default:
        if (tree->token->value.operator.type == OPERATOR_ASSIGN)
            return evalAssign(tree, outResult);
        else
            return evalOperator(tree, outResult);
    }
}

void treeDestroy(EvalNode* tree) {
    EvalNode* child;
    for (u64 i = 0; i < darrayLength(tree->children); i++) {
        darrayGet(tree->children, i, &child);
        treeDestroy(child);
    }
    darrayDestroy(tree->children);
    free(tree);
}
