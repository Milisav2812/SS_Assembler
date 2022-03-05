#include "Global.h"
#include "Second_Pass.h"

int LocationCounter = 0;
std::string CurrentSection = "Default";
std::vector<std::string> ListOfArguments;
int NumberOfArguments = 0;
SectionData* CurrentSectionData;
RelocationTable* CurrentRelTable;


void ReadAllArguments_Symbols(int& NewToken){
    
    // Colect all arguments
    while(NewToken != NEW_LINE && NewToken != COMMENT){
        std::string NewArgument = static_cast<std::string>(yytext);
        ListOfArguments.push_back(NewArgument);
        NumberOfArguments++;
        NewToken = yylex();
    }

    if(NewToken == COMMENT){
        ReadUntilEndOfLine();
    }
}

void ResetArgumentList(){
    ListOfArguments.clear();
    NumberOfArguments=0;
}

int GetRegisterIndex(char* Register){
    std::string RegName = static_cast<std::string>(Register);
    //std::cout << "String: " << RegName;
    int RegIndex = static_cast<int>(RegName.at(RegName.size()-1) - '0');  
    //std::cout << "\nReg Index: " << RegIndex << std::endl;
    return RegIndex;
}

int CalculateValueFromBits(int Value, bool High){
    // High = true, bits 4-7
    // High = false, bits 0-3
    int Mask;
    int MaskedValue; 
    int BitValue;
    int Result = 0; 
    // std::cout << "Value: " << Value << " Bits: ";
    for(int i=0; i<4; i++){
        Mask = 1 << i;
        MaskedValue = Value & Mask;
        BitValue = MaskedValue >> i;
        // std::cout << BitValue << " ";
        if(High)
            Result += BitValue * std::pow(2, i+4);
        else
            Result += BitValue * std::pow(2, i);
    }
    // std::cout << "Result: " << Result << std::endl;
    return Result;
}

void WriteData(int Value){

    int SymbolOffsetLow  = Value & 0xFF;  
    int SymbolOffsetHigh  = (Value >> 8) & 0xFF;

    // 1st Byte of Payload - DataLow
    CurrentSectionData->AddData(SymbolOffsetLow);

    // 2nd Byte of Payload - DataHigh
    CurrentSectionData->AddData(SymbolOffsetHigh);
}

int FormRelocationRow(std::string SymbolLabel, SymbolsTable& GenSymTab, RelocationType Type, int Addend){

    int SymbolOffset;

    if(GenSymTab.GetGlobal(SymbolLabel)){
        // Global Symbol
        if(Type == Absolute){
            SymbolOffset = GenSymTab.GetSymbolOffset(SymbolLabel) + Addend;
        }
        else if(Type == PC_Relative){
            SymbolOffset = Addend;
        }
        
        CurrentRelTable->AddNewRelocation(CurrentSection, LocationCounter, Type, GenSymTab.GetSymbolNumber(SymbolLabel), Symbol);
    }
    else{
        // Local Symbol
        SymbolOffset = GenSymTab.GetSymbolOffset(SymbolLabel) + Addend;
        CurrentRelTable->AddNewRelocation(CurrentSection, LocationCounter, Type, GenSymTab.GetSymbolNumber(CurrentSection), Section);
    }

    return SymbolOffset;
}

