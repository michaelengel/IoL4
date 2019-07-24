x = "a b c d" split(" ")
y = "1 2 3 4" split(" ")
x foreach(i,n,
 y foreach(j,m,
  if(j >= i, continue);
  if(j == 2, "two" linePrint;
	"z0\n" print;
  )
  "z1\n" print;
 )
 "z2\n" print;
)
"z3\n" print;