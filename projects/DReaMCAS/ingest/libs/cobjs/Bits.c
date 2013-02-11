/*
 * The contents of this file are subject to the terms
 * of the Common Development and Distribution License
 * (the "License").  You may not use this file except
 * in compliance with the License.
 *
 * You can obtain a copy of the license at
 * http://www.opensource.org/licenses/cddl1.php
 * See the License for the specific language governing
 * permissions and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL
 * HEADER in each file and include the License file at
 * http://www.opensource.org/licenses/cddl1.php.  If 
 * applicable, add the following below this CDDL HEADER, 
 * with the fields enclosed by brackets "[]" replaced 
 * with your own identifying information: 
 * Portions Copyright [yyyy]
 * [name of copyright owner]
 */ 

/*
 * $(@)Bits.c $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:30 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1998 by Sun Microsystems, Inc.
 */

/*
 * Bits.c -- Description of Bits
 */

#pragma ident "@(#)Bits.c 1.3	99/05/28 SMI"

#if	!defined(BITS_HEADER)
#define	BITS_BODY
#define	BITS_INLINE		extern
#include "cobjs/Bits.h"
#endif					   /* !defined(BITS_HEADER) */

#include <stdlib.h>
#include <values.h>

#include "cobjs/RetCode.h"
#include "cobjs/Types.h"

/*************************************************************************
 * Defines
 *************************************************************************/

/*************************************************************************
 * Private types and prototypes referenced from inlines
 *************************************************************************/

/*
 * Use INLINE_PRIVATE if non-inline-able, define in Non-inlinable section
 * Use static if inline-able, define in Private Inline-able section
 *
 * INLINE_PRIVATE void bitsInlinePrivate(void);
 */

/*************************************************************************
 * Private class data referenced from inlines
 *************************************************************************/

/*************************************************************************
 * Inline Functions
 *************************************************************************/

/*
 * BITS_INLINE void *
 * bitsInlineRoutine(void)
 * {
 * }
 */

/*************************************************************************
 * Private Inlineable Functions Called From Inlines
 *************************************************************************/

#if	!defined(BITS_HEADER)

/*************************************************************************
 * Private types
 *************************************************************************/

/*************************************************************************
 * Private function prototypes
 *************************************************************************/

/*************************************************************************
 * Private data
 *************************************************************************/

