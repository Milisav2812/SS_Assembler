#pragma once

#include "Global.h"

class SectionData{
public:

    SectionData(std::string Name, std::string Out):SectionName(Name), OutputFile(Out), Address(0), ListOfDataBytes() {}
    SectionData(std::string Name, std::string Out, int StartingAddress):SectionName(Name), OutputFile(Out), Address(StartingAddress), ListOfDataBytes() {}

    void PrintData();
    void SetAddress(int Value);
    void AddData(int Elem);

    bool CheckName(std::string Name) const;

private:
    std::string SectionName;
    int Address;
    std::list<int> ListOfDataBytes; // List of data items in each section
    static bool PrintFirstLine;
    std::string OutputFile;
};
