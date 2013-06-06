
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

#include "server.h"

int lnserver_proc_request (lnserver ctx,
                           lnserver_client client,
                           int variant,
                           lnet_buffer buffer)
{
   if (buffer->length < 2)
      return LNET_E_PROTOCOL; /* bad request */
 
   lw_i8 type = lnet_buffer_int8 (buffer);

   switch (type)
   {
      case LNET_REQUEST_CONNECT:
         return lnserver_proc_request_connect (ctx, buffer);

      case LNET_REQUEST_SET_NAME:
         return lnserver_proc_request_set_name (ctx, buffer);

      case LNET_REQUEST_JOIN_CHANNEL:
         return lnserver_proc_request_join_channel (ctx, buffer);

      case LNET_REQUEST_LEAVE_CHANNEL:
         return lnserver_proc_request_leave_channel (ctx, buffer);

      case LNET_REQUEST_CHANNEL_LIST:
         return lnserver_proc_request_channel_list (ctx, buffer);
   };

   return LNET_E_OK;
}

int lnserver_proc_server_message (lnserver ctx,
                                  lnserver_client client,
                                  lw_bool blasted,
                                  int variant,
                                  lnet_buffer buffer)
{
   if (buffer->length < 1)
      return LNET_E_PROTOCOL;

   lw_i8 subchannel = lnet_buffer_int8 (buffer);

   if (ctx->config.on_server_message)
   {
      ctx->config.on_server_message
         (ctx, client, blasted, subchannel, buffer->ptr, buffer->length, variant);
   }

   return LNET_E_OK;
}
         
int lnserver_proc_channel_message (lnserver ctx,
                                   lnserver_client client,
                                   lw_bool blasted,
                                   int variant,
                                   lnet_buffer buffer)
{
   if (buffer->length < 3)
      return LNET_E_PROTOCOL;

   lw_i8 subchannel = lnet_buffer_int8 (buffer);
   
   lnserver_channel channel = lnserver_client_read_channel (client, buffer);

   if (!channel)
      return LNET_E_OK;  /* unknown channel */

   if (ctx->config.on_channel_message)
   {
      if (!ctx->config.on_channel_message
            (ctx, client, channel, blasted, subchannel, buffer->ptr, buffer->length, variant))
      {
         return LNET_E_OK;  /* message relay canceled */
      }
   }

   list_each (channel->members, channel_member)
   {
      if (channel_member == client)
         continue;  /* don't relay back to the sender */

      lw_i16 channel_id = lnet_htons (channel->id),
             client_id = lnet_htons (client->id), 
             target_client_id = lnet_htons (channel_member->id);

      if (blasted)
      {
         lnet_message_blast (ctx->udp,
                             channel_member->udp_addr,
                             target_client_id,
                             LNET_MESSAGE_SC_CHANNEL_MESSAGE,
                             variant,
                             3,
                             &channel_id, sizeof (channel_id),
                             &client_id, sizeof (client_id),
                             buffer->ptr, buffer->length);
      }
      else
      {
         lnet_message_send (channel_member->socket,
                            LNET_MESSAGE_SC_CHANNEL_MESSAGE,
                            variant,
                            3,
                            &channel_id, sizeof (channel_id),
                            &client_id, sizeof (client_id),
                            buffer->ptr, buffer->length);
      }
   }

   return LNET_E_OK;
}

int lnserver_proc_peer_message (lnserver ctx,
                                lnserver_client client,
                                lw_bool blasted,
                                int variant,
                                lnet_buffer buffer)
{
   if (buffer->length < 3)
      return LNET_E_PROTOCOL;

   lw_i8 subchannel = lnet_buffer_int8 (buffer);
   
   lnserver_channel channel = lnserver_client_read_channel (client, buffer);

   if (!channel)
      return LNET_E_OK;  /* unknown channel */

   lnserver_client target_peer = lnserver_channel_read_member (channel, buffer);

   if ( (!target_peer) || target_peer == client)
      return LNET_E_OK;  /* unknown peer */

   if (ctx->config.on_peer_message)
   {
      if (!ctx->config.on_peer_message (ctx,
                                        client, target_peer,
                                        channel,
                                        blasted,
                                        subchannel,
                                        buffer->ptr, buffer->length,
                                        variant))
      {
         return LNET_E_OK;  /* message relay canceled */
      }
   }

   lw_i16 channel_id = lnet_htons (channel->id),
          client_id = lnet_htons (client->id);

   lnet_message_send (target_peer->socket,
                      LNET_MESSAGE_SC_PEER_MESSAGE,
                      variant,
                      3,
                      &channel_id, sizeof (channel_id),
                      &client_id, sizeof (client_id),
                      buffer->ptr, buffer->length);

   return LNET_E_OK;
}

int lnserver_proc_udp_hello (lnserver ctx,
                             lnserver_client client,
                             int variant,
                             lnet_buffer buffer)
{
   if (client->flags & LNSERVER_CLIENT_FLAG_SENT_WELCOME)
      return LNET_E_OK;  /* already sent welcome */

   lnet_message_blast (ctx->udp,
                       client->udp_addr,
                       client->id,
                       LNET_MESSAGE_SC_UDP_WELCOME,
                       0, /* variant */
                       0);

   client->flags |= LNSERVER_CLIENT_FLAG_SENT_WELCOME;

   return LNET_E_OK;
}

int lnserver_proc_channel_master (lnserver ctx,
                                  lnserver_client client,
                                  int variant,
                                  lnet_buffer buffer)
{
   /* TODO */

   return LNET_E_OK;
}

int lnserver_proc_pong (lnserver ctx,
                        lnserver_client client,
                        int variant,
                        lnet_buffer buffer)
{
   client->flags |= LNSERVER_CLIENT_FLAG_PONGED;

   return LNET_E_OK;
}