static s8 ff0Bit[256] = {
/*  00	01  02	03  04	05  06	07  08	09  0A	0B  0C	0D  0E	0F */
     0,	 1,  0,  2,  0,  1,  0,  3,  0,  1,  0,  2,  0,  1,  0,  4,
/*  10	11  12	13  14	15  16	17  18	19  1A	1B  1C	1D  1E	1F */
     0,	 1,  0,  2,  0,  1,  0,  3,  0,  1,  0,  2,  0,  1,  0,  5,
/*  20	21  22	23  24	25  26	27  28	29  2A	2B  2C	2D  2E	2F */
     0,	 1,  0,  2,  0,  1,  0,  3,  0,  1,  0,  2,  0,  1,  0,  4,
/*  30	31  32	33  34	35  36	37  38	39  3A	3B  3C	3D  3E	3F */
     0,	 1,  0,  2,  0,  1,  0,  3,  0,  1,  0,  2,  0,  1,  0,  6,
/*  40	41  42	43  44	45  46	47  48	49  4A	4B  4C	4D  4E	4F */
     0,	 1,  0,  2,  0,  1,  0,  3,  0,  1,  0,  2,  0,  1,  0,  4,
/*  50	51  52	53  54	55  56	57  58	59  5A	5B  5C	5D  5E	5F */
     0,	 1,  0,  2,  0,  1,  0,  3,  0,  1,  0,  2,  0,  1,  0,  5,
/*  60	61  62	63  64	65  66	67  68	69  6A	6B  6C	6D  6E	6F */
     0,	 1,  0,  2,  0,  1,  0,  3,  0,  1,  0,  2,  0,  1,  0,  4,
/*  70	71  72	73  74	75  76	77  78	79  7A	7B  7C	7D  7E	7F */
     0,	 1,  0,  2,  0,  1,  0,  3,  0,  1,  0,  2,  0,  1,  0,  7,
/*  80	81  82	83  84	85  86	87  88	89  8A	8B  8C	8D  8E	8F */
     0,	 1,  0,  2,  0,  1,  0,  3,  0,  1,  0,  2,  0,  1,  0,  4,
/*  90	91  92	93  94	95  96	97  98	99  9A	9B  9C	9D  9E	9F */
     0,	 1,  0,  2,  0,  1,  0,  3,  0,  1,  0,  2,  0,  1,  0,  5,
/*  A0	A1  A2	A3  A4	A5  A6	A7  A8	A9  AA	AB  AC	AD  AE	AF */
     0,	 1,  0,  2,  0,  1,  0,  3,  0,  1,  0,  2,  0,  1,  0,  4,
/*  B0	B1  B2	B3  B4	B5  B6	B7  B8	B9  BA	BB  BC	BD  BE	BF */
     0,	 1,  0,  2,  0,  1,  0,  3,  0,  1,  0,  2,  0,  1,  0,  6,
/*  C0	C1  C2	C3  C4	C5  C6	C7  C8	C9  CA	CB  CC	CD  CE	CF */
     0,	 1,  0,  2,  0,  1,  0,  3,  0,  1,  0,  2,  0,  1,  0,  4,
/*  D0	D1  D2	D3  D4	D5  D6	D7  D8	D9  DA	DB  DC	DD  DE	DF */
     0,	 1,  0,  2,  0,  1,  0,  3,  0,  1,  0,  2,  0,  1,  0,  5,
/*  E0	E1  E2	E3  E4	E5  E6	E7  E8	E9  EA	EB  EC	ED  EE	EF */
     0,	 1,  0,  2,  0,  1,  0,  3,  0,  1,  0,  2,  0,  1,  0,  4,
/*  F0	F1  F2	F3  F4	F5  F6	F7  F8	F9  FA	FB  FC	FD  FE	FF */
     0,	 1,  0,  2,  0,  1,  0,  3,  0,  1,  0,  2,  0,  1,  0,  8
};

