#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "symbols.h"
#include "lexer.h"
#include "parser.h"
#include "compiler.h"

FILE * fileOut;
char currentClass[30];
char currentMethod[30];

// Function Prototypes
void error(SyntaxErrors e, ParserInfo *pi, Token t);
ParserInfo newParserInfo();
ParserInfo classDecl();
ParserInfo memberDecl( char *parentClass );
ParserInfo classVarDecl();
Kind type( ParserInfo *pi );
ParserInfo subroutineDecl( char *parentClass );
ParserInfo paramList( SubType, char* );
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
ParserInfo expId(symbol*, Token*); // For when you expect an identifier.
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
	if (parseNum) return pi;

	// Check the name hasn't already been taken
	// Search the current scope (METHOD or CLASS)
	if ( findSymbol(t.lx, LOCAL_SEARCH) != -1) {
		error(redecIdentifier, &pi, t);
		return pi;
	}

	// Set the name of the symbol to the lexeme of the token
	strcpy(s.name, t.lx);
	// Get the index of the symbol type
	if (s.dataType == VAR)
		s.index = indexOf( s.attr->varType );

	insertSymbol(s);
	return pi;
}

int InitParser (char* file_name) {

	// Need to change the file name from .jack to .vm
	char outputFile[60] = "";
	int length = strlen(file_name) -4;
	strncpy(outputFile, file_name, length);
	strcat(outputFile, "vm");
	outputFile[length+2] = '\0';
	
	fileOut = fopen(outputFile, "w");
	if (fileOut == NULL) {
		printf("unable to open output file %s\n", outputFile);
		exit(-1);
	}

	return InitLexer(file_name);
}


