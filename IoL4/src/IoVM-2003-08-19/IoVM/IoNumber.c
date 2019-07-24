/*   Copyright (c) 2002, Steve Dekorte
 *   All rights reserved. See _BSDLicense.txt.
 */

#include "IoNumber.h"
#include "IoObject.h"
#include "IoState.h"
#include "IoNil.h"
#include "IoString.h"
#include "IoBuffer.h"
#ifndef IO_OS_L4
#include "IoDate.h"
#endif
#include "IoState.h"
#include <math.h>
#include <ctype.h>

#define DATA(self) ((IoNumberData *)self->data)

IoTag *IoNumber_tag(void *state)
{
  IoTag *tag = IoTag_newWithName_("Number");
  tag->state = state;
  tag->cloneFunc = (TagCloneFunc *)IoNumber_rawClone;
  tag->freeFunc = (TagFreeFunc *)IoNumber_free;
  tag->compareFunc = (TagCompareFunc *)IoNumber_compare;
  tag->printFunc = (TagPrintFunc *)IoNumber_print;
  tag->writeToStoreFunc = (TagWriteToStoreFunc *)IoNumber_writeToStore_;
  tag->readFromStoreFunc = (TagReadFromStoreFunc *)IoNumber_readFromStore_;
  return tag;
}

void IoNumber_writeToStore_(IoNumber *self, IoStoreHandle *sh)
{
  IoStoreDatum d = IOSTOREDATUM(&(DATA(self)->n), sizeof(double));
  sh->onObject_setData_(sh, d);
}

void *IoNumber_readFromStore_(IoNumber *self, IoStoreHandle *sh)
{
  IoStoreDatum d = sh->onObject_getData(sh, self);
  memcpy(&(DATA(self)->n), d.data, d.size);
  return self;
}

