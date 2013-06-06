
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

void lnclient_channel_set_tag (lnclient_channel channel, void * tag)
{
   channel->tag = tag;
}

void * lnclient_channel_tag (lnclient_channel channel)
{
   return channel->tag;
}

lnclient_channel lnclient_read_channel (lnclient ctx, lnet_buffer buffer)
{
   if (buffer->length < 2)
      return 0;

   lw_i16 channel_id = lnet_buffer_int16 (buffer);

   list_each_elem (ctx->channels, channel)
   {
      if (channel->id == channel_id)
         return channel;
   }

   return 0;
}

long lnclient_channel_num_peers (lnclient_channel channel)
{
   return list_length (channel->peers);
}

lnclient_peer lnclient_peer_first (lnclient_channel channel)
{
   return list_elem_front (channel->peers);
}

lnclient_peer lnclient_channel_find_peer (lnclient_channel channel, long peer_id)
{
   list_each_elem (channel->peers, peer)
   {
      if (peer->id == peer_id)
         return peer;
   }
   
   return 0;
}

const char * lnclient_channel_name (lnclient_channel channel)
{
   return channel->name;
}


