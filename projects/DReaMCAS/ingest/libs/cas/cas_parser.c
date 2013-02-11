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
 * $(@)cas_parser.c $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:29 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 2005 by Sun Microsystems, Inc.
 * Functions to perform TS parsing

 * Created:   July 29, 2005
 * Author:    Yongfang Liang
 *---------------------------------------------------------------*/
 
#include  "cas_sys.h"

static void mpeg2GatherTS(Mpeg2Ts mpeg2Ts, Pipe pipe)
{
    ca_sys_p          p_sys = (ca_sys_p)mpeg2Ts->token; 
    ca_ts_pck_p       p_ts_pkt = NULL;

    p_ts_pkt = PIPE_PEEK(pipe, ca_ts_pck_t);
    memcpy(&(p_sys->pkt_buf.data[0]), p_ts_pkt, sizeof(ca_ts_pck_t));
    return;
}

static RetCode   mpeg2PatInfoCallback (void *token, Mpeg2PatInfo *infop,
				               RetCode retCode)
{
    ca_sys_p          p_sys = (ca_sys_p)token;
    unsigned            pex;
    Mpeg2PatSection     *curPsp = &(infop->patSection);

    for (pex = 0; pex < curPsp->nPatEntries; pex++) 
    {
        Mpeg2PatEntry   *pep      = &curPsp->patEntries[pex];
        unsigned        pn        = MPEG2_PAT_ENTRY_PROGRAM_NUMBER(pep);
        unsigned        pid       = MPEG2_PAT_ENTRY_PROGRAM_MAP_PID(pep);
        p_sys->pid[pid].b_seen    = TRUE;
        p_sys->pid[pid].pn        = pn;
        p_sys->pid[pid].b_pmt_pid = TRUE;
    }
    return RETCODE_SUCCESS;
    
}

static RetCode   mpeg2GatherTSCallback (void *token, Mpeg2TsInfo *infop,
				                RetCode retCode)
{
    ca_sys_p          p_sys = (ca_sys_p)token;
    unsigned          pid;
    Pipe              pidPipe  = NULL;
    #define MPEG2_TS_GATHER_TS 7

    if ((retCode & 0xffff) == MPEG2_TS_GATHER_TS)
    {
        mpeg2GatherTS((Mpeg2Ts)token, (Pipe)infop);
        return RETCODE_SUCCESS;
    }

    pid = MPEG2_TS_TRANSPORT_PACKET_PID(&infop->transportPacket);
    pidPipe = MMP_CONTEXT_PIDTOPIPE(p_sys->mpeg2TScop, pid);

    if (p_sys->tsCallback)
    {
    	(*p_sys->tsCallback)(p_sys->ts_token, infop->position);
    }
    
    if (pidPipe != PIPE_NULL && pidPipe != NULL)
    {
    	if (pid == MPEG2_TS_NULL_PID || pid == MPEG2_TS_PAT_PID
    		|| pid == MPEG2_TS_CAT_PID)
    	{
    		/* break: output the packet as it is */
			/* output the packet  */
    		pipeSync(p_sys->outpipe);
    		pipePut(p_sys->outpipe,&(p_sys->pkt_buf.data[0]), MPEG2_TS_PKT_SIZE, FALSE, p_sys->putPosition);
			p_sys->putPosition += MPEG2_TS_PKT_SIZE;
    	}
    	else  /* pmt */
    	{
    		if (infop->transportPacket.adaptationFieldControl == MPEG2_TS_ADAPTATION_FIELD_CONTROL_ADAPTATION_ONLY 
    			|| infop->transportPacket.adaptationFieldControl == MPEG2_TS_ADAPTATION_FIELD_CONTROL_ADAPTATION_PAYLOAD)
    		{
    			/* output a new ts packet contains only adaptation field, since we will generate a new pmt */
                int i = 0;
    			/* adaptation field only */
    			p_sys->pkt_buf.data[3] |= 0x20; /* bit 4 */
    			p_sys->pkt_buf.data[3] &= 0xef; /* bit 3 */
    		    /* change adaptation field length */
    			p_sys->pkt_buf.data[4] = MPEG2_TS_PKT_SIZE - 5;
    			for (i = infop->adaptationFieldLength + 5; i < MPEG2_TS_PKT_SIZE; i++)
    			{
    			    p_sys->pkt_buf.data[i] = 0xff; /* stuffing bytes */
    			}
			    /* output this additional packet before new pmt is built */
                pipeSync(p_sys->outpipe);
			    pipePut(p_sys->outpipe,&(p_sys->pkt_buf.data[0]), MPEG2_TS_PKT_SIZE, FALSE, p_sys->putPosition);
			    p_sys->putPosition += MPEG2_TS_PKT_SIZE;
    			msg_TRACE("old PMT contains adaptation field, output one adaptation_only TS packet.\n");
    		}
    	}
    }
    else
    {
    	/* scramble the packet if required, only the payload! */
    	scrambleTsPacket(p_sys, pid, &(p_sys->pkt_buf.data[0]), &(p_sys->pkt_buf.data[0]) + MPEG2_TS_PKT_SIZE - infop->payloadLen, 
    	                 infop->payloadLen, infop->position);
	    /* output the packet  */
	    pipeSync(p_sys->outpipe);
	    pipePut(p_sys->outpipe,&(p_sys->pkt_buf.data[0]), MPEG2_TS_PKT_SIZE, FALSE, p_sys->putPosition);
	    p_sys->putPosition += MPEG2_TS_PKT_SIZE;
    }
    
    return RETCODE_SUCCESS;
}

