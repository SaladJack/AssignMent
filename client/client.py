# chat_client.py

import sys, socket, select, string
import base64

import hashlib


from server.protocol import Protocol, CmdProtocol, RspProtocol

BLOCKSIZE = 16
pad = lambda s: s + (BLOCKSIZE - len(s) % BLOCKSIZE) * chr(BLOCKSIZE - len(s) % BLOCKSIZE)
unpad = lambda s: s[:-ord(s[len(s) - 1:])]


def encrypt(key, raw):
    return raw

def decrypt(key, encoding):
    return encoding


def chat_client():
    # host = sys.argv[1]
    host = 'localhost'
    # port = int(sys.argv[2])
    port = 5554

    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.settimeout(2)

    name = raw_input('Enter your name: ')
    key = raw_input('Enter secret key: ')

    # connect to remote host
    try:
        s.connect((host, port))
    except:
        print 'Unable to connect'
        sys.exit()

    print 'Connected to remote host. You can start sending messages'
    sys.stdout.write('[Me] ');
    sys.stdout.flush()

    while 1:
        socket_list = [sys.stdin, s]

        # Get the list sockets which are readable
        read_sockets, write_sockets, error_sockets = select.select(socket_list, [], [])

        # lobby
        for sock in read_sockets:
            if sock == s:
                # incoming message from remote server, s
                data = sock.recv(4096)
                data = decrypt(key, data)
                if not data:
                    print '\nDisconnected from chat server'
                    sys.exit()
                else:
                    rsp = Protocol.toObject(data)
                    msg = ""
                    if rsp.code == RspProtocol.RSP_CREATE_ROOM_SUCCESS:
                        msg = "Create room success,roomID:%d" % rsp.room_id
                    elif rsp.code == RspProtocol.RSP_ENTER_ROOM_SUCCESS:
                        msg = "Enter room success,roomID:%d" % rsp.room_id
                    elif rsp.code == RspProtocol.RSP_QUIT_ROOM_SUCCESS:
                        msg = "Quit room success,roomID:%d" % rsp.room_id
                    else:
                        msg = rsp.msg
                    sys.stdout.write('\n')
                    sys.stdout.write(data)
                    sys.stdout.write('\n[Me] ')
                    sys.stdout.flush()

            else:
                # user entered a message
                msg = sys.stdin.readline()
                p = Protocol()
                # room
                if msg.startswith("/",0,1):
                    if msg == "/cr":
                        p.code = CmdProtocol.CMD_CREATE_ROOM
                    elif msg.startswith("/er", 0, 3):
                        p.code = CmdProtocol.CMD_ENTER_ROOM
                        msg = msg.strip()
                        p.room_id = string.atoi(msg[3:len(msg)])
                    elif msg.startswith("/qr", 0, 3):
                        p.code = CmdProtocol.CMD_QUIT_ROOM
                        msg = msg.strip()
                        p.room_id = string.atoi(msg[3:len(msg)])
                    #print "send %s" % p.toJSON()
                    s.send(p.toJSON())
                else:
                    msg = '[%s] %s' % (name, msg)
                    msg = encrypt(key, msg)
                    # encrypt and send the message
                    p.msg = msg
                    s.send(p.toJSON())
                    sys.stdout.write('[Me] ')
                    sys.stdout.flush()


if __name__ == "__main__":
    sys.exit(chat_client())