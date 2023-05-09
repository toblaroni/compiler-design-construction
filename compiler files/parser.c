#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "symbols.h"
#include "lexer.h"
#include "parser.h"

// Function Prototypes
void error(SyntaxErrors e, ParserInfo *pi, Token t);
ParserInfo newParserInfo();
ParserInfo classDecl();
ParserInfo memberDecl( char *parentClass );
ParserInfo classVarDecl();
Kind type( ParserInfo *pi );
ParserInfo subroutineDecl( char *parentClass );
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
ParserInfo expression(Token*);
ParserInfo relationalExpression(Token*);
ParserInfo arithmeticExpression(Token*);
ParserInfo term(Token*);
ParserInfo factor(Token*);
ParserInfo operand(Token*);
ParserInfo expId( symbol *sym, Token *tkn ); // For when you expect an identifier.
ParserInfo expOParen();	// For when you expect (
ParserInfo expCParen();	// For when you expect )
ParserInfo expOBrace();	// For when you expect {
ParserInfo expCBrace();	// For when you expect }
ParserInfo expSColon();	// For when you expect ;


void * newAttr() { return malloc(sizeof(attributes)); }

// Create a default ParserInfo struct
ParserInfo newParserInfo() {
	ParserInfo pi;
	pi.er = none;
	return pi;
}

// Adding a symbol to the table
ParserInfo addSymbol(symbol s, Token t) {
	ParserInfo pi = newParserInfo();

	// Check the name hasn't already been taken
	// Search the current scope (METHOD or CLASS)
	if ( findSymbol(t.lx, LOCAL_SEARCH) != -1 ) {
		error(redecIdentifier, &pi, t);
		return pi;
	}
	// Set the name of the symbol to the lexeme of the token
	strcpy(s.name, t.lx);
	insertSymbol(s);
	return pi;
}

int InitParser (char* file_name) {
	return InitLexer(file_name);
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
		error(classExpected, &pi, t);
		return pi;
	}

	// Expect class id 
	// We want to create a new symbol with the of the class
	// We also want to see if the class name already exists
	symbol s;
	s.dataType = CLASS;
	pi = expId( &s, &t );
	if (pi.er)
		return pi;

	char className[64];
	strcpy(className, t.lx);
	// Add class to the symbol table
	pi = addSymbol(s, t);
	if (pi.er)
		return pi;

	// Expect Open brace
	pi = expOBrace();
	if (pi.er)
		return pi;

	// Expect 0 or more member declarations
	t = PeekNextToken();
	while ( !strcmp(t.lx, "static")		 || !strcmp(t.lx, "field")    || 
			!strcmp(t.lx, "constructor") || !strcmp(t.lx, "function") || 
			!strcmp(t.lx, "method") ) {
						
		pi = memberDecl( className );
		if (pi.er)
			return pi;

		t = PeekNextToken();
	}
	
	// Expect closing brace
	pi = expCBrace();
	if (pi.er)
		return pi;
	
	// Change the scope of the symbol table back to program scope
	changeScope(PROG_SCOPE);
	return pi;
}


ParserInfo memberDecl( char *className ) {
	ParserInfo pi = newParserInfo();
	Token t;

	t = PeekNextToken();
	// Either a classVar Declaration or subroutine declaration
	if (!strcmp(t.lx, "static") || !strcmp(t.lx, "field"))
		pi = classVarDecl();
	else if (!strcmp(t.lx, "constructor") || !strcmp(t.lx, "function") || !strcmp(t.lx, "method")) 
		pi = subroutineDecl( className );
	else 
		error(memberDeclarErr, &pi, t);

	return pi;
}


