urjtag
======

urjtag from upstream on sourceforge http://sourceforge.net/projects/urjtag/

This repository is a fork of shuckc/urjtag because it shares the history of the SVN import. However, the additions to the master branch are unrelated.

This fork adds a naive implementation of the xvc protocol ([Xilinx Virtual Cable][xvc]). I got that idea from [this hackaday post][hackaday].
I have only tested it with impact4 plus an USBBlaster plus an Altera MAX II, which shows up as an unknown device - I hope this means that it is working fine but that is hard to tell without further testing.

This project may be utterly useless due to a bug in ISE (see [here][isebug]). I'm thinking about emulating a chain with one device for ISE but I haven't done any work towards that. My plan is to use this with a Virtex4 device which is part of a chain of three devices.

[xvc]: https://github.com/Xilinx/XilinxVirtualCable
[hackaday]: https://hackaday.io/project/4262-cheap-xilinx-virtual-cable-alternative
[isebug]: http://forums.xilinx.com/t5/Design-Tools-Others/iMPACT-XVC-broken-with-multiple-devices/td-p/496232
