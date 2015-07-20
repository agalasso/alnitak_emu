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

#include <stdio.h>
#include <wchar.h>
#include <string.h>
#include <stdlib.h>

#include "hidapi.h"
#include "dbg.h"

#include <windows.h>

#undef min
#undef max
#include <algorithm>

#define USBD_VID 0x04d8
#define USBD_PID 0xf5d1

#define CMD_SETSTATE 0x10
#define CMD_GETSTATE 0x11
#define CMD_SETINTEN 0x20
#define CMD_GETINTEN 0x21

#define DEFAULT_BRIGHTNESS 900

static int usbd_read(hid_device *handle, unsigned char *buf, size_t len)
{
    int ret = hid_read_timeout(handle, buf, len, 5000);
    if (ret < 0)
        WARN("hid_read_timeout returned %d\n", ret);
    if (ret == 0)
        VERBOSE("Read timed-out\n");
    return ret;
}

static void _dump(const char *descr, const unsigned char *buf, size_t len)
{
    printf("Data read [%s]:\n", descr);
    // Print out the returned buffer.
    for (unsigned int i = 0; i < len; i++)
        printf("%02hhx ", buf[i]);
    printf("\n");
}

static int get_inten(hid_device *handle)
{
    unsigned char buf[8];

    // getinten
    buf[0] = 0x0;
    buf[1] = CMD_GETINTEN;
    int res = hid_write(handle, buf, 5);
    if (res < 0) {
        WARN("Unable to write()\n");
        WARN("Error: %ls\n", hid_error(handle));
        return USBDimmer::USBD_COMM_ERROR;
    }

    // read response
    int ret = usbd_read(handle, buf, 5);
    if (ret > 0 && dbg::verbose)
        _dump("getinten response", buf, ret);

    if (ret < 3)
        return USBDimmer::USBD_COMM_ERROR;

    int inten = buf[1] | (((unsigned int)buf[2]) << 8);

    VERBOSE("read inten = %u\n", inten);

    return inten;
}

static int set_inten(hid_device *handle, int inten)
{
    unsigned char buf[8];

    // set intensity value
    VERBOSE("set intensity to %u\n", inten);
    buf[0] = 0x0;
    buf[1] = CMD_SETINTEN;
    buf[2] = (unsigned char)inten;
    buf[3] = (unsigned char)(inten >> 8);
    int ret = hid_write(handle, buf, 5);
    if (ret < 0) {
        WARN("hid_write: %d: %ls\n", ret, hid_error(handle));
        return USBDimmer::USBD_COMM_ERROR;
    }

    // read response
    ret = usbd_read(handle, buf, 5);
    if (ret <= 0)
    {
        WARN("timeout\n");
        return USBDimmer::USBD_COMM_ERROR;
    }
    if (dbg::verbose)
        _dump("setinten response", buf, ret);

    // setstate "goto"
    buf[0] = 0x0;
    buf[1] = CMD_SETSTATE;
    buf[2] = 3;
    ret = hid_write(handle, buf, 5);
    if (ret < 0) {
        WARN("hid_write: %d: %ls\n", ret, hid_error(handle));
        return USBDimmer::USBD_COMM_ERROR;
    }

    // read response
    ret = usbd_read(handle, buf, 5);
    if (ret <= 0)
        return USBDimmer::USBD_COMM_ERROR;
    if (dbg::verbose)
        _dump("setstate response", buf, ret);

    while (true)
    {
        // getstate
        buf[0] = 0x0;
        buf[1] = CMD_GETSTATE;
        ret = hid_write(handle, buf, 5);
        if (ret < 0) {
            WARN("hid_write: %d: %ls\n", ret, hid_error(handle));
            return USBDimmer::USBD_COMM_ERROR;
        }

        // read response
        ret = usbd_read(handle, buf, 5);
        if (ret <= 0)
            return USBDimmer::USBD_COMM_ERROR;
        if (dbg::verbose)
            _dump("getstate response", buf, ret);

        if (buf[1] == 0)
            break;

        Sleep(100);
    }

    return USBDimmer::USBD_SUCCESS;
}

USBDimmer::USBDimmer()
    :
    m_handle(0)
{
}

USBDimmer::~USBDimmer()
{
    Disconnect();
}

int USBDimmer::Connect()
{
    // Open the device using the VID, PID,
    // and optionally the Serial number.
    ////handle = hid_open(0x4d8, 0x3f, L"12345");
    m_handle = hid_open(USBD_VID, USBD_PID, NULL);
    if (!m_handle) {
        WARN("unable to open device\n");
        return USBD_COMM_ERROR;
    }

    int inten = get_inten(m_handle);
    if (inten < 0) {
        WARN("could not read intensity\n");
        return USBD_COMM_ERROR;
    }

    if (inten > 0)
    {
        m_on = true;
        m_brightness = inten;
    }
    else
    {
        m_on = false;
        m_brightness = DEFAULT_BRIGHTNESS;
    }

    return USBD_SUCCESS;
}

void USBDimmer::Disconnect()
{
    if (!m_handle)
        return;

    hid_close(m_handle);
    m_handle = 0;
}

int USBDimmer::SetBrightness(unsigned int val)
{
    if (!m_handle)
        return USBD_NOT_CONNECTED;
    val = std::min(std::max(val, 0U), 1023U);
    if (val == m_brightness)
        return USBD_SUCCESS;
    if (!m_on)
    {
        m_brightness = val;
        return USBD_SUCCESS;
    }
    int ret = set_inten(m_handle, val);
    if (ret == USBD_SUCCESS)
    {
        m_brightness = val;
        if (val == 0)
            m_on = false;
    }
    return ret;
}

int USBDimmer::GetBrightness() const
{
    if (!m_handle)
        return USBD_NOT_CONNECTED;
    return m_brightness;
}

int USBDimmer::IsLightOn() const
{
    if (!m_handle)
        return USBD_NOT_CONNECTED;

    return m_on ? 1 : 0;
}

int USBDimmer::LightOn(bool on)
{
    if (!m_handle)
        return USBD_NOT_CONNECTED;
    if (m_on == on)
        return USBD_SUCCESS;
    int inten = on ? m_brightness : 0;
    int ret = set_inten(m_handle, inten);
    if (ret != USBD_SUCCESS)
        return ret;
    m_on = inten > 0;
    return ret;
}