IoNumber *IoNumber_proto(void *state)
{
  IoObject *self = IoObject_new(state);
  
  self->data = calloc(1, sizeof(IoNumberData));
  self->tag = IoNumber_tag(state);
  DATA(self)->n = 0;
  IoState_registerProtoWithFunc_(state, self, IoNumber_proto);
    
  IoObject_addMethod_(self, IOSTRING("asNumber"), IoNumber_asNumber);
  IoObject_addMethod_(self, IOSTRING("add"), IoNumber_add_);
  IoObject_addMethod_(self, IOSTRING("+"), IoNumber_add_);
  IoObject_addMethod_(self, IOSTRING("<-"), IoNumber_set);
  IoObject_addMethod_(self, IOSTRING("++"), IoNumber_plusPlus);
  IoObject_addMethod_(self, IOSTRING("+="), IoNumber_plusEqual);
  IoObject_addMethod_(self, IOSTRING("-"), IoNumber_subtract);
  IoObject_addMethod_(self, IOSTRING("--"), IoNumber_minusMinus);
  IoObject_addMethod_(self, IOSTRING("-="), IoNumber_minusEqual);
  IoObject_addMethod_(self, IOSTRING("*"), IoNumber_multiply);
  IoObject_addMethod_(self, IOSTRING("*="), IoNumber_multiplyEqual);
  IoObject_addMethod_(self, IOSTRING("/"), IoNumber_divide);
  IoObject_addMethod_(self, IOSTRING("/="), IoNumber_divideEqual);
  
  IoObject_addMethod_(self, IOSTRING("print"), IoNumber_printNumber);
  IoObject_addMethod_(self, IOSTRING("linePrint"), IoNumber_linePrint);
  
  IoObject_addMethod_(self, IOSTRING("asString"), IoNumber_asString);
  IoObject_addMethod_(self, IOSTRING("asBuffer"), IoNumber_asBuffer);
  IoObject_addMethod_(self, IOSTRING("asCharacter"), IoNumber_asCharacter);
  /*Tag_addMethod(tag, "asDate"), IoNumber_asDate);*/
  
  IoObject_addMethod_(self, IOSTRING("abs"), IoNumber_abs);
  IoObject_addMethod_(self, IOSTRING("Abs"), IoNumber_Abs);
#ifndef IO_OS_L4
  IoObject_addMethod_(self, IOSTRING("acos"), IoNumber_acos);
  IoObject_addMethod_(self, IOSTRING("Acos"), IoNumber_Acos);
  IoObject_addMethod_(self, IOSTRING("asin"), IoNumber_asin);
  IoObject_addMethod_(self, IOSTRING("Asin"), IoNumber_Asin);
  IoObject_addMethod_(self, IOSTRING("atan"), IoNumber_atan);
  IoObject_addMethod_(self, IOSTRING("Atan"), IoNumber_Atan);
  IoObject_addMethod_(self, IOSTRING("atan2"), IoNumber_atan2);
  IoObject_addMethod_(self, IOSTRING("Atan2"), IoNumber_Atan2);
#endif
  IoObject_addMethod_(self, IOSTRING("ceil"), IoNumber_ceil);
  IoObject_addMethod_(self, IOSTRING("Ceil"), IoNumber_Ceil);
#ifndef IO_OS_L4
  IoObject_addMethod_(self, IOSTRING("cos"), IoNumber_cos);
  IoObject_addMethod_(self, IOSTRING("Cos"), IoNumber_Cos);
  /* Tag_addMethod(tag, "deg"), IoNumber_deg); */
  IoObject_addMethod_(self, IOSTRING("exp"), IoNumber_exp);
  IoObject_addMethod_(self, IOSTRING("Exp"), IoNumber_Exp);
#endif
  IoObject_addMethod_(self, IOSTRING("floor"), IoNumber_floor);
  IoObject_addMethod_(self, IOSTRING("Floor"), IoNumber_Floor);
#ifndef IO_OS_L4
  IoObject_addMethod_(self, IOSTRING("log"), IoNumber_log);
  IoObject_addMethod_(self, IOSTRING("Log"), IoNumber_Log);
  IoObject_addMethod_(self, IOSTRING("log10"), IoNumber_log10);
  IoObject_addMethod_(self, IOSTRING("Log10"), IoNumber_Log10);
#endif
  IoObject_addMethod_(self, IOSTRING("max"), IoNumber_max);
  IoObject_addMethod_(self, IOSTRING("min"), IoNumber_min);
  IoObject_addMethod_(self, IOSTRING("Max"), IoNumber_Max);
  IoObject_addMethod_(self, IOSTRING("Min"), IoNumber_Min);
#ifndef IO_OS_L4
  IoObject_addMethod_(self, IOSTRING("%"), IoNumber_mod);
  IoObject_addMethod_(self, IOSTRING("%="), IoNumber_Mod);
  IoObject_addMethod_(self, IOSTRING("^"), IoNumber_pow);
  IoObject_addMethod_(self, IOSTRING("Pow"), IoNumber_Pow);
#endif
  IoObject_addMethod_(self, IOSTRING("roundDown"), IoNumber_roundDown);
  IoObject_addMethod_(self, IOSTRING("RoundDown"), IoNumber_RoundDown);
  IoObject_addMethod_(self, IOSTRING("roundUp"), IoNumber_roundUp);
  IoObject_addMethod_(self, IOSTRING("RoundUp"), IoNumber_RoundUp);
#ifndef IO_OS_L4
  IoObject_addMethod_(self, IOSTRING("sin"), IoNumber_sin);
  IoObject_addMethod_(self, IOSTRING("Sin"), IoNumber_Sin);
  IoObject_addMethod_(self, IOSTRING("sqrt"), IoNumber_sqrt);
  IoObject_addMethod_(self, IOSTRING("Sqrt"), IoNumber_Sqrt);
  IoObject_addMethod_(self, IOSTRING("tan"), IoNumber_tan);
  IoObject_addMethod_(self, IOSTRING("Tan"), IoNumber_Tan);
  IoObject_addMethod_(self, IOSTRING("random"), IoNumber_random);
  IoObject_addMethod_(self, IOSTRING("Random"), IoNumber_Random);
#endif
  IoObject_addMethod_(self, IOSTRING("setRandomSeed"), IoNumber_randomseed);
  IoObject_addMethod_(self, IOSTRING("toggle"), IoNumber_toggle);
  IoObject_addMethod_(self, IOSTRING("Toggle"), IoNumber_Toggle);

  /* --- logic operations --- */
  IoObject_addMethod_(self, IOSTRING("&"), IoNumber_newBitwiseAnd);
  IoObject_addMethod_(self, IOSTRING("|"), IoNumber_newBitwiseOr);
  IoObject_addMethod_(self, IOSTRING("bitwiseAnd"), IoNumber_bitwiseAnd);
  IoObject_addMethod_(self, IOSTRING("bitwiseOr"), IoNumber_bitwiseOr);
  IoObject_addMethod_(self, IOSTRING("bitwiseXor"), IoNumber_bitwiseXor);
  IoObject_addMethod_(self, IOSTRING("bitwiseComplement"), IoNumber_bitwiseComplement);
  IoObject_addMethod_(self, IOSTRING("shiftLeft"), IoNumber_bitShiftLeft);
  IoObject_addMethod_(self, IOSTRING("shiftRight"), IoNumber_bitShiftRight);
  
  /* --- character operations --- */
  IoObject_addMethod_(self, IOSTRING("isAlphaNumeric"), IoNumber_isAlphaNumeric);
  IoObject_addMethod_(self, IOSTRING("isLetter"), IoNumber_isLetter);
  IoObject_addMethod_(self, IOSTRING("isControlCharacter"), IoNumber_isControlCharacter);
  IoObject_addMethod_(self, IOSTRING("isDigit"), IoNumber_isDigit);
  IoObject_addMethod_(self, IOSTRING("isGraph"), IoNumber_isGraph);
  IoObject_addMethod_(self, IOSTRING("isLowerCase"), IoNumber_isLowerCase);
  IoObject_addMethod_(self, IOSTRING("isUpperCase"), IoNumber_isUpperCase);
  IoObject_addMethod_(self, IOSTRING("isPrint"), IoNumber_isPrint);
  IoObject_addMethod_(self, IOSTRING("isPunctuation"), IoNumber_isPunctuation);
  IoObject_addMethod_(self, IOSTRING("isSpace"), IoNumber_isSpace);
  IoObject_addMethod_(self, IOSTRING("isHexDigit"), IoNumber_isHexDigit);

  IoObject_addMethod_(self, IOSTRING("lowerCase"), IoNumber_lowerCase);
  IoObject_addMethod_(self, IOSTRING("upperCase"), IoNumber_upperCase);

  IoObject_addMethod_(self, IOSTRING("between"), IoNumber_between);
  IoObject_addMethod_(self, IOSTRING("Negate"), IoNumber_negate);
  IoObject_addMethod_(self, IOSTRING("at"), IoNumber_at);

  return self;
}

