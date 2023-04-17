
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

// Initial number of tables and symbols in a table
// If more need to be added they can be reallocated
const int INIT_SYM_COUNT = 256;

// Is the global scope which contains the class level tables
symbolTable programTable;

// Initialise the program table.
void initSymTable() {
	// Initialise the symbols
	programTable.symbols = malloc( sizeof(symbol) * INIT_SYM_COUNT );

	if ( programTable.symbols == NULL ) {
		printf("COMPILER ERROR: Failed to allocate symbols\n");
		exit(-1);
	}

	// Set the tables to null becuase there's currently no tables
	// When we encounter a class then we allocate memory for that table
	programTable.tables = NULL;

	// Set the symbolCount and tableCount to 0
	programTable.symbolCount = 0;
	programTable.scopeLevel  = PROG_LVL;
}

void insertSymbol() {
}

// Loop thorugh the symbols and see if the name already exists
// Return the index of the symbol if it exists
// Else return -1
int findSymbol() {
	return 0;
}

void closeTable() {
}
