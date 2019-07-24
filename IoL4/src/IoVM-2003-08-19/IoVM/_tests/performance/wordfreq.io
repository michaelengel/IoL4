
test = method(

if(?(args) and args at(0),
  stdin = File open(args at(0)),
  stdin = File standardInput
)

words = Map clone
line = stdin readLine
while(line,
  line split(" ", "\t", ",", ".", "(", ")", ";") foreach(i, word,
    if(word length > 0) then(
      if(words at(word)) then(
        words at(word) ++
      ) else(
        words atPut(word, 1)
      )
    )
  )
  line = stdin readLine
)

wordList = List clone
words foreach(word, wordCount,
  entry = List clone
  entry add(wordCount)
  entry add(word)
  wordList add(entry)
)

wordList sort
wordList reverse

wordList foreach(i, entry, 
  wordCount = entry at(0)
  word = entry at(1)
  //write(word, "\t", wordCount, "\n")
)

entry = wordList at(0)
wordCount = entry at(0)
word = entry at(1)

write("top entry: '", word, "' count:", wordCount, "\n")

/*
if(word == "the" and(wordCount == 1639), 
  write("worked\n"), write("failed\n")
)
*/

)

test
