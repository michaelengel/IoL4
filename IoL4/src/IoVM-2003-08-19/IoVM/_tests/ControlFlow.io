
// Example break
for(i, 0, 10, if(i == 5, break); i print)

// Example continue
for(i, 0, 10, if(i == 5, continue); i print)

// Example return

a = method(
for(i, 0, 10, 
  if(i == 5, return i)
  i print
)
)

result = a
write(" returned on = ", result)