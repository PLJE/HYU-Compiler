/****************************************************/
/* File: analyze.c                                  */
/* Semantic analyzer implementation                 */
/* for the TINY compiler                            */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"
#include "symtab.h"
#include "analyze.h"
#include "util.h"
/* counter for variable memory locations */
static int location = 0;
static int infunc = 0;
static char * fname;
static ScopeList global;
static void traverse( TreeNode * t,
               void (* preProc) (TreeNode *),
               void (* postProc) (TreeNode *) )
{ if (t != NULL)
  { preProc(t);
    { int i;
      for (i=0; i < MAXCHILDREN; i++)
        traverse(t->child[i],preProc,postProc);
    }
    postProc(t);
    traverse(t->sibling,preProc,postProc);
  }
}

void built_in(){

	TreeNode * f;
	TreeNode * param;
	TreeNode * compound;

	compound = newStmtNode(CompK);
	compound -> child[0] = NULL;
	compound -> child[1] = NULL;

	param = newDecNode(Param1K);
	param -> type = Void;
	param -> attr.name = NULL;

	f = newDecNode(FuncK);
	f ->lineno = 0;
	f->type = Integer;
	f -> attr.name = "input";
	f -> child[0] = param;
	f -> child[1] = compound;
	
	st_insert("input" , 0 , inc_location() , f);

	param = newDecNode(Param1K);
	param -> type = Integer;
	param -> attr.name = "value";

	compound = newStmtNode(CompK);
	compound -> child[0] = NULL;
	compound -> child[1] = NULL;

	f = newDecNode(FuncK);
	f->lineno = 0;
	f->type = Void;
	f->attr.name="output";
	f->child[0]=param;
	f->child[1]=compound;

	st_insert("output",0, inc_location() , f);
	sc_insert("output");
	st_insert_output("value" ,"output" , param ,0,0);
}
static void nullProc(TreeNode * t)
{ if (t==NULL) return;
  else return;
}


static void insertNode( TreeNode * t)
{ switch (t->nodekind)
  { case StmtK:
      switch (t->kind.stmt)
      { case CompK:
	      if(infunc == 1)
		      infunc = 0;
	      else{
		      ScopeList new = sc_insert(fname);
		      push(new);
		      t->attr.scope = getTop();
		      location+=1; 
	      }
	      t -> attr. scope =getTop(); 
	      break;
      }
      break;
    case ExpK:
      switch (t->kind.exp)
      { case NameK:
	case NameArrK:
	case CallK:
		if(total_lookup(t->attr.name) == NULL){
			fprintf(listing, "\n*Error : Function(variable) undefined. name : %s, at line %d\n",t->attr.name , t->lineno);
		}
		else
			exist_insert(t->attr.name , t-> lineno);
	        break;
      }
      break;
    case DecK:
      switch (t->kind.dec)
      { case FuncK:
	      fname = t -> attr.name ;
	      if(total_lookup(t->attr.name) != NULL){
		      fprintf(listing, "\n*Error : Function redefined. name : %s, at line %d\n", fname,t->lineno);
	      }
	      else{
	      	infunc =1;
	      	st_insert(fname ,t->lineno , inc_location() , t);
	      	ScopeList new = sc_insert(fname);
		push(new);
	      }
	      break;
	case VarK:
	case VarArrK:
	      if(t->type == Void){
		      fprintf(listing, "\n*Error : variable or array cannot be defined as void. name : %s, at line %d\n",t-> attr.name, t->lineno);
		      break;
	      }
	      if(cur_lookup(t->attr.name) == NULL)
		      st_insert(t->attr.name , t->lineno , inc_location() , t);
	      else
		      fprintf(listing,"\n*Error : variable redefined. name :  %s, at line %d\n", t->attr.name, t->lineno);
	      break;
	case Param1K:
	case Param2K:
	      if(t->type == Integer)
		      st_insert(t->attr.name , t->lineno , inc_location() , t);
	      break;
      }
      break;  
    default:
      break;
  }
}

static void scopePop(TreeNode *t){
	if(t-> nodekind == StmtK &&  t->kind.stmt == CompK)
		pop();
}

/* Function buildSymtab constructs the symbol 
 * table by preorder traversal of the syntax tree
 */
