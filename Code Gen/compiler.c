/************************************************************************
  University of Leeds
  School of Computing
  COMP2932- Compiler Design and Construction
  The Compiler Module

  I confirm that the following code has been developed and written by me and it is entirely the result of my own work.
  I also confirm that I have not copied any parts of this program from another person or any other source or facilitated someone to copy this program from me.
  I confirm that I will not publish the program online or share it with anyone without permission of the module leader.

  Student Name: Toby Hutchinson
  Student ID: 201530569
Email: sc21t2hh@leeds.ac.uk
Date Work Commenced: 25/04/23
 *************************************************************************/

#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "compiler.h"
#include "parser.h"
#include "symbols.h"

int parseNum = 0;

void writePush(FILE *vmFile, char *seg, unsigned int index) {
	fprintf(vmFile, "push %s %i\n", seg, index);
}
void writePop(FILE *vmFile, char *seg, unsigned int index) {
	fprintf(vmFile, "pop %s %i\n", seg, index);
}
void writeArith(FILE *vmFile, char *cmd) {
	fprintf(vmFile, "%s\n", cmd);
}
void writeLabel(FILE *vmFile, char *label) {
	fprintf(vmFile, "label %s\n", label);
}
void writeGoto(FILE *vmFile, char *label) {
	fprintf(vmFile, "goto %s\n", label);
}
void writeIf(FILE *vmFile, char *label) {
	fprintf(vmFile, "if-goto %s\n", label);
}
void writeCall(FILE *vmFile, char *name, unsigned int nArgs) {
	fprintf(vmFile, "call %s %i\n", name, nArgs);
}
void writeFunc(FILE *vmFile, char *name, unsigned int nLocal) {
	fprintf(vmFile, "function %s %i\n", name, nLocal);
}
void writeRet(FILE *vmFile) {
	fprintf(vmFile, "return\n");
}

int InitCompiler() {
	// One table for the entire compile process
	initTable();
	return 1;
}

ParserInfo compile(char* dir_name) {
	// Standard libraries
	static char * stdLibs[8] = { "Array.jack", "Keyboard.jack", "Math.jack", "Memory.jack", 
								 "Output.jack", "Screen.jack", "String.jack", "Sys.jack" };

	ParserInfo p;
	// Parse all of the standard libraries
	for (int i = 0; i < 8; ++i) {
		if (!InitParser(stdLibs[i])) {
			printf("Error initialising standard lib %s\n", stdLibs[i]);
			exit(-1);
		}
		p = Parse();
		if (p.er) return p;
		StopParser();
	}

	p.er = none;
	static char currentFile[32];

	// Directory entry pointer
	struct dirent *dirEntr;

	// Loop through the files in the directory
	DIR *dir = opendir(dir_name);
	if (dir == NULL) {
		printf("Failed to open directory %s\n", dir_name);
		exit(0);
	}

	while ( 1 ) {
		dirEntr = readdir(dir);
		if (dirEntr == NULL) {
			if (parseNum) break;
			closedir(dir);
			dir = opendir(dir_name);
			dirEntr = readdir(dir);
			parseNum++;
		}

		char *fileName = dirEntr->d_name;
		char *substr = strstr(fileName, ".vm");
		if (!strcmp(fileName, ".") || !strcmp(fileName, "..") || substr)
			continue;

		// Set the compiler to open at "./<dir_name>/<file_name> 
		strcpy(currentFile, "./");
		strcat(currentFile, dir_name);
		strcat(currentFile, "/");

		// Concatencate the name of the file at the end of the folder 
		strcat(currentFile, fileName);
		if (!InitParser(currentFile))
			continue;

		p = Parse();
		if (p.er) {
			closedir(dir);
			return p;
		}

		StopParser();
	}

	closedir(dir);
	return p;
}

int StopCompiler () {
	closeTable();
	return 1;
}

void PrintError( ParserInfo pi ) {
	// Error messages in order of parser info enum
	static const char* parserMsgs[18] = { "File Successfully compiled with no errors",
		"lexical error.", "keyword class expected", "identifier expected", "{ expected", "} expected",
		"class member declaration must begin with static, field, constructor, function, or method",
		"class variables must begin with field or static",
		"a type must be int, char, boolean or identifier", "; expected",
		"subroutine declaration must begin with constructor, function, or method",
		"( expected", ") expected",
		"] expected", "= expected", "other syntax error",
		"undeclared identifier (e.g. class, subroutine or variable",
		"redeclaration of identifier in the same scope" };

	static const char* lexerMsgs[5] = {"End of file in comment", "New line in string",
		"End of file in string", "Illegal symbol", "No Lexical errors"};

	if (!pi.er)
		printf("%s\n", parserMsgs[pi.er]);
	else if (pi.tk.tp == ERR) 
		printf("Lexical error while compiling %s. %s. %s at line %i\n", pi.tk.fl, lexerMsgs[pi.tk.ec],
				pi.tk.lx, pi.tk.ln);
	else 
		printf("Error while parsing %s. line %i, close to %s, %s.\n", pi.tk.fl, pi.tk.ln, pi.tk.lx, parserMsgs[pi.er]);
}

#ifndef TEST_COMPILER
int main( int argv, char **argc ) {
	if (argv != 2) {
		printf("Usage: './compile <folder>'\n");
		exit(1);
	}

	InitCompiler();
	ParserInfo p = compile(argc[1]);
	PrintError(p);
	StopCompiler();
	return 1;
}
#endif
