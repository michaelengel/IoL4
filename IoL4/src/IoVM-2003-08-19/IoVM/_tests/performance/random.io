M = 139968
A = 3877
C = 29573

S = 42
random = method(max,
  S *=(A) +=(C) %=(M)
  max *=(S) /=(M)
)

N = 1
if(?args, N = args at(0) asNumber)

r = for(i, 1, N, random(100))

r linePrint