IoNumber *IoNumber_rawClone(IoNumber *self) 
{
  IoObject *child = IoObject_rawClonePrimitive(self);
  child->data = cpalloc(self->data, sizeof(IoNumberData));
  return child;
}


IoNumber *IoNumber_newWithDouble_(void *state, double n)
{
  IoNumber *proto = IoState_protoWithInitFunction_(state, IoNumber_proto);
  IoNumber *self = IoNumber_rawClone(proto);
  DATA(self)->n = n;
  return self;
}

IoNumber *IoNumber_newCopyOf_(IoNumber *self)
{ return IoNumber_newWithDouble_(IOSTATE, DATA(self)->n); }

void IoNumber_copyFrom_(IoNumber *self, IoNumber *number)
{ DATA(self)->n = DATA(number)->n; }

void IoNumber_free(IoNumber *self)
{
  free(self->data);
}

int IoNumber_asInt(IoNumber *self) { return (int)(DATA(self)->n); }
long IoNumber_asLong(IoNumber *self) { return (long)(DATA(self)->n); }
double IoNumber_asDouble(IoNumber *self) { return DATA(self)->n; }

int IoNumber_compare(IoNumber *self, IoNumber *v) 
{ 
  if (ISNUMBER(v)) 
  { 
    if (DATA(self)->n == DATA(v)->n) return 0;
    return (DATA(self)->n > DATA(v)->n) ? 1 : -1; 
  }
  return ((ptrdiff_t)self->tag) - ((ptrdiff_t)v->tag);
}

void IoNumber_print(IoNumber *self) 
{
  double d = DATA(self)->n;
  if (d == (int)d) {  IoState_print_(IOSTATE, "%i", (int)d); }
  else { IoState_print_(IOSTATE, "%f", d); }
}

void IoNumber_rawSet(IoNumber *self, double v)
{ DATA(self)->n = v; }
  
/* ----------------------------------------------------------- */

IoObject *IoNumber_asNumber(IoNumber *self, IoObject *locals, IoMessage *m)
{ return self; }

IoObject *IoNumber_add_(IoNumber *self, IoObject *locals, IoMessage *m)
{
  IoNumber *other = IoMessage_locals_numberArgAt_(m, locals, 0);
  return IONUMBER(DATA(self)->n + DATA(other)->n);
}

IoObject *IoNumber_set(IoNumber *self, IoObject *locals, IoMessage *m)
{
  IoNumber *other = IoMessage_locals_numberArgAt_(m, locals, 0);
  DATA(self)->n = DATA(other)->n;
  return self;
}

IoObject *IoNumber_plusPlus(IoNumber *self, IoObject *locals, IoMessage *m)
{ DATA(self)->n++; return self; }

