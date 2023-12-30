#include "defines.h"
#include "token.h"

typedef struct eval_node {
    functionptr function;
    Token* token;
    Value optimizedValue;
    darray* children; // struct eval_node**
    struct eval_node* parent;
    u32 arity;
    bool optimized;
} EvalNode;

EvalNode* treeCreate(Token* token);

void treeAddChild(EvalNode* parent, EvalNode* child);

void printTree(EvalNode* tree);
bool treeEval(EvalNode* tree, Value* outResult);

void treeDestroy(EvalNode* node);
