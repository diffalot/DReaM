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
 * $(@)cas_sys.c $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:29 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 2005 by Sun Microsystems, Inc.
 * Implementation of CAS

 * Created:   July 29, 2005
 * Author:    Yongfang Liang
 *---------------------------------------------------------------*/
 
#include  <time.h>   
#include  "cas_sys.h"
#define TS_INPUT_INTERNAL_MAX_SIZE 1024

#if 0
void msg_TRACE(const char *psz_format, ...)
{
    va_list args;
    va_start(args, psz_format);
	printf(psz_format, args);
	va_end(args);
	return;
}
#endif

static encrypt_key_p ts_encrypt_key_create(int pn, encrypt_type type, byte_p p_payload, int i_length)
{
	encrypt_key_p d = NULL;
	d = (encrypt_key_p)calloc(1, sizeof(encrypt_key_t));
	TEST_MEMORY(d);
	d->e_type = type;
	d->pn     = pn;
	d->next = NULL;
	d->ecm_pid = 0; /* no . will be assigned later in the parser*/
	d->key_length = 0;
	/* get the key content (the ECM payload) */
	d->key_buf = (byte_p)malloc(sizeof(byte_t)*i_length);
	if (d->key_buf)
	{
	    memcpy(d->key_buf, p_payload, i_length);
	    d->key_length = i_length;
	}
	
	/* FIXME: need to set the payload */ 
    return d;
}

static encrypt_key_p ts_encrypt_key_append(encrypt_key_p list, 
                        encrypt_key_p new_key) 
{
  if( list == NULL )
    return(new_key);

  list->next = ts_encrypt_key_append(list->next,new_key);
  return(list);
}

static void ts_encrypt_key_delete(encrypt_key_p list)
{
	if (list->next == NULL)
	{
        if (list->key_buf)
        {
        	free(list->key_buf);
        }
		free(list);
		return;
	}
    ts_encrypt_key_delete(list->next);
    return;
}

/* 
   call back function of the output pipe of the CAS
   all output data is redirected here 
*/
static RetCode outputStream(void *instp, void *cop, Pipe pipe)
{
   ca_sys_p             p_sys  = (ca_sys_p)instp; 
   static encrypt_key_p p_current_out_key = NULL;
   clock_t		        now;
   static clock_t		last_time;
   
   last_time = clock();
   msg_TRACE("enter outputStream function. waiting for output.\n");
   while(pipeIsAvail(pipe, TP_LENGTH))
   {
        /* output the bitstream */
        byte_p data =  pipeGet(pipe, sizeof(ca_ts_pck_t));
        if (p_sys->outputCallback)
        {
	    (*p_sys->outputCallback)(p_sys->output_token, data, sizeof(ca_ts_pck_t));
        }

        /* generate ECM periodically */
        
/* We don't do the CA injection periodly, the ECMs are inserted after each PMT  */
#if 0
        now = clock();
        if ((unsigned int)((double)(now - last_time)/CLOCKS_PER_SEC) > p_sys->ecm_frequency)
        {
            p_current_out_key = (p_current_out_key)? p_current_out_key : p_sys->p_keys; 
		    if (p_current_out_key)
		    {
		    	generateCAMessage(p_sys, TRUE, p_current_out_key->ecm_pid, 
		    		              p_current_out_key->key_buf, p_current_out_key->key_length,
		    		              FALSE);
		    	last_time = now;
		        p_current_out_key = p_current_out_key->next;
		    }
        }
#endif        
   }
   msg_TRACE("exit outputStream function.\n");
   return RETCODE_SUCCESS;
}

/*
----------------------------------------------------------------------
addKey2CAS: add one encryption key to the CAS structure
ARGUMENTS:
    p_sys:      pointer to the structure
    pn:         program number this key used for
    type:       encryption type
    p_payload:  pointer to the key
    lenght:     length of the key
RETURN:
    none
----------------------------------------------------------------------
*/
void addKey2CAS(ca_sys_p p_sys, int pn, encrypt_type type, byte_p p_payload, int length)
{
	encrypt_key_p p_key = NULL;
	if (p_sys)
	{
		p_key = ts_encrypt_key_create(pn, type, p_payload, length);
		p_sys->p_keys = ts_encrypt_key_append(p_sys->p_keys, p_key);
	}
	return;
}