void buildSymtab(TreeNode * syntaxTree)
{ 	global = sc_insert("global");
	push(global);
	built_in();
	traverse(syntaxTree,insertNode,scopePop);
  	pop();

  if (TraceAnalyze)
  { fprintf(listing,"\nSymbol table:\n\n");
    printSymTab(listing);
  }
}

static void typeError(TreeNode * t, char * message)
{ fprintf(listing,"Type error at line %d: %s\n",t->lineno,message);
  Error = TRUE;
}


static void func_comp(TreeNode *t){
	if(t->nodekind == StmtK && t->kind.stmt == CompK)
		push(t->attr.scope);
	if(t->nodekind == DecK && t->kind.dec == FuncK)
		fname = t->attr.name;
}

static void nodeKind(TreeNode *t){
	if(t->nodekind == StmtK){
		switch(t->kind.stmt){
			case CompK:
				fprintf(listing, "compK\n");
				break;
			case IfK:
				fprintf(listing, "if\n");
                                break;
			case WhileK:
				fprintf(listing, "while\n");
                                break;
			case ReturnK:
				fprintf(listing, "return\n");
                                break;
		}
	}
	else if(t->nodekind == ExpK){
		switch(t->kind.exp){
			case AssignK:
				fprintf(listing, "assign\n");
                                break;
			case NameK:
				fprintf(listing, "name\n");
                                break;
			case NameArrK:
				fprintf(listing, "namearrK\n");
                                break;
			case OpK:
				fprintf(listing, "opK\n");
                                break;
			case ConstK:
				fprintf(listing, "constK\n");
                                break;
			case CallK:
				fprintf(listing, "CallK\n");
                                break;
		}
	}
	else if(t->nodekind == DecK){
		switch(t->kind.dec){
			case VarK:
				fprintf(listing, "var\n");
                                break;
			case VarArrK:
				fprintf(listing, "vararr\n");
                                break;
			case FuncK:
				fprintf(listing, "funck\n");
                                break;
			case Param1K:
				fprintf(listing, "param1\n");
                                break;
			case Param2K:
				fprintf(listing, "param2K\n");
                                break;
		}
	}
}

