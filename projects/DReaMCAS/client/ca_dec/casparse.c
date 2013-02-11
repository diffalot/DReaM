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
 * $(@)casparse.c $Revision: 1.1.1.1 $ $Date: 2006/03/15 13:12:09 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
 
#include "ca_dec.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define TEST_MEMORY(p) if(p==NULL){fprintf(stderr,"Out of Memory: %s line %d\n",__FILE__,__LINE__);exit(-1);}
#define CAS_TRUE  1
#define CAS_FALSE 0

/* entitlement message type */
typedef enum 
{
    ET_NONE,
    ET_ECM,
    ET_EMM
} entitlement_type_e;

/* structure of entitle message section */
typedef struct
{
    /* Section elements: */
    uint8_t      i_table_id;                  /*  8 bits */
    uint8_t      b_section_syntax_indicator;  /*  1 bit  */
    uint8_t      i_DVB_reserved;              /*  1 bit  */
    uint8_t      i_ISO_reserved;              /*  2 bit  */
    uint16_t     i_CA_section_length;         /* 12 bits */
    uint8_t      *p_CA_section_data;          /* variable */
    
    /* Control data: */
    uint32_t     i_need;                 
    uint8_t      b_have_section_header;      
    uint16_t     i_max_section_size;
    uint8_t      *p_payload_end;
}ca_em_section_t;

/* structure of CA descriptor */
typedef struct
{
    uint16_t           i_ca_system_id;     /* CA system ID */
    uint16_t           i_ca_pid;           /* pid */
    uint8_t            *p_private_data;    /* private data */
    uint8_t            i_private_length;   /* length */
    
    /* Control data for gathering entitle message : */
    ca_em_section_t    *p_em_section;      /* EM section */
    uint8_t            i_continuity_counter; /* EM section continuity counter */
    void               *token;             /* token for the callback function */
    EMSCallBack         pf_callback;       /* call back function when EM section is retrieved */
} cas_ca_descriptor_t;

typedef struct cas_ca_list_s 
{
    cas_ca_descriptor_t   *elem;   /* node */
    struct cas_ca_list_s  *next;   /* next node */
} cas_ca_list_t;

/* structure of one pid */
typedef struct 
{
    uint16_t            i_pid;         /* pid number */
    boolean             b_used;        /* is it being used */
    uint16_t            i_pn;          /* program number */
    entitlement_type_e  i_entitle_message_type;
    cas_ca_list_t          *p_cadr_list;  /* CA descriptors with this pid */
} cas_pid_t;

/* structure of one program */
typedef struct
{
    uint16_t         i_pn;           /* program number */
    cas_ca_list_t    *p_cadr_list;   /* CA descriptors for the whole program */
} cas_program_t;

typedef struct cas_program_list_s 
{
    cas_program_t              *elem;   /* node */
    struct cas_program_list_s  *next;   /* next node */
} cas_program_list_t;

/* system handle structure */
typedef struct cas_sys_s
{
    cas_pid_t   *pid;
	cas_program_list_t  *p_program_list; 
	cas_ca_list_t  *p_emm_ca_list;
	EMSCallBack pf_callback;
	void        *token;
    /* Supported Conditional Access System */
    uint16_t    i_ca_system_id;
} cas_sys_t;

static void delete_em_section(ca_em_section_t *p_obj);
static void cas_ca_list_delete(cas_ca_list_t *list);

static void delete_program_info(cas_program_t *p_obj)
{
    if (p_obj)
    {
    	if (p_obj->p_cadr_list)
    	{
    		cas_ca_list_delete(p_obj->p_cadr_list);
    	}
    	free(p_obj);
    }
    return;    
}

static cas_program_t * create_program_info(uint16_t pn)
{
	cas_program_t *obj = NULL;
    
	obj = malloc(sizeof(cas_program_t));
	TEST_MEMORY(obj);
	
	obj->p_cadr_list    = NULL;
	obj->i_pn           = pn;
    return obj;
}

static cas_program_list_t *cas_program_list_append(cas_program_list_t *list, cas_program_t *obj) 
{
    if( list == NULL )
    {
        list = (cas_program_list_t *)malloc(sizeof(cas_program_list_t));
        TEST_MEMORY(list);
        list->elem = obj;
        list->next = NULL;
    }
    else
    {
    	list->next = cas_program_list_append(list->next, obj);
    }
    return (list);
}

