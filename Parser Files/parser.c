#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "lexer.h"
#include "parser.h"

// Global token and parserinfo variable
Token t;
ParserInfo pi;

// Function Prototypes
ParserInfo error(char *msg);
ParserInfo classDecl();
ParserInfo memberDecl();
ParserInfo classVarDecl();
ParserInfo type();
ParserInfo subroutineDecl();
ParserInfo paramList();
ParserInfo subroutineBody();
ParserInfo stmt();
ParserInfo varDeclarStmt();
ParserInfo letStmt();
ParserInfo ifStmt();
ParserInfo whileStmt();
ParserInfo doStmt();
ParserInfo subroutineCall();
ParserInfo expressionList();
ParserInfo returnStmt();
ParserInfo expression();
ParserInfo relationalExpression();
ParserInfo arithmeticExpression();
ParserInfo term();
ParserInfo factor();
ParserInfo operand();


int InitParser (char* file_name) {
	return InitLexer(file_name);
}


ParserInfo Parse () {
	pi.er = none;
	pi = classDecl();
	return pi;
}


ParserInfo classDecl() {

	// Expect class keyword
	t = GetNextToken();
	if ( !strcmp(t.lx, "class") && t.tp == RESWORD )
		; // Be happy :) 
	else {
		pi = error("keyword 'class' expected");
		pi.er = classExpected;
		return pi;
	}

	// Expect class id 
	t = GetNextToken();
	if ( t.tp == ID )
		; // We good
	else {
		pi = error("identifier expected at this position");
		pi.er = idExpected;
		return pi;
	}

	// Expect Open brace
	// **** TURN INTO ITS OWN FUNCTION? ****
	t = GetNextToken();
	if ( t.tp == SYMBOL && !strcmp(t.lx, "{"))
		; // Excellent
	else {
		pi = error("'{' expected at this position.");
		pi.er = openBraceExpected;
		return pi;
	}

	// Expect 0 or more member declarations
	// t = PeekNextToken();

	
	// Expect closing brace
	t = GetNextToken();
	if ( t.tp == SYMBOL && !strcmp(t.lx, "} at this position"))
		; // Excellent
	else {
		pi = error("'}' expected.");
		pi.er = openBraceExpected;
		return pi;
	}
	
	return pi;
}




ParserInfo error(char *msg) {
	ParserInfo pi;
	printf("Error, line %i, close to %s, %s.\n", t.ln, t.lx, msg);
	pi.tk = t;
	return pi;
}

int StopParser () {
	return StopLexer();
}

#ifndef TEST_PARSER
int main (int argc, char **argv) {
	InitParser(argv[1]);	

	if (Parse().er != none) {
		exit(1);
	};
	
	printf("Successfully parsed source file\n");
	return 0;
}
#endif
