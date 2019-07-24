/*   Copyright (c) 2002, Steve Dekorte
 *   All rights reserved. See _License.txt.
 */

#include "IoState.h"

int main(int argc, const char *argv[])
{
  IoState *st = IoState_new();
  int v = IoState_main(st, argc, argv);
  /*IoState_initAddons(self);*/
  /*IoState_free(st);*/
  return v;
}
