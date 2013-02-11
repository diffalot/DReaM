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
 * $(@)MkCrc32Table.c $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:31 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "cobjs/Types.h"

static u32 table[256];
static u32 calculateCrc32(int byte);

void
main(int argc, char **argv)
{
    unsigned i;

    for (i = 0; i < 256; i++) {
	table[i] = calculateCrc32(i);
    }

    printf("#include \"mmp/Crc32Table.h\"\n");
    printf("#include \"cobjs/Types.h\"\n");
    printf("\nconst u32 crc32Table[256] = {");

    for (i = 0; i < 256; i++) {
	if ((i % 4) == 0) {
	    printf("\n    /* %3u: */  ", i);
	}
	printf("0x%08lx", table[i]);
	if (i != 255) {
	    printf(", ");
	}
    }
    printf("\n};\n");
    exit(0);
}

static u32
calculateCrc32(int byte)
{
    int j;
    u32 crc;

    crc = byte;

    for (j = 8; j > 0; j--) {
	if (crc & 1) {
	    crc = (crc >> 1) ^ 0xEDB88320UL;
	} else {
	    crc >>= 1;
	}
    }
    return crc;
}
