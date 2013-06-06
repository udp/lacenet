
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

#ifndef LACENET_CLIENT_H
#define LACENET_CLIENT_H

#include <lacenet_client.h>
#include "../deps/uthash/uthash.h"

#include <stdlib.h>
#include <string.h>

#include "../message.h"

struct _lnclient
{
   void * tag;

   lnclient_config config;

   lw_client client;

   struct _lnet_message message;

   lw_stream socket;

   lw_udp udp;
   lw_addr udp_addr;

   int16_t id;
   char * name;

   list (struct _lnclient_channel, channels);

   char * welcome_message;

   lw_timer timer;

   int flags;
};

lw_stream lnclient_stream (lnclient);

#define LNCLIENT_FLAG_GOT_WELCOME  1

#include "channel.h"
#include "peer.h"

/* Message types */

int lnclient_proc_response
   (lnclient, int variant, lnet_buffer);

int lnclient_proc_server_message         
   (lnclient, lw_bool blasted, int variant, lnet_buffer);

int lnclient_proc_channel_message        
   (lnclient, lw_bool blasted, int variant, lnet_buffer);

int lnclient_proc_server_channel_message 
   (lnclient, lw_bool blasted, int variant, lnet_buffer);

int lnclient_proc_peer_message    
   (lnclient, lw_bool blasted, int variant, lnet_buffer);

int lnclient_proc_update_peer
   (lnclient, int variant, lnet_buffer);

int lnclient_proc_udp_welcome
   (lnclient, int variant, lnet_buffer);


/* Request: Connect */

void lnclient_send_request_connect
   (lnclient);

int lnclient_proc_response_connect
   (lnclient, lw_bool success, lnet_buffer);


/* Request: SetName */

void lnclient_send_request_set_name
   (lnclient, const char * name);

int lnclient_proc_response_set_name
   (lnclient, lw_bool success, lnet_buffer);


/* Request: JoinChannel */

void lnclient_send_request_join_channel
   (lnclient, const char * channel_name, lw_i8 join_flags);

int lnclient_proc_response_join_channel
   (lnclient, lw_bool success, lnet_buffer);


/* Request: LeaveChannel */
   
void lnclient_send_request_leave_channel
   (lnclient, lw_i16 channel_id);

int lnclient_proc_response_leave_channel
   (lnclient, lw_bool success, lnet_buffer);


/* Request: ChannelList */

void lnclient_send_request_channel_list
   (lnclient);

int lnclient_proc_response_channel_list
   (lnclient, lw_bool success, lnet_buffer);

#endif