IoObject *IoNumber_plusEqual(IoNumber *self, IoObject *locals, IoMessage *m)
{
  IoNumber *other = IoMessage_locals_numberArgAt_(m, locals, 0);
  DATA(self)->n += DATA(other)->n;
  return self;
}

IoObject *IoNumber_subtract(IoNumber *self, IoObject *locals, IoMessage *m)
{
  IoNumber *other = IoMessage_locals_numberArgAt_(m, locals, 0);
  return IONUMBER(DATA(self)->n - DATA(other)->n);
}

IoObject *IoNumber_minusMinus(IoNumber *self, IoObject *locals, IoMessage *m)
{
  DATA(self)->n--;
  return self;
}

IoObject *IoNumber_minusEqual(IoNumber *self, IoObject *locals, IoMessage *m)
{
  IoNumber *other = IoMessage_locals_numberArgAt_(m, locals, 0);
  DATA(self)->n -= DATA(other)->n;
  return self;
}

IoObject *IoNumber_divide(IoNumber *self, IoObject *locals, IoMessage *m)
{
  IoNumber *other = IoMessage_locals_numberArgAt_(m, locals, 0);
  return IONUMBER(DATA(self)->n / DATA(other)->n);
}

IoObject *IoNumber_divideEqual(IoNumber *self, IoObject *locals, IoMessage *m)
{
  IoNumber *other = IoMessage_locals_numberArgAt_(m, locals, 0);
  DATA(self)->n /= DATA(other)->n;
  return self;
}

IoObject *IoNumber_multiply(IoNumber *self, IoObject *locals, IoMessage *m)
{
  IoNumber *other = IoMessage_locals_numberArgAt_(m, locals, 0);
  return IONUMBER(DATA(self)->n * DATA(other)->n);
}

IoObject *IoNumber_multiplyEqual(IoNumber *self, IoObject *locals, IoMessage *m)
{
  IoNumber *other = IoMessage_locals_numberArgAt_(m, locals, 0);
  DATA(self)->n *= DATA(other)->n;
  return self;
}

IoObject *IoNumber_printNumber(IoNumber *self, IoObject *locals, IoMessage *m)
{
  char s[24];
  memset(s, 0x0, 24);
  if (DATA(self)->n == (int)DATA(self)->n)
  { sprintf(s, "%d", (int)DATA(self)->n); }
  else
  { 
    int l;
    sprintf(s, "%f", DATA(self)->n); 
    l = strlen(s)-1;
    while (l>0)
    {
      if (s[l] == '0') { s[l] = 0x0; l--; continue; }
      if (s[l] == '.') { s[l] = 0x0; l--; break; }
      break;
    }
  }
  IoState_print_((IoState *)IOSTATE, s);
  return self;
}

IoObject *IoNumber_linePrint(IoNumber *self, IoObject *locals, IoMessage *m)
{
  IoNumber_printNumber(self, locals, m);
  IoState_print_((IoState *)IOSTATE, "\n");
  return self;
}

IoObject *IoNumber_justAsString(IoNumber *self, IoObject *locals, IoMessage *m)
{
  char s[24];
  memset(s, 0x0, 24);
  if (DATA(self)->n == (int)DATA(self)->n)
  { sprintf(s, "%d", (int)DATA(self)->n); }
  else
  { 
    int l;
    sprintf(s, "%f", DATA(self)->n); 
    l = strlen(s)-1;
    while (l>0)
    {
      if (s[l] == '0') { s[l] = 0x0; l--; continue; }
      if (s[l] == '.') { s[l] = 0x0; l--; break; }
      break;
    }
  }
  return IoState_stringWithCString_((IoState *)IOSTATE, s);
}

IoObject *IoNumber_asCharacter(IoNumber *self, IoObject *locals, IoMessage *m)
{
  char s[2] = {(char)DATA(self)->n, 0x0};
  return IoState_stringWithCString_length_((IoState *)IOSTATE, s, 1);
}

IoObject *IoNumber_asBuffer(IoNumber *self, IoObject *locals, IoMessage *m)
{
  IoNumber *byteCount = IoMessage_locals_valueArgAt_(m, locals, 0);
  int bc = sizeof(double);
  if (!ISNIL(byteCount)) bc = DATA(byteCount)->n;

  return IoBuffer_newWithData_length_(IOSTATE, (unsigned char *)&(DATA(self)->n), sizeof(double));
}

