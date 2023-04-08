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
		return;
	}

	// Expect class id 
	expId();
	if (pi.er)
		return;

	// Expect Open brace
	expOBrace();
	if (pi.er)
		return;

	// Expect 0 or more member declarations
	t = PeekNextToken();
	while ( !strcmp(t.lx, "static")		 || !strcmp(t.lx, "field")    || 
			!strcmp(t.lx, "constructor") || !strcmp(t.lx, "function") || 
			!strcmp(t.lx, "method") ) {
						
		memberDecl();
		if (pi.er)
			return;

		t = PeekNextToken();
	}
	
	// Expect closing brace
	expCBrace();
	if (pi.er)
		return;
}


void memberDecl() {
	t = PeekNextToken();
	// Either a classVar Declaration or subroutine declaration
	if (!strcmp(t.lx, "static") || !strcmp(t.lx, "field"))
		classVarDecl();
	else if (!strcmp(t.lx, "constructor") || !strcmp(t.lx, "function") || !strcmp(t.lx, "method")) 
		subroutineDecl();
	else  {
		error("class member declaration must begin with static, field, constructor, function or method",
				      memberDeclarErr);
		return;
	}

	if (pi.er)
		return;
}


void classVarDecl() {
	t = GetNextToken();
	if (!strcmp(t.lx, "static") || !strcmp(t.lx, "field"))
		; // Oh yeah let's flipping go
	else {
		error("class member declaration must begin with static, field, constructor, function or method",
				      memberDeclarErr);
		return;
	}

	type();	
	if (pi.er)
		return;

	// Expect an identifier
	expId();
	if (pi.er)
		return;

	// 0 or more ", identifier"
	t = PeekNextToken();
	// If semi colon then exit
	while (!strcmp(t.lx, ",")) {
		GetNextToken(); // Consume the token

		expId();
		if (pi.er)
			return;

		t = PeekNextToken();
	}

	// Expect semi colon
	expSColon();
	if (pi.er)
		return;
}

void type() {
	t = GetNextToken();
	
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
	else {
		error("subroutine declaration must begin with constructor, function or method",
			   subroutineDeclarErr);
		return;
	}
		
	t = GetNextToken();
	// void or type
	if (!strcmp(t.lx, "void") || !strcmp(t.lx, "int")  || 
		!strcmp(t.lx, "char") || !strcmp(t.lx, "boolean") || t.tp == ID)  // If the lexeme is "void" or we have a type
		;  // We mega chillin
	else {
		error("expected void or type", syntaxError);
		return;
	}


	expId();
	if (pi.er)
		return;

	expOParen();
	if (pi.er)
		return;

	paramList();
	if (pi.er)
		return;

	expCParen();
	if (pi.er)
		return;

	subroutineBody();  // We want that subroutine body
	if (pi.er)
		return;
}


void paramList() {
	// either nothing || 1 or more type id(,)
	t = PeekNextToken();
	// Check no params
	if ( strcmp(t.lx, "int") && strcmp(t.lx, "char") && 
		 strcmp(t.lx, "boolean") && t.tp != ID )
		return;

	type();
	if (pi.er)
		return;

	expId();
	if (pi.er)
		return;
	
	t = PeekNextToken();	
	// Until you hit a close parenthesis
	while (!strcmp(t.lx, ",")) {
		GetNextToken();

		type();
		if (pi.er)
			return;

		expId();
		if (pi.er)
			return;

		t = PeekNextToken();
	}
}


void subroutineBody() {
	expOBrace();
	if (pi.er)
		return;
	
	// 0 or more statement
	t = PeekNextToken();
	while (!strcmp(t.lx, "var")   || !strcmp(t.lx, "let") || !strcmp(t.lx, "if")  ||
		   !strcmp(t.lx, "while") || !strcmp(t.lx, "do")  || !strcmp(t.lx, "return")) {

		statement();
		if (pi.er)
			return;

		t = PeekNextToken();
	}
	
	expCBrace();
	if (pi.er)
		return;
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
	else {
		error( "Expected statement", syntaxError );
		return;
	}
}


