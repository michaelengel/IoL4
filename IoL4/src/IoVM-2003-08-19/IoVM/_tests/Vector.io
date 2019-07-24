// untested

Vector = Object clone

Vector elements = List clone

Vector init = method(size, 
  self elements = elements clone
  self setSize(size)
)

Vector setSize = method(i, while(elements count < i, self elements add(0)))

Vector setSlot("+=", method(other, 
  elements foreach(i, v1, elements at(i) += other at(i))) 
)

Vector setSlot("-=", method(other, 
  elements foreach(i, v1, elements at(i) -= other at(i))) 
)

Vector setSlot(".", method(other, 
  v = 0
  elements foreach(i, v1, v += v1 *(other at(i))) 
  foo
  return v)
)

Vector print = method(otherVector, 
  write("[")
  elements print 
  write("]\n")
)

Vector at = method(i, elements at(i))
Vector atPut = method(i, v, elements atPut(i, v))

// --- test it -----------------------

v1 = Vector clone(10)
v2 = Vector clone(10)

v1 elements foreach(i, v, v1 atPut(i, i))
v2 elements foreach(i, v, v2 atPut(i, i))

write("v1: ", v1)
write("v2: ", v2)

write("v1 += v2\n")
v1 += v2
write("v1: ", v1)

write("v1 -= v2\n")
v1 -= v2
write("v1: ", v1)

write("v1 . v2 = ", v1 . v2, "\n")

