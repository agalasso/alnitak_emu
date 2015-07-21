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

#ifndef ALNITAK_EMU_INCLUDED
#define ALNITAK_EMU_INCLUDED

#include "usbdimmer.h"

#include <string>

class AlnitakEmu
{
    int prodid;
    int cover;   // 0 = none, 1=closed, 2=open, 3=timed-out
    bool light;     //  light on?
    bool motor;  //  motor on?
    unsigned char blvl; // brightness level 0..255

    USBDimmer m_dimmer;

public:

    AlnitakEmu();
    ~AlnitakEmu();

    bool Connect();
    void Disconnect();

    std::string stateStr() const;
    std::string brightnessStr() const;
    std::string responseStr(int cmd, const char *data = "000") const;

    void setLightOn(bool on);
    bool getLightOn() const;
    void setBrightness(unsigned char val);
    unsigned int getBrightness() const;
};

inline AlnitakEmu::AlnitakEmu() :
    prodid(10),
    cover(0),
    light(false),
    motor(false),
    blvl(255)
{ }

inline AlnitakEmu::~AlnitakEmu()
{
    m_dimmer.Disconnect();
}

inline bool AlnitakEmu::getLightOn() const
{
    return light;
}

inline unsigned int AlnitakEmu::getBrightness() const
{
    return blvl;
}

inline void AlnitakEmu::Disconnect()
{
    m_dimmer.Disconnect();
}

#endif
