/*   Copyright (c) 2002, Steve Dekorte
 *   All rights reserved. See _BSDLicense.txt.
 */

#include "ByteArray.h"

#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#if defined(DOS) || defined(WIN32)
  #define PATH_SEPARATOR "\\"
#else
  #define PATH_SEPARATOR "/"
#endif

ByteArray *ByteArray_new(void)
{ return ByteArray_newWithSize_(0); }

ByteArray *ByteArray_newWithCString_(char *s)
{ return ByteArray_newWithData_size_copy_(s, strlen(s), 1); }

ByteArray *ByteArray_newWithCString_length_(char *s, int length)
{ return ByteArray_newWithData_size_copy_(s, length, 1); }

ByteArray *ByteArray_newWithSize_(int size)
{
  ByteArray *self = (ByteArray *)calloc(1, sizeof(ByteArray));
  self->bytes = (unsigned char *)calloc(1, size+1);
  self->length = size;
  return self;
}

ByteArray *ByteArray_newWithData_size_copy_(unsigned char* buf, int size, char copy)
{
  ByteArray *self = (ByteArray *)calloc(1, sizeof(ByteArray));
  if (copy)
  {
    self->bytes = (unsigned char *)malloc(size+1);
    self->bytes[size] = (unsigned char)0;
    memcpy(self->bytes, buf, size);
  }
  else
  { self->bytes = buf; }
  self->length = size;
  return self;
}

ByteArray *ByteArray_clone(ByteArray *self)
{ return ByteArray_newWithData_size_copy_(self->bytes, self->length, 1); }

void ByteArray_free(ByteArray *self)
{
  if (self->bytes) { free(self->bytes); }
  free(self);
}

unsigned char * ByteArray_bytes(ByteArray *self) { return self->bytes; }

int ByteArray_length(ByteArray *self) { return self->length; }

size_t ByteArray_memorySize(ByteArray *self)
{ return sizeof(ByteArray) + self->memSize; }

void ByteArray_compact(ByteArray *self)
{ ByteArray_sizeTo_(self, self->length); }

void ByteArray_clear(ByteArray *self)
{
  memset(self->bytes, 0, self->length);
  self->length = 0;
}

void ByteArray_sizeTo_(ByteArray *self, int size)
{
  if (size > self->memSize)
  {
    self->bytes = (unsigned char *)realloc(self->bytes, size+1);
    self->memSize = size;
  }
  self->bytes[size] = (unsigned char)0x0;
}

void ByteArray_setLength_(ByteArray *self, int length)
{
  ByteArray_sizeTo_(self, length);
  self->length = length;
}

void ByteArray_copy_(ByteArray *self, ByteArray *other)
{ ByteArray_setData_length_(self, other->bytes, other->length); }

void ByteArray_setData_length_(ByteArray *self, unsigned char *bytes, size_t length)
{
  ByteArray_setLength_(self, length);
  memcpy(self->bytes, bytes, length);
}

int ByteArray_compare_(ByteArray *self, ByteArray *other)
{
  int n = self->length < other->length ? self->length : other->length;
  return memcmp(self->bytes, other->bytes, n);
}

/****************************************************
** single byte/character operations
****************************************************/

int ByteArray_hasDigit(ByteArray *self) 
{
  int i;
  for (i=0;i<self->length;i++) { if (isdigit(self->bytes[i])) return 1; }
  return 0;
}

unsigned char ByteArray_at_(ByteArray *self, int pos) 
{
  int len = self->length;
  if (len == -1 || pos > len) { return 0; }
  return self->bytes[ByteArray_wrapPos_(self, pos)];
}

int ByteArray_at_put_(ByteArray *self, int pos, unsigned char c) 
{
  int len = self->length;
  if (len == -1 || pos > len) { return 0; }
  self->bytes[ByteArray_wrapPos_(self, pos)] = c;
  return 1;
}

void ByteArray_dropLastByte(ByteArray *self) { self->length--; }

void ByteArray_removeByteAt_(ByteArray *self, int pos) 
{ ByteArray_removeFrom_to_(self, pos, pos); }

