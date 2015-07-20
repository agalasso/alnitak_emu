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

#include "serialport.h"
#include "dbg.h"

#ifdef _MSC_VER
#define snprintf _snprintf_s
#define strncpy strncpy_s
#endif

const char *
SerialPort_DCS::GetSettings()
{
    const char ac[5] = {'N','O','E','M','S'};
    memset(buf,0,sizeof(buf));
    snprintf(buf,sizeof(buf)-1,"%i%c%i %i",
             wordlen,
             ac[parity],
             stopbits,
             baud);
    return buf;
}

SerialPort::SerialPort()
{
    m_devname[0] = '\0';
    m_fd = INVALID_HANDLE_VALUE;
    memset(&m_ov_rd, 0, sizeof(m_ov_rd));
    memset(&m_ov_wr, 0, sizeof(m_ov_wr));
}

SerialPort::~SerialPort()
{
    Close();
};

void
SerialPort::Close()
{
    if (m_fd != INVALID_HANDLE_VALUE) {
        CloseHandle(m_ov_rd.hEvent);
        CloseHandle(m_ov_wr.hEvent);
        CloseHandle(m_fd);
        m_fd = INVALID_HANDLE_VALUE;
    }
}

int
SerialPort::Open(const char *portname, int baudrate,
                 const char *protocol,
                 FlowControl flowControl)
{
    SerialPort_DCS dcs;

    dcs.baud = baudrate;

    // default wordlen is 8
    if ((protocol[0] >= '5') && (protocol[0] <= '8'))
        dcs.wordlen = protocol[0] - '0';
    else
        return -1;

    // protocol is given as a string like "8N1", the first
    // character specifies the data bits (5...8), the second
    // the parity (None,Odd,Even,Mark,Space).
    // The third character defines the stopbit (1...2).
    switch (protocol[1]) {
    case 'N': case 'n': dcs.parity = ParityNone; break;
    case 'O': case 'o': dcs.parity = ParityOdd; break;
    case 'E': case 'e': dcs.parity = ParityEven; break;
    case 'M': case 'm': dcs.parity = ParityMark; break;
    case 'S': case 's': dcs.parity = ParitySpace; break;
    default: return -1;
    }
    // default stopbits is 1
    if ((protocol[2] >= '1') && (protocol[2] <= '2'))
        dcs.stopbits = protocol[2] - '0';
    else
        return -1;

    // default flow control is disabled
    dcs.rtscts = (flowControl == RtsCtsFlowControl);

    dcs.xonxoff = (flowControl == XonXoffFlowControl);

    return OpenDevice(portname, dcs);
}

