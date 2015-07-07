
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

/* lnserver_client is a stream, and the socket (lw_server_client) gets written
 * to it.
 *
 * This is done rather than using an on_data hook to take advantage of the
 * buffering provided by lw_stream (i.e. sink_data does not have to consume
 * everything, and we can tell it to retry when more data arrives)
 */

static size_t client_sink_data(lw_stream stream, const char * buf, size_t length);

const static lw_streamdef server_client_streamdef =
{
   client_sink_data,
   0, /* sink_stream */
   0, /* retry */
   0, /* is_transparent */
   0, /* close */
   0, /* bytes_left */
   0, /* read */
   0, /* cleanup */
   sizeof(struct _lnserver_client) /* tail size */
};

lnserver_client lnserver_client_new (lnserver ctx, lw_server_client socket)
{
   lw_stream stream = lw_stream_new (&server_client_streamdef, ctx->pump);

   if (!stream)
      return 0;

   lnserver_client client = (lnserver_client) lw_stream_tail (stream);

   client->server = ctx;

   client->socket = socket;
   lw_stream_set_tag (socket, ctx);

   lnet_message_next (&client->message);

   lw_stream_write_stream (stream, socket, -1, lw_false);
   lw_stream_retry (stream, lw_stream_retry_more_data);

   return client;
}

void lnserver_client_delete (lnserver_client client)
{
   lw_stream_delete ((lw_stream) client);
}

static size_t client_sink_data (lw_stream stream,
                                const char * buf,
                                size_t length)
{
   int res, status;
   struct _lnet_buffer buffer;

   lnserver_client client = (lnserver_client) lw_stream_tail (stream);
   lnserver ctx = client->server;

   buffer.ptr = buf;
   buffer.length = length;

   while (buffer.length > 0)
   {
      if ((res = lnet_message_read (&client->message,
                                    &buffer,
                                    &status)) != LNET_E_OK)
      {
          goto error;
      }

      if (status != LNET_MESSAGE_STATUS_READY)
         break;

      if (buffer.length < client->message.length)
         break;

      buffer.ptr = buffer.ptr;
      buffer.length = client->message.length;
      
      switch (client->message.type)
      {
      case LNET_MESSAGE_CS_REQUEST:
         res = lnserver_proc_request
            (ctx, client, client->message.variant, &buffer);
         break;

      case LNET_MESSAGE_CS_SERVER_MESSAGE:
         res = lnserver_proc_server_message
            (ctx, client, lw_false, client->message.variant, &buffer);
         break;

      case LNET_MESSAGE_CS_CHANNEL_MESSAGE:
         res = lnserver_proc_channel_message
            (ctx, client, lw_false, client->message.variant, &buffer);
         break;

      case LNET_MESSAGE_CS_PEER_MESSAGE:
         res = lnserver_proc_peer_message
            (ctx, client, lw_false, client->message.variant, &buffer);
         break;

      case LNET_MESSAGE_CS_CHANNEL_MASTER:
         res = lnserver_proc_channel_master
            (ctx, client, client->message.variant, &buffer);
         break;

      case LNET_MESSAGE_CS_PONG:
         res = lnserver_proc_pong
            (ctx, client, client->message.variant, &buffer);
         break;
      };

      if (res == LNET_E_OK && buffer.length > 0)
      {
         res = LNET_E_PROTOCOL; /* unprocessed data left over */
         goto error;
      }

      lnet_message_next (&client->message);
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

lw_stream lnserver_client_stream (lnserver_client ctx)
{
   return (lw_stream) (ctx - 1);
}

lnserver_client lnserver_read_client (lnserver ctx, lnet_buffer buffer)
{
   if (buffer->length < 2)
      return 0;

   lw_i16 id = lnet_buffer_int16 (buffer);

   for (lw_client client_socket = lw_server_client_first (ctx->server);
        client_socket;
        client_socket = lw_server_client_next (client_socket))
   {
      lnserver_client client = (lnserver_client) lw_stream_tag (client_socket);

      if (!client)
         continue;

      if (client->id == id)
         return client;
   }

   return 0;
}

lnserver_channel lnserver_client_read_channel (lnserver_client client,
                                               lnet_buffer buffer)
{
   if (buffer->length < 2)
      return 0;

   lw_i16 id = lnet_buffer_int16 (buffer);

   list_each (client->channels, channel)
   {
      if (channel->id == id)
         return channel;
   }

   return 0;
}