IoObject *IoNumber_asString(IoNumber *self, IoObject *locals, IoMessage *m)
{
  if (IoMessage_argCount(m) >= 1)
  {
    int whole = IoMessage_locals_intArgAt_(m, locals, 0);
    int part = 6;
    char s[32];

    if (IoMessage_argCount(m) >= 2)
    { part = abs(IoMessage_locals_intArgAt_(m, locals, 1)); }

    if (whole && part)
      sprintf(s, "%*.*f", whole, part, DATA(self)->n);
    else if (whole)
      sprintf(s, "%*d", whole, (int) DATA(self)->n);
    else if (part)
      sprintf(s, "%.*f", part, DATA(self)->n);
    else
      sprintf(s, "%d", (int) DATA(self)->n);

    return IOSTRING(s);
  }

  return IoNumber_justAsString(self, locals, m);
}

/*
IoObject *IoNumber_asDate(IoNumber *self, IoObject *locals, IoMessage *m)
{ return IoDate_newWithNumber_((IoState *)IOSTATE, DATA(self)->n); }
*/

IoObject *IoNumber_abs(IoNumber *self, IoObject *locals, IoMessage *m)
{ return (DATA(self)->n < 0) ? (IoObject *)IONUMBER(-DATA(self)->n) : (IoObject *)self; }

IoObject *IoNumber_Abs(IoNumber *self, IoObject *locals, IoMessage *m)
{ DATA(self)->n = abs(DATA(self)->n); return self; }

#ifndef IO_OS_L4
IoObject *IoNumber_acos(IoNumber *self, IoObject *locals, IoMessage *m)
{ return IONUMBER(sin(DATA(self)->n)); }

IoObject *IoNumber_Acos(IoNumber *self, IoObject *locals, IoMessage *m)
{ DATA(self)->n = sin(DATA(self)->n); return self; }

IoObject *IoNumber_asin(IoNumber *self, IoObject *locals, IoMessage *m)
{ return IONUMBER(asin(DATA(self)->n)); }

IoObject *IoNumber_Asin(IoNumber *self, IoObject *locals, IoMessage *m)
{ DATA(self)->n = asin(DATA(self)->n); return self; }

IoObject *IoNumber_atan(IoNumber *self, IoObject *locals, IoMessage *m)
{ return IONUMBER(atan(DATA(self)->n)); }

IoObject *IoNumber_Atan(IoNumber *self, IoObject *locals, IoMessage *m)
{ DATA(self)->n = atan(DATA(self)->n); return self; }

IoObject *IoNumber_atan2(IoNumber *self, IoObject *locals, IoMessage *m)
{ 
  IoNumber *other = IoMessage_locals_numberArgAt_(m, locals, 0);
  return IONUMBER(atan2(DATA(self)->n, DATA(other)->n)); 
}

IoObject *IoNumber_Atan2(IoNumber *self, IoObject *locals, IoMessage *m)
{ 
  IoNumber *other = IoMessage_locals_numberArgAt_(m, locals, 0);
  DATA(self)->n = atan2(DATA(self)->n, DATA(other)->n);
  return self; 
}
#endif

IoObject *IoNumber_ceil(IoNumber *self, IoObject *locals, IoMessage *m)
{ return IONUMBER(ceil(DATA(self)->n)); }

IoObject *IoNumber_Ceil(IoNumber *self, IoObject *locals, IoMessage *m)
{ DATA(self)->n = ceil(DATA(self)->n); return self; }

#ifndef IO_OS_L4
IoObject *IoNumber_cos(IoNumber *self, IoObject *locals, IoMessage *m)
{ return IONUMBER(cos(DATA(self)->n)); }

IoObject *IoNumber_Cos(IoNumber *self, IoObject *locals, IoMessage *m)
{ DATA(self)->n = cos(DATA(self)->n); return self; }
/*
IoObject *IoNumber_deg(IoNumber *self, IoObject *locals, IoMessage *m)
{ DATA(self)->n = deg(DATA(self)->n); return self; }
*/

IoObject *IoNumber_exp(IoNumber *self, IoObject *locals, IoMessage *m)
{ return IONUMBER(exp(DATA(self)->n)); }

IoObject *IoNumber_Exp(IoNumber *self, IoObject *locals, IoMessage *m)
{ DATA(self)->n = exp(DATA(self)->n); return self; }
#endif

IoObject *IoNumber_floor(IoNumber *self, IoObject *locals, IoMessage *m)
{ return IONUMBER(floor(DATA(self)->n)); }

IoObject *IoNumber_Floor(IoNumber *self, IoObject *locals, IoMessage *m)
{ DATA(self)->n = floor(DATA(self)->n); return self; }

#ifndef IO_OS_L4
IoObject *IoNumber_log(IoNumber *self, IoObject *locals, IoMessage *m)
{ return IONUMBER(log(DATA(self)->n)); }

