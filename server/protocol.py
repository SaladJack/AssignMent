import json
from json import JSONEncoder


class Protocol(object):
    def __init__(self):
        self.from_id = 0
        self.to_id = 0
        self.room_id = 0
        self.msg = ""
        self.code = 0

    def __init__(self, data):
        self.msg = data["msg"]
        self.from_id = data["from_id"]
        self.to_id = data["data_id"]
        self.room_id = data["room_id"]
        self.code = data["cmd"]

    def __init__(self, from_id=0, to_id=0, room_id=0, msg="", cmd=0):
        self.from_id = from_id
        self.to_id = to_id
        self.room_id = room_id
        self.msg = msg
        self.code = cmd


    def toJSON(self):
        return json.dumps(self,default=lambda o: o.__dict__, sort_keys=True, indent=4)

    @staticmethod
    def toObject(self,data):
        data_dict = json.loads(data)
        p = Protocol(data_dict)
        return p

class CmdProtocol(object):
    CMD_CREATE_ROOM = 1
    CMD_ENTER_ROOM = 2
    CMD_QUIT_ROOM = 3

class RspProtocol(object):
    RSP_CREATE_ROOM_SUCCESS = 1 << 15
    RSP_CREATE_ROOM_OVER_CAP = 1 << 15 + 1

    RSP_ENTER_ROOM_SUCCESS = 2 << 14
    RSP_ENTER_ROOM_ID_NOT_EXIST = 2 << 14 + 1
    RSP_ENTER_ROOM_OVER_CAP = 2 << 14 + 2

    RSP_QUIT_ROOM_SUCCESS = 2 << 13
    RSP_QUIT_ROOM_ALREADY_IN_LOBBY = 2 << 13 + 1
