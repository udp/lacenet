
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

#include "common.h"
#include "message.h"

#ifndef _MSC_VER
#include <alloca.h>
#else
#include <malloc.h>
#define alloca _alloca
#endif
#include <string.h>

int lnet_message_read (lnet_message message,
                       lnet_buffer buffer,
                       int * status)
{
   const int flag_got_type = 1,
             flag_got_size = 2,
             flag_size_16  = 4,
             flag_size_32  = 8;

   *status = LNET_MESSAGE_STATUS_MORE;

   while (buffer->length > 0)
   {
      if (! (message->parse_flags & flag_got_type))
      {
         message->parse_flags |= flag_got_type;

         message->type = lnet_buffer_int8 (buffer);
         message->variant = (message->type << 4);

         message->type >>= 4;
         message->variant >>= 4;

         continue;
      }

      if (! (message->parse_flags & flag_got_size))
      {
         if (message->parse_flags & flag_size_32)
         {
            if (buffer->length < 4)
               return LNET_E_OK;

            message->length = lnet_buffer_int32 (buffer);

            message->parse_flags &= ~ flag_size_32;
            message->parse_flags |= flag_got_size;

            continue;
         }

         if (message->parse_flags & flag_size_16)
         {
            if (buffer->length < 2)
               return LNET_E_OK;

            message->length = lnet_buffer_int16 (buffer);

            message->parse_flags &= ~ flag_size_16;
            message->parse_flags |= flag_got_size;

            continue;
         }

         unsigned char b = lnet_buffer_int8 (buffer);

         if (b < 254)
         {
            /* Simple 8-bit size */

            message->length = b;
            message->parse_flags |= flag_got_size;

            continue;
         }

         if (b == 254)
         {
            /* 16-bit size to follow */

            message->parse_flags |= flag_size_16;
            continue;
         }

         if (b == 255)
         {
            /* 32-bit size to follow */

            message->parse_flags |= flag_size_32;
            continue;
         }
      }

      *status = LNET_MESSAGE_STATUS_READY;
      return LNET_E_OK;
   }

   *status = LNET_MESSAGE_STATUS_MORE;
   return LNET_E_OK;
}

void lnet_message_next (lnet_message message)
{
   message->parse_flags = 0;
}

void lnet_message_send (lw_stream stream,
                        lw_ui8 type,
                        lw_ui8 variant,
                        size_t num, ...)
{
   va_list args;
   size_t size = 0;

   type <<= 4;
   type |= variant;

   lw_stream_write (stream, (const char *) &type, sizeof (type)); 


   /* Calculate the total size of the message */

   va_start (args, num);

   for (size_t i = 0; i < num; ++ i)
   {
      va_arg (args, char *);
      size += va_arg (args, size_t);
   }

   va_end (args);


   /* Write the size header */

   if (size < 254)
   {
      lw_i8 size_hdr = (lw_i8) size;
      lw_stream_write (stream, (const char *) &size_hdr, sizeof (size_hdr));
   }
   else if (size <= 0xFFFF)
   {
      lw_i16 size_hdr = lnet_htons (size);
      lw_stream_write (stream, (const char *) &size_hdr, sizeof (size_hdr));
   }
   else if (size <= 0xFFFFFFFF)
   {
      lw_i32 size_hdr = lnet_htonl (size);
      lw_stream_write (stream, (const char *) &size_hdr, sizeof (size_hdr));
   }
   else
   {
      /* TODO: Message too big - generate error */
   }


   /* Write the message body */

   va_start (args, num);

   for (size_t i = 0; i < num; ++ i)
   {
      char * buffer = va_arg (args, char *);
      size_t size = va_arg (args, size_t);

      /* TODO: Use lw_stream_writev when implemented */

      lw_stream_write (stream, buffer, size);
   }

   va_end (args);
}

void lnet_message_blast (lw_udp udp, lw_addr addr,
                         lw_i16 id,
                         lw_ui8 type, lw_ui8 variant,
                         size_t num, ...)
{
   va_list args;
   size_t size = 0;

   type <<= 4;
   type |= variant;


   /* Calculate the total size of the message */

   va_start (args, num);

   for (size_t i = 0; i < num; ++ i)
   {
      va_arg (args, char *);
      size += va_arg (args, size_t);
   }

   va_end (args);


   /* Build the message buffer.
    *
    * TODO: liblacewing support for MSG_MORE would avoid having to do this,
    * at least w/ Linux.
    */

   char * p, * buffer;
   p = buffer = (char *) alloca (size + 3);

   *p ++ = type;

   *(lw_i16 *) p = lnet_htons (id);
   p += sizeof (lw_i16);

   va_start (args, num);

   for (size_t i = 0; i < num; ++ i)
   {
      char * cur_buffer = va_arg (args, char *);
      size_t cur_size = va_arg (args, size_t);

      memcpy (p, cur_buffer, cur_size);
      p += cur_size;
   }

   va_end (args);

   lw_udp_send (udp, addr, buffer, size + 3);
}

