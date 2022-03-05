#pragma once

#include <stdio.h>

#include "SymbolsTable.h"

void FirstPass(SymbolsTable& GenSymTab);
int CalculateSizeJumpInstruction();
int CalculateSizeInstruction();
