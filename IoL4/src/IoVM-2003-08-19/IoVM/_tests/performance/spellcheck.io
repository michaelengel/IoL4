
test = method(

dictFile = File clone open(args at(0)),

dict = Map clone

line = dictFile readLine
while(line, 
  dict atPut(line, 1)
  line = dictFile readLine
)

write("loaded ", dict count, " words\n")

dictFile close

stdin = File clone open(args at(1))

line = stdin readLine
while(line, 
  words = line split(" ", "\t", "\n", ",", ".", "!", ";", ":")
  words foreach(i, word, 
    if(dict at(word lower) == Nil, write(word, "\n"))
  )
  line = stdin readLine
)

)
test

