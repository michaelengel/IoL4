
// future method
Object setSlot("@@", method(
  f = Future clone
  methodName = thisMessage arguments at(0) name
  //write("thisMessage arguments at(0) = ", thisMessage arguments at(0) name, "\n")
  self @perform(methodName, getSlot("f"))
  return getSlot("f")
))

Future = Object clone
Future init = method(
  self value = Nop
  self setValue = method(v, write("resuming1\n"); getSlot("self") value = v; write("resuming\n"); resume )
  self activate = method(
    "Future activate\n" print
    while (value == Nop, pause)
    "Future activate done\n" print
    return value
  )
)

Producer = Object clone
Producer test = method(future,
  "Producer test\n" print
  getSlot("future") setValue(1+2+3)
  "Producer yield\n" print
  yield
  "Producer yield done\n" print
)

Test = Object clone
Test test = method(
  v = Producer @@test
  v // cause this thread to yield while awaiting future value to be set
  write("result = ", v, "\n")
)

Test @test