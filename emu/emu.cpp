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

#include "alnitak_emu.h"
#include "serialport.h"
#include "dbg.h"
#include <string>
#include <stdio.h>

#ifdef _MSC_VER
# define snprintf _snprintf_s
# define sscanf sscanf_s
#endif

static AlnitakEmu panel;

static void respond(SerialPort& dev, const std::string& resp)
{
    VERBOSE("send response: %s", resp.c_str());

    bool ok = dev.Write(resp.c_str(), resp.length());
    if (!ok) {
        WARN("respond: write failed\n");
    }
}

// ===========================
// usage: emu COM33

int
main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "usage: emu.exe COM_PORT\n"
                "\n"
                "  Example: emu.exe COM32\n");
        return 1;
    }

#ifndef _MSC_VER
    setvbuf(stdout, NULL, _IOLBF, 0);
#endif

    SerialPort dev;
    char devname[128];
    snprintf(devname, sizeof(devname), "\\\\.\\%s", argv[1]);

    MSG("open %s\n", devname);

    int ret = dev.Open(devname, 9600, "8N1", SerialPort::NoFlowControl);
    if (ret != 0) {
        ERR("open failed for %s\n", devname);
        return 1;
    }

    while (true)
    {
        MSG("Connecting to panel\n");
        if (panel.Connect())
            break;
        ERR("could not connect to panel, waiting...\n");
        Sleep(10000);
    }

    MSG("panel light %s brightness %u\n", panel.getLightOn() ? "ON" : "OFF", panel.getBrightness());

    while (1) {

        char buf[1024];
        size_t rem = sizeof(buf) - 1;
        char *p = &buf[0];
        while (rem > 0) {
            ssize_t const nread = dev.Read(p, 1);
            if (nread == 0) {
//                printf("%s\n", "read timed-out");
            }
            if (nread < 0) {
                WARN("COM port read error\n");
                return 1;
            }
            if (*p == '\n') {
                *p = 0;
                VERBOSE("recv %s\n", buf);
                break;
            }
            p += nread;
            rem -= nread;
        }

        switch (buf[1]) {
        case 'S': {
            MSG("REQ: get state\n");
            respond(dev, panel.stateStr());
            break;
        }

        case 'L': {
            MSG("REQ: light on\n");
            panel.setLightOn(true);
            MSG("panel light %s brightness %u\n", panel.getLightOn() ? "ON" : "OFF", panel.getBrightness());
            respond(dev, panel.responseStr('L'));
            break;
        }

        case 'D': {
            MSG("REQ: light off\n");
            panel.setLightOn(false);
            MSG("panel light %s brightness %u\n", panel.getLightOn() ? "ON" : "OFF", panel.getBrightness());
            respond(dev, panel.responseStr('D'));
            break;
        }

        case 'B': {
            unsigned int b;
            if (sscanf(&buf[2], "%u", &b) == 1) {
                MSG("REQ: set brightness %u\n", b);
                panel.setBrightness(b);
                MSG("panel light %s brightness %u\n", panel.getLightOn() ? "ON" : "OFF", panel.getBrightness());
                respond(dev, panel.responseStr('B', &buf[2]));
            }
            break;
        }

        case 'J': {
            MSG("REQ: get brightness\n");
            char bbuf[3];
            snprintf(bbuf, sizeof(bbuf), "%03u", panel.getBrightness());
            respond(dev, panel.responseStr('J', bbuf));
            break;
        }

        case 'V': {
            MSG("REQ: get version\n");
            respond(dev, panel.responseStr('V', "999"));
            break;
        }

        default:
            MSG("REQ: unexpected command '%c'\n", buf[1]);
            break;
        }
    }

    return 0;
}
