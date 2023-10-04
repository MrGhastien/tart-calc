#include "defines.h"
#include "token.h"

typedef struct eval_node {
    functionptr function;
    u32 arity;
    Token* token;
    darray* children; //struct eval_node**
    struct eval_node* parent;
} EvalNode;

EvalNode* treeCreate(Token* token);

void treeAddChild(EvalNode* parent, EvalNode* child);

void printTree(EvalNode *tree);
double treeEval(EvalNode* tree);
