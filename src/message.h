
/* vim: set et ts=3 sw=3 ft=c sts=3:
 *
 * Copyright (C) 2013 James McLaughlin et al.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef LNET_MESSAGE_H
#define LNET_MESSAGE_H

#include "common.h"
#include "buffer.h"

typedef struct _lnet_message
{
    char parse_flags;

    lw_ui8 type;
    lw_ui8 variant;

    size_t length;

} * lnet_message;

#define LNET_MESSAGE_STATUS_MORE    0  /* need more data */
#define LNET_MESSAGE_STATUS_READY   1  /* the message is ready to process */

int lnet_message_read (lnet_message, lnet_buffer, int * status);
void lnet_message_next (lnet_message);

void lnet_message_send (lw_stream stream,
                        lw_ui8 type,
                        lw_ui8 variant,
                        size_t num, ...);

void lnet_message_blast (lw_udp udp,
                         lw_addr addr,
                         lw_i16 id,
                         lw_ui8 type,
                         lw_ui8 variant,
                         size_t num, ...);

/* Server -> client messages */

#define LNET_MESSAGE_SC_RESPONSE                   0
#define LNET_MESSAGE_SC_SERVER_MESSAGE             1
#define LNET_MESSAGE_SC_CHANNEL_MESSAGE            2
#define LNET_MESSAGE_SC_PEER_MESSAGE               3
#define LNET_MESSAGE_SC_SERVER_CHANNEL_MESSAGE     4
#define LNET_MESSAGE_SC_UPDATE_PEER                9
#define LNET_MESSAGE_SC_UDP_WELCOME                10
#define LNET_MESSAGE_SC_PING                       11


/* Client -> server messages */

#define LNET_MESSAGE_CS_REQUEST                    0
#define LNET_MESSAGE_CS_SERVER_MESSAGE             1
#define LNET_MESSAGE_CS_CHANNEL_MESSAGE            2
#define LNET_MESSAGE_CS_PEER_MESSAGE               3
#define LNET_MESSAGE_CS_UDP_HELLO                  7
#define LNET_MESSAGE_CS_CHANNEL_MASTER             8
#define LNET_MESSAGE_CS_PONG                       9


/* Request types */

#define LNET_REQUEST_CONNECT         0
#define LNET_REQUEST_SET_NAME        1
#define LNET_REQUEST_JOIN_CHANNEL    2
#define LNET_REQUEST_LEAVE_CHANNEL   3
#define LNET_REQUEST_CHANNEL_LIST    4


/* Response types */

#define LNET_RESPONSE_CONNECT          0
#define LNET_RESPONSE_SET_NAME         1
#define LNET_RESPONSE_JOIN_CHANNEL     2
#define LNET_RESPONSE_LEAVE_CHANNEL    3
#define LNET_RESPONSE_CHANNEL_LIST     4


/* Channel flags */

#define LNET_CHANNEL_FLAG_HIDE         1
#define LNET_CHANNEL_FLAG_AUTO_CLOSE   2

#endif

