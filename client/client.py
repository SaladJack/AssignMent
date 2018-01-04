#!/usr/local/bin/python
# *-* coding:utf-8 -*-

"""
client.py
"""

import sys
import time
import socket
import threading

from p4c import P4C,P4CliType,P4SvrRsp


class Client(object):
    COMMAND_CREATE_ROOM = 'create room'
    COMMAND_ENTER_ROOM = 'enter room'
    COMMAND_QUIT_ROOM = 'quit room'


    def __init__(self, host, port=33333, timeout=1, reconnect=2):
        self.__host = host
        self.__port = port
        self.__timeout = timeout
        self.__buffer_size = 1024
        self.__flag = 1
        self.client = None
        self.__lock = threading.Lock()
        self.__online = False
        self.signining = False # 正在登录
        self.cur_state = 0 # 0:lobby, 1:privat 2:room
        self.usr_name = ''
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
            raise
        return client

    def send_msg(self):
        if not self.client:
            return
        while True:
            time.sleep(0.1)
            if self.signining is True:
                continue
            # data = raw_input()
            if self.__online is False:
                # sign in
                self.signining = True
                usr_name = raw_input("usr_name:")
                usr_pwd = raw_input("usr_pwd:")
                p_c2s = P4C()
                p_c2s.type = P4CliType.TYPE_SIGN_IN
                p_c2s.msg = usr_name + ':' + usr_pwd
                self.usr_name = usr_name
                self.client.sendall(p_c2s.toJSON())
            else:
                data = sys.stdin.readline().strip()
                #data = raw_input('[{}]:'.format(self.usr_name))
                data = '[{}]:{}'.format(self.usr_name, data)
                sys.stdout.write('[{}]:'.format(self.usr_name))
                sys.stdout.flush()

                if "exit" == data.lower():
                    with self.__lock:
                        self.flag = 0
                    break

                p_c2s = P4C()
                p_c2s.type = P4CliType.TYPE_LOBBY_CHAT
                p_c2s.msg = data
                self.client.sendall(p_c2s.toJSON())
        return

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
            except:
                raise
            if data:
                p_s2c = P4C.toObject(data)
                if p_s2c.type is P4CliType.TYPE_SIGN_IN:
                    if p_s2c.result_id is 0:
                        self.__online = True
                        self.signining = False
                        self.print_data('Sign In Success')

                elif p_s2c.type is P4CliType.TYPE_LOBBY_CHAT:
                    self.print_data(p_s2c.msg)

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

    def print_data(self,data):
        sys.stdout.write('\n')
        sys.stdout.write(data)
        sys.stdout.write('\n[{}]:'.format(self.usr_name))
        sys.stdout.flush()

if "__main__" == __name__:
    Client('localhost').run()