#pragma once

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <list>
#include <vector>
#include <math.h>
#include <string.h>

// Flex variables
extern int yylex();
extern void yyrestart(FILE*);
extern int yylineno;
extern char* yytext;
extern FILE* yyin;
extern FILE* yyout;

// Helper functions
void ReadUntilEndOfLine();
void ErrorMessage(std::string Message);
void TooManyArgumentsCheck();
int ReadHexLiteral(char* yytext);
int ReadSignHexLiteral(char* yytext);
int ReadHexLiteral(std::string Str);
int ReadSignHexLiteral(std::string Str);
void CheckRegisterValidity(int RegIndex);

// DIRECTIVES
#define END     0

#define EXTERN  1
#define SECTION 2
#define WORD    3
#define SKIP    4
#define EQU     5
#define GLOBAL  6

// INSTRUCTIONS
#define HALT    7
#define INT     8
#define IRET    9
#define CALL    10
#define RET     11

#define JMP     12
#define JEQ     13
#define JNE     14
#define JGT     15

#define PUSH    16
#define POP     17

#define XCHG    18

#define ADD     19
#define SUB     20
#define MUL     21
#define DIV     22
#define CMP     23

#define NOT     24
#define AND     25
#define OR      26
#define XOR     27
#define TEST    28

#define SHL     29
#define SHR     30

#define LDR     31
#define STR     32

// ARGUMENTS
#define SYMBOL          33
#define DOLLAR_SYMBOL   34
#define PERCENT_SYMBOL  35
#define ASTERISK_SYMBOL 36

#define LITERAL          37
#define DOLLAR_LITERAL   38
#define PERCENT_LITERAL  39
#define ASTERISK_LITERAL 40

#define REGISTER                    41
#define ASTERISK_REGISTER           42
#define MEM_REG                     43
#define MEM_REG_LITERAL             44
#define MEM_REG_SYMBOL              45
#define ASTERISK_MEM_REG            46
#define ASTERISK_MEM_REG_LITERAL    47
#define ASTERISK_MEM_REG_SYMBOL     48

#define LABEL   49

#define NEW_LINE    50
#define BLANK 51
#define COMMENT 52
#define NEGATIVE 53

#define HEX_LITERAL 54
#define DOLLAR_HEX_LITERAL 55
#define PERCENT_HEX_LITERAL 56
#define ASTERIKS_HEX_LITERAL 57
#define NEGATIVE_HEX_LITERAL 58

#define ASTERISK_MEM_REG_HEX_LITERAL 59
#define MEM_REG_HEX_LITERAL 60
