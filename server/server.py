#!/bin/env python
# -*- coding:utf8 -*-

"""
server select
"""

import sys
import time
import socket
import select
import logging
import Queue

from dbMgr import DBMgr
from p4s import P4S, P4SvrType, P4SvrRsp

g_select_timeout = 10


class Server(object):
    CLIENT_INFO_ADDR = 0
    CLIENT_INFO_ONLINE = 1
    CLIENT_INFO_USR_ID = 2
    CLIENT_INFO_USR_NAME = 3

    def __init__(self, host='localhost', port=33333, timeout=2, client_nums=10):
        self.__host = host
        self.__port = port
        self.__timeout = timeout
        self.__client_nums = client_nums
        self.__buffer_size = 1024

        self.server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.server.setblocking(False)
        self.server.settimeout(self.__timeout)
        self.server.setsockopt(socket.SOL_SOCKET, socket.SO_KEEPALIVE, 1)  # keepalive
        self.server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)  # 端口复用
        server_host = (self.__host, self.__port)
        try:
            self.server.bind(server_host)
            self.server.listen(self.__client_nums)
        except:
            raise

        self.inputs = [self.server]  # select 接收文件描述符列表
        self.outputs = []  # 输出文件描述符列表
        self.message_queues = {}  # 消息队列
        self.client_info = {}
        self.usr_id_2_connection = {}
        self.usr_name_2_connection = {}
        self.client_cur_id = 1
        self.room_list = {}  # 房间列表
        self.room_cur_id = 1

    def run(self):
        while True:
            readable, writable, exceptional = select.select(self.inputs, self.outputs, self.inputs, g_select_timeout)
            if not (readable or writable or exceptional):
                continue

            for s in readable:
                if s is self.server:  # 是客户端连接
                    connection, client_address = s.accept()
                    # print "connection", connection
                    print "%s connect." % str(client_address)
                    connection.setblocking(0)  # 非阻塞
                    self.inputs.append(connection)  # 客户端添加到inputs
                    self.client_info[connection] = [str(client_address), False, 0, '']  # [地址,是否已登录,用户id,用户名]
                    self.message_queues[connection] = Queue.Queue()  # 每个客户端一个消息队列

                else:  # 是client, 数据发送过来
                    try:
                        data = s.recv(self.__buffer_size)
                        if data:
                            # print data
                            # data = "%s %s say: %s" % (time.strftime("%Y-%m-%d %H:%M:%S"), self.client_info[s], data)
                            self.message_queues[s].put(data)  # 队列添加消息

                            if s not in self.outputs:  # 要回复消息
                                self.outputs.append(s)
                        else:  # 客户端断开
                            # Interpret empty result as closed connection
                            print "Client:%s Close." % str(self.client_info[s])
                            if s in self.outputs:
                                self.outputs.remove(s)
                            self.inputs.remove(s)
                            s.close()
                            del self.message_queues[s]
                            del self.client_info[s]
                    except Exception, e:
                        self.handle_exception(s, s, e)

            for s in writable:  # outputs 有消息就要发出去了
                try:
                    next_msg = self.message_queues[s].get_nowait()  # 非阻塞获取
                except Queue.Empty:
                    err_msg = "Output Queue == Empty!"
                    # g_logFd.writeFormatMsg(g_logFd.LEVEL_INFO, err_msg)
                    self.outputs.remove(s)
                except Exception, e:  # 发送的时候客户端关闭了则会出现writable和readable同时有数据，会出现message_queues的keyerror
                    err_msg = "Send Data Error! ErrMsg:%s" % str(e)
                    logging.error(err_msg)
                    if s in self.outputs:
                        self.outputs.remove(s)
                else:
                    p_c2s = P4S.toObject(next_msg)

                    if p_c2s.type == P4SvrType.TYPE_LOBBY_CHAT:
                        self.send_msg_to_lobby(s, p_c2s.toJSON())

                    elif p_c2s.type == P4SvrType.TYPE_PRIVATE_CHAT:
                        if p_c2s.to_id in self.usr_id_2_connection:
                            p_s2c = p_c2s
                            receiver = self.usr_id_2_connection[p_c2s.to_id]
                            self.send_msg_to_usr(s, receiver, p_s2c.toJSON())
                        else:
                            p_c2s.result_id = P4SvrRsp.RSP_PRIVATE_CHAT_TO_USR_ALREADY_OFFLINE
                            p_s2c = p_c2s
                            self.send_msg_to_usr(s, s, p_s2c.toJSON())

                    elif p_c2s.type == P4SvrType.TYPE_FIND_USR_INFO_BY_USR_NAME:
                        p_s2c = self.handle_find_usr_info(p_c2s.msg)
                        self.send_msg_to_usr(s, s, p_s2c.toJSON())

                    elif p_c2s.type == P4SvrType.TYPE_ROOM_CHAT:
                        p_s2c = p_c2s
                        self.send_msg_to_room(s, p_s2c.room_id, p_s2c.toJSON())

                    elif p_c2s.type == P4SvrType.TYPE_CREATE_ROOM:
                        p_s2c = self.handle_create_room(s)
                        self.send_msg_to_usr(s, s, p_s2c.toJSON())

                    elif p_c2s.type == P4SvrType.TYPE_QUIT_ROOM:
                        p_s2c = self.handle_quit_room(s, p_c2s.room_id)
                        self.send_msg_to_usr(s, s, p_s2c.toJSON())

                    elif p_c2s.type == P4SvrType.TYPE_ENTER_ROOM:
                        p_s2c = self.handle_enter_room(s, p_c2s.room_id)
                        self.send_msg_to_usr(s, s, p_s2c.toJSON())

                    elif p_c2s.type == P4SvrType.TYPE_SIGN_IN:
                        print 'rcv sign in pkg'
                        usr_name, usr_pwd = self.parse_usr_name_and_pwd(p_c2s.msg)
                        p_s2c = self.handle_sign_in(s, usr_name, usr_pwd)
                        print 'sign in resultId:%s' % p_s2c.result_id
                        self.send_msg_to_usr(s, s, p_s2c.toJSON())

                    elif p_c2s.type == P4SvrType.TYPE_SIGN_OUT:
                        usr_name = self.parse_usr_name(p_c2s.msg)
                        p_s2c = self.handle_sign_out(s, usr_name)
                        self.send_msg_to_usr(s, s, p_s2c.toJSON())

                    elif p_c2s.type == P4SvrType.TYPE_REGISTER:
                        usr_name, usr_pwd = self.parse_usr_name_and_pwd(p_c2s.msg)
                        p_s2c = self.handle_register(usr_name, usr_pwd)
                        self.send_msg_to_usr(s, s, p_s2c.toJSON())

            for s in exceptional:
                # logging.error("Client:%s Close Error." % str(self.client_info[cli]))
                if s in self.inputs:
                    self.inputs.remove(s)
                    s.close()
                if s in self.outputs:
                    self.outputs.remove(s)
                if s in self.message_queues:
                    del self.message_queues[s]
                del self.client_info[s]

                for room_id, room_connection_set in self.room_list.items():
                    if room_connection_set.__contains__(s):
                        room_connection_set.remove(s)

    def parse_usr_name(self, msg):
        return msg

    def parse_usr_name_and_pwd(self, msg):
        # format: xxx:yyy
        fore = msg.find(':', 0, len(msg))
        return msg[0:fore], msg[fore + 1:len(msg)]

    def handle_sign_in(self, connection, usr_name, usr_pwd):
        p_s2c = P4S()
        p_s2c.type = P4SvrType.TYPE_SIGN_IN

        p_s2c.result_id, usr_id = DBMgr().sign_in(usr_name, usr_pwd)
        if p_s2c.result_id == 0:
            p_s2c.to_id = usr_id
            self.client_info[connection][Server.CLIENT_INFO_ONLINE] = True
            self.client_info[connection][Server.CLIENT_INFO_USR_ID] = usr_id
            self.client_info[connection][Server.CLIENT_INFO_USR_NAME] = usr_name
            self.usr_id_2_connection[usr_id] = connection
            self.usr_name_2_connection[usr_name] = connection

        return p_s2c

    def handle_sign_out(self, connection, usr_name):
        p_s2c = P4S()
        p_s2c.type = P4SvrType.TYPE_SIGN_OUT
        p_s2c.result_id = DBMgr().sign_out(usr_name)
        if p_s2c.result_id == 0:
            self.client_info[connection][Server.CLIENT_INFO_ONLINE] = False
        return p_s2c

    def handle_register(self, usr_name, usr_pwd):
        p_s2c = P4S()
        p_s2c.type = P4SvrType.TYPE_REGISTER
        p_s2c.result_id = DBMgr().register(usr_name, usr_pwd)
        return p_s2c

    def handle_find_usr_info(self, usr_name):
        p_s2c = P4S()
        p_s2c.type = P4SvrType.TYPE_FIND_USR_INFO_BY_USR_NAME
        if usr_name in self.usr_name_2_connection:
            p_s2c.result_id = 0
            p_s2c.to_id = self.client_info[self.usr_name_2_connection[usr_name]][Server.CLIENT_INFO_USR_ID]
        else:
            p_s2c.result_id = P4SvrRsp.RSP_FIND_USR_OFFLINE_OR_NOT_REGISTER_AT_ALL
        return p_s2c

    def handle_create_room(self, connection):
        self.room_list[self.room_cur_id] = {connection}
        p_s2c = P4S()
        p_s2c.result_id = 0
        p_s2c.type = P4SvrType.TYPE_CREATE_ROOM
        p_s2c.room_id = self.room_cur_id
        self.room_cur_id += 1
        return p_s2c

    def handle_enter_room(self, connection, room_id):
        p_s2c = P4S()
        p_s2c.type = P4SvrType.TYPE_ENTER_ROOM
        p_s2c.room_id = room_id
        if room_id not in self.room_list:
            p_s2c.result_id = P4SvrRsp.RSP_ENTER_ROOM_ID_NOT_EXIST
            return p_s2c

        self.room_list[room_id].add(connection)
        p_s2c.result_id = 0

        return p_s2c

    def handle_quit_room(self, connection, room_id):
        self.room_list[room_id].remove(connection)
        p_s2c = P4S()
        p_s2c.result_id = 0
        p_s2c.type = P4SvrType.TYPE_QUIT_ROOM
        p_s2c.room_id = room_id
        return p_s2c

    def send_msg_to_lobby(self, sender, msg):
        for receiver in self.client_info:  # 发送给其他客户端
            if receiver is not sender:
                try:
                    receiver.sendall(msg)
                except Exception, e:  # 发送失败就关掉
                    self.handle_exception(sender, receiver, e)

    def send_msg_to_usr(self, sender, receiver, msg):
        try:
            receiver.sendall(msg)
        except Exception, e:
            self.handle_exception(sender, receiver, e)

    def send_msg_to_room(self, sender, room_id, msg):
        room_connect_set = self.room_list[room_id]
        for receiver in room_connect_set:
            if receiver is not sender:
                try:
                    receiver.sendall(msg)
                except Exception, e:
                    self.handle_exception(sender, receiver, e)

    def handle_exception(self, sender, receiver, e):
        # err_msg = "Send Data to %s  Error! ErrMsg:%s" % (str(self.client_info[receiver][0]), str(e))
        # logging.error(err_msg)
        print "Client: %s Close Error." % str(self.client_info[receiver][0])
        usr_id = self.client_info[receiver][Server.CLIENT_INFO_USR_ID]
        usr_name = self.client_info[receiver][Server.CLIENT_INFO_USR_NAME]

        if usr_id in self.usr_id_2_connection:
            self.usr_id_2_connection.pop(usr_id)

        if usr_name in self.usr_name_2_connection:
            self.usr_name_2_connection.pop(usr_name)

        if receiver in self.inputs:
            self.inputs.remove(receiver)
            receiver.close()

        if receiver in self.outputs:
            self.outputs.remove(sender)

        if receiver in self.message_queues:
            del self.message_queues[sender]

        del self.client_info[receiver]

        for room_id, room_connection_set in self.room_list.items():
            if receiver in room_connection_set:
                room_connection_set.remove(receiver)




if "__main__" == __name__:
    Server().run()
