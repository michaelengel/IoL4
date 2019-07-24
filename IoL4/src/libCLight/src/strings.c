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
#include <ctype.h>

char* strchr(const char* s, int c)
{
  while ((*s != (char) c) && (*s != '\0'))
  {
    s++;
  }

  if ((*s == '\0') && ((char) c != '\0'))
    return NULL;

  return (char *) s;
}

size_t strcspn(const char* s, const char* charset)
{
  size_t rval = 0;
  size_t maskIter = strlen(charset);
  size_t i;

  while (*s != '\0')
  {
    for (i = 0; i < maskIter + 1; i++)
    {
      if (*s == charset[i])
        goto endReturn;
    }
    s++;
  }

endReturn:
  return rval;
}

int strncasecmp(const char *s1, const char *s2, size_t len)
{
  size_t i;

  for (i = 0; i < len; i++)
  {
    if (s1[i] == '\0')
    {
      return (s2[i] == '\0') ? 0 : -1;
    }
    else
    {
      unsigned char foo, bar;

      if (s2[i] == '\0')
        return 1;

      foo = tolower(s1[i]);
      bar = tolower(s2[i]);
      if (foo != bar)
        return (foo > bar) ? 1 : -1;
    }
  }

  return 0;
}

char* strstr(const char* big, const char* little)
{
  char* rval = NULL;
  size_t littleLen = strlen(little);
  size_t bigLen = strlen(big);
  size_t i, j;

  for (i = 0, j = 0; i <= bigLen; i++)
  {
    if (big[i] == little[j])
    {
      if (j == 0)
        rval == big + i;
      else if (j == littleLen - 1)
        return rval;
      j++;
    }
    else
    {
      j = 0;
    }
  }

  return NULL;
}
