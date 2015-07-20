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

#include <SDKDDKVer.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "usbd.h"
#include "usbdimmer.h"

extern "C" {

USBD_API usbd_handle USBD_Open()
{
    return new USBDimmer();
}

USBD_API void USBD_Close(usbd_handle usbd)
{
    if (usbd)
    {
        USBDimmer *p = (USBDimmer *)usbd;
        delete p;
    }
}

USBD_API int USBD_Connect(usbd_handle usbd)
{
    USBDimmer *p = (USBDimmer *)usbd;
    return p->Connect();
}

USBD_API void USBD_Disconnect(usbd_handle usbd)
{
    USBDimmer *p = (USBDimmer *)usbd;
    p->Disconnect();
}

USBD_API int USBD_SetBrightness(usbd_handle usbd, unsigned int val)
{
    USBDimmer *p = (USBDimmer *)usbd;
    return p->SetBrightness(val);
}

USBD_API int USBD_GetBrightness(usbd_handle usbd)
{
    USBDimmer *p = (USBDimmer *)usbd;
    return p->GetBrightness();
}

USBD_API int USBD_IsLightOn(usbd_handle usbd)
{
    USBDimmer *p = (USBDimmer *)usbd;
    return p->IsLightOn();
}

USBD_API int USBD_LightOn(usbd_handle usbd, bool on)
{
    USBDimmer *p = (USBDimmer *)usbd;
    return p->LightOn(on);
}

}
