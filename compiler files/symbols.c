
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

#define MAX_SYMBOL_COUNT 128

symbol symbolTable[MAX_SYMBOL_COUNT]; // Symbol table is an array of symbols
int symbolCount; // Number of symbols

// Functions for inserting a symbol, finding a symbol and initialising the symbol Table
void initSymTable() {
	symbolCount = 0;
}

void insertSymbol( char *name, dataType type ) {
	symbolTable[symbolCount].dType = type;
	strcpy( symbolTable[symbolCount].name, name );
	symbolCount ++;
}

// Loop thorugh the symbols and see if the name already exists
// Return the index of the symbol if it exists
// Else return -1
int findSymbol( char *name ) {
	
	// Loop through array checking if the name already exists
	for (int i = 0; i < symbolCount; ++i) {
		if ( !strcmp(symbolTable[i].name, name) )
			return i; // Name already exists
	}
	
	return -1;
}
