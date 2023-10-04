#pragma once
#include "eval-tree.h"
#include "token.h"

darray *tokenize(const char *str);

EvalNode *parse(darray *tokens);

double evaluate(const char *expression);
