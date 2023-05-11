#ifndef SYMBOLS_H
#define SYMBOLS_H

#include "lexer.h"
#include "parser.h"

// Different levels of scope
extern int PROG_SCOPE;
extern int CLASS_SCOPE;
extern int METHOD_SCOPE;

// Is initialised flag
extern int IS_INIT;
extern int NOT_INIT;

// Search local or search class flags for finding symbols
extern int LOCAL_SEARCH;
extern int CLASS_SEARCH;
extern int PROG_SEARCH;

extern int MAX_SYMBOLS; // Maxumimum symbols in a table

// define your own types and function prototypes for the symbol table(s) module below
typedef enum { CLASS, VAR, SUB, BAD_TYPE } Type;
typedef enum { INTEGER, CHAR, BOOL, TYPE, VOID, BAD_KIND } Kind;
typedef enum { CONSTRUCTOR, METHOD, FUNCTION } SubType;
typedef enum { STATIC, FIELD, ARG } VarType;

// Structure that holds the attributes of 
typedef struct {
  unsigned int size; 
  Kind kind;
  Kind returnType;
  SubType subType;
  VarType varType;
  char belongsTo[32];
} attributes;


typedef struct {
  char *name;
  Type dataType;
  unsigned int index;  // Index of symbol in its respected memory segment
  attributes *attr; // Attributes ( Pointer to further data about the symbol )
} symbol;


typedef struct {
  Token tkns[128];
  unsigned int uSymCount; 
} undeclSymbols;


// Holds a list of symbols and other tables
// Program table is the top level scope
// Then class level
// Then method level
typedef struct programTable programTable;
typedef struct classTable classTable;
typedef struct methodTable methodTable;

// Has an array of class tables
struct programTable {
  symbol symbols[128]; // For holding class names
  classTable *classes;
  undeclSymbols *uSymTable;
  unsigned int classIndex;
  unsigned int classCount; // Keep track of what class you're in
};

// Has an array of methods
struct classTable {
  symbol symbols[128];
  methodTable *methods;
  unsigned int symbolCount;
  unsigned int methodCount; // Keep track of what method you're in
  unsigned int methodIndex;
};

// Has an array of symbols
struct methodTable {
  symbol symbols[128];
  unsigned int symbolCount;
};


void initTable();
void insertSymbol( symbol s ); // Insert takes a symbol and inserts it into the table
int findSymbol( char *name, unsigned int flag );
symbol * getSymbol( char *name ); // Returns the symbol with that name
void closeTable();
void changeScope( unsigned int newScope );
unsigned int indexOf( VarType );
unsigned int getNLocals( char*, char* ); // Gets the number of local symbos
unsigned int getArgc( char*, char* ); // Get arg Count
int findSymbolInClass( char*, char*, char* );

#endif
