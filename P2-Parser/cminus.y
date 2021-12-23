/****************************************************/
/* File: tiny.y                                     */
/* The TINY Yacc/Bison specification file           */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/
%{
#define YYPARSER /* distinguishes Yacc output from other code files */

#include "globals.h"
#include "util.h"
#include "scan.h"
#include "parse.h"

#define YYSTYPE TreeNode *
static char * savedName; /* for use in assignments */
static int savedNum;
static int savedLineNo;  /* ditto */
static TreeNode * savedTree; /* stores syntax tree for later return */
static int yylex(void); // added 11/2/11 to ensure no conflict with lex
static int yyerror(char * mesesage);

%}

%token IF ELSE WHILE RETURN INT VOID
%token ID NUM 
%left  PLUS MINUS
%left  TIMES OVER
%token ASSIGN EQ NE LT LE GT GE LPAREN RPAREN LBRACE RBRACE LCURLY RCURLY SEMI COMMA 
%token ERROR

%nonassoc NIF
%nonassoc ELSE

%% /* Grammar for TINY */

program     : declaration_list
                 { savedTree = $1;}
	    ;

id	: ID {savedName = copyString(tokenString);}
   	;
num	: NUM {savedNum = atoi(tokenString);}
    	;

declaration_list	: declaration declaration_list
		 	{ YYSTYPE t = $1;
                   		if (t != NULL)
                   		{ while (t->sibling != NULL)
                        		t = t->sibling;
                     		t->sibling =$2;
                     		$$ = $1; }
                     		else $$ = $2;
                 	}
			| declaration {$$ =$1;}
			;

declaration		: var_declaration {$$=$1;}
			| fun_declaration {$$=$1;}
			;

var_declaration		: INT id SEMI
		 	{ $$ = newDecNode(VarK);
			  $$ -> attr.name = savedName;
			  $$ -> type = Integer; 
			}
			| VOID id SEMI
			{ $$ = newDecNode(VarK);
			  $$ -> attr.name = savedName;
			  $$ -> type = Void;
			}
		 	| INT id LBRACE num RBRACE SEMI
			{ $$ = newDecNode(VarArrK);
			  $$ -> attr.name = savedName;
			  $$ -> type = Integer;
			  YYSTYPE arrsize = newExpNode(ConstK);
			  arrsize -> attr.val = savedNum;
			  $$ -> child[0] = arrsize;
			}
			| VOID id LBRACE num RBRACE SEMI
                        { $$ = newDecNode(VarArrK);
                          $$ -> attr.name = savedName;
			  $$ -> type = Void;
                          YYSTYPE arrsize = newExpNode(ConstK);
                          arrsize -> attr.val = savedNum;
                          $$ -> child[0] = arrsize;
                        }
			;

fun_declaration		: INT id
		 	{ $$ = newDecNode(FuncK);
			  $$ -> attr.name = savedName;
			  $$ -> type = Integer;
			}
			LPAREN params RPAREN compound_stmt
			{ $$ = $3;
			  $$ -> child[0] = $5;
			  $$ -> child[1] = $7;
			}
			| VOID id
			{ $$ = newDecNode(FuncK);
			  $$ -> attr.name = savedName;
			  $$ -> type = Void;
			}
			LPAREN params RPAREN compound_stmt
			{ $$ = $3;
			  $$ -> child[0] = $5;
			  $$ -> child[1] = $7;
			}
			;

params		: param_list
		{ $$ = $1;}
		| VOID 
		{ $$ = newDecNode(Param1K);
		  $$ -> type = Void; /*void param*/
		  $$ -> attr.name = NULL;
		}
		;

param_list	: param_list COMMA param
	   	{ YYSTYPE t = $1;
                  if (t != NULL)
                  { while (t->sibling != NULL)
                           t = t->sibling;
                    t->sibling = $3;
                    $$ = $1; }
                    else $$ = $3;
                }
		| param { $$=$1;}
		;

param		: INT id
       		{$$ = newDecNode(Param1K);
		 $$ -> attr.name = savedName;
		 $$ -> type = Integer;
		}
       		| VOID id
		{$$ = newDecNode(Param1K);
		 $$ -> attr.name = savedName;
		 $$ -> type = Void;
		}
		| INT id LBRACE RBRACE
		{$$ = newDecNode(Param2K);
		 $$ -> attr.name = savedName;
		 $$ -> type = Integer;
		}
		| VOID id LBRACE RBRACE
		{$$ = newDecNode(Param2K);
		 $$ -> attr.name = savedName;
		 $$ -> type = Void;
		}
		;

compound_stmt	: LCURLY local_declarations statement_list RCURLY
	      	{$$ = newStmtNode(CompK);
		 $$ -> child[0] = $2;
		 $$ -> child[1] = $3;
		}
		;

local_declarations	: local_declarations var_declaration
		   	{ YYSTYPE t = $1;
                                if (t != NULL)
                                { while (t->sibling != NULL)
                                        t = t->sibling;
                                t->sibling = $2;
                                $$ = $1; }
                                else $$ = $2;
                        }
		   	| {$$ = NULL;} /*empty*/
			;

statement_list	: statement_list statement
	       	{ YYSTYPE t = $1;
                                if (t != NULL)
                                { while (t->sibling != NULL)
                                        t = t->sibling;
                                t->sibling = $2;
                                $$ = $1; }
                                else $$ = $2;
                        }

	       	|{$$=NULL;}
		;

