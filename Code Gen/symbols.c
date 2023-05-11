
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
#include "parser.h"
#include <string.h>
#include "compiler.h"
#include <stdlib.h>
#include <stdio.h>

const int MAX_CLASSES = 128; // Maxiumum Classes in a program
const int MAX_METHODS = 128; // Maxiumum methods in a class
int parseNum;
int MAX_SYMBOLS  = 128;

int CLASS_SEARCH = 0;
int LOCAL_SEARCH = 1;
int PROG_SEARCH  = 2;

int PROG_SCOPE   = 3;
int CLASS_SCOPE  = 4;
int METHOD_SCOPE = 5;

int IS_INIT      = 6;
int NOT_INIT     = 7;

// Is the global scope which contains the class level tables
programTable progTable;
unsigned int scope;

// Inserting symbols into class level scope or method level scope
static void insertToClass( symbol s );
static void insertToMethod( symbol s );

// Inserting a class or method into the current scope
static int findInClass( char *name );
static int findInMethod( char *name );
// Only used to find classes or methods at the end of compilation
static int findInProgram( char *name ); // For searching the program for a method or class

// Get the current method / class
static classTable *getCurrentClass(); 
static methodTable *getCurrentMethod(); 

// Allocating memory for the new table and changing the program scope
static void insertTable(symbol s);

// Initialise the program table.
void initTable() {
	scope = PROG_SCOPE;

	// Set the tables to null becuase there's currently no tables
	// When we encounter a class then we allocate memory for that table
	progTable.classes = malloc( sizeof(classTable) * MAX_CLASSES );

	if ( progTable.classes == NULL ) {
		printf("COMPILER ERROR: Failed to allocate tables to program symbol table.\n");
		exit(-1);
	}

	// Set all the symbol attributes to null
	for (int i = 0; i < MAX_SYMBOLS; ++i)
		progTable.symbols[i].attr = NULL;

	// Set the symbolCount and tableCount to 0
	progTable.classCount = 0;
	progTable.classIndex = 0;

	progTable.uSymTable = malloc(sizeof(undeclSymbols));
	progTable.uSymTable->uSymCount = 0;
}


void insertSymbol( symbol s ) {

	if ( scope == CLASS_SCOPE )
		insertToClass(s);
	else if ( scope == METHOD_SCOPE )
		insertToMethod(s);
	else {
		progTable.symbols[progTable.classCount] = s;
	}

	// Checking to see if the symbol is a class or a method
	// If so allocate memory for the table and change the scope of the program
	if ( s.dataType == CLASS || s.dataType == SUB )
		insertTable(s);
}


// Loop thorugh the symbols and see if the name already exists
// Return the index of the symbol if it exists
// Else return -1
int findSymbol( char *name, unsigned int flag ) {
	static int symbolIndex;
	if (flag == PROG_SEARCH)
		return findInProgram(name);

	if (scope == PROG_SCOPE) {
		for (int i = 0; i < progTable.classCount; ++i) {
			if (!strcmp(progTable.symbols[i].name, name))
				return i;
		}
		return -1;
	}

	// First check current scope.
	// Then check the scope above ( if method, then class etc. )
	if ( scope == METHOD_SCOPE && flag == LOCAL_SEARCH ) {
		symbolIndex = findInMethod(name);
		return symbolIndex;

	} else if ( scope == CLASS_SCOPE && flag == LOCAL_SEARCH ) {
		symbolIndex = findInClass(name);
		return symbolIndex;
	} 

	// Class level search
	if (scope == METHOD_SCOPE) {
		symbolIndex = findInMethod(name);
		if (symbolIndex != -1)
			return symbolIndex;
		symbolIndex = findInClass(name);
		return symbolIndex;
	}
	symbolIndex = findInClass(name);
	return symbolIndex;
}


