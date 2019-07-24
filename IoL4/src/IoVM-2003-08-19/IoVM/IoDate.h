/*   Copyright (c) 2002, Steve Dekorte
 *   All rights reserved. See _BSDLicense.txt.
 */

#ifndef IODATE_DEFINED
#define IODATE_DEFINED 1

#include "base/Common.h"
#include "IoObject.h"
#include "IoString.h"
#include <time.h>

#define ISDATE(self) (self->tag->cloneFunc == (TagCloneFunc *)IoDate_rawClone)
#define IODATE(time) IoDate_newWithTime_((IoState*)IOSTATE, (time_t)time)

typedef IoObject IoDate;

typedef struct
{
  time_t time;
  IoString *format;
} IoDateData;

IoDate *IoDate_proto(void *state);
IoDate *IoDate_new(void *state);
IoDate *IoDate_newWithTime_(void *state, time_t t);
IoDate *IoDate_rawClone(IoDate *self);
int IoDate_compare(IoDate *self, IoDate *other);
void IoDate_mark(IoDate *self);

void IoDate_writeToStore_(IoDate *self, IoStoreHandle *store);
void *IoDate_readFromStore_(IoDate *self, IoStoreHandle *store);

/* ----------------------------------------------------------- */
IoObject *IoDate_type(IoDate *self, IoObject *locals, IoMessage *m);
IoObject *IoDate_clone(IoDate *self, IoObject *locals, IoMessage *m);
IoObject *IoDate_now(IoDate *self, IoObject *locals, IoMessage *m);
IoObject *IoDate_clock(IoDate *self, IoObject *locals, IoMessage *m);
IoObject *IoDate_secondsToRun(IoDate *self, IoObject *locals, IoMessage *m);
IoObject *IoDate_year(IoDate *self, IoObject *locals, IoMessage *m);
IoObject *IoDate_setYear(IoDate *self, IoObject *locals, IoMessage *m);
IoObject *IoDate_month(IoDate *self, IoObject *locals, IoMessage *m);
IoObject *IoDate_setMonth(IoDate *self, IoObject *locals, IoMessage *m);
IoObject *IoDate_day(IoDate *self, IoObject *locals, IoMessage *m);
IoObject *IoDate_setDay(IoDate *self, IoObject *locals, IoMessage *m);
IoObject *IoDate_hour(IoDate *self, IoObject *locals, IoMessage *m);
IoObject *IoDate_setHour(IoDate *self, IoObject *locals, IoMessage *m);
IoObject *IoDate_minute(IoDate *self, IoObject *locals, IoMessage *m);
IoObject *IoDate_setMinute(IoDate *self, IoObject *locals, IoMessage *m);
IoObject *IoDate_second(IoDate *self, IoObject *locals, IoMessage *m);
IoObject *IoDate_setSecond(IoDate *self, IoObject *locals, IoMessage *m);
IoObject *IoDate_isDaylightSavingsTime(IoDate *self, IoObject *locals, IoMessage *m);
IoObject *IoDate_zone(IoDate *self, IoObject *locals, IoMessage *m);

IoObject *IoDate_secondsSince_(IoDate *self, IoObject *locals, IoMessage *m);
IoObject *IoDate_dateAfterSeconds_(IoDate *self, IoObject *locals, IoMessage *m);
IoObject *IoDate_printDate(IoDate *self, IoObject *locals, IoMessage *m);

IoObject *IoDate_asString(IoDate *self, IoObject *locals, IoMessage *m);
IoObject *IoDate_asNumber(IoDate *self, IoObject *locals, IoMessage *m);

IoObject *IoDate_fromNumber(IoDate *self, IoObject *locals, IoMessage *m);
IoObject *IoDate_fromString(IoDate *self, IoObject *locals, IoMessage *m);

IoObject *IoDate_setFormat(IoDate *self, IoObject *locals, IoMessage *m);
IoObject *IoDate_format(IoDate *self, IoObject *locals, IoMessage *m);

/* --- Durations -------------------------------------------------------- */
IoObject *IoDate_subtract(IoDate *self, IoObject *locals, IoMessage *m);
IoObject *IoDate_subtractInPlace(IoDate *self, IoObject *locals, IoMessage *m);
IoObject *IoDate_add(IoDate *self, IoObject *locals, IoMessage *m);
IoObject *IoDate_addInPlace(IoDate *self, IoObject *locals, IoMessage *m);

#endif
