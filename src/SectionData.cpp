#include "SectionData.h"

bool SectionData::PrintFirstLine = true;

void SectionData::SetAddress(int Value){
    Address = Value;
}

void SectionData::PrintData() {
    if(ListOfDataBytes.size() == 0){
        return;
    }

    std::ofstream Output;
    Output.open(OutputFile, std::ios_base::app);

    if(PrintFirstLine){
        Output << std::endl << std::endl;
        Output << "##################################### Data Sections #####################################" << std::endl << std::endl;
        PrintFirstLine = false;
    }

    Output << "# Data: " << SectionName << std::endl;
    Output << std::setw(54) << std::setfill('-') << " " << std::setfill(' ') << std::endl;
    
    Output << std::hex << std::setw(4) << std::setfill('0') << Address << ": ";
    
    int i=0;
    for(const int& Elem: ListOfDataBytes){
        Output << std::hex << std::setw(2) << std::setfill('0') << Elem << " ";

        i++;
        if(i%16==0){
            Output << std::endl;
            Output << std::hex << std::setw(4) << std::setfill('0') << Address + 16*(i/16) << ": ";
        }
    }

    Output << std::endl << std::endl;

    Output.flush();

    Output.close();

}

void SectionData::AddData(int Elem){
    ListOfDataBytes.push_back(Elem);
}

bool SectionData::CheckName(std::string Name) const{
    return SectionName == Name;
}



