/* #################################################################################################################### */
void CalculateJumpOperand(int& Token, SymbolsTable& GenSymTab){
    int Instruction = Token; // Save for later
    int HighBits, LowBits;

    // InstrDescr - 1st Byte
    HighBits = CalculateValueFromBits(5, true);
    switch(Instruction){
        case JMP: {
            LowBits = CalculateValueFromBits(0,false);
            CurrentSectionData->AddData(LowBits + HighBits);
            LocationCounter += 1;
        } break;
        case JEQ: {
            LowBits = CalculateValueFromBits(1,false);
            CurrentSectionData->AddData( LowBits+HighBits );
            LocationCounter += 1;
        } break;
        case JNE: {
            LowBits = CalculateValueFromBits(2,false);
            CurrentSectionData->AddData( LowBits+HighBits );
            LocationCounter += 1;
        } break;
        case JGT: {
            LowBits = CalculateValueFromBits(3,false);
            CurrentSectionData->AddData( LowBits+HighBits );
            LocationCounter += 1;
        } break;
    }

    // Read Operand
    int NewToken = yylex();
    switch(NewToken){
        case HEX_LITERAL:
        case LITERAL: {
            // RegsDescr - 2nd Byte
            {
                LowBits = CalculateValueFromBits(15,false); // 1111 Source
                HighBits = CalculateValueFromBits(15, true); // 1111 Destination
                CurrentSectionData->AddData(LowBits + HighBits);
                LocationCounter += 1;
            }

            // AddrMode - 3rd Byte
            {
                LowBits = CalculateValueFromBits(0,false);  // 0000 Addressing Mode - Neposredno
                HighBits = CalculateValueFromBits(0, true); // 0000 Update
                CurrentSectionData->AddData(LowBits + HighBits);
                LocationCounter += 1;
            }

            // Data High + Data Low - 4th and 5th Byte
            {
                int Literal;
                if(NewToken == HEX_LITERAL) 
                    Literal = ReadHexLiteral(yytext);
                else
                    Literal = std::atoi(yytext);

                WriteData(Literal);

                LocationCounter += 2;
            }
        } break;
        case SYMBOL: {
            // RegsDescr - 2nd Byte
            {
                LowBits = CalculateValueFromBits(15,false); // 1111 Source
                HighBits = CalculateValueFromBits(15, true); // 1111 Destination
                CurrentSectionData->AddData(LowBits + HighBits);
                LocationCounter += 1;
            }

            // AddrMode - 3rd Byte
            {
                LowBits = CalculateValueFromBits(0,false);  // 0000 Addressing Mode - Neposredno
                HighBits = CalculateValueFromBits(0, true); // 0000 Update
                CurrentSectionData->AddData(LowBits + HighBits);
                LocationCounter += 1;
            }

            // Data High + Data Low - 4th and 5th Byte
            {
                std::string SymbolLabel = static_cast<std::string>(yytext);

                if(!GenSymTab.CheckIfSymbolExists(SymbolLabel)){
                    ErrorMessage("Symbol does not exist!");
                }

                int SymbolOffset = FormRelocationRow(SymbolLabel, GenSymTab, Absolute, 0);
                WriteData(SymbolOffset);

                LocationCounter += 2;
            }
        } break;        
        case PERCENT_SYMBOL: {
            // RegsDescr - 2nd Byte
            {
                // QUESTION: Da li da koristim PC registar ovde?
                LowBits = CalculateValueFromBits(15,false); // 1111 Source
                HighBits = CalculateValueFromBits(15, true); // 1111 Destination
                CurrentSectionData->AddData(LowBits + HighBits);
                LocationCounter += 1;
            }

            // AddrMode - 3rd Byte
            {
                LowBits = CalculateValueFromBits(5,false);  // 0101 Addressing Mode - Registarsko direktno sa 16bit pomerajem
                HighBits = CalculateValueFromBits(0, true); // 0000 Update
                CurrentSectionData->AddData(LowBits + HighBits);
                LocationCounter += 1;
            }

            // Data High + Data Low - 4th and 5th Byte
            {
                std::string NewString = static_cast<std::string>(yytext);
                std::string SymbolLabel = NewString.substr(1, NewString.size() - 1);

                if(!GenSymTab.CheckIfSymbolExists(SymbolLabel)){
                    ErrorMessage("Symbol does not exist!");
                }

                int SymbolOffset = FormRelocationRow(SymbolLabel, GenSymTab, PC_Relative, -2);
                WriteData(SymbolOffset);

                LocationCounter += 2;
            }
        } break; 
        case ASTERIKS_HEX_LITERAL:
        case ASTERISK_LITERAL: {
            // RegsDescr - 2nd Byte
            {
                LowBits = CalculateValueFromBits(15,false); // 1111 Source
                HighBits = CalculateValueFromBits(15, true); // 1111 Destination
                CurrentSectionData->AddData(LowBits + HighBits);
                LocationCounter += 1;
            }

            // AddrMode - 3rd Byte
            {
                LowBits = CalculateValueFromBits(4,false);  // 0100 Addressing Mode - Memorijsko
                HighBits = CalculateValueFromBits(0, true); // 0000 Update
                CurrentSectionData->AddData(LowBits + HighBits);
                LocationCounter += 1;
            }

            // Data High + Data Low - 4th and 5th Byte
            {
                std::string NewString = static_cast<std::string>(yytext);
                NewString = NewString.substr(1, NewString.size() - 1);
                int Literal;
                
                if(NewToken == ASTERIKS_HEX_LITERAL)
                    Literal = ReadSignHexLiteral(yytext);
                else
                    Literal = std::stoi(NewString);

                WriteData(Literal);

                LocationCounter += 2;
            }
        } break;
        case ASTERISK_SYMBOL: {
            // RegsDescr - 2nd Byte
            {
                LowBits = CalculateValueFromBits(15,false); // 1111 Source
                HighBits = CalculateValueFromBits(15, true); // 1111 Destination
                CurrentSectionData->AddData(LowBits + HighBits);
                LocationCounter += 1;
            }

            // AddrMode - 3rd Byte
            {
                LowBits = CalculateValueFromBits(4,false);  // 0100 Addressing Mode - mEMORIJSKO
                HighBits = CalculateValueFromBits(0, true); // 0000 Update
                CurrentSectionData->AddData(LowBits + HighBits);
                LocationCounter += 1;
            }

            // Data High + Data Low - 4th and 5th Byte
            {
                std::string NewString = static_cast<std::string>(yytext);
                std::string SymbolLabel = NewString.substr(1, NewString.size() - 1);

                if(!GenSymTab.CheckIfSymbolExists(SymbolLabel)){
                    ErrorMessage("Symbol does not exist!");
                }

                int SymbolOffset = FormRelocationRow(SymbolLabel, GenSymTab, Absolute, 0);
                WriteData(SymbolOffset);

                LocationCounter += 2;
            }
        } break; 
        case ASTERISK_REGISTER: {
            // RegsDescr - 2nd Byte
            {
                int RegIndex = GetRegisterIndex(yytext); 
                CheckRegisterValidity(RegIndex);

                LowBits = CalculateValueFromBits(RegIndex,false); // ???? Source - Passed register
                HighBits = CalculateValueFromBits(15, true); // 1111 Destination
                CurrentSectionData->AddData(LowBits + HighBits);
                LocationCounter += 1;
            }

            // AddrMode - 3rd Byte
            {
                LowBits = CalculateValueFromBits(1,false);  // 0001 Addressing Mode - Registarsko direktno
                HighBits = CalculateValueFromBits(0, true); // 0000 Update
                CurrentSectionData->AddData(LowBits + HighBits);
                LocationCounter += 1;
            }
        } break;  
        case ASTERISK_MEM_REG: {
            // RegsDescr - 2nd Byte
            {
                std::string MemReg = static_cast<std::string>(yytext);

                int RegIndex = std::stoi(MemReg.substr(3, MemReg.size()-2)); 
                //std::cout << "Index: " << RegIndex << std::endl;
                CheckRegisterValidity(RegIndex);

                LowBits = CalculateValueFromBits(RegIndex,false); // ???? Source - Passed register
                HighBits = CalculateValueFromBits(15, true); // 1111 Destination
                CurrentSectionData->AddData(LowBits + HighBits);
                LocationCounter += 1;
            }

            // AddrMode - 3rd Byte
            {
                LowBits = CalculateValueFromBits(2,false);  // 0010 Addressing Mode - Registarsko indirektno
                HighBits = CalculateValueFromBits(0, true); // 0000 Update
                CurrentSectionData->AddData(LowBits + HighBits);
                LocationCounter += 1;
            }
        } break;  
        case ASTERISK_MEM_REG_HEX_LITERAL:      
        case ASTERISK_MEM_REG_LITERAL: {
            std::string PassedString = static_cast<std::string>(yytext);
            // RegsDescr - 2nd Byte
            {
                int RegIndex = std::stoi(PassedString.substr(3, 3)); 
                // std::cout << "Index: " << RegIndex << std::endl;
                CheckRegisterValidity(RegIndex);

                LowBits = CalculateValueFromBits(RegIndex,false); // ???? Source - Passed Register
                HighBits = CalculateValueFromBits(15, true); // 1111 Destination
                CurrentSectionData->AddData(LowBits + HighBits);
                LocationCounter += 1;
            }

            // AddrMode - 3rd Byte
            {
                LowBits = CalculateValueFromBits(5,false);  // 0011 Addressing Mode - Registarsko indirektno sa 16bit pomerajem
                HighBits = CalculateValueFromBits(0, true); // 0000 Update
                CurrentSectionData->AddData(LowBits + HighBits);
                LocationCounter += 1;
            }

            // Data High + Data Low - 4th and 5th Byte
            {
                int PlusIndex = PassedString.find_first_of('+');
                int BracketIndex = PassedString.find_first_of(']');

                std::string NumberString = PassedString.substr(PlusIndex+1, BracketIndex-1);
                // std::cout << "String: " << NumberString << std::endl;
                int Literal;

                if(NewToken == ASTERISK_MEM_REG_HEX_LITERAL){
                    Literal = ReadHexLiteral(NumberString);
                }
                else
                {
                    Literal = std::stoi(NumberString);
                }

                // std::cout << "Literal: " << Literal << std::endl;

                WriteData(Literal);

                LocationCounter += 2;
            }
        } break;
        case ASTERISK_MEM_REG_SYMBOL: {            
            std::string PassedString = static_cast<std::string>(yytext);
            // RegsDescr - 2nd Byte
            {
                int RegIndex = std::stoi(PassedString.substr(3, 3)); 
                // std::cout << "Index: " << RegIndex << std::endl;
                CheckRegisterValidity(RegIndex);

                LowBits = CalculateValueFromBits(RegIndex,false); // ???? Source - Passed Register
                HighBits = CalculateValueFromBits(15, true); // 1111 Destination
                CurrentSectionData->AddData(LowBits + HighBits);
                LocationCounter += 1;
            }

            // AddrMode - 3rd Byte
            {
                LowBits = CalculateValueFromBits(5,false);  // 0011 Addressing Mode - Registarsko indirektno sa 16bit pomerajem
                HighBits = CalculateValueFromBits(0, true); // 0000 Update
                CurrentSectionData->AddData(LowBits + HighBits);
                LocationCounter += 1;
            }

            // Data High + Data Low - 4th and 5th Byte
            {
                int IndPlus = PassedString.find_first_of('+') + 1;
                int IndBracket = PassedString.find_first_of(']') - 1;
                
                std::string SymbolLabel = PassedString.substr(IndPlus, IndBracket);
                SymbolLabel = SymbolLabel.substr(0,SymbolLabel.size()-1);

                if(!GenSymTab.CheckIfSymbolExists(SymbolLabel)){
                    ErrorMessage("Symbol does not exist!");
                }
                
                int SymbolOffset = FormRelocationRow(SymbolLabel, GenSymTab, Absolute, 0);
                WriteData(SymbolOffset);

                LocationCounter += 2;
            }
        } break; 
        default:{
            ErrorMessage("Incorrect operand for a jump instruction!");
        }
    }
}

