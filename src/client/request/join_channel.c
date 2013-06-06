
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

#include "../client.h"

void lnclient_send_request_join_channel (lnclient ctx,
                                         const char * channel_name,
                                         lw_i8 join_flags)
{
   lw_i8 request_type = LNET_REQUEST_JOIN_CHANNEL;

   lnet_message_send (ctx->socket,
                      LNET_MESSAGE_CS_REQUEST,
                      0, /* variant */
                      3,
                      &request_type, sizeof (request_type),
                      &join_flags, sizeof (join_flags),
                      channel_name, strlen (channel_name));
}

int lnclient_proc_response_join_channel (lnclient ctx,
                                         lw_bool success,
                                         lnet_buffer buffer)
{
   if (buffer->length < 2)
      return LNET_E_PROTOCOL;

   lw_i8 flags = lnet_buffer_int8 (buffer);

   char * channel_name = lnet_buffer_sstring (buffer, lw_true);

   if (!channel_name)
      return LNET_E_MEM;

   if (!success)
   {
      char * deny_reason = lnet_buffer_string (buffer, lw_true);

      if (!deny_reason)
      {
         free (channel_name);
         return LNET_E_MEM;
      }

      if (ctx->config.on_join_denied)
         ctx->config.on_join_denied (ctx, channel_name, deny_reason);

      free (channel_name);
      free (deny_reason);

      return LNET_E_OK;
   }

   if (buffer->length < 2)
   {
      free (channel_name);
      return LNET_E_PROTOCOL;
   }

   struct _lnclient_channel _channel = {};
   list_push (ctx->channels, _channel);

   lnclient_channel channel = list_elem_back (ctx->channels);

   channel->flags = flags;
   channel->id = lnet_buffer_int16 (buffer);
   channel->name = channel_name;

   lw_dump (buffer->ptr, buffer->length);

   while (buffer->length > 0)
   {
      if (buffer->length < 5)
      {
         lnet_trace ("Bad peer list: %d bytes left over", (int) buffer->length);

         list_elem_remove (channel);
         return LNET_E_PROTOCOL; /* bad peer list */
      }

      struct _lnclient_peer peer = {};

      peer.channel = list_elem_back (ctx->channels);

      peer.id = lnet_buffer_int16 (buffer);

      if (lnet_buffer_int8 (buffer) & 1)  /* flags */
         channel->master = list_elem_back (channel->peers);

      peer.name = lnet_buffer_sstring (buffer, lw_true);

      list_push (channel->peers, peer);
   }

   if (ctx->config.on_join)
      ctx->config.on_join (ctx, channel);

   return LNET_E_OK;
}