ParserInfo Parse () {
	printf("Parsing %i\n", parseNum);
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
	s.attr = NULL;
	s.dataType = CLASS;
	pi = expId( &s, &t );
	if (pi.er)
		return pi;

	char className[64];
	strcpy(className, t.lx);
	strcpy(currentClass, t.lx);
	// Add class to the symbol table
	pi = addSymbol(s, t);
	if (pi.er)
		return pi;

	// Expect Open brace
	pi = expOBrace();
	if (pi.er)
		return pi;

	changeScope(CLASS_SCOPE);

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
	s.attr->varType = vType;
	Kind k = type(&pi);
	if (pi.er)
		return pi;
	s.attr->kind = k;

	// If the kind == TYPE then store the token
	if (k == TYPE && findSymbol(t.lx, PROG_SEARCH) == -1 && parseNum) {
		error(undecIdentifier, &pi, t);
		return pi;
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
	s.dataType = SUB;

	t = GetNextToken();
	// Expect constructor, function or method
	if (!strcmp(t.lx, "constructor"))
		s.attr->subType = CONSTRUCTOR;
	else if (!strcmp(t.lx, "function"))
		s.attr->subType = FUNCTION;  // We groovin' 
	else if (!strcmp(t.lx, "method"))
		s.attr->subType = METHOD;
	else {
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
	else if (t.tp == ID) {
		s.attr->returnType = TYPE;	
		if (findSymbol(t.lx, PROG_SEARCH) == -1 && parseNum) {
			error(undecIdentifier, &pi, t);
			return pi;
		}
	}
	else {
		error(syntaxError, &pi, t);
		return pi;
	}


	pi = expId(&s, &t);
	if (pi.er)
		return pi;
	// Check that the constructor is called "new" 
	if (s.attr->subType == CONSTRUCTOR && strcmp("new", t.lx)) {
		error(syntaxError, &pi, t);
		return pi;
	}

	pi = addSymbol(s, t); // If it's all gravy add to the table 
	if (pi.er)
		return pi;
	strcpy(currentMethod, t.lx);	

	// If it's the second time parsing
	if (parseNum) {
		char name[30] = "";
		strcat(name, parentClass);
		strcat(name, ".");
		strcat(name, s.name);

		writeFunc(fileOut, name, getNLocals(parentClass, t.lx));
	}

	changeScope(METHOD_SCOPE);

	pi = expOParen();
	if (pi.er)
		return pi;

	pi = paramList(s.attr->subType, parentClass);
	if (pi.er)
		return pi;

	pi = expCParen();
	if (pi.er)
		return pi;

	// ***** GETS PASSED THE RETURN TYPE OF THE METHOD *****
	pi = subroutineBody(); 
	return pi;
}


ParserInfo paramList( SubType sType, char *parentClass ) {
	ParserInfo pi = newParserInfo();
	Token t;
	symbol s;
	s.attr = newAttr();

	// ADD THE 'THIS' SYMBOL AS THE FIRST ARGUMENT IF 
	// SUBROUTINE IS A METHOD
	if (sType == METHOD) {
		symbol this;	

		// Allocate
		this.attr = malloc(sizeof(attributes));
		this.name = malloc(strlen("this")+1);
		this.dataType = VAR;
		this.attr->varType = ARG;
		this.index = 0;

		strcpy(t.lx, "this");
		strcpy(this.attr->belongsTo, parentClass);
		addSymbol(this, t);
	}

	t = PeekNextToken();
	// either nothing || 1 or more type id(,)
	// Check no params
	if ( strcmp(t.lx, "int") && strcmp(t.lx, "char") && 
		 strcmp(t.lx, "boolean") && t.tp != ID )
		return pi;

	s.dataType = VAR;
	s.attr->kind = type(&pi);
	s.attr->varType = ARG;
	if (pi.er)
		return pi;

	if (s.attr->kind == TYPE && findSymbol(t.lx, PROG_SEARCH) == -1 && parseNum) {
		error(undecIdentifier, &pi, t);
		return pi;
	}

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

		s.attr->kind = type(&pi);
		s.attr->varType = ARG;
		if (pi.er)
			return pi;
		if (s.attr->kind == TYPE && findSymbol(t.lx, PROG_SEARCH) == -1 && parseNum) {
			error(undecIdentifier, &pi, t);
			return pi;
		}

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

	t = PeekNextToken();
	s.attr = newAttr();
	Kind k = type(&pi);
	s.attr->kind = k;
	if (pi.er)
		return pi;

	if (k == TYPE && findSymbol(t.lx, PROG_SEARCH) == -1 && parseNum) {
		error(undecIdentifier, &pi, t);
		return pi;
	}

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
	int f = findSymbolInClass(currentClass, currentMethod, t.lx);
	if (f == -1) {
		error(undecIdentifier, &pi, t);
		return pi;
	}

	// 0 Or 1 [ expression ]
	// Expression should result in an integer
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
		error(syntaxError, &pi, t);
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
		error(syntaxError, &pi, t);
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
	char subCall[30] = "";
	int argC;

	// MAKE SURE THE SUBROUTINE EXISTS
	pi = expId(&s, &t);
	if (pi.er)
		return pi;
	// First search locally
	if (findSymbolInClass(currentClass, currentMethod, t.lx) == -1 && parseNum) {
		if (findSymbol(t.lx, PROG_SEARCH) == -1) {
			error(undecIdentifier, &pi, t);
			return pi;
		}
	}

	if (parseNum) strcat(subCall, t.lx);


	t = PeekNextToken();
	if (!strcmp(t.lx, ".")) {
		GetNextToken(); // Consume token

		// MAKE SURE THE SUBROUTINE EXISTS
		pi = expId(&s, &t);
		if (pi.er)
			return pi;

		if (findSymbol(t.lx, PROG_SEARCH) == -1 && parseNum) {
			error(undecIdentifier, &pi, t);
			return pi;
		}

		if (parseNum) {
			argC = getArgc(subCall, t.lx);
			strcat(subCall, ".");
			strcat(subCall, t.lx);
		}

	} else {
		
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
		error(syntaxError, &pi, t );
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
	if (strcmp(t.lx, "}"))
		error(syntaxError, &pi, t);

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

		// CHECK THAT THE SYMBOL EXISTS
		if (findSymbol(t.lx, PROG_SEARCH) == -1 && 
			findSymbolInClass(currentClass, currentMethod, t.lx) == -1 && parseNum) {
			// Add to the undeclared symbol table
			error(undecIdentifier, &pi, t);
			return pi;
		}

		t = PeekNextToken();
		if (!strcmp(t.lx, ".")) {
			GetNextToken(); // consume token
			t = GetNextToken();
			if (t.tp == ID) {
				if (findSymbol(t.lx, PROG_SEARCH) == -1 && parseNum) {
					error(undecIdentifier, &pi, t);
					return pi;
				}
			}
			else {
				error(idExpected, &pi, t);
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
				error(closeBracketExpected, &pi, t);
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
	fclose(fileOut);
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
