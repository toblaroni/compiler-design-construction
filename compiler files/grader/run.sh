gcc -g -Wall -std=c99 parser.c symbols.c lexer.c compiler.c -o compiler && ./compiler $1
