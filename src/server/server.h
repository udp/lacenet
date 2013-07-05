
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

#ifndef LNET_SERVER_H
#define LNET_SERVER_H

#include <lacenet_server.h>
#include "../deps/uthash/uthash.h"

#include <stdlib.h>
#include <string.h>

#include "../message.h"

struct _lnserver
{
   void * tag;

   lnserver_config config;

   lw_pump pump;

   lw_server server;

   lw_udp udp;

   lnserver_channel channels;
};

#include "client.h"
#include "channel.h"

int lnserver_proc_data (lnserver, lnet_buffer);


/* Message types */

int lnserver_proc_request
   (lnserver, lnserver_client, int variant, lnet_buffer);

int lnserver_proc_server_message         
   (lnserver, lnserver_client, lw_bool blasted, int variant, lnet_buffer);

int lnserver_proc_channel_message        
   (lnserver, lnserver_client, lw_bool blasted, int variant, lnet_buffer);

int lnserver_proc_server_channel_message 
   (lnserver, lnserver_client, lw_bool blasted, int variant, lnet_buffer);

int lnserver_proc_peer_message    
   (lnserver, lnserver_client, lw_bool blasted, int variant, lnet_buffer);

int lnserver_proc_udp_hello
   (lnserver, lnserver_client, int variant, lnet_buffer);

int lnserver_proc_channel_master
   (lnserver, lnserver_client, int variant, lnet_buffer);

int lnserver_proc_pong
   (lnserver, lnserver_client, int variant, lnet_buffer);


/* Request: Connect */

int lnserver_proc_request_connect (lnserver,
                                   lnserver_client,
                                   lnet_buffer);

void lnserver_send_success_connect (lnserver,
                                    lnserver_client,
                                    lw_i16 peer_id,
                                    const char * welcome_message);

void lnserver_send_failure_connect (lnserver,
                                    lnserver_client,
                                    const char * deny_reason);


/* Request: SetName */

int lnserver_proc_request_set_name (lnserver,
                                    lnserver_client,
                                    lnet_buffer);

void lnserver_send_success_set_name (lnserver,
                                     lnserver_client,
                                     const char * name);

void lnserver_send_failure_set_name (lnserver,
                                     lnserver_client,
                                     const char * name,
                                     const char * deny_reason);


/* Request: JoinChannel */

int lnserver_proc_request_join_channel (lnserver,
                                        lnserver_client,
                                        lnet_buffer);

void lnserver_send_success_join_channel (lnserver,
                                         lnserver_client,
                                         lw_i8 join_flags,
                                         lw_i16 channel_id,
                                         const char * channel_name);

void lnserver_send_failure_join_channel (lnserver,
                                         lnserver_client,
                                         const char * channel_name,
                                         const char * deny_reason);


/* Request: LeaveChannel */
   
int lnserver_proc_request_leave_channel (lnserver,
                                         lnserver_client,
                                         lnet_buffer);

void lnserver_send_success_leave_channel (lnserver,
                                          lnserver_client,
                                          lw_i16 channel_id);

void lnserver_send_failure_leave_channel (lnserver,
                                          lnserver_client,
                                          lw_i16 channel_id,
                                          const char * deny_reason);


/* Request: ChannelList */

int lnserver_proc_request_channel_list (lnserver,
                                        lnserver_client,
                                        lnet_buffer);

void lnserver_send_success_channel_list (lnserver, lnserver_client);

void lnserver_send_failure_channel_list (lnserver, lnserver_client);

#endif

