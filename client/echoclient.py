# Echo client program
import socket
import sys

HOST = '192.168.21.197'    # The remote host
PORT = 9995              # The same port as used by the server
with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((HOST, PORT))

    c = None
    while c != 'x':
        sys.stdout.write("==> ")
        sys.stdout.flush()
        c = sys.stdin.readline().strip()
        if c == 't':
            s.sendall(b'Hello, world')
            data = s.recv(1024)
            print('Received', repr(data))
        elif c == 'x':
            #s.close()
            break;    
