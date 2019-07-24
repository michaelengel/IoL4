/*
 * SplayTree:
 * Top down splay tree with definable compare function.
 * - Steve Dekorte 2002
 * 
 * Based on:
 *   An implementation of top-down splaying
 *   D. Sleator <sleator@cs.cmu.edu> March 1992
 */

#include <stdio.h>
#include <stdlib.h>
#include "SplayTree.h"

SplayTreeNode *SplayTreeNode_newWithKey_value_(void *key, void *value) 
{
  SplayTreeNode *self = (SplayTreeNode *)calloc(1, sizeof(SplayTreeNode));
  if ((!key) || (!value))
  {
    printf("SplayTree_key_value_ error\n");
    exit(1);
  }
  
  self->key = key;
  self->value = value;
  self->left = self->right = NULL;
  return self;
}
    
void SplayTreeNode_free(SplayTreeNode *self) 
{
  if (self->left)  { SplayTreeNode_free(self->left); }
  if (self->right) { SplayTreeNode_free(self->right); }
  free(self);
}

void SplayTreeNode_do_(SplayTreeNode *self, SplayTreeDoCallback *callback)
{
  (*callback)(self->key);
  if (self->left)  SplayTreeNode_do_(self->left, callback);
  if (self->right) SplayTreeNode_do_(self->right, callback);
}

void SplayTreeNode_doOnKeyAndValue_(SplayTreeNode *self, SplayTreeDoCallback *callback)
{
  (*callback)(self->key);
  (*callback)(self->value);
  if (self->left)  SplayTreeNode_do_(self->left, callback);
  if (self->right) SplayTreeNode_do_(self->right, callback);
}

int SplayTreeNode_count(SplayTreeNode *self)
{
  int c = 1;
  if (self->left)  c += SplayTreeNode_count(self->left);
  if (self->right) c += SplayTreeNode_count(self->right);
  return c;
}

/* --- SplayTree ------------------------------------------*/

SplayTree *SplayTree_new(void)
{
  SplayTree *self = (SplayTree *)calloc(1, sizeof(SplayTree));
  memset(self, (unsigned char)0x0, sizeof(SplayTree));
  self->count = 0;
  self->first = (SplayTreeNode *)0x0;
  self->collection = List_new();
  return self;
}

void SplayTree_free(SplayTree *self)
{
  if (self->first) { SplayTreeNode_free(self->first); }
  List_free(self->collection);
  free(self);
}

void SplayTree_compareFunction_(SplayTree *self, SplayTreeCompareFunc *func)
{ self->compareFunction = func; }

SplayTreeNode *SplayTree_splay(SplayTree *self, SplayTreeNode *t, void *key) 
{
  /* Simple top down splay, not requiring key to be in the tree t.  */
  SplayTreeCompareFunc *cmpFunc = self->compareFunction;
    SplayTreeNode N, *l, *r, *y;
    if (t == NULL) return (SplayTreeNode *)0x0;
    N.left = N.right = NULL;
    l = r = &N;

    for (;;) {
        int c = (cmpFunc)(key, t->key);
	if (c < 0) /*(key < t->key)*/ {
	    if (t->left == NULL) break;
            c = (cmpFunc)(key, t->left->key);
	    if (c < 0) /*(key < t->left->key)*/ {
		y = t->left;                           /* rotate right */
		t->left = y->right;
		y->right = t;
		t = y;
		if (t->left == NULL) break;
	    }
	    r->left = t;                               /* link right */
	    r = t;
	    t = t->left;
	} else if (c > 0) /*(key > t->key)*/ {
	    if (t->right == NULL) break;
            c = (cmpFunc)(key, t->right->key);
	    if (c > 0) /*(i > t->right->key)*/ {
		y = t->right;                          /* rotate left */
		t->right = y->left;
		y->left = t;
		t = y;
		if (t->right == NULL) break;
	    }
	    l->right = t;                              /* link left */
	    l = t;
	    t = t->right;
	} else {
	    break;
	}
    }
    l->right = t->left;                                /* assemble */
    r->left = t->right;
    t->left = N.right;
    t->right = N.left;
    self->first = t;
    return self->first;
}

void *SplayTree_valueForKey_(SplayTree *self, void *key)
{
  if (self->first) 
  { 
    SplayTreeNode *match = SplayTree_splay(self, self->first, key);
    if (self->compareFunction(key, match->key)==0) return match->value; 
  }
  return (void *)0x0;
}

void SplayTree_key_value_(SplayTree *self, void *key, void *value)
{
  SplayTreeNode *t;
  int c;
  
  if ((!key) || (!value))
  {
    printf("SplayTree_key_value_ error\n");
    exit(1);
  }
  if (!self->first) 
  {
    SplayTreeNode *new = SplayTreeNode_newWithKey_value_(key, value);
    new->left = new->right = NULL;
    self->count = 1;
    self->first = new;
    return;
  }
  SplayTree_splay(self, self->first, key);
  t = self->first;
  c = self->compareFunction(key, t->key);
  if (c < 0) /*(i < t->key)*/
  {
    SplayTreeNode *new = SplayTreeNode_newWithKey_value_(key, value);
    new->left = t->left;
    new->right = t;
    t->left = NULL;
    self->count++;
    self->first = new;
    return;
  } 
  else if (c > 0) /*(key > t->key)*/
  {
    SplayTreeNode *new = SplayTreeNode_newWithKey_value_(key, value);
    new->right = t->right;
    new->left = t;
    t->right = NULL;
    self->count++;
    self->first = new;
    return;
  } 
  /* We get here if it's already in the tree */
}

int SplayTree_removeKey_(SplayTree *self, void *key)
{
  SplayTreeNode *t;
  SplayTreeNode *x;
  int c;
  
  if (!self->first) return 0;
  SplayTree_splay(self, self->first, key);
  t = self->first;
  
  c = self->compareFunction(key, t->key);

  if (c == 0) { /* found it */
    if (t->left == NULL) {
      x = t->right;
    } else {
      x = SplayTree_splay(self, t->left, key);
      x->right = t->right;
    }
    self->count--;
    free(t);
    self->first = x;
    return 1;
  }
  return 0;
}

int SplayTree_count(SplayTree *self) { return self->count; }

void SplayTree_verify(SplayTree *self) 
{ 
  if ((!self->first && self->count != 0) ||
    self->count != SplayTreeNode_count(self->first))
  {
    printf("SplayTree_verify failed %i != %i\n",
     self->count, SplayTreeNode_count(self->first));
    exit(1);
  } 
}

/* --- perform -------------------------------------------------- */

void SplayTree_do_(SplayTree *self, SplayTreeDoCallback *callback)
{ if (self->first) { SplayTreeNode_do_(self->first, callback); } }

void SplayTree_doOnKeyAndValue_(SplayTree *self, SplayTreeDoCallback *callback)
{ if (self->first) { SplayTreeNode_doOnKeyAndValue_(self->first, callback); } }

void *SplayTree_findWithFunction_match_(SplayTree *self, SplayTreeCompareFunc* callback, void *key)
{
  SplayTreeCompareFunc *oldCompare = self->compareFunction;
  void *value;
  self->compareFunction = callback;
  value = SplayTree_valueForKey_(self, key);
  self->compareFunction = oldCompare;
  return value;
}
