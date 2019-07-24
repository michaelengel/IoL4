
Calc = Object clone do(
  factorial = method(n, 
    self value = n
    self total = self fact(n)
    results add(self)
  )
  fact = method(n, 
    yield
    if(n == 1, 1, n * self fact(n-1))
  )
  print = method(write(value, "! = ", total, "\n"))
)

results = List clone
list = List clone
for(i, 1, 5,
  o = Calc clone 
  o @factorial(i)
  list add(o) 
  write(i, "!\n")
)

write("wait for all results")
while (results count != list count, write("."); yield)

write("\n--- results ---\n")
results foreach(i, o, o print)
