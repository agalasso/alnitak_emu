Alnitak Flat-Man emulation for Spike-a Flat Panel USB Dimmer
------------------------------------------------------------

This package provides Alnitak emulation for Spike-a flat panels. If
your imaging application is designed to communicate with Alnitak flat
panels, you can use this emulator to control your Spike-a flat panel
using the Spike-a USB dimmer.


Installation
------------

Imaging applications communicate with Alnitak panels in one of two
ways: either by using AACmd.exe or by communicating over a serial
(COM) port. How you setup the emulator depends on which method your
imaging app uses.  If your imaging app is not listed below, please
contact me and I'll work with you to add instructions for your imaging
app.


Installation for apps that use AACmd.exe
----------------------------------------

Follow these steps for ACP.

1. Un-zip the alnitak_emu.zip file to any location you like. Make note
   of the location as you will need to specify the location in an ACP
   config file in the next step.  For this example, let's say you put
   the files in C:\alnitak_emu

2. Update the following lines in the autoflat config file:

LightCtrlProgram      C:\alnitak_emu\AACmd.exe
LightOnCommand        x L B#BRT# S      ; Command string to turn light on and set/change brightness
LightOffCommand       x D S             ; Command string to turn light off
LightOnDelay          5                 ; Time needed (sec) for brightness to stabilize

Be sure to update the LightCtrlProgram path to specify the actual
location you chose where the emulator files are located.

CCDAP also uses AACmd.exe but I do not have any information on how to
configure CCDAP, though I believe it should be sufficient to just
specify the path to the emulator's AACmd.exe.


Installation for apps that use the serial port
----------------------------------------------

Follow these steps for SGP or other apps that have support for
communicating with an Alnitak flat panel using the Alnitak serial
command protocol.

For these apps, the emulator works by listening for Alnitak commands
on a virtual COM port, then sending the corresponding translated
command to the Spike-a USB dimmer.

1. Un-zip the alnitak_emu.zip file to any location you like.

2. Install com0com version 2.2.2.0.  This allows you to create a
virtual com port for apps to connect to.  The latest version of
com0com (3.0) does not provide signed drivers, and 2.2.2.0 works fine.

Create a virtual port pair and make a note of the 2 ports. The
emulator will connect to one port and your imaging app will connect to
the other port.

3. Start the emulator, alnitak_emu.exe, with the COM port on the
command-line.  For example, if the files are in C:\alnitak_emu\ and
your virtual COM ports are COM22 and COM23, you could make a shortcut
with the Target:

   C:\alnitak_emu\alnitak_emu.exe COM23

You will need ensure the emulator is running before you attempt to
control the panel in SGP. You can setup a shortcut in the Startup
folder in the Windows Start Menu so that the emulator is started
automatically.

4. In your imaging program, connect to your "Alnitak" on the other
virtual COM port. For example, if your virtual COM ports are COM22 and
COM23 and alnitak_emu.exe is started with COM23, your imaging program
would connect to the emulated Alnitak on COM22.


----------
Extras - programs included in the zip file
----------

usbdctrl.exe - a command-line program for controlling the panel.

Usage: usbdctrl.exe [BRIGHTNESS]

BRIGHTNESS is a value from 0 to 1023. If run with no command-line
option (BRIGHTNESS omitted), the program reports the current
brightness value.

Example:
 usbdctrl.exe 800           -- sets the brightness to 800 / 1023
 usbdctrl.exe 0             -- turns off the panel
 usbdctrl.exe               -- reports the current brightness value (0-1023)

----------

aacmd.exe - a command-line program for controlling the panel, compatible with Alnitak's AAcmd.exe

Usage:

aacmd.exe <COM port number> L Bnnn S      - turns the panel on to brightness nnn 1-255
aacmd.exe <COM port number> D S           - turns the panel off
aacmd.exe <COM port number> V             - version: x.x.x
aacmd.exe <COM port number> O S           - opens the FlipFlat (does nothing for the Spike-a)
aacmd.exe <COM port number> C S           - closes the FlipFlat (does nothing for the Spike-a)

<COM Port number> is a dummy argument that is ignored, present for compatibility with Alnitak's AACmd.exe.

--

Andy Galasso <andy.galasso@gmail.com>
