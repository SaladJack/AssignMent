#!/usr/local/bin/python
# *-* coding:utf-8 -*-

"""
client.py
"""

import sys
import time
import socket
import threading

from p4c import P4C, P4CliType, P4SvrRsp


class Client(object):
    COMMAND_CREATE_ROOM = '/create room'
    COMMAND_ENTER_ROOM = '/enter room'
    COMMAND_QUIT_ROOM = '/quit room'
    COMMAND_ENTER_PRIVATE_CHAT = '/chat to '
    COMMAND_QUIT_PRIVATE_CHAT = '/chat quit'
    COMMAND_SIGN_OUT = '/sign out'
    COMMAND_21_GAME = '/21game'

    # 用户状态码
    IN_LOBBY = 0
    IN_PRIVATE = 1
    IN_ROOM = 2

    def __init__(self, host, port=33333, timeout=1, reconnect=2):
        self.__host = host
        self.__port = port
        self.__timeout = timeout
        self.__buffer_size = 1024
        self.__flag = 1
        self.client = None
        self.__lock = threading.Lock()
        self.online = False
        self.signining = False  # 正在登录
        self.cur_state = Client.IN_LOBBY  # 0:lobby, 1:private 2:room
        self.cur_room_id = 0
        self.usr_id = 0
        self.usr_name = ''
        self.to_usr_id = 0
        self.to_usr_name = ''
        self.usr_name_2_id = {}
        self.game_numbers = [0, 0, 0, 0]
        self.room_21_game_start = False

    @property
    def flag(self):
        return self.__flag

    @flag.setter
    def flag(self, new_num):
        self.__flag = new_num

    def __connect(self):
        client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        client.setblocking(True)
        client.settimeout(self.__timeout)
        client.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)  # 端口复用
        server_host = (self.__host, self.__port)
        try:
            client.connect(server_host)
        except:
            print 'server is closed'
            return
        return client

    def send_msg(self):
        if not self.client:
            return
        while True:
            time.sleep(0.1)
            if self.signining is True:
                continue
            # data = raw_input()
            if self.online is False:
                ret = raw_input('Press \'i\' to Sign In, \'u\' to Sign Up: ')

                if ret == 'i':
                    # sign in
                    self.signining = True
                    usr_name = raw_input("usr_name:")
                    usr_pwd = raw_input("usr_pwd:")
                    p_c2s = P4C()
                    p_c2s.type = P4CliType.TYPE_SIGN_IN
                    p_c2s.msg = usr_name + ':' + usr_pwd
                    self.usr_name = usr_name
                    self.client.sendall(p_c2s.toJSON())
                elif ret == 'u':
                    # sign up
                    usr_name = raw_input("usr_name:")
                    usr_pwd = raw_input("usr_pwd:")
                    p_c2s = P4C()
                    p_c2s.type = P4CliType.TYPE_REGISTER
                    p_c2s.msg = usr_name + ':' + usr_pwd
                    self.usr_name = usr_name
                    self.client.sendall(p_c2s.toJSON())
            else:
                data = sys.stdin.readline().strip()

                # sys.stdout.write('[{}]:'.format(self.usr_name))
                # sys.stdout.flush()
                self.print_data('', True)

                if "exit" == data.lower():
                    with self.__lock:
                        self.flag = 0
                    break

                if data.startswith(Client.COMMAND_CREATE_ROOM):
                    p_c2s = P4C()
                    p_c2s.type = P4CliType.TYPE_CREATE_ROOM
                    self.client.sendall(p_c2s.toJSON())

                elif data.startswith(Client.COMMAND_ENTER_ROOM):
                    room_id = self.parse_room_id(Client.COMMAND_ENTER_ROOM, data)
                    if room_id is None:
                        self.print_data('command error')
                        continue
                    if isinstance(room_id, int):
                        if room_id >= 0:
                            p_c2s = P4C()
                            p_c2s.type = P4CliType.TYPE_ENTER_ROOM
                            p_c2s.room_id = room_id
                            self.client.sendall(p_c2s.toJSON())
                        else:
                            self.print_data('Error: room_id < 0')
                    else:
                        self.print_data('Error: cannot parse room_id, the command format is incorrect.')

                elif data.startswith(Client.COMMAND_QUIT_ROOM):
                    # room_id = self.parse_room_id(Client.COMMAND_QUIT_ROOM, data)
                    room_id = self.cur_room_id
                    if room_id is None:
                        self.print_data('command error')
                        continue
                    if isinstance(room_id, int):
                        if room_id >= 0:
                            p_c2s = P4C()
                            p_c2s.type = P4CliType.TYPE_QUIT_ROOM
                            p_c2s.room_id = room_id
                            self.client.sendall(p_c2s.toJSON())
                            continue
                        else:
                            self.print_data('Error: room_id < 0')
                    else:
                        self.print_data('Error: cannot parse room_id, the command format is incorrect.')

                elif data.startswith(Client.COMMAND_ENTER_PRIVATE_CHAT):
                    usr_name = self.parse_usr_name(Client.COMMAND_ENTER_PRIVATE_CHAT, data)
                    if usr_name is None:
                        self.print_data('command error')
                        continue
                    p_c2s = P4C()
                    p_c2s.type = P4CliType.TYPE_FIND_USR_INFO_BY_USR_NAME
                    p_c2s.msg = usr_name
                    self.to_usr_name = usr_name
                    self.client.sendall(p_c2s.toJSON())

                elif data.startswith(Client.COMMAND_QUIT_PRIVATE_CHAT):
                    self.cur_state = Client.IN_LOBBY
                    self.to_usr_id = 0
                    self.print_data('You are in lobby now')

                elif data.startswith(Client.COMMAND_SIGN_OUT):
                    p_c2s = P4C()
                    p_c2s.type = P4CliType.TYPE_SIGN_OUT
                    p_c2s.msg = self.usr_name
                    self.client.sendall(p_c2s.toJSON())

                elif data.startswith(Client.COMMAND_21_GAME):
                    formula = data[len(Client.COMMAND_21_GAME): len(data)].strip().replace("++", "+").replace("+-",
                                                                                                              "-").replace(
                        "-+", "-").replace("- -", "+")
                    if self.check_21_game_formula_number(formula) is True:
                        try:
                            result = eval(formula)
                            if result <= 21:
                                p_c2s = P4C()
                                p_c2s.type = P4CliType.TYPE_21_GAME_PLAYER_ANSWER
                                p_c2s.from_id = self.usr_id
                                p_c2s.room_id = self.cur_room_id
                                p_c2s.msg = formula
                                self.client.sendall(p_c2s.toJSON())
                                self.print_data('Your answer has sent to server, wait seconds...')
                            else:
                                self.print_data('result error: result greater than 21')
                        except Exception, e:
                            self.print_data('formula parse error')
                    else:
                        self.print_data('formula error: you can only use the given number')


                else:
                    # data = '[{}]:{}'.format(self.usr_name, data)
                    if self.cur_state == Client.IN_LOBBY:
                        data = '[{}{}]:{}'.format(self.usr_name, '-lobby', data)
                        p_c2s = P4C()
                        p_c2s.type = P4CliType.TYPE_LOBBY_CHAT
                        p_c2s.msg = data
                        self.client.sendall(p_c2s.toJSON())

                    elif self.cur_state == Client.IN_PRIVATE:
                        data = '[{}{}]:{}'.format(self.usr_name, '-private', data)
                        p_c2s = P4C()
                        p_c2s.type = P4CliType.TYPE_PRIVATE_CHAT
                        p_c2s.msg = data
                        p_c2s.from_id = self.usr_id
                        p_c2s.to_id = self.to_usr_id
                        self.client.sendall(p_c2s.toJSON())

                    elif self.cur_state == Client.IN_ROOM:
                        data = '[{}{}]:{}'.format(self.usr_name, '-room-{}'.format(self.cur_room_id), data)
                        p_c2s = P4C()
                        p_c2s.type = P4CliType.TYPE_ROOM_CHAT
                        p_c2s.room_id = self.cur_room_id
                        p_c2s.msg = data
                        self.client.sendall(p_c2s.toJSON())

    def recv_msg(self):
        if not self.client:
            return
        while True:
            data = None
            with self.__lock:
                if not self.flag:
                    print 'ByeBye~~'
                    break
            try:
                data = self.client.recv(self.__buffer_size)
            except socket.timeout:
                continue
            if data:
                try:
                    p_s2c = P4C.toObject(data)
                except:
                    print data
                if p_s2c.type == P4CliType.TYPE_SIGN_IN:
                    self.signining = False
                    if p_s2c.result_id == 0:
                        self.online = True
                        self.usr_id = p_s2c.to_id
                        self.print_data(
                            'Sign in success. Your usr_id is {}, online time is {} seconds '.format(p_s2c.to_id,
                                                                                                    p_s2c.msg))
                    elif p_s2c.result_id == P4SvrRsp.SIGN_IN_USR_NOT_FOUND:
                        self.online = False
                        print 'Sign in failed: usr not found'
                    elif p_s2c.result_id == P4SvrRsp.SIGN_IN_USR_ALREADY_ONLINE:
                        self.online = False
                        print 'Sign in failed: usr is already online'

                elif p_s2c.type == P4CliType.TYPE_REGISTER:
                    if p_s2c.result_id == 0:
                        print 'Sign up success'
                    elif p_s2c.result_id == P4SvrRsp.REGISTER_USR_ALREADY_EXIST:
                        print 'Sign up failed: usr_name already exists'
                    elif p_s2c.result_id == P4SvrRsp.REGISTER_USR_NAME_NOT_VALID:
                        print 'Sign up failed: usr_name is not valid'
                    elif p_s2c.result_id == P4SvrRsp.REGISTER_USR_PWD_NOT_VALID:
                        print 'Sign up failed: usr_pwd is not valid'

                elif p_s2c.type == P4CliType.TYPE_SIGN_OUT:
                    if p_s2c.result_id == 0:
                        self.online = False
                        self.usr_id = 0
                        print 'Sign out success'
                    elif p_s2c.result_id == P4SvrRsp.SIGN_OUT_USR_ALREADY_OFFLINE:
                        print 'Sign out failed: usr is already offline'
                    elif p_s2c.result_id == P4SvrRsp.SIGN_OUT_USR_NOT_FOUND:
                        print 'Sign out failed: usr not found'

                elif p_s2c.type == P4CliType.TYPE_LOBBY_CHAT:
                    self.print_data(p_s2c.msg)

                elif p_s2c.type == P4CliType.TYPE_ROOM_CHAT:
                    self.print_data(p_s2c.msg)

                elif p_s2c.type == P4CliType.TYPE_CREATE_ROOM:
                    if p_s2c.result_id == 0:
                        self.cur_state = Client.IN_ROOM
                        self.cur_room_id = p_s2c.room_id
                        self.print_data(
                            'Create room(room_id:{}) success, and enter room automatically.'.format(p_s2c.room_id))

                elif p_s2c.type == P4CliType.TYPE_ENTER_ROOM:
                    if p_s2c.result_id == 0:
                        self.cur_state = Client.IN_ROOM
                        self.cur_room_id = p_s2c.room_id
                        self.print_data('Enter room(room_id:{}) success'.format(p_s2c.room_id))
                    elif p_s2c.result_id == P4SvrRsp.RSP_ENTER_ROOM_ID_NOT_EXIST:
                        self.print_data(
                            'Failed to enter room.(room_id:{} cannot be founnd.)'.format(p_s2c.room_id, p_s2c.room_id))

                elif p_s2c.type == P4CliType.TYPE_QUIT_ROOM:
                    if p_s2c.result_id == 0:
                        self.cur_state = Client.IN_LOBBY
                        self.cur_room_id = 0
                        self.print_data('Quit room(room_id:{}) success'.format(p_s2c.room_id))

                elif p_s2c.type == P4CliType.TYPE_PRIVATE_CHAT:
                    if p_s2c.result_id == 0:
                        # self.cur_state = Client.IN_PRIVATE
                        self.print_data(p_s2c.msg)
                    elif p_s2c.result_id == P4SvrRsp.RSP_PRIVATE_CHAT_TO_USR_ALREADY_OFFLINE:
                        self.to_usr_id = 0
                        if self.to_usr_name in self.usr_name_2_id and self.usr_name_2_id[
                            self.to_usr_name] == p_s2c.to_id:
                            self.usr_name_2_id.pop(self.to_usr_name)
                        self.cur_state = Client.IN_LOBBY
                        self.print_data("{} already offline, return lobby automatically".format(self.to_usr_name))
                        self.to_usr_name = ''

                elif p_s2c.type == P4CliType.TYPE_FIND_USR_INFO_BY_USR_NAME:
                    if p_s2c.result_id == 0:
                        self.to_usr_id = p_s2c.to_id
                        self.usr_name_2_id[self.to_usr_name] = p_s2c.to_id
                        self.cur_state = Client.IN_PRIVATE
                        self.print_data(
                            'You are talking to {} now, you can input \'{}\' to return lobby'.format(self.to_usr_name,
                                                                                                     Client.COMMAND_QUIT_PRIVATE_CHAT))
                    elif p_s2c.result_id == P4SvrRsp.RSP_FIND_USR_OFFLINE_OR_NOT_REGISTER_AT_ALL:
                        self.print_data('Server cannot find online client called {}'.format(self.to_usr_name))

                elif p_s2c.type == P4CliType.TYPE_21_GAME:
                    if p_s2c.result_id == 0:
                        i = 0
                        for num in p_s2c.msg.strip().split(' '):
                            self.room_21_game_start = True
                            self.game_numbers[i] = int(num)
                            i += 1
                        self.print_data('21 Game Start,the numbers are {} {} {} {}'.format(self.game_numbers[0],
                                                                                           self.game_numbers[1],
                                                                                           self.game_numbers[2],
                                                                                           self.game_numbers[3]))
                elif p_s2c.type == P4CliType.TYPE_21_GAME_PLAYER_ANNOUCE_WINNER:
                    if p_s2c.result_id == 0:
                        if self.cur_room_id == p_s2c.room_id:
                            self.print_data(p_s2c.msg)
                            self.room_21_game_start = False
            time.sleep(0.1)
        return

    def run(self):
        self.client = self.__connect()
        send_proc = threading.Thread(target=self.send_msg)
        recv_proc = threading.Thread(target=self.recv_msg)
        recv_proc.start()
        send_proc.start()
        recv_proc.join()
        send_proc.join()
        self.client.close()

    def print_data(self, data, use_for_after_input=False):
        new_line = ''
        if not use_for_after_input:
            sys.stdout.write('\n')
            sys.stdout.write(data)
            new_line = '\n'

        state_str = ''
        if self.cur_state == Client.IN_LOBBY:
            state_str = '-lobby'
        elif self.cur_state == Client.IN_PRIVATE:
            state_str = '-private'.format(self.to_usr_id)
        elif self.cur_state == Client.IN_ROOM:
            state_str = '-room-{0}'.format(self.cur_room_id)

        sys.stdout.write('{}[{}{}]:'.format(new_line, self.usr_name, state_str))
        sys.stdout.flush()

    def parse_room_id(self, cmd, msg):
        try:
            return eval(msg[len(cmd):len(msg)])
        except:
            return None

    def parse_usr_id(self, cmd, msg):
        try:
            return eval(msg[len(cmd):len(msg)])
        except:
            return None

    def parse_usr_name(self, cmd, msg):
        try:
            return eval(msg[len(cmd):len(msg)])
        except:
            return None

    def check_21_game_formula_number(self, formula):
        numbers = []
        parsing_number = False
        # parse numbers in formula
        for i in range(0, len(formula), 1):
            c = formula[i]

            if c in ('+', '-', '*', '/', '(', ')'):
                parsing_number = False
                continue
            else:
                try:
                    num = int(c)
                except:
                    return False
                if parsing_number:
                    numbers[-1] = numbers[-1] * 10 + num
                else:
                    numbers.append(num)
                parsing_number = True

        if len(numbers) == 4:
            for num in self.game_numbers:
                for i in range(0, 4, 1):
                    if numbers[i] == num:
                        numbers[i] = -1
                        break
            for i in range(0, 4, 1):
                if numbers[i] != -1:
                    return False
            return True

        return False


if "__main__" == __name__:
    Client('localhost').run()
