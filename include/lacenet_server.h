
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

#ifndef LACENET_SERVER_PUB_H
#define LACENET_SERVER_PUB_H

#include <lacenet.h>

#if defined(__cplusplus) && !defined(_MSC_VER)
extern "C" {
#endif

typedef struct _lnserver * lnserver;

/* lnserver_client: represents a client on the server
 */
typedef struct _lnserver_client * lnserver_client;

   void lnserver_client_set_tag (lnserver_client, void *);
   void * lnserver_client_tag (lnserver_client);

   const char * lnserver_client_name (lnserver_client);

   void lnserver_client_send (lnserver_client,
                              long subchannel,
                              const char * buffer,
                              size_t size,
                              long variant);

   void lnserver_client_blast (lnserver_client,
                               long subchannel,
                               const char * buffer,
                               size_t size,
                               long variant);


/* lnserver_channel: represents a channel on the server.  These are created
 * by a client upon joining, and reach user code via the callbacks.
 */
typedef struct _lnserver_channel * lnserver_channel;

   void lnserver_channel_set_tag (lnserver_channel, void *);
   void * lnserver_channel_tag (lnserver_channel);

   const char * lnserver_channel_name (lnserver_channel);
   lnserver_client lnserver_channel_master (lnserver_channel);

   void lnserver_channel_send (lnserver_channel,
                               long subchannel,
                               const char * buffer,
                               size_t size,
                               long variant);

   void lnserver_channel_blast (lnserver_channel,
                                long subchannel,
                                const char * buffer,
                                size_t size,
                                long variant);


/* lnserver_config: passed to lnserver_new to configure callbacks etc.
 */
typedef struct _lnserver_config
{
   void (* on_error) (lnserver, lw_error);

   void (* on_connect) (lnserver, lnserver_client);
   void (* on_disconnect) (lnserver, lnserver_client);

   lw_bool (* on_server_message) (lnserver,
                                  lnserver_client,
                                  lw_bool blasted,
                                  long subchannel,
                                  const char * buffer,
                                  size_t length,
                                  int variant);
 
   lw_bool (* on_channel_message) (lnserver,
                                   lnserver_client,
                                   lnserver_channel,
                                   lw_bool blasted,
                                   long subchannel,
                                   const char * buffer,
                                   size_t length,
                                   int variant);

   lw_bool (* on_peer_message) (lnserver,
                                lnserver_client from,
                                lnserver_client to,
                                lnserver_channel,
                                lw_bool blasted,
                                long subchannel,
                                const char * buffer,
                                size_t length,
                                int variant);

} lnserver_config;

lnserver lnserver_new (lw_pump, struct _lnserver_config);
void lnserver_delete (lnserver);

void lnserver_host (lnserver, long port);
void lnserver_host_filter (lnserver, lw_filter);

#if defined(__cplusplus) && !defined(_MSC_VER)
}
#endif

#endif

