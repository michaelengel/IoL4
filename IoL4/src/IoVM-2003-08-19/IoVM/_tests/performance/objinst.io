// Note: this demo takes advantage of differential inheritance 
//debug = 1

Toggle = Object clone
Toggle value = 0
Toggle callit = method(value toggle; self)

NthToggle = Toggle clone
NthToggle countMax = 0
NthToggle counter = 0
NthToggle callit = method(
  counter ++
  if(counter >= countMax,
    value toggle
    counter <- 0
  )
  self
)

NUM = if(hasSlot("args"), args at(0) asNumber, 1)

toggle = Toggle clone
toggle value = 1
for(i, 0, 4,
  if(toggle callit value, write("true ", i, "\n"), write("false ", i, "\n"))
)

t = Toggle clone
t value = 1
for(i, 0, NUM, toggle = t clone)

write("\n")

ntoggle = NthToggle clone
ntoggle value = 1
ntoggle countMax = 3
for(i, 0, 7,
  if(ntoggle callit value, write("true ", i, "\n"), write("false ", i, "\n"))
)

nt = NthToggle clone
nt value = 1
nt countMax = 3
for(i, 0, NUM, ntoggle = nt clone)

