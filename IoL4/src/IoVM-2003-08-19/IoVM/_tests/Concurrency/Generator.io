
// The coroutine in the Generator stays around and processes 
// a little bit each time "next(v)" is invoked.

Generator = Object clone
Generator value = 0
Generator input = 0
Generator next = method(v, 
  self input = v
  self resume
  yield
  return value
)

Generator run = method(
  for(i, 0, 1000, 
    self value = i + self input
    self pause
  )
  self value = Nil
)

Generator @run
write(Generator next(10), "\n"); yield
write(Generator next(100), "\n"); yield
write(Generator next(1000), "\n"); yield

"output should have been: 10, 101, 1002"
