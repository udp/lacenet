
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

int lnclient_proc_request_join_channel (lnserver ctx,
                                        lnserver_client client,
                                        lw_bool success,
                                        lnet_buffer buffer)
{
   if (!*client->name)
   {
      lnserver_send_failure_join_channel
         (ctx, client, "Name must be set before joining a channel");

      return LNET_E_OK;
   }

   char * channel_name = lnet_buffer_sstring (buffer, lw_true);

   if (!channel_name)
      return LNET_E_MEM;

   lw_i8 create_flags = lnet_buffer_int8 (buffer);

   if (!*channel_name)
   {
      lnserver_send_failure_join_channel
         (ctx, client, channel_name, "Invalid channel name");

      return LNET_E_OK;
   }

   lnserver_channel channel = 0;

   HASH_FIND (hh, ctx->channels, channel_name, strlen (channel_name), channel);

   if (channel)
   {
      /* Joining an existing channel.  Is our name already taken here? */

      list_each (channel->members, channel_member)
      {
         if (!strcasecmp (channel_member->name, client->name))
         {
            lnserver_send_failure_join_channel
               (ctx, client, channel_name, "Name already taken");

            return LNET_E_OK;
         }
      }

      list_push (channel->members, client);

      lnserver_send_success_join_channel (ctx, client, channel->name);

      return LNET_E_OK;
   }

   /* Creating a channel */

   if (! (channel = lnserver_channel_new (ctx, channel_name)))
      return LNET_E_MEM;

   channel->master = client;
   channel->flags = create_flags;

   list_push (channel->members, client);

   HASH_ADD_KEYPTR (hh, ctx->channels,
                    channel->name, strlen (channel->name),
                    channel);

   lnserver_send_success_join_channel (ctx, client, channel->name);

   return LNET_E_OK;
}

void lnserver_send_success_join_channel (lnserver ctx,
                                         lnserver_client client,
                                         lw_i16 channel_id,
                                         const char * channel_name)
{
   channel_id = lnet_htons (channel_id);

   lw_i8 request_type = LNET_REQUEST_SET_NAME;
   lw_i8 success = 1;

   lnet_message_send (client->socket,
                      LNET_MESSAGE_SC_RESPONSE,
                      0, /* variant */
                      4,
                      &request_type, sizeof (request_type),
                      &success, sizeof (success),
                      &channel_id, sizeof (channel_id),
                      name, strlen (name));
}

void lnserver_send_failure_join_channel (lnserver ctx,
                                         lnclient client,
                                         const char * channel_name,
                                         const char * deny_reason)
{
   lw_i8 request_type = LNET_REQUEST_SET_NAME;
   lw_i8 success = 0;

   lw_i16 channel_name_length = lnet_htons (strlen (channel_name));

   lnet_message_send (client->socket,
                      LNET_MESSAGE_SC_RESPONSE,
                      0, /* variant */
                      4,
                      &request_type, sizeof (request_type),
                      &success, sizeof (success),
                      &channel_name_length, sizeof (channel_name_length),
                      channel_name, strlen (channel_name),
                      deny_reason, strlen (deny_reason));
}

