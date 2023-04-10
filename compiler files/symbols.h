#ifndef SYMBOLS_H
#define SYMBOLS_H

#include "lexer.h"
#include "parser.h"

// define your own types and function prototypes for the symbol table(s) module below
typedef enum { integer } dataType;

typedef struct {
  char name[128];
  dataType dType;
} symbol;

void initSymTable();
void insertSymbol( char *name, dataType type );
int findSymbol( char *name );

#endif
