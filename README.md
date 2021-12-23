# HYU-Compiler
2021 fall ELE4029   
컴파일러설계

## Project 1 - Scanner
> c-minus scanner implementation(2 methods)   
>  1. recognize tokens by DFA   
>  code : scan.c    
>  exe file : cminus_cimpl     
>  2. specify lexical patterns by regular expression( lex(flex) )    
>  code : cminus.l   
>  exe file : cminus_lex

## Project 2 - Parser
> c-minus parser implementation using Yacc(bison)   
> Parser read an input source code, tokenize and parse it   
> with c-minus grammar , and return abstract syntax tree(AST)   
> exe file : cminus_parser

## Project 3 - Semantic analysis(symbol table & type checker)   
> semantic analyzer traverses the AST(from project 2) and prints semantic errors   
> exe file : ciminus_semantic
