/*   Copyright (c) 2002, Steve Dekorte
 *   All rights reserved. See _BSDLicense.txt.
 * 
 */

#include "IoDuration.h"
#include "IoState.h"
#include "IoCFunction.h"
#include "IoObject.h"
#include "IoNil.h"
#include "IoNumber.h"
#include <time.h>

#define DATA(self) ((IoDurationData *)self->data)

/* --- Extend Message object ------------------------------------------ */

void *IoMessage_locals_durationArgAt_(IoMessage *self, void *locals, int n)
{
  IoObject *v = IoMessage_locals_valueArgAt_(self, locals, n);
  if (!ISDURATION(v)) IoMessage_locals_numberArgAt_errorForType_(self, locals, n, "Duration");
  return v;
}

/* --------------------------------------------- */

typedef struct tm tm;

IoTag *IoDuration_tag(void *state)
{
  IoTag *tag = IoTag_newWithName_("Duration");
  tag->state = state;
  tag->cloneFunc = (TagCloneFunc *)IoDuration_rawClone;
  tag->compareFunc = (TagCompareFunc *)IoDuration_compare;
  tag->writeToStoreFunc = (TagWriteToStoreFunc *)IoDuration_writeToStore_;
  tag->readFromStoreFunc = (TagReadFromStoreFunc *)IoDuration_readFromStore_;
  return tag;
}

void IoDuration_writeToStore_(IoDuration *self, IoStoreHandle *sh)
{
  time_t t = IoDuration_asTime(self);
  IoStoreDatum d = IOSTOREDATUM(&t, sizeof(time_t));
  sh->onObject_setData_(sh, d);
}

void *IoDuration_readFromStore_(IoDuration *self, IoStoreHandle *sh)
{
  IoStoreDatum d = sh->onObject_getData(sh, self);
  time_t t;
  memcpy(&t, d.data, d.size);
  IoDuration_setTime_(self, t);
  return self;
}

IoDuration *IoDuration_proto(void *state)
{
  IoObject *self = IoObject_new(state);

  self->data = calloc(1, sizeof(IoDurationData));
  self->tag = IoDuration_tag(state);
  IoState_registerProtoWithFunc_(state, self, IoDuration_proto);
  
  IoObject_addMethod_(self, IOSTRING("years"), IoDuration_years);
  IoObject_addMethod_(self, IOSTRING("setYears"), IoDuration_setYears);
  IoObject_addMethod_(self, IOSTRING("days"), IoDuration_days);
  IoObject_addMethod_(self, IOSTRING("setDays"), IoDuration_setDays);
  IoObject_addMethod_(self, IOSTRING("hours"), IoDuration_hours);
  IoObject_addMethod_(self, IOSTRING("setHours"), IoDuration_setHours);
  IoObject_addMethod_(self, IOSTRING("minutes"), IoDuration_minutes);
  IoObject_addMethod_(self, IOSTRING("setMinutes"), IoDuration_setMinutes);
  IoObject_addMethod_(self, IOSTRING("seconds"), IoDuration_seconds);
  IoObject_addMethod_(self, IOSTRING("setSeconds"), IoDuration_setSeconds);

  IoObject_addMethod_(self, IOSTRING("asString"), IoDuration_asString);
  IoObject_addMethod_(self, IOSTRING("asNumber"), IoDuration_asNumber);
  
  IoObject_addMethod_(self, IOSTRING("fromNumber"), IoDuration_fromNumber);
  /*Tag_addMethod(tag, "fromString"), IoDuration_fromString);*/
  
  IoObject_addMethod_(self, IOSTRING("print"), IoDuration_printDuration);
  IoObject_addMethod_(self, IOSTRING("+="), IoDuration_add);
  IoObject_addMethod_(self, IOSTRING("-="), IoDuration_subtract);
  return self;
}

IoDuration *IoDuration_rawClone(IoDuration *self) 
{ 
  IoObject *child = IoObject_rawClonePrimitive(self);
  child->data = cpalloc(self->data, sizeof(IoDurationData));
  return child; 
}

IoDuration *IoDuration_new(void *state)
{
  IoDuration *proto = IoState_protoWithInitFunction_(state, IoDuration_proto);
  return IoDuration_rawClone(proto);
}

IoDuration *IoDuration_newWithNumber_(void *state, double t)
{
  IoDuration *self = IoDuration_new(state);
  IoDuration_setTime_(self, t);
  return self;
}

int IoDuration_compare(IoDuration *self, IoDuration *duration) 
{ 
  if (ISDURATION(duration)) 
  { 
    time_t t1 = IoDuration_asTime(self);
    time_t t2 = IoDuration_asTime(duration);
    return (int)(t1 - t2);
  }
  IoState_error_description_(IOSTATE, 0x0, "Io.Duration", "Duration method 'compare' requires a Duration argument\n");
  return 0;
}

#define SECONDS_IN_YEAR   (60 * 60 * 24 * 365)
#define SECONDS_IN_DAY    (60 * 60 * 24)
#define SECONDS_IN_HOUR   (60 * 60)
#define SECONDS_IN_MINUTE (60)

IoDuration *IoDuration_setTime_(IoDuration *self, double t)
{
  DATA(self)->years   = t / SECONDS_IN_YEAR;   t -= DATA(self)->years   * SECONDS_IN_YEAR;
  DATA(self)->days    = t / SECONDS_IN_DAY;    t -= DATA(self)->days    * SECONDS_IN_DAY;
  DATA(self)->hours   = t / SECONDS_IN_HOUR;   t -= DATA(self)->hours   * SECONDS_IN_HOUR;
  DATA(self)->minutes = t / SECONDS_IN_MINUTE; t -= DATA(self)->minutes * SECONDS_IN_MINUTE;
  DATA(self)->seconds = t;
  return self;
}

