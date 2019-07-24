
test = method(
n = if(?args at(0), args at(0) asNumber, 1)

x = List clone
y = List clone

for(i, 0, n, x add(i))
for(j, n, 0, y atPut(j, x at(j)))
write(y at(n - 1), "\n")
)

test