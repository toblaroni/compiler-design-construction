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
ParserInfo expId();		  // For when you expect an identifier


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
	t = PeekNextToken();

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
	while (strcmp(t.lx, ";")) {
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
	t = PeekNextToken();
	
	if ((!strcmp(t.lx, "int")     || !strcmp(t.lx, "char") ||
		 !strcmp(t.lx, "boolean") || t.tp == ID))
		;  // Big time!
	else
		return error("a type must be int, char, boolean or identifier", illegalType);

	return pi;
}


ParserInfo subroutineDecl() {
	t = GetNextToken();
	// Expect constructor, function or method
	if ((t.tp == RESWORD) &&
		(!strcmp(t.lx, "constructor") || 
		 !strcmp(t.lx, "function")    || 
		 !strcmp(t.lx, "method")))
		;  // We groovin' 
	else
		return error("subroutine declaration must begin with constructor, function or method", subroutineDeclarErr);
		
	
	t = PeekNextToken();
	//  type or void
	if (!strcmp(t.lx, "void") || type().er == none)  // If the lexeme is "void" or we have a type
		;  // We mega chillin
	else
		return error("expected 'void' keyword or type", syntaxError);

	t = GetNextToken();
	// We want an ID pls
	if (expId().er != none)
		return expId();

	// Open parenthesis  **** THESE FEW LINES HAVE BEEN WRITTEN SO MUCH !!!!!! *****
	t = GetNextToken();
	if (!strcmp(t.lx, "("))
		;  // YUh yuh
	else
		return error("( expected", openParenExpected);
			

	// List of parameters
	pi = paramList();
	if (pi.er != none)
		return pi;

	// Close paranethesis
	t = GetNextToken();
	if (!strcmp(t.lx, ")"))
		;  // YUh yuh
	else
		return error(") expected", openParenExpected);
	
	// subroutine body
	
	return pi;
}


ParserInfo paramList() {
	// either nothing || 1 or more type id(,)
	t = PeekNextToken();

	// Check no params
	if (strcmp(t.lx, ")"))
		return pi;
			

	// Check for type
	if (type().er != none)
		return type();

	t = GetNextToken();
	// Check for id
	if (expId().er == none)
		; // Let's gooooo
	else
		return expId();
	
	t = PeekNextToken();	
	// Until you hit a close parenthesis
	while (strcmp(t.lx, ")")) {
		// Make sure there's a comma
		t = GetNextToken();
		if (!strcmp(t.lx, ","))
			;  // Just what we needed !!
		else
			return error(", expected", syntaxError);

		t = GetNextToken();
		// Expecting a type
		if (type().er != none)
			return type();

		t = GetNextToken();
		// Check for id
		if (expId().er == none)
			; // Let's gooooo
		else
			return expId();

		t = PeekNextToken();
	}

	return pi;
}



ParserInfo expId() {
	if (t.tp == ID)
		return pi;
	else 
		return error("identifier expected", idExpected);
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
