
test = method(

if(?(args) and args at(0),
  stdin = File clone open(args at(0)),
  stdin = File standardInput
)

sum = 0

line = stdin readLine
while(line,
    sum += line asNumber
    line = stdin readLine
)

write(sum, "\n")

)

test
