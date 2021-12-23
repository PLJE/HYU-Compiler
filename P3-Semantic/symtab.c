/****************************************************/
/* File: symtab.c                                   */
/* Symbol table implementation for the TINY compiler*/
/* (allows only one symbol table)                   */
/* Symbol table is implemented as a chained         */
/* hash table                                       */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"

/* SIZE is the size of the hash table */
#define SIZE 211

/* SHIFT is the power of two used as multiplier
   in hash function  */
#define SHIFT 4


static ScopeList scopes[SIZE];
static ScopeList scStack[SIZE];
static int totalscope = 0;
static int tos = 0;
static int location[SIZE];

static int hash ( char * key )
{ int temp = 0;
  int i = 0;
  while (key[i] != '\0')
  { temp = ((temp << SHIFT) + key[i]) % SIZE;
    ++i;
  }
  return temp;
}

ScopeList getTop(){
	if(tos <=0)
		return NULL;
        return scStack[tos-1];
}

void exist_insert(char * name , int lineno){
	BucketList bl = total_lookup(name);
	
	LineList lt = bl -> lines;
	while(lt -> next != NULL) lt = lt -> next;
	lt -> next = (LineList)malloc(sizeof(struct LineListRec));
	lt -> next -> lineno = lineno;
	lt -> next -> next = NULL;
}

void st_insert( char * name, int lineno, int loc, TreeNode * treenode )
{ int h = hash(name);
  
	ScopeList scope = getTop();
	BucketList l = NULL;
  	l = (BucketList) malloc(sizeof(struct BucketListRec));
    	l->name = name;
    	l->lines = (LineList) malloc(sizeof(struct LineListRec));
    	l->lines->lineno = lineno;
    	l->memloc = loc;
    	l->treenode = treenode;
    	l->lines->next = NULL;
    	l->next = scope ->hashTable[h];
    	scope -> hashTable[h] = l;
} /* st_insert */

BucketList total_lookup ( char * name )
{ int h = hash(name);
	BucketList bl = NULL;
	ScopeList s = getTop();
	while(s != NULL){
		bl = s -> hashTable[h];

		while((bl != NULL) && (strcmp(name , bl->name) != 0 ))
			bl = bl->next;
		if(bl != NULL)
			return bl;
		s = s-> parent;
		if(s == NULL)
			break;
	}
	return NULL;
}

BucketList cur_lookup (char* name){
	int h = hash(name);
	BucketList bl = NULL;
	ScopeList s = getTop();
	
	//printf("%d %d \n" , tos , totalscope);
	//printf("%s\n" , scStack[3]->name);
	
	if( s!= NULL){
		bl = s -> hashTable[h];
		while( (bl!=NULL) && strcmp(name, bl->name)!=0 )
			bl= bl -> next;
		if (bl!= NULL)
			return bl;
	}
	return NULL;
}

void st_insert_output(char * val ,char* scope, TreeNode*treenode , int loc, int lineno ){
	// (value , output, param, 0,0);
	ScopeList output ;
	for(int i=0;i<totalscope ;i++){
		if(strcmp(scopes[i]->name , scope ) == 0){
			output = scopes[i];
			break;
		}
	}	
		
	int h = hash(val);
	BucketList l = NULL;
        l = (BucketList) malloc(sizeof(struct BucketListRec));
        l->name = val;
        l->lines = (LineList) malloc(sizeof(struct LineListRec));
        l->lines->lineno = lineno;
        l->memloc = loc;
	l->treenode = treenode;
        l->lines->next = NULL;
        l->next = output->hashTable[h];
        output -> hashTable[h] = l;

}
ScopeList sc_insert(char*name){
	ScopeList in;
	in = (ScopeList)malloc(sizeof(struct ScopeListRec));
	in -> nestlevel = tos;
	in -> parent = getTop();
	in -> name = name;
	scopes[totalscope] = in;
	totalscope +=1;
	
	return in;
}

int inc_location(){
	int loc = location[tos-1];
	location[tos-1]+=1;
	return loc;
}

void pop(){
	if(tos <=0){
		fprintf(listing, "error : tos is zero\n");
	}
	tos-=1;
}

void push(ScopeList s){
	scStack[tos] = s;
	location[tos] = 0;
	tos+=1;
}

