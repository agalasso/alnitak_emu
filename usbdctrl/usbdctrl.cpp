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

#include "usbdimmer.h"
#include "dbg.h"

#include <stdio.h>
int main(int argc, char* argv[])
{
    int inten = -1;
    if (argc == 2)
    {
        unsigned int t;
        if (sscanf_s(argv[1], "%u", &t) == 1)
            inten = t;
    }

    USBDimmer dimmer;

    int ret = dimmer.Connect();
    if (ret != USBDimmer::USBD_SUCCESS)
    {
        ERR("could not connect\n");
        return 1;
    }

    if (inten == 0)
    {
        dimmer.LightOn(false);
    }
    else if (inten > 0)
    {
        dimmer.SetBrightness(inten);
        dimmer.LightOn(true);
    }
    else {
        int is_on = dimmer.IsLightOn();
        if (is_on < 0)
        {
            return 1;
        }
        if (is_on == 0)
        {
            printf("light is off\n");
        }
        else
        {
            int inten = dimmer.GetBrightness();
            printf("light is on, intensity %d\n", inten);
        }
    }

	return 0;
}

