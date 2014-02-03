
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

#include "client.h"

void lnclient_peer_set_tag (lnclient_peer peer, void * tag)
{
   peer->tag = tag;
}

void * lnclient_peer_tag (lnclient_peer peer)
{
   return peer->tag;
}

lnclient_channel lnclient_peer_channel (lnclient_peer peer)
{
   return peer->channel;
}

const char * lnclient_peer_name (lnclient_peer peer)
{
   return peer->name;
}

long lnclient_peer_id (lnclient_peer peer)
{
   return peer->id;
}

lnclient_peer lnclient_peer_next (lnclient_peer peer)
{
   return list_elem_next (peer);
}

void lnclient_peer_send (lnclient_peer peer,
                         long _subchannel,
                         const char * buffer,
                         size_t size,
                         long variant)
{
   lnclient_channel channel = peer->channel;
   lnclient ctx = channel->ctx;

   lw_i16 channel_id = lnet_htons (channel->id);
   lw_i16 peer_id = lnet_htons (peer->id);
   lw_i8 subchannel = (lw_i8) _subchannel;

   lnet_message_send (ctx->socket,
                      LNET_MESSAGE_CS_CHANNEL_MESSAGE,
                      variant, 
                      4,
                      &subchannel, sizeof (subchannel),
                      &channel_id, sizeof (channel_id),
                      &peer_id, sizeof (peer_id),
                      buffer, size);
}


void lnclient_peer_blast (lnclient_peer peer,
                          long _subchannel,
                          const char * buffer,
                          size_t size,
                          long variant)
{
   lnclient_channel channel = peer->channel;
   lnclient ctx = channel->ctx;

   lw_i16 channel_id = lnet_htons (channel->id);
   lw_i16 peer_id = lnet_htons (peer->id);
   lw_i8 subchannel = (lw_i8) _subchannel;

   lnet_message_blast (ctx->udp,
                       ctx->udp_addr,
                       ctx->id,
                       LNET_MESSAGE_CS_CHANNEL_MESSAGE,
                       variant, 
                       4,
                       &subchannel, sizeof (subchannel),
                       &channel_id, sizeof (channel_id),
                       &peer_id, sizeof (peer_id),
                       buffer, size);
}

lw_bool lnclient_peer_is_master (lnclient_peer peer)
{
   return peer->channel->master == peer;
}


