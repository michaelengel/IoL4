
typedef enum
{
  SDQuickSort,
  SDShellSort
} SDSortType;

typedef int (SDSortCompareCallback)(void *context, int i, int j);
typedef void (SDSortSwapCallback)(void *context, int i, int j);

void SDSort_sortContext_comp_swap_size_type_(void *context, 
 SDSortCompareCallback *comp, SDSortSwapCallback *swap, 
 int size, SDSortType type);