void ByteArray_removeCharAt_(ByteArray *self, int pos) 
{
  int csize = BYTEARRAY_BYTES_PER_CHARACTER;
  int len = self->length;
  if (len < 1) return;
  pos = ByteArray_wrapPos_(self, pos);
  memmove(self->bytes+pos*csize, self->bytes+(pos+1)*csize, (len-pos+1)*csize);
}

void ByteArray_removeFrom_to_(ByteArray *self, int from, int to)
{
  int len = self->length;
  if (len < 1) return;
  from = ByteArray_wrapPos_(self, from);
  to = ByteArray_wrapPos_(self, to);
  to++;
  if (to < len) memmove(self->bytes+from, self->bytes+to, len-to);
  ByteArray_setLength_(self, len-(to-from));
}

/* --- access --------------------------------------------*/

char *ByteArray_asCString(ByteArray *self) { return self->bytes; }

/* --- escape --------------------------------------------*/
        
void ByteArray_escape(ByteArray *self)
{
  ByteArray *ba = ByteArray_new();
  char *s = (char *)self->bytes;
  int i;
  for (i = 0; i < self->length; i++)
  {
    char c = s[i];
    switch (c)
    { 
      case '"':  ByteArray_appendCString_(ba, "\\\"");break;
      case '\a': ByteArray_appendCString_(ba, "\\a"); break;
      case '\b': ByteArray_appendCString_(ba, "\\b"); break;
      case '\f': ByteArray_appendCString_(ba, "\\f"); break;
      case '\n': ByteArray_appendCString_(ba, "\\n"); break;
      case '\r': ByteArray_appendCString_(ba, "\\r"); break;
      case '\t': ByteArray_appendCString_(ba, "\\t"); break;
      case '\v': ByteArray_appendCString_(ba, "\\v"); break;
      default: ByteArray_appendChar_(ba, c);
    }
  }
  s[i] = (char)NULL;
  ByteArray_copy_(self, ba);
  ByteArray_free(ba);
}

void ByteArray_unescape(ByteArray *self)
{
  int getIndex = 0;
  int putIndex = 0;
  char *s = (char *)self->bytes;
  while (getIndex < self->length)
  {
    char c = s[getIndex];
    char nextChar = s[getIndex+1];
    if (c != '\\')
    {
      if (getIndex != putIndex) { s[putIndex] = c; }
      putIndex++;
    }
    else
    {
      char realChar = nextChar;
      if (nextChar == 'a') { realChar = '\a'; } else
      if (nextChar == 'b') { realChar = '\b'; } else
      if (nextChar == 'f') { realChar = '\f'; } else
      if (nextChar == 'n') { realChar = '\n'; } else
      if (nextChar == 'r') { realChar = '\r'; } else
      if (nextChar == 't') { realChar = '\t'; } else      
      if (nextChar == 'v') { realChar = '\v'; }     
      s[putIndex] = realChar;
      getIndex++; putIndex++;
    }
    getIndex++;
  }
  s[putIndex] = (char)NULL;
  ByteArray_setLength_(self, putIndex);
}

void ByteArray_quote(ByteArray *self)
{
  int oldLength = self->length;
  ByteArray_setLength_(self, self->length+2);
  memmove(self->bytes+1, self->bytes, oldLength);
  self->bytes[0] = '"';
  self->bytes[self->length-1] = '"';
}

void ByteArray_unquote(ByteArray *self)
{
  if (self->length == 2)
  { ByteArray_setLength_(self, 0); }
  else
  {
    memmove(self->bytes, self->bytes+1, self->length-1);
    ByteArray_setLength_(self, self->length-2);
  }
}

/* --- append --------------------------------------------*/

void ByteArray_appendChar_(ByteArray *self, char c)
{
  char s[1];
  s[0] = c;
  ByteArray_appendBytes_length_(self, s, 1);
}

void ByteArray_append_(ByteArray *self, ByteArray *other)
{ ByteArray_appendBytes_length_(self, other->bytes, other->length); }