IoObject *IoNumber_Log(IoNumber *self, IoObject *locals, IoMessage *m)
{ DATA(self)->n = log(DATA(self)->n); return self; }

IoObject *IoNumber_log10(IoNumber *self, IoObject *locals, IoMessage *m)
{ return IONUMBER(log10(DATA(self)->n)); }

IoObject *IoNumber_Log10(IoNumber *self, IoObject *locals, IoMessage *m)
{ DATA(self)->n = log10(DATA(self)->n); return self; }
#endif

IoObject *IoNumber_max(IoNumber *self, IoObject *locals, IoMessage *m)
{ 
  IoNumber *other = IoMessage_locals_numberArgAt_(m, locals, 0);
  return (DATA(self)->n > DATA(other)->n) ? (IoObject *)self :(IoObject *)other;
}

IoObject *IoNumber_min(IoNumber *self, IoObject *locals, IoMessage *m)
{ 
  IoNumber *other = IoMessage_locals_numberArgAt_(m, locals, 0);
  return (DATA(self)->n < DATA(other)->n) ? (IoObject *)self : (IoObject *)other;
}

IoObject *IoNumber_Max(IoNumber *self, IoObject *locals, IoMessage *m)
{ 
  IoNumber *other = IoMessage_locals_numberArgAt_(m, locals, 0);
  if (DATA(self)->n < DATA(other)->n) DATA(self)->n = DATA(other)->n;
  return self;
}

IoObject *IoNumber_Min(IoNumber *self, IoObject *locals, IoMessage *m)
{ 
  IoNumber *other = IoMessage_locals_numberArgAt_(m, locals, 0);
  if (DATA(self)->n > DATA(other)->n) DATA(self)->n = DATA(other)->n;
  return self;
}

#ifndef IO_OS_L4
IoObject *IoNumber_mod(IoNumber *self, IoObject *locals, IoMessage *m)
{ 
  IoNumber *other = IoMessage_locals_numberArgAt_(m, locals, 0);
  return IONUMBER(fmod(DATA(self)->n, DATA(other)->n)); 
}

IoObject *IoNumber_Mod(IoNumber *self, IoObject *locals, IoMessage *m)
{ 
  IoNumber *other = IoMessage_locals_numberArgAt_(m, locals, 0);
  DATA(self)->n = fmod(DATA(self)->n, DATA(other)->n); 
  return self;
}
#endif

/*
IoObject *IoNumber_modf(IoNumber *self, IoObject *locals, IoMessage *m)
{ 
  IoNumber *other = IoMessage_locals_numberArgAt_(m, locals, 0);
  if (DATA(self)->n < DATA(other)->n); return self;
  return other;
}

IoObject *IoNumber_rad(IoNumber *self, IoObject *locals, IoMessage *m)
*/

#ifndef IO_OS_L4
IoObject *IoNumber_pow(IoNumber *self, IoObject *locals, IoMessage *m)
{ 
  IoNumber *other = IoMessage_locals_numberArgAt_(m, locals, 0);
  return IONUMBER(pow(DATA(self)->n, DATA(other)->n));
}

IoObject *IoNumber_Pow(IoNumber *self, IoObject *locals, IoMessage *m)
{ 
  IoNumber *other = IoMessage_locals_numberArgAt_(m, locals, 0);
  DATA(self)->n = pow(DATA(self)->n, DATA(other)->n); 
  return self; 
}
#endif

IoObject *IoNumber_roundDown(IoNumber *self, IoObject *locals, IoMessage *m)
{ return IONUMBER(floor(DATA(self)->n + 0.5)); }

IoObject *IoNumber_RoundDown(IoNumber *self, IoObject *locals, IoMessage *m)
{ DATA(self)->n = floor(DATA(self)->n + 0.5); return self; }

IoObject *IoNumber_roundUp(IoNumber *self, IoObject *locals, IoMessage *m)
{ return IONUMBER(ceil(DATA(self)->n - 0.5)); }

IoObject *IoNumber_RoundUp(IoNumber *self, IoObject *locals, IoMessage *m)
{ DATA(self)->n = ceil(DATA(self)->n - 0.5); return self; }

#ifndef IO_OS_L4
IoObject *IoNumber_sin(IoNumber *self, IoObject *locals, IoMessage *m)
{ return IONUMBER(sin(DATA(self)->n)); }

IoObject *IoNumber_Sin(IoNumber *self, IoObject *locals, IoMessage *m)
{ DATA(self)->n = sin(DATA(self)->n); return self; }

