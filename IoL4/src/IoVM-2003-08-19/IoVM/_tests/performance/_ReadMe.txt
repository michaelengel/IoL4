
These are the performance tests from:

  The Great Computer Language Shootout
  http://www.bagley.org/~doug/shootout/

I've rewritten them in Io but not many are tested yet and some call as-yet-unimplemented methods. These will all be working with Io 1.0 release.

Here are some timings run on my 350Mhz G4(averages):
(Python 2.0, Lua 4.0, Io beta 2002-06-02)

methcall.python 100000 = 3.53 seconds
methcall.io 100000     = 2.01 seconds
methcall.lua 100000    = 1.70 seconds

objinst.python 100000 = 4.55 seconds
objinst.io 100000     = 2.68 seconds
objinst.lua 100000    = 4.27 seconds

hash.python 10000 = 0.34 seconds
hash.io 10000     = 0.40 seconds
hash.lua 10000    = 0.28 seconds

hash2.python 10 = 1.47 seconds
hash2.io 10     = 1.40 seconds
hash2.lua 10    = 0.45 seconds

lists.python 3  = 0.76 seconds
lists.io 3      = 1.49 seconds
lists.lua 3     = 0.97 seconds

strcat.python 100000 = 0.65 seconds
strcat.io 100000     = 0.29 seconds
strcat.lua 100000     = 1.49 seconds

except.python 10000 = 1.01 seconds
except.io 10000     = 0.79 seconds
except.lua 10000    = 1.05 seconds

nestedloop.python 12  = 5.65 seconds
nestedloop.io 12      = 3.21 seconds
nestedloop.lua 12     = 1.20 seconds

random.python 100000  = 1.26 seconds
random.io 100000      = 1.46 seconds
random.lua 100000     = 0.58 seconds


