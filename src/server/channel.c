
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

lnserver_channel lnserver_channel_new (lnserver ctx, const char * name)
{
   lnserver_channel channel = calloc
      (sizeof (struct _lnserver_channel) + strlen (name) + 1, 1);

   if (!channel)
      return 0;

   strcpy (channel->name, name);

   return channel;
}

void lnserver_channel_set_tag (lnserver_channel channel, void * tag)
{
   channel->tag = tag;
}

void * lnserver_channel_tag (lnserver_channel channel)
{
   return channel->tag;
}


lnserver_client lnserver_channel_read_member (lnserver_channel channel, 
                                              lnet_buffer buffer)
{
   if (buffer->length < 2)
      return 0;

   lw_i16 id = lnet_buffer_int16 (buffer);

   list_each (channel->members, member)
   {
      if (member->id == id)
         return member;
   }

   return 0;
}