void closeTable() {
	// ALSO NEED TO FREE SYMBOL NAMES IF ALLOCATED
	// ALso have to loop through all symbols freeing the attributes
	for (int i = 0; i < progTable.classCount; ++i) {
		// printf("Now freeing the %s table\n", progTable.symbols[i].name);
		classTable *cClass = progTable.classes + i;

		// printf("Freeing from %s\n", progTable.symbols[i].name);

		for (int j = 0; j < cClass->methodCount; ++j) {
			methodTable *cMethod = cClass->methods + j; 

			// Free method level symbol attributes
			for (int k = 0; k < cMethod->symbolCount; ++k) {
				if (cMethod->symbols[k].attr != NULL) {
					// printf("Now freeing method symbol %s\n", cMethod->symbols[k].name);
					free( cMethod->symbols[k].attr );
				}
			}
		}

		// Then free the class symbol attributes
		// printf("Freeing the class level symbols\n");
		for (int j = 0; j < cClass->symbolCount; ++j) {
			if (cClass->symbols[j].attr != NULL) {
				// printf("Freeing %s\n", cClass->symbols[j].name);
				free(cClass->symbols[j].attr);
			}
		}
	}

	// Free all the program level symbols
	for (int i = 0; i < progTable.classCount; ++i) {
		if (progTable.symbols[i].attr != NULL)
			free(progTable.symbols[i].attr);
	}


	// loop through all classes
	for (int i = 0; i < progTable.classCount; ++i) { 
		if ((progTable.classes+i)->methodCount > 0)
			free((progTable.classes+i)->methods); // Free method tables
	}


	free(progTable.classes);
	free(progTable.uSymTable);
}


static int findInMethod( char *name ) {
	methodTable *currentMethod = getCurrentMethod();

	symbol sym;
	for (int i = 0; i < currentMethod->symbolCount; ++i) {
		sym = currentMethod->symbols[i];
		if (!strcmp(sym.name, name))
			return i;
	}

	return -1;
}


static int findInClass( char *name ) {
	classTable *currentClass = getCurrentClass();

	symbol sym;
	for ( int i = 0; i < currentClass->symbolCount; ++i ) {
		sym = currentClass->symbols[i];
		if (!strcmp(sym.name, name))
			return i;
	}

	return -1;
}


// Insert a symbol into the current class table
static void insertToClass( symbol s ) {

	classTable *currentClass = getCurrentClass();

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

	methodTable *currentMethod = getCurrentMethod();

	// Check that there's space
	if ( currentMethod->symbolCount == MAX_SYMBOLS ) {
		printf("ERROR: Maxiumum number of method level symbols reached. Identifier: %s\n", s.name);
		exit(-1);
	}

	currentMethod->symbols[currentMethod->symbolCount] = s;
	currentMethod->symbolCount ++;
}


symbol * getSymbol( char *name ) {


	return NULL;
}


// Inserting a table into existing tables
void insertTable( symbol s ) {
	classTable *currentClass;
	methodTable *currentMethod;

	if ( scope == METHOD_SCOPE ) {
		// There must've been an error that hasn't been caught by our parser
		printf("PARSING ERROR: Can't add a method or class inside a method.\n");
		exit(-1);
	}

	// If we're in program scope it means we're inserting a class
	if ( scope == PROG_SCOPE ) {
		// Initialise the values of the new tablE
		progTable.classCount ++;
		scope = CLASS_SCOPE;

		// Get the class that we just inserted
		currentClass = getCurrentClass();
		currentClass->methodCount = 0;
		currentClass->symbolCount = 0;

		// Set all the symbol attributes to null
		for ( int i = 0; i < MAX_SYMBOLS; ++i )
			currentClass->symbols[i].attr = NULL;

		return;
	}

	// Class scope means we're inserting a method
	// If there's already a method inside the table then we don't need to malloc
	currentClass = getCurrentClass();
	if ( currentClass->methodCount == 0 && currentClass != NULL ) 
		// We want to allocate memory to the tables field of the current table
		currentClass->methods = malloc( sizeof(methodTable) * MAX_METHODS );

	currentClass->methodCount ++;
	scope = METHOD_SCOPE;

	currentMethod = getCurrentMethod();
	currentMethod->symbolCount = 0;

	// Set all the symbol attributes to null
	for ( int i = 0; i < MAX_SYMBOLS; ++i )
		currentMethod->symbols[i].attr = NULL;
}


