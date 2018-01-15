import json
from json import JSONEncoder


class P4C(object):
    def __init__(self):
        self.from_id = 0
        self.to_id = 0
        self.room_id = 0
        self.msg = ""
        self.type = 0
        self.result_id = 0

    def loadDict(self,data):
        self.msg = data["msg"]
        self.from_id = data["from_id"]
        self.to_id = data["to_id"]
        self.room_id = data["room_id"]
        self.type = data["type"]
        self.result_id = data['result_id']

    def toJSON(self):
        return json.dumps(self, default=lambda o: o.__dict__, sort_keys=True, indent=4)

    @staticmethod
    def toObject(data):
        data_dict = json.loads(data)
        p = P4C()
        p.loadDict(data_dict)
        return p

    def isLobbyChat(self):
        return self.room_id == 0

    def isPrivateChat(self):
        return self.from_id != 0 and self.to_id != 0

    def isRoomChat(self):
        return self.room_id != 0

class P4CliType(object):
    TYPE_CREATE_ROOM = 1
    TYPE_ENTER_ROOM = 2
    TYPE_QUIT_ROOM = 3
    TYPE_LOBBY_CHAT = 4
    TYPE_PRIVATE_CHAT = 5
    TYPE_ROOM_CHAT = 6
    TYPE_SIGN_IN = 7
    TYPE_SIGN_OUT =8
    TYPE_REGISTER = 9
    TYPE_FIND_USR_INFO_BY_USR_NAME = 10
    TYPE_21_GAME = 11
    TYPE_21_GAME_PLAYER_ANSWER = 12
    TYPE_21_GAME_PLAYER_ANNOUCE_WINNER = 13

class P4SvrRsp(object):
    RSP_CREATE_ROOM_OVER_CAP = 1 << 15 + 1
    RSP_ENTER_ROOM_ID_NOT_EXIST = 1 << 14 + 1
    RSP_ENTER_ROOM_OVER_CAP = 1 << 14 + 2
    RSP_QUIT_ROOM_ALREADY_IN_LOBBY = 1 << 13 + 1
    RSP_PRIVATE_CHAT_TO_USR_ALREADY_OFFLINE = 1 << 12
    RSP_FIND_USR_OFFLINE_OR_NOT_REGISTER_AT_ALL = 1 << 11
    SIGN_IN_USR_ALREADY_ONLINE = 1
    SIGN_IN_USR_NOT_FOUND = 2

    SIGN_OUT_USR_ALREADY_OFFLINE = 1
    SIGN_OUT_USR_NOT_FOUND = 2

    REGISTER_USR_ALREADY_EXIST = 1
    REGISTER_USR_NAME_NOT_VALID = 2
    REGISTER_USR_PWD_NOT_VALID = 3
