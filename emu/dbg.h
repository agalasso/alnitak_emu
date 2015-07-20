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

#ifndef DBG_H
#define DBG_H

#include <stdio.h>

namespace dbg {
    extern bool verbose;
}

#ifdef DBG_ENABLED
extern void dbgpr(const char *format, ...);
# define ERR(format, ...) dbgpr("ERROR: " format, ## __VA_ARGS__)
# define FATAL(format, ...) do { dbgpr(stderr, "FATAL: " format, ## __VA_ARGS__); exit(1); } while (0)
# define WARN(format, ...) dbgpr("WARN: " format, ## __VA_ARGS__)
# define MSG(format, ...) dbgpr("MSG: " format, ## __VA_ARGS__)
# define VERBOSE(format, ...) do { if (dbg::verbose) dbgpr("VERBOSE: " format, ## __VA_ARGS__); } while (0)
#else
# define ERR(format, ...) do { } while (0)
# define FATAL(format, ...)  do { } while (0)
# define WARN(format, ...)  do { } while (0)
# define MSG(format, ...)  do { } while (0)
# define VERBOSE(format, ...)  do { } while (0)
#endif

#endif