void printsymbol(){
	int i=0;
	int j=0;

	fprintf(listing, "<Symbol Table>\n");
	fprintf(listing , "Symbol Name   Symbol Kind   Symbol Type    Scope Name   Location  Line Numbers\n");
	fprintf(listing, "-------------  -----------  -------------  ------------  --------  ------------\n");

	for(i=0 ;i < totalscope ;i++){
		ScopeList sp = scopes[i];
		BucketList * table = sp->hashTable;
		for(j=0;j<SIZE;j++){
			if(table[j] != NULL){
				BucketList b = table[j];
				while(b!=NULL){
					fprintf(listing , "%-14s" ,b->name); //symbol name
					if(b->treenode->nodekind == DecK){
						TreeNode* node = b -> treenode;
						switch(node->kind.dec){
							case VarK:
								fprintf(listing,"%-14s", "Variable");
								if(node->type == Integer)
									fprintf(listing,"%-15s","int");
								else if(node->type == Void)
									fprintf(listing,"%-15s","void");
								break;	
							case VarArrK:
								fprintf(listing, "%-14s", "Variable");
								fprintf(listing, "%-16s", "int[]");
								break;
							case FuncK:
								fprintf(listing, "%-14s","Function");
								if (node-> type == Integer)
									fprintf(listing, "%-15s" ,"int");
								else if (node->type == Void)
									fprintf(listing, "%-15s" , "void");
								break;
							case Param1K:
								fprintf(listing, "%-14s" , "Variable");
								if(node->type == Integer)
									fprintf(listing, "%-15s" , "int");
								else if(node->type == Void)
									fprintf(listing, "%-15s" , "void");
								break;
							case Param2K: //array
								fprintf(listing, "%-14s" , "Variable");
								if(node->type == Integer)
									fprintf(listing, "%-14s", "int[]");
								break;

						}
						fprintf(listing, "%-14s" , sp->name); //scope name
						fprintf(listing, "%-10d", b->memloc); //location
						LineList l= b->lines;
						while(l!=NULL){
							fprintf(listing, "%2d ", l->lineno); //line numbers
							l = l->next;
						}
						fprintf(listing, "\n");
						b = b -> next;

					}
					
				}
			}
			
		}
	}
}