static void cas_program_list_delete(cas_program_list_t *list)
{
    if (list)
    {
		if (list->next == NULL)
		{
			delete_program_info(list->elem);
			free(list);
		}
		else
		{
	        cas_program_list_delete(list->next);
		}
    }
    return;
}

static void delete_ca_descriptor(cas_ca_descriptor_t *p_obj)
{
    if (p_obj)
    {
    	if (p_obj->p_private_data)
    	{
    		free(p_obj->p_private_data);
    	}
    	if(p_obj->p_em_section)
    	{
    	    delete_em_section(p_obj->p_em_section);
    	}
    	free(p_obj);
    }
    return;    
}

static cas_ca_descriptor_t * create_ca_descriptor(uint16_t ca_system_id,
	uint16_t ca_pid, uint8_t * p_private_data, uint32_t i_private_length, EMSCallBack callback, void *token)
{
	cas_ca_descriptor_t *dr = NULL;
    
	dr = (cas_ca_descriptor_t *)malloc(sizeof(cas_ca_descriptor_t));
	TEST_MEMORY(dr);
	
	dr->i_ca_system_id    = ca_system_id;
	dr->i_ca_pid          = ca_pid;
	dr->p_private_data    = NULL;
	dr->i_private_length  = 0;
	dr->p_em_section      = NULL;
    dr->i_continuity_counter = -1;
    dr->token             = token;
    dr->pf_callback       = callback;
    if (p_private_data && i_private_length > 0)
    {
        dr->p_private_data   = (uint8_t *)malloc(sizeof(uint8_t) * i_private_length);
        TEST_MEMORY(dr->p_private_data);
        dr->i_private_length = i_private_length;
    }
    return dr;
}

static cas_ca_list_t *cas_ca_list_append(cas_ca_list_t *list, cas_ca_descriptor_t *obj) 
{
    if( list == NULL )
    {
        list = (cas_ca_list_t *)malloc(sizeof(cas_ca_list_t));
        TEST_MEMORY(list);
        list->elem = obj;
        list->next = NULL;
    }
    else
    {
    	list->next = cas_ca_list_append(list->next, obj);
    }
    return (list);
}

static void cas_ca_list_delete(cas_ca_list_t *list)
{
    if (list)
    {
		if (list->next == NULL)
		{
			delete_ca_descriptor(list->elem);
			free(list);
		}
		else
		{
	        cas_ca_list_delete(list->next);
		}
    }
    return;
}

static void delete_pid(cas_pid_t *p_obj)
{
    if (p_obj)
    {
    	if (p_obj->p_cadr_list)
    	{
    		cas_ca_list_delete(p_obj->p_cadr_list);
    	}
    }
    return;   
}