void ByteArray_appendCString_(ByteArray *self, char *s)
{ ByteArray_appendBytes_length_(self, (unsigned char *)s, strlen(s)); }

void ByteArray_appendAndEscapeCString_(ByteArray *self, char *s)
{ 
  char oneChar[2];
  oneChar[1] = (char)0x0;
  while (*s)
  {
    if (*s == '\n')
    { ByteArray_appendBytes_length_(self, "\\n", 2); }
    else if (*s == '\t')
    { ByteArray_appendBytes_length_(self, "\\t", 2); }
    else if (*s == '"')
    { ByteArray_appendBytes_length_(self, "\\\"", 2); }
    else
    {
      oneChar[0] = *s;
      ByteArray_appendBytes_length_(self, oneChar, 1);
    }
    s++;
  }
  self->bytes[self->length] = (unsigned char)0x0;
}

void ByteArray_appendBytes_length_(ByteArray *self, unsigned char *bytes, long length)
{
  int oldlength = self->length;
  ByteArray_setLength_(self, oldlength + length);
  memcpy(self->bytes+oldlength, bytes, length);
  self->bytes[self->length] = (unsigned char)0x0; /* not needed? */
  /*self->bytes+oldlength+length*/
}

/* --- prepend --------------------------------------------*/

void ByteArray_prepend_(ByteArray *self, ByteArray *other)
{ ByteArray_prependBytes_length_(self, other->bytes, other->length); }

void ByteArray_prependCString_(ByteArray *self, char *s)
{ ByteArray_prependBytes_length_(self, (unsigned char *)s, strlen(s)); }

void ByteArray_prependBytes_length_(ByteArray *self, unsigned char *bytes, long length)
{
  int oldSize = self->length;
  ByteArray_setLength_(self, oldSize + length);
  memmove(self->bytes + length, self->bytes, oldSize);
  memcpy(self->bytes, bytes, length);
}

/* --- subarray --------------------------------------------*/

ByteArray *ByteArray_newWithBytesFrom_to_(ByteArray *self, int startpos, int endpos) 
{
  int newlen;
  startpos = ByteArray_wrapPos_(self, startpos);
  endpos   = ByteArray_wrapPos_(self, endpos);
  newlen = endpos - startpos+1;
  if (newlen < 1) return ByteArray_new();
  return ByteArray_newWithData_size_copy_(self->bytes+startpos, newlen, 1);
}

/* --- insert --------------------------------------------*/

void ByteArray_insert_at_(ByteArray *self, ByteArray *other, int pos) 
{ ByteArray_insertBytes_length_at_(self, other->bytes, other->length, pos); }

void ByteArray_insertCString_at_(ByteArray *self, char *s, int pos) 
{ ByteArray_insertBytes_length_at_(self, s, strlen(s), pos); }

void ByteArray_insertBytes_length_at_(ByteArray *self, unsigned char *bytes, long length, int pos) 
{
  long oldSize = self->length;
  pos = ByteArray_wrapPos_(self, pos);
  if (pos == self->length) { ByteArray_appendBytes_length_(self, bytes, length); return; }
  ByteArray_setLength_(self, self->length + length);
  memmove(self->bytes+pos+length, self->bytes+pos, oldSize - pos);
  memcpy(self->bytes+pos, bytes, length);
}

/* --- case --------------------------------------------*/

int ByteArray_isLowerCase(ByteArray *self)
{
  int i;
  for (i = 0; i <= self->length; i++)
  {
    unsigned char c = self->bytes[i];
    if (c != tolower(c)) return 0;
  }
  return 1;
}

int ByteArray_isUpperCase(ByteArray *self)
{
  int i;
  for (i = 0; i <= self->length; i++)
  {
    unsigned char c = self->bytes[i];
    if (c != toupper(c)) return 0;
  }
  return 1;
}

void ByteArray_lower(ByteArray *self)
{
  int i;
  for (i = 0; i <= self->length; i++)
    self->bytes[i] = (unsigned char)tolower((char)self->bytes[i]);
}

