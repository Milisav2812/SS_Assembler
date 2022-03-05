#include "Global.h"
#include "First_Pass.h"


int CalculateSizeJumpInstruction(){
    int Result = 0;

    int NewToken = yylex();
    // std::cout << "Token: " << NewToken << " Symbol: " << yytext << std::endl;
    switch(NewToken){
        
        case HEX_LITERAL:
        case LITERAL: {
            // Neposredno
            Result = 5;
        } break;
        case SYMBOL: {
            // Neposredno
            Result = 5;
        } break;        
        case PERCENT_SYMBOL: {
            // Reg Dir sa 16bit pomerajem
            Result = 5;
        } break; 
        case ASTERIKS_HEX_LITERAL:
        case ASTERISK_LITERAL: {
            // Memorijsko
            Result = 5;
        } break;
        case ASTERISK_SYMBOL: {
            // Memorijsko
            Result = 5;
        } break; 
        case ASTERISK_REGISTER: {
            // Reg direktno
            Result = 3;
        } break;  
        case ASTERISK_MEM_REG: {
            // Reg indirektno
            Result = 3;
        } break;        
        case ASTERISK_MEM_REG_HEX_LITERAL:
        case ASTERISK_MEM_REG_LITERAL: {
            // Reg indirektno sa 16bit pomerajem
            Result = 5;
        } break;
        case ASTERISK_MEM_REG_SYMBOL: {            
            // Reg indirektno sa 16bit pomerajem
            Result = 5;
        } break; 
        default:{
            ErrorMessage("Incorrect operand for a jump instruction!");
        }
    }
    return Result;
}
int CalculateSizeInstruction(){
    int Result = 0;

    int NewToken = yylex();
    switch(NewToken){
        case DOLLAR_HEX_LITERAL:
        case DOLLAR_LITERAL: {
            // Neposredno
            Result = 5;
        } break;
        case DOLLAR_SYMBOL: {
            // Neposredno
            Result = 5;
        } break;  
        case HEX_LITERAL:      
        case LITERAL: {
            // Memorijsko
            Result = 5;
        } break; 
        case SYMBOL: {
            // Memorijsko
            Result = 5;
        } break;
        case PERCENT_SYMBOL: {
            // Reg indirektno sa 16bit pomerajem
            Result = 5;
        } break; 
        case REGISTER: {
            // Reg direktno
            Result = 3;
        } break;
        case MEM_REG: {
            // Reg indirektno
            Result = 3;
        } break;  
        case MEM_REG_HEX_LITERAL:
        case MEM_REG_LITERAL: {
            // Reg indirektno sa 16bit pomerajem
            Result = 5;
        } break;        
        case MEM_REG_SYMBOL: {
            // Reg indirektno sa 16bit pomerajem
            Result = 5;
        } break;
        default:{
            ErrorMessage("Incorrect operand!");
        }
    }
    return Result;
}

