PROGRAM = Assembler

FLEX = flex 
GPP = g++ lex.yy.c

FLAG_INCLUDE = -I inc

Lex_File = src/lex.l

List_Of_Cpp_Files = \
src/First_Pass.cpp \
src/Main.cpp \
src/SymbolsTable.cpp \
src/Second_Pass.cpp \
src/SectionData.cpp \
src/Global.cpp \
src/RelocationTable.cpp

AS:
	$(FLEX) $(Lex_File)
	$(GPP) $(List_Of_Cpp_Files) -o $(PROGRAM) $(FLAG_INCLUDE)
	./$(PROGRAM) -o Output1.txt general.s
	./$(PROGRAM) -o Output2.txt general2.s