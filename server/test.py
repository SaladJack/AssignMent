import random

import time

import datetime

if __name__ == "__main__":
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

    # p = P4S()
    # print type(p.toJSON())
    #
    # print type(json.loads(p.toJSON()))

    # f = '-1++2++3++4'
    # f = f.replace("++", "+").replace("+-", "-").replace("-+", "-").replace("- -", "+")
    # a = eval(f)
    # print a

    # print '{} {} {} {}'.format(random.randint(1,10),random.randint(1,10),random.randint(1,10),random.randint(1,10))
    cur_time = 1517288400
    #cur_time = 1515906000
    today = datetime.date.today()
    print int(time.mktime(today.timetuple()))
    print (cur_time - int(time.mktime(today.timetuple()))) % 1800 == 0
    #print (((cur_time + 200) % 100000) % 1800) == 0
