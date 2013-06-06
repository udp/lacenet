
/* vim: set et ts=3 sw=3 ft=c:
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

#include <stdio.h>
#include <lacenet_client.h>

lnclient clientA;
lnclient clientB;

void on_connect (lnclient client, const char * welcome_message)
{
   printf ("Connected to %s.  Welcome message: %s\n",
            lw_addr_tostring (lnclient_server_addr (client)),
            welcome_message);

   if (client == clientA)
      lnclient_set_name (client, "Client A");
   else
      lnclient_set_name (client, "Client B");
}

void on_error (lnclient client, lw_error error)
{
   printf ("Error: %s\n", lw_error_tostring (error));
}

void on_name_set (lnclient client, const char * old_name)
{
   printf ("Name set to %s\n", lnclient_name (client));

   lnclient_join (client, "Lacenet Test", lw_false, lw_false);
}

void on_join (lnclient client, lnclient_channel channel)
{
   printf ("Joined channel %s\n", lnclient_channel_name (channel));

   printf ("Number of peers: %d\n", (int) lnclient_channel_num_peers (channel));

   for (lnclient_peer peer = lnclient_peer_first (channel);
        peer;
        peer = lnclient_peer_next (peer))
   {
      printf ("   Peer %d: %s\n", (int) lnclient_peer_id (peer), lnclient_peer_name (peer));
   }

   printf ("<end peer list>\n");
}

int main (int argc, char * argv [])
{
   lw_pump eventpump = lw_eventpump_new ();

   lnclient_config config = {};

   config.on_connect = on_connect;
   config.on_error = on_error;
   config.on_name_set = on_name_set;
   config.on_join = on_join;

   clientA = lnclient_new (eventpump, config); 
   clientB = lnclient_new (eventpump, config); 

   lnclient_connect (clientA, "villy.net", 6121);
   lnclient_connect (clientB, "villy.net", 6121);

   lw_eventpump_start_eventloop (eventpump);

   return 0;
}



