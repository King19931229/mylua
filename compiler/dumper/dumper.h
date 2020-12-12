#pragma once
#include "compiler/ast/block.h"
#include "compiler/ast/exp.h"
#include "compiler/ast/stat.h"

void DumpWithIndent(int indent, const String& msg);
void DumpExp(ExpPtr exp, int indent);
void DumpStat(StatPtr stst, int indent);
void DumpBlock(BlockPtr block, int indent);