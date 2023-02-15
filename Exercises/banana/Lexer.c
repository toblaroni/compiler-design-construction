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

unsigned short keywordNum = 12;
const char* keywords[12] = {"bello", "minion", "banana", "gelato", "bapple", "bee", "do", "stopa", "para", "tu", "bank", "yu"};

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
	Token t = {};
	// consume any leading white space or comments
	int c;
	c = getc(f);
	while (isspace(c))
	{
		c = getc(f);
	}

    // Comments
    while (c == '!' || c == '\n' || c == ' ') 
    {
        while (c != '\n') {
            c = getc(f);
        }
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

        strcpy(t.x, temp);

        for(int i = 0; i < keywordNum; ++i) {
            if (!strcmp(temp, keywords[i])) {
                t.t = keyword;
                return t;
            }
        }
        t.t = id;
        
	}
	else if (isdigit(c))
	{
        while (isdigit(c) || c == 'p' || c == 'b') {
			t.x[i] = c;
			i++;
			c = getc(f);
        }
        t.x[i] = '\0';
        t.t = num;
	}
	else
	{
        t.t = sym;
        t.x[0] = c;
        t.x[1] = '\0';
	}
    return t;
}

int main()
{
	if (!Init())
        printf("Bad file name\n");

    // For printing
    const char *Tokens[6] = {"keyword", "id", "num", "sym", "eof", "err"};
    
    Token t;

    while (t.t != eof) {
        t = GetToken();
        printf("<%s, %s>\n", Tokens[t.t], t.x);
    }
}