/*
----------------------------------------------------------------------
createCAS: return a CAS handle
ARGUMENTS:
RETURN:
    structure pointer
----------------------------------------------------------------------
*/
cas_sys_p casOpen(uint32_t id)
{
	uint16_t       i = 0;
	cas_sys_t *p_sys = NULL;
	cas_pid_t *pid   = NULL;
	
	p_sys = (cas_sys_t *)malloc(sizeof(cas_sys_t));
	TEST_MEMORY(p_sys);
	p_sys->pid = (cas_pid_t *)malloc(sizeof(cas_pid_t) * 8192);
	TEST_MEMORY(p_sys->pid);
	
	/* pid initialization */
    for( i = 0; i < 8192; i++ )
    {
        pid = &p_sys->pid[i];
        pid->i_pid            = i;
        pid->b_used           = CAS_FALSE;
        pid->i_pn             = -1;
        pid->i_entitle_message_type = ET_NONE;                  
        pid->p_cadr_list      = NULL;
    }
    /* reserved pids */
    for( i = 0x0; i < 0x10; i++ )
    {
        pid = &p_sys->pid[i];
        pid->b_used   = CAS_TRUE;
    }

    p_sys->p_program_list = NULL;
    p_sys->p_emm_ca_list  = NULL;
    p_sys->i_ca_system_id = id;
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
void casDelete(cas_sys_p p_sys)
{
	if (p_sys)
	{
		uint16_t i = 0;
		if (p_sys->pid)
		{
		    for( i = 0; i < 8192; i++ )
		    {
		        cas_pid_t *pid = &p_sys->pid[i];
		        delete_pid(pid);
		    }
		    free(p_sys->pid);
		    p_sys->pid = NULL;
		}
	    if (p_sys->p_program_list)
	    {
	    	cas_program_list_delete(p_sys->p_program_list);
	    	p_sys->p_program_list = NULL;
	    }
	    if (p_sys->p_emm_ca_list)
	    {
	    	cas_ca_list_delete(p_sys->p_emm_ca_list);
	    	p_sys->p_emm_ca_list = NULL;
	    }
	    
	    free(p_sys);
	}
	return;
}

/*
----------------------------------------------------------------------
casSetEMSCallBack: set entitle message section callback function
ARGUMENTS:
    p_sys:        pointer to the structure
    callback:     
    token:        
RETURN:
    none
----------------------------------------------------------------------
*/
void casSetEMSCallBack(cas_sys_p p_sys, EMSCallBack callback, void *token)
{
    if (p_sys)
    {
    	p_sys->pf_callback = callback;
    	p_sys->token    = token;
    }
    return;
}

/*
----------------------------------------------------------------------
casSetProgramNum: assign pid to one program
ARGUMENTS:
    p_sys:        pointer to the structure
    pid_no:       pid number
    pn:           program number of this PID
RETURN:
    none
----------------------------------------------------------------------
*/
void casSetProgramNum(cas_sys_p p_sys, uint16_t pid_no, uint16_t pn)
{
    if (p_sys)
    {
	    cas_pid_t *pid = &p_sys->pid[pid_no];
    	pid->i_pn      = pn;
    	pid->b_used    = CAS_TRUE;
    }
    return;
}

/*
----------------------------------------------------------------------
casAddCADescriptor2PID: add ca descriptor to pid
ARGUMENTS:
    p_sys:        pointer to the structure
    pn:           program number of this PID
    epid:         pid number
    ca_system_id:
    ca_pid:
    p_private_data:
    i_private_length:
    callback:    the function that is called once 
                 the corresponding ECM/EMM is parsed wiht pid = ca_pid
    
RETURN:
    none
----------------------------------------------------------------------
*/
void casAddCADescriptor2PID( cas_sys_p p_sys, uint16_t pn, uint16_t epid, uint16_t ca_system_id,
	uint16_t ca_pid, uint8_t * p_private_data, uint32_t i_private_length)
{
	cas_pid_t *pid  = &p_sys->pid[epid];
    cas_ca_descriptor_t *p_cadr = create_ca_descriptor(ca_system_id,
	              ca_pid, p_private_data, i_private_length, p_sys->pf_callback, p_sys->token);
    pid->p_cadr_list = cas_ca_list_append(pid->p_cadr_list, p_cadr); 
    pid->i_pn        = pn;
    pid->b_used      = CAS_TRUE;

    /* set the type of corresponding ECM */
    pid  = &p_sys->pid[ca_pid];
    pid->i_entitle_message_type = ET_ECM;
    pid->b_used                 = CAS_TRUE;
    
    return;
}

/*
----------------------------------------------------------------------
casAddCADescriptor2PRO: add ca descriptor to program
ARGUMENTS:
    p_sys:        pointer to the structure
    pn:           program number of this PID
    ca_system_id:
    ca_pid:
    p_private_data:
    i_private_length:
    callback:    the function that is called once 
                 the corresponding ECM/EMM is parsed wiht pid = ca_pid
    
RETURN:
    none
----------------------------------------------------------------------
*/
void casAddCADescriptor2PRO( cas_sys_p p_sys, uint16_t pn, uint16_t ca_system_id,
	uint16_t ca_pid, uint8_t * p_private_data, uint32_t i_private_length)
{
	cas_pid_t *pid             = NULL;
	cas_program_list_t *p_list = NULL;
	cas_program_t *p_info      = NULL;
	
    cas_ca_descriptor_t *p_cadr = create_ca_descriptor(ca_system_id,
	              ca_pid, p_private_data, i_private_length, p_sys->pf_callback, p_sys->token);
    
    for (p_list = p_sys->p_program_list; p_list != NULL; 
          p_list = p_list->next)
    {
        p_info = (cas_program_t *)p_list->elem;
        if (p_info && p_info->i_pn == pn)
        {
        	break;
        }
        else
        {
        	p_info = NULL;
        }
    }
          
    if (p_info == NULL)
    {
    	p_info = create_program_info(pn);
    	p_sys->p_program_list = cas_program_list_append(p_sys->p_program_list, p_info);
    }
    
	p_info->p_cadr_list = cas_ca_list_append(p_info->p_cadr_list, p_cadr); 

    /* set the type of corresponding ECM */
    pid  = &p_sys->pid[ca_pid];
    pid->i_entitle_message_type = ET_ECM;
    pid->b_used                 = CAS_TRUE;
    pid->i_pn                   = pn;
    
    return;
}

/*
----------------------------------------------------------------------
casAddEMMCADescriptor: add EMM ca descriptor
ARGUMENTS:
    p_sys:        pointer to the structure
    ca_system_id:
    ca_pid:
    p_private_data:
    i_private_length:
    callback:    the function that is called once 
                 the corresponding ECM/EMM is parsed wiht pid = ca_pid
    
RETURN:
    none
----------------------------------------------------------------------
*/
void casAddEMMCADescriptor( cas_sys_p p_sys, uint16_t ca_system_id,
	uint16_t ca_pid, uint8_t * p_private_data, uint32_t i_private_length)
{
	cas_pid_t *pid          = NULL;
	
    cas_ca_descriptor_t *p_cadr = create_ca_descriptor(ca_system_id,
	              ca_pid, p_private_data, i_private_length, p_sys->pf_callback, p_sys->token);
    
    p_sys->p_emm_ca_list = cas_ca_list_append(p_sys->p_emm_ca_list, p_cadr);

    /* set the type of corresponding ECM */
    pid  = &p_sys->pid[ca_pid];
    pid->i_entitle_message_type = ET_EMM;
    pid->b_used                 = CAS_TRUE;
    
    return;
}

/*
----------------------------------------------------------------------
casGetPIDKey: get corresponding ECM payload
ARGUMENTS:
    p_sys:        pointer to the structure
    pn:           program number
    pid_no:       pid # of the TS packet
    key_buf:      pointer to pointer to the EM payload
    key_len:      pointet to the length of the EM payload
RETURN:
    none
----------------------------------------------------------------------
*/
void casGetPIDKey(cas_sys_p p_sys, uint16_t pn,uint16_t pid_no, ca_info_t *p_info)
{
	cas_program_list_t *p_program_list  = NULL;
    cas_ca_list_t *p_ca_list = NULL;
    cas_ca_descriptor_t *dr = NULL;
	cas_pid_t *pid = &p_sys->pid[pid_no];

    p_info->b_have_key = CAS_FALSE;
    
	/* search its own CA descriptor list first */
	for (p_ca_list = pid->p_cadr_list; p_ca_list != NULL;
	     p_ca_list = p_ca_list->next)
	{
        dr = (cas_ca_descriptor_t *)p_ca_list->elem;
        if (dr->i_ca_system_id == p_sys->i_ca_system_id
        	&& dr->p_em_section)
        {
			p_info->i_ca_system_id   = dr->i_ca_system_id;
	        p_info->i_ca_pid         = dr->i_ca_pid;
	        p_info->p_private_data   = dr->p_private_data;
	        p_info->i_private_length = dr->i_private_length;
	        p_info->i_CA_section_length = dr->p_em_section->i_CA_section_length;
	        p_info->p_CA_section_data   = dr->p_em_section->p_CA_section_data + 3;
	        p_info->b_have_key = CAS_TRUE;
        	goto done;
        }
	}

	/* search the program CA dr list */
    for (p_program_list = p_sys->p_program_list; p_program_list != NULL; 
          p_program_list = p_program_list->next)
    {
        cas_program_t *p_pro_info = (cas_program_t *)p_program_list->elem;
        if (p_pro_info && p_pro_info->i_pn == pn)
        {
	        for (p_ca_list = p_pro_info->p_cadr_list; p_ca_list != NULL; 
	             p_ca_list = p_ca_list->next)
	        {
	        	dr = (cas_ca_descriptor_t *)p_ca_list->elem;
	        	if (dr->i_ca_system_id == p_sys->i_ca_system_id
	        		&& dr->p_em_section)
	        	{
			        p_info->i_ca_system_id   = dr->i_ca_system_id;
			        p_info->i_ca_pid         = dr->i_ca_pid;
			        p_info->p_private_data   = dr->p_private_data;
			        p_info->i_private_length = dr->i_private_length;
			        p_info->i_CA_section_length = dr->p_em_section->i_CA_section_length;
			        p_info->p_CA_section_data   = dr->p_em_section->p_CA_section_data + 3;
			        p_info->b_have_key = CAS_TRUE;
		        	goto done;
	        	}
	        }
        }
    }
done:
	return ;
}

static void delete_em_section(ca_em_section_t *p_obj)
{
    if (p_obj)
    {
    	if (p_obj->p_CA_section_data)
    	{
    		free(p_obj->p_CA_section_data);
    	}
    	free(p_obj);
    }
    return;    
}

static ca_em_section_t *create_em_section()
{
	ca_em_section_t *p_section = NULL;
	p_section = malloc(sizeof(ca_em_section_t));
	TEST_MEMORY(p_section);
	
	p_section->i_need                = 3; /*need the header to know the section length */
	p_section->b_have_section_header = CAS_FALSE; /* waiting for the header */
	p_section->i_CA_section_length = 0;
	p_section->i_max_section_size  = 256; /* ETR289 */
	
	p_section->p_CA_section_data      = (uint8_t *)malloc(sizeof(uint8_t)*256);
	TEST_MEMORY(p_section->p_CA_section_data);
	p_section->p_payload_end = p_section->p_CA_section_data;
	return p_section;
}

static cas_ca_descriptor_t * get_em_ca_descriptor(cas_sys_t *p_sys, uint16_t pid_no)
{
	cas_program_list_t *p_program_list  = NULL;
    cas_ca_list_t *p_ca_list = NULL;
    cas_ca_descriptor_t *dr = NULL;
    int i = 0;
    cas_pid_t *pid = &p_sys->pid[pid_no];

	/* search the program list */
    for (p_program_list = p_sys->p_program_list; p_program_list != NULL; 
          p_program_list = p_program_list->next)
    {
        cas_program_t *p_info = (cas_program_t *)p_program_list->elem;
        if (p_info && p_info->i_pn == pid->i_pn)
        {
		    for (p_ca_list = p_info->p_cadr_list; p_ca_list != NULL; 
		         p_ca_list = p_ca_list->next)
		    {
		    	dr = (cas_ca_descriptor_t *)p_ca_list->elem;
		    	if (dr->i_ca_pid == pid_no)
		    	{
			    	goto done;
		    	}
		    }
        }
    }
          
	/* check emm ca list */
	for (p_ca_list = p_sys->p_emm_ca_list; p_ca_list != NULL;
	     p_ca_list = p_ca_list->next)
	{
	    dr = (cas_ca_descriptor_t *)p_ca_list->elem;
	    if (dr->i_ca_pid == pid_no)
	    {
	    	goto done;
	    }
	}

    /* check all other descriptors */
    for (i = 0; i < 8192; i++)
    {
	    cas_pid_t *pid = &p_sys->pid[i];
		for (p_ca_list = pid->p_cadr_list; p_ca_list != NULL;
		     p_ca_list = p_ca_list->next)
		{
	        dr = (cas_ca_descriptor_t *)p_ca_list->elem;
	        if (dr->i_ca_pid == pid_no)
	        {
	        	goto done;
	        }
		}
    }
          
done:
	return dr;
}

static void parse_ems(cas_ca_descriptor_t *p_ca_dr, uint8_t *p_data, uint32_t i_pkt_len)
{
    uint8_t i_expected_counter = 0;       /* expected continuity counter */
    ca_em_section_t *p_section = NULL;    /* em section */
    uint8_t *p_payload_pos     = NULL;    /* current position */
    uint32_t i_available       = 0;       /* available bytes left*/

    /* Continuity check */
    i_expected_counter = (p_ca_dr->i_continuity_counter + 1) & 0xf;
    p_ca_dr->i_continuity_counter = p_data[3] & 0xf;

    if(i_expected_counter == ((p_ca_dr->i_continuity_counter + 1) & 0xf))
    {
      /* we got an error: packet duplicated */
      return;
    }

    if(i_expected_counter != p_ca_dr->i_continuity_counter)
    {
      /* we got an error: packet missing */
      if(p_ca_dr->p_em_section)
      {
        delete_em_section(p_ca_dr->p_em_section);
        p_ca_dr->p_em_section = NULL;
      }
    }

    /* have data? */
    if(!(p_data[3] & 0x10))
    {
      return;
    }

    /* Skip the adaptation_field if we have it */
    if(p_data[3] & 0x20)
      p_payload_pos = p_data + 5 + p_data[4];
    else
      p_payload_pos = p_data + 4;
    
    p_section = p_ca_dr->p_em_section;

    /* check if a new section begins: unit_start_indicator */
    if(p_data[1] & 0x40)
    {
        if (p_section)
        {
      	  delete_em_section(p_section);
      	  p_section = p_ca_dr->p_em_section = NULL;
        }
        p_ca_dr->p_em_section = p_section = create_em_section();
        /* update payload position: skip pointer_field */
        p_payload_pos = p_payload_pos + *p_payload_pos + 1;
    }
    else if (p_section == NULL)
    {
    	return;
    }
    
    /* Remaining bytes in the payload */
    i_available = i_pkt_len + p_data - p_payload_pos;
  
    while(i_available > 0)
    {
        if (i_available < p_section->i_need)
        {
	        /* not enough bytes */
	        memcpy(p_section->p_payload_end, p_payload_pos, i_available);
	        p_section->p_payload_end += i_available;
	        p_section->i_need        -= i_available;
	        break;
        }
        else 
        {
            memcpy(p_section->p_payload_end, p_payload_pos, p_section->i_need);
            p_payload_pos            += p_section->i_need;
            p_section->p_payload_end += p_section->i_need;
            i_available              -= p_section->i_need;
  
            if(p_section->b_have_section_header == CAS_FALSE)
            {
                p_section->b_have_section_header = CAS_TRUE;
                p_section->i_CA_section_length = ((uint16_t)(p_section->p_CA_section_data[1] & 0x0f)) << 8
                             | p_section->p_CA_section_data[2];
                p_section->i_need = p_section->i_CA_section_length;
          
                /* Check that the section isn't too long */
                if(p_section->i_need > p_section->i_max_section_size - 3)
                {
                    delete_em_section(p_section);
                    p_ca_dr->p_em_section = p_section = create_em_section();
                    i_available = i_pkt_len + p_data - p_payload_pos;
                }
            }
            else
		    {
		        /* now we can parse the data */
		        p_section->b_section_syntax_indicator = p_section->p_CA_section_data[1] & 0x80;
		  
		        if(!p_section->b_section_syntax_indicator)
		        {
		            /* PSI section is valid */
		            p_section->i_table_id = p_section->p_CA_section_data[0];

                    if (p_ca_dr->pf_callback)
                    {
                        ca_info_t info;
                        info.i_ca_system_id   = p_ca_dr->i_ca_system_id;
                        info.i_ca_pid         = p_ca_dr->i_ca_pid;
                        info.p_private_data   = p_ca_dr->p_private_data;
                        info.i_private_length = p_ca_dr->i_private_length;
                        info.i_CA_section_length = p_section->i_CA_section_length;
                        info.p_CA_section_data  = p_section->p_CA_section_data + 3;
                        info.b_have_key         = CAS_TRUE;
   		                p_ca_dr->pf_callback(p_ca_dr->token, &info);
                    }
		        }
		        else
		        {
		            /* invalid PSI section*/
		            delete_em_section(p_section);
		            p_ca_dr->p_em_section = p_section = NULL;
		        }
                /* skip the stuffing byte */
                while (i_available > 0 && (*p_payload_pos) == 0xFF)
                {
                	i_available--;
                	p_payload_pos++;

                }
		        /* process the rest of the data in the packet */
		        if(i_available)
		        {
                    delete_em_section(p_section);
	  	            p_ca_dr->p_em_section = p_section = create_em_section();
		        }
		    }
        }
    }  /* end of while */
    return;
}

/*
----------------------------------------------------------------------
casParseTS:  parse one TS packet (ECM/EMM)
ARGUMENTS:
    p_sys:        pointer to the structure
    p_data:       pointer to TS packet buffer
    i_pkt_len:    TS packet length
RETURN:
    none
----------------------------------------------------------------------
*/
void casParseTS(cas_sys_p p_sys, uint8_t *p_data, uint32_t i_pkt_len)
{
	uint16_t pid_no = ((p_data[1] & 0x1f) << 8) | p_data[2];
	cas_pid_t *pid = &p_sys->pid[pid_no];
	
    /* TS start code */
    if(p_data[0] != 0x47)
    {
      /* we got an error: not TS packet*/
      return;
    }
	
	switch (pid->i_entitle_message_type)
	{
		case ET_ECM:
		case ET_EMM:
			{
				cas_ca_descriptor_t *p_dr = get_em_ca_descriptor(p_sys, pid_no);
				parse_ems(p_dr, p_data, i_pkt_len);
		    }
			break;
		case ET_NONE:
		default:
			break;
	}
	return;
}

