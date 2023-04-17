#ifndef SYMBOLS_H
#define SYMBOLS_H

#include "lexer.h"
#include "parser.h"

// Different levels of scope
#define PROG_LVL   0
#define CLASS_LVL  1
#define METHOD_LVL 2

// define your own types and function prototypes for the symbol table(s) module below
const typedef enum { TYPE, VAR, FUNCTION, BAD_TYPE } Type;
const typedef enum { INTEGER, PARAM, CLASS, STR, ARRAY, VOID } Kind;
const typedef enum { STATIC, FIELD } VarType;

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
  symbol      *symbols;
  symbolTable *tables;
  unsigned int symbolCount;
  unsigned int scopeLevel; // Keep track of what table you're in

};


void initTable();
void insertSymbol(); // Insert takes a symbol and inserts it into the table
void insertTable(); // Insert a table into an existing table
int findSymbol();
void closeTable();

#endif
