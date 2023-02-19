/************************************************************************
University of Leeds
School of Computing
COMP2932- Compiler Design and Construction
Lexer Module

I confirm that the following code has been developed and written by me and it is entirely the result of my own work.
I also confirm that I have not copied any parts of this program from another person or any other source or facilitated someone to copy this program from me.
I confirm that I will not publish the program online or share it with anyone without permission of the module leader.

Student Name: Toby Hutchinson
Student ID: sc21t2hh
Email: sc21t2hh@leeds.ac.uk
Date Work Commenced: 18/02/23
*************************************************************************/


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"


// YOU CAN ADD YOUR OWN FUNCTIONS, DECLARATIONS AND VARIABLES HERE

const char *tokenTypes[7] = {"RESWORD", "ID", "INT", "SYMBOL", "STRING", "EOFile", "ERR"};

char *fileName;
// Main file pointer and secondary pointer for peakToken
FILE *f;
FILE *f2;


// Initialise the lexer to read from source file
// file_name is the name of the source file
// This requires opening the file and making any necessary initialisations of the lexer
// If an error occurs, the function should return 0
// if everything goes well the function should return 1
int InitLexer (char* file_name)
{
  fileName = file_name;
  f = fopen(file_name, "r");
  if (!f) {
    printf("Error: Bad File Name \"%s\"\n", file_name);
    return 0;
  }
  return 1;
}


// Get the next token from the source file
Token GetNextToken ()
{
	Token t;
  t.ln = 5;
  strcpy(t.fileName, fileName);
  strcpy(t.lxm, "Test");
  t.tt = ERR;
  return t;
}

// peek (look) at the next token in the source file without removing it from the stream
Token PeekNextToken ()
{
  Token t;
  t.tt = ERR;
  return t;
}

// clean out at end, e.g. close files, free memory, ... etc
int StopLexer ()
{

	return 0;
}

// do not remove the next line
#ifndef TEST
int main (int argc, char **argv)
{
  int openFile = InitLexer(argv[1]);
  if (openFile == 0)
    return -1;
  
  FILE *fOut = fopen(argv[2], "w");
  Token t;
  while (t.tt != ERR) {
    t = GetNextToken();
    fprintf(fOut, "< %s, %i, %s, %s >\n",
            t.fileName, t.ln, t.lxm, tokenTypes[t.tt]);
  }

  fclose(fOut);
	return 0;
}
// do not remove the next line
#endif
