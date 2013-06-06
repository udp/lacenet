
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

int lnclient_proc_request_set_name (lnserver ctx,
                                    lnclient client,
                                    lw_bool success,
                                    lnet_buffer buffer)
{
   char * name = lnet_buffer_sstring (buffer, lw_true);

   if (!name)
      return LNET_E_MEM;

   if (!*name)
   {
      lnserver_send_failure_set_name (ctx, client, "Invalid name");
      return LNET_E_OK;
   }

   list_each (client->channels, channel)
   {
      list_each (channel->members, channel_member)
      {
         if (!strcasecmp (channel_member->name, name))
         {
            lnserver_send_failure_set_name (ctx, client, "Name already taken");
            return LNET_E_OK;
         }
      }
   }

   lnserver_send_success_set_name (ctx, client, name);

   return LNET_E_OK;
}

void lnserver_send_success_set_name (lnserver ctx,
                                     lnclient client,
                                     const char * name)
{
   lw_i8 request_type = LNET_REQUEST_SET_NAME;
   lw_i8 success = 1;

   lnet_message_send (client->socket,
                      LNET_MESSAGE_SC_RESPONSE,
                      0, /* variant */
                      3,
                      &request_type, sizeof (request_type),
                      &success, sizeof (success),
                      name, strlen (name));
}

void lnserver_send_failure_set_name (lnserver ctx,
                                     lnclient client,
                                     const char * deny_reason)
{
   lw_i8 request_type = LNET_REQUEST_SET_NAME;
   lw_i8 success = 0;

   lnet_message_send (client->socket,
                      LNET_MESSAGE_SC_RESPONSE,
                      0, /* variant */
                      3,
                      &request_type, sizeof (request_type),
                      &success, sizeof (success),
                      deny_reason, strlen (deny_reason));
}



