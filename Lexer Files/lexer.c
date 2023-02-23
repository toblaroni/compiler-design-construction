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

// Array of keywords
const char *keywords[21] = {"class", "constructor", "method", "function",
                            "int", "boolean", "char", "void", "var",
                            "static", "field", "let", "do", "if", "else",
                            "while", "return", "true", "false", "null",
                            "this"};

char *fileName;

// Main file pointer and secondary pointer for peakToken
FILE *fptr;
FILE *fptr2;

// Global line number variable
int lineNumber;


// Function to remove comments
void rmComments(Token *t) {
    // Get one more Char
    char c = getc(fptr);

    switch (c) {
        case '/': 
        {
            // Single line comment
            printf("Getting Single line comment\n");
            while(c != '\n') {
                c = getc(fptr);
            }
            break;
        } 
        case '*':
        {
            // Multi line / Api comment
            printf("Getting Multi Line comment\n");
            break;
        }
        default:
        {
            // It's a slash character
            ungetc(c, fptr);
            t->lx[0] = c;
            t->lx[1] = '\0';
            t->tp = SYMBOL;
        }
    }
}


// Initialise the lexer to read from source file
// File_name is the name of the src file
int InitLexer (char* file_name) {
  fileName = file_name;
  lineNumber = 1;

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
    strcpy(t.fl, fileName); // Set the filename of source

    char c = getc(fptr);

    // Get rid of whitespace and update line number accordingly
    while (isspace(c)) {
        if (c == '\n')
            lineNumber ++;
        printf("Getting whitespace\n");
        c = getc(fptr);
    }

    printf("Current char = %c, Current line number = %i\n", c, lineNumber);

    char *temp = (char *) malloc(sizeof(char) * 255);

    // Remove comments
    if (c == '/')
        rmComments(&t);

    // Check for end of file
    c = getc(fptr);
    if (c == EOF) {
        strcpy(t.lx, "End of File");
        t.tp = EOFile;
    }

    lineNumber ++; // Update to account for newline after collecting comments
        

    t.ln = lineNumber;
    free(temp);
    return t;
}


// peek (look) at the next token in the source file without removing it from the stream
Token PeekNextToken () {
  Token t;
  t.tp = ERR;
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
    int i = 0;
    while (t.tp != EOFile) {
        t = GetNextToken();
        fprintf(fOut, "<%s, %i, %s, %s>\n",
                t.fl, t.ln, t.lx, tokenTypes[t.tp]);
        i++;
    }

    fclose(fOut);
    return 0;
}
// do not remove the next line
#endif
