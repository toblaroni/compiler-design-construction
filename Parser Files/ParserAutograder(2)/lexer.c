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

#define MAX_LEXEME_LENGTH 128

const char *tokenTypes[7] = { "RESWORD", "ID", "INT", "SYMBOL", "STRING", "EOFile", "ERR" };
const char symbols[19] = { '(', ')', '{', '}', '[', ']',
                           ',', ';', '=', '.', '+', '-',
                           '*', '/', '&', '|', '<', '>', '~' };

// Array of keywords
const char *keywords[21] = {"class", "constructor", "method", "function", "int", "boolean", "char", "void", "var", "static", "field", "let", "do", "if", "else", "while", "return", "true", "false", "null", "this"};

char *fileName;

// Main file pointer and secondary pointer for peakToken
FILE *fptr;

// Global line number variable
int lineNumber;


// Function for checking if it's the end of the file after a newline
// If it's not the end of the file then update lineNumber
void peekNl(char *c) {
    if (*c == '\n') {
        *c = getc(fptr);
        lineNumber++;
    } else if (*c == EOF)
        lineNumber++;
}

void mLineComments(Token *t, char *c) {
    // Multi line / Api comment
    while (1) {
        *c = getc(fptr);
        if (*c == EOF) {
            strcpy(t->lx, "Error: unexpected eof in comment");
            t->ln = lineNumber;
            t->ec = EofInStr;
            t->tp = ERR;
            return;
        }

        if (*c == '\n') {
            lineNumber++;
            continue;
        }

        if (*c != '*')
            continue;

        // Get another character
        *c = getc(fptr); 
        if (*c != '/') {
            ungetc(*c, fptr);
            continue;
        }
        break;
    }

    *c = getc(fptr);
}

// ************ REFACTOR PLS ************
// Function to remove comments
void rmComments(Token *t, char *c) {
    // Get one more Char
    char C = getc(fptr);

    if (C == '/') {
        // Single line comment
        while(*c != '\n') {
            *c = getc(fptr);
        }
        return;
    } else if (C == '*') {
        // Multi Line comments
        mLineComments(t, c);
        return;
    }

    // Else it's a '/' symbol
    t->ln    = lineNumber;
    t->lx[0] = *c;
    t->lx[1] = '\0';
    t->tp    = SYMBOL;

}


void getId(Token *t, char c) {
    int i = 0;
    char temp[MAX_LEXEME_LENGTH];

    // Loop until you find a character that can't be in an ID
    while ( (isalnum(c) || c == '_')  && i < MAX_LEXEME_LENGTH) {
        temp[i] = c;
        c = getc(fptr);
        i ++;
    } 
    ungetc(c, fptr);
    temp[i] = '\0'; 
    t->ln = lineNumber;

    // Check if it's a reserved word
    for (int i = 0; i < 21; ++i) {
        if (strcmp(keywords[i], temp) == 0) {
            t->tp = RESWORD;
            strcpy(t->lx, temp);
            return;
        }
    }

    // We got an ID
    t->tp = ID;
    strcpy(t->lx, temp);
}

void getBadStr(Token *t, char c) {
    if (c == '\n') {
        strcpy(t->lx, "Error: new line in string constant");
        t->ec = NewLnInStr;
        t->tp = ERR;
        t->ln = lineNumber; 
        return;
    } else if (c == EOF) {
        strcpy(t->lx, "Error: unexpected eof in string constant");
        t->ec = EofInStr;
        t->tp = ERR;
        t->ln = lineNumber; 
        return;
    }
}


void getStr(Token *t) {
    int i = 0;
    char temp[MAX_LEXEME_LENGTH];
    char c = getc(fptr);

    while (1) {
        temp[i] = c;    
        c = getc(fptr);

        // Check for Errors in the string
        if (i >= MAX_LEXEME_LENGTH  - 1) {
            // NEEDS TO BE FULLY IMPLEMENTED
            // Truncate the string
            temp[i+1] = '"';
            temp[i+2] = '\0';
            strcpy(t->lx, temp);
            t->tp = STRING;
            t->ln = lineNumber; 
            return;

        } else if (c == '\n' || c == EOF) {
            getBadStr(t, c);
            return;

        } else if (c == '"') {
            temp[i+1] = '\0';
            strcpy(t->lx, temp);
            t->tp = STRING;
            t->ln = lineNumber; 
            return;
        }
        i++;
    }
}


