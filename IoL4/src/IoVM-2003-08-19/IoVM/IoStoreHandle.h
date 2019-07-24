/*   Copyright (c) 2002, Steve Dekorte
 *   All rights reserved. See _BSDLicense.txt.
 */

#ifndef IOSTOREHANDLE_DEFINED
#define IOSTOREHANDLE_DEFINED 1

#include "base/Common.h"
#include "base/ByteArray.h"

typedef struct
{
  void *target;
  void *locals;
  void *message;
} IoContext;


typedef struct
{
  unsigned char *data;
  int size;
} IoStoreDatum;

#define IOSTOREDATUM(data, size) { (unsigned char *)data, (int)size }


typedef struct IoStoreHandle IoStoreHandle;

typedef void  (StoreSetSlotFunc)(IoStoreHandle *, void *, void *, void *);  
typedef void *(StoreGetSlotFunc)(IoStoreHandle *, void *, void *);  

typedef void  (StoreSetDataFunc)(IoStoreHandle *, IoStoreDatum); 
typedef IoStoreDatum (StoreGetDataFunc)(IoStoreHandle *, void *); 

typedef void *(StoreWillFreeObjectFunc)(IoStoreHandle *, void *); 

struct IoStoreHandle
{
  IoContext *context;
  
  StoreSetSlotFunc *onObject_setSlot_to_;
  StoreGetSlotFunc *onObject_getSlot_;

  StoreSetDataFunc *onObject_setData_;
  StoreGetDataFunc *onObject_getData;

  StoreWillFreeObjectFunc *willFreeObject_;
};

#endif