void ByteArray_upper(ByteArray *self)
{
  int i;
  for (i = 0; i <= self->length; i++)
    self->bytes[i] = (unsigned char)toupper((char)self->bytes[i]);
}

/****************************************************
** string comparision operations
****************************************************/

int ByteArray_equals_(ByteArray *self, ByteArray *other)		
{
  /* returns 1 if equal, 0 if not equal. */
  if (self->length != other->length) return 0;
  return (memcmp(self->bytes, other->bytes, self->length)==0);
}

int ByteArray_equalsAnyCase_(ByteArray *self, ByteArray *other)		
{
  if (self->length != other->length) return 0;
  return ByteArray_containsAnyCase_(self, other);
}

int ByteArray_contains_(ByteArray *self, ByteArray *other)		
{ return (ByteArray_find_(self, other) != -1); }

int ByteArray_containsAnyCase_(ByteArray *self, ByteArray *other)		
{ return (ByteArray_findAnyCase_(self, other) != -1); }

int ByteArray_find_(ByteArray *self, ByteArray *other)		
{ return ByteArray_find_from_(self, other, 0); }

int ByteArray_beginsWith_(ByteArray *self, ByteArray *other)		
{ 
  if (!other->length || !self->length || other->length > self->length) return 0;
  return (strncmp(self->bytes, other->bytes, other->length) == 0); 
}

int ByteArray_endsWith_(ByteArray *self, ByteArray *other)		
{ 
  if (!other->length || !self->length || other->length > self->length) return 0;
  return (strcmp(self->bytes + self->length - other->length, other->bytes) == 0); 
}

int ByteArray_findAnyCase_(ByteArray *self, ByteArray *other)		
{ return ByteArray_findAnyCase_from_(self, other, 0); }

/* return -1 for no match, starting position of match if found */
int ByteArray_find_from_(ByteArray *self, ByteArray *other, int from)		
{
  from = ByteArray_wrapPos_(self, from);
  {
  char *p = strstr(self->bytes + from, other->bytes);
  ptrdiff_t rval;
  if (!p) return -1;
  rval = (ptrdiff_t)p - (ptrdiff_t)(self->bytes);
  return rval;
  }
}

int ByteArray_findCString_from_(ByteArray *self, char *other, int from)
{
  from = ByteArray_wrapPos_(self, from);
  {
  char *p = strstr(self->bytes + from, other);
  ptrdiff_t rval;
  if (!p) return -1;
  rval = (ptrdiff_t)p - (ptrdiff_t)(self->bytes);
  return rval;
  }
}

int ByteArray_rFindCString_from_(ByteArray *self, char *other, int from)		
{
  /* return -1 for no match, starting position of match if found */
  unsigned char *bytes = self->bytes;

  from = ByteArray_wrapPos_(self, from);
  bytes = bytes + from - strlen(other);
  while (bytes >= self->bytes)
  {
    if (memcmp(bytes, other, strlen(other))==0)
    { return bytes - self->bytes; }
    bytes--;
  }
  return -1;
}

int ByteArray_rFind_from_(ByteArray *self, ByteArray *other, int from)		
{
  /* return -1 for no match, starting position of match if found */
  unsigned char *bytes = self->bytes;

  from = ByteArray_wrapPos_(self, from);
  bytes = bytes + from - other->length;
  while (bytes >= self->bytes)
  {
    if (memcmp(bytes, other->bytes, other->length)==0)
    { return bytes - self->bytes; }
    bytes--;
  }
  return -1;
}

int ByteArray_findAnyCase_from_(ByteArray *self, ByteArray *other, int from)		
{
  /* return -1 for no match */
  int n, m;
  int len2 = other->length;
  int max = self->length - other->length;
  from = ByteArray_wrapPos_(self, from);

  if (self->length < len2) return -1; /* too big */
  if (len2 <= 0) return -1; /* too small */
    
  for (n = from; n <= max; n++) 
  {
    m = 0;
    while ((tolower(self->bytes[n+m]) == tolower(other->bytes[m]) && (m < len2+1))) 
    {
      if (m == len2-1) return n;
      m++;
    }
  }
  return -1;
}

