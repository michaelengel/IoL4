/*
 * Copyright 2003, Kentaro A. Kurahone
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of Kentaro A. Kurahone nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY KENTARO A. KURAHONE AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL KENTARO A. KURAHONE OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <string.h>
#include <malloc.h>

int memcmp(const void* b1, const void* b2, size_t len)
{
  int rval = 0, i;

  if (len > 0)
  {
    for (i = 0; i < len; i++)
    {
      if (*(unsigned char*) b1 != *(unsigned char*) b2)
      {
        rval = (*(unsigned char*) b1 > *(unsigned char*) b2) ?
               *(unsigned char*) b1 - *(unsigned char*) b2 :
               *(unsigned char*) b2 - *(unsigned char*) b1;
        break;
      }
      b1++;
      b2++;
    }
  }

  return rval;
}

void* memcpy(void* dst, void* src, size_t len)
{
  if (len > 0)
  {
    size_t i = (len+7)/8;
    char* ssrc = (char *)src;
    char* ddst = (char *)dst;

    switch (len%8)
    {
      case 0: do { *ddst++ = *ssrc++;
      case 7:      *ddst++ = *ssrc++;
      case 6:      *ddst++ = *ssrc++;
      case 5:      *ddst++ = *ssrc++;
      case 4:      *ddst++ = *ssrc++;
      case 3:      *ddst++ = *ssrc++;
      case 2:      *ddst++ = *ssrc++;
      case 1:      *ddst++ = *ssrc++;
              } while (--i > 0);
    }
  }

  return dst;
}

void* memset(void* b, int c, size_t len)
{
  if (len > 0)
  {
    size_t i = (len+7)/8;
    unsigned char* dst = (unsigned char*) b;

    switch (len%8)
    {
      case 0: do { *dst++ = (unsigned char) c;
      case 7:      *dst++ = (unsigned char) c;
      case 6:      *dst++ = (unsigned char) c;
      case 5:      *dst++ = (unsigned char) c;
      case 4:      *dst++ = (unsigned char) c;
      case 3:      *dst++ = (unsigned char) c;
      case 2:      *dst++ = (unsigned char) c;
      case 1:      *dst++ = (unsigned char) c;
                 } while (--i > 0);
    }
  }

  return b;
}

void* memmove(void* dst, void* src, size_t len)
{
  if (len > 0)
  {
    /* If they do not overlap this is eqiv to memcpy. */
    if (((dst < src) && (dst + len <= src)) ||
        ((dst > src) && (dst >= src + len)))
    {
      return memcpy(dst, src, len);
    }
    else
    {
      /* Since this needs to be a nondestructive move,
       * allocate a temporary buffer to store src. */
      void* tmp = malloc(len);

      memcpy(tmp, src, len);
      memmove(dst, tmp, len);
      free(tmp);
    }
  }

  return dst;
}
