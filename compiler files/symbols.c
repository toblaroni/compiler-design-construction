
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
programTable progTable;
unsigned int scope;

// Inserting symbols into class level scope or method level scope
static void insertToClass( symbol s );
static void insertToMethod( symbol s );
// Inserting a class or method into the current scope
static int findInClass( char *name );
static int findInMethod( char *name );
// Get the current method / class
static classTable *getCurrentClass(); 
static methodTable *getCurrentMethod(); 
// Allocating memory for the new table and changing the program scope
static void insertTable();

// Initialise the program table.
void initSymTable() {
	scope = PROG_SCOPE;
	
	// Set the tables to null becuase there's currently no tables
	// When we encounter a class then we allocate memory for that table
	progTable.classes = malloc( sizeof(classTable) * MAX_CLASSES );

	if ( progTable.classes == NULL ) {
		printf("COMPILER ERROR: Failed to allocate tables to program symbol table.\n");
		exit(-1);
	}

	// Set the symbolCount and tableCount to 0
	progTable.classCount  = 0;
}


void insertSymbol( symbol s ) {

	// Check that the symbol doesn't already exist
	// if ( findSymbol( s ) != -1 ) ...
	
	if ( scope == CLASS_SCOPE )
		insertToClass(s);
	else if ( scope == METHOD_SCOPE )
		insertToMethod(s);

	// Checking to see if the symbol is a class or a method
	// If so allocate memory for the table and change the scope of the program
	if ( s.dataType == CLASS || s.dataType == METHOD )
		insertTable();
}


// Loop thorugh the symbols and see if the name already exists
// Return the index of the symbol if it exists
// Else return -1
int findSymbol( char *name ) {
	
	// First check current scope.
	// Then check the scope above ( if method, then class etc. )
	if ( scope == METHOD_SCOPE ) {
		int symbolIndex = findInMethod(name);

		if ( symbolIndex != -1 )
			return symbolIndex;

	} else if ( scope == CLASS_SCOPE ) {
		int symbolIndex = findInClass(name);

		if ( symbolIndex != -1 )
			return symbolIndex;
	} 

	return -1;
}




void closeTable() {
	// ALso have to loop through all symbols freeing the attributes
	for (int i = 0; i < progTable.classCount; ++i) {
		classTable *cClass = progTable.classes + i;

		for (int j = 0; j < cClass->methodCount; ++j) {
			methodTable *cMethod = cClass->methods + j; 

			// Free method level symbol attributes
			for (int k = 0; k < cMethod->symbolCount; ++k) {
				if (cMethod->symbols[k].attr != NULL)
					free( cMethod->symbols[k].attr );
			}
		}

		// Then free the class symbol attributes
		for (int j = 0; j < cClass->symbolCount; ++j) {
			if (cClass->symbols[j].attr != NULL)
				free(cClass->symbols[j].attr);
		}
	}
	
	// Free all the program level symbols
	for (int i = 0; i < progTable.classCount; ++i) {
		if (progTable.symbols[i].attr != NULL)
			free(progTable.symbols[i].attr);
	}

	
	// Loop through the tables freeing up memory allocated for the table arrays
	for (int i = 0; i < progTable.classCount; ++i) { 
		// loop through all classes
		free((progTable.classes + i)->methods); // Free method tables
	}

	free(progTable.classes);
}


static int findInMethod( char *name ) {
	methodTable *currentMethod = getCurrentMethod();

	// Search Prog Level
	for (int i = 0; i < currentMethod->symbolCount; ++i) {
		symbol sym = currentMethod->symbols[i];
		if (!strcmp(sym.name, name))
			return i;
	}

	// Search the class
	int symbolIndex = findInClass( name );
	if (symbolIndex != -1)
		return symbolIndex;

	return -1;
}


static int findInClass( char *name ) {
	classTable *currentClass = getCurrentClass();

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


// Inserting a table into existing tables
void insertTable() {
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
		for ( int i = 0; i < MAX_SYMBOLS; ++i ) {
			currentClass->symbols[i].attr = NULL;
		}
		
		return;
	}

	// Class scope means we're inserting a method
	// If there's already a method inside the table then we don't need to malloc
	currentClass = getCurrentClass();
	if ( currentClass->methodCount == 0 && currentClass != NULL ) {
		// We want to allocate memory to the tables field of the current table
		currentClass->methods = malloc( sizeof(methodTable) * MAX_METHODS );
	}

	currentClass->methodCount ++;
	scope = METHOD_SCOPE;

	currentMethod = getCurrentMethod();
	currentMethod->symbolCount = 0;

	// Set all the symbol attributes to null
	for ( int i = 0; i < MAX_SYMBOLS; ++i ) {
		currentMethod->symbols[i].attr = NULL;
	}
}


static classTable *getCurrentClass() {
	if ( progTable.classCount == 0)
		return NULL;
	return (progTable.classes + (progTable.classCount-1));
}

static methodTable *getCurrentMethod() {
	classTable *currentClass = getCurrentClass();
	if ( currentClass == NULL || currentClass->methodCount == 0 )
		return NULL;
	return (currentClass->methods + currentClass->methodCount-1);
}

int main(void) {
	printf("What's up\n");
	return 0;
}