static int findInProgram( char *name ) {

	// Loop through classes
	for (int i = 0; i < progTable.classCount; ++i) {
		if (!strcmp(progTable.symbols[i].name, name))
			return i;

		classTable * ct = progTable.classes+i;
		// Loop through class symbols
		for (int j = 0; j < ct->symbolCount; ++j)
			if (!strcmp(ct->symbols[j].name, name) && ct->symbols[j].dataType == SUB)
				return j;
	}

	return -1;
}


/*
 * We want to find the index of that varType
 * AKA - How many of that vartype are there?
 * In the current class. 
 */
unsigned int indexOf( VarType vType ) {
	unsigned int count = 0;

	// If vType == ARG search the current method
	if (vType == ARG) {
		methodTable * cMethod = getCurrentMethod();
		for (int i = 0; i < cMethod->symbolCount; ++i) {
			if (cMethod->symbols[i].attr->varType == vType)
				count++;
		}
		return count;
	}

	// Else we search through the class symbols
	classTable * cClass = getCurrentClass();
	for (int i = 0; i < cClass->symbolCount; ++i) {
		if (cClass->symbols[i].attr->varType == vType)
			count++;
	}
	return count;
}


classTable * getClass(char *name) {
	for (int i = 0; i < progTable.classCount; i++) {
		if (!strcmp(progTable.symbols[i].name, name))
			return (progTable.classes+i);
	}
	return NULL;
}

methodTable * getMethod ( classTable* parent, char *name) {
	int mIndex = 0;
	for (int i = 0; i < parent->symbolCount; ++i) {
		if (parent->symbols[i].dataType != SUB)
			continue;
		if (strcmp(parent->symbols[i].name, name)) {
			mIndex ++;
			continue;
		}
		return (parent->methods+mIndex);
		
	}
	return NULL;
}

int findSymbolInClass(char *parentClass, char *method, char *symbol) {
	classTable * c = getClass(parentClass);

	// Loop through the symbols of c looking for symbol
	for (int i = 0; i < c->symbolCount; ++i) {
		if (!strcmp(c->symbols[i].name, symbol))
			return i;
	}

	// Check the method table if there is one
	methodTable * m = getMethod(c, method);
	if (m != NULL) {
		for (int i = 0; i < m->symbolCount; ++i) {
			if (!strcmp(m->symbols[i].name, symbol))
				return i;
		}
	}

	return -1;
}

// GET PARENT CLASS -> Returns the class that the function resides in

unsigned int getNLocals(char *pName, char *name) {
	static unsigned int count = 0;
	classTable * c = getClass(pName);
	methodTable * m = getMethod(c, name);

	// Loop through symbols counting the number of non arg symbols
	for (int i = 0; i < m->symbolCount; i++) {
		if (m->symbols[i].attr->varType != ARG)
			count++;
	}
	
	return count;	
}

unsigned int getArgc(char *pName, char *name) {
	static unsigned int count = 0;
	classTable * c = getClass(pName);
	methodTable * m = getMethod(c, name);

	// Loop through symbols counting the number of arg symbols
	for (int i = 0; i < m->symbolCount; i++) {
		if (m->symbols[i].attr->varType == ARG)
			count++;
	}

	return count;
}


static classTable *getCurrentClass() {
	if ( progTable.classCount == 0 || scope == PROG_SCOPE)
		return NULL;
	return (progTable.classes + (progTable.classCount-1));
}

static methodTable *getCurrentMethod() {
	classTable *currentClass = getCurrentClass();
	if ( currentClass == NULL || currentClass->methodCount == 0 || scope == CLASS_SCOPE )
		return NULL;
	return (currentClass->methods + currentClass->methodCount-1);
}

void changeScope(unsigned int newScope) { scope = newScope; }

