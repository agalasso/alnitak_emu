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

#ifdef _MSC_VER
# define snprintf _snprintf_s
# define sscanf sscanf_s
#endif

static unsigned char alnitak_val(unsigned int inten)
{
    return (unsigned char)(inten / 4);
}

static unsigned int usbd_val(unsigned int blvl)
{
    return blvl == 255 ? 1023 : blvl * 4;
}

bool AlnitakEmu::Connect()
{
    int ret = m_dimmer.Connect();
    if (ret == USBDimmer::USBD_SUCCESS)
    {
        light = m_dimmer.IsLightOn() == 1;
        blvl = alnitak_val(m_dimmer.GetBrightness());
        return true;
    }
    return false;
}

std::string AlnitakEmu::responseStr(int cmd, const char *data) const
{
    char buf[] = {
        '*', (char)cmd,
        '0' + (char)prodid / 10, '0' + (char)(prodid % 10),
        data[0], data[1], data[2], '\n', 0 };
    return buf;
}

std::string AlnitakEmu::stateStr() const
{
    char buf[] = {
        motor ? '1' : '0',
        light ? '1' : '0',
        '0' + cover
    };
    return responseStr('S', buf);
}

std::string AlnitakEmu::brightnessStr() const
{
    char buf[3];
    snprintf(buf, sizeof(buf), "%03u", (unsigned int)blvl);
    return responseStr('B', buf);
}

void AlnitakEmu::setLightOn(bool on)
{
    if (m_dimmer.LightOn(on) == USBDimmer::USBD_SUCCESS)
        light = on;
}

void AlnitakEmu::setBrightness(unsigned char val)
{
    unsigned int dval = usbd_val(val);
    if (m_dimmer.SetBrightness(dval) == USBDimmer::USBD_SUCCESS)
        blvl = val;
}
