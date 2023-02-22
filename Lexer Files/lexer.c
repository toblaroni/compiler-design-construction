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
FILE *fptr;
FILE *fptr2;


// Trim Whitespace from the input file
// Returns number of newlines encountered
int trimSpace(char *C) {
  char c;
  int nl = 0;
  while (isspace(c)) {
    if (c == '\n')
      nl ++;

    c = getc(fptr);
  }
  *C = c;
  return nl;
}

// Function to remove comments
void rmComments(char *c, Token *t) {
    // Get one more Char
    *c = getc(fptr);

    if (*c == '/') {
        // Line comment -> Refactor to function?
        while(*c != '\n') {
            *c = getc(fptr);
            printf("Getting comment");
        }

        
    } else if (*c == '*') {
        // Multi line comment or API documentation

    } else {
        // It's a slash character
        ungetc(*c, fptr);
        t->lxm[0] = *c;
        t->lxm[1] = '\0';
        t->tt = SYMBOL;
    }
}


// Initialise the lexer to read from source file
// File_name is the name of the src file
int InitLexer (char* file_name) {
  fileName = file_name;
  fptr = fopen(file_name, "r");
  fptr2 = fptr;
  if (!fptr) {
    printf("Error: Bad File Name \"%s\"\n", file_name);
    return 0;
  }
  return 1;
}


// Get the next token from the source file
Token GetNextToken () {
	Token t;
    t.ln = 0; // init line number
    strcpy(t.srcFile, fileName); // Set the filename of source

    char c;

    // Get rid of whitespace
    t.ln += trimSpace(&c);

    char *temp = (char *) malloc(sizeof(char) * 255);

    // Remove comments
    if (c == '/')
        rmComments(&c, &t);
    else {
        strcpy(t.lxm, "End of File");
        t.tt = EOFile;
    }
    free(temp);
    return t;
}


// peek (look) at the next token in the source file without removing it from the stream
Token PeekNextToken () {
  Token t;
  t.tt = ERR;
  return t;
}


// clean out at end, e.g. close files, free memory, ... etc
int StopLexer () {
  fclose(fptr);
  fclose(fptr2);
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
  while (t.tt != ERR && t.tt != EOFile) {
    t = GetNextToken();
    fprintf(fOut, "<%s, %i, %s, %s>\n",
            t.srcFile, t.ln, t.lxm, tokenTypes[t.tt]);
  }

  fclose(fOut);
	return 0;
}
// do not remove the next line
#endif
