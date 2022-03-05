#pragma once

#include "Global.h"

#define SYMBOLS_NUMBER   6
#define SYMBOLS_LABEL    20
#define SYMBOL_SECTION   20
#define SYMBOLS_OFFSET   15
#define SYMBOLS_GLOBAL   15
#define SYMBOLS_EXTERNAL 15

typedef struct{
    int             Number;     // Number in the Table
    std::string     Label;      // Symbol name
    std::string     Section;    // Symbol Section
    int             Offset;     // Offset from the beginning of the Section
    bool            Global;     // True - Global, False - Local
    bool            External;   // Defined by .extern
} TableElement;

class SymbolsTable{
public:

    SymbolsTable(std::string Out): OutputFile(Out){}

    void PrintSymbolTable(); 
    void AddNewElement(std::string Label, std::string Section, int Offset, bool Global, bool External);
    bool CheckIfSymbolExists(std::string SymbolLabel) const;
    void ChangeGlobality(std::string SymbolLabel);

    TableElement& GetSpecificSymbolRow(std::string SymbolLabel);
    int GetSymbolOffset(std::string SymbolLabel);
    bool GetExternal(std::string SymbolLabel);
    bool GetGlobal(std::string SymbolLabel);
    int GetSymbolNumber(std::string SymbolLabel);

private:
    std::list<TableElement> ListOfElements;
    static int CurrentNumber;   // Specifies current number while adding a new element
    std::string OutputFile;

};