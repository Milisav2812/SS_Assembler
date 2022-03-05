#include "Global.h"
#include "First_Pass.h"
#include "Second_Pass.h"
#include "RelocationTable.h"


int main(int argc, char* argv[]){

    std::string InputFileLocation = "tests/";
    std::string InputFile = static_cast<std::string>(argv[3]);
    InputFile = InputFileLocation + InputFile;
    const char* cInputFile = InputFile.c_str();
    char* OutputFile = argv[2];

    SymbolsTable GenSymTab(OutputFile);
    std::list<RelocationTable*> RelTables; 
    std::list<SectionData*> Sections;

    FILE* Input; 
    FILE* Output;

    // std::cout << "Arg1: " << argv[1] << std::endl;
    // std::cout << "Arg2: " << argv[2] << std::endl;
    // std::cout << "Arg3: " << argv[3] << std::endl;
    // std::cout << "ARGC: " << argc << std::endl;



    if(argc == 4 && (strcmp(argv[1], "-o") == 0)){
        

        if(!(Input = fopen(cInputFile, "r"))){
            ErrorMessage("Could not open input file! First Time!");
        }

        if(!(Output = fopen(OutputFile, "w"))){
            ErrorMessage("Could not create output file!");
        }

        yyin = Input;
        yyout = Output;
        yyrestart(Input);

        FirstPass(GenSymTab);

        // Second Pass
        fclose(Input);
        if(!(Input = fopen(cInputFile, "r"))){
            ErrorMessage("Could not open input file! Second Time!");
        }
        yyrestart(Input);
        yylineno=1;

        SecondPass(GenSymTab, RelTables, Sections, OutputFile);

        GenSymTab.PrintSymbolTable();
        for(auto& Element: Sections){
            Element->PrintData();
        }
        for(auto& Element: RelTables){
            Element->Print();
        }

        fclose(Input);
        fclose(Output);
    }else{
        ErrorMessage("Incorrect arguments!");
    }



    return 0;
}