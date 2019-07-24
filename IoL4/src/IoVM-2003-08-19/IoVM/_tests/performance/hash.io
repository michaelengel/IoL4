//debug = 1

test = method(

if(hasSlot("args"),
  n = args at(0) asNumber,
  n = 10000
)

numbers = List clone sizeTo(n)
for(i, 0, n, numbers add(i asString))

X = Map clone
for(i, 1, n, X atPut(numbers at(i), 1))
write("X\n")

c = 0
for(i, n, 1, if(X at(numbers at(i)), c ++))

write("c = ", c, "\n")
)

test