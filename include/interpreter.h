#pragma once
#include "eval-tree.h"
#include "token.h"

bool tokenize(const char *str, darray* tokenBuffer);

EvalNode *parse(darray *tokens);

bool evaluate(const char* expression, double* outResult);
