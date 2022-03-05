#include "SymbolsTable.h"

int SymbolsTable::CurrentNumber = 0;


void SymbolsTable::PrintSymbolTable(){
    // Define Output file
    std::ofstream Output;
    Output.open(OutputFile, std::ios_base::app);

    Output << "##################################### Symbols Table #####################################" << std::endl;
    Output << std::setw(SYMBOLS_NUMBER) << "Number" << std::setw(SYMBOLS_LABEL) << "Label" << std::setw(SYMBOL_SECTION) << "Section" << std::setw(SYMBOLS_OFFSET) << "Offset" << std::setw(SYMBOLS_GLOBAL) << "Global" << std::setw(SYMBOLS_EXTERNAL) << "External" << std::endl;

    for(TableElement& Element: ListOfElements ){
        Output << std::setw(SYMBOLS_NUMBER) << Element.Number << std::setw(SYMBOLS_LABEL) << Element.Label << std::setw(SYMBOL_SECTION) << Element.Section << std::setw(SYMBOLS_OFFSET) << std::dec << Element.Offset;
        
        if(Element.Global)
            Output << std::setw(SYMBOLS_GLOBAL) << "Global";
        else
            Output << std::setw(SYMBOLS_GLOBAL) << "Local";

        if(Element.External)
            Output << std::setw(SYMBOLS_EXTERNAL) << "YES" << std::endl;
        else
            Output << std::setw(SYMBOLS_EXTERNAL) << "NO" << std::endl;
    }

    Output.flush();

    Output.close();
}

void SymbolsTable::AddNewElement(std::string Label, std::string Section, int Offset, bool Global, bool External){ 
    
    TableElement NewElement = {CurrentNumber++, Label, Section, Offset, Global, External};

    ListOfElements.push_back(NewElement);
}

bool SymbolsTable::CheckIfSymbolExists(std::string SymbolLabel) const{
    for(const auto& Elem: ListOfElements){
        if(Elem.Label == SymbolLabel){
            return true;
        }
    }
    return false;
}

TableElement& SymbolsTable::GetSpecificSymbolRow(std::string SymbolLabel){
    static TableElement Temp;
    
    for(auto& Elem: ListOfElements){
        if(Elem.Label == SymbolLabel){
            Temp = Elem;
        }
    }

    return Temp;
}

void SymbolsTable::ChangeGlobality(std::string SymbolLabel){
    for(auto& Elem: ListOfElements){
        if(Elem.Label == SymbolLabel){
            Elem.Global = true;
        }
    }
}

int SymbolsTable::GetSymbolOffset(std::string SymbolLabel){
    int Result = -1;
    for(auto& Elem: ListOfElements){
        if(Elem.Label == SymbolLabel){
            Result = Elem.Offset;
        }
    }
    return Result;
}

bool SymbolsTable::GetExternal(std::string SymbolLabel){
    bool Result = false;
    for(auto& Elem: ListOfElements){
        if(Elem.Label == SymbolLabel){
            Result = Elem.External;
        }
    }
    return Result;
}

bool SymbolsTable::GetGlobal(std::string SymbolLabel){
    bool Result = false;
    for(auto& Elem: ListOfElements){
        if(Elem.Label == SymbolLabel){
            Result = Elem.Global;
        }
    }
    return Result;
}

int SymbolsTable::GetSymbolNumber(std::string SymbolLabel){
    int Result;
    for(auto& Elem: ListOfElements){
        if(Elem.Label == SymbolLabel){
            Result = Elem.Number;
        }
    }
    return Result;
}