/* #################################################################################################################### */
void CalculateLDRSTRCallOperand(int Operation, int RegD, SymbolsTable& GenSymTab){

    int HighBits, LowBits;

    // InstrDescr - 1st Byte
    {
        switch(Operation){
            case LDR: {
                HighBits = CalculateValueFromBits(10, true); // 1010
                LowBits = CalculateValueFromBits(0, false); // 0000
            } break;
            case STR: {
                HighBits = CalculateValueFromBits(11, true); // 1011
                LowBits = CalculateValueFromBits(0, false); // 0000
            } break;
            case CALL: {
                HighBits = CalculateValueFromBits(3, true); // 0011
                LowBits = CalculateValueFromBits(0, false); // 0000
            } break;
            default: ErrorMessage("LDR STR Error");
        }

        CurrentSectionData->AddData(LowBits + HighBits);
        LocationCounter += 1;
    }

    int NewToken = yylex();
    switch(NewToken){
        case DOLLAR_HEX_LITERAL:
        case DOLLAR_LITERAL: {
            // RegsDescr - 2nd Byte
            {
                LowBits = CalculateValueFromBits(15,false); // 1111 Source
                HighBits = CalculateValueFromBits(RegD, true); // ???? Destination
                CurrentSectionData->AddData(LowBits + HighBits);
                LocationCounter += 1;
            }

            // AddrMode - 3rd Byte
            {
                LowBits = CalculateValueFromBits(0,false);  // 0000 Addressing Mode - Neposredno
                HighBits = CalculateValueFromBits(0, true); // 0000 Update
                CurrentSectionData->AddData(LowBits + HighBits);
                LocationCounter += 1;
            }

            // Data High + Data Low - 4th and 5th Byte
            {

                std::string NewString = static_cast<std::string>(yytext);

                int Literal;
                if(NewToken == DOLLAR_HEX_LITERAL)
                    Literal = ReadSignHexLiteral(NewString);
                else
                    Literal = std::stoi(NewString.substr(1, NewString.size() - 1));

                WriteData(Literal);

                LocationCounter += 2;
            }
        } break;
        case DOLLAR_SYMBOL: {
            // RegsDescr - 2nd Byte
            {
                LowBits = CalculateValueFromBits(15,false); // 1111 Source
                HighBits = CalculateValueFromBits(RegD, true); // ???? Destination
                CurrentSectionData->AddData(LowBits + HighBits);
                LocationCounter += 1;
            }

            // AddrMode - 3rd Byte
            {
                LowBits = CalculateValueFromBits(0,false);  // 0000 Addressing Mode - Neposredno
                HighBits = CalculateValueFromBits(0, true); // 0000 Update
                CurrentSectionData->AddData(LowBits + HighBits);
                LocationCounter += 1;
            }

            // Data High + Data Low - 4th and 5th Byte
            {
                std::string NewString = static_cast<std::string>(yytext);
                std::string SymbolLabel = NewString.substr(1, NewString.size() - 1);
                
                if(!GenSymTab.CheckIfSymbolExists(SymbolLabel)){
                    ErrorMessage("Symbol does not exist!");
                }

                int SymbolOffset = FormRelocationRow(SymbolLabel, GenSymTab, Absolute, 0);
                WriteData(SymbolOffset);

                LocationCounter += 2;
            }
        } break;   
        case HEX_LITERAL:     
        case LITERAL: {
            // RegsDescr - 2nd Byte
            {
                LowBits = CalculateValueFromBits(15,false); // 1111 Source
                HighBits = CalculateValueFromBits(RegD, true); // ???? Destination
                CurrentSectionData->AddData(LowBits + HighBits);
                LocationCounter += 1;
            }

            // AddrMode - 3rd Byte
            {
                LowBits = CalculateValueFromBits(4,false);  // 0100 Addressing Mode - Memorijsko
                HighBits = CalculateValueFromBits(0, true); // 0000 Update
                CurrentSectionData->AddData(LowBits + HighBits);
                LocationCounter += 1;
            }

            // Data High + Data Low - 4th and 5th Byte
            {
                int Literal;
                
                if(NewToken == HEX_LITERAL)
                    Literal = ReadHexLiteral(yytext);
                else
                    Literal = std::atoi(yytext);

                WriteData(Literal);

                LocationCounter += 2;
            }
        } break; 
        case SYMBOL: {
            // RegsDescr - 2nd Byte
            {
                LowBits = CalculateValueFromBits(15,false); // 1111 Source
                HighBits = CalculateValueFromBits(RegD, true); // ???? Destination
                CurrentSectionData->AddData(LowBits + HighBits);
                LocationCounter += 1;
            }

            // AddrMode - 3rd Byte
            {
                LowBits = CalculateValueFromBits(4,false);  // 0100 Addressing Mode - Memorijsko
                HighBits = CalculateValueFromBits(0, true); // 0000 Update
                CurrentSectionData->AddData(LowBits + HighBits);
                LocationCounter += 1;
            }

            // Data High + Data Low - 4th and 5th Byte
            {
                std::string SymbolLabel = static_cast<std::string>(yytext);

                if(!GenSymTab.CheckIfSymbolExists(SymbolLabel)){
                    ErrorMessage("Symbol does not exist!");
                }

                int SymbolOffset = FormRelocationRow(SymbolLabel, GenSymTab, Absolute, 0);
                WriteData(SymbolOffset);

                LocationCounter += 2;
            }
        } break;
        case PERCENT_SYMBOL: {
            // RegsDescr - 2nd Byte
            {
                // QUESTION: Da li ovde ide PC registar?
                LowBits = CalculateValueFromBits(15,false); // 1111 
                HighBits = CalculateValueFromBits(RegD, true); // ???? Destination
                CurrentSectionData->AddData(LowBits + HighBits);
                LocationCounter += 1;
            }

            // AddrMode - 3rd Byte
            {
                // QUESTION: Adresiranje?
                LowBits = CalculateValueFromBits(4,false);  // 0100 Addressing Mode - Memorijsko
                HighBits = CalculateValueFromBits(0, true); // 0000 Update
                CurrentSectionData->AddData(LowBits + HighBits);
                LocationCounter += 1;
            }

            // Data High + Data Low - 4th and 5th Byte
            {
                std::string NewString = static_cast<std::string>(yytext);
                std::string SymbolLabel = NewString.substr(1, NewString.size() - 1);

                if(!GenSymTab.CheckIfSymbolExists(SymbolLabel)){
                    ErrorMessage("Symbol does not exist!");
                }

                int SymbolOffset = FormRelocationRow(SymbolLabel, GenSymTab, PC_Relative, -2);
                WriteData(SymbolOffset);

                LocationCounter += 2;
            }
        } break; 
        case REGISTER: {
            // RegsDescr - 2nd Byte
            {
                int RegIndex = GetRegisterIndex(yytext); 
                CheckRegisterValidity(RegIndex);

                LowBits = CalculateValueFromBits(RegIndex,false); // ???? Source - Passed register
                HighBits = CalculateValueFromBits(RegD, true); // ???? Destination
                CurrentSectionData->AddData(LowBits + HighBits);
                LocationCounter += 1;
            }

            // AddrMode - 3rd Byte
            {
                LowBits = CalculateValueFromBits(1,false);  // 0001 Addressing Mode - Registarsko direktno
                HighBits = CalculateValueFromBits(0, true); // 0000 Update
                CurrentSectionData->AddData(LowBits + HighBits);
                LocationCounter += 1;
            }
        } break;
        case MEM_REG: {
            // RegsDescr - 2nd Byte
            {
                std::string MemReg = static_cast<std::string>(yytext);

                int RegIndex = std::stoi(MemReg.substr(2, 2)); 
                CheckRegisterValidity(RegIndex);

                LowBits = CalculateValueFromBits(RegIndex,false); // ???? Source - Passed register
                HighBits = CalculateValueFromBits(RegD, true); // ???? Destination
                CurrentSectionData->AddData(LowBits + HighBits);
                LocationCounter += 1;
            }

            // AddrMode - 3rd Byte
            {
                LowBits = CalculateValueFromBits(2,false);  // 0010 Addressing Mode - Registarsko indirektno
                HighBits = CalculateValueFromBits(0, true); // 0000 Update
                CurrentSectionData->AddData(LowBits + HighBits);
                LocationCounter += 1;
            }
        } break;  
        case MEM_REG_HEX_LITERAL:
        case MEM_REG_LITERAL: {
            std::string PassedString = "*" + static_cast<std::string>(yytext);
            // RegsDescr - 2nd Byte
            {
                int RegIndex = std::stoi(PassedString.substr(3, 3)); 
                // std::cout << "Index: " << RegIndex << std::endl;
                CheckRegisterValidity(RegIndex);

                LowBits = CalculateValueFromBits(RegIndex,false); // ???? Source - Passed Register
                HighBits = CalculateValueFromBits(RegD, true); // ???? Destination
                CurrentSectionData->AddData(LowBits + HighBits);
                LocationCounter += 1;
            }

            // AddrMode - 3rd Byte
            {
                LowBits = CalculateValueFromBits(5,false);  // 0011 Addressing Mode - Registarsko indirektno sa 16bit pomerajem
                HighBits = CalculateValueFromBits(0, true); // 0000 Update
                CurrentSectionData->AddData(LowBits + HighBits);
                LocationCounter += 1;
            }

            // Data High + Data Low - 4th and 5th Byte
            {
                int PlusIndex = PassedString.find_first_of('+');
                int BracketIndex = PassedString.find_first_of(']');

                std::string NumberString = PassedString.substr(PlusIndex+1, BracketIndex-1);

                int Literal;
                
                if(NewToken == MEM_REG_HEX_LITERAL)
                    Literal = ReadHexLiteral(NumberString);
                else
                    Literal = std::stoi(NumberString);
                // std::cout << "Literal: " << Literal << std::endl;

                WriteData(Literal);

                LocationCounter += 2;
            }
        } break;        
        case MEM_REG_SYMBOL: {
            std::string PassedString = "*" + static_cast<std::string>(yytext);
            // RegsDescr - 2nd Byte
            {
                int RegIndex = std::stoi(PassedString.substr(3, 3)); 
                // std::cout << "Index: " << RegIndex << std::endl;
                CheckRegisterValidity(RegIndex);

                LowBits = CalculateValueFromBits(RegIndex,false); // ???? Source - Passed Register
                HighBits = CalculateValueFromBits(RegD, true); // ???? Destination
                CurrentSectionData->AddData(LowBits + HighBits);
                LocationCounter += 1;
            }

            // AddrMode - 3rd Byte
            {
                LowBits = CalculateValueFromBits(5,false);  // 0011 Addressing Mode - Registarsko indirektno sa 16bit pomerajem
                HighBits = CalculateValueFromBits(0, true); // 0000 Update
                CurrentSectionData->AddData(LowBits + HighBits);
                LocationCounter += 1;
            }

            // Data High + Data Low - 4th and 5th Byte
            {
                int IndPlus = PassedString.find_first_of('+') + 1;
                int IndBracket = PassedString.find_first_of(']') - 1;
                
                std::string SymbolLabel = PassedString.substr(IndPlus, IndBracket);
                SymbolLabel = SymbolLabel.substr(0,SymbolLabel.size()-1);

                if(!GenSymTab.CheckIfSymbolExists(SymbolLabel)){
                    ErrorMessage("Symbol does not exist!");
                }
                
                int SymbolOffset = FormRelocationRow(SymbolLabel, GenSymTab, Absolute, 0);
                WriteData(SymbolOffset);

                LocationCounter += 2;
            }
        } break;
        default:{
            ErrorMessage("Incorrect operand!");
        }
    }
}


