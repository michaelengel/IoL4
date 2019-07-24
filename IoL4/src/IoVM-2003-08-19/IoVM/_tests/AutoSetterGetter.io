
Object createSlot = method(varName, value,
  self setSlot("_"+varName, value)
  self setSlot(varName, doString("method(_"+varName+")"))
  self setSlot(
    "set"+ varName substring(0, 0) upper + varName substring(1, -1), 
    doString("method(v, self _"+varName+"=v)"))
) 

Person = Object clone
Person createSlot("name", "Joe")

write("Person = ", Person, "\n")

Person setName("Moe")

write("name = ", Person name, "\n")