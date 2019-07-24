
test = method(
NUM = if(hasSlot("args"), args at(0) asNumber, 1)

flags = List clone
for(n, NUM, 1,
  count = 0
  flags = List clone
  for(i, 2, 8192, flags atPut(i, 1))
  for(i, 2, 8192,
    if(flags at(i) == 1,
      k = i + i
      while(k <= 8192, flags atPut(k, 0); k += i)
      count ++
    )
  )
)

write("Count: ", count, "\n")
)

test
