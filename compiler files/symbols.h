#ifndef SYMBOLS_H
#define SYMBOLS_H

#include "lexer.h"
#include "parser.h"

// Different levels of scope
#define PROG_SCOPE   0
#define CLASS_SCOPE  1
#define METHOD_SCOPE 2
#define IS_INIT  1
#define NOT_INIT 0

 #define MAX_SYMBOLS 256 // Maxumimum symbols in a table

// define your own types and function prototypes for the symbol table(s) module below
typedef enum { CLASS, VAR, METHOD, BAD_TYPE } Type;
typedef enum { INTEGER, CHAR, BOOL, TYPE, ARG, STR, ARRAY, VOID, CONSTRUCTOR, BAD_KIND } Kind;
typedef enum { STATIC, FIELD } VarType;


// Structure that holds the attributes of 
typedef struct {
  unsigned int size; 
  Kind kind;
  Kind returnType;
  Kind arrayType;
  VarType varType;
  unsigned int paramCount; // Number of parameters
  short isInit; // Is it initialised
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
typedef struct programTable programTable;
typedef struct classTable classTable;
typedef struct methodTable methodTable;

// Has an array of class tables
struct programTable {
  symbol symbols[MAX_SYMBOLS]; // For holding class names
  classTable *classes;
  unsigned int classCount; // Keep track of what class you're in
};

// Has an array of methods
struct classTable {
  symbol symbols[MAX_SYMBOLS];
  methodTable *methods;
  unsigned int symbolCount;
  unsigned int methodCount; // Keep track of what method you're in
};

// Has an array of symbols
struct methodTable {
  symbol symbols[MAX_SYMBOLS];
  unsigned int symbolCount;
};

void initTable();
void insertSymbol( symbol s ); // Insert takes a symbol and inserts it into the table
int findSymbol( char *name );
void closeTable();
void changeScope( unsigned int newScope );

#endif
