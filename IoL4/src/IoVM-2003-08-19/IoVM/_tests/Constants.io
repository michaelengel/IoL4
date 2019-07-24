// by Kevin Glen Roy Greer

Constant = Object clone do (
  type = "Constant"; 
  init = method(msg, 
    self msg = msg; 
    self activate = method(thisMessage setName(msg asString); msg)
  )
)

const = method(_obj, Constant clone(_obj));

// Let's try it out:

// Make PI a regular number

PI = 3.1415926

// Make b a method that uses it
b = method(PI+PI)

//write(b,"\n")                  // -> 6.283185
//write(getSlot("b") code,"\n")  // -> block(PI +(PI))

// Make PI a constant

PI = const(3.1415926)

//write(b,"\n")                  // -> 6.283185
write(getSlot("b") code,"\n")  // -> block(3.141593 +(3.141593))
write(getSlot("b") message name ,"\n")  // -> block(3.141593 +(3.141593))

// Still works but when we look at the code of 'b' we see that
// the PI's have been replaced with PI's value!!!

