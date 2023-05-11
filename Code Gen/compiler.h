#ifndef COMPILER_H
#define COMPILER_H

// #define TEST_COMPILER    // uncomment to run the compiler autograder

#include <stdio.h>
#include "parser.h"
#include "symbols.h"

extern int parseNum;

int InitCompiler ();
ParserInfo compile (char* dir_name);
int StopCompiler();

// Writer functions to emit VM Code
void writePush(FILE*, char*, unsigned int);
void writePop(FILE*, char*, unsigned int);
void writeArith(FILE*, char*);
void writeLabel(FILE*, char*);
void writeGoto(FILE*, char*);
void writeIf(FILE*, char*);
void writeCall(FILE*, char*, unsigned int);
void writeFunc(FILE*, char*, unsigned int);
void writeRet(FILE*);

#endif
