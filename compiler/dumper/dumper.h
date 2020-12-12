#pragma once
#include "compiler/ast/block.h"
#include "compiler/ast/exp.h"
#include "compiler/ast/stat.h"
#include <cstdio>

void DumpWithIndent(const String& msg, int indent, FILE* fp);
void DumpExp(ExpPtr exp, int indent, FILE* fp);
void DumpStat(StatPtr stst, int indent, FILE* fp);
void DumpBlock(BlockPtr block, int indent, FILE* fp);