/* callback functin for PMT */
static RetCode   mpeg2InjectCA2Pmt (void *token, Mpeg2PmtInfo *infop,
				                           RetCode retCode)
{
    ca_sys_p          p_sys = (ca_sys_p)token;
    Mpeg2PmtTsProgramMapSection *pmsp = &infop->tsProgramMapSection;

     unsigned   pcr_pid;
    Mpeg2PmtStream    *streamList = pmsp->streamList;
    Mpeg2PmtDescriptor *desc = NULL;
    unsigned            pn;  /* program number */
    encrypt_key_p p_keys = NULL;
    ts_ca_descriptor_p p_ca_descriptor = NULL; /* CA descriptors to be inserted for this pmt */
    int i = 0;
 
    pn = MPEG2_PMT_PROGRAM_NUMBER(&pmsp->psiExtension);

    /* record the pids (pcr_pid and elementary stream pid) that have been used */
    pcr_pid = MPEG2_PMT_HEADER_PCR_PID(&(pmsp->pmtHeader));
    p_sys->pid[pcr_pid].b_seen = TRUE;
    p_sys->pid[pcr_pid].pn     = pn;

    /* note: we assume there are no CA descriptors for current program
       and that is why we need to introduce them */
    for (streamList = pmsp->streamList; streamList != NULL; 
             streamList = streamList->next)
    {
        unsigned   ePid = MPEG2_PMT_STREAM_HEADER_ELEMENTARY_PID(&streamList->streamHeader);
        p_sys->pid[ePid].b_seen = TRUE;
        p_sys->pid[ePid].pn     = pn;
    }

    /* look up the keys. do we need CA injection for this PMT ? */
    for (p_keys = p_sys->p_keys; p_keys != NULL; 
             p_keys = p_keys->next)
    {
    	int key_count = 0;
        if (p_keys->pn == pn || p_keys->pn == FOR_ANY_PROGRAM)
        {
            /* create the CAs if an injection is needed */
            short availabe_pid = 0;
            int   i = 0;

            key_count ++;
            if (p_keys->ecm_pid != 0)
            {
            	/* we already got one */
            	availabe_pid = p_keys->ecm_pid;
            }
            else
            {
	            /* find available pid */
	            for (i = 0x10; i < 0x01FFE; i++)
	            {
	            	if (p_sys->pid[i].b_seen == FALSE)
	            	{
	                    availabe_pid         = (short)i;
	                    p_sys->pid[i].b_seen = TRUE;
	                    p_sys->pid[i].pn     = pn;
                        msg_TRACE("ECM pid allocated: %d,the %dth key.\n", availabe_pid, key_count);
	            		break;
	            	}
	            }
            }
            if (availabe_pid != 0)
            {
                p_sys->pid[availabe_pid].b_seen = TRUE;
                p_sys->pid[availabe_pid].pn     = pn;
                
                /* assign pid to this key */
                p_keys->ecm_pid = availabe_pid;
                
                /* we can support multiple CA system, now just CA_SYS_ID */
                /* we don't have privdate data for the descriptor */
                ts_ca_descriptor_p d = ts_ca_descriptor_create(p_keys->e_type, p_sys->i_ca_system_id, availabe_pid, NULL, 0);
                d->p_key             = p_keys;
                p_ca_descriptor      = ts_ca_descriptor_append(p_ca_descriptor, d);
            }
            else
            {
            	printf("all the pids are being used, key ignored, pn# %d\n", p_keys->pn);
            }
        }
    }

    if (p_ca_descriptor)
    {
        /* set up the encryption key for each ES */
	    for (streamList = pmsp->streamList; streamList != NULL; 
	             streamList = streamList->next)
	    {
            ts_ca_descriptor_p d = NULL;
	        short    ePid = MPEG2_PMT_STREAM_HEADER_ELEMENTARY_PID(&streamList->streamHeader);

	        for (d = p_ca_descriptor; d != NULL; d = d->next)
	        {
                /* what type of CA? for the whole program, video or audio? */
	        	switch (d->en_type)
	        	{
	        		case ENCRYPT_ALL:
                        p_sys->pid[pcr_pid].p_key = d->p_key;
                        
	        			p_sys->pid[ePid].p_key    = d->p_key;
	        			/* now we have the specific stream type */
	        			d->stream_type = ANY_STREAM_TYPE; 
	        			break;
	        	    case ENCRYPT_VIDEO_ONLY:
                    /* see ISO/IEC 13818-1, Table 2-29 */
	        	    	if (getStreamType(streamList->streamHeader.streamType) == ENCRYPT_VIDEO_ONLY)
	        	    	{
	        	    		p_sys->pid[ePid].p_key = d->p_key;
	        	    		/* now we have the specific stream type */
	        	    		d->stream_type = streamList->streamHeader.streamType; 
	        	    	}
	        		    break;
	        		case ENCRYPT_AUDIO_ONLY:
	        	    	if (getStreamType(streamList->streamHeader.streamType) == ENCRYPT_AUDIO_ONLY)
	        	    	{
	        	    		p_sys->pid[ePid].p_key = d->p_key;
	        	    		/* now we have the specific stream type */
	        	    		d->stream_type = streamList->streamHeader.streamType;
	        	    	}
	        			break;
	        		default:
	        			break;
	        	}
	        }
	    }
    }
    
	/* do the actual CA injection */
    for( i = 0x10; i < 8192; i++ )
    {
    	if (p_sys->pid[i].b_seen && p_sys->pid[i].b_pmt_pid
    		&& p_sys->pid[i].pn == pn)
    	{
			injectCA2PMT(p_sys, infop, i, p_ca_descriptor);
			/* output ECMs for this program */
			if (p_sys->b_outputECMafterPMT)
			{
			    ouputECMs(p_sys, pn);
			}
			break;
    	}
    }
	
    if (p_ca_descriptor)
    {
    	ts_ca_descriptor_delete(p_ca_descriptor);
    }

    return RETCODE_SUCCESS;
}
/*
----------------------------------------------------------------------
setupNewParser: setup a MPEG2 TS parser for the CAS structure
ARGUMENTS:
    p_sys:      pointer to the structure
RETURN:
    the parser pointer
----------------------------------------------------------------------
*/
parser_p setupNewParser(ca_sys_p p_sys)
{
  parser_p parser = (parser_p)calloc(1, sizeof(parser_t));
  MmpParserObject    *psi2TableIdToPop[MPEG2_PSI_TABLE_ID_STUFF];
  MmpParserObject    *mmpPops[1] ;

  /* we only parse the bitstream until PMT,we don't need PES parser */
  parser->mmp      = mmpNew() ;
  parser->mpeg2Ts  = mpeg2TsNew() ;
  parser->mpeg2Psi = mpeg2PsiNew() ;
  parser->mpeg2Pat = mpeg2PatNew();
  parser->mpeg2Pmt = mpeg2PmtNew();

  mpeg2PatSetCallBack(parser->mpeg2Pat, mpeg2PatInfoCallback,      p_sys);
  mpeg2PmtSetCallBack(parser->mpeg2Pmt, mpeg2InjectCA2Pmt, p_sys);
  mpeg2TsSetCallBack (parser->mpeg2Ts , mpeg2GatherTSCallback,     p_sys);
  	
  /* Set Psi Parser with the Ts Parser */
  mpeg2TsSetPsiParser(parser->mpeg2Ts, mpeg2PsiParserObject(parser->mpeg2Psi));

  /* Set Pat, Pmt Parser with the Psi Parser */
  memset(psi2TableIdToPop, 0, sizeof(psi2TableIdToPop));
  psi2TableIdToPop[MPEG2_PMT_TABLE_ID] = mpeg2PmtParserObject(parser->mpeg2Pmt);
  psi2TableIdToPop[MPEG2_PAT_TABLE_ID] = mpeg2PatParserObject(parser->mpeg2Pat);
  mpeg2PsiSetTableParsers(parser->mpeg2Psi, psi2TableIdToPop);

  mpeg2PatSetPsiParsers(parser->mpeg2Pat, NULL, mpeg2PsiParserObject(parser->mpeg2Psi));
  
  mmpPops[0] = mpeg2TsParserObject(parser->mpeg2Ts) ;
  mmpSetParsers(parser->mmp, mmpPops);
  
  p_sys->mpeg2TScop = parser->mpeg2Ts->cop; /* important !, let the cas have the context of the TS parser */

  return parser;
}

