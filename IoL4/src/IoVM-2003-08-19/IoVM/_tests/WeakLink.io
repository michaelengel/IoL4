
test = method(
  monitor = Object clone
  monitor collectedWeakLink = method(wlink, 
    write("collected weak link #", wlink identifier, "\n")
  )

  a = Object clone

  w = WeakLink clone setLink(a) 
  w setDelegate(monitor) setIdentifier(w uniqueId)

  collectGarbage
  removeSlot("a")
  collectGarbage

)

test