Alnitak Flat-Man emulation for Spike-a Flat Panel USB Dimmer
------------------------------------------------------------

My imaging software of choice (Sequence Generator Pro) has integrated
support for Alnitak Flat Panels.  I needed a way to control my Spike-a
Flat Panel from SGP, so I put together this emulation app.

The emulator works by listening for Alnitak commands on a virtual COM
port, then sending the corresponding translated command to the Spike-a
USB dimmer.

Installation
------------

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

4. In your imaging program, connect to your "Alnitak" on the other
virtual COM port. For example, if your virtual COM ports are COM22 and
COM23 and alnitak_emu.exe is started with COM23, your imaging program
would connect to the emulated Alnitak on COM22.

----------

usbdctrl.exe is also included in the zip file.

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

aacmd.exe is also included in the zip file.

aacmd.exe - a command-line program for controlling the panel, compatible with Alnitak's AAcmd.exe

Usage:

aacmd.exe <COM port number> L Bnnn S      - turns the panel on to brightness nnn 1-255
aacmd.exe <COM port number> D S           - turns the panel off
aacmd.exe <COM port number> V             - version: x.x.x
aacmd.exe <COM port number> O S           - opens the FlipFlat
aacmd.exe <COM port number> C S           - closes the FlipFlat

<COM Port number> is a dummy argument that is ignored, present for compatibility with Alnitak's AACmd.exe.

--

Andy Galasso <andy.galasso@gmail.com>
