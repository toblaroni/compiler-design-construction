#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "symbols.h"
#include "lexer.h"
#include "parser.h"

// Function Prototypes
void error(char *msg, SyntaxErrors e, ParserInfo *pi, Token t);
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
	Token t;

	// Expect class keyword
	t = GetNextToken();
	if ( !strcmp(t.lx, "class") && t.tp == RESWORD )
		; // Be happy :) 
	else {
		error("keyword class expected", classExpected, &pi, t);
		return pi;
	}

	// Expect class id 
	pi = expId();
	if (pi.er)
		return pi;

	// Expect Open brace
	expOBrace();
	if (pi.er)
		return pi;

	// Expect 0 or more member declarations
	t = PeekNextToken();
	while ( !strcmp(t.lx, "static")		 || !strcmp(t.lx, "field")    || 
			!strcmp(t.lx, "constructor") || !strcmp(t.lx, "function") || 
			!strcmp(t.lx, "method") ) {
						
		pi = memberDecl();
		if (pi.er)
			return pi;

		t = PeekNextToken();
	}
	
	// Expect closing brace
	pi = expCBrace();
	return pi;
}


ParserInfo memberDecl() {
	ParserInfo pi = newParserInfo();
	Token t;

	t = PeekNextToken();
	// Either a classVar Declaration or subroutine declaration
	if (!strcmp(t.lx, "static") || !strcmp(t.lx, "field"))
		pi = classVarDecl();
	else if (!strcmp(t.lx, "constructor") || !strcmp(t.lx, "function") || !strcmp(t.lx, "method")) 
		pi = subroutineDecl();
	else  {
		error("class member declaration must begin with static, field, constructor, function or method",
			   memberDeclarErr, &pi, t);
	}

	return pi;
}


ParserInfo classVarDecl() {
	ParserInfo pi = newParserInfo();
	Token t;

	t = GetNextToken();
	if (!strcmp(t.lx, "static") || !strcmp(t.lx, "field"))
		; // Oh yeah let's flipping go
	else {
		error("class member declaration must begin with static, field, constructor, function or method",
			   memberDeclarErr, &pi, t);
		return pi;
	}

	pi = type();	
	if (pi.er)
		return pi;

	// Expect an identifier
	pi = expId();
	if (pi.er)
		return pi;

	// 0 or more ", identifier"
	t = PeekNextToken();
	// If semi colon then exit
	while (!strcmp(t.lx, ",")) {
		GetNextToken(); // Consume the token

		pi = expId();
		if (pi.er)
			return pi;

		t = PeekNextToken();
	}

	// Expect semi colon
	pi = expSColon();
	return pi;
}

ParserInfo type() {
	ParserInfo pi = newParserInfo();
	Token t;
	t = GetNextToken();
	
	if ((!strcmp(t.lx, "int")     || !strcmp(t.lx, "char") ||
		 !strcmp(t.lx, "boolean") || t.tp == ID))
		;  // Big time!
	else
		error("a type must be int, char, boolean or identifier", illegalType, &pi, t);

	return pi;
}


ParserInfo subroutineDecl() {
	ParserInfo pi = newParserInfo();
	Token t;

	t = GetNextToken();
	// Expect constructor, function or method
	if ((t.tp == RESWORD) &&
		(!strcmp(t.lx, "constructor") || 
		 !strcmp(t.lx, "function")    || 
		 !strcmp(t.lx, "method")))
		;  // We groovin' 
	else {
		error("subroutine declaration must begin with constructor, function or method",
			   subroutineDeclarErr, &pi, t);
		return pi;
	}
		
	t = GetNextToken();
	// void or type
	if (!strcmp(t.lx, "void") || !strcmp(t.lx, "int")  || 
		!strcmp(t.lx, "char") || !strcmp(t.lx, "boolean") || t.tp == ID)  // If the lexeme is "void" or we have a type
		;  // We mega chillin
	else {
		error("expected void or type", syntaxError, &pi, t);
		return pi;
	}


	pi = expId();
	if (pi.er)
		return pi;

	pi = expOParen();
	if (pi.er)
		return pi;

	pi = paramList();
	if (pi.er)
		return pi;

	pi = expCParen();
	if (pi.er)
		return pi;

	pi = subroutineBody();  // We want that subroutine body
	return pi;
}


