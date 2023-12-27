#include "function.h"
#include "error.h"
#include "var-handler.h"
#include "math.h"

static Function* functions;

void initFunctions() {
}

//TODO: all of these !
bool funcAdd(Value* args, Value* outResult) {
    return true;
}

bool funcSubtract(Value* args, Value* outResult) {
    return true;
}

bool funcMultiply(Value* args, Value* outResult) {
    return true;
}

bool funcDivide(Value* args, Value* outResult) {
    return true;
}

bool funcModulo(Value* args, Value* outResult) {
    return true;
}

bool funcPower(Value* args, Value* outResult) {
    return true;
}

bool funcAssign(char* symbol, Value* value) {
    return true;
}

const Function ADD = {funcAdd, 2};
const Function SUBTRACT = {funcSubtract, 2};
const Function MULTIPLY = {funcMultiply, 2};
const Function DIVIDE = {funcDivide, 2};
const Function MODULO = {funcModulo, 2};
const Function POWER = {funcPower, 2};
const Function ASSIGN = {null, 2};