statement	: expression_stmt{$$=$1;}
	  	| compound_stmt {$$=$1;}
		| selection_stmt {$$=$1;}
		| iteration_stmt{$$=$1;}
		| return_stmt{$$=$1;}
		;

expression_stmt	: expression SEMI {$$=$1;}
		| SEMI {$$=NULL;}
		;

selection_stmt	: IF LPAREN expression RPAREN statement %prec NIF
	       	{$$ = newStmtNode(IfK);
		 $$ -> child[0] = $3;
		 $$ -> child[1] = $5;
		 $$ -> child[2] = NULL;
		}
		| IF LPAREN expression RPAREN statement ELSE statement
		{$$ = newStmtNode(IfK);
		 $$ -> child[0] = $3;
		 $$ -> child[1] = $5;
		 $$ -> child[2] = $7;
		}
		; 
iteration_stmt	: WHILE LPAREN expression RPAREN statement
	       	{$$ = newStmtNode(WhileK);
		 $$ -> child[0] = $3;
		 $$ -> child[1] = $5;
		}
		;

return_stmt	: RETURN SEMI
	    	{$$ = newStmtNode(ReturnK);
		 $$ -> child[0] = NULL;
		}
	    	| RETURN expression SEMI
		{$$ = newStmtNode(ReturnK);
		 $$ -> child[0] = $2;
		}
		;

expression	: var ASSIGN expression
	   	{$$=newExpNode(AssignK);
		 $$ -> child[0] = $1;
		 $$ -> child[1] = $3;
		}
	   	| simple_expression {$$=$1;}
		;

var		: id 
     		{$$ = newExpNode(NameK);
		 $$ -> attr.name = savedName;
		}
     		| id 
		{$$ = newExpNode(NameArrK);
		 $$ -> attr.name = savedName;
		}
		LBRACE expression RBRACE
		{$$ = $2;
		 $$ -> child[0] = $4;
		}
		;

simple_expression	: additive_expression LE additive_expression
		  	{$$ = newExpNode(OpK);
			 $$ -> child[0] = $1;
			 $$ -> child[1] = $3;
			 $$ -> attr.op = LE;
			}
		  	| additive_expression LT additive_expression
			{$$ = newExpNode(OpK);
                         $$ -> child[0] = $1;
                         $$ -> child[1] = $3;
                         $$ -> attr.op = LT;
                        }
			| additive_expression GT additive_expression
			{$$ = newExpNode(OpK);
                         $$ -> child[0] = $1;
                         $$ -> child[1] = $3;
                         $$ -> attr.op = GT;
                        }
			| additive_expression GE additive_expression
			{$$ = newExpNode(OpK);
                         $$ -> child[0] = $1;
                         $$ -> child[1] = $3;
                         $$ -> attr.op = GE;
                        }
			| additive_expression NE additive_expression
			{$$ = newExpNode(OpK);
                         $$ -> child[0] = $1;
                         $$ -> child[1] = $3;
                         $$ -> attr.op = NE;
                        }
			| additive_expression EQ additive_expression
			{$$ = newExpNode(OpK);
                         $$ -> child[0] = $1;
                         $$ -> child[1] = $3;
                         $$ -> attr.op = EQ;
                        }
			| additive_expression
			{$$=$1;}
			;

additive_expression	: additive_expression PLUS term
		    	{$$ = newExpNode(OpK);
			 $$ -> child[0] = $1;
			 $$ -> child[1] = $3;
			 $$ -> attr.op = PLUS;
			}
			| additive_expression MINUS term
                        {$$ = newExpNode(OpK);
                         $$ -> child[0] = $1;
                         $$ -> child[1] = $3;
			 $$ -> attr.op = MINUS;
                        }
			| term {$$=$1;}
			;

term	: term TIMES factor
     	{$$ = newExpNode(OpK);
	 $$ -> child[0] = $1;
	 $$ -> child[1] = $3;
	 $$ -> attr.op = TIMES;
	}
     	| term OVER factor
	{$$ = newExpNode(OpK);
         $$ -> child[0] = $1;
         $$ -> child[1] = $3;
	 $$ -> attr.op = OVER;
        }
	| factor{$$=$1;}
	;

factor	: LPAREN expression RPAREN
       	{$$ = $2;}
       	| var {$$=$1;}
	| call {$$=$1;}
	| num
	{$$ = newExpNode(ConstK);
	 $$ -> attr.val = savedNum;
	}
	;

call	: id
     	{$$ = newExpNode(CallK);
	 $$ -> attr.name = savedName;
	}
	LPAREN args RPAREN
	{$$ = $2;
	 $$ -> child[0] = $4;
	}
	;
args	: arg_list {$$=$1;}
     	| {$$=NULL;}
	;

arg_list	: arg_list COMMA expression
	 	{ YYSTYPE t = $1;
                                if (t != NULL)
                                { while (t->sibling != NULL)
                                        t = t->sibling;
                                t->sibling = $3;
                                $$ = $1; }
                                else $$ = $3;
                        }

	 	| expression { $$= $1;}
		;

/*-----------------------additive_expression LE additive_expression--------------*/

%%

int yyerror(char * message)
{ fprintf(listing,"Syntax error at line %d: %s\n",lineno,message);
  fprintf(listing,"Current token: ");
  printToken(yychar,tokenString);
  Error = TRUE;
  return 0;
}

/* yylex calls getToken to make Yacc/Bison output
 * compatible with ealier versions of the TINY scanner
 */
static int yylex(void)
{ return getToken(); }

TreeNode * parse(void)
{ yyparse();
  return savedTree;
}