/*
----------------------------------------------------------------------
createCAS: return a CAS handle
ARGUMENTS:
    seconds:      freqency of inserting ECMs, (seconds)
RETURN:
    structure pointer
----------------------------------------------------------------------
*/
ca_sys_p createCAS(int sys_id,unsigned int seconds)
{
	ca_sys_p p_sys = NULL;
	int i = 0;
	ts_pid_t *pid = NULL;
	p_sys = (ca_sys_p)calloc(1, sizeof(ca_sys_t));
	TEST_MEMORY(p_sys);
	
	/* setup data members */
	p_sys->outpipe     = pipeNew(TS_INPUT_INTERNAL_MAX_SIZE, outputStream, NULL, p_sys, NULL);
	p_sys->putPosition = 0LL;
	p_sys->p_keys      = NULL;
	p_sys->ecm_frequency = seconds;
	p_sys->mpeg2TScop    = NULL;
	p_sys->b_outputECMafterPMT = TRUE;
    p_sys->i_pmt_version_number = 0;
    
	/* pid initialization */
    for( i = 0; i < 8192; i++ )
    {
        pid = &p_sys->pid[i];
        pid->b_seen     = FALSE;
        pid->b_pmt_pid  = FALSE;
        pid->pn         = 0;
        pid->p_key      = NULL;
        pid->continuity_counter = 0;
    }
    /* reserved pids */
    for( i = 0x0; i < 0x1F4; i++ )
    {
        pid = &p_sys->pid[i];
        pid->b_seen   = TRUE;
    }
    /* setup the parser */
    p_sys->p_parser = setupNewParser(p_sys);

    p_sys->outputCallback    = NULL;
    p_sys->emsCallback       = NULL;
    p_sys->scrambleCallback  = NULL;
    p_sys->tsCallback        = NULL;
    p_sys->output_token      = NULL;
    p_sys->ems_token         = NULL;
    p_sys->scramble_token    = NULL;
    p_sys->ts_token          = NULL;
    p_sys->i_ca_system_id = sys_id;
    return p_sys;
}

/*
----------------------------------------------------------------------
freeCAS: delete a CAS handler
ARGUMENTS:
    p_sys:  pointer to the structure
RETURN:
    none
----------------------------------------------------------------------
*/
void freeCAS(ca_sys_p p_sys)
{
    if (p_sys)
    {
	    if (p_sys->outpipe)
	    {
	        pipeFree(p_sys->outpipe);
	    }
		if (p_sys->p_keys)
		{
	        ts_encrypt_key_delete(p_sys->p_keys);
		}
		if (p_sys->p_parser)
		{
		    /* cleanup */
		    freeParser(p_sys->p_parser);
		}
	    free(p_sys);
    }
    return;
}

/*
----------------------------------------------------------------------
scrambleTsPacket: scramble one TS packet
ARGUMENTS:
    p_sys:    pointer to the structure
    version_number: PMT version number
RETURN:
    none
----------------------------------------------------------------------
*/
void casSetPMTVersionNumber(ca_sys_p p_sys, int version_number)
{
    p_sys->i_pmt_version_number = version_number;
}