int ByteArray_count_(ByteArray *self, ByteArray *other)		
{
  /* return number of non-overlapping occurances of other in self */
  int count = 0;
  
  if (other->length > 0)
  {
    int index = ByteArray_find_from_(self, other, 0);
    while (index != -1) 
    {
      index = ByteArray_find_from_(self, other, index + other->length);
      count++;
    }
  }
  return count;
}

void ByteArray_replaceCString_withCString_(ByteArray *self, char *s1, char *s2)
{
  ByteArray *b1 = ByteArray_newWithCString_(s1);
  ByteArray *b2 = ByteArray_newWithCString_(s2);
  ByteArray_replace_with_(self, b1, b2);
  ByteArray_free(b1);
  ByteArray_free(b2);
}

void ByteArray_replace_with_output_(ByteArray *self, ByteArray *substring, ByteArray *other, ByteArray *output)
{
  int lastGetIndex = 0;
  int getIndex = 0;
  while (getIndex < self->length)
  {
    getIndex = ByteArray_find_from_(self, substring, getIndex);
    if (getIndex == -1) getIndex = self->length;
    
    /* append the non-matching chunk */
    ByteArray_appendBytes_length_(output, self->bytes + lastGetIndex, getIndex - lastGetIndex);
    if (getIndex == self->length) break;
    
    /* append the other string */
    ByteArray_append_(output, other);
    getIndex += substring->length;
    lastGetIndex = getIndex;
  }
}

void ByteArray_replace_with_(ByteArray *self, ByteArray *substring, ByteArray *other)
{
  ByteArray *output = ByteArray_new();
  ByteArray_replace_with_output_(self, substring, other, output);
  ByteArray_copy_(self, output);
  ByteArray_free(output);
}

void ByteArray_replace_with_OLD(ByteArray *self, ByteArray *substring, ByteArray *other)
{
  /* replace occurances of substring with other */
  /* for effiency, replacement is done *without* allocating a seperate byte array */
  int count = ByteArray_count_(self, substring);
  int newSize = self->length + count*(other->length-substring->length);
  
  if (!count) { return; }
  
  if ( substring->length == other->length)
  {
    int i = ByteArray_find_from_(self, substring, 0);
    while (i!=-1)
    {
      memcpy(self->bytes+i, other->bytes, other->length);
      i = ByteArray_find_from_(self, substring, i+substring->length);
    }
  }
  else if (substring->length > other->length)
  {
    int last = -1;
    int i = ByteArray_find_from_(self, substring, 0);
    int put = i;
    while (i!=-1)
    {
      if (last!=-1)
      {
        int span = i-(last + substring->length);
        memmove(self->bytes + put, self->bytes + last + substring->length, span);
        put += span;
      }
      memcpy(self->bytes+put, other->bytes, other->length);
      put += other->length;
      last = i;
      i = ByteArray_find_from_(self, substring, i + substring->length);
    }
    if (last != -1)
    {
      int span = self->length-(last + substring->length);
      memmove(self->bytes + put, self->bytes + last + substring->length, span);
    }
    ByteArray_setLength_(self, newSize);
  }
  else /* (substring->length < other->length) */
  {
    int length = self->length;
    int last = length;
    int i = ByteArray_rFind_from_(self, substring, length);
    int put = i + substring->length;
    int span;
    ByteArray_setLength_(self, newSize);

    /* memset(self->bytes+length, ' ', newSize-length); */

    span = last - (i + substring->length);
    put = newSize;

    while (i!=-1 && span >= 0)
    {
      /* copy non-matching space */
      span = last - (i + substring->length);
      
      put -= span;
      memmove(self->bytes + put, self->bytes + i + substring->length, span);
      put -= other->length;
        
      /* copy replacement */
       memcpy(self->bytes+put, other->bytes, other->length);
       last = i;
       i = ByteArray_rFind_from_(self, substring, i - 1);

       span = last - (i + substring->length);
    }
  }
}

/****************************************************
 * I/O
 ****************************************************/

