
//debugCollector = 1

basicTests = method(
  Lobby doFile("tests/Account.io")
  Lobby doFile("tests/Blocks.io")
  Lobby doFile("tests/Conditions.io")
  Lobby doFile("tests/ControlFlow.io")
  Lobby doFile("tests/Exceptions.io")
)

total = 0

dir = launchPath appendPath("/performance/")

runit = method(fileName, arg, arg2,
  path = dir appendPath(fileName)
  write("---------------------------------------\n")
  write("running: ", path, " ", arg, " ", if(arg2, arg2, " "), "\n")
  t = Date secondsToRun(Lobby doFile(path, arg, arg2))
  write("[", t, " seconds]\n")
  total += t
)

runit("ary3.io", 10)
runit("except.io", 3000)
runit("fibo.io", 10)
runit("hash.io", 10000)
runit("hash2.io", 2)
////runit("heapsort.io", 10)
runit("hello.io", 10)
runit("lists.io", 2)
////runit("matrix.io", 10)
runit("methcall.io", 10)
//runit("moments.io", dir appendPath("data/columns.txt"))
runit("nestedloop.io", 10)
runit("objinst.io", 10)
runit("random.io", 10)
//runit("regexmatch.io", 10)
//runit("reversefile.io", dir appendPath("data/prince.txt"))
runit("sieve.io", 3)
//runit("spellcheck.io", 
//  dir appendPath("data/words"), 
//  dir appendPath("data/prince.txt"))
runit("strcat.io", 10000)
runit("sumcol.io", dir appendPath("data/columns.txt"))
runit("wc.io", dir appendPath("data/prince.txt"))
runit("wordfreq.io", dir appendPath("data/prince.txt"))

write("time for all tests: ", total, " seconds\n")