ParserInfo paramList() {
	ParserInfo pi = newParserInfo();
	Token t;

	// either nothing || 1 or more type id(,)
	t = PeekNextToken();
	// Check no params
	if ( strcmp(t.lx, "int") && strcmp(t.lx, "char") && 
		 strcmp(t.lx, "boolean") && t.tp != ID )
		return pi;

	pi = type();
	if (pi.er)
		return pi;

	pi = expId();
	if (pi.er)
		return pi;
	
	t = PeekNextToken();	
	// Until you hit a close parenthesis
	while (!strcmp(t.lx, ",")) {
		GetNextToken();

		pi = type();
		if (pi.er)
			return pi;

		pi = expId();
		if (pi.er)
			return pi;

		t = PeekNextToken();
	}

	return pi;
}


ParserInfo subroutineBody() {
	ParserInfo pi = newParserInfo();
	Token t;

	pi = expOBrace();
	if (pi.er)
		return pi;
	
	// 0 or more statement
	t = PeekNextToken();
	while (!strcmp(t.lx, "var")   || !strcmp(t.lx, "let") || !strcmp(t.lx, "if")  ||
		   !strcmp(t.lx, "while") || !strcmp(t.lx, "do")  || !strcmp(t.lx, "return")) {

		pi = statement();
		if (pi.er)
			return pi;

		t = PeekNextToken();
	}
	
	pi = expCBrace();
	return pi;
}


ParserInfo statement() {
	ParserInfo pi = newParserInfo();
	Token t;

	t = PeekNextToken();
	if (!strcmp(t.lx, "var"))
		pi = varDeclarStmt();
	else if (!strcmp(t.lx, "let"))
		pi = letStmt();
	else if (!strcmp(t.lx, "if"))
		pi = ifStmt();
	else if (!strcmp(t.lx, "while"))
		pi = whileStmt();
	else if (!strcmp(t.lx, "do"))
		pi = doStmt();
	else if (!strcmp(t.lx, "return"))
		pi = returnStmt();
	else 
		error( "Expected statement", syntaxError, &pi, t);

	return pi;
}


ParserInfo varDeclarStmt() {
	ParserInfo pi = newParserInfo();
	Token t;

	t = GetNextToken();
	if (!strcmp(t.lx, "var"))
		;  // Look on down from the bridggeeee
	else {
		error("var keyword expected", syntaxError, &pi, t);
		return pi;
	}

	pi = type();
	if (pi.er)
		return pi;

	pi = expId();
	if (pi.er)
		return pi;

	// { , id }
	t = PeekNextToken();
	while (!strcmp(t.lx, ",")) {
		GetNextToken(); // consume token

		pi = expId();
		if (pi.er)
			return pi;

		t = PeekNextToken();
	}

	
	pi = expSColon();
	return pi;
}


ParserInfo letStmt() {
	ParserInfo pi = newParserInfo();
	Token t;

	// Let keyword
	t = GetNextToken();
	if (!strcmp(t.lx, "let"))
		;  // We're just obvious
	else {
		error("let keyword expected", syntaxError, &pi, t);
		return pi;
	}

	pi = expId();
	if (pi.er)
		return pi;

	// 0 Or 1 [ expression ]
	t = PeekNextToken();
	if (!strcmp(t.lx, "[")) {
		GetNextToken(); // consume the token

		pi = expression();
		if (pi.er)
			return pi;

		// Closing ]
		t = GetNextToken();
		if (!strcmp(t.lx, "]"))
			;  // :::::DDDDDDD
		else {
			error("] expected", closeBracketExpected, &pi, t);
			return pi;
		}
	}

	t = GetNextToken();
	if (!strcmp(t.lx, "="))
		;  // Yeah that's calm
	else {
		error("= expected", equalExpected, &pi, t);
		return pi;
	}

	pi = expression();
	if (pi.er)
		return pi;

	pi = expSColon();
	return pi;
}


