#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

typedef enum TT {keyword, id, num, sym, eof, err} TokenTypes;

typedef struct {
	char x[128];
	TokenTypes t;
} Token;

FILE* f;

const char* KeyWords[1] = { "bello" };

int Init()
{
	f = fopen("bob.txt", "r");
	if (f == 0)
	{
		printf("Error: can't open file\n");
		return 0; // 0 means error
	}
	return 1; // sucess
}

Token GetToken()
{
	Token t;
	// consume any leading white space or comments
	int c;
	c = getc(f);
	while (isspace(c))
	{
		c = getc(f);
	}

	if (c == EOF)
	{
		strcpy (t.x , "EOF");
		t.t = eof;
		return t;
	}
	char temp[128];
	int i = 0;

	if (isalpha(c))
	{
		while (isalpha(c))
		{
			temp[i] = c;
			i++;
			c = getc(f);
		}
		temp[i] = '\0';

		// try to find the lexeme in the array of tokens
		// if found as a keyword
		// return the token with type = keyword
		// else return the token with type = id
	}
	else if (isdigit(c))
	{
		// extract the number token and return it
	}
	else
	{
		// it is a symbol
		// wrap it in a token and return 
	}
}

int main()
{
	Init();
}
