import os


class DBMgr(object):

    __instance = None

    def __new__(cls, *args, **kwd):
        if DBMgr.__instance is None:
            DBMgr.__instance = object.__new__(cls, *args, **kwd)
        return DBMgr.__instance


    def __init__(self, db_file_name='server/db'):
        self.db_file_name = db_file_name

    def sign_in(self, usr_name, usr_pwd):
        with open(self.db_file_name, 'r+') as f:
            for d in f:
                for kv in [d.strip().split(' ')]:
                    if usr_name == kv[0] and usr_pwd == kv[1]:
                        return 0, kv[2]
        return DBState.SIGN_IN_USR_NOT_FOUND, -1

    def sign_out(self, usr_name):
        with open(self.db_file_name, 'rw') as f:
            for d in f:
                for kv in [d.strip().split(' ')]:
                    if usr_name == kv[0]:
                        return 0
        return DBState.SIGN_OUT_USR_NOT_FOUND

    def register(self, usr_name, usr_pwd):
        if not self.check_info_valid(usr_name):
            return DBState.REGISTER_USR_NAME_NOT_VALID
        if not self.check_info_valid(usr_pwd):
            return DBState.REGISTER_USR_PWD_NOT_VALID

        with open(self.db_file_name, 'r+') as f:
            for d in f:
                for kv in [d.strip().split(' ')]:
                    if usr_name == kv[0]:
                        return DBState.REGISTER_USR_ALREADY_EXIST
            f.write(usr_name + ' ' + usr_pwd + ' 0\n')

        return 0

    def check_info_valid(self, info):
        if info is None:
            return False
        return True
if __name__ == '__main__':
    print os.getcwd()
    print DBMgr().sign_in('netease1','123')

class DBState(object):
    SIGN_IN_USR_ALREADY_ONLINE = 1
    SIGN_IN_USR_NOT_FOUND = 2

    SIGN_OUT_USR_ALREADY_OFFLINE = 1
    SIGN_OUT_USR_NOT_FOUND = 2

    REGISTER_USR_ALREADY_EXIST = 1
    REGISTER_USR_NAME_NOT_VALID = 2
    REGISTER_USR_PWD_NOT_VALID = 3
