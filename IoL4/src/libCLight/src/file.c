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

#include <stdio.h>
#include <l4/kdebug.h>

FILE* fopen(const char* path, const char* mode)
{
  return NULL;
}

char* fgets(char* str, int size, FILE* stream)
{
  return NULL;
}

size_t fread(void* ptr, size_t size, size_t nmemb, FILE* stream)
{
  return 0;
}

size_t fwrite(const void* ptr, size_t size, size_t nmemb, FILE* stream)
{
  return 0;
}

int fseek(FILE* stream, long offset, int whence)
{
  return EOF;
}

long ftell(FILE* stream)
{
  return -1;
}

int fclose(FILE* stream)
{
  return -1;
}

int feof(FILE* stream)
{
  return 1;
}

int ferror(FILE* stream)
{
  return 1;
}

int fgetc(FILE* stream)
{
  int foo;
  if (stream == (FILE*) stdin)
#if 0
    return getc();
#else
  {
    foo = L4_KDB_ReadChar_Blocked();
    L4_KDB_PrintChar(foo);
    return foo;
  }
#endif
  else
    return -1;
}

int fputs(const char* str, FILE* stream)
{
  if ((stream == (FILE*) stdout) || (stream == (FILE*) stderr))
    return printf("%s", str);
  else
    return -1;
}