static void nodeType(TreeNode*t){
	if(t->type == Integer)
		printf("int\n");
	else
		printf("not int?!\n");
}
static void nodeName(TreeNode*t){
	printf("%s\n" , t->attr.name);
}
static void checkNode(TreeNode * t)
{ switch (t->nodekind)
  { case ExpK:
      switch (t->kind.exp)
      { case NameK:
	      {
	      BucketList b;
	      b = total_lookup(t->attr.name);
	      if(b == NULL){
		      fprintf(listing,"\n*Error : use not defined variable at line %d, name:%s\n" , t->lineno , t->attr.name);
		      break;
	      }
	      if(b->treenode->nodekind == DecK && b->treenode->kind.dec == VarArrK){
		      fprintf(listing , "\n*Error : invalid expression at line %d\n", t->lineno);
		      break;
	      }
	      if(b->treenode->nodekind == DecK && b -> treenode->kind.dec == Param2K){
		      fprintf(listing , "\n*Error : invalid expression at line %d\n", t->lineno);
                      break;
	      }
	      t -> type = Integer;
	      break;
	case NameArrK:
	      {
	      BucketList b;
              b = total_lookup(t->attr.name);
	      
              if(b == NULL){
                      fprintf(listing,"\n*Error : use not defined variable at line %d\n" , t->lineno);
                      break;
              }
	      if(t -> child[0]->type != Integer){
		      fprintf(listing, "\n*Error : invalid array indexing at line %d , name : %s\n" ,t->lineno,  t->attr.name);
		      break;
	      	}
	      }
	      break;
	case CallK:
	      {
	      BucketList b = total_lookup(t->attr.name); //x
	      if(b== NULL) break;
	      TreeNode * realfunc = b -> treenode;
	      TreeNode * realparam = realfunc -> child[0]; // int y 

	      TreeNode * callparam = t -> child[0]; // a,b,c

	      if(realparam -> type == Void && callparam == NULL){
		      t-> type = realfunc -> type;
		      break;
	      }

	      if(callparam == NULL && realparam!= NULL){
		      fprintf(listing,"\n*Error : invalid function call at line %d\n", t->lineno);
	      	      t -> type = realfunc -> type;
		      break;
	      	}
	      if(callparam != NULL && realparam == NULL){
		      fprintf(listing, "\n*Error : invalid function call at line %d\n", t->lineno);
		      t->type = realfunc -> type;
		      break;
	      	}
	      if(callparam == NULL && realparam == NULL){
		      t -> type = realfunc -> type;
	      }
	      int chk = 0;
	      while(1){
		if(realparam != NULL && callparam != NULL){
	      		ExpType realtype = realparam -> type;
			ExpType curtype = callparam -> type;
			if( realtype != curtype){
                        	fprintf(listing, "\n*Error : invalid function call at line %d\n", t->lineno);
                        	break;
                	}

		}
		
		//callparam = callparam -> sibling;
		//realparam = realparam -> sibling;

		if(callparam != NULL && realparam == NULL){
			fprintf(listing ,"\n*Error : invalid function call at line %d\n", t->lineno);
			chk=1;
			break;
		}
		if(callparam == NULL && realparam!= NULL){
			fprintf(listing ,"\n*Error : invalid function call at line %d\n", t->lineno);
                        chk = 1;
			break;
		}
		if(callparam == NULL && realparam == NULL){
                        //fprintf(listing ,"\n*Error : invalid function call at line %d\n", t->lineno);
                        break;
                }
		callparam = callparam -> sibling;
                realparam = realparam -> sibling;
	      }
	      t -> type = realfunc -> type;
	      }
	      break;
	case OpK:
		      if(t->child[0]->kind.exp == NameK && t->child[1]->kind.exp == NameArrK){
			      fprintf(listing, "\n*Error : invalid expression at line %d\n",t->lineno);
		      		break;
		      }
		      if(t->child[0]->kind.exp == NameArrK && t->child[1]->kind.exp == NameK){
			      fprintf(listing , "\n*Error : invalid expression at line %d\n",t->lineno);
			      break;
		      }
	      if(t->child[0]->type == Integer && t -> child[1]->type == Integer){
		      t -> type = Integer;
	      }
	      else
		      fprintf(listing, "\n*Error : operand type error. at line %d\n" , t->lineno);
	      break;
	case ConstK:
	      t->type = Integer;
	      break;
	case AssignK:
	      if( t -> child[1] ->type == Integer){
		      t->type = Integer; //assignk
		      t->child[0]->type = Integer;
	      }
	      break;
	      //namek constk
      }
      }
      break;
    case StmtK:
      switch (t->kind.stmt)
      { case CompK:
	      pop();
	      break;
	case IfK: 
	      if(t-> child[0] -> type != Integer)
		      fprintf(listing, "\n*Error : if condition should be int. at line : %d\n", t->lineno);
	      break;
	case WhileK:
	      if(t-> child[0] -> type != Integer)
                      fprintf(listing, "\n*Error : While condition should be int. at line : %d\n", t->lineno);
	      break;
	case ReturnK:
	      {
		      BucketList b = total_lookup(fname);
		      if(b==NULL) break;
		      TreeNode * curfunc = b -> treenode;
		      if(curfunc -> type != t->child[0]->type)
			      fprintf(listing, "\n*Error : invalid function return at line %d\n", t->lineno);
	      }
	      break;
      }
      break;
    case DecK:
      switch (t->kind.dec)
      {
	      case VarK:
	      case VarArrK:
		      if(t->type == Void)
			      fprintf(listing, "\n*Error : variable cannot be void type. name : %s  at line %d\n",t->attr.name , t->lineno);
		      break;
	      case Param1K:
	      case Param2K:
		      if(t->type == Void && t->attr.name != NULL)
			      fprintf(listing, "\n*Error : variable cannot be void type. name : %s at line %d\n",t->attr.name, t->lineno);
		      break;

      }
      break;
    default:
      break;

  }
}

/* Procedure typeCheck performs type checking 
 * by a postorder syntax tree traversal
 */
void typeCheck(TreeNode * syntaxTree)
{
        push(global);	
	traverse(syntaxTree,func_comp,checkNode);
	pop();
}
