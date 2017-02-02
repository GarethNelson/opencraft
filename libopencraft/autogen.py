import json
from quarry.data import packets as q_packets

fd = open('burger_data/1.11.json','rb')
json_data = json.loads(fd.read().strip('\n'))
fd.close()

packets_names  = {} # map packet class names to shorter names
packets_fields = {} # map packet class names to lists of field data types
packets_idents = {} # map packet class names to packet ID integers

unknown_packets = {(315, u'handshaking', 'upstream', 0):'handshake'}
PROTOCOL_VER = 315

protostates_packets = {'play':[],'status':[],'login':[],'handshaking':[]}
packets_clientbound = {}

for k,v in json_data[0]['packets']['packet'].items():
    protover  = PROTOCOL_VER
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
    else:
       if unknown_packets.has_key(key):
          packet_name = unknown_packets[key]
    packfields = []
    for field in v['instructions']:
        if field.has_key('type'):
           if field['type']=='byte[]':
             packfields.append('bytes')
           else:
             packfields.append(field['type'])
        else:
           packfields.append('bytes')
    k = '%s_%s_%s' % (packet_name,protomode,packdir)

    packets_clientbound[k] = (packdir=='downstream')
    protostates_packets[protomode].append(k)
    packets_names[k]  = packet_name
    packets_fields[k] = packfields
    packets_idents[k] = pack_id

alphabet ='abcdefghijklmnopqrstuvwxyz'

header_fd = open('include/libopencraft/packets.autogen.h','w')
cpp_fd    = open('src/packets.autogen.cpp','w')
idents_fd = open('include/libopencraft/idents.autogen.inc','w')

header_fd.write("// AUTOGENERATED - See autogen.py, do not modify directly\n")
header_fd.write("#pragma once\n")
header_fd.write("#include <string>\n")
header_fd.write("#include <tuple>\n")
header_fd.write("#include <libopencraft/base_packet.h>\n")
header_fd.write("namespace opencraft {\n namespace packets {\n")

cpp_fd.write("// AUTOGENERATED - See autogen.py, do not modify directly\n")
cpp_fd.write("#include <libopencraft/base_packet.h>\n")
cpp_fd.write("#include <libopencraft/packets.autogen.h>\n")
cpp_fd.write("namespace opencraft {\n namespace packets {\n")





# we need this cos burger isn't perfect
# basically we override what burger said with manually defined types
overrides = {'login_success_login_downstream':('login_success',0x02,['string8','string8']),
             'join_game_play_downstream':('join_game',0x23,['int','byte','enum','byte','byte','string8','boolean'])}

for k,v in overrides.items():
    packets_names[k]  = v[0]
    packets_idents[k] = v[1]
    packets_fields[k] = v[2]

mctypes = {'boolean' :'bool',
           'byte'    :'char',
           'char'    :'char',
           'string16':'std::string',
           'chatcomponent':'std::string',
           'double'  :'double',
           'float'   :'float',
           'int'     :'int32_t',
           'long'    :'int64_t',
           'short'   :'int16_t',
           'string8' :'std::string',
           'varint'  :'int32_t',
           'position':'std::tuple<int,int,int>',
           'enum'    :'int32_t',
           'bytes'   :'std::vector<unsigned char>'}

skipped_packets = set()
for k,v in packets_fields.items():
    skip = False
    params      = []
    param_types = []
    for f in xrange(len(v)):
        if not mctypes.has_key(v[f]):
           print 'WARNING: Can not autogenerate %s due to missing type %s ' % (k,v[f])
           print "         %s(%s)" % (k,','.join(v))
           skipped_packets.add(k)
           skip = True
        else:
           param_types.append(v[f])
           params.append('%s %s' % (mctypes[v[f]],alphabet[f]))
    if not skip:
       # write out the header first
       header_fd.write("class %s : public opencraft_packet {\n" % k)
       header_fd.write("  public:\n")
       header_fd.write("    %s(const std::vector<unsigned char>& packdata);\n" % k)      # constructor from data, not including the ident field at the start, used by static unpack() method
       header_fd.write("    %s(%s);\n" % (k,','.join(params))) # constructor from values
       header_fd.write("    std::string name() { return \"%s\"; }\n" % packets_names[k])
       header_fd.write("    uint32_t ident() { return %s; }\n" % packets_idents[k])
       for f in xrange(len(params)):
           header_fd.write("    %s; //%s\n" % (params[f],v[f]))
       header_fd.write("};\n")

       # then write out the ident
       idents_fd.write('#define OPENCRAFT_PACKIDENT_%s %s\n' % (k.upper(), packets_idents[k]))

       # then write out the class implementation
       cpp_fd.write("%s::%s(const std::vector<unsigned char>& packdata) {\n" % (k,k)) # constructor from data
       cpp_fd.write("   this->packed = packdata;\n")
       cpp_fd.write("   this->bufpos = 0;\n")
       for f in xrange(len(v)): # constructor from data
           cpp_fd.write("   this->%s = this->unpack_%s();\n" % (alphabet[f],param_types[f]))
       cpp_fd.write("}\n")
       cpp_fd.write('%s::%s(%s) {\n' % (k,k,','.join(params)))         # constructor from values

       for f in xrange(len(v)):
           cpp_fd.write("   this->%s = %s;\n" % (alphabet[f],alphabet[f]))

       cpp_fd.write("   this->bufpos = 0;\n");
       for f in xrange(len(v)):
           cpp_fd.write("   this->pack_%s(this->%s);\n" % (param_types[f],alphabet[f]))
       cpp_fd.write('}\n')

unpack_fd = open('include/libopencraft/unpack_packet.inc','w')

for pstate in protostates_packets.keys():

    unpack_fd.write('if(proto_state==OPENCRAFT_STATE_%s && client_bound) {\n' % pstate.upper())
    unpack_fd.write('   switch(raw_pack.pack_ident) {\n')
    for k in protostates_packets[pstate]:
	if packets_clientbound[k] and (not k in skipped_packets):
	   unpack_fd.write('      case %s:\n' % packets_idents[k])
	   unpack_fd.write('       retval = new %s(raw_pack.pack_data);\n' % k)
	   unpack_fd.write('      break;\n')
    unpack_fd.write('   }\n')
    unpack_fd.write('} else if (proto_state==OPENCRAFT_STATE_%s) {\n' % pstate.upper())
    unpack_fd.write('   switch(raw_pack.pack_ident) {\n')
    for k in protostates_packets[pstate]:
	if not packets_clientbound[k] and (not k in skipped_packets):
	   unpack_fd.write('      case %s:\n' % packets_idents[k])
	   unpack_fd.write('       retval = new %s(raw_pack.pack_data);\n' % k)
	   unpack_fd.write('      break;\n')
    unpack_fd.write('   }\n')
    unpack_fd.write('}\n')

cpp_fd.write("}\n}\n");       
header_fd.write("}\n}\n");
header_fd.close()
cpp_fd.close()
unpack_fd.close()
idents_fd.close()

