
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
#include "client.h"

#include <assert.h>

/* lnclient is a stream, and the socket (lw_client) gets written to it.
 *
 * This is done rather than using an on_data hook to take advantage of the
 * buffering provided by lw_stream (i.e. sink_data does not have to consume
 * everything, and we can tell it to retry when more data arrives)
 */

const static lw_streamdef client_streamdef;

static void on_socket_error (lw_client socket, lw_error error)
{
   lnclient ctx = lw_stream_tag (socket);

   if (ctx->config.on_error)
      ctx->config.on_error (ctx, error);
}

static void on_socket_connect (lw_client socket)
{
   lnclient ctx = lw_stream_tag (socket);

   lw_stream_write (socket, "\0", 1);

   lw_udp_host_addr (ctx->udp, lw_client_server_addr (ctx->socket));

   ctx->udp_addr = lw_addr_clone (lw_client_server_addr (ctx->socket));
   lw_addr_set_type (ctx->udp_addr, lw_addr_type_udp);

   lnclient_send_request_connect (ctx);

   lnet_trace ("Connected to %s\n",
               lw_addr_tostring (lw_client_server_addr (socket)));
}

static void on_socket_disconnect (lw_client socket)
{
   lnclient ctx = lw_stream_tag (socket);

   ctx->flags &= ~ LNCLIENT_FLAG_GOT_WELCOME;
}

