
Consumer = Object clone
Consumer consumed = 0
Consumer run = method(n,
  i = 0
  while(i < n,
        i <- Producer nextValue
        //write("Consumer << ", i, "\n")
	yield
        consumed ++
  )
  write("Consumer done\n")
)
Consumer done = method(self)

Producer = Object clone
Producer produced = 0
Producer data = 0
Producer run = method(n,
  for(i, 1, n,
        self data <- i
        //write("Producer >> ", data, "\n")
        produced ++
	self pause
	//yield
  )
  write("Producer done\n")
)

Producer nextValue = method(
  self resume
  data
)

main = method(n,
  setSchedulerSleepSeconds(0)
  Producer @run(n)
  Consumer @run(n)
  Consumer @(done) value
  write("produced:", Producer produced, " consumed:", Consumer consumed, "\n")
)

n = ?args at(0) asNumber
main(if(n, n, 100))


