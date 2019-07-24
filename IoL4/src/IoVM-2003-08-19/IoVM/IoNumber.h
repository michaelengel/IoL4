/*   Copyright (c) 2002, Steve Dekorte
 *   All rights reserved. See _BSDLicense.txt.
 */

#ifndef IONUMBER_DEFINED
#define IONUMBER_DEFINED 1

#include "base/Common.h"
#include "IoState.h"
#include "IoObject.h"

#define ISNUMBER(self) (self->tag->cloneFunc == (TagCloneFunc *)IoNumber_rawClone)
#define IONUMBER(num) IoNumber_newWithDouble_((IoState*)IOSTATE, (double)num)
#define IONUMBERVALUE(self) ((IoNumberData *)self->data)->n

typedef IoObject IoNumber;

typedef struct
{
  double n;
} IoNumberData;

IoNumber *IoNumber_proto(void *state);
IoNumber *IoNumber_rawClone(IoNumber *self);
IoNumber *IoNumber_newWithDouble_(void *state, double n);
IoNumber *IoNumber_newCopyOf_(IoNumber *number);
void IoNumber_copyFrom_(IoNumber *self, IoNumber *number);

void IoNumber_writeToStore_(IoNumber *self, IoStoreHandle *store);
void *IoNumber_readFromStore_(IoNumber *self, IoStoreHandle *store);

void IoNumber_free(IoNumber *self);

int IoNumber_asInt(IoNumber *self);
long IoNumber_asLong(IoNumber *self);
double IoNumber_asDouble(IoNumber *self);
int IoNumber_compare(IoNumber *self, IoNumber *v);
void IoNumber_print(IoNumber *self);
void IoNumber_rawSet(IoNumber *self, double v);

/* ----------------------------------------------------------- */
IoObject *IoNumber_clone(IoNumber *self, IoObject *locals, IoMessage *m);
IoObject *IoNumber_asNumber(IoNumber *self, IoObject *locals, IoMessage *m);
IoObject *IoNumber_add_(IoNumber *self, IoObject *locals, IoMessage *m);
IoObject *IoNumber_set(IoNumber *self, IoObject *locals, IoMessage *m);
IoObject *IoNumber_plusPlus(IoNumber *self, IoObject *locals, IoMessage *m);
IoObject *IoNumber_plusEqual(IoNumber *self, IoObject *locals, IoMessage *m);
IoObject *IoNumber_subtract(IoNumber *self, IoObject *locals, IoMessage *m);
IoObject *IoNumber_minusMinus(IoNumber *self, IoObject *locals, IoMessage *m);
IoObject *IoNumber_minusEqual(IoNumber *self, IoObject *locals, IoMessage *m);
IoObject *IoNumber_divide(IoNumber *self, IoObject *locals, IoMessage *m);
IoObject *IoNumber_divideEqual(IoNumber *self, IoObject *locals, IoMessage *m);
IoObject *IoNumber_multiply(IoNumber *self, IoObject *locals, IoMessage *m);
IoObject *IoNumber_multiplyEqual(IoNumber *self, IoObject *locals, IoMessage *m);
IoObject *IoNumber_printNumber(IoNumber *self, IoObject *locals, IoMessage *m);
IoObject *IoNumber_linePrint(IoNumber *self, IoObject *locals, IoMessage *m);

IoObject *IoNumber_asBuffer(IoNumber *self, IoObject *locals, IoMessage *m);
IoObject *IoNumber_asString(IoNumber *self, IoObject *locals, IoMessage *m);
IoObject *IoNumber_justAsString(IoNumber *self, IoObject *locals, IoMessage *m);
IoObject *IoNumber_asCharacter(IoNumber *self, IoObject *locals, IoMessage *m);
/*IoObject *IoNumber_asDate(IoNumber *self, IoObject *locals, IoMessage *m);*/

/* --- no arg methods --- */
IoObject *IoNumber_abs(IoNumber *self, IoObject *locals, IoMessage *m);
IoObject *IoNumber_Abs(IoNumber *self, IoObject *locals, IoMessage *m);
IoObject *IoNumber_acos(IoNumber *self, IoObject *locals, IoMessage *m);
IoObject *IoNumber_Acos(IoNumber *self, IoObject *locals, IoMessage *m);
IoObject *IoNumber_asin(IoNumber *self, IoObject *locals, IoMessage *m);
IoObject *IoNumber_Asin(IoNumber *self, IoObject *locals, IoMessage *m);
IoObject *IoNumber_atan(IoNumber *self, IoObject *locals, IoMessage *m);
IoObject *IoNumber_Atan(IoNumber *self, IoObject *locals, IoMessage *m);
IoObject *IoNumber_atan2(IoNumber *self, IoObject *locals, IoMessage *m);
IoObject *IoNumber_Atan2(IoNumber *self, IoObject *locals, IoMessage *m);
IoObject *IoNumber_ceil(IoNumber *self, IoObject *locals, IoMessage *m);
IoObject *IoNumber_Ceil(IoNumber *self, IoObject *locals, IoMessage *m);
IoObject *IoNumber_cos(IoNumber *self, IoObject *locals, IoMessage *m);
IoObject *IoNumber_Cos(IoNumber *self, IoObject *locals, IoMessage *m);
IoObject *IoNumber_deg(IoNumber *self, IoObject *locals, IoMessage *m);
IoObject *IoNumber_exp(IoNumber *self, IoObject *locals, IoMessage *m);
IoObject *IoNumber_Exp(IoNumber *self, IoObject *locals, IoMessage *m);
IoObject *IoNumber_floor(IoNumber *self, IoObject *locals, IoMessage *m);
IoObject *IoNumber_Floor(IoNumber *self, IoObject *locals, IoMessage *m);
IoObject *IoNumber_log(IoNumber *self, IoObject *locals, IoMessage *m);
IoObject *IoNumber_Log(IoNumber *self, IoObject *locals, IoMessage *m);
IoObject *IoNumber_log10(IoNumber *self, IoObject *locals, IoMessage *m);
IoObject *IoNumber_Log10(IoNumber *self, IoObject *locals, IoMessage *m);
IoObject *IoNumber_max(IoNumber *self, IoObject *locals, IoMessage *m);
IoObject *IoNumber_min(IoNumber *self, IoObject *locals, IoMessage *m);
IoObject *IoNumber_Min(IoNumber *self, IoObject *locals, IoMessage *m);
IoObject *IoNumber_Max(IoNumber *self, IoObject *locals, IoMessage *m);
IoObject *IoNumber_mod(IoNumber *self, IoObject *locals, IoMessage *m);
IoObject *IoNumber_Mod(IoNumber *self, IoObject *locals, IoMessage *m);