void ByteArray_print(ByteArray *self)
{ ByteArray_writeToCStream_(self, stdout); }

size_t ByteArray_writeToCStream_(ByteArray *self, FILE *stream)
{ return fwrite(self->bytes, self->length, 1, stream); }

int ByteArray_readFromCStream_(ByteArray *self, FILE *fp)
{
  size_t fileSize;
  size_t pos; 
  
  if (!fp) return -1; 
  /* get file size */
  pos = ftell(fp);
  fseek(fp, 0, SEEK_END);
  fileSize = ftell(fp) - pos;
  fseek(fp, 0, pos);
  /*rewind(fp);*/
  ByteArray_setLength_(self, fileSize);
  
  /* read whole file into buffer */
  {
  size_t n = fread(self->bytes, 1, fileSize, fp);
  if (n!=fileSize) 
  { printf("WARNING: read full file but only found %i bytes while ftell() indicated a size of %i\n", (int)n, (int)fileSize); }
  } 
  return 0;
}

int ByteArray_readFromFilePath_(ByteArray *self, char *path)
{
  FILE *fp = fopen(path, "rb");
  int error;
  if (!fp) return -1;

  error = ByteArray_readFromCStream_(self, fp);
  fclose(fp); 
  return error;
}

unsigned char ByteArray_readLineFromCStream_(ByteArray *self, FILE *stream)
{
  char *s = malloc(1024*4+1);
  memset(s, (unsigned char)0x0, 1024*4);
  if (fgets(s, 1024*4, stream) == NULL)
  {
    free(s);
    return 0;
  }
  for(;;)
  {
    char *eol = strchr(s, '\n');
    if (eol) { *eol = 0x0; } /* remove the return character */
    ByteArray_appendCString_(self, s);
    if (eol) { break; }
    fgets(s, 1024*4, stream);
    if (feof(stream) != 0 || ferror(stream) != 0) break;
  }
  free(s);
  if (ftell(stream)==0) return 0;
  return 1;
}

/*
void ByteArray_readLineFromCStream_(ByteArray *self, FILE *stream)
{
  int readSize = 1024*4;
  int offset = 0;
  while (feof(stream) == 0 && ferror(stream) == 0) 
  {
    char *eol;
    ByteArray_sizeTo_(self, offset + readSize);
    fgets(self->bytes + offset, readSize+1, stream);
    eol = strchr(self->bytes + offset, '\n');
    if (eol) { *eol = 0x0; break; } 
    offset += readSize;
  }
}
*/

size_t ByteArray_readNumberOfBytes_fromCStream_(ByteArray *self, long length, FILE *stream)
{
  size_t readSize;
  ByteArray_setLength_(self, length);
  readSize =  fread((void *)self->bytes, 1, length, stream);
  ByteArray_setLength_(self, readSize);
  return readSize;
}

/****************************************************
** private utility functions
****************************************************/

int ByteArray_wrapPos_(ByteArray *self, int pos)
{
  int len = self->length;
  if (pos > len) return len; 
  else if (pos < 0) return len + pos;
  return pos;
}

ByteArray *ByteArray_newWithFormat_(const char *format, ...)
{
  ByteArray *self;
  va_list ap;
  va_start(ap, format);
  self = ByteArray_newWithVargs_(format, ap);
  va_end(ap);
  return self;
}

ByteArray *ByteArray_newWithVargs_(const char *format, va_list ap)
{
  ByteArray *self = ByteArray_new();
  ByteArray_fromVargs_(self, format,ap);
  return self;
}

ByteArray *ByteArray_fromFormat_(ByteArray *self, const char *format, ...)
{
  va_list ap;
  va_start(ap, format);
  ByteArray_fromVargs_(self, format, ap);
  va_end(ap);
  return self;
}

