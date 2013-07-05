
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

#include "../server.h"

int lnserver_proc_request_connect (lnserver ctx,
                                   lnserver_client client,
                                   lnet_buffer buffer)
{

   return LNET_E_OK;
}

void lnserver_send_success_connect (lnserver ctx,
                                    lnserver_client client,
                                    lw_i16 peer_id,
                                    const char * welcome_message)
{
   lw_i8 request_type = LNET_REQUEST_CONNECT;
   lw_i8 success = 1;

   lnet_message_send (client->socket,
                      LNET_MESSAGE_SC_RESPONSE,
                      0, /* variant */
                      4,
                      &request_type, sizeof (request_type),
                      &success, sizeof (success),
                      &peer_id, sizeof (peer_id),
                      welcome_message, strlen (welcome_message));
}

void lnserver_send_failure_connect (lnserver ctx,
                                    lnserver_client client,
                                    const char * deny_reason)
{
   lw_i8 request_type = LNET_REQUEST_CONNECT;
   lw_i8 success = 0;

   lnet_message_send (client->socket,
                      LNET_MESSAGE_SC_RESPONSE,
                      0, /* variant */
                      3,
                      &request_type, sizeof (request_type),
                      &success, sizeof (success),
                      deny_reason, strlen (deny_reason));
}

