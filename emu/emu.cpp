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
static time64_t disconnect_time;

static unsigned int idle_timeout = 15 * 1000;  // disconnect after 15 seconds

static void respond(SerialPort& dev, const std::string& resp)
{
    VERBOSE("send response: %s", resp.c_str());

    bool ok = dev.Write(resp.c_str(), resp.length());
    if (!ok) {
        WARN("respond: write failed\n");
    }
}

inline static void update_disconnect_time()
{
    disconnect_time = time64() + idle_timeout;
}

static bool ConnectPanel()
{
    if (panel.IsConnected())
        return true;

    MSG("Connecting to panel\n");

    if (!panel.Connect())
    {
        ERR("could not connect to panel\n");
        return false;
    }

    MSG("panel light %s brightness %u\n", panel.getLightOn() ? "ON" : "OFF", panel.getBrightness());

    update_disconnect_time();
    return true;
}

static void DisconnectPanel()
{
    MSG("disconnect panel\n");
    panel.Disconnect();
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

    bool quit = false;

    while (!quit) {

        char buf[1024];
        size_t rem = sizeof(buf) - 1;
        char *p = &buf[0];
        while (rem > 0) {
            ssize_t const nread = dev.Read(p, 1);
            if (nread == 0) {
                // read timed-out (3s)
                if (panel.IsConnected() && time64() > disconnect_time) {
                    MSG("idle timeout\n");
                    DisconnectPanel();
                }
                continue;
            }
            if (nread < 0) {
                ERR("COM port read error!\n");
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
            if (ConnectPanel()) {
                std::string state = panel.stateStr();
                update_disconnect_time();
                respond(dev, state);
            }
            break;
        }

        case 'L': {
            MSG("REQ: light on\n");
            if (ConnectPanel()) {
                if (panel.setLightOn(true)) {
                    MSG("panel light %s brightness %u\n", panel.getLightOn() ? "ON" : "OFF", panel.getBrightness());
                    update_disconnect_time();
                    respond(dev, panel.responseStr('L'));
                }
                else {
                    MSG("set light on failed, disconnect\n");
                    DisconnectPanel();
                    // no response
                }
            }
            break;
        }

        case 'D': {
            MSG("REQ: light off\n");
            if (ConnectPanel()) {
                if (panel.setLightOn(false)) {
                    MSG("panel light %s brightness %u\n", panel.getLightOn() ? "ON" : "OFF", panel.getBrightness());
                    update_disconnect_time();
                    respond(dev, panel.responseStr('D'));
                }
                else {
                    MSG("set light on failed, disconnect\n");
                    DisconnectPanel();
                    // no response
                }
            }
            break;
        }

        case 'B': {
            unsigned int b;
            if (sscanf(&buf[2], "%u", &b) == 1) {
                MSG("REQ: set brightness %u\n", b);
                if (ConnectPanel()) {
                    if (panel.setBrightness(b)) {
                        MSG("panel light %s brightness %u\n", panel.getLightOn() ? "ON" : "OFF", panel.getBrightness());
                        update_disconnect_time();
                        respond(dev, panel.responseStr('B', &buf[2]));
                    }
                    else {
                        MSG("set brightness failed, disconnect\n");
                        DisconnectPanel();
                        // no response
                    }
                }
            }
            break;
        }

        case 'J': {
            MSG("REQ: get brightness\n");
            if (ConnectPanel()) {
                char bbuf[3];
                snprintf(bbuf, sizeof(bbuf), "%03u", panel.getBrightness());
                update_disconnect_time();
                respond(dev, panel.responseStr('J', bbuf));
            }
            break;
        }

        case 'V': {
            MSG("REQ: get version\n");
            if (ConnectPanel()) {
                update_disconnect_time();
                respond(dev, panel.responseStr('V', "999"));
            }
            break;
        }

        case 'u': { // not part of alnitak proto
            if (strcmp(buf, "quit") == 0)
                quit = true;
            break;
        }

        default:
            MSG("REQ: unexpected command '%c'\n", buf[1]);
            break;
        }
    }

    return 0;
}
