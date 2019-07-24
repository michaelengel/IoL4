//debug = 1

n = if(hasSlot("args"), args at(0) asNumber, 1)

x = 1
for(a,1,n,
    for(b,1,n,
	for(c,1,n,
	    for(d,1,n,
		for(e,1,n,
		    for(f,1,n,
			x ++
		    )
		)
	    )
	)
    )
)
write(x,"\n")
