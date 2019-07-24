/*   Copyright (c) 2002, Steve Dekorte
 *   All rights reserved. See _BSDLicense.txt.
 *
 *   fromString method by Sean Perry
 */

#include "IoDate.h"
#include "IoState.h"
#include "IoCFunction.h"
#include "IoObject.h"
#include "IoNil.h"
#include "IoNumber.h"
#include "IoDuration.h"
#include "base/PortableStrptime.h"

typedef struct tm tm;

#define DATA(self) ((IoDateData *)self->data)

IoTag *IoDate_tag(void *state)
{
  IoTag *tag = IoTag_newWithName_("Date");
  tag->state = state;
  tag->cloneFunc   = (TagCloneFunc *)IoDate_rawClone;
  tag->compareFunc = (TagCompareFunc *)IoDate_compare;
  tag->markFunc    = (TagMarkFunc *)IoDate_mark;
  tag->writeToStoreFunc = (TagWriteToStoreFunc *)IoDate_writeToStore_;
  tag->readFromStoreFunc = (TagReadFromStoreFunc *)IoDate_readFromStore_;
  return tag;
}

void IoDate_writeToStore_(IoDate *self, IoStoreHandle *sh)
{
  IoStoreDatum d = IOSTOREDATUM(&(DATA(self)->time), sizeof(time_t));
  sh->onObject_setData_(sh, d);
}

void *IoDate_readFromStore_(IoDate *self, IoStoreHandle *sh)
{
  IoStoreDatum d = sh->onObject_getData(sh, self);
  memcpy(&(DATA(self)->time), d.data, d.size);
  /* need to handle format */
  return self;
}

IoDate *IoDate_proto(void *state)
{
  IoObject *self = IoObject_new(state);

  self->data = calloc(1, sizeof(IoDateData));
  self->tag = IoDate_tag(state);
  DATA(self)->time = 0;
  DATA(self)->format = IOSTRING("%Y-%m-%d %H:%M:%S %Z");
  IoState_registerProtoWithFunc_(state, self, IoDate_proto);
  
  IoObject_addMethod_(self, IOSTRING("now"), IoDate_now);
  IoObject_addMethod_(self, IOSTRING("clock"), IoDate_clock);
  IoObject_addMethod_(self, IOSTRING("secondsToRun"), IoDate_secondsToRun);
  IoObject_addMethod_(self, IOSTRING("year"), IoDate_year);
  IoObject_addMethod_(self, IOSTRING("setYear"), IoDate_setYear);
  IoObject_addMethod_(self, IOSTRING("month"), IoDate_month);
  IoObject_addMethod_(self, IOSTRING("setMonth"), IoDate_setMonth);
  IoObject_addMethod_(self, IOSTRING("day"), IoDate_day);
  IoObject_addMethod_(self, IOSTRING("setDay"), IoDate_setDay);
  IoObject_addMethod_(self, IOSTRING("hour"), IoDate_hour);
  IoObject_addMethod_(self, IOSTRING("setHour"), IoDate_setHour);
  IoObject_addMethod_(self, IOSTRING("minute"), IoDate_minute);
  IoObject_addMethod_(self, IOSTRING("setMinute"), IoDate_setMinute);
  IoObject_addMethod_(self, IOSTRING("second"), IoDate_second);
  IoObject_addMethod_(self, IOSTRING("setSecond"), IoDate_setSecond);
  IoObject_addMethod_(self, IOSTRING("isDaylightSavingsTime"), IoDate_isDaylightSavingsTime);
  IoObject_addMethod_(self, IOSTRING("zone"), IoDate_zone);
  IoObject_addMethod_(self, IOSTRING("secondsSince"), IoDate_secondsSince_);
  IoObject_addMethod_(self, IOSTRING("dateAfterSeconds"), IoDate_dateAfterSeconds_);
  IoObject_addMethod_(self, IOSTRING("asString"), IoDate_asString);
  IoObject_addMethod_(self, IOSTRING("asNumber"), IoDate_asNumber);
  IoObject_addMethod_(self, IOSTRING("fromNumber"), IoDate_fromNumber);
  IoObject_addMethod_(self, IOSTRING("fromString"), IoDate_fromString);
  IoObject_addMethod_(self, IOSTRING("print"), IoDate_printDate);
  IoObject_addMethod_(self, IOSTRING("+"), IoDate_add);
  IoObject_addMethod_(self, IOSTRING("-"), IoDate_subtract);
  IoObject_addMethod_(self, IOSTRING("+="), IoDate_addInPlace);
  IoObject_addMethod_(self, IOSTRING("-="), IoDate_subtractInPlace);
  IoObject_addMethod_(self, IOSTRING("setFormat"), IoDate_setFormat);
  IoObject_addMethod_(self, IOSTRING("format"), IoDate_format);
  return self;
}

