#McLogic

A logic analyzer for the [mchck](mchck.org).

##Features

- 1MHz max sample rate
- 8 probes
- 4096 bytes of memory
- ~~Triggers~~ (**WIP**)
- ~~RLE~~ (**WIP**)


##Building

Unfortunately you have to target the mclogic to your client UI.


###Standard
This should work with OLS (**untested**).

    $> make flash

NOTE: Don't forget ot copy *old.profile-mchck.cfg* to the plugins folder.

###sigrok-cli/Pulseview
Tested with sigrok-cli 0.4.0 and pulseview 0.1.0.

    $> make SIGROK=1 flash

sigrok-cli example:

    $> sigrok-cli --driver=ols:conn=/dev/ttyACM0 --config external_clock=0 --config rle=0 -samples 1024

###VUSB (for local testing)
You can build a simple host mode demo using McHCK's VUSB (which doesn't require a mchck device):

    $> make VUSB=1
    $> sudo ./mclogic 2> log

NOTE: You might need to *modprobe* a linux usb module.


##Settings
Make sure your UI is set to use:

- "internal" clock
- no RLE
- no triggers
- no filter
- no demux
- 8 probes
- 1024 samples