time_t IoDuration_asTime(IoDuration *self)
{
  double t = DATA(self)->years * SECONDS_IN_YEAR;
  t += DATA(self)->days * SECONDS_IN_DAY;
  t += DATA(self)->hours * SECONDS_IN_HOUR;
  t += DATA(self)->minutes * SECONDS_IN_MINUTE;
  t += DATA(self)->seconds;
  return (time_t)t;
}

/* --- years -------------------------------------------------------- */

IoObject *IoDuration_years(IoDuration *self, IoObject *locals, IoMessage *m)
{ return IONUMBER(DATA(self)->years); }

IoObject *IoDuration_setYears(IoDuration *self, IoObject *locals, IoMessage *m)
{ 
  DATA(self)->years = IoMessage_locals_intArgAt_(m, locals, 0);
  return self; 
}

/* --- days -------------------------------------------------------- */

IoObject *IoDuration_days(IoDuration *self, IoObject *locals, IoMessage *m)
{ return IONUMBER(DATA(self)->days); }

IoObject *IoDuration_setDays(IoDuration *self, IoObject *locals, IoMessage *m)
{ 
  DATA(self)->days = IoMessage_locals_intArgAt_(m, locals, 0);
  return self; 
}

/* --- hours -------------------------------------------------------- */

IoObject *IoDuration_hours(IoDuration *self, IoObject *locals, IoMessage *m)
{ return IONUMBER(DATA(self)->hours); }

IoObject *IoDuration_setHours(IoDuration *self, IoObject *locals, IoMessage *m)
{ 
  DATA(self)->hours = IoMessage_locals_intArgAt_(m, locals, 0);
  return self; 
}

/* --- minutes -------------------------------------------------------- */

IoObject *IoDuration_minutes(IoDuration *self, IoObject *locals, IoMessage *m)
{ return IONUMBER(DATA(self)->minutes); }

IoObject *IoDuration_setMinutes(IoDuration *self, IoObject *locals, IoMessage *m)
{ 
  DATA(self)->minutes = IoMessage_locals_intArgAt_(m, locals, 0);
  return self; 
}

/* --- seconds -------------------------------------------------------- */

IoObject *IoDuration_seconds(IoDuration *self, IoObject *locals, IoMessage *m)
{ return IONUMBER(DATA(self)->seconds); }

IoObject *IoDuration_setSeconds(IoDuration *self, IoObject *locals, IoMessage *m)
{ 
  DATA(self)->seconds = IoMessage_locals_intArgAt_(m, locals, 0);
  return self; 
}

/* --- conversion -------------------------------------------------------- */

IoObject *IoDuration_asString(IoDuration *self, IoObject *locals, IoMessage *m)
{
  ByteArray *ba;
  IoString *format;
  if (IoMessage_argCount(m) == 1)
  { format = IoMessage_locals_stringArgAt_(m, locals, 0); }
  else
  { format = IOSTRING("%H:%M:%S"); }
  
  {
    char s[128];
    ba = ByteArray_clone(IoString_rawByteArray(format));
    
    sprintf(s, "%02i", DATA(self)->years);
    ByteArray_replaceCString_withCString_(ba, "%Y", s);

    sprintf(s, "%02i", DATA(self)->days);
    ByteArray_replaceCString_withCString_(ba, "%j", s);

    sprintf(s, "%02i", DATA(self)->hours);
    ByteArray_replaceCString_withCString_(ba, "%H", s);
        
    sprintf(s, "%02i", DATA(self)->minutes);
    ByteArray_replaceCString_withCString_(ba, "%M", s);

    sprintf(s, "%02i", DATA(self)->seconds);
    ByteArray_replaceCString_withCString_(ba, "%S", s);
  }
  return IoState_stringWithByteArray_copy_(IOSTATE, ba, 0);
}

IoObject *IoDuration_printDuration(IoDuration *self, IoObject *locals, IoMessage *m)
{
  IoString *s = (IoString *)IoDuration_asString(self, locals, m);
  IoString_printString(s, locals, m);
  return self; 
}

IoObject *IoDuration_asNumber(IoDuration *self, IoObject *locals, IoMessage *m)
{ return IONUMBER(IoDuration_asTime(self)); }

IoObject *IoDuration_fromNumber(IoDuration *self, IoObject *locals, IoMessage *m)
{ 
  IoDuration_setTime_(self, IoMessage_locals_intArgAt_(m, locals, 0));
  return self;
}

/* --- math -------------------------------------------------------- */

IoObject *IoDuration_add(IoDuration *self, IoObject *locals, IoMessage *m)
{ 
  IoDuration *d = IoMessage_locals_durationArgAt_(m, locals, 0);
  time_t t = IoDuration_asTime(self) + IoDuration_asTime(d);
  IoDuration_setTime_(self, t);
  return self; 
}

IoObject *IoDuration_subtract(IoDuration *self, IoObject *locals, IoMessage *m)
{ 
  IoDuration *d = IoMessage_locals_durationArgAt_(m, locals, 0);
  time_t t = IoDuration_asTime(self) + IoDuration_asTime(d);
  IoDuration_setTime_(self, t);
  return self; 
}


