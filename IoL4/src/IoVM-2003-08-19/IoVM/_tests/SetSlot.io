
// overridding "="

MyObject = Object clone
MyObject setSlot = method(slotName, value,
  write("setting slot \"", slotName, "\" to a ", value type, "\n")
  super setSlot(slotName, value)
) 

MyObject a = 1