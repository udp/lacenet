
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

#ifndef LACENET_CLIENT_PUB_H
#define LACENET_CLIENT_PUB_H

#include <lacenet.h>

#if defined(__cplusplus) && !defined(_MSC_VER)
extern "C" {
#endif

typedef struct _lnclient * lnclient;


/* lnclient_channel: represents a channel the client is a member of
 */
typedef struct _lnclient_channel * lnclient_channel;

   void lnclient_channel_set_tag (lnclient_channel, void *);
   void * lnclient_channel_tag (lnclient_channel);

   long lnclient_num_channels (lnclient);

   lnclient_channel lnclient_channel_first (lnclient);
   lnclient_channel lnclient_channel_next (lnclient_channel);

   const char * lnclient_channel_name (lnclient_channel);
   lw_bool lnclient_channel_is_master (lnclient_channel);

   void lnclient_channel_send (lnclient_channel,
                               long subchannel,
                               const char * buffer,
                               size_t size,
                               long variant);

   void lnclient_channel_blast (lnclient_channel,
                                long subchannel,
                                const char * buffer,
                                size_t size,
                                long variant);


/* lnclient_peer: represents another client on the same channel
 */
typedef struct _lnclient_peer * lnclient_peer;

   long lnclient_channel_num_peers (lnclient_channel);

   lnclient_peer lnclient_peer_first (lnclient_channel);
   lnclient_peer lnclient_peer_next (lnclient_peer);

   void lnclient_channel_leave (lnclient_channel);

   void lnclient_peer_set_tag (lnclient_peer, void *);
   void * lnclient_peer_tag (lnclient_peer);

   long lnclient_peer_id (lnclient_peer);
   const char * lnclient_peer_name (lnclient_peer);

   lnclient_channel lnclient_peer_channel (lnclient_peer);
   lw_bool lnclient_peer_is_master (lnclient_peer);

   void lnclient_peer_send (lnclient_peer,
                            long subchannel,
                            const char * buffer,
                            size_t size,
                            long variant);

   void lnclient_peer_blast (lnclient_peer,
                             long subchannel,
                             const char * buffer, 
                             size_t size,
                             long variant);


/* lnclient_channel_listing: an item in the channel list, with information
 * about the channel (even prior to joining)
 */
typedef struct _lnclient_channel_listing
{
   const char * name;
   size_t num_peers;

} * lnclient_channel_listing;


/* lnclient_config: passed to lnclient_new to configure callbacks etc.
 */
typedef struct _lnclient_config
{
   void (* on_error) (lnclient, lw_error);

   void (* on_connect) (lnclient, const char * welcome_message);
   void (* on_connection_denied) (lnclient, const char * deny_reason);
   void (* on_disconnect) (lnclient);

   void (* on_server_message)  /* server -> client */
              (lnclient, lw_bool blasted, long subchannel,
               const char * buffer, size_t size, long variant);

   void (* on_channel_message)  /* peer/server -> channel */
              (lnclient, lnclient_channel, lnclient_peer,
               lw_bool blasted, long subchannel, const char * buffer,
               size_t size, long variant);

   void (* on_peer_message)  /* peer -> peer */
              (lnclient, lnclient_channel, lnclient_peer,
               lw_bool blasted, long subchannel, const char * buffer,
               size_t size, long variant);

   void (* on_join) (lnclient, lnclient_channel);

   void (* on_join_denied) (lnclient, const char * channel_name,
                                      const char * deny_reason);

   void (* on_leave) (lnclient, lnclient_channel);
   void (* on_leave_denied) (lnclient, lnclient_channel);

   void (* on_name_set) (lnclient, const char * old_name);

   void (* on_name_denied)
      (lnclient, const char * name, const char * deny_reason);

   void (* on_peer_connect)
      (lnclient, lnclient_channel, lnclient_peer);

   void (* on_peer_disconnect)
      (lnclient, lnclient_channel, lnclient_peer);

   void (* on_peer_change_name)
            (lnclient, lnclient_channel,
             lnclient_peer, const char * old_name);

   void (* on_channel_list)
            (lnclient, size_t num_channels, lnclient_channel_listing **);

} lnclient_config;

lnclient lnclient_new (lw_pump, struct _lnclient_config);
void lnclient_delete (lnclient);

void lnclient_set_tag (lnclient, void *);
void * lnclient_tag (lnclient);

void lnclient_connect (lnclient, const char * host, long port);
void lnclient_connect_addr (lnclient, lw_addr);

lw_bool lnclient_connecting (lnclient);
lw_bool lnclient_connected (lnclient);

void lnclient_disconnect (lnclient);

lw_addr lnclient_server_addr (lnclient);

long lnclient_id (lnclient);

void lnclient_set_name (lnclient, const char * name);
const char * lnclient_name (lnclient);

void lnclient_join (lnclient,
                    const char * channel,
                    lw_bool hide_from_list,
                    lw_bool auto_close);

void lnclient_request_channel_list (lnclient);

#if defined(__cplusplus) && !defined(_MSC_VER)
}
#endif

#endif

