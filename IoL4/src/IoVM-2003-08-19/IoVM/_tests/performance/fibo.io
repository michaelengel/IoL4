

test = method(
  fib = method(n, if(n < 2, 1, fib(n - 2) + fib(n - 1)))

  if(hasSlot("args"),
    N = args at(0) asNumber,
    N = 14
  )

  write(fib(N), "\n")
)

test