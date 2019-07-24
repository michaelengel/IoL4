
NULL = 0 asCharacter
b = Buffer clone
b append("proxyId", NULL)
b append("methodName")
b append(NULL)

list = b split(0 asCharacter)
list foreach(i, v, write(v, "\n"))