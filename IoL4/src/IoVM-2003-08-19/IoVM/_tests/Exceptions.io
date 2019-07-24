
"testing exceptions:\n" print

catchException("Test", 
  raiseException("Test.blah", "this is a test")
  print("got past exception\n"),
  write("got exception: ", exceptionName, " : ", exceptionDescription, "\n")
)


