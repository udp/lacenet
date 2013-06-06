
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

void lnclient_send_request_set_name (lnclient ctx, const char * name)
{
   lw_i8 request_type = LNET_REQUEST_SET_NAME;

   lnet_message_send (ctx->socket,
                      LNET_MESSAGE_CS_REQUEST,
                      0, /* variant */
                      2,
                      &request_type, sizeof (request_type),
                      name, strlen (name));
}

int lnclient_proc_response_set_name (lnclient ctx,
                                     lw_bool success,
                                     lnet_buffer buffer)
{
   char * name = lnet_buffer_sstring (buffer, lw_true);

   if (!name)
      return LNET_E_MEM;

   if (success)
   {
      char * old_name = ctx->name;
      ctx->name = name;

      if (ctx->config.on_name_set)
         ctx->config.on_name_set (ctx, old_name);

      free (old_name);
   }
   else
   {
      char * deny_reason = lnet_buffer_string (buffer, lw_true);

      if (!deny_reason)
      {
         free (name);
         return LNET_E_MEM;
      }

      if (ctx->config.on_name_denied)
         ctx->config.on_name_denied (ctx, name, deny_reason);

      free (name);
      free (deny_reason);
   }

   return LNET_E_OK;
}


