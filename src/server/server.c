
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

#include "../message.h"
#include "server.h"

#include <assert.h>

static void on_socket_error (lw_server server, lw_error error)
{
   lnserver ctx = lw_server_tag (server);

   if (ctx->config.on_error)
      ctx->config.on_error (ctx, error);
}

static void on_socket_connect (lw_server server, lw_server_client client)
{
   lnserver ctx = lw_server_tag (server);

   lnserver_client client_ctx = lnserver_client_new (ctx, client);

   if (!client_ctx)
   {
      /* failed to allocate client context */

      lw_stream_close (client, lw_true);
      return;
   }

   lw_stream_set_tag (client, client_ctx);
}

static void on_socket_disconnect (lw_server server, lw_server_client socket)
{
   lnserver ctx = lw_stream_tag (socket);

   lnserver_client client = lw_stream_tag (socket);

   if (!client)
      return;  /* client had no context */

   /* Only call the on_disconnect callback if we already called on_connect */

   if (client->flags & LNSERVER_CLIENT_FLAG_CALLED_ON_CONNECT)
   {
      if (ctx->config.on_disconnect)
         ctx->config.on_disconnect (ctx, client);
   }

   lnserver_client_delete (client);
}

static void on_udp_error (lw_udp udp, lw_error error)
{ 
   lnserver ctx = lw_udp_tag (udp);

   if (ctx->config.on_error)
      ctx->config.on_error (ctx, error);
}

static void on_udp_data (lw_udp udp,
                         lw_addr addr,
                         const char * buffer,
                         size_t length)
{
   if (length < 1)
      return;

   lnserver ctx = lw_udp_tag (udp);

   lw_ui8 type = *buffer;
   type >>= 4;

   lw_ui8 variant = *buffer;
   variant <<= 4;
   variant >>= 4;

   struct _lnet_buffer message;

   message.ptr = buffer;
   message.length = length;

   lnserver_client client = lnserver_read_client (ctx, &message);

   if (!client)
      return;  /* bad client ID */

   /* TODO: check client addr */

   int res = LNET_E_OK;

   switch (type)
   {
   case LNET_MESSAGE_CS_UDP_HELLO:
      res = lnserver_proc_udp_hello (ctx, client, variant, &message);
      break;

   case LNET_MESSAGE_CS_SERVER_MESSAGE:
      res = lnserver_proc_server_message (ctx, client, lw_true, variant, &message);
      break;

   case LNET_MESSAGE_CS_CHANNEL_MESSAGE:
      res = lnserver_proc_channel_message (ctx, client, lw_true, variant, &message);
      break;

   case LNET_MESSAGE_CS_PEER_MESSAGE:
      res = lnserver_proc_peer_message (ctx, client, lw_true, variant, &message);
      break;

   default:
      res = LNET_E_PROTOCOL;
      break;
   };

   if (res != LNET_E_OK)
   {
      /* error in UDP message - ignore silently */
   }
}

lnserver lnserver_new (lw_pump pump, struct _lnserver_config config)
{
   lnserver ctx = (lnserver) calloc (sizeof (*ctx), 1);

   if (!ctx)
      return 0;

   if (! (ctx->server = lw_server_new (pump)))
   {
      free (ctx);
      return 0;
   }

   lw_server_on_error (ctx->server, on_socket_error);
   lw_server_on_connect (ctx->server, on_socket_connect);
   lw_server_on_disconnect (ctx->server, on_socket_disconnect);

   if (! (ctx->udp = lw_udp_new (pump)))
   {
      lw_server_delete (ctx->server);

      free (ctx);
      return 0;
   }

   lw_udp_on_error (ctx->udp, on_udp_error);
   lw_udp_on_data (ctx->udp, on_udp_data);

   memcpy (&ctx->config, &config, sizeof (config));

   return ctx;
}

void lnserver_delete (lnserver ctx)
{
   free (ctx);
}

void lnserver_host (lnserver ctx, long port)
{
   lw_server_host (ctx->server, port);
}

void lnserver_host_filter (lnserver ctx, lw_filter filter)
{
   lw_server_host_filter (ctx->server, filter);
}

