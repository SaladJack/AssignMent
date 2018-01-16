import os

import time
import os

class DBState(object):
    SIGN_IN_USR_ALREADY_ONLINE = 1
    SIGN_IN_USR_NOT_FOUND = 2
    SIGN_IN_USR_PWD_INCORRECT = 3

    SIGN_OUT_USR_ALREADY_OFFLINE = 1
    SIGN_OUT_USR_NOT_FOUND = 2

    REGISTER_USR_ALREADY_EXIST = 1
    REGISTER_USR_NAME_NOT_VALID = 2
    REGISTER_USR_PWD_NOT_VALID = 3


    def __init__(self):
        pass


class DBMgr(object):

    __instance = None

    def __new__(cls, *args, **kwd):
        if DBMgr.__instance is None:
            DBMgr.__instance = object.__new__(cls, *args, **kwd)
        return DBMgr.__instance


    def __init__(self, db_file_name='db'):
        dir = os.path.dirname(os.path.realpath(__file__))

        self.db_file_name = '{}/{}'.format(dir, db_file_name)

    def sign_in(self, usr_name, usr_pwd):
        """
            update the time stamp when client sign in successfully
        """
        result_id = DBState.SIGN_IN_USR_NOT_FOUND
        usr_id = -1
        usr_online_time = 0
        #lines = []
        with open(self.db_file_name, 'r+') as f:
            lines = f.readlines()
            for i, line in enumerate(lines):
                for kv in [line.strip().split(' ')]:
                    if usr_name == kv[0]:
                        if usr_pwd == kv[1]:
                            lines[i] = '{} {} {} {} {}\n'.format(kv[0], kv[1], kv[2], int(time.time()), kv[4])
                            result_id = 0
                            usr_id = kv[2]
                            usr_online_time = kv[4]
                        else:
                            result_id = DBState.SIGN_IN_USR_PWD_INCORRECT

        with open(self.db_file_name, 'w') as f:
            f.writelines(lines)

        return result_id, usr_id, usr_online_time

    def sign_out(self, usr_name):
        """
            update the online time when client sign out successfully
        """
        result_id = DBState.SIGN_OUT_USR_NOT_FOUND
        with open(self.db_file_name, 'r+') as f:
            lines = f.readlines()
            for i, line in enumerate(lines):
                for kv in [line.strip().split(' ')]:
                    if usr_name == kv[0]:
                        result_id = 0
                        lines[i] = '{} {} {} {} {}\n'.format(kv[0], kv[1], kv[2], kv[3], int(time.time()) - int(kv[3]))

        with open(self.db_file_name, 'w') as f:
            f.writelines(lines)
        return result_id

    def register(self, usr_name, usr_pwd):
        if not self.check_info_valid(usr_name):
            return DBState.REGISTER_USR_NAME_NOT_VALID
        if not self.check_info_valid(usr_pwd):
            return DBState.REGISTER_USR_PWD_NOT_VALID

        ret = False
        with open(self.db_file_name, 'r+') as f:
            lines = f.readlines()
            for i, line in enumerate(lines):
                for kv in [line.strip().split(' ')]:
                    if usr_name == kv[0]:
                        return DBState.REGISTER_USR_ALREADY_EXIST
                if line.strip() == '':
                    lines[-1] = '{} {} {} {} {}\n'.format(usr_name, usr_pwd, len(lines), 0, 0)
                    ret = True
            if ret is False:
                f.write('{} {} {} {} {}\n'.format(usr_name, usr_pwd, len(lines) + 1, 0, 0))

        if ret:
            with open(self.db_file_name, 'w+') as f:
                f.writelines(lines)
        return 0

    def check_info_valid(self, info):
        if info is None:
            return False
        return True
if __name__ == '__main__':
    print __file__
    print(os.path.dirname(os.path.realpath(__file__)))
    # dbMgr = DBMgr('db')
    # print dbMgr.sign_in('netease1', '123')
    # dbMgr.register("hello",'sss')
"""
netease1 123 1 1515646605 0
netease2 123 2 0 0
netease3 123 3 1515646610 0
"""

