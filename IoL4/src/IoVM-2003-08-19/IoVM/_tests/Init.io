
Point = Point clone
Point setX(0)
Point setY(0)

a = Object clone
a pos = Point clone
a init = method(
  "a init\n" print
  self pos = self pos clone
  self pos = self pos clone
)

b = a clone
b init = method(super init)
c = a clone
c init = method(super init)

c pos setX(1)

write("b pos x = ", b pos x, "(should be 0)\n")
write("c pos x = ", c pos x, "(should be 1)\n")