//debug = 1

Toggle = Object clone
Toggle value = 0
Toggle callit = method(value toggle; self)

NthToggle = Toggle clone
NthToggle countMax = 0
NthToggle counter = 0
NthToggle callit = method(
  counter ++
  if(counter >= countMax, value toggle; counter <- 0)
  self
)

N = if(hasSlot("args"), args at(0) asNumber, 1)

val = 1
toggle = Toggle clone
toggle value = val

for(i, 0, N - 1, val <- toggle callit value)
if(val, write("true\n"), write("false\n"))
    
val = 1
toggle = NthToggle clone
toggle value = val
toggle countMax = 3
for(i, 0, N - 1, val <- toggle callit value)
if(val, write("true\n"), write("false\n"))
