%{

    #include "Global.h"

    bool LeftBracket = false;
    bool RightBracket = false;

%}

%%


\s*.global { return GLOBAL; }
\s*.extern { return EXTERN; }
\s*.section { return SECTION; }
\s*.word { return WORD; }
\s*.skip { return SKIP; }
\s*.equ { return EQU; }

\s*halt { return HALT; }
\s*int { return INT; }
\s*iret { return IRET; }
\s*call { return CALL; }
\s*ret { return RET; }

\s*jmp { return JMP; }
\s*jeq { return JEQ; }
\s*jne { return JNE; }
\s*jgt { return JGT; }

\s*pop { return POP; }
\s*push { return PUSH; }

\s*xchg { return XCHG; }

\s*add { return ADD; }
\s*sub { return SUB; }
\s*mul { return MUL; }
\s*div { return DIV; }
\s*cmp { return CMP; }

\s*not { return NOT; }
\s*and { return AND; }
\s*or { return OR; }
\s*xor { return XOR; }
\s*test { return TEST; }

\s*shl { return SHL; }
\s*shr { return SHR; }
\s*ldr { return LDR; }
\s*str { return STR; }

\s*0[xX][0-9a-fA-F]{1,4}  {return HEX_LITERAL;}
\s*[$]0[xX][0-9a-fA-F]{1,4} {return DOLLAR_HEX_LITERAL;}
\s*[%]0[xX][0-9a-fA-F]{1,4} {return PERCENT_HEX_LITERAL;}
\s*[*]0[xX][0-9a-fA-F]{1,4} {return ASTERIKS_HEX_LITERAL;}
\s*[-]0[xX][0-9a-fA-F]{1,4} {return NEGATIVE_HEX_LITERAL;}

\s*[$][0-9]+ { return DOLLAR_LITERAL; }
\s*[%][0-9]+ { return PERCENT_LITERAL; }
\s*[*][0-9]+ { return ASTERISK_LITERAL; }
\s*[-][0-9]+ { return NEGATIVE; }
\s*[0-9]+ { return LITERAL; }

\s*"*["r[0-7]"+"0[xX][0-9a-fA-F]{1,4}+"]"        { return ASTERISK_MEM_REG_HEX_LITERAL; }
\s*"["r[0-7]"+"0[xX][0-9a-fA-F]{1,4}+"]"         { return MEM_REG_HEX_LITERAL; }

\s*"*["r[0-7]"+"[0-9]+"]"        { return ASTERISK_MEM_REG_LITERAL; }
\s*"*["r[0-7]"+"[a-zA-Z0-9]+"]"  { return ASTERISK_MEM_REG_SYMBOL; }
\s*"*["r[0-7]"]"                 { return ASTERISK_MEM_REG; }

\s*"["r[0-7]"+"[0-9]+"]"        { return MEM_REG_LITERAL; }
\s*"["r[0-7]"+"[a-zA-Z0-9]+"]"  { return MEM_REG_SYMBOL; }
\s*"["r[0-7]"]"                 { return MEM_REG; }

\s*[*]r[0-7]                    { return ASTERISK_REGISTER; }
\s*r[0-7]                       { return REGISTER; }

\s*[$][_a-zA-Z0-9]+ { return DOLLAR_SYMBOL; }
\s*[%][_a-zA-Z0-9]+ { return PERCENT_SYMBOL; }
\s*[*][_a-zA-Z0-9]+ { return ASTERISK_SYMBOL; }
\s*[a-zA-Z][_a-zA-Z0-9]* { return SYMBOL; }

\s*[a-zA-Z][_a-zA-Z0-9]*: { return LABEL; }

[#] { return COMMENT;  }
\t { }
\n { yylineno++; return NEW_LINE; }
.end { return END; }
, { /* DO NOTHING */}
- { return NEGATIVE; }
. { /* DO NOTHING */ }

%%

int yywrap(void) { return 1; }