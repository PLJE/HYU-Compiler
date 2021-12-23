/****************************************************/
/* File: symtab.h                                   */
/* Symbol table interface for the TINY compiler     */
/* (allows only one symbol table)                   */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/
#define SIZE 211

#include "globals.h"

#ifndef _SYMTAB_H_
#define _SYMTAB_H_


typedef struct LineListRec
   { int lineno;
     struct LineListRec * next;
   } * LineList;

typedef struct BucketListRec
   { char * name;
     LineList lines;
     int memloc ; /* memory location for variable */
     struct BucketListRec * next;
     TreeNode * treenode;
   } * BucketList;

typedef struct ScopeListRec{
	BucketList hashTable[SIZE];
	char * name;
	struct ScopeListRec * parent;
	int nestlevel;
}*ScopeList;

/* Procedure st_insert inserts line numbers and
 * memory locations into the symbol table
 * loc = memory location is inserted only the
 * first time, otherwise ignored
 */

static int hash( char * key);
ScopeList getTop();
void exist_insert(char * n , int i );
void st_insert( char * name, int lineno, int loc, TreeNode * treenode );
BucketList total_lookup ( char * name );
BucketList cur_lookup (char* name);
void set_global();
ScopeList sc_insert(char*name);
void pop();
void push(ScopeList s);

void printSymTab(FILE * listing);

#endif
