
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

#include "../common.h"
#include "client.h"

int lnclient_proc_server_message (lnclient ctx,
                                  lw_bool blasted,
                                  int variant,
                                  lnet_buffer buffer)
{
   if (buffer->length < 1)
      return LNET_E_PROTOCOL;

   long subchannel = lnet_buffer_int8 (buffer);

   if (ctx->config.on_server_message)
   {
      ctx->config.on_server_message
         (ctx, blasted, subchannel, buffer->ptr, buffer->length, variant);
   }

   return LNET_E_OK;
}

int lnclient_proc_channel_message (lnclient ctx,
                                   lw_bool blasted,
                                   int variant,
                                   lnet_buffer buffer)
{
   if (buffer->length < 5)
      return LNET_E_PROTOCOL; /* bad channel message */

   long subchannel = lnet_buffer_int8 (buffer);
   
   lnclient_channel channel = lnclient_read_channel (ctx, buffer);

   if (!channel)
      return LNET_E_OK;  /* unknown channel */

   long peer_id = lnet_buffer_int16 (buffer);

   lnclient_peer peer = lnclient_channel_find_peer (channel, peer_id);

   if (!peer)
      return LNET_E_OK;  /* unknown peer */

   if (ctx->config.on_channel_message)
   {
      ctx->config.on_channel_message
         (ctx, channel, peer, blasted, subchannel, buffer->ptr, buffer->length, variant);
   }

   return LNET_E_OK;
}

int lnclient_proc_peer_message (lnclient ctx,
                                lw_bool blasted,
                                int variant,
                                lnet_buffer buffer)
{
   if (buffer->length < 5)
      return LNET_E_PROTOCOL; /* bad peer message */

   long subchannel = lnet_buffer_int8 (buffer);
   
   lnclient_channel channel = lnclient_read_channel (ctx, buffer);

   if (!channel)
      return LNET_E_OK;  /* unknown channel */

   lw_i16 peer_id = lnet_buffer_int16 (buffer);

   if (!peer_id)
      return LNET_E_PROTOCOL; /* invalid peer ID */

   lnclient_peer peer = lnclient_channel_find_peer (channel, peer_id);

   if (!peer)
      return LNET_E_OK;  /* unknown peer */

   if (ctx->config.on_peer_message)
   {
      ctx->config.on_peer_message
         (ctx, channel, peer, blasted, subchannel, buffer->ptr, buffer->length, variant);
   }

   return LNET_E_OK;
}

int lnclient_proc_server_channel_message (lnclient ctx,
                                          lw_bool blasted,
                                          int variant,
                                          lnet_buffer buffer)
{
   if (buffer->length < 3)
      return LNET_E_PROTOCOL; /* bad server channel message */

   long subchannel = lnet_buffer_int8 (buffer);
   
   lnclient_channel channel = lnclient_read_channel (ctx, buffer);

   if (!channel)
      return LNET_E_OK;  /* unknown channel */

   if (ctx->config.on_channel_message)
   {
      ctx->config.on_channel_message
         (ctx, channel, 0, blasted, subchannel, buffer->ptr, buffer->length, variant);
   }

   return LNET_E_OK;
}

int lnclient_proc_update_peer (lnclient ctx,
                               int variant,
                               lnet_buffer buffer)
{
   if (buffer->length < 5)
      return LNET_E_PROTOCOL; /* bad update peer message */

   lnclient_channel channel = lnclient_read_channel (ctx, buffer);

   if (!channel)
      return LNET_E_OK;  /* unknown channel */

   lw_i16 peer_id = lnet_buffer_int16 (buffer);

   lnclient_peer peer = lnclient_channel_find_peer (channel, peer_id);

   if (!peer)
      return LNET_E_OK;  /* unknown peer */

   if (buffer->length == 0)
   {
      /* No flags/name - the peer must have left the channel */

      if (!peer)
         return LNET_E_OK;

      if (ctx->config.on_peer_disconnect)
         ctx->config.on_peer_disconnect (ctx, channel, peer);

      return LNET_E_OK;
   }

   lw_i8 flags = lnet_buffer_int8 (buffer);
   char * name = lnet_buffer_string (buffer, lw_true);

   if (peer)
   {
      /* The peer already exists; changed name? */

      if (strcmp (peer->name, name))
      {
         char * old_name = peer->name;
         peer->name = name;

         if (ctx->config.on_peer_change_name)
            ctx->config.on_peer_change_name (ctx, channel, peer, old_name);

         free (old_name);
      }
      else
      {
         free (name);
      }

      return LNET_E_OK;
   }

   /* New peer */

   struct _lnclient_peer new_peer = {};

   new_peer.name = name;
   new_peer.flags = flags;

   list_push (channel->peers, new_peer);

   return LNET_E_OK;
}

int lnclient_proc_udp_welcome (lnclient ctx,
                               int variant,
                               lnet_buffer buffer)
{
   if (ctx->flags & LNCLIENT_FLAG_GOT_WELCOME)
      return LNET_E_OK;

   ctx->flags |= LNCLIENT_FLAG_GOT_WELCOME;

   lw_timer_stop (ctx->timer);

   if (ctx->config.on_connect)
      ctx->config.on_connect (ctx, ctx->welcome_message);

   return LNET_E_OK;
}

int lnclient_proc_response (lnclient ctx,
                            int variant,
                            lnet_buffer buffer)
{
   if (buffer->length < 2)
      return LNET_E_PROTOCOL; /* bad response */
 
   lw_i8 type = lnet_buffer_int8 (buffer);
   lw_bool success = lnet_buffer_int8 (buffer) != 0;

   switch (type)
   {
      case LNET_RESPONSE_CONNECT:
         return lnclient_proc_response_connect (ctx, success, buffer);

      case LNET_RESPONSE_SET_NAME:
         return lnclient_proc_response_set_name (ctx, success, buffer);

      case LNET_RESPONSE_JOIN_CHANNEL:
         return lnclient_proc_response_join_channel (ctx, success, buffer);

      case LNET_RESPONSE_LEAVE_CHANNEL:
         return lnclient_proc_response_leave_channel (ctx, success, buffer);

      case LNET_RESPONSE_CHANNEL_LIST:
         return lnclient_proc_response_channel_list (ctx, success, buffer);
   };

   return LNET_E_OK;
}

