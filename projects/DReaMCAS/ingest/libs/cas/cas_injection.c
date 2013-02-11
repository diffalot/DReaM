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
 * $(@)cas_injection.c $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:29 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 2005 by Sun Microsystems, Inc.
 * Functions to perform CA injection

 * Created:   July 29, 2005
 * Author:    Yongfang Liang
 *---------------------------------------------------------------*/

#include <assert.h>
#include "mpeg2/mpeg2ts_demux_stub.h"
#include  "cas_sys.h"

#define CA_MESSAGE_SECTION_HEADER_SIZE 3     

typedef struct 
{
    byte_t table_id;                  /*  8 bits */
    byte_t section_syntax_indicator;  /*  1 bit  */
    byte_t DVB_reserved;              /*  1 bit  */
    byte_t ISO_reserved;              /*  2 bit  */
    unsigned short section_length;    /* 12 bits */
    void* section_data;               /* variable */
    
    /* Control data: */
    short packet_pid;
} ts_ca_mesage_section_t, *ts_ca_mesage_section_p;

/*
----------------------------------------------------------------------
ts_ca_descriptor_create: create a CA descriptor
ARGUMENTS:
  ca_system_id
  ca_pid
  p_private_data:  Pointer to descriptor private data.
  length (int) - length of the private data buffer
  RETURN
    the descriptor
CREATED July 2005 BY Yongfang Liang
----------------------------------------------------------------------
*/

ts_ca_descriptor_p ts_ca_descriptor_create(encrypt_type en_type, int ca_system_id,
	int ca_pid, byte_p p_private_data, int i_private_length)
{
	ts_ca_descriptor_p d = NULL;

    
    d = (ts_ca_descriptor_p) calloc(1, sizeof(ts_ca_descriptor_t));
	TEST_MEMORY(d);
	d->stream_type = ANY_STREAM_TYPE;
	d->en_type     = en_type;
	d->next        = NULL;
	
	/* set up the PSI descriptor */
    OpaqueDescriptor *p_descriptor = (OpaqueDescriptor*)calloc(1,sizeof(OpaqueDescriptor));
    TEST_MEMORY(p_descriptor);  
    /* Set descriptor values */
    p_descriptor->tag     = (byte_t)0x9;
    p_descriptor->buffer.length  = 4 + i_private_length;
    p_descriptor->buffer.data[0] = ca_system_id >> 8;
    p_descriptor->buffer.data[1] = ca_system_id & 0xff;
    p_descriptor->buffer.data[2] = 0xe0 | ((ca_pid >> 8) & 0x1f);
    p_descriptor->buffer.data[3] = ca_pid & 0xff;
    /* assert(i_private_length < 252) */
    memcpy(p_descriptor->buffer.data + 4, p_private_data,i_private_length);
	d->desc = (void *)p_descriptor;
    
    return d;
}

ts_ca_descriptor_p ts_ca_descriptor_append(ts_ca_descriptor_p list, 
                        ts_ca_descriptor_p new_key) 
{
  if( list == NULL )
    return(new_key);

  list->next = ts_ca_descriptor_append(list->next,new_key);
  return(list);
}

void ts_ca_descriptor_delete(ts_ca_descriptor_p list)
{
	if (list->next == NULL)
	{
        if (list->desc)
        {
        	free(list->desc);
        }
		free(list);
		return;
	}
    ts_ca_descriptor_delete(list->next);
    return;
}

ts_ca_mesage_section_p ts_ca_mesage_section_create(int ecm, short pid, byte_p buf, int buf_len)
{
    ts_ca_mesage_section_p  section = NULL;
    /* Refer to ETR289 Table 4*/
    int table_id = ecm? 0x80: 0x82;
    
    /* Allocate memory for the structure */
    section = (ts_ca_mesage_section_p)calloc(1,sizeof(ts_ca_mesage_section_t));
    TEST_MEMORY(section);
    /* Set known section values */
    section->table_id = (byte_t)table_id;
    section->section_syntax_indicator = 0; /* always 0 */
    section->DVB_reserved = 0;
    section->ISO_reserved = 0;
    section->section_length = (unsigned short)buf_len;
    section->packet_pid = (short)pid;
    
    /* copy payload data */
    section->section_data = (void *)calloc( buf_len, sizeof(byte_p));
    TEST_MEMORY(section->section_data);
    memcpy(section->section_data, buf, buf_len*sizeof(byte_p));
	return section;
}

