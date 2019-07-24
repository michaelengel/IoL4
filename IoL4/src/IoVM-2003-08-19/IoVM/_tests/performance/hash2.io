

test = method(
n = if(hasSlot("args"), args at(0) asNumber, 1)

hash1 = Map clone
for(i, 0, 10000, 
  hash1 atPut("foo_" .. i, i)
)

hash2 = Map clone
for(i, 1, n,
  hash1 foreach(k, v,
    if(hash2 at(k), 
      hash2 at(k) += v, 
      hash2 atPut(k, v)
    )
  )
)

write(hash1 at("foo_1"), " ",
      hash1 at("foo_9999"), " ",
      hash2 at("foo_1"), " ",
      hash2 at("foo_9999"), "\n")

)

test