int
SerialPort::OpenDevice(const char *devname, const SerialPort_DCS& dcs)
{
    m_dcs = dcs;

    m_fd = CreateFileA(devname,
                       GENERIC_READ | GENERIC_WRITE,
                       0,		// not shared
                       NULL,
                       OPEN_EXISTING,
                       FILE_FLAG_OVERLAPPED,
                       NULL);

    if (m_fd == INVALID_HANDLE_VALUE) {
        MSG("%s CreateFile(%s) returned invalid file handle", __FUNCTION__, devname);
        return -1;
    }

    // save the device name
    strncpy(m_devname, devname, sizeof(m_devname));
    m_devname[sizeof(m_devname)-1] = '\0';

    // device control block
    DCB dcb;
    memset(&dcb, 0, sizeof(dcb));
    dcb.DCBlength = sizeof(dcb);
    dcb.BaudRate = m_dcs.baud;
    dcb.fBinary = 1;

    // Specifies whether the CTS (clear-to-send) signal is monitored for
    // output flow control. If this member is TRUE and CTS is turned off,
    // output is suspended until CTS is sent again.
    dcb.fOutxCtsFlow = m_dcs.rtscts;

    // Specifies the DTR (data-terminal-ready) flow control.
    // This member can be one of the following values:
    // DTR_CONTROL_DISABLE   Disables the DTR line when the device is
    //                       opened and leaves it disabled.
    // DTR_CONTROL_ENABLE    Enables the DTR line when the device is
    //                       opened and leaves it on.
    // DTR_CONTROL_HANDSHAKE Enables DTR handshaking. If handshaking is
    //                       enabled, it is an error for the application
    //                       to adjust the line by using the
    //                       EscapeCommFunction function.
    dcb.fDtrControl = DTR_CONTROL_ENABLE;

    // Specifies the RTS flow control. If this value is zero, the
    // default is RTS_CONTROL_HANDSHAKE. This member can be one of
    // the following values:
    // RTS_CONTROL_DISABLE   Disables the RTS line when device is
    //                       opened and leaves it disabled.
    // RTS_CONTROL_ENABLED   Enables the RTS line when device is
    //                       opened and leaves it on.
    // RTS_CONTROL_HANDSHAKE Enables RTS handshaking. The driver
    //                       raises the RTS line when the
    //                       "type-ahead" (input)buffer is less than
    //                       one-half full and lowers the RTS line
    //                       when the buffer is more than three-quarters
    //                       full. If handshaking is enabled, it is an
    //                       error for the application to adjust the
    //                       line by using the EscapeCommFunction function.
    // RTS_CONTROL_TOGGLE    Specifies that the RTS line will be high if
    //                       bytes are available for transmission. After
    //                       all buffered bytes have been send, the RTS
    //                       line will be low.
    if (m_dcs.rtscts)
        dcb.fRtsControl = RTS_CONTROL_HANDSHAKE;
    else
        dcb.fRtsControl = RTS_CONTROL_DISABLE;

    // Specifies the XON/XOFF flow control.
    // If fOutX is true (the default is false), transmission stops when the
    // XOFF character is received and starts again, when the XON character
    // is received.
    dcb.fOutX = m_dcs.xonxoff;
    // If fInX is true (default is false), the XOFF character is sent when
    // the input buffer comes within XoffLim bytes of being full, and the
    // XON character is sent, when the input buffer comes within XonLim
    // bytes of being empty.
    dcb.fInX = m_dcs.xonxoff;
    // default character for XOFF is 0x13 (hex 13)
    dcb.XoffChar = 0x13;
    // default character for XON is 0x11 (hex 11)
    dcb.XonChar = 0x11;
    // set the minimum number of bytes allowed in the input buffer before
    // the XON character is sent (3/4 of full size)
    dcb.XonLim = (SERIALPORT_BUFSIZE >> 2) * 3;
    // set the maximum number of free bytes in the input buffer, before the
    // XOFF character is sent (3/4 of full size)
    dcb.XoffLim = (SERIALPORT_BUFSIZE >> 2) * 3;

    // parity
    switch (m_dcs.parity) {
    case ParityOdd: dcb.Parity = ODDPARITY; break;
    case ParityEven: dcb.Parity = EVENPARITY; break;
    case ParityMark: dcb.Parity = MARKPARITY; break;
    case ParitySpace: dcb.Parity = SPACEPARITY; break;
    default: dcb.Parity = NOPARITY;
    }

    // stopbits
    if (m_dcs.stopbits == 2)
        dcb.StopBits = TWOSTOPBITS;
    else
        dcb.StopBits = ONESTOPBIT;
    // wordlen, valid values are 5,6,7,8
    dcb.ByteSize = m_dcs.wordlen;

    if(!SetCommState(m_fd, &dcb))
        return -2;

    m_ov_rd.hEvent = CreateEvent(NULL,// LPSECURITY_ATTRIBUTES lpsa
                                 FALSE, // BOOL fManualReset
                                 FALSE, // BOOL fInitialState
                                 NULL); // LPTSTR lpszEventName
    if (m_ov_rd.hEvent == INVALID_HANDLE_VALUE)
        return -3;

    m_ov_wr.hEvent = CreateEvent(NULL,// LPSECURITY_ATTRIBUTES lpsa
                                 FALSE, // BOOL fManualReset
                                 FALSE, // BOOL fInitialState
                                 NULL); // LPTSTR lpszEventName
    if (m_ov_wr.hEvent == INVALID_HANDLE_VALUE)
        return -3;

    COMMTIMEOUTS cto;

    // Arrange for ReadFile to return in 3000 ms
    cto.ReadIntervalTimeout = MAXDWORD;
    cto.ReadTotalTimeoutMultiplier = 0;
    cto.ReadTotalTimeoutConstant = 3000;

    // no write timeouts
    cto.WriteTotalTimeoutMultiplier = 0;
    cto.WriteTotalTimeoutConstant = 0;

    if (!SetCommTimeouts(m_fd, &cto))
        return -5;

    if (!SetupComm(m_fd, SERIALPORT_BUFSIZE, SERIALPORT_BUFSIZE))
        return -6;

    return 0;
}

ssize_t
SerialPort::Read(void *buf, size_t len)
{
    BOOL ok = ReadFile(m_fd, buf, len, NULL, &m_ov_rd);

    if (!ok && GetLastError() != ERROR_IO_PENDING) {
        WARN("%s: ReadFile failed", __FUNCTION__);
        return -1;
    }

    DWORD nread;

    ok = GetOverlappedResult(m_fd, &m_ov_rd, &nread, TRUE /*wait*/);
    if (!ok) {
        WARN("%s: GetOverlappedResult failed", __FUNCTION__);
        return -1;
    }

    return nread;
}

bool
SerialPort::Write(const void *buf, size_t len)
{
    DWORD written;
    BOOL ok = GetOverlappedResult(m_fd, &m_ov_wr, &written, FALSE /*no wait*/);
    if (!ok && GetLastError() != ERROR_IO_INCOMPLETE) {
        WARN("%s: GetOverlappedResult failed", __FUNCTION__);
        return false;
    }

    if (!ok) {
        // prior write not finished, drop this one
        WARN("%s: prior write not complete, dropping write", __FUNCTION__);
        return false;
    }

    ok = WriteFile(m_fd, buf, len, NULL, &m_ov_wr);
    if (!ok && GetLastError() != ERROR_IO_PENDING) {
        WARN("%s: WriteFile failed", __FUNCTION__);
        return false;
    }

    FlushFileBuffers(m_fd); // ignore error
    return true;
}
