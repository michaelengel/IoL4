// By Jason Sackett

Cards = List clone
Suits = "a b c d" split(" ")
Values = List clone

for(a,2,14,Values add(a asString))

Suits foreach(i, suit, 
  Values foreach(j, value, Cards add(value .. suit))
)

Deck = Object clone
Deck cards = Cards

Date asNumber setRandomSeed

Deck shuffle  = method(cards randomize)
Deck dealCard = method(cards pop linePrint)

Deck show = method(cards foreach(i, card, write(card, " ")); "\n" print)

Deck show
Deck shuffle
Deck show

Deck dealCard
Deck dealCard
Deck dealCard
