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

int InitCompiler() {
	// One table for the entire compile process
	initTable();
	return 1;
}

ParserInfo compile(char* dir_name) {
	ParserInfo p;
	p.er = none;
	static char currentFile[256];

	// Directory entry pointer
	struct dirent *dirEntr;
	
	// Loop through the files in the directory
	DIR *dir = opendir(dir_name);
	if (dir == NULL) {
		printf("Failed to open directory %s\n", dir_name);
		exit(0);
	}


	while ( (dirEntr = readdir(dir)) != NULL ) {
		char *fileName = dirEntr->d_name;
		if (!strcmp(fileName, ".") || !strcmp(fileName, ".."))
			continue;

		// Set the compiler to open at "./<dir_name>/<file_name> 
		strcpy(currentFile, "./");
		strcat(currentFile, dir_name);
		strcat(currentFile, "/");

		// Concatencate the name of the file at the end of the folder 
		strcat(currentFile, fileName);
		printf("Parsing %s\n", currentFile);
		if (!InitParser(currentFile))
			exit(-1);

		p = Parse();
		if (p.er)
			break;

		StopParser();
		printf("Successfully parsed %s.\n", currentFile);
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
	static const char* errorMsgs[18] = { "File Successfully compiled with no errors",
										 "lexical error.", "keyword class expected",
										 "identifier expected", "{ expected", "} expected",
										 "class member declaration must begin with static, field, constructor, function, or method",
										 "class variables must begin with field or static",
										 "a type must be int, char, boolean or identifier",
										 "( expected", ") expected", "] expected", "= expected",
										 "other syntax error", "undeclared identifier (e.g. class, subroutine or variable",
										 "redeclaration of identifier in the same scope" };

	if (!pi.er)
		printf("%s\n", errorMsgs[pi.er]);
	else if (pi.tk.tp == ERR) 
	 	printf("%s at line %i\n", pi.tk.lx, pi.tk.ln);
	else 
		printf("Error, line %i, close to %s, %s.\n", pi.tk.ln, pi.tk.lx, errorMsgs[pi.er]);
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