void ByteArray_fromVargs_(ByteArray *self, const char *format, va_list ap)
{  
  while (*format)
  {
    if (*format == '%')
    {
      format++;
      if (*format == 's')
      {
        char *s = va_arg(ap, char *);
        if (!s) { printf("IoState_print missing param"); return; }
        ByteArray_appendCString_(self, s);
      }
      else if (*format == 'i' || *format == 'd')
      {
        int i = va_arg(ap, int);
        char s[100];
        sprintf(s, "%i", i);
        ByteArray_appendCString_(self, s);
      }
      else if (*format == 'f')
      {
        double d = va_arg(ap, double);
        char s[100];
        sprintf(s, "%f", d);
        ByteArray_appendCString_(self, s);
      }
      else if (*format == 'p')
      {
        void *p = va_arg(ap, void *);
        char s[100];
        sprintf(s, "%p", p);
        ByteArray_appendCString_(self, s);
      }
      else if (*format == '#') /* new format command for a given number adding spaces */
      {
        int n;
        int i = va_arg(ap, int);
        char *s = " ";
        for (n=0; n < i; n++) ByteArray_appendCString_(self, s);
      }
    }
    else
    {
      char s[2];
      sprintf(s, "%c", *format);
      ByteArray_appendCString_(self, s);
    }
    format++;
  }
}

ByteArray *ByteArray_asNewHexStringByteArray(ByteArray *self)
{
  ByteArray *ba = ByteArray_newWithSize_(self->length * 2);
  int i;
  for(i=0; i<self->length;i++)
  {
    int c = self->bytes[i];
    char *s = ba->bytes + i*2;
    if (c < 16) sprintf(s, "0%x", c); else sprintf(s, "%x", c);
  }
  return  ba;
}

/****************************************************
** file paths
****************************************************/

unsigned char ByteArray_lastByte(ByteArray *self)
{
  if (!self->length) return 0x0;
  return self->bytes[self->length-1];
}

void ByteArray_appendPathCString_(ByteArray *self, char *path)
{
  char lastChar = ByteArray_lastByte(self);
  if (!strlen(path)) return;

  lastChar = ByteArray_lastByte(self);
  if ((lastChar == *(PATH_SEPARATOR)) && (*path == *(PATH_SEPARATOR)))
  { ByteArray_appendCString_(self, path+1); }
  else
  if ((lastChar != *(PATH_SEPARATOR)) && (*path != *(PATH_SEPARATOR)))
  {
    if (self->length) ByteArray_appendCString_(self, PATH_SEPARATOR);
    ByteArray_appendCString_(self, path);
  }
  else
  { ByteArray_appendCString_(self, path); }
  
  ByteArray_replaceCString_withCString_(self, "/./", "/");
}

void ByteArray_removeLastPathComponent(ByteArray *self)
{
  int pos = ByteArray_rFindCString_from_(self, PATH_SEPARATOR, self->length-1);
  if (pos == -1) 
    ByteArray_setLength_(self, 0); 
  else
    ByteArray_setLength_(self, pos);
}

void ByteArray_clipBeforeLastPathComponent(ByteArray *self)
{
  int pos = ByteArray_rFindCString_from_(self, PATH_SEPARATOR, self->length-1);
  if (pos != -1) ByteArray_removeFrom_to_(self, 0, pos);
}

ByteArray *ByteArray_lastPathComponent(ByteArray *self)
{ return ByteArray_newWithCString_(ByteArray_lastPathComponentAsCString(self)); }

char *ByteArray_lastPathComponentAsCString(ByteArray *self)
{
  int pos = ByteArray_rFindCString_from_(self, PATH_SEPARATOR, self->length-1);
  char *s;
  if (pos == -1) { return self->bytes; }
  s = (char *)(self->bytes+pos);
  while (*s == '/') s++;
  return s;
}

void ByteArray_removePathExtension(ByteArray *self)
{ 
  int pos = ByteArray_rFindCString_from_(self, ".", self->length-1);
  if (pos != -1) ByteArray_setLength_(self, pos);
}

ByteArray *ByteArray_pathExtension(ByteArray *self)
{
  int pos = ByteArray_rFindCString_from_(self, ".", self->length-1);
  if (pos == -1) return ByteArray_newWithCString_("");
  return ByteArray_newWithCString_(self->bytes+pos+1);
}
