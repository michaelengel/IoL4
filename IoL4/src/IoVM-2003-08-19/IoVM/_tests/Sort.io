
words = List clone
words add("video ")
words add("killed ")
words add("the ")
words add("radio ")
words add("star ")
"original: " print;words do(method(v, v print)); "\n" print

words sortBy(method(a, b, a < b))
"sort1: " print; words do(method(v, v print)); "\n" print

// ok now without cheating

words randomize
"randomized: " print; words do(method(v, v print)); "\n" print


for(i, 0, words count,
  done = 1
  for(j, i, words count - 1, words
    if(words at(j) > words at(j + 1), words swapIndexWithIndex(j, j + 1); done = Nil)
  )
  if(done, break)
)

"sort2: " print;words do(method(v, v print)); "\n" print
