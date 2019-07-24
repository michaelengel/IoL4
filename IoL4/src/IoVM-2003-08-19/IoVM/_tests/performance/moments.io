if(hasSlot("args") and args at(0),
  stdin = File open(args at(0)),
  stdin = File standardInput
)

nums = List clone
sum = 0
while(1, 
  line = stdin readLine
  if(line == Nil, break)
  num = line asNumber
  sum += num
  nums add(num)
)
n = nums count

write("n = ", n, "\n")

mean = sum / n

averageDeviation = variance = skew = kurtosis = 0

for(i, 0, n - 1,
  deviation = nums at(i) - mean
  averageDeviation += deviation abs
  variance += deviation ^ 2
  skew += deviation ^ 3
  kurtosis += deviation ^ 4
)

averageDeviation /= n
variance /= n - 1
standardDeviation = variance sqrt
if(variance != 0,
  skew /= n * variance * standardDeviation
  kurtosis /= n * variance * variance 
  kurtosis -= 3
)

nums sort

mid = n / 2 floor

if(n % 2 == 1,
  median = nums at(mid + 1),
  median = nums at(mid) + nums at(mid + 1) / 2
)

write("n:", n, "\n")
write("median:", median, "\n")
write("mean:", mean, "\n")
write("averageDeviation:", averageDeviation, "\n")
write("standardDeviation:", standardDeviation, "\n")
write("variance:", variance, "\n")
write("skew:", skew, "\n")
write("kurtosis:", kurtosis, "\n")
