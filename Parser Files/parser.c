#include <bits/pthreadtypes.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "lexer.h"
#include "parser.h"

// Global token and parserinfo variable
Token t;
ParserInfo pi;


// Function Prototypes
void error(char *msg, SyntaxErrors e);
void classDecl();
void memberDecl();
void classVarDecl();
void type();
void subroutineDecl();
void paramList();
void subroutineBody();
void statement();
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
void expId();		// For when you expect an identifier
void expOParen();	// For when you expect (
void expCParen();	// For when you expect )
void expOBrace();	// For when you expect {
void expCBrace();	// For when you expect }
void expSColon();	// For when you expect ;


int InitParser (char* file_name) {
	return InitLexer(file_name);
}


ParserInfo Parse () {
	pi.er = none;
	classDecl();
	return pi;
}


void classDecl() {

	// Expect class keyword
	t = GetNextToken();
	if ( !strcmp(t.lx, "class") && t.tp == RESWORD )
		; // Be happy :) 
	else {
		error("keyword class expected", classExpected);
	}

	// Expect class id 
	expId();

	// Expect Open brace
	expOBrace();

	// Expect 0 or more member declarations
	t = PeekNextToken();
	while (strcmp(t.lx, "}")) {
		memberDecl();
		t = PeekNextToken();
	}
	
	// Expect closing brace
	expCBrace();
}


void memberDecl() {
	t = PeekNextToken();

	// Either a classVar Declaration or subroutine declaration
	if (!strcmp(t.lx, "static") || !strcmp(t.lx, "field"))
		classVarDecl();
	else if (!strcmp(t.lx, "constructor") || !strcmp(t.lx, "function") || !strcmp(t.lx, "method")) 
		subroutineDecl();
	else 
		error("class member declaration must begin with static, field, constructor, function or method",
				      memberDeclarErr);
}


void classVarDecl() {

	type();	

	// Expect an identifier
	expId();

	// 0 or more ", identifier"
	t = PeekNextToken();
	// If semi colon then exit
	while (strcmp(t.lx, ";")) {
		// Comma
		t = GetNextToken();
		if (!strcmp(t.lx, ","))
			; // Gravy
		else
			error("comma expected after identifier", syntaxError);

		expId();

		t = PeekNextToken();
	}

	// Expect semi colon
	expSColon();
}

void type() {
	GetNextToken();
	
	if ((!strcmp(t.lx, "int")     || !strcmp(t.lx, "char") ||
		 !strcmp(t.lx, "boolean") || t.tp == ID))
		;  // Big time!
	else
		error("a type must be int, char, boolean or identifier", illegalType);
}


void subroutineDecl() {
	t = GetNextToken();
	// Expect constructor, function or method
	if ((t.tp == RESWORD) &&
		(!strcmp(t.lx, "constructor") || 
		 !strcmp(t.lx, "function")    || 
		 !strcmp(t.lx, "method")))
		;  // We groovin' 
	else
		error("subroutine declaration must begin with constructor, function or method",
					 subroutineDeclarErr);
		
	t = PeekNextToken();
	//  type or void
	if (!strcmp(t.lx, "void"))  // If the lexeme is "void" or we have a type
		;  // We mega chillin
	else if ((!strcmp(t.lx, "int")     || !strcmp(t.lx, "char") ||
		      !strcmp(t.lx, "boolean") || t.tp == ID))
		;  // Big time!
	else
		error("expected void or type", syntaxError);


	expId();

	expOParen();

	paramList();

	expCParen();

	subroutineBody();  // We want that subroutine body
}


void paramList() {
	// either nothing || 1 or more type id(,)
	t = PeekNextToken();

	// Check no params
	if (!strcmp(t.lx, ")"))
		return;

	type();

	expId();
	
	t = PeekNextToken();	

	// Until you hit a close parenthesis
	while (!strcmp(t.lx, ",")) {
		// Make sure there's a comma
		t = GetNextToken();
		if (!strcmp(t.lx, ","))
			;  // Just what we needed !!
		else
			error(", expected", syntaxError);

		type();

		expId();

		t = PeekNextToken();
	}
}


void subroutineBody() {
	expOBrace();
	
	// 0 or more statement
	t = PeekNextToken();
	while (!strcmp(t.lx, "var")   || !strcmp(t.lx, "let")    || !strcmp(t.lx, "if")  ||
		   !strcmp(t.lx, "while") ||  !strcmp(t.lx, "do")    || !strcmp(t.lx, "return")) {
		statement();
		t = PeekNextToken();
	}
	
	expCBrace();
}


void statement() {
	t = PeekNextToken();
	if (!strcmp(t.lx, "var"))
		varDeclarStmt();
	else if (!strcmp(t.lx, "let"))
		letStmt();
	else if (!strcmp(t.lx, "if"))
		ifStmt();
	else if (!strcmp(t.lx, "while"))
		whileStmt();
	else if (!strcmp(t.lx, "do"))
		doStmt();
	else if (!strcmp(t.lx, "return"))
		returnStmt();
}


void varDeclarStmt() {
	t = GetNextToken();
	if (!strcmp(t.lx, "var"))
		;  // Look on down from the bridggeeee
	else
		error("var keyword expected", syntaxError);

	type();
	if (pi.er)
		return;

	expId();
	if (pi.er)
		return;

	// 0 Or 1 [ expression ]
	t = PeekNextToken();
	if (!strcmp(t.lx, "[")) {
		expression();
		if (pi.er)
			return;

		// Closing ]
		if (!strcmp(t.lx, "]"))
			;  // :::::DDDDDDD
		else {
			error("] expected", closeBracketExpected);
			return;
		}
	}

	// Equal sign
	if (!strcmp(t.lx, "="))
		;  // Look on down from the bridggeeee
	else
		error("= expected", equalExpected);
	
	
	// Expression
	expression();
	if (pi.er)
		return;
	
	expSColon();
	if (pi.er)
		return;
}


