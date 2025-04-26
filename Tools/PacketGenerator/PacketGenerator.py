import argparse
import jinja2
import ProtoParser

def main():

    arg_parser = argparse.ArgumentParser(description = 'PacketGenerator')
    arg_parser.add_argument('--path', type=str, default=r'D:\GS_Term\GameServer\Common\proto\bin\Protocol.proto', help='proto path')
    arg_parser.add_argument('--output', type=str, default='TestPacketHandler', help='output file')
    arg_parser.add_argument('--recv', type=str, default='CS_', help='recv convention')
    arg_parser.add_argument('--send', type=str, default='SC_', help='send convention')
    args = arg_parser.parse_args()

    is_db = args.recv.startswith('D') or args.send.startswith('D')
    protocol_ns = 'DBProtocol' if is_db else 'Protocol'
    handler_var_name = 'GDBPacketHandler' if is_db else 'GPacketHandler'
    count_num = 2000 if is_db else 1000
    parser = ProtoParser.ProtoParser(count_num, args.recv, args.send, is_db)
    parser.parse_proto(args.path)
    file_loader = jinja2.FileSystemLoader('Templates')
    env = jinja2.Environment(loader=file_loader)
   
    template = env.get_template('PacketHandler.h')
    output = template.render(parser = parser, output=args.output, protocol_ns=protocol_ns, handler_var_name=handler_var_name)
  
    f = open(args.output+'.h', 'w+')
    f.write(output)
    f.close()

    print(output)
    return

if __name__ == '__main__':
    main()