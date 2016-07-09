import socket
import select
import time

class SmartSprinkler:
    def __init__(self, ip):
        self.s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        try:
            self.s.connect((ip, 80))
            self.s.setblocking(False)
            self.connected = True
        except:
            self.connected = False

    def close(self):
        self.s.close()

    def setOn(self):
        values = []
        if self.connected:
            self.s.send(b'DGN:SETON')
            ready = select.select([self.s], [], [], 10)
            reply = str(self.s.recv(1024))
            for section in reply.split('=')[1:]:
                values.append( int(section.split(' ')[0]))
        else:
            values = [0, 0, 0, 0]

        return values

    def setOff(self):
        values = []
        if self.connected:
            self.s.send(b'DGN:SETOFF')
            ready = select.select([self.s], [], [], 10)
            reply = str(self.s.recv(1024))
            for section in reply.split('=')[1:]:
                values.append( int(section.split(' ')[0]))
        else:
            values = [0, 0, 0, 0]

        return values

    def query(self):
        values = []
        if self.connected:
            self.s.send(b'DGN:')
            redeable, writable, exceptional = select.select([self.s], [], [], 10)
            print 'redeable =', redeable
            reply = str(self.s.recv(1024))
            for section in reply.split('=')[1:]:
                values.append( int(section.split(' ')[0]))
        else:
            values = [0, 0, 0, 0]

        return values

if __name__=="__main__":
    con = SmartSprinkler("192.168.0.201")
    print(con.setOn())
    time.sleep(2)
    con.setOff()
    con.close()

