

a = "foobar"

if(a == "foo", 
  "a is foo\n" print, 
  "a is not foo\n" print
)

command = "C"
if( command == "A" ) then (
    write( "A\n" )
) elseif( command == "C" ) then (
    write( "C\n" )
) else (
    write( "Unknown command '", command, "'\n" )
)
