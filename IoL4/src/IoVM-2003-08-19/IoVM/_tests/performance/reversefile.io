
fileName = ?args at(0)
file = if(fileName, File open(fileName), File standardInput)
file readLines reverse
//foreach(i, v, write(v, "\n"))