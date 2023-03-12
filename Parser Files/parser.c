#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "lexer.h"
#include "parser.h"

// Global token and parserinfo variable
Token t;
ParserInfo pi;

// Function Prototypes
void printToken(Token t);
ParserInfo error(char *msg, SyntaxErrors e);
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
		return error("keyword 'class' expected", classExpected);
	}

	// Expect class id 
	t = GetNextToken();
	if ( t.tp == ID )
		; // We good
	else {
		return error("identifier expected at this position", idExpected);
	}

	// Expect Open brace
	// **** TURN INTO ITS OWN FUNCTION? ****
	t = GetNextToken();
	if ( t.tp == SYMBOL && !strcmp(t.lx, "{"))
		; // Excellent
	else {
		return error("'{' expected at this position.", openBraceExpected);
	}

	// Expect 0 or more member declarations
	t = PeekNextToken();
	while (strcmp(t.lx, "}") && t.tp != EOFile) {
		pi = memberDecl();

		// If there's an error return pi
		if (pi.er != none)
			return pi;

		t = PeekNextToken();
	}
	
	// Expect closing brace
	t = GetNextToken();
	if ( t.tp == SYMBOL && !strcmp(t.lx, "}"))
		; // Excellent
	else {
		return error("'}' expected.", closeBraceExpected);
	}
	
	return pi;
}


ParserInfo memberDecl() {
	t = GetNextToken();

	// Either a classVar Declaration or subroutine declaration
	if (!strcmp(t.lx, "static") || !strcmp(t.lx, "field"))
		return classVarDecl();
	else if (!strcmp(t.lx, "constructor") || !strcmp(t.lx, "function") || !strcmp(t.lx, "method")) 
		return subroutineDecl();
	else 
		return error("class member declaration must begin with static, field, constructor, function or method",
				      memberDeclarErr);

	return pi;				  
}


ParserInfo classVarDecl() {
	// Expect type
	pi = type();	
	if (pi.er != none) 
		return pi;

	// Expect an identifier
	t = GetNextToken();
	if (t.tp == ID)
		; // Happy days :D
	else
		return error("identifier expected", idExpected);

	// 0 or more ", identifier"
	t = PeekNextToken();
	// If semi colon then exit
	while (strcmp(t.lx, ";") && !strcmp(t.lx, ",")) {
		// Comma
		t = GetNextToken();
		if (!strcmp(t.lx, ","))
			; // Gravy
		else
			return error("comma expected after identifier", syntaxError);

		// Identifier
		// Expect an identifier **** TURN IT INTO OWN FUNCTION ****
		t = GetNextToken();
		if (t.tp == ID)
			; // Happy days :D
		else
			return error("identifier expected", idExpected);

		t = PeekNextToken();
	}

	// Expect semi colon
	t = GetNextToken();
	if (!strcmp(t.lx, ";"))
		;  // Whoopdey do 
	else
		return error("; expected", semicolonExpected);

	return pi;
}

ParserInfo type() {
	t = GetNextToken();
	
	if ((!strcmp(t.lx, "int")     || !strcmp(t.lx, "char") ||
		 !strcmp(t.lx, "boolean") || t.tp == ID))
		;  // Big time!
	else
		error("a type must be int, char, boolean or identifier", illegalType);

	return pi;
}


ParserInfo subroutineDecl() {
	return pi;
}


ParserInfo error(char *msg, SyntaxErrors e) {
	ParserInfo pi;
	pi.tk = t;
	pi.er = e;
	if (t.tp == ERR) {
		printf("%s at line %i\n", t.lx, t.ln);
		pi.er = lexerErr;
	}
	else 
		printf("Error, line %i, close to %s, %s.\n", t.ln, t.lx, msg);
	return pi;
}

int StopParser () {
	return StopLexer();
}

void printToken(Token t) {
	printf("Current token is %s\n", t.lx);
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
