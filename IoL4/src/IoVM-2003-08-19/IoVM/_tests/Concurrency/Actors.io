
/*
a = Object clone
a test = method(for(i, 1, 3, write(i, "\n"); yield))
b = a clone
a @test; b @test

// should print 112233
*/

Test = Object clone
Test test = method(a,
  for(i, 1, 2,
    write(a, "\n")
    wait(.5)
  )
)

Test @test(1); Test @test(2); Test @test(3)
//while(1, yield)