ParserInfo classVarDecl() {
	ParserInfo pi = newParserInfo();
	Token t;
	symbol s;
	s.dataType = VAR;

	// Allocate memory for the variables attributes
	s.attr = newAttr();
	s.attr->isInit = NOT_INIT;
	VarType vType;

	t = GetNextToken();
	if (!strcmp(t.lx, "static")) 
		vType = STATIC; // We've got a static variable
	else if (!strcmp(t.lx, "field")) 
		vType = FIELD; // We've got a field variable
	else {
		error(memberDeclarErr, &pi, t);
		return pi;
	}

	t = PeekNextToken();
	char temp[64];
	s.attr->varType = vType;
	Kind k = type(&pi);
	if (pi.er)
		return pi;
	s.attr->kind = k;
	// If the kind == TYPE then store the id of that type
	if (k == TYPE) {
		strcpy(temp, t.lx);
		strcpy(s.attr->typeName, t.lx);
	}

	// Expect an identifier
	pi = expId(&s, &t);
	if (pi.er)
		return pi;
	pi = addSymbol(s, t); // If there's no errors add the symbol to the table
	if (pi.er)
		return pi;

	// 0 or more ", identifier"
	t = PeekNextToken();
	// If semi colon then exit
	while (!strcmp(t.lx, ",")) {
		GetNextToken(); // Consume the token
		// Make a new symbol for each id we encounter
		// Must have the same vartype and type
		symbol s;
		s.dataType = VAR;
		s.attr = newAttr();
		s.attr->varType = vType;
		strcpy(s.attr->typeName, temp);
		s.attr->isInit = NOT_INIT;
		s.attr->kind = k;

		pi = expId(&s, &t);
		if (pi.er)
			return pi;
		pi = addSymbol(s, t);
		if (pi.er)
			return pi;

		t = PeekNextToken();
	}

	// Expect semi colon
	pi = expSColon();
	return pi;
}

Kind type(ParserInfo *pi) {
	Token t;
	t = GetNextToken();

	if (!strcmp(t.lx, "int"))
			return INTEGER;
	else if (!strcmp(t.lx, "char"))
			return CHAR;
	else if (!strcmp(t.lx, "boolean"))
			return BOOL;
	else if ( t.tp == ID )
			return TYPE;

	error(illegalType, pi, t);
	return BAD_KIND;
}


ParserInfo subroutineDecl( char *parentClass ) {
	ParserInfo pi = newParserInfo();
	Token t;
	symbol s;
	s.attr = newAttr();
	s.attr->isInit = IS_INIT;

	t = GetNextToken();
	// Expect constructor, function or method
	if (!strcmp(t.lx, "constructor")) {
		s.dataType   = METHOD;
		s.attr->kind = CONSTRUCTOR;
	} else if (!strcmp(t.lx, "function") || !strcmp(t.lx, "method")) {
		s.dataType = METHOD;  // We groovin' 
	} else {
		error(subroutineDeclarErr, &pi, t);
		return pi;
	}
		
	t = GetNextToken();
	// void or type
	// Return type of the subroutine
	if (!strcmp(t.lx, "void"))
		s.attr->returnType = VOID;
	else if (!strcmp(t.lx, "int")) 
		s.attr->returnType = INTEGER;	
	else if (!strcmp(t.lx, "char")) 
		s.attr->returnType = CHAR;	
	else if (!strcmp(t.lx, "boolean")) 
		s.attr->returnType = BOOL;	
	else if (t.tp == ID) 
		s.attr->returnType = TYPE;	
	else {
		error(syntaxError, &pi, t);
		return pi;
	}


	pi = expId(&s, &t);
	if (pi.er)
		return pi;
	// Check that the constructor is called "new" 
	if (s.attr->kind == CONSTRUCTOR && strcmp("new", t.lx)) {
		error(syntaxError, &pi, t);
		return pi;
	}

	pi = addSymbol(s, t); // If it's all gravy add to the table 
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

	// ***** GETS PASSED THE RETURN TYPE OF THE METHOD *****
	pi = subroutineBody(); 
	return pi;
}


