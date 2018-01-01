# chat_server.py
import json
import sys, socket, select

from protocol import Protocol, CmdProtocol, RspProtocol

HOST = ''
SOCKET_LIST = []
ROOM_DEFAULT_ID = 0
ROOM_LIST = [[]]
RECV_BUFFER = 4096
PORT = 5554


def chat_server():
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.setblocking(False)
    server_socket.settimeout(3)
    server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_KEEPALIVE, 1)
    server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server_socket.bind((HOST, PORT))
    server_socket.listen(10)

    # add server socket object to the list of readable connections
    SOCKET_LIST.append(server_socket)

    print "Chat server started on port " + str(PORT)

    while 1:

        # get the list sockets which are ready to be read through select
        # 4th arg, time_out  = 0 : poll and never block
        ready_to_read, ready_to_write, in_error = select.select(SOCKET_LIST, [], [], 0)

        for sock in ready_to_read:
            # a new connection request recieved
            if sock == server_socket:
                sockfd, addr = server_socket.accept()
                SOCKET_LIST.append(sockfd)
                print "Client (%s, %s) connected" % addr

            # a message from a client, not a new connection
            else:
                # process data recieved from client,
                try:
                    # receiving data from the socket.
                    print "socket receive"
                    data = sock.recv(RECV_BUFFER)
                    print data,type(data)
                    if data:
                        data_dict = json.loads(data)
                        p = Protocol(data_dict)
                        print data_dict["code"]
                        #p = Protocol.toObject(data)
                        print type(p)
                        if p.code == CmdProtocol.CMD_CREATE_ROOM:
                            room_id = create_room()
                            enter_room(sock, room_id)

                            rsp = Protocol(0, 0, room_id, "", RspProtocol.RSP_CREATE_ROOM_SUCCESS)
                            sock.send(rsp.toJSON())
                        elif p.code == CmdProtocol.CMD_ENTER_ROOM:
                            enter_room(sock,p.room_id)
                            send_msg_in_room(server_socket, sock, p.room_id ,"Client (%s, %s) enter room" % addr)

                            rsp = Protocol(0, 0, room_id, "", RspProtocol.RSP_ENTER_ROOM_SUCCESS)
                            sock.send(rsp.toJSON())
                        elif p.code == CmdProtocol.CMD_QUIT_ROOM:
                            quit_room(sock, CmdProtocol.CMD_QUIT_ROOM)
                            send_msg_in_room(server_socket, sock, p.room_id,"Client (%s, %s) quit room" % addr)
                        else:
                            # client data incoming, broadcast to all clients
                            send_msg_in_lobby(server_socket, sock, p.msg)
                    else:
                        # remove the socket that's broken
                        print "data == null"
                        if sock in SOCKET_LIST:
                            SOCKET_LIST.remove(sock)
                            # at this stage, no data means probably the connection has been broken
                            #send_msg_in_lobby(server_socket, sock, "Client eee(%s, %s) is offline\n" % addr)

                # exception
                except:
                    print "Client (%s, %s) is offline\n" % addr
                    send_msg_in_lobby(server_socket, sock, "Client (%s, %s) is offline\n" % addr)
                    continue

    server_socket.close()

# create room and return room_id
def create_room():
    for i in range(len(ROOM_LIST)):
        if len(ROOM_LIST[i]) == 0:
            return i
    new_room = []
    ROOM_LIST.append(new_room)
    return len(ROOM_LIST) - 1


def enter_room(socket,room_id):
    ROOM_LIST[room_id].append(socket)
    return 0

def quit_room(socket,room_id):
    ROOM_LIST[room_id].remove(socket)
    return 0




# broadcast chat messages to all connected clients except sock
def send_msg_in_lobby(server_socket, to_socket, message):
    for socket in SOCKET_LIST:
        # send the message only to peer
        if socket != server_socket and socket != to_socket:
            try:
                p = Protocol(msg=message)
                socket.send(p)
            except:
                # broken socket connection
                socket.close()
                # broken socket, remove it
                if socket in SOCKET_LIST:
                    SOCKET_LIST.remove(socket)

def send_msg_in_room(server_socket,sender,room_id,message):
    for socket in ROOM_LIST[room_id]:
        # send the message only to peer
        if socket != server_socket and socket != sender:
            try:
                p = Protocol()
                p.room_id = room_id
                p.code = 0
                p.msg = message
                socket.send(message)
            except:
                # broken socket connection
                socket.close()
                # broken socket, remove it
                if socket in SOCKET_LIST:
                    ROOM_LIST[room_id].remove(socket)
                    SOCKET_LIST.remove(socket)



if __name__ == "__main__":
    sys.exit(chat_server())