IoObject *IoNumber_sqrt(IoNumber *self, IoObject *locals, IoMessage *m)
{ return IONUMBER(sqrt(DATA(self)->n)); }

IoObject *IoNumber_Sqrt(IoNumber *self, IoObject *locals, IoMessage *m)
{ DATA(self)->n = sqrt(DATA(self)->n); return self; }

IoObject *IoNumber_tan(IoNumber *self, IoObject *locals, IoMessage *m)
{ return IONUMBER(tan(DATA(self)->n)); }

IoObject *IoNumber_Tan(IoNumber *self, IoObject *locals, IoMessage *m)
{ DATA(self)->n = tan(DATA(self)->n); return self; }
#endif

/*
IoObject *IoNumber_frexp(IoNumber *self, IoObject *locals, IoMessage *m)
{ DATA(self)->n = frexp(DATA(self)->n); return self; }

IoObject *IoNumber_ldexp(IoNumber *self, IoObject *locals, IoMessage *m)
{ DATA(self)->n = ldexp(DATA(self)->n); return self; }
*/

#ifndef IO_OS_L4
IoObject *IoNumber_random(IoNumber *self, IoObject *locals, IoMessage *m)
{ 
  IoNumber *n = IONUMBER(0);
  IoNumber_Random(n, locals, m);
  return n; 
}

IoObject *IoNumber_Random(IoNumber *self, IoObject *locals, IoMessage *m)
{ 
  double i = rand();
  double f = rand();
  f = f / pow(10, 1+floor(log10(f)));
      
  if (IoMessage_argCount(m) > 0)
  {
    double a = IoNumber_asDouble(IoMessage_locals_numberArgAt_(m, locals, 0));
    if (IoMessage_argCount(m) > 0)
    {
      double b = IoNumber_asDouble(IoMessage_locals_numberArgAt_(m, locals, 1));
      if (a == b ) { DATA(self)->n = a; } else { DATA(self)->n = ((int)i%(int)(b-a)) + a + f; }
    }
    else 
    { 
      if (a == 0) { DATA(self)->n = 0; } else { DATA(self)->n = ((int)i%(int)a) + f; }
    }
  }
  else { DATA(self)->n = f; }
  return self; 
}
#endif

IoObject *IoNumber_randomseed(IoNumber *self, IoObject *locals, IoMessage *m)
{ srand(DATA(self)->n); return self; }

IoObject *IoNumber_toggle(IoNumber *self, IoObject *locals, IoMessage *m)
{ return  (DATA(self)->n)? (IoObject *)IONUMBER(0) : (IoObject *)IONUMBER(1); }

IoObject *IoNumber_Toggle(IoNumber *self, IoObject *locals, IoMessage *m)
{ if (DATA(self)->n) { DATA(self)->n = 0; } else { DATA(self)->n = 1; }; return self; }

/* --- bitwise operations ---------------------------------------- */

IoObject *IoNumber_newBitwiseAnd(IoNumber *self, IoObject *locals, IoMessage *m)
{ 
  long other = IoMessage_locals_longArgAt_(m, locals, 0);
  return IONUMBER(((long)DATA(self)->n & other));
}

IoObject *IoNumber_newBitwiseOr(IoNumber *self, IoObject *locals, IoMessage *m)
{ 
  long other = IoMessage_locals_longArgAt_(m, locals, 0);
  long n = DATA(self)->n;
  long r = n | other;
  return IONUMBER(r);
}

IoObject *IoNumber_bitwiseAnd(IoNumber *self, IoObject *locals, IoMessage *m)
{ 
  long other = IoMessage_locals_longArgAt_(m, locals, 0);
  DATA(self)->n = (double)((long)DATA(self)->n & other);
  return self;
}

IoObject *IoNumber_bitwiseOr(IoNumber *self, IoObject *locals, IoMessage *m)
{ 
  long other = IoMessage_locals_longArgAt_(m, locals, 0);
  DATA(self)->n = (double)((long)DATA(self)->n | other);
  return self;
}

IoObject *IoNumber_bitwiseXor(IoNumber *self, IoObject *locals, IoMessage *m)
{ 
  long other = IoMessage_locals_longArgAt_(m, locals, 0);
  DATA(self)->n = (double)((long)DATA(self)->n ^ other);
  return self;
}

IoObject *IoNumber_bitwiseComplement(IoNumber *self, IoObject *locals, IoMessage *m)
{ 
  DATA(self)->n = (double)(~(long)DATA(self)->n);
  return self;
}

