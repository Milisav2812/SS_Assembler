#pragma once

#include "Global.h"

#define REL_SECTION         20
#define REL_OFFSET          6
#define REL_TYPE            14
#define REL_SYMBOL_NUMBER   12
#define REL_SYMSEC          16

typedef enum {Absolute, PC_Relative} RelocationType;
typedef enum {Symbol, Section} SymbolSection;

typedef struct{
    std::string     Section;    // Section in which we're currently in
    int             Offset;     // Offset from the beginning of the Section    
    RelocationType  Type;
    int             SymbolNum;  // Symbol Number from the Symbols Table
    SymbolSection   SymSec;
} RelocationRow;

class RelocationTable{
public:
    

    RelocationTable(std::string Name, std::string Out):TableName(Name), OutputFile(Out){}

    void Print();
    void AddNewRelocation(std::string Section, int Offset, RelocationType Type, int SymbolNum, SymbolSection SymSec);

private:
    std::string TableName; 
    std::list<RelocationRow> ListOfRelocations;
    static bool PrintFirstLine;
    std::string OutputFile;
};