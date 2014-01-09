#McLogic

A logic analyzer for the [mchck](https://mchck.org) compatible with the [SUMP](http://www.sump.org/projects/analyzer/protocol/) protocol.

##Features

- 1MHz max sample rate
- 8 probes
- 4096 bytes of memory
- Triggers (*serial only*)
- ~~RLE~~ (**WIP**)
- ~~DMA transfers~~ (**WIP, you can test passing DMA=1**)

##Building
There are 3 basic ways of building McLogic.

###Standard
This should work with [OLS](http://www.lxtreme.nl/ols/) (**untested**).

    $> make flash

NOTE: Don't forget ot copy *ols.profile-mchck.cfg* to the OLS plugins folder.

###sigrok-cli/Pulseview
We will use the "ols" driver in [sigrok](http://sigrok.com).
Tested with sigrok-cli 0.4.0 and pulseview 0.1.0.

    $> make SIGROK=1 flash

A sigrok-cli example:

    $> sigrok-cli --driver=ols:conn=/dev/ttyACM0 --config external_clock=0 --config rle=0 --config samplerate=1M --samples 1024

Pulseview setup:

- Select *File->Connect to Device...*
- Select *Openbench Logic Sniffer (ols)*
- Enter `/dev/ttyACM0` in the Serial Port
- Click *Scan for Devices*, you should see *MCHCK FPGA version ...*
- In the main window click on the tool icon and set the pattern generator mode to *internal*.

###VUSB (for local testing)
You can build a simple host mode demo using McHCK's VUSB (which doesn't require a mchck device):

    $> make VUSB=1
    $> sudo ./mclogic 2> log

NOTE: You might need to *modprobe* a linux usb module.


##Settings
Make sure your UI is set to use:

- "internal" clock mode
- no RLE
- serial trigger
- no filter
- no demux
- 8 probes
- 1024 samples