/* Free the parser */
void freeParser(parser_p parser)
{
  if (parser)
  {
	  /* Propagate free on each of the individual parsers */
	  mpeg2TsFree (parser->mpeg2Ts );
	  mpeg2PsiFree(parser->mpeg2Psi);
	  mpeg2PatFree(parser->mpeg2Pat);
	  mpeg2PmtFree(parser->mpeg2Pmt);
	  free(parser);
  }
  return;
}

#if 0
static RetCodeId    retCodeId;

/* ARGSUSED  changed from mpeg2TsParse()*/
static RetCode
mpeg2TsParseForCAS(void *instp, MmpContextObject *cop, Pipe pipe)
{
    RetCode             retCode;

    Mpeg2Ts             mpeg2Ts = (Mpeg2Ts) instp;
    Mpeg2TsInfo         info;
    Mpeg2TsInfo        *infop = &info;
    unsigned		lastPid = MPEG2_TS_NULL_PID;
    
    mpeg2Ts->isDirty = TRUE;
    MMP_CONTEXT_ADDPIPE(mpeg2Ts->cop, MPEG2_TS_PAT_PID,
			MMP_PARSER_NEWPIPE(mpeg2Ts->psip, mpeg2Ts->cop));

    MMP_CONTEXT_ADDPIPE(mpeg2Ts->cop, MPEG2_TS_CAT_PID,
			MMP_PARSER_NEWPIPE(mpeg2Ts->psip, mpeg2Ts->cop));

    while (pipeIsAvail(pipe, 1)) {
	PipePosition        tpEnd;
	unsigned            pid;
	Pipe                pidPipe;

	(void) memset(infop, 0, sizeof(*infop));

	infop->position = pipePosition(pipe);
	tpEnd = infop->position + MPEG2_TS_PKT_SIZE;

	/* added for CA injecton */
	if (mpeg2Ts->callBack != NULL)
	{
	    mpeg2GatherTS(mpeg2Ts, pipe);
	}
	/* ---> */
       
	infop->transportPacket = *PIPE_GET(pipe, Mpeg2TsTransportPacket);
	if (infop->transportPacket.syncByte != MPEG2_TS_SYNC_BYTE) {
	    retCode = RETCODE_CONS(retCodeId, MPEG2_TS_ERROR_SYNC);
	    goto error;
	}
	pid = MPEG2_TS_TRANSPORT_PACKET_PID(&infop->transportPacket);
	if (pid == MPEG2_TS_NULL_PID) {
	    infop->payloadLen = tpEnd - pipePosition(pipe);
	    goto skip;
	}
	if (infop->transportPacket.transportErrorIndicator) {
	    retCode = RETCODE_CONS(retCodeId, MPEG2_TS_ERROR_TEI);
	    goto error;
	}
	switch (infop->transportPacket.adaptationFieldControl) {
	case MPEG2_TS_ADAPTATION_FIELD_CONTROL_RESERVED:
	    retCode = RETCODE_CONS(retCodeId, MPEG2_TS_ERROR_AFC);
	    goto error;
	case MPEG2_TS_ADAPTATION_FIELD_CONTROL_ADAPTATION_ONLY:
	case MPEG2_TS_ADAPTATION_FIELD_CONTROL_ADAPTATION_PAYLOAD:
	    infop->adaptationFieldLength = *PIPE_PEEK(pipe, u8);
	    if (infop->adaptationFieldLength == 0) {
		PIPE_SKIP(pipe, u8);
	    } else if ((retCode = mpeg2TsAdaptationField(mpeg2Ts, pipe, infop))
		       != RETCODE_SUCCESS) {
		goto error;
	    }
	    break;
	}
	infop->payloadLen = tpEnd - pipePosition(pipe);
	if (infop->payloadLen < 0
		|| (infop->payloadLen > 0
		    && infop->transportPacket.adaptationFieldControl
		    == MPEG2_TS_ADAPTATION_FIELD_CONTROL_ADAPTATION_ONLY)) {
	    retCode = RETCODE_CONS(retCodeId, MPEG2_TS_ERROR_LEN);
	    goto error;
	}
skip:
	if (pid != lastPid && pid != MPEG2_TS_NULL_PID) {
	    /*
	     * Switching pid. Force child pipe to parse accumulated 
	     * TS payloads because that might be a PSI table that
	     * will create a new pipe (the one for this new payload).
	     */
	    retCode = pipeSync(pipe);
	    if (retCode != RETCODE_SUCCESS) {
		goto done;
	    }
	    lastPid = pid;
	}

	if (mpeg2Ts->callBack != NULL) {
	    /* added for CA injecton by Yongfang Liang */
	    mpeg2TSPsi(mpeg2Ts, MMP_CONTEXT_PIDTOPIPE(mpeg2Ts->cop, pid));
	    /* ----> */
	    retCode = (*mpeg2Ts->callBack) (mpeg2Ts->token, infop,
					    RETCODE_SUCCESS);
	    if (retCode != RETCODE_SUCCESS) {
		return retCode;
	    }
	}
	if (pid == MPEG2_TS_NULL_PID) {
	    infop->transportPacket.adaptationFieldControl
		= MPEG2_TS_ADAPTATION_FIELD_CONTROL_PAYLOAD_ONLY;
	}
	switch (infop->transportPacket.adaptationFieldControl) {
	case MPEG2_TS_ADAPTATION_FIELD_CONTROL_ADAPTATION_PAYLOAD:
	case MPEG2_TS_ADAPTATION_FIELD_CONTROL_PAYLOAD_ONLY:
	    pidPipe = MMP_CONTEXT_PIDTOPIPE(mpeg2Ts->cop, pid);
	    if (pidPipe == NULL) {
		pidPipe = PIPE_NULL;
	    }
	    retCode = pipeTransfer(pidPipe, pipe, infop->payloadLen,
			  infop->transportPacket.payloadUnitStartIndicator,
			  mpeg2Ts->doFlush);
	    if (retCode != RETCODE_SUCCESS) {
		goto done;
	    }
	    break;
	}
    }
    retCode = RETCODE_SUCCESS;
    goto done;
error:
    if (mpeg2Ts->callBack != NULL) {
	(void) (*mpeg2Ts->callBack) (mpeg2Ts->token, infop, retCode);
    }
done:
    /*
     * PAT and CAT pipes (along with any other created pipes)
     * are deleted when mpeg2TsRecover is called from pipeThreadWrapper.
     */
 
    return retCode;
}
#endif