IoDate *IoDate_rawClone(IoDate *self) 
{ 
  IoObject *child = IoObject_rawClonePrimitive(self);
  child->data = cpalloc(self, sizeof(IoDateData));
  DATA(child)->time = time(NULL);
  return child; 
}

IoDate *IoDate_new(void *state)
{
  IoDate *proto = IoState_protoWithInitFunction_(state, IoDate_proto);
  return IoDate_rawClone(proto);
}

IoDate *IoDate_newWithTime_(void *state, time_t t)
{
  IoDate *self = IoDate_new(state);
  DATA(self)->time = t;
  return self;
}

int IoDate_compare(IoDate *self, IoDate *date) 
{ 
  if (ISDATE(date)) { return (int)(difftime(DATA(self)->time, DATA(date)->time)*1000); }
  IoState_error_description_(IOSTATE, 0x0, "Io.Date", "Date method 'compare' requires a Date argument\n");
  return 0;
}

void IoDate_mark(IoDate *self)
{ 
  IoObject_makeGrayIfWhite((IoObject *)DATA(self)->format); 
}

/* ----------------------------------------------------------- */

IoObject *IoDate_now(IoDate *self, IoObject *locals, IoMessage *m)
{ 
  DATA(self)->time = time(NULL);
  return self; 
}

IoObject *IoDate_clock(IoDate *self, IoObject *locals, IoMessage *m)
{ return IONUMBER(((double)clock())/((double)CLOCKS_PER_SEC)); }

IoObject *IoDate_secondsToRun(IoDate *self, IoObject *locals, IoMessage *m)
{ 
  IoMessage_assertArgCount_receiver_(m, 1, self);
  {
  clock_t t = clock();
  IoMessage *doMessage = IoMessage_rawArgAt_(m, 0);
  IoMessage_locals_performOn_(doMessage, locals, locals);
  return IONUMBER(((double)(clock() - t))/((double)CLOCKS_PER_SEC));
  }
}

IoObject *IoDate_year(IoDate *self, IoObject *locals, IoMessage *m)
{ 
  tm *tp = localtime(&(DATA(self)->time));
  return IONUMBER(1900+tp->tm_year);
}

IoObject *IoDate_setYear(IoDate *self, IoObject *locals, IoMessage *m)
{ 
  tm *tp = localtime(&(DATA(self)->time));
  tp->tm_year = IoMessage_locals_intArgAt_(m, locals, 0);
  DATA(self)->time = mktime(tp);
  return self; 
}

IoObject *IoDate_month(IoDate *self, IoObject *locals, IoMessage *m)
{ return IONUMBER(localtime(&(DATA(self)->time))->tm_mon); }

IoObject *IoDate_setMonth(IoDate *self, IoObject *locals, IoMessage *m)
{ 
  tm *tp = localtime(&(DATA(self)->time));
  tp->tm_mon = IoMessage_locals_intArgAt_(m, locals, 0);
  DATA(self)->time = mktime(tp);
  return self; 
}

IoObject *IoDate_day(IoDate *self, IoObject *locals, IoMessage *m)
{ return IONUMBER(localtime(&(DATA(self)->time))->tm_yday); }

IoObject *IoDate_setDay(IoDate *self, IoObject *locals, IoMessage *m)
{ 
  tm *tp = localtime(&(DATA(self)->time));
  tp->tm_yday = IoMessage_locals_intArgAt_(m, locals, 0);
  DATA(self)->time = mktime(tp);
  return self; 
}

IoObject *IoDate_hour(IoDate *self, IoObject *locals, IoMessage *m)
{ return IONUMBER(localtime(&(DATA(self)->time))->tm_hour); }

IoObject *IoDate_setHour(IoDate *self, IoObject *locals, IoMessage *m)
{ 
  tm *tp = localtime(&(DATA(self)->time));
  tp->tm_hour = IoMessage_locals_intArgAt_(m, locals, 0);
  DATA(self)->time = mktime(tp);
  return self; 
}

IoObject *IoDate_minute(IoDate *self, IoObject *locals, IoMessage *m)
{ return IONUMBER(localtime(&(DATA(self)->time))->tm_min); }

IoObject *IoDate_setMinute(IoDate *self, IoObject *locals, IoMessage *m)
{ 
  tm *tp = localtime(&(DATA(self)->time));
  tp->tm_min = IoMessage_locals_intArgAt_(m, locals, 0);
  DATA(self)->time = mktime(tp);
  return self; 
}

IoObject *IoDate_second(IoDate *self, IoObject *locals, IoMessage *m)
{ return IONUMBER(localtime(&(DATA(self)->time))->tm_sec); }

IoObject *IoDate_setSecond(IoDate *self, IoObject *locals, IoMessage *m)
{ 
  tm *tp = localtime(&(DATA(self)->time));
  tp->tm_sec = IoMessage_locals_intArgAt_(m, locals, 0);
  DATA(self)->time = mktime(tp);
  return self; 
}

