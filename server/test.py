import sys

from dbMgr import DBMgr
import json

from protocol import Protocol

if __name__ == "__main__":
    #
    # dbMgr = DBMgr()
    #
    # cmd_type = sys.argv[1]
    # if cmd_type == 'i':
    #     usr_name = sys.argv[2]
    #     usr_pwd = sys.argv[3]
    #     print 'Sign In Code %d' % dbMgr.sign_in(usr_name,usr_pwd)
    # elif cmd_type == 'o':
    #     usr_name = sys.argv[2]
    #     print 'Sign Out Code %d' % dbMgr.sign_out(usr_name)
    # elif cmd_type == 'r':
    #     usr_name = sys.argv[2]
    #     usr_pwd = sys.argv[3]
    #     print 'Register Code %d' % dbMgr.register(usr_name,usr_pwd)


    # data1 = {'b': 789, 'c': 456, 'a': 123}
    # encode_json = json.dumps(data1)
    # print type(encode_json), encode_json
    #
    # decode_json = json.loads(encode_json)
    # print type(decode_json)
    # print decode_json['a']
    # print decode_json


    p = Protocol()
    print type(p.toJSON())

    print type(json.loads(p.toJSON()))