ParserInfo paramList() {
	ParserInfo pi = newParserInfo();
	Token t;
	symbol s;
	s.attr = newAttr();
	s.attr->isInit = IS_INIT; // Value passed in when function is called

	// either nothing || 1 or more type id(,)
	t = PeekNextToken();
	// Check no params
	if ( strcmp(t.lx, "int") && strcmp(t.lx, "char") && 
		 strcmp(t.lx, "boolean") && t.tp != ID )
		return pi;

	s.dataType = VAR;
	s.attr->kind = type(&pi);
	s.attr->varType = ARG;
	if (pi.er)
		return pi;

	pi = expId(&s, &t);
	if (pi.er)
		return pi;
	pi = addSymbol(s, t);
	if (pi.er)
		return pi;
	
	t = PeekNextToken();	
	// Until you hit a close parenthesis
	while (!strcmp(t.lx, ",")) {
		GetNextToken();
		symbol s;
		s.dataType = VAR;
		s.attr = newAttr();
		s.attr->isInit = IS_INIT;

		s.attr->kind = type(&pi);
		s.attr->varType = ARG;
		if (pi.er)
			return pi;

		pi = expId(&s, &t);
		if (pi.er)
			return pi;
		pi = addSymbol(s, t);
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
	changeScope(CLASS_SCOPE); // End of the method
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
		error(syntaxError, &pi, t);

	return pi;
}


ParserInfo varDeclarStmt() {
	ParserInfo pi = newParserInfo();
	Token t;
	symbol s;

	t = GetNextToken();
	if (!strcmp(t.lx, "var"))
		s.dataType = VAR;  // Look on down from the bridggeeee
	else {
		error(syntaxError, &pi, t);
		return pi;
	}

	s.attr = newAttr();
	s.attr->isInit = NOT_INIT;
	Kind k = type(&pi);
	s.attr->kind = k;
	if (pi.er)
		return pi;

	pi = expId(&s, &t);
	if (pi.er)
		return pi;
	pi = addSymbol(s, t);
	if (pi.er)
		return pi;

	// { , id }
	t = PeekNextToken();
	while (!strcmp(t.lx, ",")) {
		GetNextToken(); // consume token

		symbol s;
		s.dataType = VAR;
		s.attr = newAttr();
		s.attr->isInit = NOT_INIT;
		s.attr->kind = k;

		pi = expId(&s, &t);
		if (pi.er)
			return pi;
		pi = addSymbol(s, t);
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
	symbol s;

	// Let keyword
	t = GetNextToken();
	if (!strcmp(t.lx, "let"))
		;  // We're just obvious
	else {
		error(syntaxError, &pi, t);
		return pi;
	}

	pi = expId(&s, &t);
	if (pi.er)
		return pi;
	// Make sure the symbol exists in the current class
	int f = findSymbol(t.lx, CLASS_SEARCH);
	if (f == -1) {
		error(undecIdentifier, &pi, t);
		return pi;
	}

	// We need to know the type of the symbol on the LHS
	// Get the symbol's kind
	symbol *lhs	= getSymbol(t.lx);

	// 0 Or 1 [ expression ]
	// Expression should result in an integer
	t = PeekNextToken();
	if (!strcmp(t.lx, "[")) {
		GetNextToken(); // consume the token

		pi = expression(&t);
		if (pi.er)
			return pi;
		if (t.tp != INT) {
			error(syntaxError, &pi, t);
		}

		// Closing ]
		t = GetNextToken();
		if (!strcmp(t.lx, "]"))
			;  // :::::DDDDDDD
		else {
			error(closeBracketExpected, &pi, t);
			return pi;
		}
	}

	t = GetNextToken();
	if (!strcmp(t.lx, "="))
		;  // Yeah that's calm
	else {
		error(equalExpected, &pi, t);
		return pi;
	}

	// Check that the type(LHS) == type(RHS)
	pi = expression(&t);
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
		error(syntaxError, &pi, t);
		return pi;
	}

	pi = expOParen();
	if (pi.er)
		return pi;

	pi = expression(&t);
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
		error(syntaxError, &pi, t);
		return pi;
	}

	pi = expOParen();
	if (pi.er)
		return pi;

	pi = expression(&t);
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
		error(syntaxError, &pi, t);
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
	symbol s;

	// MAKE SURE THE SUBROUTINE EXISTS
	pi = expId(&s, &t);
	if (pi.er)
		return pi;

	t = PeekNextToken();
	if (!strcmp(t.lx, ".")) {
		GetNextToken(); // Consume token

		symbol s;
		// MAKE SURE THE SUBROUTINE EXISTS
		pi = expId(&s, &t);
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

	pi = expression(&t);
	if (pi.er)
		return pi;

	t = PeekNextToken();
	while (!strcmp(t.lx, ",")) {
		GetNextToken();

		pi = expression(&t);
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
		error(syntaxError, &pi, t );
		return pi;
	}

	// 0 or 1 expressions
	t = PeekNextToken();
	if ( !strcmp(t.lx, "-")     || !strcmp(t.lx, "~")	 || 
		 !strcmp(t.lx, "(")     || !strcmp(t.lx, "true") ||
		 !strcmp(t.lx, "false") || !strcmp(t.lx, "null") ||
		 !strcmp(t.lx, "this")  || t.tp == INT || t.tp == ID || t.tp == STRING ) {
		pi = expression(&t);
		if (pi.er)
			return pi;
	}

	pi = expSColon();
	if (pi.er)
		return pi;

	// Check for unreachable code
	t = PeekNextToken();
	if (strcmp(t.lx, "}"))
		error(syntaxError, &pi, t);
	
	return pi;
}


ParserInfo expression(Token * tkn) {
	ParserInfo pi = newParserInfo();
	Token t;

	// Relational expression
	pi = relationalExpression(tkn);
	if (pi.er)
		return pi;

	// 0 or more & or | followed by a relationalExpression()
	t = PeekNextToken();
	while ( !strcmp(t.lx, "&") || !strcmp(t.lx, "|") ) {
		GetNextToken(); // Consume the token
		pi = relationalExpression(tkn);
		if (pi.er)
			return pi;

		t = PeekNextToken();
	}

	return pi;
}


ParserInfo relationalExpression(Token * tkn) {
	ParserInfo pi = newParserInfo();
	Token t;

	// Arithmetic expression
	pi = arithmeticExpression(tkn);
	if (pi.er)
		return pi;

	// 0 or more = or > or < followed by a arithmeticExpression()
	t = PeekNextToken();
	while ( !strcmp(t.lx, "=") || !strcmp(t.lx, ">") || !strcmp(t.lx, "<") ) {
		GetNextToken(); // consume the token
	
		pi = arithmeticExpression(tkn);
		if (pi.er)
			return pi;

		t = PeekNextToken();
	}

	return pi;
}


ParserInfo arithmeticExpression(Token * tkn) {
	ParserInfo pi = newParserInfo();
	Token t;

	// Term
	pi = term(tkn);
	if (pi.er)
		return pi;

	// zero or more + or - followed by an arithmeticExpression()
	t = PeekNextToken();
	while ( !strcmp(t.lx, "-") || !strcmp(t.lx, "+") ) {
		GetNextToken(); // Consume token
		pi = term(tkn);
		if (pi.er)
			return pi;

		t = PeekNextToken();
	}

	return pi;
}


ParserInfo term(Token * tkn) {
	ParserInfo pi = newParserInfo();
	Token t;

	pi = factor(tkn);
	if (pi.er)
		return pi;

	// 0 or more * or / and then a factor
	t = PeekNextToken();
	while ( !strcmp(t.lx, "*") || !strcmp(t.lx, "/") ) {
		GetNextToken();

		pi = factor(tkn);
		if (pi.er)
			return pi;

		t = PeekNextToken();
	}

	return pi;
}

ParserInfo factor(Token * tkn) {
	ParserInfo pi = newParserInfo();
	Token t;

	// Exp either - or ~ or nothing.
	t = PeekNextToken();
	if ( !strcmp(t.lx, "-") || !strcmp(t.lx, "~") )
		GetNextToken(); // Consume the token

	// Exp operand
	pi = operand(tkn);
	return pi;
}


ParserInfo operand(Token * tkn) {
	ParserInfo pi = newParserInfo();
	Token t;

	t = GetNextToken();
	*tkn = t;
	if (t.tp == INT)
		return pi; // Integer constant

	// ID [ .ID ] [ [ expression ] | ( expressionList ) ]
	if (t.tp == ID) {

		// CHECK THAT THE SYMBOL EXISTS
		
		t = PeekNextToken();
		if (!strcmp(t.lx, ".")) {
			GetNextToken(); // consume token
			t = GetNextToken();
			if (t.tp == ID)
				; // Chillin'
			else {
				error(idExpected, &pi, t);
				return pi;
			}
		}

		t = PeekNextToken();
		if (!strcmp(t.lx, "[")) {
			GetNextToken(); // Consume token
							
			pi = expression(tkn);
			if (pi.er)
				return pi;
			
			t = GetNextToken();
			if (!strcmp(t.lx, "]"))
				; // Jazzy
			else {
				error(closeBracketExpected, &pi, t);
				return pi;
			}
		} else if (!strcmp(t.lx, "(")) {
			GetNextToken();

			pi = expressionList(tkn);
			if (pi.er)
				return pi;

			pi = expCParen();
			if (pi.er)
				return pi;
		}
		return pi;
	}

	if (!strcmp(t.lx, "(")) {
		pi = expression(tkn);
		if (pi.er)
			return pi;

		pi = expCParen();
		return pi;
	}

	if (t.tp == STRING)
		return pi;

	if (t.tp != RESWORD) {
		error(syntaxError, &pi, t);
		return pi;
	}

	if ( !strcmp(t.lx, "true") || !strcmp(t.lx, "false") ||
		 !strcmp(t.lx, "null") || !strcmp(t.lx, "this") )
		;
	
	return pi;
}


// takes in a symbol and a token
ParserInfo expId( symbol *sym, Token *tkn ) {
	ParserInfo pi = newParserInfo();
	Token t;

	t = GetNextToken();
	if (t.tp == ID) {
		// Copy the lexeme of the identifier to the name of the symbol
		sym->name = malloc(strlen(t.lx)+1);
		strcpy(sym->name, t.lx); 
		*tkn = t;
	}
	else
		error(idExpected, &pi, t);

	return pi;
}

ParserInfo expOParen() {
	ParserInfo pi = newParserInfo();
	Token t;

	t = GetNextToken();
	if (!strcmp(t.lx, "("))
		;  //  Whoaaaaaaa whoaaaaaaa whoaaaaa
	else
		error(openParenExpected, &pi, t);
	return pi;
}

ParserInfo expCParen() {
	ParserInfo pi = newParserInfo();
	Token t;

	t = GetNextToken();
	if (!strcmp(t.lx, ")"))
		;  // It's deep init it's deep mate
	else
		error(closeParenExpected, &pi, t);
	return pi;
}

ParserInfo expOBrace() {
	ParserInfo pi = newParserInfo();
	Token t;

	t = GetNextToken();
	if (!strcmp(t.lx, "{"))
		;  // Isimii yata motche
	else
		error(openBraceExpected, &pi, t);
	return pi;
}

ParserInfo expCBrace() {
	ParserInfo pi = newParserInfo();
	Token t;

	t = GetNextToken();
	if (!strcmp(t.lx, "}"))
		;  // Isimii yata motche
	else
		error(closeBraceExpected, &pi, t);
	return pi;
}

ParserInfo expSColon() {
	ParserInfo pi = newParserInfo();
	Token t;

	t = GetNextToken();
	if (!strcmp(t.lx, ";"))
		;  // Isimii yata motche
	else
		error(semicolonExpected, &pi, t);
	return pi;
}


void error(SyntaxErrors e, ParserInfo *pi, Token t) {
	pi->tk = t;
	pi->er = e;
	if (t.tp == ERR)
		pi->er = lexerErr;
}


int StopParser () {
	return StopLexer();
}

#if 0
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
#endif
