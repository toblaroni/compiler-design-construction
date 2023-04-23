#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "symbols.h"
#include "lexer.h"
#include "parser.h"

// Function Prototypes
void error(char *msg, SyntaxErrors e, ParserInfo *pi);
ParserInfo newParserInfo();
ParserInfo classDecl();
ParserInfo memberDecl();
ParserInfo classVarDecl();
ParserInfo type();
ParserInfo subroutineDecl();
ParserInfo paramList();
ParserInfo subroutineBody();
ParserInfo statement();
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
ParserInfo expId();		// For when you expect an identifier
ParserInfo expOParen();	// For when you expect (
ParserInfo expCParen();	// For when you expect )
ParserInfo expOBrace();	// For when you expect {
ParserInfo expCBrace();	// For when you expect }
ParserInfo expSColon();	// For when you expect ;

// Create a default ParserInfo struct
ParserInfo newParserInfo() {
	ParserInfo pi;
	pi.er = none;
	return pi;
}

int InitParser (char* file_name) {
	int openFile = InitLexer(file_name);
	if (!openFile)
		return openFile;
	initTable();
	return openFile;
}


ParserInfo Parse () {
	return classDecl();
}


ParserInfo classDecl() {
	ParserInfo pi = newParserInfo();

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


ParserInfo memberDecl() {
	ParserInfo pi = newParserInfo();
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


ParserInfo classVarDecl() {
	ParserInfo pi = newParserInfo();
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
	ParserInfo pi = newParserInfo();
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
	ParserInfo pi = newParserInfo();
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
	ParserInfo pi = newParserInfo();
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
	ParserInfo pi = newParserInfo();
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
	ParserInfo pi = newParserInfo();
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
	ParserInfo pi = newParserInfo();
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
	ParserInfo pi = newParserInfo();
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
	ParserInfo pi = newParserInfo();
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
	ParserInfo pi = newParserInfo();
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
	ParserInfo pi = newParserInfo();
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
	ParserInfo pi = newParserInfo();
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
	ParserInfo pi = newParserInfo();
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

	// Check for unreachable code
	t = PeekNextToken();
	
	
}


void expression() {
	ParserInfo pi = newParserInfo();
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
	ParserInfo pi = newParserInfo();
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
	ParserInfo pi = newParserInfo();
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
	ParserInfo pi = newParserInfo();
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
	ParserInfo pi = newParserInfo();
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
	ParserInfo pi = newParserInfo();
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
	ParserInfo pi = newParserInfo();
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
	ParserInfo pi = newParserInfo();
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
	ParserInfo pi = newParserInfo();
	t = GetNextToken();
	if (!strcmp(t.lx, "}"))
		;  // Isimii yata motche
	else
		error("} expected", closeBraceExpected);
}

void expSColon() {
	ParserInfo pi = newParserInfo();
	t = GetNextToken();
	if (!strcmp(t.lx, ";"))
		;  // Isimii yata motche
	else
		error("; expected", semicolonExpected);
}


void error(char *msg, SyntaxErrors e, ParserInfo *pi) {
	pi.tk = t;
	pi.er = e;
	if (t.tp == ERR) {
	 	printf("%s at line %i\n", t.lx, t.ln);
		pi.er = lexerErr;
	}
	else 
		printf("Error, line %i, close to %s, %s.\n", t.ln, t.lx, msg);
	// exit(1);
}


int StopParser () {
	closeTable();
	return StopLexer();
}


#ifndef TEST_PARSER
int main (void) {
	if (!InitParser("Math.jack"))
		exit(-1);

	if (Parse().er) {
		StopParser();
		exit(1);
	};
	
	printf("Successfully parsed source file\n");
	StopParser();
	return 0;
}
#endif
