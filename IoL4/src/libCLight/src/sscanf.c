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

#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>

static int do_sscanf(const char* str, const char* format, va_list args);

int sscanf(const char* str, const char* format, ...)
{
  int rval;
  va_list args;

  va_start(args, format);
  rval = do_sscanf(str, format, args);
  va_end(args);

  return rval;
}

static int do_sscanf(const char* str, const char* format, va_list args)
{
  int i = 0;
  char* endPtr = (char *) str;

  if (format == NULL)
    return 0;

  /* Iterate over arg list. */
  while (*format)
  {
    switch (*format)
    {
      case '%':
      {
        format++;
        switch (*format)
        {
          case 'x':
          case 'o':
          {
            long *foo = va_arg(args, long*);
            *foo = (*format == 'x') ? strtol(str, &endPtr, 16) :
                                      strtol(str, &endPtr, 8);
            i++;
          }
          break;

          default:
          {
            /* Nothing to do. */
          }
        }
      }
      break;

      default:
      {
        /* Nothing to do. */
      }
    }
    format++;
    str = endPtr;
  }

  return i;
}
