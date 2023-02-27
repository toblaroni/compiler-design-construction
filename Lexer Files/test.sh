#!/bin/bash

# Test script for Lexer (Copied from Michael Nestor)
#Jack Files
jackFiles=(Ball.jack Empty.jack EofInComment.jack EofInStr.jack Fraction.jack
             IllegalSymbol.jack List.jack Main.jack NewLineInStr.jack OnlyComments.jack
             Square.jack SquareGame.jack)

# Corresponding Token Files
tokenFiles=(Ball.jack_tokens.txt Empty.jack_tokens.txt EofInComment.jack_tokens.txt
             EofInStr.jack_tokens.txt Fraction.jack_tokens.txt
             IllegalSymbol.jack_tokens.txt List.jack_tokens.txt Main.jack_tokens.txt
             NewLineInStr.jack_tokens.txt OnlyComments.jack_tokens.txt
             Square.jack_tokens.txt SquareGame.jack_tokens.txt)

printf "Building Lexer"
gcc lexer.c -o lexer

index=0
for file in ${jackFiles[@]}; do
   printf "Testing $file\n"
   ./lexer $file output
   diff ${tokenFiles[$index]} output
   index=$((index+1))
done
rm lexer output