void FirstPass(SymbolsTable& GenSymTab){

    // std::cout << "--------------------------------- FIRST PASS ------------------------------------" << std::endl;
    
    int LocationCounter = 0;
    std::string CurrentSection = "UND";
    std::string ABSSection = "ABS";
    GenSymTab.AddNewElement(CurrentSection, CurrentSection, 0, false, false);

    int Token = yylex();
    // std::cout << "Token: " << Token << " Symbol: " << yytext << std::endl; 
    while(Token != END){
        switch(Token){
            // DIRECTIVES
            case GLOBAL: {
                ReadUntilEndOfLine();
            } break;
            case SECTION: {

                // Grab next token, which should be the name of the section
                int NewToken = yylex();

                // Check whether the next token is a Symbol(33)
                if(NewToken != SYMBOL){
                    ErrorMessage("Invalid name for a Section! Expecting a Symbol!"); 
                }

                // Check whether section has already been created
                if(GenSymTab.CheckIfSymbolExists(yytext)){
                    ErrorMessage("A symbol with the name " + static_cast<std::string>(yytext) + " already exists!");
                }

                CurrentSection = yytext;
                LocationCounter = 0;
                GenSymTab.AddNewElement(yytext, CurrentSection, LocationCounter, false, false);

                TooManyArgumentsCheck();

            } break;
            case EXTERN: {
                // Do nothing
                ReadUntilEndOfLine();
            } break;
            case WORD: {
                int NewToken = yylex();
                int NumberOfArguments = 0;
                if(NewToken != SYMBOL && NewToken != LITERAL && NewToken != NEGATIVE && NewToken != HEX_LITERAL && NewToken != NEGATIVE_HEX_LITERAL){
                    ErrorMessage("Invalid argument for WORD!");
                }

                while(NewToken != NEW_LINE && NewToken != COMMENT){
                    switch (NewToken)
                    {
                        case HEX_LITERAL:
                        case NEGATIVE_HEX_LITERAL:
                        case NEGATIVE:
                        case LITERAL: 
                        case SYMBOL: {
                            NumberOfArguments++;
                        } break;
                        
                        default:{
                            ErrorMessage("Invalid argument for WORD!");
                        }
                    }
                    NewToken = yylex();
                }

                LocationCounter += NumberOfArguments*2;

                if(NewToken == COMMENT){
                    ReadUntilEndOfLine();
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

                LocationCounter += Number;

                TooManyArgumentsCheck();
            } break;
            case EQU: {
                // First Argument - SYMBOL
                int NewToken = yylex();
                if(NewToken != SYMBOL){
                    ErrorMessage("Invalid first argument for EQU! Symbol expected!");
                }

                std::string FirstArg = static_cast<std::string>(yytext);

                // Second Argument - LITERAL
                NewToken = yylex();
                if(NewToken != LITERAL && 
                    NewToken != NEGATIVE && 
                    NewToken != HEX_LITERAL &&
                    NewToken != NEGATIVE_HEX_LITERAL){
                    ErrorMessage("Invalid second argument for EQU! Literal expected!");
                }

                int SecondArg;
                if(NewToken == HEX_LITERAL){
                    SecondArg = ReadHexLiteral(yytext);
                }
                else if(NewToken == NEGATIVE_HEX_LITERAL){
                    SecondArg = -ReadSignHexLiteral(yytext);
                }
                else{
                    SecondArg = std::atoi(yytext);                
                }

                // QUESTION: Da li je EQU promenljiva global?
                if(GenSymTab.CheckIfSymbolExists(FirstArg)){
                    ErrorMessage("EQU: Symbol already exists!");
                }

                GenSymTab.AddNewElement(FirstArg, ABSSection, SecondArg, true, false);

                TooManyArgumentsCheck();
            } break;

            // INSTRUCTIONS
            case HALT: {
                LocationCounter += 1;
                ReadUntilEndOfLine();
            } break;
            case INT: {
                LocationCounter += 2;
                ReadUntilEndOfLine();
            } break;
            case IRET: {
                LocationCounter += 1;
                ReadUntilEndOfLine();
            } break;
            case CALL: {
                LocationCounter += CalculateSizeInstruction();
                ReadUntilEndOfLine();
            } break;
            case RET: {
                LocationCounter += 1;
                ReadUntilEndOfLine();
            } break;
            
            case JMP: 
            case JEQ: 
            case JNE: 
            case JGT: {
                LocationCounter += CalculateSizeJumpInstruction();
                ReadUntilEndOfLine();
            } break;

            case XCHG: {
                LocationCounter += 2;
                ReadUntilEndOfLine();
            } break;

            case ADD: 
            case SUB: 
            case MUL: 
            case DIV: 
            case CMP: {
                LocationCounter += 2;
                ReadUntilEndOfLine();
            } break;

            case NOT: 
            case AND: 
            case OR:  
            case XOR: 
            case TEST: {
                LocationCounter += 2;
                ReadUntilEndOfLine();
            } break;

            case PUSH: 
            case POP: { 
                LocationCounter += 1;
                ReadUntilEndOfLine();
            } break;

            case LDR: 
            case STR: {
                // Read first Argument
                int NewToken = yylex();
                if(NewToken != REGISTER){
                    ErrorMessage("Invalid FIRST argument!");
                }

                // Read second Argument and calculate length
                LocationCounter += CalculateSizeInstruction();
                ReadUntilEndOfLine();
            } break;

            case SHL:
            case SHR: {
                LocationCounter += 2;
                ReadUntilEndOfLine();
            } break;

            // OTHER
            case LABEL: {
                int LabelSize = static_cast<std::string>(yytext).size();
                std::string Label = static_cast<std::string>(yytext).substr(0,LabelSize-1);

                // Check whether symbol already exists
                if(GenSymTab.CheckIfSymbolExists(Label)){
                    ErrorMessage("Symbol with the name " + Label + " already exists!");
                }

                // Add symbol to table
                GenSymTab.AddNewElement(Label, CurrentSection, LocationCounter, false, false);
            } break;
             
            case NEW_LINE: {
                // Do nothing
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
        // std::cout << "Token: " << Token << " Symbol: " << yytext << std::endl;
    }
}