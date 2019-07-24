test = method(

stdin = File standardInput

if(?(args) and args at(0),
  path = args at(0)
  stdin = File open(path)
)

cc = 0
lc = 0
wc = 0

line = stdin readLine

while(line, 
    cc += line length
    wc += line splitCount(" ", "\t", ";", ":", ".")
    lc ++
    line = stdin readLine
)

write(lc, " ", wc, " ", cc, "\n")

)

test