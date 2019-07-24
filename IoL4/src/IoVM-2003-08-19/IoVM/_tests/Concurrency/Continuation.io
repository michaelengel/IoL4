
a = Object clone
a foo = method(
  "2 - in foo, pausing\n" print
  self pause
  "4 - in foo, after pause\n" print
)

a @foo
"1 - in main, sent async message, yielding\n" print

yield
"3 - in main, after yield\n" print

a resume; yield
"5 - in main, after resume\n" print
