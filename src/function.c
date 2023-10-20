#include "function.h"
#include "error.h"
#include "var-handler.h"
#include "math.h"

static Function* functions;

void initFunctions() {
}

double funcAdd(double* args) {
    return args[0] + args[1];
}

double funcSubtract(double* args) {
    return args[0] - args[1];
}

double funcMultiply(double* args) {
    return args[0] * args[1];
}

double funcDivide(double* args) {
    if (args[1] == 0) {
        signalErrorNoToken(ERR_DIV_BY_ZERO, null, -1);
        return 0;
    }
    return args[0] / args[1];
}

double funcModulo(double* args) {
    if (args[1] == 0) {
        signalErrorNoToken(ERR_DIV_BY_ZERO, null, -1);
        return 0;
    }
    return (i64)args[0] % (i64)args[1];
}

double funcPower(double* args) {
    return pow(args[0], args[1]);
}

double funcAssign(char* symbol, double value) {
    setVariable(symbol, value);
    return value;
}

const Function ADD = {funcAdd, 2};
const Function SUBTRACT = {funcSubtract, 2};
const Function MULTIPLY = {funcMultiply, 2};
const Function DIVIDE = {funcDivide, 2};
const Function MODULO = {funcModulo, 2};
const Function POWER = {funcPower, 2};
const Function ASSIGN = {null, 2};
