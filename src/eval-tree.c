#include "eval-tree.h"

#include <stdlib.h>
#include <stdio.h>

EvalNode *treeCreate(Token *token) {
    EvalNode* node = malloc(sizeof *node);
    if(node == null)
        return null;

    node->children = darrayCreate(2, sizeof *node->children);
    node->parent = null;
    node->token = token;
    node->function = token->function.ptr; //TODO
    node->arity = token->function.arity;
    return node;
}

void treeAddChild(EvalNode *parent, EvalNode *child) {
    darrayAdd(parent->children, &child);
    child->parent = parent;
}

static void printTreeRec(EvalNode *tree, u64 level) {
    for (u64 i = 0; i < level; i++) {
        printf("%s", "  ");
    }
    printf("%s\n", tree->token->symbol);
    for (u64 i = 0; i < tree->arity; i++) {
        printTreeRec(tree->children[i], level + 1);
    }
}

void printTree(EvalNode *tree) {
    printTreeRec(tree, 0);
}

double treeEval(EvalNode *tree) {
    if(tree->token->identifier == NUMBER)
        return tree->token->value.number;
    double args[tree->arity];
    for (u64 i = 0; i < tree->arity; i++) {
        args[i] = treeEval(tree->children[i]);
    }
    return tree->function(args);
}