ParserInfo ifStmt() {
	ParserInfo pi = newParserInfo();
	Token t;

	t = GetNextToken();
	if (!strcmp(t.lx, "if"))
		;  // Think for yourself. figure it out yourself
	else {
		error("if keyword expected", syntaxError, &pi, t);
		return pi;
	}

	pi = expOParen();
	if (pi.er)
		return pi;

	pi = expression();
	if (pi.er)
		return pi;

	pi = expCParen();
	if (pi.er)
		return pi;

	pi = expOBrace();
	if (pi.er)
		return pi;

	// Zero or more statements
	t = PeekNextToken();
	while (!strcmp(t.lx, "var")   || !strcmp(t.lx, "let")    || !strcmp(t.lx, "if")  ||
		   !strcmp(t.lx, "while") ||  !strcmp(t.lx, "do")    || !strcmp(t.lx, "return")) {

		pi = statement();
		if (pi.er)
			return pi;
		 
		t = PeekNextToken();
	}

	pi = expCBrace();
	if (pi.er)
		return pi;

	t = PeekNextToken();
	if (!strcmp(t.lx, "else")) {
		// Consume the token
		GetNextToken();

		pi = expOBrace();
		if (pi.er)
			return pi;

		t = PeekNextToken();
		while (!strcmp(t.lx, "var")   || !strcmp(t.lx, "let")    || !strcmp(t.lx, "if")  ||
			   !strcmp(t.lx, "while") ||  !strcmp(t.lx, "do")    || !strcmp(t.lx, "return")) {
			pi = statement();
			if (pi.er)
				return pi;

			t = PeekNextToken();
		}

		pi = expCBrace();
		if (pi.er)
			return pi;
	}

	return pi;
}


ParserInfo whileStmt() {
	ParserInfo pi = newParserInfo();
	Token t;

	t = GetNextToken();
	if (!strcmp(t.lx, "while"))
		;  // Big time mega chilling
	else {
		error("while keyword expected", syntaxError, &pi, t);
		return pi;
	}

	pi = expOParen();
	if (pi.er)
		return pi;

	pi = expression();
	if (pi.er)
		return pi;

	pi = expCParen();
	if (pi.er)
		return pi;

	pi = expOBrace();
	if (pi.er)
		return pi;

	t = PeekNextToken();
	while (!strcmp(t.lx, "var")   || !strcmp(t.lx, "let") || !strcmp(t.lx, "if")  ||
		   !strcmp(t.lx, "while") || !strcmp(t.lx, "do")  || !strcmp(t.lx, "return")) {

		pi = statement();
		if (pi.er)
			return pi;

		t = PeekNextToken();
	}

	pi = expCBrace();
	return pi;
}


ParserInfo doStmt() {
	ParserInfo pi = newParserInfo();
	Token t;

	t = GetNextToken();
	if (!strcmp(t.lx, "do"))
		;  // Love you
	else {
		error("expected do keyword", syntaxError, &pi, t);
		return pi;
	}

	pi = subroutineCall();
	if (pi.er)
		return pi;

	pi = expSColon();
	return pi;
}


ParserInfo subroutineCall() {
	ParserInfo pi = newParserInfo();
	Token t;

	pi = expId();
	if (pi.er)
		return pi;

	t = PeekNextToken();
	if (!strcmp(t.lx, ".")) {
		GetNextToken(); // Consume token
		pi = expId();
		if (pi.er)
			return pi;
	}

	pi = expOParen();
	if (pi.er)
		return pi;

	pi = expressionList();
	if (pi.er)
		return pi;

	pi = expCParen();
	return pi;
}


