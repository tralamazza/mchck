#McLogic
A [SUMP](http://www.sump.org/projects/analyzer/protocol/) compatible logic analyzer for the [mchck](https://mchck.org) platform.

##Features
- 2MHz max sample rate
- 8 probes (PORTD)
- 4096 bytes of memory
- Triggers (*serial only*)

##Building
To build and flash McLogic into your mchck simply type:

    $> make flash

##Clients
Make sure the client respects the following settings:
- "internal" clock mode
- no RLE
- serial trigger (simple mode)
- no filter
- no demux
- 8 probes
- 4096 samples max

McLogic is known to work with the following clients:

###sigrok-cli
Command example:

    $> sigrok-cli --driver=ols:conn=/dev/ttyACM0 --config external_clock=0 --config rle=0 --config samplerate=1M --samples 1024

Tested version: 0.4.0

###Pulseview
Setup:
- Select *File->Connect to Device...*
- Select *Openbench Logic Sniffer (ols)*
- Enter `/dev/ttyACM0` in the Serial Port
- Click *Scan for Devices*, you should see *MCHCK FPGA version ...*
- In the main window click on the tool icon and set the pattern generator mode to *internal*.

Tested version: 0.1.0

###[OLS](http://www.lxtreme.nl/ols/)
Copy *ols.profile-mchck.cfg* to the OLS plugins folder.

##Development
For USB/protocol debugging build a simple host mode demo using McHCK's VUSB:

    $> make VUSB=1
    $> sudo ./mclogic 2> log

NOTE: You might need to *modprobe* a linux usb staging module.

##WIP
- RLE
- DMA transfers (*you can test with `make DMA=1 flash`)
