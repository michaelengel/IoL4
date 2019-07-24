ObjcBridge debugOn

IoConverter = Object clone
IoConverter setInput:  = block(v, self input = v)
IoConverter setOutput: = block(v, self output = v)
IoConverter convert:   = block(sender,
  //output setIntValue:(input intValue * 2)
  output foo
)

ObjcBridge newClassWithNameAndProto("Converter", IoConverter)
