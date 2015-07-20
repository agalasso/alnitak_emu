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

#ifndef SERIALPORT_H
#define SERIALPORT_H

#include <windows.h>
#include <sys/types.h>

#ifdef _MSC_VER
typedef int ssize_t;
#endif

#define SERIALPORT_NAME_LEN 32
#define SERIALPORT_BUFSIZE 4096

enum Parity
{
    ParityNone,
    ParityOdd,
    ParityEven,
    ParityMark,
    ParitySpace
};

struct SerialPort_DCS
{
    int baud;
    Parity parity;
    unsigned char wordlen;
    unsigned char stopbits;
    bool rtscts;
    bool xonxoff;
    char buf[16];

    SerialPort_DCS() {
        baud = 38400;
        parity = ParityNone;
        wordlen = 8;
        stopbits = 1;
        rtscts = false;
        xonxoff = false;
    }

    const char *GetSettings();
};

struct SerialPort
{
    SerialPort_DCS m_dcs;
    char m_devname[SERIALPORT_NAME_LEN];

    HANDLE m_fd;
    OVERLAPPED m_ov_rd;
    OVERLAPPED m_ov_wr;

    int OpenDevice(const char *devname, const SerialPort_DCS& dcs);

public:

    enum FlowControl
    {
        NoFlowControl, /*!< No flow control at all */
        RtsCtsFlowControl, /*!< Enable RTS/CTS hardware flow control */
        XonXoffFlowControl /*!< Enable XON/XOFF protocol */
    };

    SerialPort();
    ~SerialPort();

    int Open(const char *portname, int baudrate,
             const char *protocol = "8N1",
             FlowControl flowControl = NoFlowControl);

    void Close();

    ssize_t Read(void *buf, size_t len);
    bool Write(const void *buf, size_t len);
};

#endif
