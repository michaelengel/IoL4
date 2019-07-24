// example of avoiding unwanted activation

Test = Object clone

Test foo = "foobar"

Test test = method(
  write("test\n")
  write(self foo, "\n")
)

Test activate = method(
  if (getSlot("sender") getSlot("proto") == getSlot("self"), return getSlot("self"))
  write("activated\n")
)

getSlot("Test") test