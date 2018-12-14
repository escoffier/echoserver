
import sys
import socket

MSGLEN = 1024
class MySocket:
    """demonstration class only
      - coded for clarity, not efficiency
    """

    def __init__(self, sock=None):
        if sock is None:
            self.sock = socket.socket(
                socket.AF_INET, socket.SOCK_STREAM)
        else:
            self.sock = sock
        
        #self.MSGLEN = 1024

    def connect(self, host, port):
        self.sock.connect((host, port))

    def mysend(self, msg):
        totalsent = 0
        while totalsent < MSGLEN:
            sent = self.sock.send(msg[totalsent:])
            if sent == 0:
                raise RuntimeError("socket connection broken")
            totalsent = totalsent + sent

    def myreceive(self):
        chunks = []
        bytes_recd = 0
        while bytes_recd < MSGLEN:
            chunk = self.sock.recv(min(MSGLEN - bytes_recd, 2048))
            if chunk == b'':
                raise RuntimeError('socket connection broken')
            chunks.append(chunk)
            bytes_recd = bytes_recd + len(chunk)
        return b''.join(chunks)

def main():
    mysock = MySocket()
    mysock.connect('192.168.21.197', 9995)
    c = None
    while c != 'x':
        sys.stdout.write("==> ")
        sys.stdout.flush()
        c = sys.stdin.readline().strip()
        if c == 't':
            mysock.mysend(b'my name is robbie')
            mysock.myreceive()
        elif c == 'x':
            return

    # my code here


if __name__ == "__main__":
    main()