static void on_udp_error (lw_udp udp, lw_error error)
{ 
   lnclient ctx = lw_udp_tag (udp);

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

   lnclient ctx = lw_udp_tag (udp);

   lw_ui8 type = *buffer;
   type >>= 4;

   lw_ui8 variant = *buffer;
   variant <<= 4;
   variant >>= 4;

   struct _lnet_buffer message;

   message.ptr = buffer;
   message.length = length;

   int res = LNET_E_OK;

   switch (type)
   {
   case LNET_MESSAGE_SC_UDP_WELCOME:
      res = lnclient_proc_udp_welcome (ctx, variant, &message);
      break;

   case LNET_MESSAGE_SC_SERVER_MESSAGE:
      res = lnclient_proc_server_message (ctx, lw_true, variant, &message);
      break;

   case LNET_MESSAGE_SC_CHANNEL_MESSAGE:
      res = lnclient_proc_channel_message (ctx, lw_true, variant, &message);
      break;

   case LNET_MESSAGE_SC_PEER_MESSAGE:
      res = lnclient_proc_peer_message (ctx, lw_true, variant, &message);
      break;

   case LNET_MESSAGE_SC_SERVER_CHANNEL_MESSAGE:
      res = lnclient_proc_server_channel_message (ctx, lw_true, variant, &message);
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

static void on_timer_tick (lw_timer timer)
{
   lnclient ctx = lw_timer_tag (timer);

   lnet_message_blast (ctx->udp,
                       ctx->udp_addr,
                       ctx->id,
                       LNET_MESSAGE_CS_UDP_HELLO,
                       0, /* variant */
                       0);
}

lnclient lnclient_new (lw_pump pump, struct _lnclient_config config)
{
   lw_stream stream = lw_stream_new (&client_streamdef, pump);

   if (!stream)
      return 0;

   lnclient ctx = lw_stream_tail (stream);

   memcpy (&ctx->config, &config, sizeof (config));

   ctx->socket = lw_client_new (pump);
   lw_stream_set_tag (ctx->socket, ctx);
   lw_client_on_error (ctx->socket, on_socket_error);
   lw_client_on_connect (ctx->socket, on_socket_connect);
   lw_client_on_disconnect (ctx->socket, on_socket_disconnect);

   ctx->udp = lw_udp_new (pump);
   lw_udp_set_tag (ctx->udp, ctx);
   lw_udp_on_error (ctx->udp, on_udp_error);
   lw_udp_on_data (ctx->udp, on_udp_data);

   ctx->timer = lw_timer_new (pump);
   lw_timer_set_tag (ctx->timer, ctx);
   lw_timer_on_tick (ctx->timer, on_timer_tick);

   lnet_message_next (&ctx->message);

   lw_stream_write_stream (stream, ctx->socket, -1, lw_false);
   lw_stream_retry (stream, lw_stream_retry_more_data);

   return ctx;
}

void lnclient_delete (lnclient ctx)
{
   lw_stream stream;

   if (!ctx)
       return;

   stream = lw_stream_from_tail (ctx);

   lw_timer_delete (ctx->timer);
   lw_stream_delete (ctx->socket);
   lw_udp_delete (ctx->udp);
   
   lw_stream_delete (stream);
}

lw_stream lnclient_stream (lnclient ctx)
{
   return (lw_stream) (ctx - 1);
}

void lnclient_set_tag (lnclient client, void * tag)
{
   client->tag = tag;
}

void * lnclient_tag (lnclient client)
{
   return client->tag;
}

void lnclient_connect (lnclient ctx, const char * host, long port)
{
   lw_client_connect (ctx->socket, host, port);
}

lw_addr lnclient_server_addr (lnclient ctx)
{
   return lw_client_server_addr (ctx->socket);
}

void lnclient_set_name (lnclient ctx, const char * name)
{
   lnclient_send_request_set_name (ctx, name);
}

const char * lnclient_name (lnclient ctx)
{
   return ctx->name;
}

void lnclient_join (lnclient ctx, const char * name,
                    lw_bool hide_from_list, lw_bool auto_close)
{
   lw_i8 join_flags = 0;

   if (hide_from_list)
      join_flags |= LNET_CHANNEL_FLAG_HIDE;

   if (auto_close)
      join_flags |= LNET_CHANNEL_FLAG_AUTO_CLOSE;

   lnclient_send_request_join_channel (ctx, name, join_flags);
}

static size_t client_sink_data (lw_stream stream,
                                const char * buf,
                                size_t length)
{
   int res, status;
   struct _lnet_buffer buffer;

   lnclient ctx = lw_stream_tail (stream);

   buffer.ptr = buf;
   buffer.length = length;

   while (buffer.length > 0)
   {
      if ((res = lnet_message_read (&ctx->message,
                                    &buffer,
                                    &status)) != LNET_E_OK)
      {
          goto error;
      }

      if (status != LNET_MESSAGE_STATUS_READY)
         break;

      if (buffer.length < ctx->message.length)
         break;

      buffer.ptr = buffer.ptr;
      buffer.length = ctx->message.length;
      
      switch (ctx->message.type)
      {
      case LNET_MESSAGE_SC_RESPONSE:
         res = lnclient_proc_response
            (ctx, ctx->message.variant, &buffer);
         break;

      case LNET_MESSAGE_SC_SERVER_MESSAGE:
         res = lnclient_proc_server_message
            (ctx, lw_false, ctx->message.variant, &buffer);
         break;

      case LNET_MESSAGE_SC_CHANNEL_MESSAGE:
         return lnclient_proc_channel_message
            (ctx, lw_false, ctx->message.variant, &buffer);
         break;

      case LNET_MESSAGE_SC_PEER_MESSAGE:
         return lnclient_proc_peer_message
            (ctx, lw_false, ctx->message.variant, &buffer);
         break;

      case LNET_MESSAGE_SC_UPDATE_PEER:
         return lnclient_proc_update_peer
            (ctx, ctx->message.variant, &buffer);
         break;

      case LNET_MESSAGE_SC_SERVER_CHANNEL_MESSAGE:
         return lnclient_proc_server_channel_message
            (ctx, lw_false, ctx->message.variant, &buffer);
         break;
      };

      if (res == LNET_E_OK && buffer.length > 0)
      {
         res = LNET_E_PROTOCOL; /* unprocessed data left over */
         goto error;
      }

      lnet_message_next (&ctx->message);
   }

   return (length - buffer.length);

error:
   
   if (ctx->config.on_error)
   {
      lw_error error = lw_error_new ();
      lw_error_addf (error, "Protocol error"); 

      ctx->config.on_error (ctx, error);

      lw_error_delete (error);
   }

   return (length - buffer.length);
}

const static lw_streamdef client_streamdef =
{
   client_sink_data,
   0, /* sink_stream */
   0, /* retry */
   0, /* is_transparent */
   0, /* close */
   0, /* bytes_left */
   0, /* read */
   0, /* cleanup */
   sizeof (struct _lnclient) /* tail size */
};