ParserInfo expressionList() {
	ParserInfo pi = newParserInfo();
	Token t;

	// CAN ALSO BE NOTHING
	// Expression can start with - | ~ | int | id | ( | string | true | false | null | this
	// If it doesn't start with any of these then return 
	t = PeekNextToken();
	if ( strcmp(t.lx, "-")     && strcmp(t.lx, "~") &&
		 strcmp(t.lx, "(")     && strcmp(t.lx, "true") &&
		 strcmp(t.lx, "false") && strcmp(t.lx, "null") &&
		 strcmp(t.lx, "this")  && t.tp != INT && 
		 t.tp != ID && t.tp != STRING )
		return pi;

	pi = expression();
	if (pi.er)
		return pi;

	t = PeekNextToken();
	while (!strcmp(t.lx, ",")) {
		GetNextToken();

		pi = expression();
		if (pi.er)
			return pi;

		t = PeekNextToken();
	}

	return pi;
}


ParserInfo returnStmt() {
	ParserInfo pi = newParserInfo();
	Token t;

	t = GetNextToken();
	if (!strcmp(t.lx, "return"))
		;  // We good yo
	else {
		error( "'return' keyword expected", syntaxError, &pi, t );
		return pi;
	}

	// 0 or 1 expressions
	t = PeekNextToken();
	if ( !strcmp(t.lx, "-")     || !strcmp(t.lx, "~")	 || 
		 !strcmp(t.lx, "(")     || !strcmp(t.lx, "true") ||
		 !strcmp(t.lx, "false") || !strcmp(t.lx, "null") ||
		 !strcmp(t.lx, "this")  || t.tp == INT || t.tp == ID || t.tp == STRING ) {
		pi = expression();
		if (pi.er)
			return pi;
	}

	pi = expSColon();
	if (pi.er)
		return pi;

	// Check for unreachable code
	t = PeekNextToken();
	
	return pi;
}


ParserInfo expression() {
	ParserInfo pi = newParserInfo();
	Token t;

	// Relational expression
	pi = relationalExpression();
	if (pi.er)
		return pi;

	// 0 or more & or | followed by a relationalExpression()
	t = PeekNextToken();
	while ( !strcmp(t.lx, "&") || !strcmp(t.lx, "|") ) {
		GetNextToken(); // Consume the token
		pi = relationalExpression();
		if (pi.er)
			return pi;

		t = PeekNextToken();
	}

	return pi;
}


ParserInfo relationalExpression() {
	ParserInfo pi = newParserInfo();
	Token t;

	// Arithmetic expression
	pi = arithmeticExpression();
	if (pi.er)
		return pi;

	// 0 or more = or > or < followed by a arithmeticExpression()
	t = PeekNextToken();
	while ( !strcmp(t.lx, "=") || !strcmp(t.lx, ">") || !strcmp(t.lx, "<") ) {
		GetNextToken(); // consume the token
	
		pi = arithmeticExpression();
		if (pi.er)
			return pi;

		t = PeekNextToken();
	}

	return pi;
}


ParserInfo arithmeticExpression() {
	ParserInfo pi = newParserInfo();
	Token t;

	// Term
	pi = term();
	if (pi.er)
		return pi;

	// zero or more + or - followed by an arithmeticExpression()
	t = PeekNextToken();
	while ( !strcmp(t.lx, "-") || !strcmp(t.lx, "+") ) {
		GetNextToken(); // Consume token
		pi = term();
		if (pi.er)
			return pi;

		t = PeekNextToken();
	}

	return pi;
}


ParserInfo term() {
	ParserInfo pi = newParserInfo();
	Token t;

	pi = factor();
	if (pi.er)
		return pi;

	// 0 or more * or / and then a factor
	t = PeekNextToken();
	while ( !strcmp(t.lx, "*") || !strcmp(t.lx, "/") ) {
		GetNextToken();

		pi = factor();
		if (pi.er)
			return pi;

		t = PeekNextToken();
	}

	return pi;
}


ParserInfo factor() {
	ParserInfo pi = newParserInfo();
	Token t;

	// Exp either - or ~ or nothing.
	t = PeekNextToken();
	if ( !strcmp(t.lx, "-") || !strcmp(t.lx, "~") )
		GetNextToken(); // Consume the token

	// Exp operand
	pi = operand();
	return pi;
}


