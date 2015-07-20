/*
 * Alnitak Flat-man emulator for Spike-a USB dimmer
 *
 * Copyright (c) 2015 Andy Galasso <andy.galasso@gmail.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * Neither the name of Andy Galasso, adgsoftware.com nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "usbdimmer.h"
#include "dbg.h"

static void usage()
{
    fprintf(stderr, "Usage: aacmd.exe OPTIONS\n"
        "\n"
        "aacmd.exe <COM port number> L Bnnn S      - turns the panel on to brightness nnn 1-255\n"
        "aacmd.exe <COM port number> D S           - turns the panel off\n"
        "aacmd.exe <COM port number> V             - version: x.x.x\n"
        "aacmd.exe <COM port number> O S           - opens the FlipFlat\n"
        "aacmd.exe <COM port number> C S           - closes the FlipFlat\n");
    exit(1);
}

static unsigned int usbd_val(unsigned int blvl)
{
    return blvl == 255 ? 1023 : blvl * 4;
}

/*
Using the command line application, AACmd.exe
> This is a command line application for controlling Flip-Flat, Flat-Man and Flat-Man XL.
> TheWindows application must be closed before the AACmd.exe application can be run.
>
> From the command prompt type "<path>/AACmd" followed by a COM port
> number and a command and, optionally a switch (where the path points to the installation
> directory).
> The (case insensitive) single letter commands are:
> A find all Alnitak devices and the COM ports they are associated with
> C close (works with Flip-Flat only)
> O open (works with Flip-Flat only)
> L turn on light
> D turn off light
> Bxxx set brightness. Can be a 1,2, or 3 digit number between 0-255
> G get brightness from device
> V get AACmd.exe version number
> F get device firmware version number
> S silent: issue command and close window immediately after command has
> completed. If this command is not used the console window displays for 3 seconds.
>
> For example:
> “AACmd” A will list the COM ports of connected devices
> "AACmd” 6 o will open the Flip-Flat on COM 6.
> A message will be displayed in the command window reflecting the result or an error
> message. The command window will remain open for 3 seconds.
> “AACmd” b34 s will set the brightness to level 34 and immediately close the console
> window. Note: if a number outside the 0-255 range is entered it will be limited to 0-255.
> A file named fflog.txt (in the same directory as AACmd.exe) will be generated (and
> appended to) whenever AACmd is run.
>
> CCDAutoPilot calls aacmd.exe after navigating to the fully qualified path with the following options used:
> aacmd.exe<space> <COM port number><space>L<space>Bnnn<space> S (turns the panel on to specified brightness)
> where nnn is from 1 - 255
> (I know your panel has finer granularity but CCDAutoPilot has an automatic exposure routine so it doesn’t have to be that precise.)
>
> aacmd.exe<space> <COM port number><space>D<space>S (turns the panel off)
>
> aacmd.exe<space> <COM port number><space>V (returns “version: x.x.x” as a string. CCDAutoPilot parses the remainder after the ‘:’ as the version.
>
> Your application should ignore the following:
> aacmd.exe<space> <COM port number><space>O<space>S (opens the FlipFlat)
> aacmd.exe<space> <COM port number><space>C<space>S (closes the FlipFlat)
>
> CCDAutoPilot will generate the COM port number from the user interface but obviously your application should ignore it.
*/
// aacmd.exe <COM port number> L Bnnn S    // panel on to brightness nnn 1-255
// aacmd.exe <COM port number> D S         // turns the panel off
// aacmd.exe <COM port number> V           // “version: x.x.x”
// aacmd.exe <COM port number> O S         // opens the FlipFlat
// aacmd.exe <COM port number> C S         // closes the FlipFlat

int main(int argc, char *argv[])
{
    if (argc < 3)
        usage();

    int inten = -1;

    switch (toupper(argv[2][0])) {
    case 'L': {
        if (argc < 4 || toupper(argv[3][0]) != 'B')
            usage();
        unsigned int val = 0;
        int n = sscanf_s(&argv[3][1], "%u", &val);
        if (n != 1 || val < 1 || val > 255)
            usage();
        inten = (int)val;
        break;
    }
    case 'D':
        inten = 0;
        break;
    case 'V':
        printf("version: 3.14.16\n");
        return 0;
    case 'O':
    case 'C':
        // no-op
        return 0;
    default:
        usage();
    }

    USBDimmer dimmer;

    int ret = dimmer.Connect();
    if (ret != USBDimmer::USBD_SUCCESS)
    {
        fprintf(stderr, "could not connect to usb dimmer!\n");
        return 1;
    }

    if (inten == 0)
    {
        dimmer.LightOn(false);
    }
    else
    {
        dimmer.SetBrightness(usbd_val(inten));
        dimmer.LightOn(true);
    }

    return 0;
}