void varDeclarStmt() {
	t = GetNextToken();
	if (!strcmp(t.lx, "var"))
		;  // Look on down from the bridggeeee
	else {
		error("var keyword expected", syntaxError);
		return;
	}

	type();
	if (pi.er)
		return;

	expId();
	if (pi.er)
		return;

	// { , id }
	t = PeekNextToken();
	while (!strcmp(t.lx, ",")) {
		GetNextToken(); // consume token

		expId();
		if (pi.er)
			return;

		t = PeekNextToken();
	}

	
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
		GetNextToken(); // consume the token

		expression();
		if (pi.er)
			return;

		// Closing ]
		t = GetNextToken();
		if (!strcmp(t.lx, "]"))
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

	// Zero or more statements
	t = PeekNextToken();
	while (!strcmp(t.lx, "var")   || !strcmp(t.lx, "let")    || !strcmp(t.lx, "if")  ||
		   !strcmp(t.lx, "while") ||  !strcmp(t.lx, "do")    || !strcmp(t.lx, "return")) {

		statement();
		if (pi.er)
			return;
		 
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
			if (pi.er)
				return;

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
	while (!strcmp(t.lx, "var")   || !strcmp(t.lx, "let") || !strcmp(t.lx, "if")  ||
		   !strcmp(t.lx, "while") || !strcmp(t.lx, "do")  || !strcmp(t.lx, "return")) {

		statement();
		if (pi.er)
			return;

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
	// Expression can start with - | ~ | int | id | ( | string | true | false | null | this
	// If it doesn't start with any of these then return 
	t = PeekNextToken();
	if ( strcmp(t.lx, "-")     && strcmp(t.lx, "~") &&
		 strcmp(t.lx, "(")     && strcmp(t.lx, "true") &&
		 strcmp(t.lx, "false") && strcmp(t.lx, "null") &&
		 strcmp(t.lx, "this")  && t.tp != INT && 
		 t.tp != ID && t.tp != STRING )
		return;

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


void returnStmt() {
	t = GetNextToken();
	if (!strcmp(t.lx, "return"))
		;  // We good yo
	else {
		error( "'return' keyword expected", syntaxError );
		return;
	}

	// 0 or 1 expressions
	t = PeekNextToken();
	if ( !strcmp(t.lx, "-")     || !strcmp(t.lx, "~")	 || 
		 !strcmp(t.lx, "(")     || !strcmp(t.lx, "true") ||
		 !strcmp(t.lx, "false") || !strcmp(t.lx, "null") ||
		 !strcmp(t.lx, "this")  || t.tp == INT || t.tp == ID || t.tp == STRING ) {
		expression();
		if (pi.er)
			return;
	}

	expSColon();
	if (pi.er)
		return;
}


void expression() {
	// Relational expression
	relationalExpression();
	if (pi.er)
		return;

	// 0 or more & or | followed by a relationalExpression()
	t = PeekNextToken();
	while ( !strcmp(t.lx, "&") || !strcmp(t.lx, "|") ) {
		GetNextToken(); // Consume the token
		relationalExpression();
		if (pi.er)
			return;

		t = PeekNextToken();
	}
}


void relationalExpression() {
	// Arithmetic expression
	arithmeticExpression();
	if (pi.er)
		return;

	// 0 or more = or > or < followed by a arithmeticExpression()
	t = PeekNextToken();
	while ( !strcmp(t.lx, "=") || !strcmp(t.lx, ">") || !strcmp(t.lx, "<") ) {
		GetNextToken(); // consume the token
	
		arithmeticExpression();
		if (pi.er)
			return;

		t = PeekNextToken();
	}
}


void arithmeticExpression() {
	// Term
	term();
	if (pi.er)
		return;

	// zero or more + or - followed by an arithmeticExpression()
	t = PeekNextToken();
	while ( !strcmp(t.lx, "-") || !strcmp(t.lx, "+") ) {
		GetNextToken(); // Consume token
		term();
		if (pi.er)
			return;

		t = PeekNextToken();
	}
}


void term() {
	factor();
	if (pi.er)
		return;

	// 0 or more * or / and then a factor
	t = PeekNextToken();
	while ( !strcmp(t.lx, "*") || !strcmp(t.lx, "/") ) {
		GetNextToken();

		factor();
		if (pi.er)
			return;

		t = PeekNextToken();
	}
}


void factor() {
	// Exp either - or ~ or nothing.
	t = PeekNextToken();
	if ( !strcmp(t.lx, "-") || !strcmp(t.lx, "~") )
		GetNextToken(); // Consume the token

	// Exp operand
	operand();
	if (pi.er)
		return;
}


void operand() {
	t = GetNextToken();
	if (t.tp == INT)
		return; // Integer constant

	// ID [ .ID ] [ [ expression ] | ( expressionList ) ]
	if (t.tp == ID) {
		t = PeekNextToken();
		if (!strcmp(t.lx, ".")) {
			GetNextToken(); // consume token
			t = GetNextToken();
			if (t.tp == ID)
				; // Chillin'
			else {
				error("identifier expected", idExpected);
				return;
			}
		}

		t = PeekNextToken();
		if (!strcmp(t.lx, "[")) {
			GetNextToken(); // Consume token
							
			expression();
			if (pi.er)
				return;
			
			t = GetNextToken();
			if (!strcmp(t.lx, "]"))
				; // Jazzy
			else {
				error("] expected", closeBracketExpected);
				return;
			}
		} else if (!strcmp(t.lx, "(")) {
			GetNextToken();

			expressionList();
			if (pi.er)
				return;

			expCParen();
			if (pi.er)
				return;
		}
		return;
	}

	if (!strcmp(t.lx, "(")) {
		expression();
		if (pi.er)
			return;

		expCParen();
		return;
	}

	if (t.tp == STRING)
		return;

	if (t.tp != RESWORD) {
		error("expected operand", syntaxError);
		return;
	}

	if ( !strcmp(t.lx, "true") || !strcmp(t.lx, "false") ||
		 !strcmp(t.lx, "null") || !strcmp(t.lx, "this") )
		return;
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
	// 	printf("%s at line %i\n", t.lx, t.ln);
		pi.er = lexerErr;
	}
	// else 
	//	printf("Error, line %i, close to %s, %s.\n", t.ln, t.lx, msg);
	// exit(1);
}


int StopParser () {
	return StopLexer();
}


#ifndef TEST_PARSER
int main (void) {
	InitParser("closeParenExpected.jack");	

	if (Parse().er) {
		exit(1);
	};
	
	printf("Successfully parsed source file\n");
	return 0;
}
#endif
