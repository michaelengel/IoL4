
test = method(
HI = 0
LO = 0

someFunction = method(n,
  catchException("", hiFunction(n), write("shouldn't get here\n"))
)

hiFunction = method(n,
  catchException("Hi_Exception", loFunction(n), HI ++)
)

loFunction = method(n,
  catchException("Lo_Exception", blowup(n), LO ++)
)

blowup = method(n,
  if( n % (2) != (0),
    raiseException("Lo_Exception", ""),
    raiseException("Hi_Exception", "")
  )
)

N = if(?args at(0), args at(0) asNumber, 1)
for(i, 1, N, someFunction(i))

write("Exceptions: HI=", HI, " / LO=", LO, "\n")
)
test