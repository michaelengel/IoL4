
debugCollector = 1

collectGarbage

RingItem = Object clone
RingItem next = Nil
RingItem id = -1
RingItem foo = method(
    write(id, "\n")
    //write(next, "\n")
    next @@foo
)

maxThreads = ?args at(0) asNumber
if (maxThreads == Nil, maxThreads = 3)

setSchedulerSleepSeconds(0)

write("starting ring with ", maxThreads, " threads\n")

/*
first = RingItem clone
first next = first
first foo
wait(10)
exit
*/

first = RingItem clone
first id = 1
last = first
for(i, 2, maxThreads,
  item = RingItem clone
  item next = last
  item id = i
  last = item
)
first next = last

write("setup ring of ", maxThreads, " active threads\n")
write("starting message ring\n")
write("first = ", first, "\n")
first @@foo
write("running for 10 seconds...\n")
wait(1)
exit


