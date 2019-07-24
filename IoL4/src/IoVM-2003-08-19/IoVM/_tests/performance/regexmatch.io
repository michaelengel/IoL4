// +++ NOT WORKING YET +++

N = if(hasSlot("args"), args at(0) asNumber, 1)

phones = File standardInput readLines

rx = RegularExpression clone

rx compile(
    '(?:^|[^\d\(])' ..			// must be preceeded by non-digit
    '(?:\((\d\d\d)\)|(\d\d\d))' ..	// match 1 or 2: area code is 3 digits
    '[ ]' ..				// area code followed by one space
    '(\d\d\d)' ..			// match 3: prefix of 3 digits
    '[ -]' ..				// separator is either space or dash
    '(\d\d\d\d)' ..			// match 4: last 4 digits
    '\D'				// must be followed by a non-digit
)

count = 0
m = 0
line = 0
iter = 0
for(iter, 1, NUM,
  phones foreach(i, line,
    m = rx match(line)
    if(m != -1,
      num = '(' + (m[1] || m[2]) + ') ' + m[3] + '-' + m[4];
      if(iter == NUM, count ++; write(count. ":", num))
    )
  )
)
