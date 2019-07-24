/* 
 * basic file tests
 * Miles Egan <miles@caddr.com>
 */

//------------------------------------------------------------------------------ 
// stat tests
//------------------------------------------------------------------------------ 
inspect = method(f, stat,
    write("inspecting ", f path, "\n")
    
    write("mode : ", stat mode, "\n")
    write("atime : ", stat atime, "\n")
    write("ctime : ", stat ctime, "\n")
    write("mtime : ", stat mtime, "\n")
    write("uid : ", stat uid, "\n")
    write("gid : ", stat gid, "\n")
    write("size : ", stat size, "\n")
    if(f isDir, "type: directory" linePrint) 
    if(f isFifo, "type: fifo" linePrint) 
    if(f isLink, "type: link" linePrint) 
    if(f isReg, "type: regular file" linePrint) 
    if(f isSock, "type: socket" linePrint) 
    write("\n")
)

// path stats
f = File clone

f setPath(launchPath)
inspect(f, f stat)

f setPath("/etc")
inspect(f, f stat)

f setPath("/etc/passwd")
inspect(f, f stat)

f setPath("/usr/lib")
inspect(f, f stat)

f setPath("/nofilehereforsure")
catchException("File.stat",
    inspect(f, f stat),
    write("caught exception: ", exceptionName, " : ", exceptionDescription, "\n")
)

// fstat
f = File openForReading("/etc/fstab")
inspect(f, f stat)
inspect(f, f fstat)
