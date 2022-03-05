#include "RelocationTable.h"

bool RelocationTable::PrintFirstLine = true;

void RelocationTable::AddNewRelocation(std::string Section, int Offset, RelocationType Type, int SymbolNum, SymbolSection SymSec){    
    RelocationRow NewRow {Section, Offset, Type, SymbolNum, SymSec};
    ListOfRelocations.push_back(NewRow);
}

void RelocationTable::Print(){
    if(ListOfRelocations.size() == 0){
        return;
    }

    std::ofstream Output;
    Output.open(OutputFile, std::ios_base::app);

    if(PrintFirstLine){
        Output << std::endl << std::endl;
        Output << "##################################### Relocation Tables #####################################" << std::endl << std::endl;
        PrintFirstLine = false;
    }

    Output << "# Rel." << TableName << std::endl;
    Output << std::setw(REL_OFFSET) << "Offset" << std::setw(REL_SECTION) << "Section" << std::setw(REL_TYPE) << "Type" << std::setw(REL_SYMBOL_NUMBER) << "SymbolNum" << std::setw(REL_SYMSEC) << "Symbol/Section" << std::endl;
    Output << std::setw(REL_OFFSET+REL_SECTION+REL_TYPE+REL_SYMBOL_NUMBER+REL_SYMSEC) << std::setfill('-') << " " << std::setfill(' ') << std::endl;

    for(auto& Element: ListOfRelocations ){
        Output << "  " << std::setw(4) << std::hex << std::setfill('0') << Element.Offset << std::setfill(' ');
        Output << std::setw(REL_SECTION) << Element.Section;
        if(Element.Type == Absolute)
            Output << std::setw(REL_TYPE) << "R_16";
        else if(Element.Type == PC_Relative)
            Output << std::setw(REL_TYPE) << "R_PC16";
        
        Output << std::dec << std::setw(REL_SYMBOL_NUMBER) << Element.SymbolNum;;
        if(Element.SymSec == Symbol)
            Output << std::setw(REL_SYMSEC) << "Symbol";
        else if(Element.SymSec == Section)
            Output << std::setw(REL_SYMSEC) << "Section";

        Output << std::endl;
    }

    Output << std::endl;

    Output.flush();

    Output.close();
}