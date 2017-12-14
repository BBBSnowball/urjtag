# Usage: PYTHONPATH=/home/user/urjtag-install/lib64/python2.7/site-packages/ LD_LIBRARY_PATH=/home/user/urjtag-install/lib/ python test_shift_raw.py
import urjtag
c=urjtag.chain()
c.cable("UsbBlaster")
print "frequency: %d" % c.get_frequency()
print "max todo length: %d" % c.get_max_len_for_shift_raw()
print "state: %s" % c.get_state()

#urjtag.loglevel(0)

print c.tap_detect()
print c.set_instruction("BYPASS")
print c.shift_ir()

def shift_raw(c, tms, tdi):
    print "TMS: %s" % tms
    print "TDI: %s" % tdi
    tdo = c.shift_raw(tms, tdi)
    print "TDO: %s" % tdo
    print "state: %s" % c.get_state()
    return tdo

def shift_raw_ir(c, ir):
    shift_raw(c, "1100" + (len(ir)-1)*"0" + "110", "1111" + ir[::-1] + "11")
def shift_raw_dr(c, dr):
    if type(dr)==int:
        dr = "0"*dr
    return shift_raw(c, "100" + (len(dr)-1)*"0" + "11", "111" + dr[::-1] + "1")[3:-1][::-1]

def to_hex(bits):
    bits = bits+"."
    s = ""
    print "len: (%d+3)/4 = %r" % (len(bits), (len(bits)+3)/4)
    for i in range((len(bits)-1+3)/4):
        x = bits[-4*i-5:-4*i-1]
        print "i=%d, x=%r" % (i, x)
        s = "%x" % int(x,2) + s
    return s

shift_raw(c, "1111111", "1111111")

tms = ""
tdi = ""

# shift at least five TMS=1 to go to RESET
tms += "1111111"
tdi += "1111111"

# go to SHIFT_IR
tms += "01100"
tdi += "11111"

# shift IR
tms += "0"*201
tdi += "1"*100 + "0" + "1"*100

# go to PAUSE_IR
tms += "10"
tdi += "11"

shift_raw(c, tms, tdi)

shift_raw(c, "11111110", "11111111")
c.set_instruction("IDCODE")
c.shift_ir()
c.shift_dr()
print "IDCODE by shift_ir+shift_dr: %s" % c.get_dr_out_string()

shift_raw(c, "11111110", "11111111")
c.set_instruction("IDCODE")
c.shift_ir()
print c.get_state()
#print "shift_ir: data for IDCODE: %s" % c.get_ir_in_string()
tdo = shift_raw(c, "100" + 32*"0" + "110", "1"*38)
print "IDCODE by shift_ir+shift_raw: %s" % tdo[3:35][::-1]

shift_raw(c, "1100" + 9*"0" + "110", "1111" + "0000000110"[::-1] + "11")
c.shift_dr()
print "IDCODE by shift_raw+shift_dr: %s" % c.get_dr_out_string()

shift_raw(c, "1100" + 9*"0" + "110", "1111" + "0000000110"[::-1] + "11")
tdo = shift_raw(c, "100" + 32*"0" + "110", "1"*38)
print "IDCODE by shift_raw: %s" % tdo[3:35][::-1]

shift_raw_ir(c, "0000000110")
print "IDCODE by shift_raw_ir+shift_raw_dr: %s" % to_hex(shift_raw_dr(c, 32))

#shift_raw(c, "1100" + 10*"0" + "110", "1111" + 10*"1" + "111")
#tdo = shift_raw(c, "100" + 64*"0" + "110", "1"*60 + "0" + 9*"1")
#print "BYPASS (with shift_raw): %s" % tdo
#
#shift_raw(c, "1100" + 10*"0" + "110", "1111" + "0000000110" + "111")
#tdo = shift_raw(c, "100" + 64*"0" + "110", "1"*60 + "0" + 9*"1")
#print "IDCODE? (with shift_raw): %s" % tdo
#
#shift_raw(c, "1100" + 10*"0" + "110", "1111" + "0000000110"[::-1] + "111")
#tdo = shift_raw(c, "100" + 64*"0" + "110", "1"*60 + "0" + 9*"1")
#print "IDCODE? (with shift_raw): %s" % tdo
#
#shift_raw(c, "1100" + 10*"0" + "110", "1111" + "0000000101"[::-1] + "111")
#tdo = shift_raw(c, "100" + 64*"0" + "110", "1"*60 + "0" + 9*"1")
#print "IDCODE? (with shift_raw): %s" % tdo
#
#shift_raw(c, "1100" + 10*"0" + "110", "1111" + "0000000101" + "111")
#tdo = shift_raw(c, "100" + 64*"0" + "110", "1"*60 + "0" + 9*"1")
#print "IDCODE? (with shift_raw): %s" % tdo
#
