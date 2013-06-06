
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
#include "buffer.h"

#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>

#ifdef _WIN32
   #include <winsock2.h>
#else
   #include <netinet/in.h>
#endif

int8_t lnet_buffer_int8 (lnet_buffer buffer)
{
   assert (buffer->length >= 1);
   -- buffer->length;

   return *buffer->ptr ++;
}

int16_t lnet_buffer_int16 (lnet_buffer buffer)
{
   assert (buffer->length >= 2);
   buffer->length -= 2;

   int16_t value = lnet_ntohs (*(int16_t *) buffer->ptr);
   buffer->ptr += 2;
   return value;
}

int32_t lnet_buffer_int32 (lnet_buffer buffer)
{
   assert (buffer->length >= 4);
   buffer->length -= 4;

   int32_t value = lnet_ntohl (*(int32_t *) buffer->ptr);
   buffer->ptr += 4;
   return value;
}

char * lnet_buffer_string (lnet_buffer buffer, lw_bool trim)
{
   if (trim)
   {
      while (isspace (*buffer->ptr))
      {
         ++ buffer->ptr;
         -- buffer->length;
      }

      while (isspace (buffer->ptr [buffer->length - 1]))
         -- buffer->length;
   }

   char * str = (char *) malloc (buffer->length + 1);
   memcpy (str, buffer->ptr, buffer->length);
   str [buffer->length] = 0;

   buffer->ptr += buffer->length;
   buffer->length = 0;

   return str;
}

char * lnet_buffer_sstring (lnet_buffer buffer, lw_bool trim)
{
   if (buffer->length < 1)
      return 0;

   long length = lnet_buffer_int8 (buffer);

   if (buffer->length < length)
      return 0;

   const char * ptr = buffer->ptr;

   buffer->ptr += length;
   buffer->length -= length;

   if (trim)
   {
      while (isspace (*ptr))
      {
         ++ ptr;
         -- length;
      }

      while (isspace (ptr [length - 1]))
         -- length;
   }

   char * str = (char *) malloc (length + 1);
   memcpy (str, ptr, length);
   str [length] = 0;

   return str;
}

lw_i16 lnet_htons (lw_i16 v)
{
   return v;
}

lw_i32 lnet_htonl (lw_i32 v)
{
   return v;
}

lw_i16 lnet_ntohs (lw_i16 v)
{
   return v;
}

lw_i32 lnet_ntohl (lw_i32 v)
{
   return v;
}

