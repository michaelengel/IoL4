
test = method(

IM = 139968
IA =   3877
IC =  29573

LAST = 42
genrandom = method(max,
    LAST = LAST * IA + IC 
    LAST = LAST mod(IM)
    (max * LAST) / IM
)

heapsort = method(n, ra, 
    l = n / 2 floor + 1
    ir = n
    while(1,
	if(l > 1,
	    l = l - 1
	    rra = ra at(l)
	,
	    rra = ra at(ir)
	    ra[ir] = ra at(1)
	    ir = ir - 1
	    if(ir == 1,
		ra atPut(1, rra)
		return
	    )
	)
	i = l
	j = l * 2
	while(j <= ir,
	    if((j < ir) and (ra at(j) < ra at(j + 1)),
		j = j + 1
	    )
	    if(rra < ra at(j),
		ra atPut(i, ra at(j))
		i = j
		j = j + i
	    ,
		j = ir + 1
	    )
	)
	ra atPut(i, rra)
    )
)

ary = List clone

if(hasSlot("args"),
  N = args at(0),
  N = 1
)

for(i, 1, N, ary add(genrandom(1.0)) )

heapsort(N, ary)

write(ary[N], "\n")
)

test