static s8 oneBits[256] = {
/*  00	01  02	03  04	05  06	07  08	09  0A	0B  0C	0D  0E	0F */
     0,	 1,  1,  2,  1,  2,  2,  3,  1,  2,  2,  3,  2,  3,  3,  4,
/*  10	11  12	13  14	15  16	17  18	19  1A	1B  1C	1D  1E	1F */
   1+0,1+1,1+1,1+2,1+1,1+2,1+2,1+3,1+1,1+2,1+2,1+3,1+2,1+3,1+3,1+4,
/*  20	21  22	23  24	25  26	27  28	29  2A	2B  2C	2D  2E	2F */
   1+0,1+1,1+1,1+2,1+1,1+2,1+2,1+3,1+1,1+2,1+2,1+3,1+2,1+3,1+3,1+4,
/*  30	31  32	33  34	35  36	37  38	39  3A	3B  3C	3D  3E	3F */
   2+0,2+1,2+1,2+2,2+1,2+2,2+2,2+3,2+1,2+2,2+2,2+3,2+2,2+3,2+3,2+4,
/*  40	41  42	43  44	45  46	47  48	49  4A	4B  4C	4D  4E	4F */
   1+0,1+1,1+1,1+2,1+1,1+2,1+2,1+3,1+1,1+2,1+2,1+3,1+2,1+3,1+3,1+4,
/*  50	51  52	53  54	55  56	57  58	59  5A	5B  5C	5D  5E	5F */
   2+0,2+1,2+1,2+2,2+1,2+2,2+2,2+3,2+1,2+2,2+2,2+3,2+2,2+3,2+3,2+4,
/*  60	61  62	63  64	65  66	67  68	69  6A	6B  6C	6D  6E	6F */
   2+0,2+1,2+1,2+2,2+1,2+2,2+2,2+3,2+1,2+2,2+2,2+3,2+2,2+3,2+3,2+4,
/*  70	71  72	73  74	75  76	77  78	79  7A	7B  7C	7D  7E	7F */
   3+0,3+1,3+1,3+2,3+1,3+2,3+2,3+3,3+1,3+2,3+2,3+3,3+2,3+3,3+3,3+4,
/*  80	81  82	83  84	85  86	87  88	89  8A	8B  8C	8D  8E	8F */
   1+0,1+1,1+1,1+2,1+1,1+2,1+2,1+3,1+1,1+2,1+2,1+3,1+2,1+3,1+3,1+4,
/*  90	91  92	93  94	95  96	97  98	99  9A	9B  9C	9D  9E	9F */
   2+0,2+1,2+1,2+2,2+1,2+2,2+2,2+3,2+1,2+2,2+2,2+3,2+2,2+3,2+3,2+4,
/*  A0	A1  A2	A3  A4	A5  A6	A7  A8	A9  AA	AB  AC	AD  AE	AF */
   2+0,2+1,2+1,2+2,2+1,2+2,2+2,2+3,2+1,2+2,2+2,2+3,2+2,2+3,2+3,2+4,
/*  B0	B1  B2	B3  B4	B5  B6	B7  B8	B9  BA	BB  BC	BD  BE	BF */
   3+0,3+1,3+1,3+2,3+1,3+2,3+2,3+3,3+1,3+2,3+2,3+3,3+2,3+3,3+3,3+4,
/*  C0	C1  C2	C3  C4	C5  C6	C7  C8	C9  CA	CB  CC	CD  CE	CF */
   2+0,2+1,2+1,2+2,2+1,2+2,2+2,2+3,2+1,2+2,2+2,2+3,2+2,2+3,2+3,2+4,
/*  D0	D1  D2	D3  D4	D5  D6	D7  D8	D9  DA	DB  DC	DD  DE	DF */
   3+0,3+1,3+1,3+2,3+1,3+2,3+2,3+3,3+1,3+2,3+2,3+3,3+2,3+3,3+3,3+4,
/*  E0	E1  E2	E3  E4	E5  E6	E7  E8	E9  EA	EB  EC	ED  EE	EF */
   3+0,3+1,3+1,3+2,3+1,3+2,3+2,3+3,3+1,3+2,3+2,3+3,3+2,3+3,3+3,3+4,
/*  F0	F1  F2	F3  F4	F5  F6	F7  F8	F9  FA	FB  FC	FD  FE	FF */
   4+0,4+1,4+1,4+2,4+1,4+2,4+2,4+3,4+1,4+2,4+2,4+3,4+2,4+3,4+3,4+4
};

/*************************************************************************
 * Instance Methods
 *************************************************************************/

s32
bitsFindFirst0(s32 word)
{
    s32 bitOff = 0;

    if ((word & 0xffff) == 0xffff) {
	word >>= 16;
	bitOff = 16;
    }
    if ((word & 0xff) == 0xff) {
	word >>= 8;
	bitOff += 8;
    }
    return ff0Bit[word & 0xff] + bitOff;
}

s32
bitsFindFirst1(s32 word)
{
    return bitsFindFirst0(~word);
}

s32
bitsCount1s(s32 word)
{
    s32 count;

    count = oneBits[word & 0xff];
    word >>= 8;
    count += oneBits[word & 0xff];
    word >>= 8;
    count += oneBits[word & 0xff];
    word >>= 8;
    count += oneBits[word & 0xff];

    return count;
}

s32
bitsCount0s(s32 word)
{
    return bitsCount1s(~word);
}

u32
bitsLow1s(s32 count)
{
    return count >= BITS(u32) ? (u32) -1 : ~((u32) -1 << count);
}

/*************************************************************************
 * Private Functions
 *************************************************************************/

#endif					   /* !defined(BITS_HEADER) */
