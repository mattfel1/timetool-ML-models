import Constellation_pb2
import tensorflow as tf
from google.protobuf import text_format
from google.protobuf.any_pb2 import Any
import google.protobuf

'''
with tf.gfile.FastGFile("../output/graph.pbtxt",'r') as f:
    graph_def = tf.GraphDef()
    #print(f.read())
    graph_def.ParseFromString(f.read())
    tf.import_graph_def(graph_def, name='lattice_graph')

'''


with open('../output/graph.pbtxt') as f:
    txt = f.read()

graph_def = text_format.Parse(txt, tf.GraphDef())


counter = 0
prefix = "tfl_calibrated_rtl/lattice_"
suffix = "/stack"
input_prefix = "tfl_calibrated_rtl/unstack:"

lattices = []


for node in graph_def.node:
    params = []
    if node.name == (prefix + str(counter) + suffix):
        str_array = node.input
        for elem in str_array:
            if len(elem) == len(input_prefix) - 1:
                params.append(0)
            else:
                val = int(elem[len(input_prefix):])
                params.append(val)
        
        lattices.append(params)
         
        # Assuming lattices are named as lattice_# by tensorflow
        counter = counter + 1




program_graph = Constellation_pb2.ProgramGraph()

program_graph.name = "rtl_calibrated_lattice_network"
#nodes = program_graph.nodes.add()

#Hack to make edge list
for idx in range(1,len(lattices)):
    
    #edge = Constellation_pb2.Edge()
    edge = program_graph.edges.add()
    
    edge.inputNodeId = idx - 1
    edge.outputNodeId = idx
    edge.inputParameterIndex = 0
    edge.outputParameterIndex = 0
    

node_map = {}

for lidx,lattice in enumerate(lattices):
    node = Constellation_pb2.Node()
    
    
    # Hardcoded name to match on for now

    parameters = Constellation_pb2.Parameter()
    name = "lattice_" + str(lidx) + "/params"
    parameters.name = name
    iv = Constellation_pb2.IntVector()
    for pidx,param in enumerate(lattice):

        iv.values.append(param)
        
    #parameters.values.extend(params)
    param_value = parameters.values.add()
    param_value.Pack(iv)

    # Is this right?
    data_format_in = Constellation_pb2.DataFormat()
    data_format_in.name = "float32"
    
    data_format_out = Constellation_pb2.DataFormat()
    data_format_out.name = "float32"
    
    #param_map = {}
    #param_map[name] = parameters

    node = Constellation_pb2.Node() 
    node.name="hypercube"
    node.parameters[name].CopyFrom(parameters)
    it = node.inputTypes.add() 
    it = data_format_in
    ot = node.outputTypes.add()
    ot = data_format_out
    #node_map[lidx] = node

    program_graph.nodes[lidx].CopyFrom(node)

f = open("test.pb", "w")
f.write(program_graph.SerializeToString())
f.close()

#print(text_format.MessageToString(program_graph))
    









