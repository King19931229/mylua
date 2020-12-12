#pragma once
#include "compiler/ast/exp.h"

ExpPtr OptimizeLogicOr(ExpPtr exp);
ExpPtr OptimizeLogicAnd(ExpPtr exp);
ExpPtr OptimizeBitwiseBinaryOp(ExpPtr exp);
ExpPtr OptimizeArithBinaryOp(ExpPtr exp);
ExpPtr OptimizePow(ExpPtr exp);
ExpPtr OptimizeUnaryOp(ExpPtr exp);
ExpPtr OptimizeUnm(ExpPtr exp);
ExpPtr OptimizeNot(ExpPtr exp);
ExpPtr OptimizeBnot(ExpPtr exp);
bool IsFalse(ExpPtr exp);
bool IsTrue(ExpPtr exp);
bool IsVarargOrFuncCall(ExpPtr exp);
std::tuple<Int64, bool> CastToInt(ExpPtr exp);
std::tuple<Float64, bool> CastToFloat(ExpPtr exp);