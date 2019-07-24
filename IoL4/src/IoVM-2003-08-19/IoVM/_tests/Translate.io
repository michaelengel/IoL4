 write("For List\n")
 l = List clone add(1, 1, 2, 3, 5, 8)
 
 write("  Before Translation\n")
 l foreach(i,v,
     write("    Item[", i, "] is: ", v, "\n")
 )
 
 l translate(i, v, v * v)
 
 write("  After Translation\n")
 l foreach(i,v,
     write("    Item[", i, "] squared is: ", v, "\n")
 )
 
 write("For Linked List\n")
 l = LinkedList clone add(1, 1, 2, 3, 5, 8)
 
 write("  Before Translation\n")
 l foreach(i,v,
     write("    Item[", i, "] is: ", v, "\n")
 )
 
 l translate(i, v, v * v)
 
 write("  After Translation\n")
 l foreach(i,v,
     write("    Item[", i, "] squared is: ", v, "\n")
 )
 
