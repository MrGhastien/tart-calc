#pragma once
#include "token.h"
#include "eval-tree.h"

Token** tokenize(const char *str);

EvalNode *parse(Token **tokens);
