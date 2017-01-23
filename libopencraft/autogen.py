import json
from quarry.data import packets as q_packets

fd = open('burger_data/1.11.json','rb')
json_data = json.loads(fd.read().strip('\n'))
fd.close()

packets_fields = {}

for k,v in json_data[0]['packets']['packet'].items():
    protover  = 315
    protomode = v['state'].lower()
    if v['direction']=='CLIENTBOUND':
       packdir='downstream'
    else:
       packdir='upstream'
    pack_id = v['id']
    key = (protover,protomode,packdir,pack_id)
    packet_name = 'unknown_%s_%s_%s' % (packdir,protomode,pack_id)
    if q_packets.packet_names.has_key(key):
       packet_name = q_packets.packet_names[key]
    packfields = []
    for field in v['instructions']:
        if field.has_key('type'):
           packfields.append(field['type'])
        else:
           packfields.append('bytes')
    k = '%s_%s_%s' % (packet_name,protomode,packdir)
    packets_fields[k] = packfields

alphabet ='abcdefghijklmnopqrstuvwxyz'

header_fd = open('include/packets.autogen.h','w')

header_fd.write("// AUTOGENERATED - See autogen.py, do not modify directly\n")
header_fd.write("#pragma once\n")
header_fd.write("#include <string>\n")
header_fd.write("#include <tuple>\n")
header_fd.write("#include <base_packet.h>\n")
header_fd.write("namespace opencraft {\n namespace packets {\n")


mctypes = {'boolean' :'bool',
           'byte'    :'char',
           'byte[]'  :'char*',
           'char'    :'char',
           'string16':'std::string',
           'double'  :'double',
           'float'   :'float',
           'int'     :'int32_t',
           'long'    :'int64_t',
           'short'   :'int16_t',
           'string8' :'std::string',
           'varint'  :'int32_t',
           'position':'std::tuple<float,float,float>',
           'enum'    :'int32_t',
           'bytes'   :'char*'}


for k,v in packets_fields.items():
    skip = False
    params = []
    for f in xrange(len(v)):
        if not mctypes.has_key(v[f]):
           print 'WARNING: Can not autogenerate %s due to missing type %s ' % (k,v[f])
           skip = True
    if not skip:
       header_fd.write("class %s : public opencraft_packet {\n" % k)
       header_fd.write("  public:\n")
       header_fd.write("    %s(%s);\n" % (k,','.join(params)))
       for p in params:
           header_fd.write("    %s;\n" % p)
       header_fd.write("};\n")
header_fd.write("}\n}\n");
header_fd.close()
