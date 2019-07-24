
test = method(
n = args at(0) asNumber
s = Buffer clone
for(i, 1, n, s append("hello\n"))

write(s length, "\n")
)

test