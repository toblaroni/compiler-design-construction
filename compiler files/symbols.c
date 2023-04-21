
/************************************************************************
University of Leeds
School of Computing
COMP2932- Compiler Design and Construction
The Symbol Tables Module

I confirm that the following code has been developed and written by me and it is entirely the result of my own work.
I also confirm that I have not copied any parts of this program from another person or any other source or facilitated someone to copy this program from me.
I confirm that I will not publish the program online or share it with anyone without permission of the module leader.

Student Name: Toby Hutchinson
Student ID: 201530569
Email: sc21t2hh@leeds.ac.uk
Date Work Commenced: 08/04/23
*************************************************************************/

#include "symbols.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

const int MAX_CLASSES = 128; // Maxiumum Classes in a program
const int MAX_METHODS = 128; // Maxiumum methods in a class

// Is the global scope which contains the class level tables
symbolTable progTable;

// Inserting symbols into class level scope or method level scope
static void insertToClass( symbol s );
static void insertToMethod( symbol s );
// Inserting a class or method into the current scope
static void insertClass();
static void insertMethod();
static int findInClass( char *name );
static int findInMethod( char *name );
// Get the current method / class
static symbolTable *getCurrentClass(); 
static symbolTable *getCurrentMethod(); 

// Initialise the program table.
void initSymTable() {

	// Set the tables to null becuase there's currently no tables
	// When we encounter a class then we allocate memory for that table
	progTable.tables = malloc( sizeof(symbolTable) * MAX_CLASSES );

	if ( progTable.tables == NULL ) {
		printf("COMPILER ERROR: Failed to allocate tables to program symbol table.\n");
		exit(-1);
	}

	// Set the symbolCount and tableCount to 0
	progTable.scope = PROG_SCOPE;
	progTable.symbolCount = 0;
	progTable.methodCount = 0;
	progTable.classCount  = 0;
}


void insertSymbol( symbol s ) {

	// Check that the symbol doesn't already exist
	// if ( findSymbol( s ) != -1 ) ...

	if ( progTable.scope == CLASS_SCOPE )
		insertToClass(s);
	else if ( progTable.scope == METHOD_SCOPE )
		insertToMethod(s);

	// Check that the maximum number of symbols hasn't been reached
	if ( progTable.symbolCount == MAX_SYMBOLS ) {
		printf("ERROR: Maxiumum number of Program Level symbols reached. Identifier: %s\n", s.name);
		exit(-1);
	}  	

	progTable.symbols[progTable.symbolCount] = s;
	progTable.symbolCount ++;
}

// Loop thorugh the symbols and see if the name already exists
// Return the index of the symbol if it exists
// Else return -1
int findSymbol( char *name ) {
	
	// First check current scope.
	// Then check the scope above ( if method, then class etc. )
	if ( progTable.scope == METHOD_SCOPE ) {
		int symbolIndex = findInMethod(name);

		if ( symbolIndex != -1 )
			return symbolIndex;

	} else if ( progTable.scope == CLASS_SCOPE ) {
		int symbolIndex = findInClass(name);

		if ( symbolIndex != -1 )
			return symbolIndex;
	} 

	// Search Prog Level
	for ( int i = 0; i < progTable.symbolCount; ++i ) {
		symbol sym = progTable.symbols[i];
		if (!strcmp(sym.name, name))
			return i;
	}
	
	return -1;
}


void closeTable() {
}


static int findInMethod( char *name ) {
	symbolTable *currentMethod = getCurrentMethod();

	// Search Prog Level
	for ( int i = 0; i < currentMethod->symbolCount; ++i ) {
		symbol sym = currentMethod->symbols[i];
		if (!strcmp(sym.name, name))
			return i;
	}

	// Search the class
	int symbolIndex = findInClass( name );
	if ( symbolIndex != -1 )
		return symbolIndex;

	return -1;
}


static int findInClass( char *name ) {
	symbolTable *currentClass = getCurrentClass();

	// Search Prog Level
	for ( int i = 0; i < currentClass->symbolCount; ++i ) {
		symbol sym = currentClass->symbols[i];
		if (!strcmp(sym.name, name))
			return i;
	}
	
	return -1;
}


// Insert a symbol into the current class table
static void insertToClass( symbol s ) {

	symbolTable *currentClass = (progTable.tables + (progTable.classCount-1));

	// Check to see if it's already got the maximum number of symbols
	if ( currentClass->symbolCount == MAX_SYMBOLS ) {
		printf("ERROR: Maxiumum number of class level symbols reached. Identifier: %s\n", s.name);
		exit(-1);
	}

	// Insert symbol s into the class level table
	currentClass->symbols[currentClass->symbolCount] = s;
	currentClass->symbolCount++;
}


// Inserting a symbol into the current level scope
static void insertToMethod( symbol s ) {

	symbolTable *currentClass = (progTable.tables + (progTable.classCount-1));
	symbolTable *currentMethod = (currentClass->tables + currentClass->methodCount-1);
	
	// Check that there's space
	if ( currentMethod->symbolCount == MAX_SYMBOLS ) {
		printf("ERROR: Maxiumum number of method level symbols reached. Identifier: %s\n", s.name);
		exit(-1);
	}

	currentMethod->symbols[currentMethod->symbolCount] = s;
	currentMethod->symbolCount ++;
}


static symbolTable *getCurrentClass() {
	return (progTable.tables + (progTable.classCount-1));
}

static symbolTable *getCurrentMethod() {
	symbolTable *currentClass = getCurrentClass();
	return (currentClass->tables + currentClass->methodCount-1);
}
