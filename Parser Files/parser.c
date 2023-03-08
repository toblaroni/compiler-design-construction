#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "lexer.h"
#include "parser.h"


// Function Prototypes
ParserInfo error(char *msg, Token T);
void classDecl();
void memberDecl();
void classVarDecl();
void type();
void subroutineDecl();
void paramList();
void subroutineBody();
void stmt();
void varDeclarStmt();
void letStmt();
void ifStmt();
void whileStmt();
void doStmt();
void subroutineCall();
void expressionList();
void returnStmt();
void expression();
void relationalExpression();
void arithmeticExpression();
void term();
void factor();
void operand();


int InitParser (char* file_name)
{
	return InitLexer(file_name);
}

ParserInfo Parse ()
{
	ParserInfo pi;
	Token t = GetNextToken();

	// Expect class declaration
	if ( !strcmp(t.lx, "class") && t.tp == RESWORD )
		; // Be happy :) 


	pi.er = none;
	return pi;
}


int StopParser ()
{
	return StopLexer();
}

#ifndef TEST_PARSER
int main ()
{

	return 1;
}
#endif