void printFunctions(){
	fprintf(listing, "<Functions>\n");
	fprintf(listing, "Function Name   Return Type   Parameter Name  Parameter Type\n");
	fprintf(listing, "-------------  -------------  --------------  --------------\n");
	
	BucketList * table = NULL;
	for(int i=0 ; i< totalscope;i++){
		if(strcmp(scopes[i]->name,"global") ==0 ){
			ScopeList scope = scopes[i];
			table = scope-> hashTable;
			//fprintf(listing,"find gloabl! %d , %s\n" ,i, scopes[i]->name);	
		}
	}
	for(int i=0;i<SIZE ; i++){
		BucketList b = table[i];
		if(b==NULL)continue;	
		TreeNode * t = b -> treenode;
		
		if(t->nodekind == DecK && t->kind.dec == FuncK){
			fprintf(listing, "%-16s", b->name); // Function Name
			if(t -> type == Void)
				fprintf(listing, "%-13s" , "void");
			else if(t->type == Integer)
				fprintf(listing, "%-13s" , "int");
			TreeNode * paramnode = t->child[0];
			if(paramnode -> type == Void){
				fprintf(listing, "%-18s" ,"");
				fprintf(listing ,"%-14s" ,"void");
			}else if (paramnode -> type == Integer){
				while(1){
					fprintf(listing, "\n");
                                	fprintf(listing, "%-31s" ,"");
                                	fprintf(listing, "%-16s" , paramnode -> attr.name);
                                	fprintf(listing, "%-14s" , "int");
					if(paramnode -> sibling == NULL)
						break;
					if(paramnode -> sibling != NULL)
						paramnode = paramnode -> sibling;
				}
			}
		}
		fprintf(listing,"\n");
	}
}
void printGlobalSymbols(){
	fprintf(listing, "<Global Symbols>\n");
	fprintf(listing, " Symbol Name   Symbol Kind   Symbol Type\n");
	fprintf(listing, "-------------  -----------  -------------\n");
	
	BucketList * table = NULL;
        for(int i=0 ; i< totalscope;i++){
                if(strcmp(scopes[i]->name,"global") ==0 ){
                        ScopeList scope = scopes[i];
                        table = scope-> hashTable;
                }
        }
	for(int i=0;i<SIZE;i++){
		BucketList b = table[i];
		if(b == NULL) continue;
		TreeNode * t = b -> treenode;
		if(t-> nodekind == DecK){
			switch(t->kind.dec){
				case FuncK:
					fprintf(listing, "%-16s" , t->attr.name);
					fprintf(listing, "%-13s" , "Function");
					if(t -> type == Void)
						fprintf(listing , "%-13s" , "void");
					else if(t->type == Integer)
						fprintf(listing, "%-13s" , "int");
					break;
				case VarK:
					fprintf(listing, "%-16s" , t->attr.name);
					fprintf(listing, "%-13s" , "Variable");
					if(t->type == Integer)
						fprintf(listing, "%-13s" , "int");
					break;
				case VarArrK:
					fprintf(listing, "%-16s" , t->attr.name);
                                        fprintf(listing, "%-13s" , "Variable");
                                        if(t->type == Integer)
                                                fprintf(listing, "%-13s" , "int[]");
					break;
			}
		}
		fprintf(listing,"\n");
	}
}
void printScopes(){
	fprintf(listing, "<Scopes>\n");
	fprintf(listing, " Scope Name   Nested Level   Symbol Name   Symbol Type\n");
	fprintf(listing, "------------  ------------  -------------  -----------\n");
	for(int i=1 ; i<totalscope ;i++){
		ScopeList scope = scopes[i];
		
		BucketList * table = scope -> hashTable;
		for(int j=0;j<SIZE;j++){
			BucketList b = table[j];
			if(b == NULL) continue;
			TreeNode * t = b -> treenode;
			if(t -> nodekind == DecK){
				switch(t-> kind.dec){
					case VarK:
						if(t -> type = Integer){
							fprintf(listing, "%-14s" , scopes[i]->name);
                					fprintf(listing, "%-12d" , scopes[i]->nestlevel);

							fprintf(listing , "%-13s" , t ->attr.name);
							fprintf(listing, "%-13s" , "int");
						}
						break;
					case VarArrK:
						if(t -> type = Integer){
							fprintf(listing, "%-14s" , scopes[i]->name);
                					fprintf(listing, "%-12d" , scopes[i]->nestlevel);
		
                                                        fprintf(listing , "%-13s" , t ->attr.name);
                                                        fprintf(listing, "%-13s" , "int[]");
                                                }
						break;
					case Param1K:
						if(t -> type == Void){
							fprintf(listing, "%-14s" , scopes[i]->name);
                                                        fprintf(listing, "%-12d" , scopes[i]->nestlevel);
							fprintf(listing , "%-13s" , "");
                                                        fprintf(listing, "%-13s" , "");
						}else if(t-> type == Integer){
							fprintf(listing, "%-14s" , scopes[i]->name);
                                                        fprintf(listing, "%-12d" , scopes[i]->nestlevel);
                                                        fprintf(listing , "%-13s" , t->attr.name);
                                                        fprintf(listing, "%-13s" , "int");
						}
						break;
					case Param2K:
						if(t -> type == Void){
                                                        fprintf(listing, "%-14s" , scopes[i]->name);
                                                        fprintf(listing, "%-12d" , scopes[i]->nestlevel);
                                                        fprintf(listing , "%-13s" , "");
                                                        fprintf(listing, "%-13s" , "");
                                                }else if(t-> type == Integer){
                                                        fprintf(listing, "%-14s" , scopes[i]->name);
                                                        fprintf(listing, "%-12d" , scopes[i]->nestlevel);
                                                        fprintf(listing , "%-13s" , t->attr.name);
                                                        fprintf(listing, "%-13s" , "int[]");
                                                }
						break;
				}
			}
			fprintf(listing, "\n");
		}

		fprintf(listing, "\n");
	}
}
void printSymTab(FILE * listing)
{ 
	//int i=0;
	//for(i=0;i<3;i++){
	//	printf("%s\n" , scopes[i]->name);
	//}
	printsymbol();
	printFunctions();
	printGlobalSymbols();
	printScopes();
} 