// Get an integer token
void getInt(Token *t, char c) {
    char temp[MAX_LEXEME_LENGTH];
    int i = 0;

    while (isdigit(c)) {
        temp[i] = c;
        c = getc(fptr);
        i++;

        if (i >= MAX_LEXEME_LENGTH -1) {
            // Truncate the lexeme
            temp[i] = '\0';
            strcpy(t->lx, temp);
            t->ln = lineNumber; 
            t->tp = INT;
            ungetc(c, fptr);
            return;
        }
    }

    temp[i] = '\0';
    strcpy(t->lx, temp);
    t->ln = lineNumber; 
    t->tp = INT;
    ungetc(c, fptr);
}


void getSym(Token *t, char c) {
    for (int i = 0; i < 19; ++i) {
        if (symbols[i] != c)
            continue;

        t->lx[0] = c;
        t->lx[1] = '\0';
        t->ln    = lineNumber;
        t->tp    = SYMBOL;
        return;
    }
    strcpy(t->lx, "Error: illegal symbol in source file");
    t->ec    = IllSym;
    t->tp    = ERR;
    t->ln    = lineNumber;
    ungetc(c, fptr);
}

// Initialise the lexer to read from source file
// File_name is the name of the src file
int InitLexer (char* file_name) {
    fileName = file_name;
    lineNumber = 1;

    fptr = fopen(file_name, "r");
    if (!fptr) {
        printf("Error: Bad File Name \"%s\"\n", file_name);
        return 0;
    }
    return 1;
}


// Consume whitespace
void trimSpace(char *c) {
    while (isspace(*c)) {
        if (*c == '\n')
            lineNumber ++;
        *c = getc(fptr);
    }
}


// Get the next token from the source file
Token GetNextToken () {
	Token t = {};
    strcpy(t.fl, fileName); // Set the filename of source

    char c = getc(fptr);

    // Remove whitespace
    trimSpace(&c);

    // Remove comments 
    while (c == '/') {
        rmComments(&t, &c);
        trimSpace(&c);
        if (t.tp == SYMBOL || t.tp == ERR)
            return t;
    }


    if (c == EOF) {
        strcpy(t.lx, "End of File");
        t.tp = EOFile;
        t.ln = lineNumber;
        return t;
    }
        
    // Handle identifiers and keywords
    if (isalpha(c) || c == '_') {
        getId(&t, c);
    } else if (c == '"') {
        getStr(&t); // Get strings
    } else if (isdigit(c)) {
        getInt(&t, c); // Integers
    } else 
        getSym(&t, c); // Symbols

    peekNl(&c);
    return t;
}


// peek (look) at the next token in the source file without removing it from the stream
Token PeekNextToken () {
    fpos_t p; // make a note of the current position in the file
    fgetpos(fptr, &p);
    int oldLn = lineNumber;  // make a note of the line number

    Token t = GetNextToken();
    
    fsetpos(fptr, &p); // Set the position back to the previous position
    lineNumber = oldLn;
    return t;
}


// clean out at end, e.g. close files, free memory, ... etc
int StopLexer () {
    fclose(fptr);
	return 0;
}


/*
// do not remove the next line
#ifndef TEST
int main (int argc, char **argv)
{
    int openFile = InitLexer(argv[1]);
    if (openFile == 0)
        return -1;

    FILE *fOut = fopen(argv[2], "w");

    Token t, t2;
    int i = 0;
    while (t.tp != EOFile && t.tp != ERR) {
        t = GetNextToken();
        t2 = PeekNextToken();
        fprintf(fOut, "Token < %s, %i, %s, %s >, Next token < %s, %i, %s, %s > \n",
                t.fl, t.ln, t.lx, tokenTypes[t.tp],
                t2.fl, t2.ln, t2.lx, tokenTypes[t2.tp]);
        i++;
    }

    fclose(fOut);
    return 0;
}
// do not remove the next line
#endif
*/
