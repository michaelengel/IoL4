
Visual = Object clone
Visual addChild = method(child, child parent = self)

Window = Visual clone
Window addChild = method(child, super addChild(child))

button = Object clone 
Window addChild( button )

write("Window = ", Window, "\n")
write("button = ", button, "\n")
