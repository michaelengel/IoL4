#include "SDSort.h"

typedef struct
{
  void *context;
  SDSortCompareCallback *comp; 
  SDSortSwapCallback *swap;
} SDSort;

int SDSort_isSorted(SDSort *self, int size);
void SDSort_quickSort(SDSort *self, int lb, int ub);
int SDSort_quickSortRearrange(SDSort *self, int lb, int ub);
void SDSort_shellSort(SDSort *self, int size);

void SDSort_sortContext_comp_swap_size_type_(void *context, 
 SDSortCompareCallback *comp, SDSortSwapCallback *swap, int size, SDSortType type)
{
  SDSort q;
  SDSort *self = &q;
  self->context = context;
  self->comp = comp;
  self->swap = swap;
  switch (type)
  {
    case SDQuickSort:
      if (!SDSort_isSorted(self, size)) SDSort_quickSort(self, 0, size-1);
      break;
    case SDShellSort:
      if (!SDSort_isSorted(self, size)) SDSort_shellSort(self, size);
      break;
  }
}

int SDSort_isSorted(SDSort *self, int size)
{
  int i;
  for (i=0; i<size-1; i++)
  {
    if ((*(self->comp))(self->context, i, i+1) > 0) 
    return 0;  
  }
  return 1;
}

void SDSort_quickSort(SDSort *self, int lb, int ub)
{
  if (lb < ub) 
  {
    int j = SDSort_quickSortRearrange(self, lb,ub);
    if (j) SDSort_quickSort(self, lb,j-1);
    SDSort_quickSort(self, j+1,ub);
  }
}

int SDSort_quickSortRearrange(SDSort *self, int lb, int ub)
{
  do {
    while (ub > lb && (*(self->comp))(self->context, ub, lb) >=0)
      ub--;
    if (ub != lb) 
    {
      (*(self->swap))(self->context, ub, lb);
      while (lb < ub && (*(self->comp))(self->context, lb, ub)<=0)
      { lb++; }
      if (lb != ub)
      (*(self->swap))(self->context, lb, ub);
    }
  } while (lb != ub);
  return lb;
}

void SDSort_shellSort(SDSort *self, int size)
{
  int m = size;
  int h, i, j, k;
  while (m /= 2) 
  {
    k = size - m;
    j = 1;
    do 
    {
      i = j;
      do 
      {
        h = i + m;
        if((*(self->comp))(self->context, i - 1,h - 1) > 0) 
        {
          (*(self->swap))(self->context, i - 1, h - 1);
          i -= m;
        }
        else
        { break; }
      } while (i >= 1);
      j += 1;
    } while(j <= k);
  }
}


