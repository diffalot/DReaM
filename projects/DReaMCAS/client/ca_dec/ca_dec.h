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
 * $(@)ca_dec.h $Revision: 1.1.1.1 $ $Date: 2006/03/15 13:12:09 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
 
#ifndef _CA_DEC_H
#define _CA_DEC_H

typedef unsigned char       uint8_t;
typedef signed char         int8_t;
typedef unsigned short      uint16_t;
typedef signed short        int16_t;
typedef unsigned int        uint32_t;
typedef signed int          int32_t;
typedef uint8_t             boolean;

typedef struct
{
    uint8_t      b_have_key;         
    uint16_t     i_ca_system_id;      /* CA system ID */
    uint16_t     i_ca_pid;            /* pid */
    uint8_t      *p_private_data;     /* private data */
    uint8_t      i_private_length;    /* length */
    uint16_t     i_CA_section_length; /* 12 bits */
    uint8_t      *p_CA_section_data;  /* variable */
}ca_info_t;

typedef void  (*EMSCallBack) (void *token, ca_info_t *p_info);

typedef struct cas_sys_s *cas_sys_p;

/* Funtions declaration */
#ifdef __cplusplus
extern "C" {
#endif

cas_sys_p casOpen(uint32_t id);
void casDelete(cas_sys_p p_sys);
void casSetEMSCallBack(cas_sys_p p_sys, EMSCallBack callback, void *token);
void casSetProgramNum(cas_sys_p p_sys, uint16_t pid_no, uint16_t pn);

/* add ca descriptor to pid */
void casAddCADescriptor2PID( cas_sys_p p_sys, uint16_t pn, uint16_t epid, uint16_t ca_system_id,
	uint16_t ca_pid, uint8_t * p_private_data, uint32_t i_private_length);
	
/* add ca descriptor to program */
void casAddCADescriptor2PRO( cas_sys_p p_sys, uint16_t pn, uint16_t ca_system_id,
	uint16_t ca_pid, uint8_t * p_private_data, uint32_t i_private_length);

/* add ca descriptor in CAT */
void casAddEMMCADescriptor( cas_sys_p p_sys, uint16_t ca_system_id,
	uint16_t ca_pid, uint8_t * p_private_data, uint32_t i_private_length);

/* get corresponding ECM payload */
void casGetPIDKey(cas_sys_p p_sys, uint16_t pn,uint16_t pid_no, ca_info_t *p_info);

/* parse ECM or EMM */
void casParseTS(cas_sys_p p_sys, uint8_t *p_data, uint32_t i_pkt_len);
void vlcGetUserInfo( int i_argc, char *ppsz_argv[] );
char *GetConfigurationValue(char *configuration_name, char *field);
#define CONFIG_FILE_NAME "configure.txt"

#ifdef __cplusplus
}
#endif

#endif




