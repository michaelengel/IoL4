// an asynchronous feed-forward neural network
// messages that begin with a colon are asynchronous

// +++ UNFINIHSED +++ 

// --------------------------------------------------
// Connection
// --------------------------------------------------

Connection = Object clone
Connection weight = 1
Connection target = Nil
Connection input = method(v, self target @input(v * self weight))

// --------------------------------------------------
// Neuron
// --------------------------------------------------

Neuron = Object clone
Neuron value = 0
Neuron decayRate = .9
Neuron connections = List clone
Neuron init = method(self connections = self connections clone)
Neuron setValue = method(v, value = 0; self input(v))
Neuron input = method(v, 
  self value += v
  if (self value > 1, self fire)
  if (self isActive == Nil, self @timestep)
)

Neuron timestep = method(
  self value *= decayRate
  if (self value != 0 and(decayRate != 1), self @timestep)
)

Neuron fire = method(
  self connections foreach(i, con, con input(i)
  self value = 0
)

Neuron connectTo = method(aNeuron, 
  con = Connection clone
  con target = aNeuron
  self connections add(con)
  return con
)

// --------------------------------------------------
// Layer
// --------------------------------------------------

Layer = Object clone
Layer neurons = List clone
Layer init = method(self neurons = self neurons clone)
Layer setSize = method(size,
  while (self neurons count < size, self neurons add(Neuron clone))
)
Layer setValues = method(values,
  for(i, 0, values count,
    self neurons at(i) ?setValue(values at(i))
  )
)

Layer setDecayRate = method(r, neurons foreach(i, neuron, neuron decayRate = r))

Layer connectTo = method(layer,
  neurons foreach(i, myNeuron,
    layer neurons foreach(n, otherNeuron,
      myNeuron connectTo(otherNeuron) weight = .5
    )
  )
)

Layer print = method(
  self neurons foreach(i, neuron, write(neuron value asString("%.2f"), " "))
  write("\n")
)

Layer isStillActive = method(
  self neurons foreach(i, neuron, if (neuron isActive, return 1))
  return Nil
)

// --------------------------------------------------
// Net
// --------------------------------------------------

Net = Object clone
Net layers = List clone
Net init = method(self layers = self layers clone)
Net inputLayer = method(self layers first)
Net outputLayer = method(self layers last)

Net addLayerOfSize = method(size,
  layer = Layer clone
  layer setSize(size)
  outputLayer connectTo(layer)
  layers add(layer)
)

Net run = method(
  self print
  while(self isStillActive, self @run)
)

Net print = method(
  write("net inputs:  "); inputLayer print; write("\n")
  write("net outputs: "); outputLayer print; write("\n\n")
)

Net isStillActive = method(
  self layers foreach(i, layer, if (layer isStillActive, return 1))
  return Nil
)

// --------------------------------------------------
// Example use
// --------------------------------------------------

xor = Net clone
xor addLayerOfSize(2)
xor addLayerOfSize(2)
xor outputLayer setValues(List clone add(0,0))
xor outputLayer setDecayRate(1)
xor inputLayer setValues(List clone add(0,1))
Net run

