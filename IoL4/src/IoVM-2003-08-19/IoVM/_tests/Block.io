
// example of a lexically scoped block (Like a Smalltalk block)
// noticed that "b" is found in the context in which the block is created

Loop = Object clone
Loop doBlock = method(aBlock,
  write("2 + b = ", aBlock(2), "\n")
)

Test = Object clone
Test foo = method(
  b = 3
  Loop doBlock(block(a, a + b))
)

Test foo