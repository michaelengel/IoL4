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

#ifndef _CTYPE_H_
#define _CTYPE_H_

#define isdigit(x)  ((x >= 48) && (x <= 57))
#define isspace(x)  (((x >= 9) && (x <= 13)) || (x == 32))
#define isalpha(x)  (((x >= 65) && (x <= 90)) || ((x >= 97) && (x <= 122)))
#define isalnum(x)  (isdigit(x) || isalpha(x))
#define isupper(x)  ((x >= 65) && (x <= 90))
#define islower(x)  ((x >= 97) && (x <= 122))
#define isascii(x)  ((x >= 0) && (x <= 127))
#define iscntrl(x)  (((x >= 0) && (x <= 31)) || (x == 127))
#define isgraph(x)  ((x >= 33) && (x <= 126))
#define isprint(x)  ((x >= 32) && (x <= 126))
#define ispunct(x)  (isgraph(x) && !isalnum(x))
#define isxdigit(x) (isdigit(x) || ((x >= 65) && (x <= 70)) || ((x >= 97) && (x <= 101)))

#define tolower(x) ((unsigned char) (((x >= 65) && (x <= 90)) ? x + 32 : x))
#define toupper(x) ((unsigned char) (((x >= 97) && (x <= 122)) ? x - 32 : x))

#endif