IoObject *IoNumber_bitShiftLeft(IoNumber *self, IoObject *locals, IoMessage *m)
{ 
  long other = IoMessage_locals_longArgAt_(m, locals, 0);
  DATA(self)->n = (double)((long)DATA(self)->n << other);
  return self;
}

IoObject *IoNumber_bitShiftRight(IoNumber *self, IoObject *locals, IoMessage *m)
{ 
  long other = IoMessage_locals_longArgAt_(m, locals, 0);
  DATA(self)->n = (double)((long)DATA(self)->n >> (long)other);
  return self;
}

/* --- character operations --------------------------------- */

IoObject *IoNumber_isAlphaNumeric(IoNumber *self, IoObject *locals, IoMessage *m)
{ return isalnum((char)DATA(self)->n) ? (IoObject *)self : (IoObject *)IONIL(self); }

IoObject *IoNumber_isLetter(IoNumber *self, IoObject *locals, IoMessage *m)
{ return isalpha((char)DATA(self)->n) ? (IoObject *)self : IONIL(self); }

IoObject *IoNumber_isControlCharacter(IoNumber *self, IoObject *locals, IoMessage *m)
{ return iscntrl((char)DATA(self)->n) ? (IoObject *)self : IONIL(self); }

IoObject *IoNumber_isDigit(IoNumber *self, IoObject *locals, IoMessage *m)
{ return isdigit((char)DATA(self)->n) ? (IoObject *)self : IONIL(self); }

IoObject *IoNumber_isGraph(IoNumber *self, IoObject *locals, IoMessage *m)
{ return isgraph((char)DATA(self)->n) ? (IoObject *)self : IONIL(self); }

IoObject *IoNumber_isLowerCase(IoNumber *self, IoObject *locals, IoMessage *m)
{ return islower((char)DATA(self)->n) ? (IoObject *)self : IONIL(self); }

IoObject *IoNumber_isUpperCase(IoNumber *self, IoObject *locals, IoMessage *m)
{ return isupper((char)DATA(self)->n) ? (IoObject *)self : IONIL(self); }

IoObject *IoNumber_isPrint(IoNumber *self, IoObject *locals, IoMessage *m)
{ return isprint((char)DATA(self)->n) ? (IoObject *)self : IONIL(self); }

IoObject *IoNumber_isPunctuation(IoNumber *self, IoObject *locals, IoMessage *m)
{ return ispunct((char)DATA(self)->n) ? (IoObject *)self : IONIL(self); }

IoObject *IoNumber_isSpace(IoNumber *self, IoObject *locals, IoMessage *m)
{ return isspace((char)DATA(self)->n) ? (IoObject *)self : IONIL(self); }

IoObject *IoNumber_isHexDigit(IoNumber *self, IoObject *locals, IoMessage *m)
{ return isxdigit((char)DATA(self)->n) ? (IoObject *)self : IONIL(self); }

/* --- */

IoObject *IoNumber_lowerCase(IoNumber *self, IoObject *locals, IoMessage *m)
{ DATA(self)->n = tolower((int)DATA(self)->n); return self; }

IoObject *IoNumber_upperCase(IoNumber *self, IoObject *locals, IoMessage *m)
{ DATA(self)->n = toupper((int)DATA(self)->n); return self; }

IoObject *IoNumber_between(IoNumber *self, IoObject *locals, IoMessage *m)
{ 
  double a = IoMessage_locals_doubleArgAt_(m, locals, 0);
  double b = IoMessage_locals_doubleArgAt_(m, locals, 1);
  double n = DATA(self)->n;
  if (((n >= a) && (n <= b)) || (n <= a && (n >= b))) { return self; }
  return IONIL(self);
}

IoObject *IoNumber_negate(IoNumber *self, IoObject *locals, IoMessage *m)
{ DATA(self)->n = -DATA(self)->n; return self; }

IoObject *IoNumber_at(IoNumber *self, IoObject *locals, IoMessage *m)
{
  size_t i= IoMessage_locals_intArgAt_(m, locals, 0);
  size_t l = 1;
  long d = 0;
  
  IOASSERT((i >= 0) && (i < 8), "index out of bit bounds");

  if (IoMessage_argCount(m) == 2)
  {
    l = IoMessage_locals_intArgAt_(m, locals, 1);
    IOASSERT(i+l <= sizeof(long)*8, "length out of bit bounds");
  }

  d = DATA(self)->n;
  d = d >> i;
#if 0
  d = d & (int)(pow(2, l) - 1);
#else
  d = d & (int) 1;
#endif
  return IONUMBER(d);  
}