static void ts_ca_mesage_section_delete(ts_ca_mesage_section_p section) 
{
    if(section == NULL)
      return;
    if(section->section_data != NULL)
      free(section->section_data);
    free(section);
}
 
static void ts_ca_message_to_stream(byte_p stream, ts_ca_mesage_section_p s) 
{
    assert(stream != NULL);
    
    /* table_id;                    8 bits */
    /* section_syntax_indicator;    1 bit  */
    /* DVB_reserved;                1 bit  */
    /* ISO_reserved                 2 bits */
    /* section_length;             12 bits */
    /* section_data;              variable */
    
    /* BYTE 0 */
    stream[0] = s->table_id;
    
    /* BYTE 1 */
    /* Bits 4 5 and 6 are reserved */
    stream[1] = 0x30 | getbit(s->section_length,8,0xF);
    putbit(stream[1],7,s->section_syntax_indicator&0x1);
    
    /* BYTE 2 */
    stream[2] = s->section_length & 0xFF;
    
    /* Output section data. */
    memcpy(stream + 3, s->section_data, s->section_length);

    return;
}

/*
----------------------------------------------------------------------
InjectCA2PMT: Inject CAs into a PMT
ARGUMENTS:
    p_sys:		sys info
    info:		pmt info from the PMT parser
    pmt_pid:    pid of the PMT
    ca_list:    ca descriptors to be inserted
RETURN:
	none
CREATED July 2005 BY Yongfang Liang
----------------------------------------------------------------------
*/
void injectCA2PMT(ca_sys_p p_sys, Mpeg2PmtInfo *infop, short pmt_pid, ts_ca_descriptor_p ca_list)
{
    OpaqueDescriptor * psi_descriptor=NULL;
    int length;
    Mpeg2PmtTsProgramMapSection *pmsp = &infop->tsProgramMapSection;
    Mpeg2PmtStream    *streamList = NULL;
    Mpeg2PmtDescriptor *pmt_desc = NULL;
    int pn;
    int pos = 0; /*buffer position */
    ts_ca_descriptor_p p_ca = NULL;
    

    pn = MPEG2_PMT_PROGRAM_NUMBER(&pmsp->psiExtension);

    struct PmtHeader *pmthdr=createProgramMapSection( pn, p_sys->i_pmt_version_number,
			       MPEG2_PMT_HEADER_PCR_PID(&pmsp->pmtHeader));

    assert( pmsp->descList == NULL );

    /* add new CAs */
    for (p_ca = ca_list; p_ca != NULL; p_ca = p_ca->next)
    {
    	if (p_ca->stream_type == ANY_STREAM_TYPE) /* for the whole program */
    	{
	    OpaqueDescriptor *caopaque = (OpaqueDescriptor*)p_ca->desc;
    	
	    addProgramMapDescriptor( pmthdr, caopaque );
	}
    }

    /* descriptors for each elementary stream */
    for (streamList = pmsp->streamList; streamList != NULL; 
             streamList = streamList->next)
    {
        unsigned            ePid = MPEG2_PMT_STREAM_HEADER_ELEMENTARY_PID(
						    &streamList->streamHeader);

#define MAX_DESCRIPTORS_SUPPORTED 16
	OpaqueDescriptor descriptorArray[MAX_DESCRIPTORS_SUPPORTED];
	int              idesc = -1;
        for (pmt_desc = streamList->descList; pmt_desc != NULL; pmt_desc = pmt_desc->next)
        {
	    ++idesc;
	    assert( idesc<MAX_DESCRIPTORS_SUPPORTED );

	    descriptorArray[idesc].tag = pmt_desc->descriptor.descriptorTag;
	    descriptorArray[idesc].buffer.length = pmt_desc->descriptor.descriptorLen;
	    memcpy(descriptorArray[idesc].buffer.data, pmt_desc->descriptor.un.descriptor, 
                   sizeof(descriptorArray[idesc].buffer.data));
        }
        /* add new CAs */
        for (p_ca = ca_list; p_ca != NULL; p_ca = p_ca->next)
        {
	    if (p_ca->stream_type == streamList->streamHeader.streamType) 
            {
                psi_descriptor = (OpaqueDescriptor*)calloc(1,sizeof(OpaqueDescriptor));
                TEST_MEMORY(psi_descriptor);
		OpaqueDescriptor *caopaque = (OpaqueDescriptor*)p_ca->desc;
                memcpy(psi_descriptor->buffer.data, caopaque->buffer.data, 
                       sizeof(psi_descriptor->buffer.data));
		psi_descriptor->tag = caopaque->tag;
		psi_descriptor->buffer.length = caopaque->buffer.length;

		++idesc;
		assert( idesc<MAX_DESCRIPTORS_SUPPORTED );
		descriptorArray[idesc].tag = psi_descriptor->tag;
		descriptorArray[idesc].buffer.length = psi_descriptor->buffer.length;
		memcpy(descriptorArray[idesc].buffer.data, psi_descriptor->buffer.data,
                       sizeof(descriptorArray[idesc].buffer.data));
	    }
        }
	addPmtEntry( pmthdr, streamList->streamHeader.streamType, ePid, 
		     descriptorArray, idesc+1 );
    }

     /* Create output bytes. */

    PsiBytes bpsi = getPmtBytes( pmthdr );
    p_sys->pid[pmt_pid].continuity_counter++;
    TsBytes tspkt = createTransportPacketForPsi( pmt_pid,
				 p_sys->pid[pmt_pid].continuity_counter,
						 bpsi );

/*     printf("----ts-----\n"); */
/*     stringify( tspkt.data, 188 ); */
/*     printf(" ---- tsend --- \n"); */

    pipeSync( p_sys->outpipe );
    pipePut( p_sys->outpipe, tspkt.data, TP_LENGTH, FALSE, p_sys->putPosition );
    p_sys->putPosition += TP_LENGTH;

    return;
}   

