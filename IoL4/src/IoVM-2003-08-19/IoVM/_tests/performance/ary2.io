
test = method(
n = if(?args at(0),args at(0) asNumber, 1)

x = List clone
y = List clone

i = 0
while(i < n,
  for (c, 1, 10, x atPut(i, i); i ++)
)

i = n - 1
while(i >= 0,
  for (c, 1, 10, y atPut(i, x at(i)); i --)
)

write(y at(n - 1), "\n")
)
test