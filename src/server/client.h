
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

#ifndef LACENET_SERVER_CLIENT_H
#define LACENET_SERVER_CLIENT_H

#include "server.h"

struct _lnserver_client
{
   void * tag;

   struct _lnet_message message;

   lnserver server;

   lw_stream socket;

   lw_addr udp_addr;

   int16_t id;
   char * name;

   list (lnserver_channel, channels);

   char flags;
};

lnserver_client lnserver_client_new (lnserver ctx, lw_server_client socket);
void lnserver_client_delete (lnserver_client);

lw_stream lnserver_client_stream (lnserver_client);

lnserver_client lnserver_read_client (lnserver, lnet_buffer);

lnserver_channel lnserver_client_read_channel (lnserver_client, lnet_buffer);

#define LNSERVER_CLIENT_FLAG_PONGED  1
#define LNSERVER_CLIENT_FLAG_CALLED_ON_CONNECT  2
#define LNSERVER_CLIENT_FLAG_SENT_WELCOME  4

#endif