/*
----------------------------------------------------------------------
scrambleTsPacket: scramble one TS packet
ARGUMENTS:
    p_sys:    pointer to the structure
    pid:      pid of this packet
    pkt:      pointer to the packet
    payload:  pointer of the data buffer to be scrambled
    length:   length of the buffer
RETURN:
    none
----------------------------------------------------------------------
*/
void scrambleTsPacket(ca_sys_p p_sys, unsigned pid, byte_p pkt, byte_p payload, int length, long long curTSpos)
{
    int i = 0;
    encrypt_key_p p_key = p_sys->pid[pid].p_key;
    if (p_key && p_sys->scrambleCallback)
    {
		/* turn on the scrambling control bit */
		p_sys->pkt_buf.data[3] |= 0x40;
		(*p_sys->scrambleCallback)(p_sys->scramble_token, 
			 p_key->key_buf,p_key->key_length, pkt, payload,length, curTSpos);
    }
    else
    {
      /*   	msg_TRACE("no scrambling for pid %d \n", pid);  */
    	
    }
    
    return;
}

/*
----------------------------------------------------------------------
getStreamType: determine the stream type
ARGUMENTS:
    i_stream_type:   stream type value in the PMT es section
RETURN:
    stream type
----------------------------------------------------------------------
*/
encrypt_type getStreamType(int i_stream_type )
{
    encrypt_type retType;
    switch( i_stream_type )
    {
        case 0x01:  /* MPEG-1 video */
        case 0x02:  /* MPEG-2 video */
        case 0x80:  /* MPEG-2 MOTO video */
        case 0x10:  /* MPEG4 (video) */
        case 0x1B:  /* H264 */
            retType = ENCRYPT_VIDEO_ONLY;
            break;

        case 0x03:  /* MPEG-1 audio */
        case 0x04:  /* MPEG-2 audio */
        case 0x11:  /* MPEG4 (audio) */
        case 0x0f:  /* ISO/IEC 13818-7 Audio with ADTS transport syntax */
        case 0x81:  /* A52 (audio) */
        case 0x83:  /* LPCM (audio) */
        case 0x84:  /* SDDS (audio) */
        case 0x85:  /* DTS (audio) */
        case 0x94:  /* SDDS (audio) */
            retType = ENCRYPT_AUDIO_ONLY;
            break;
        default:
        	retType = ENCRYPT_UNKNOWN;
        	break;
    }

    return retType;
}

/*
----------------------------------------------------------------------
casPutData: get the input data to parse
ARGUMENTS:
    p_sys        : CAS handle
    buf          : pointer to the buffer
    buf_len      : size of the buffer
RETURN:
    int
----------------------------------------------------------------------
*/
int casPutData(ca_sys_p p_sys, byte_p buf, int buf_len)
{
    if (mmpPut(p_sys->p_parser->mmp, buf, buf_len) == RETCODE_SUCCESS)
    {
    	return 0;
    }
    else
    {
    	return -1;
    }
}


/*
----------------------------------------------------------------------
casEOF: get CAS status
ARGUMENTS:
    p_sys        : CAS handle
RETURN:
    whether it is the end of the input TS
----------------------------------------------------------------------
*/
int casEOF(ca_sys_p p_sys)
{
	if (mmpEof(p_sys->p_parser->mmp) == RETCODE_SUCCESS)
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

/*
----------------------------------------------------------------------
casNoECMafterPMT: 
ARGUMENTS:
    p_sys        : CAS handle
RETURN:
    whether it is the end of the input TS
----------------------------------------------------------------------
*/
void casECMafterPMT(ca_sys_p p_sys, int value)
{
	p_sys->b_outputECMafterPMT = value;
	return;
}

void casSetOutputCallback (ca_sys_p p_sys, casOutputCallBack callBack, void *token)
{
	p_sys->outputCallback = callBack;
	p_sys->output_token   = token;
	return;
}

void casSetEMSCallBack (ca_sys_p p_sys, casEMSCallBack callBack, void *token)
{
	p_sys->emsCallback = callBack;
	p_sys->ems_token   = token;
	return;
}

void casSetScrambleCallBack (ca_sys_p p_sys, casScrambleCallBack callBack, void *token)
{
	p_sys->scrambleCallback = callBack;
	p_sys->scramble_token   = token;
	return;
}

void casSetTsCallBack (ca_sys_p p_sys, casTSCallBack callBack, void *token)
{
	p_sys->tsCallback = callBack;
	p_sys->ts_token   = token;
	return;
}


