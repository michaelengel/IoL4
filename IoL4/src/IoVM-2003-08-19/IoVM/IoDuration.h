/*   Copyright (c) 2002, Steve Dekorte
 *   All rights reserved. See _BSDLicense.txt.
 *
 */

#ifndef IODURATION_DEFINED
#define IODURATION_DEFINED 1

#include <sys/types.h>
#include "base/Common.h"
#include "IoObject.h"

/* --- Extend Message object ------------------------------------------ */
void *IoMessage_locals_durationArgAt_(IoMessage *self, void *locals, int n);

#define ISDURATION(self) (self->tag->cloneFunc == (TagCloneFunc *)IoDuration_rawClone)

typedef IoObject IoDuration;

typedef struct
{
  int years;
  short days;
  short hours;
  short minutes;
  short seconds;
} IoDurationData;

IoDuration *IoDuration_proto(void *state);
IoDuration *IoDuration_rawClone(IoDuration *self);
IoDuration *IoDuration_new(void *state);
IoDuration *IoDuration_newWithNumber_(void *state, double t);
int IoDuration_compare(IoDuration *self, IoDuration *other);

void IoDuration_writeToStore_(IoDuration *self, IoStoreHandle *store);
void *IoDuration_readFromStore_(IoDuration *self, IoStoreHandle *store);

IoDuration *IoDuration_setTime_(IoDuration *self, double t);
time_t IoDuration_asTime(IoDuration *self);

/* ----------------------------------------------------------- */
IoObject *IoDuration_type(IoDuration *self, IoObject *locals, IoMessage *m);
IoObject *IoDuration_clone(IoDuration *self, IoObject *locals, IoMessage *m);

/* --- getters/setters -------------------------------------------------------- */
IoObject *IoDuration_years(IoDuration *self, IoObject *locals, IoMessage *m);
IoObject *IoDuration_setYears(IoDuration *self, IoObject *locals, IoMessage *m);

IoObject *IoDuration_days(IoDuration *self, IoObject *locals, IoMessage *m);
IoObject *IoDuration_setDays(IoDuration *self, IoObject *locals, IoMessage *m);

IoObject *IoDuration_hours(IoDuration *self, IoObject *locals, IoMessage *m);
IoObject *IoDuration_setHours(IoDuration *self, IoObject *locals, IoMessage *m);

IoObject *IoDuration_minutes(IoDuration *self, IoObject *locals, IoMessage *m);
IoObject *IoDuration_setMinutes(IoDuration *self, IoObject *locals, IoMessage *m);

IoObject *IoDuration_seconds(IoDuration *self, IoObject *locals, IoMessage *m);
IoObject *IoDuration_setSeconds(IoDuration *self, IoObject *locals, IoMessage *m);
IoObject *IoDuration_totalSeconds(IoDuration *self, IoObject *locals, IoMessage *m);

IoObject *IoDuration_setFromTo(IoDuration *self, IoObject *locals, IoMessage *m);
IoObject *IoDuration_printDuration(IoDuration *self, IoObject *locals, IoMessage *m);

/* --- conversion -------------------------------------------------------- */
IoObject *IoDuration_asString(IoDuration *self, IoObject *locals, IoMessage *m);
IoObject *IoDuration_asNumber(IoDuration *self, IoObject *locals, IoMessage *m);

IoObject *IoDuration_fromNumber(IoDuration *self, IoObject *locals, IoMessage *m);
IoObject *IoDuration_fromString(IoDuration *self, IoObject *locals, IoMessage *m);

/* --- math -------------------------------------------------------- */
IoObject *IoDuration_add(IoDuration *self, IoObject *locals, IoMessage *m);
IoObject *IoDuration_subtract(IoDuration *self, IoObject *locals, IoMessage *m);

#endif
