
// intentionally cause an error to see that we
// get a proper stack trace

testing = method(
//test0 = Nil
test1 = method(n, test0(n-1) ) 
test2 = method(n, test1(n-1) )
test3 = method(n, Object clone; test2(n-1) )
test3(3)
)

testing