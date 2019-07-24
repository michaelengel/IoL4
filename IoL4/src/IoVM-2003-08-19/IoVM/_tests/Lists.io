"Start\n" print

/*
image = List clone sizeTo(1000)
x = 0
for (i,1,1000,
  l = List clone sizeTo(1000)
  image atPut(i, l)
  for (j,1,1000, l atPut(j,"0") )
)

"Done\n" print
image at(500) at(500) print
"End\n" print
*/

BImage = Object clone
BImage width = 0
BImage height = 0
BImage init = method(self data = Buffer clone)
BImage setSize = method(w, h, self width = w; self height = w; self)
BImage indexAt = method(x, y, index = y; index *= width += x)
BImage at = method(x, y,
  self data at(self indexAt(x, y))
)

BImage atPut = method( 
  index = thisMessage argAt(1) doInContext(sender); index *= width += thisMessage argAt(0) doInContext(sender)
  self data atPut(index, thisMessage argAt(2) cachedResult)
)

image = BImage clone setSize(1000, 1000)

for (i,1,1000,
  for (j,1,1000, image atPut(i,j,0) )
)