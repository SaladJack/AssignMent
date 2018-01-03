import json
from json import JSONEncoder


class P4S(object):
    def __init__(self):
        self.from_id = 0
        self.to_id = 0
        self.room_id = 0
        self.msg = ""
        self.type = 0
        self.result_id = 0

    def __init__(self, data):
        self.msg = data["msg"]
        self.from_id = data["from_id"]
        self.to_id = data["data_id"]
        self.room_id = data["room_id"]
        self.type = data["type"]
        self.result_id = data['result_id']

    def __init__(self, from_id=0, to_id=0, room_id=0, msg="", type=0):
        self.from_id = from_id
        self.to_id = to_id
        self.room_id = room_id
        self.msg = msg
        self.type = type


    def toJSON(self):
        return json.dumps(self,default=lambda o: o.__dict__, sort_keys=True, indent=4)

    @staticmethod
    def toObject(self,data):
        data_dict = json.loads(data)
        p = P4S(data_dict)
        return p

    def isLobbyChat(self):
        return self.room_id == 0

    def isPrivateChat(self):
        return self.from_id != 0 and self.to_id != 0

    def isRoomChat(self):
        return self.room_id != 0

class P4SvrType(object):
    TYPE_CREATE_ROOM = 1
    TYPE_ENTER_ROOM = 2
    TYPE_QUIT_ROOM = 3
    TYPE_LOBBY_CHAT = 4
    TYPE_PRIVATE_CHAT = 5
    TYPE_ROOM_CHAT = 6
    TYPE_SIGN_IN = 7
    TYPE_SIGN_OUT =8
    TYPE_REGISTER = 9

class P4SvrRsp(object):
    RSP_CREATE_ROOM_SUCCESS = 1 << 15
    RSP_CREATE_ROOM_OVER_CAP = 1 << 15 + 1

    RSP_ENTER_ROOM_SUCCESS = 1 << 14
    RSP_ENTER_ROOM_ID_NOT_EXIST = 1 << 14 + 1
    RSP_ENTER_ROOM_OVER_CAP = 1 << 14 + 2

    RSP_QUIT_ROOM_SUCCESS = 1 << 13
    RSP_QUIT_ROOM_ALREADY_IN_LOBBY = 1 << 13 + 1