ParserInfo operand() {
	ParserInfo pi = newParserInfo();
	Token t;

	t = GetNextToken();
	if (t.tp == INT)
		return pi; // Integer constant

	// ID [ .ID ] [ [ expression ] | ( expressionList ) ]
	if (t.tp == ID) {
		t = PeekNextToken();
		if (!strcmp(t.lx, ".")) {
			GetNextToken(); // consume token
			t = GetNextToken();
			if (t.tp == ID)
				; // Chillin'
			else {
				error("identifier expected", idExpected, &pi, t);
				return pi;
			}
		}

		t = PeekNextToken();
		if (!strcmp(t.lx, "[")) {
			GetNextToken(); // Consume token
							
			pi = expression();
			if (pi.er)
				return pi;
			
			t = GetNextToken();
			if (!strcmp(t.lx, "]"))
				; // Jazzy
			else {
				error("] expected", closeBracketExpected, &pi, t);
				return pi;
			}
		} else if (!strcmp(t.lx, "(")) {
			GetNextToken();

			pi = expressionList();
			if (pi.er)
				return pi;

			pi = expCParen();
			if (pi.er)
				return pi;
		}
		return pi;
	}

	if (!strcmp(t.lx, "(")) {
		pi = expression();
		if (pi.er)
			return pi;

		pi = expCParen();
		return pi;
	}

	if (t.tp == STRING)
		return pi;

	if (t.tp != RESWORD) {
		error("expected operand", syntaxError, &pi, t);
		return pi;
	}

	if ( !strcmp(t.lx, "true") || !strcmp(t.lx, "false") ||
		 !strcmp(t.lx, "null") || !strcmp(t.lx, "this") )
		;
	
	return pi;
}


ParserInfo expId() {
	ParserInfo pi = newParserInfo();
	Token t;


	t = GetNextToken();
	if (t.tp == ID)
		;  // Ain't that funkin' kind of hard on youuuuuu
	else
		error("identifier expected", idExpected, &pi, t);

	return pi;
}

ParserInfo expOParen() {
	ParserInfo pi = newParserInfo();
	Token t;

	t = GetNextToken();
	if (!strcmp(t.lx, "("))
		;  //  Whoaaaaaaa whoaaaaaaa whoaaaaa
	else
		error("( expected", openParenExpected, &pi, t);
	return pi;
}

ParserInfo expCParen() {
	ParserInfo pi = newParserInfo();
	Token t;

	t = GetNextToken();
	if (!strcmp(t.lx, ")"))
		;  // It's deep init it's deep mate
	else
		error(") expected", closeParenExpected, &pi, t);
	return pi;
}

ParserInfo expOBrace() {
	ParserInfo pi = newParserInfo();
	Token t;

	t = GetNextToken();
	if (!strcmp(t.lx, "{"))
		;  // Isimii yata motche
	else
		error("{ expected", openBraceExpected, &pi, t);
	return pi;
}

ParserInfo expCBrace() {
	ParserInfo pi = newParserInfo();
	Token t;

	t = GetNextToken();
	if (!strcmp(t.lx, "}"))
		;  // Isimii yata motche
	else
		error("} expected", closeBraceExpected, &pi, t);
	return pi;
}

ParserInfo expSColon() {
	ParserInfo pi = newParserInfo();
	Token t;

	t = GetNextToken();
	if (!strcmp(t.lx, ";"))
		;  // Isimii yata motche
	else
		error("; expected", semicolonExpected, &pi, t);
	return pi;
}


void error(char *msg, SyntaxErrors e, ParserInfo *pi, Token t) {
	pi->tk = t;
	pi->er = e;
	if (t.tp == ERR) {
	 	printf("%s at line %i\n", t.lx, t.ln);
		pi->er = lexerErr;
	}
	else 
		printf("Error, line %i, close to %s, %s.\n", t.ln, t.lx, msg);
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