/*
----------------------------------------------------------------------
GenerateCAMessage: Generate ECM or EMM  
ARGUMENTS:
    ecm:          ECM -> 1
    pid:          pid of the output ECM/EMM packets
    buf:            ECM/EMM message (usually, the keys)
    buf_len
 RETURN:
    none
CREATED July 2005 BY Yongfang Liang
----------------------------------------------------------------------
*/
long generateCAMessage(ca_sys_p p_sys, bool ecm, short pid, byte_p buf, int buf_len)
{
#define TS_INPUT_INTERNAL_MAX_SIZE 1024
    byte_t tmp[TS_INPUT_INTERNAL_MAX_SIZE];
    ts_ca_mesage_section_p  section;
    long pack_nr = 0;

	if (p_sys->emsCallback)
	{
        /* we can do the encryption here */
        int out_len = buf_len + 8;/* add extra 8 bytes */
        byte_p out_buf = (byte_p)malloc(out_len); 
        TEST_MEMORY(out_buf);
        
		(*p_sys->emsCallback)(p_sys->ems_token, buf, buf_len, out_buf, &out_len, (long long)(p_sys->putPosition));
        section = ts_ca_mesage_section_create(ecm, pid, out_buf, out_len);
        free(out_buf);
	}
	else
	{
        /* just output the payload as it is */
        section = ts_ca_mesage_section_create(ecm, pid, buf, buf_len);
	}
  
    ts_ca_message_to_stream(tmp, section);

    PsiBytes cabytes;
    cabytes.length = section->section_length + CA_MESSAGE_SECTION_HEADER_SIZE;

    memcpy( cabytes.data, tmp, cabytes.length );
    p_sys->pid[pid].continuity_counter++;
    
    TsBytes tspkt = createTransportPacketForPsi(section->packet_pid,
						p_sys->pid[pid].continuity_counter, 
						cabytes );

    pipeSync( p_sys->outpipe );
    pipePut( p_sys->outpipe, tspkt.data, TP_LENGTH, FALSE, p_sys->putPosition );
    p_sys->putPosition += TP_LENGTH;

/*     printf( " ca ts pkt\n" ); */
/*     stringify( tspkt.data, 188 ); */
/*     printf( "end ca ts pkt///////// \n"); */


    ts_ca_mesage_section_delete(section);

    return 0;
}

/*
----------------------------------------------------------------------
ouputECMs: output ECM 
ARGUMENTS:
    p_sys:		    sys info
    program_number: pn of the pmt
RETURN:
	none
CREATED July 2005 BY Yongfang Liang
----------------------------------------------------------------------
*/
void ouputECMs(ca_sys_p p_sys, int program_number)
{
    encrypt_key_p p_out_key = NULL;
    for (p_out_key = p_sys->p_keys; p_out_key != NULL; p_out_key = p_out_key->next)
    {
        if (p_out_key->pn == program_number
        	|| p_out_key->pn == FOR_ANY_PROGRAM)
        {
		    generateCAMessage(p_sys, TRUE, p_out_key->ecm_pid, 
		    	              p_out_key->key_buf, p_out_key->key_length
		    	              );
        	
        }
    }
    return;
}

