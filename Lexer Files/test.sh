#!/bin/bash

# Test script for Lexer (Copied from Michael Nestor)

#Jack Files
jackFiles=(Ball.jack Empty.jack EofInComment.jack EofInStr.jack Fraction.jack
             IllegalSymbol.jack List.jack Main.jack NewLineInStr.jack OnlyComments.jack
             Square.jack SquareGame.jack)

# Corresponding Token Files
jackFiles=(Ball.jack_tokens.jack Empty.jack_tokens.jack EofInComment.jack_tokens.jack
             EofInStr.jack_tokens.jack Fraction.jack_tokens.jack
             IllegalSymbol.jack_tokens.jack List.jack_tokens.jack Main.jack_tokens.jack
             NewLineInStr.jack_tokens.jack OnlyComments.jack_tokens.jack
             Square.jack_tokens.jack SquareGame.jack_tokens.jack)

printf "Building Lexer"
gcc lexer.c -o lexer

index=0
for file in ${jackFiles[@]}; do
   printf "Testing $file\n"
   ./lexer $file output
   diff ${tokenFiles[$index]} output
   index=$(index+1)
done
rm lexer output
