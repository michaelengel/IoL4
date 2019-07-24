/*
  SplayTree:
  Top down splay tree with definable compare function.
  - Steve Dekorte 2002
  
  Based on:
    An implementation of top-down splaying
    D. Sleator <sleator@cs.cmu.edu> March 1992
*/

#ifndef SPLAYTREE_DEFINED
#define SPLAYTREE_DEFINED 1

#include "Common.h"
#include <stdio.h>
#include "List.h"

typedef int (SplayTreeCompareFunc)(void *, void *);
typedef void (SplayTreeDoCallback)(void *);
typedef int (SplayTreeRemoveTrueForCallback)(void *);

/* --- SplayTreeNode ------------------------------------------*/

typedef struct SplayTreeNode SplayTreeNode;

struct SplayTreeNode 
{
  void *key;
  void *value;
  SplayTreeNode *left, *right;
};

SplayTreeNode *SplayTreeNode_newWithKey_value_(void *key, void *value);
void SplayTreeNode_free(SplayTreeNode *self);

void SplayTreeNode_do_(SplayTreeNode *self, SplayTreeDoCallback *callback);
void SplayTreeNode_doOnKeyAndValue_(SplayTreeNode *self, SplayTreeDoCallback *callback);
int SplayTreeNode_count(SplayTreeNode *self);

/* --- SplayTree ------------------------------------------*/

typedef struct 
{
  SplayTreeNode *first;
  SplayTreeCompareFunc *compareFunction;
  int count;
  List *collection;
} SplayTree;

SplayTree *SplayTree_new(void);
void SplayTree_free(SplayTree *self);
void SplayTree_compareFunction_(SplayTree *self, SplayTreeCompareFunc *func);

SplayTreeNode *SplayTree_splay(SplayTree *self, SplayTreeNode *t, void *key);
void *SplayTree_valueForKey_(SplayTree *self, void *key);
void SplayTree_key_value_(SplayTree *self, void *key, void *value);
int SplayTree_removeKey_(SplayTree *self, void *key);

void SplayTree_verify(SplayTree *self);
int SplayTree_count(SplayTree *self);

/* --- perform -------------------------------------------------- */
void SplayTree_do_(SplayTree *self, SplayTreeDoCallback *callback);
void *SplayTree_findWithFunction_match_(SplayTree *self, SplayTreeCompareFunc *callback, void *v);

#endif
