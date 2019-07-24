//debug = 1

test = method(

n = if(?args at(0), args at(0) asNumber, 5)

x = List clone
y = List clone
last = n - 1

for(i, 1, last + 1,
  x add(i)
  y add(0)
)


for(k, 1, 1000, 
  for(j, last, 0, 
    y at(j) += x at(j)
  )
)

write(y at(0), " ", y at(last), "\n")
)

test