
Ack = method(M, N,
  if(M == 0, N + 1,
  if(N == 0, Ack(M - 1, 1),
    Ack(M - 1, Ack(M, N - 1))
  ))
)

NUM = args at(0) asNumber
write("Ack(3,", NUM , "): ", Ack(3, NUM), "\n")