void letStmt() {
	// Let keyword
	t = GetNextToken();
	if (!strcmp(t.lx, "let"))
		;  // We're just obvious
	else {
		error("let keyword expected", syntaxError);
		return;
	}

	expId();
	// Check for errors
	if (pi.er)
		return;

	// 0 Or 1 [ expression ]
	t = PeekNextToken();
	if (!strcmp(t.lx, "[")) {
		expression();
		if (pi.er)
			return;

		// Closing ]
		if (strcmp(t.lx, "]"))
			;  // :::::DDDDDDD
		else {
			error("] expected", closeBracketExpected);
			return;
		}
	}

	t = GetNextToken();
	if (!strcmp(t.lx, "="))
		;  // Yeah that's calm
	else {
		error("= expected", equalExpected);
		return;
	}

	expression();
	if (pi.er)
		return;

	expSColon();
	if (pi.er)
		return;
}


void ifStmt() {
	t = GetNextToken();
	if (!strcmp(t.lx, "if"))
		;  // Think for yourself. figure it out yourself
	else {
		error("if keyword expected", syntaxError);
		return;
	}

	expOParen();
	if (pi.er)
		return;

	expression();
	if (pi.er)
		return;

	expCParen();
	if (pi.er)
		return;

	expOBrace();
	if (pi.er)
		return;

	t = PeekNextToken();
	while (!strcmp(t.lx, "var")   || !strcmp(t.lx, "let")    || !strcmp(t.lx, "if")  ||
		   !strcmp(t.lx, "while") ||  !strcmp(t.lx, "do")    || !strcmp(t.lx, "return")) {
		statement();
		t = PeekNextToken();
	}

	expCBrace();
	if (pi.er)
		return;

	t = PeekNextToken();
	if (!strcmp(t.lx, "else")) {
		// Consume the token
		GetNextToken();

		expOBrace();
		if (pi.er)
			return;

		t = PeekNextToken();
		while (!strcmp(t.lx, "var")   || !strcmp(t.lx, "let")    || !strcmp(t.lx, "if")  ||
			   !strcmp(t.lx, "while") ||  !strcmp(t.lx, "do")    || !strcmp(t.lx, "return")) {
			statement();
			t = PeekNextToken();
		}

		expCBrace();
		if (pi.er)
			return;
	}
}


void whileStmt() {
	t = GetNextToken();
	if (!strcmp(t.lx, "while"))
		;  // Big time mega chilling
	else {
		error("while keyword expected", syntaxError);
		return;
	}

	expOParen();
	if (pi.er)
		return;

	expression();
	if (pi.er)
		return;

	expCParen();
	if (pi.er)
		return;

	expOBrace();
	if (pi.er)
		return;

	t = PeekNextToken();
	while (!strcmp(t.lx, "var")   || !strcmp(t.lx, "let")    || !strcmp(t.lx, "if")  ||
		   !strcmp(t.lx, "while") ||  !strcmp(t.lx, "do")    || !strcmp(t.lx, "return")) {
		statement();
		t = PeekNextToken();
	}

	expCBrace();
	if (pi.er)
		return;
}


void doStmt() {
	t = GetNextToken();
	if (!strcmp(t.lx, "do"))
		;  // Love you
	else {
		error("expected do keyword", syntaxError);
		return;
	}

	subroutineCall();
	if (pi.er)
		return;

	expSColon();
	if (pi.er)
		return;
}


void subroutineCall() {
	expId();
	if (pi.er)
		return;

	t = PeekNextToken();
	if (!strcmp(t.lx, ".")) {
		GetNextToken(); // Consume token
		expId();
		if (pi.er)
			return;
	}

	expOParen();
	if (pi.er)
		return;

	expressionList();
	if (pi.er)
		return;

	expCParen();
	if (pi.er)
		return;
}


void expressionList() {
	// CAN ALSO BE NOTHING

	expression();
	if (pi.er)
		return;

	t = PeekNextToken();
	while (!strcmp(t.lx, ",")) {
		GetNextToken();

		expression();
		if (pi.er)
			return;

		t = PeekNextToken();
	}
}


void expId() {
	t = GetNextToken();
	if (t.tp == ID)
		;  // Ain't that funkin' kind of hard on youuuuuu
	else
		error("identifier expected", idExpected);
}

void expOParen() {
	t = GetNextToken();
	if (!strcmp(t.lx, "("))
		;  //  Whoaaaaaaa whoaaaaaaa whoaaaaa
	else
		error("( expected", openParenExpected);
}

void expCParen() {
	t = GetNextToken();
	if (!strcmp(t.lx, ")"))
		;  // It's deep init it's deep mate
	else
		error(") expected", closeParenExpected);
}

void expOBrace() {
	t = GetNextToken();
	if (!strcmp(t.lx, "{"))
		;  // Isimii yata motche
	else
		error("{ expected", openBraceExpected);
}

void expCBrace() {
	t = GetNextToken();
	if (!strcmp(t.lx, "}"))
		;  // Isimii yata motche
	else
		error("} expected", closeBraceExpected);
}

void expSColon() {
	t = GetNextToken();
	if (!strcmp(t.lx, ";"))
		;  // Isimii yata motche
	else
		error("; expected", semicolonExpected);
}


void error(char *msg, SyntaxErrors e) {
	pi.tk = t;
	pi.er = e;
	if (t.tp == ERR) {
		printf("%s at line %i\n", t.lx, t.ln);
		pi.er = lexerErr;
	}
	else 
		printf("Error, line %i, close to %s, %s.\n", t.ln, t.lx, msg);
	exit(1);
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
