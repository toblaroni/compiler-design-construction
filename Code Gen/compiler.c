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
	// Standard libraries
	static char * stdLibs[8] = { "Array.jack", "Keyboard.jack", "Math.jack", "Memory.jack", 
								 "Output.jack", "Screen.jack", "String.jack", "Sys.jack" };

	// Parse all of the standard libraries
	for (int i = 0; i < 8; ++i) {
		if (!InitParser(stdLibs[i]))
			printf("Failed to compile standard lib %s\n", stdLibs[i]);
		Parse();
		StopParser();
	}

	ParserInfo p;
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
		if (!InitParser(currentFile))
			continue;

		p = Parse();
		if (p.er) {
			closedir(dir);
			return p;
		}

		StopParser();
	}

	p = undeclSymCheck();
	
	closedir(dir);
	return p;
}

int StopCompiler () {
	closeTable();
	return 1;
}
