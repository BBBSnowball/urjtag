# Usage: PYTHONPATH=/home/user/urjtag-install/lib64/python2.7/site-packages/ LD_LIBRARY_PATH=/home/user/urjtag-install/lib/ python xvcd.py
# impact: xilinx_xvc host=127.0.0.1:9999 maxpacketsize=1024 disableversioncheck=true
# SDK:    -cable type xilinx_plugin modulename xilinx_xvc modulearg host=127.0.0.1:9999 modulearg maxpacketsize=1024 modulearg disableversioncheck=true
# xmd:    xrjtagchain -cable type xilinx_plugin modulename xilinx_xvc modulearg host=127.0.0.1:9999 modulearg maxpacketsize=1024 modulearg disableversioncheck=true
import urjtag
c=urjtag.chain()
c.cable("UsbBlaster")
print "frequency: %d" % c.get_frequency()

#urjtag.loglevel(0)

def shift_raw(c, tms, tdi):
    print "TMS: %s" % tms
    print "TDI: %s" % tdi
    tdo = c.shift_raw(tms, tdi)
    print "TDO: %s" % tdo
    print "state: %s" % c.get_state()
    return tdo

import SocketServer
import struct

class MyTCPHandler(SocketServer.BaseRequestHandler):
    def handle(self):
        print "handle %s" % self.client_address[0]
        while True:
            cmd = self.request.recv(2)
            while cmd=="" or cmd[-1] != ":" and len(cmd) < 20:
                x = self.request.recv(1)
                if x == "":
                    print "connection closed"
                    return
                cmd += x
            print "cmd: %r" % cmd
            
            if cmd == "getinfo:":
                self.request.sendall("xvcServer_v1.0:2048\n")
            elif cmd == "settck:":
                requested_tck = struct.unpack("<I", self.request.recv(4))[0]
                c.set_frequency(requested_tck)
                self.request.sendall(struct.pack("<I", c.get_frequency()))
            elif cmd == "shift:":
                numbits = struct.unpack("<I", self.request.recv(4))[0]
                numbytes = (numbits+7)/8
                tmsbin = self.request.recv(numbytes)
                tdibin = self.request.recv(numbytes)
                tms = ""
                for x in struct.unpack(str(numbytes)+"B", tmsbin):
                    for bit in range(8):
                        if (x&1) != 0:
                            tms += "1"
                        else:
                            tms += "0"
                        x = x/2
                tms = tms[0:numbits]
                tdi = ""
                for x in struct.unpack(str(numbytes)+"B", tdibin):
                    for bit in range(8):
                        if (x&1) != 0:
                            tdi += "1"
                        else:
                            tdi += "0"
                        x = x/2
                tdi = tdi[0:numbits]
                print "TMS: %r (%d bits)" % (tms, numbits)
                print "TDI: %r" % tdi
                tdo = c.shift_raw(tms, tdi)
                print "TDO: %r" % tdo
                print "st:  %s" % c.get_state()
                tdobin = [None]*numbytes
                for i in range(numbytes):
                    tdobin[i] = int(tdo[i*8:i*8+8][::-1], 2)
                self.request.sendall(struct.pack(str(numbytes)+"B", *tdobin))
            else:
                print "unsupported command %r" % cmd
                break

if __name__ == "__main__":
    HOST, PORT = "localhost", 9999
    server = SocketServer.TCPServer((HOST, PORT), MyTCPHandler)
    server.serve_forever()

