#ifndef SYMBOLS_H
#define SYMBOLS_H

#include "lexer.h"
#include "parser.h"

// Different levels of scope
#define PROG_SCOPE   0
#define CLASS_SCOPE  1
#define METHOD_SCOPE 2


const int MAX_SYMBOLS = 256; // Maxumimum symbols in a table

// define your own types and function prototypes for the symbol table(s) module below
typedef enum { TYPE, VAR, FUNCTION, BAD_TYPE } Type;
typedef enum { INTEGER, PARAM, CLASS, STR, ARRAY, VOID } Kind;
typedef enum { STATIC, FIELD } VarType;

// Structure that holds the attributes of 
typedef struct {
  unsigned int size; 
  Kind kind;
  Kind returnType;
  Kind arrayType;
  VarType varType;

} attributes;

typedef struct {
  char *name;
  Type dataType;
  attributes *attr; // Attributes ( Pointer to further data about the symbol )
} symbol;

// Holds a list of symbols and other tables
// Program table is the top level scope
// Then class level
// Then method level

typedef struct symbolTable symbolTable;

struct symbolTable {
  symbol symbols[MAX_SYMBOLS];
  symbolTable *tables;
  unsigned int symbolCount;
  unsigned int scope; // Keep track of what table you're in
  unsigned int classCount; // Keep track of what class you're in
  unsigned int methodCount; // Keep track of what method you're in
};


void initTable();
void insertSymbol( symbol s ); // Insert takes a symbol and inserts it into the table
void insertTable(); // Insert a table into an existing table
int findSymbol();
void closeTable();

#endif
