
test = method(

Collection = LinkedList

test = method(
  size = 10000

  L1 = Collection clone
  for(i, 1, size, L1 add(i asString))
  L2 = L1 clone

  L3 = Collection clone 
  while(L2 count > 0, L3 add(L2 removeAt(0)) )
  while(L3 count > 0, L2 add(L3 pop))

  L1 reverse
  if(L1 == L2, L1 count, Nil)
)

n = 10
if(?args, n = args at(0) asNumber)

for(i, 1, n, result = test)
write(result, "\n")

)

test