/*
IoObject *IoNumber_mod(IoNumber *self, IoObject *locals, IoMessage *m);
IoObject *IoNumber_rad(IoNumber *self, IoObject *locals, IoMessage *m);
*/
IoObject *IoNumber_pow(IoNumber *self, IoObject *locals, IoMessage *m);
IoObject *IoNumber_Pow(IoNumber *self, IoObject *locals, IoMessage *m);

IoObject *IoNumber_roundDown(IoNumber *self, IoObject *locals, IoMessage *m);
IoObject *IoNumber_RoundDown(IoNumber *self, IoObject *locals, IoMessage *m);
IoObject *IoNumber_roundUp(IoNumber *self, IoObject *locals, IoMessage *m);
IoObject *IoNumber_RoundUp(IoNumber *self, IoObject *locals, IoMessage *m);

IoObject *IoNumber_sin(IoNumber *self, IoObject *locals, IoMessage *m);
IoObject *IoNumber_Sin(IoNumber *self, IoObject *locals, IoMessage *m);
IoObject *IoNumber_sqrt(IoNumber *self, IoObject *locals, IoMessage *m);
IoObject *IoNumber_Sqrt(IoNumber *self, IoObject *locals, IoMessage *m);
IoObject *IoNumber_tan(IoNumber *self, IoObject *locals, IoMessage *m);
IoObject *IoNumber_Tan(IoNumber *self, IoObject *locals, IoMessage *m);
/*
IoObject *IoNumber_frexp(IoNumber *self, IoObject *locals, IoMessage *m);
IoObject *IoNumber_ldexp(IoNumber *self, IoObject *locals, IoMessage *m);
*/
IoObject *IoNumber_random(IoNumber *self, IoObject *locals, IoMessage *m);
IoObject *IoNumber_Random(IoNumber *self, IoObject *locals, IoMessage *m);
IoObject *IoNumber_randomseed(IoNumber *self, IoObject *locals, IoMessage *m);
IoObject *IoNumber_toggle(IoNumber *self, IoObject *locals, IoMessage *m);
IoObject *IoNumber_Toggle(IoNumber *self, IoObject *locals, IoMessage *m);

/* --- bitwise operations ---------------------------------------- */
IoObject *IoNumber_newBitwiseAnd(IoNumber *self, IoObject *locals, IoMessage *m);
IoObject *IoNumber_newBitwiseOr(IoNumber *self, IoObject *locals, IoMessage *m);

IoObject *IoNumber_bitwiseAnd(IoNumber *self, IoObject *locals, IoMessage *m);
IoObject *IoNumber_bitwiseOr(IoNumber *self, IoObject *locals, IoMessage *m);
IoObject *IoNumber_bitwiseXor(IoNumber *self, IoObject *locals, IoMessage *m);
IoObject *IoNumber_bitwiseComplement(IoNumber *self, IoObject *locals, IoMessage *m);
IoObject *IoNumber_bitShiftLeft(IoNumber *self, IoObject *locals, IoMessage *m);
IoObject *IoNumber_bitShiftRight(IoNumber *self, IoObject *locals, IoMessage *m);

/* --- character operations --------------------------------- */
IoObject *IoNumber_isAlphaNumeric(IoNumber *self, IoObject *locals, IoMessage *m);
IoObject *IoNumber_isLetter(IoNumber *self, IoObject *locals, IoMessage *m);
IoObject *IoNumber_isControlCharacter(IoNumber *self, IoObject *locals, IoMessage *m);
IoObject *IoNumber_isDigit(IoNumber *self, IoObject *locals, IoMessage *m);
IoObject *IoNumber_isGraph(IoNumber *self, IoObject *locals, IoMessage *m);
IoObject *IoNumber_isLowerCase(IoNumber *self, IoObject *locals, IoMessage *m);
IoObject *IoNumber_isUpperCase(IoNumber *self, IoObject *locals, IoMessage *m);
IoObject *IoNumber_isPrint(IoNumber *self, IoObject *locals, IoMessage *m);
IoObject *IoNumber_isPunctuation(IoNumber *self, IoObject *locals, IoMessage *m);
IoObject *IoNumber_isSpace(IoNumber *self, IoObject *locals, IoMessage *m);
IoObject *IoNumber_isHexDigit(IoNumber *self, IoObject *locals, IoMessage *m);

IoObject *IoNumber_lowerCase(IoNumber *self, IoObject *locals, IoMessage *m);
IoObject *IoNumber_upperCase(IoNumber *self, IoObject *locals, IoMessage *m);

IoObject *IoNumber_between(IoNumber *self, IoObject *locals, IoMessage *m);
IoObject *IoNumber_negate(IoNumber *self, IoObject *locals, IoMessage *m);
IoObject *IoNumber_at(IoNumber *self, IoObject *locals, IoMessage *m);

#endif
