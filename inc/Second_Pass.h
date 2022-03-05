#pragma once

#include <stdio.h>

#include "SymbolsTable.h"
#include "SectionData.h"
#include "RelocationTable.h"

void SecondPass(SymbolsTable& GenSymTab, std::list<RelocationTable*>& RelTables, std::list<SectionData*>& Sections, std::string OutputFile);
void ResetArgumentList();
int GetRegisterIndex(char* Register);
int CalculateValueFromBits(int Value, bool High);

void CalculateJumpOperand(int& Token, SymbolsTable& GenSymTab);
void CalculateLDRSTRCallOperand(int Operation, int RegD, SymbolsTable& GenSymTab);

void WriteData(int Value);
int FormRelocationRow(std::string SymbolLabel, SymbolsTable& GenSymTab, RelocationType Type, int Addend);