/* #################################################################################################################### */
void SecondPass(SymbolsTable& GenSymTab, std::list<RelocationTable*>& RelTables, std::list<SectionData*>& Sections, std::string OutputFile){

    // std::cout << "--------------------------------- SECOND PASS ------------------------------------" << std::endl;

    CurrentSectionData = new SectionData("Default", OutputFile);
    CurrentRelTable = new RelocationTable("Default", OutputFile);

    Sections.push_back(CurrentSectionData);
    RelTables.push_back(CurrentRelTable);

    int Token = yylex();
    // std::cout << "Second Pass First Token: " << Token << " Symbol: " << yytext << std::endl;
    while(Token != END){
        switch(Token){
            // DIRECTIVES
            case GLOBAL: {

                int NewToken = yylex();
                if(NewToken != SYMBOL){
                    ErrorMessage("GLOBAL must have at least one symbol defined!");
                }

                // Colect all arguments
                while(NewToken != NEW_LINE && NewToken != COMMENT){
                    std::string NewArgument = static_cast<std::string>(yytext);
                    if(!GenSymTab.CheckIfSymbolExists(NewArgument)){
                        ErrorMessage("GLOBAL: Symbol " + NewArgument + " is not defined!");
                    }
                    GenSymTab.ChangeGlobality(NewArgument);
                    NewToken = yylex();
                }

                if(NewToken == COMMENT){
                    ReadUntilEndOfLine();
                }                
            } break;
            case SECTION: {
                // Read Agrument
                int NewToken = yylex();
                if(NewToken != SYMBOL){
                    ErrorMessage("Invalid argument for SECTION!");
                }

                // Reset Location Counter at the beginning of every section
                LocationCounter = 0;

                std::string NewSectionName = static_cast<std::string>(yytext);
                SectionData* NewSectionData = new SectionData(NewSectionName, OutputFile);
                CurrentSectionData = NewSectionData;
                Sections.push_back(NewSectionData);

                RelocationTable* NewRelocationTable = new RelocationTable(NewSectionName, OutputFile);
                CurrentRelTable = NewRelocationTable;
                RelTables.push_back(NewRelocationTable);

                CurrentSection = NewSectionName;

                TooManyArgumentsCheck();
            } break;
            case EXTERN: {

                int NewToken = yylex();
                if(NewToken != SYMBOL){
                    ErrorMessage("EXTERN must have at least one symbol defined!");
                }

                // Colect all arguments
                while(NewToken != NEW_LINE && NewToken != COMMENT){
                    std::string NewArgument = static_cast<std::string>(yytext);
                    if(GenSymTab.CheckIfSymbolExists(NewArgument)){
                        ErrorMessage("EXTERN: Symbol " + NewArgument + " is already defined and cannot be declared as external!");
                    }
                    GenSymTab.AddNewElement(NewArgument, "UND", 0, true, true);
                    NewToken = yylex();
                }

                if(NewToken == COMMENT){
                    ReadUntilEndOfLine();
                }  
            } break;
            case WORD: {
                int NewToken = yylex();
                if(NewToken != SYMBOL && 
                    NewToken != LITERAL && 
                    NewToken != NEGATIVE && 
                    NewToken != HEX_LITERAL && 
                    NewToken != NEGATIVE_HEX_LITERAL){
                    ErrorMessage("Invalid argument for WORD!");
                }

                while(NewToken != NEW_LINE && NewToken != COMMENT){
                    switch (NewToken)
                    {
                        case HEX_LITERAL: {
                            int Literal = ReadHexLiteral(yytext);

                            WriteData(Literal);

                            LocationCounter += 2;
                        } break;
                        case NEGATIVE_HEX_LITERAL: {
                            int Literal = -ReadSignHexLiteral(yytext);

                            WriteData(Literal);

                            LocationCounter += 2;
                        } break;
                        case NEGATIVE:
                        case LITERAL: {
                            int Literal = std::atoi(yytext);

                            WriteData(Literal);

                            LocationCounter += 2;
                        } break;
                        case SYMBOL: {
                            std::string SymbolLabel = static_cast<std::string>(yytext);
                            if(GenSymTab.CheckIfSymbolExists(SymbolLabel)){

                                int SymbolOffset = FormRelocationRow(SymbolLabel, GenSymTab, Absolute, 0);
                                WriteData(SymbolOffset);

                                LocationCounter += 2;
                            }else{
                                ErrorMessage("WORD: Symbol is not defined!");
                            }
                        } break;
                        
                        default:{
                            ErrorMessage("Invalid argument for WORD!");
                        }
                    }
                    NewToken = yylex();
                }


            } break;
            case SKIP: {
                int NewToken = yylex();
                if(NewToken != LITERAL && NewToken != HEX_LITERAL){
                    ErrorMessage("Invalid argument for SKIP!");
                }
                int Number;

                if(NewToken == HEX_LITERAL){
                    Number = ReadHexLiteral(yytext);
                }else{
                    Number = atoi(yytext);
                }
                
                for(int i=0; i<Number; i++){
                    CurrentSectionData->AddData(0);
                }

                LocationCounter += Number;
                TooManyArgumentsCheck();
            } break;
            case EQU: {
                // Do nothing
                ReadUntilEndOfLine();
            } break;

            // INSTRUCTIONS
            case HALT: {
                CurrentSectionData->AddData(0); // 0000 0000
                LocationCounter += 1;
                TooManyArgumentsCheck();
            } break;
            case INT: {
                int NewToken = yylex();
                if(NewToken != REGISTER){
                    ErrorMessage("Invalid argument for INT!");
                }

                int RegIndex = GetRegisterIndex(yytext);

                CheckRegisterValidity(RegIndex);

                uint8_t LowBits = CalculateValueFromBits(15, false);
                uint8_t HighBits = CalculateValueFromBits(RegIndex,true);

                CurrentSectionData->AddData(16); // 0001 0000
                CurrentSectionData->AddData( LowBits+HighBits ); // DDDD 1111
                

                LocationCounter += 2;
                TooManyArgumentsCheck();
            } break;
            case IRET: {
                CurrentSectionData->AddData(32); // 0010 0000

                LocationCounter += 1;
                TooManyArgumentsCheck();
            } break;
            case CALL: {
                // Read Call Argument
                CalculateLDRSTRCallOperand(Token, 15, GenSymTab);

                TooManyArgumentsCheck();
            } break;
            case RET: {
                CurrentSectionData->AddData(64); // 0100 0000

                LocationCounter += 1;
                TooManyArgumentsCheck();
            } break;
            
            case JMP: 
            case JEQ: 
            case JNE: 
            case JGT: {
                CalculateJumpOperand(Token, GenSymTab);
                TooManyArgumentsCheck();
            } break;

            case NOT: {

                // InstrDescr - 1st Byte
                {
                    uint8_t InstructionHigh = CalculateValueFromBits(8, true); 
                    uint8_t InstructionLow = CalculateValueFromBits(0, false);

                    CurrentSectionData->AddData(InstructionHigh + InstructionLow); 

                    LocationCounter += 1;
                }

                // RegsDescr - 2nd Byte
                {
                    // First Argument
                    int NewToken = yylex();
                    if(NewToken != REGISTER){
                        ErrorMessage("Invalid FIRST argument!");
                    }

                    int RegIndex1 = GetRegisterIndex(yytext);
                    CheckRegisterValidity(RegIndex1);

                    uint8_t LowBits = CalculateValueFromBits(15, false);
                    uint8_t HighBits = CalculateValueFromBits(RegIndex1,true);
                    
                    CurrentSectionData->AddData(LowBits + HighBits);

                    LocationCounter += 1;
                }

                TooManyArgumentsCheck();
                
            } break;

            case AND:  
            case OR:  
            case XOR:  
            case TEST:
            case SHL:
            case SHR: 
            case ADD: 
            case SUB:
            case MUL:
            case DIV: 
            case CMP:
            case XCHG: {
                // Save Operation for later
                int Operation = Token;

                // InstrDescr - 1st Byte
                {
                    // Calculate Operation Bits
                    uint8_t InstructionHigh;
                    uint8_t InstructionLow;
                    switch(Operation){
                        case AND: { 
                            InstructionHigh = CalculateValueFromBits(8, true); // AND OR XOR TEST
                            InstructionLow = CalculateValueFromBits(1, false); // 0001
                        } break;
                        case OR: { 
                            InstructionHigh = CalculateValueFromBits(8, true); // AND OR XOR TEST
                            InstructionLow = CalculateValueFromBits(2, false); // 0010
                        } break;
                        case XOR: { 
                            InstructionHigh = CalculateValueFromBits(8, true); // AND OR XOR TEST
                            InstructionLow = CalculateValueFromBits(3, false); // 0011
                        } break;
                        case TEST: { 
                            InstructionHigh = CalculateValueFromBits(8, true); // AND OR XOR TEST
                            InstructionLow = CalculateValueFromBits(4, false); // 0100
                        } break;
                    // --------------------------------------------------------------------------------------
                        case SHL: {
                            InstructionHigh = CalculateValueFromBits(9, true); // SHL and SHR
                            InstructionLow = CalculateValueFromBits(0, false);
                        } break;
                        case SHR: {
                            InstructionHigh = CalculateValueFromBits(9, true); // SHL and SHR
                            InstructionLow = CalculateValueFromBits(1, false);
                        } break;
                    // --------------------------------------------------------------------------------------
                        case ADD: { 
                            InstructionHigh = CalculateValueFromBits(7, true); // ADD SUB MUL DIV CMP
                            InstructionLow = CalculateValueFromBits(0, false); // 0000
                        } break;
                        case SUB: {
                            InstructionHigh = CalculateValueFromBits(7, true); // ADD SUB MUL DIV CMP
                            InstructionLow = CalculateValueFromBits(1, false); // 0001
                        } break;
                        case MUL: {
                            InstructionHigh = CalculateValueFromBits(7, true); // ADD SUB MUL DIV CMP
                            InstructionLow = CalculateValueFromBits(2, false); // 0010
                        } break;
                        case DIV: {
                            InstructionHigh = CalculateValueFromBits(7, true); // ADD SUB MUL DIV CMP
                            InstructionLow = CalculateValueFromBits(3, false); // 0011
                        } break;
                        case CMP: {
                            InstructionHigh = CalculateValueFromBits(7, true); // ADD SUB MUL DIV CMP
                            InstructionLow = CalculateValueFromBits(4, false); // 0100
                        } break;

                        case XCHG: {
                            InstructionHigh = CalculateValueFromBits(6, true); // XHCG
                            InstructionLow = CalculateValueFromBits(0, false);
                        } break;
                    }

                    CurrentSectionData->AddData(InstructionHigh + InstructionLow); 

                    LocationCounter += 1;
                }

                // RegsDescr - 2nd Byte
                {
                    // First Argument
                    int NewToken = yylex();
                    if(NewToken != REGISTER){
                        ErrorMessage("Invalid FIRST argument!");
                    }

                    int RegIndex1 = GetRegisterIndex(yytext);
                    CheckRegisterValidity(RegIndex1);

                    // Second Argument
                    NewToken = yylex();
                    if(NewToken != REGISTER){
                        ErrorMessage("Invalid SECOND argument!");
                    }

                    int RegIndex2 = GetRegisterIndex(yytext);
                    CheckRegisterValidity(RegIndex2);

                    uint8_t LowBits = CalculateValueFromBits(RegIndex2, false);
                    uint8_t HighBits = CalculateValueFromBits(RegIndex1,true);
                    
                    CurrentSectionData->AddData(LowBits + HighBits);

                    LocationCounter += 1;
                }

                TooManyArgumentsCheck();
            } break;

            case PUSH: 
            case POP: { 
                int LowBits, HighBits;
                // InstrDescr - 1st Byte
                {
                    switch(Token){
                        case PUSH: {
                            HighBits = CalculateValueFromBits(12, true); // 1100
                            LowBits = CalculateValueFromBits(0, false); // 0000
                        } break;
                        case POP: {
                            HighBits = CalculateValueFromBits(13, true); // 1101
                            LowBits = CalculateValueFromBits(0, false); // 0000
                        } break;
                        default: ErrorMessage("LDR STR Error");
                    }

                    CurrentSectionData->AddData(LowBits + HighBits);
                    LocationCounter += 1;
                }

                int NewToken = yylex();
                if(NewToken != REGISTER){
                    ErrorMessage("Invalid argument!");
                }
                int RegD = GetRegisterIndex(yytext);
                CheckRegisterValidity(RegD);
                // RegsDescr - 2nd Byte
                {
                    LowBits = CalculateValueFromBits(15,false); // 1111 Source
                    HighBits = CalculateValueFromBits(RegD, true); // ???? Destination
                    CurrentSectionData->AddData(LowBits + HighBits);
                    LocationCounter += 1;
                }
                TooManyArgumentsCheck();
                } break;

            case LDR:
            case STR: {
                int Operation = Token;
                int RegD = yylex();
                if(RegD != REGISTER){
                    ErrorMessage("Invalid first argument!");
                }
                int RegIndex = GetRegisterIndex(yytext);
                CheckRegisterValidity(RegIndex);
                CalculateLDRSTRCallOperand(Operation, RegIndex, GenSymTab);
                TooManyArgumentsCheck();
            } break;

            // OTHER
            case LABEL: {
                // Do Nothing
            } break;
             
            case NEW_LINE: {
                // Do Nothing
            } break;            
            case COMMENT: {
                ReadUntilEndOfLine();
            } break;
            default:
            {
                ErrorMessage("Syntax Error!");
            }
        }
        Token = yylex();
    }
}