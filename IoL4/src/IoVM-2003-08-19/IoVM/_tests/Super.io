//debugParser = 1


a = Object clone
a init = method(v1, v2, v3,
  write("a ", self uniqueId, " init(", v1, ",", v2, ",", v3, ")\n")
)

b = a clone
b init = method(v1, v2, v3,
  super init(v1, v2, v3)
  write("b ", self uniqueId, " init(", v1, ",", v2, ",", v3, ")\n")
  b foo = 123
)

c = b clone
c init = method(v1, v2, v3,
  super init(v1, v2, v3)
  write("c ", self uniqueId, " init(", v1, ",", v2, ",", v3, ")\n")
  b foo = 123
)

"------------------\n" print
t = c clone(1, 2, 3)

write("t = ", t uniqueId, " init\n")
