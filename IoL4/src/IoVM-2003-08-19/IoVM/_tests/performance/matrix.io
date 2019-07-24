
if (hashSlot("args"),
  N = args at(0) asNumber,
  N = 1
)

size = 30

Matrix = Object clone
Matrix setSize = method(numRows, numCols,
    self rows = List clone
    self height = numRows
    self width = numCols
    count = 1
    for(i, 1, numRows,
	row = self rows add(List clone)
	for(j, 1, numCols, row add(count ++))
    )
)

Matrix at = method(x, y, self rows at(x) at(y)) 
Matrix atPut = method(x, y, v, self rows at(x) atPut(y, v)) 

Matrix setSlot("*", method(m2,
    m3 = Matrix clone setSize(self height, self width)
    for(i, 0, rows - 1,
        for(j, 0, cols - 1,
            v = m3 at(i, j)
            for(k, 0, cols - 1, v += self at(i, k) *= m2 at(k, j) )
        )
    )
    return(m3)
))

m1 = Matrix clone setSize(size, size)
m2 = Matrix clone setSize(size, size)
for(i, 1, n, mm = m1 * m2)

write(mm at(0,0), mm at(2,3), mm at(3,2), mm at(4,4), "\n")
