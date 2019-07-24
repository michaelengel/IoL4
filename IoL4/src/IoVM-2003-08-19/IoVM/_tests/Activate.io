
// example of an Object that invokes a method when it is accessed in a slot


inc = Object clone 
inc count = 0
inc activate = method(v, count += v)

write("inc(1) = ", inc(1), "\n")
write("inc(2) = ", inc(2), "\n")