IoObject *IoDate_zone(IoDate *self, IoObject *locals, IoMessage *m)
{ 
  char s[32];
  strftime(s, 32,"%Z", (void *)localtime(&(DATA(self)->time)));
  return IOSTRING(s);
}

IoObject *IoDate_isDaylightSavingsTime(IoDate *self, IoObject *locals, IoMessage *m)
{ return (localtime(&(DATA(self)->time))->tm_isdst) ? (IoObject *)self : IONIL(self); }

IoObject *IoDate_secondsSince_(IoDate *self, IoObject *locals, IoMessage *m)
{ 
  IoDate *date = IoMessage_locals_dateArgAt_(m, locals, 0); 
  return IONUMBER(difftime(DATA(self)->time, DATA(date)->time));
}

IoObject *IoDate_dateAfterSeconds_(IoDate *self, IoObject *locals, IoMessage *m)
{ 
  IoDate *newDate = IoDate_new(IOSTATE);
  DATA(newDate)->time += IoMessage_locals_intArgAt_(m, locals, 0);
  return newDate;
}

IoObject *IoDate_asString(IoDate *self, IoObject *locals, IoMessage *m)
{
  char s[1024];
  IoString *format;
  if (IoMessage_argCount(m) == 1)
  { format = IoMessage_locals_stringArgAt_(m, locals, 0); }
  else
  { format = DATA(self)->format; }
  strftime(s, 1024, CSTRING(format), localtime(&(DATA(self)->time)));
  return IOSTRING(s);
}

IoObject *IoDate_printDate(IoDate *self, IoObject *locals, IoMessage *m)
{
  IoString *s = (IoString *)IoDate_asString(self, locals, m);
  IoString_printString(s, locals, m);
  return self; 
}

IoObject *IoDate_asNumber(IoDate *self, IoObject *locals, IoMessage *m)
{ return IONUMBER(DATA(self)->time); }

IoObject *IoDate_fromNumber(IoDate *self, IoObject *locals, IoMessage *m)
{ 
  DATA(self)->time = IoMessage_locals_intArgAt_(m, locals, 0);
  return self;
}

IoObject *IoDate_fromString(IoDate *self, IoObject *locals, IoMessage *m)
{
  IoMessage_assertArgCount_receiver_(m, 2, self);
  {
  IoString *date_input = IoMessage_locals_stringArgAt_(m, locals, 0);
  IoString *format = IoMessage_locals_stringArgAt_(m, locals, 1);
  time_t tmp = time(NULL);
  struct tm *t = localtime(&tmp);
  io_strptime(CSTRING(date_input), CSTRING(format), t);
  DATA(self)->time = mktime(t);
  }
  return self;
}


IoObject *IoDate_setFormat(IoDate *self, IoObject *locals, IoMessage *m)
{
  IoString *s = IoMessage_locals_stringArgAt_(m, locals, 0);
  DATA(self)->format = IOREF(s);
  return self;
}

IoObject *IoDate_format(IoDate *self, IoObject *locals, IoMessage *m)
{ return DATA(self)->format; }

/* --- Durations -------------------------------------------------------- */
IoObject *IoDate_subtract(IoDate *self, IoObject *locals, IoMessage *m)
{
  IoObject *v = IoMessage_locals_valueArgAt_(m, locals, 0);
  if (ISDATE(v))
  {
    IoDate *date = (IoDate *)v;
    double d = DATA(self)->time - DATA(date)->time;
    return IoDuration_newWithNumber_(IOSTATE, d);
  } else if (ISDURATION(v))
  {
    IoDuration *duration = (IoDuration *)v;
    double d = DATA(self)->time - IoDuration_asTime(duration);
    IoDate *newDate = IoDate_rawClone(self);
    DATA(newDate)->time = d;
    return newDate;  
  }
  IOASSERT(1, "Date or Duration argument required");
  return IONIL(self);
}

IoObject *IoDate_subtractInPlace(IoDate *self, IoObject *locals, IoMessage *m)
{
  IoDuration *duration = IoMessage_locals_durationArgAt_(m, locals, 0);
  double t = IoDuration_asTime(duration);
  DATA(self)->time -= t;
  return self;
}

IoObject *IoDate_addInPlace(IoDate *self, IoObject *locals, IoMessage *m)
{
  IoDuration *duration = IoMessage_locals_durationArgAt_(m, locals, 0);
  double t = IoDuration_asTime(duration);
  DATA(self)->time += t;
  return self;
}

IoObject *IoDate_add(IoDate *self, IoObject *locals, IoMessage *m)
{
  IoDate *newDate = IoDate_rawClone(self);
  return IoDate_addInPlace(newDate, locals, m);
}
