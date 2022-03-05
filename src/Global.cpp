#include "Global.h"

void ReadUntilEndOfLine(){
    // Read until the end of the line
    int NewToken = yylex();

    // std::string YYTextString = static_cast<std::string>(yytext);

    // int PlusPos = YYTextString.find('+');
    // std::string Register = YYTextString.substr(1, PlusPos-1);
    // std::cout << "\nRegister: " << Register << std::endl;
    // std::string Literal = YYTextString.substr(PlusPos+1,YYTextString.size()-1);
    // int LiteralNum = std::stoi(Literal);
    // std::cout << "\nLiteral: " << LiteralNum << std::endl;
    while(NewToken != NEW_LINE){
        NewToken = yylex();
    }
}

void ErrorMessage(std::string Message){
    std::cout << "ERROR: " << Message << " Line: " << yylineno << std::endl;
    exit(1);
}

void TooManyArgumentsCheck(){
    int NewToken = yylex();
    if(NewToken != NEW_LINE && NewToken != COMMENT){
        ErrorMessage("Too many arguments!");
    }
    else if(NewToken == COMMENT){
        ReadUntilEndOfLine();
    }
}

int ReadHexLiteral(char* yytext){
    std::string StrYYtext = static_cast<std::string>(yytext);
    std::string SecondArgument = StrYYtext.substr(2, StrYYtext.size()-1);
    
    int Literal;   
    std::stringstream ss;
    ss << std::hex << SecondArgument;
    ss >> Literal;

    return Literal;
}

int ReadSignHexLiteral(char* yytext){
    std::string StrYYtext = static_cast<std::string>(yytext);
    std::string SecondArgument = StrYYtext.substr(3, StrYYtext.size()-1);
    
    int Literal;   
    std::stringstream ss;
    ss << std::hex << SecondArgument;
    ss >> Literal;

    return Literal;
}


int ReadHexLiteral(std::string Str){
    std::string StrYYtext = Str;
    std::string SecondArgument = StrYYtext.substr(2, StrYYtext.size()-1);
    
    int Literal;   
    std::stringstream ss;
    ss << std::hex << SecondArgument;
    ss >> Literal;

    return Literal;
}

int ReadSignHexLiteral(std::string Str){
    std::string StrYYtext = Str;
    std::string SecondArgument = StrYYtext.substr(3, StrYYtext.size()-1);
    
    int Literal;   
    std::stringstream ss;
    ss << std::hex << SecondArgument;
    ss >> Literal;

    return Literal;
}

void CheckRegisterValidity(int RegIndex){
    if(RegIndex < 0 || RegIndex > 7){
        ErrorMessage("Invalid Register!");
    }
}
