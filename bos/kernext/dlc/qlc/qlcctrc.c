static char sccsid[] = "@(#)15  1.5  src/bos/kernext/dlc/qlc/qlcctrc.c, sysxdlcq, bos411, 9428A410j 2/15/94 13:32:57";
/*
 * COMPONENT_NAME: (SYSXDLCQ) X.25 QLLC module
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or 
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "qlcg.h"   
#include "qlcq.h"  
#include "qlcqmisc.h"
#include "qlcv.h"  
#include "qlcvfac.h"  
#include "qlcb.h"  
#include "qlcp.h"
#include "qlcc.h"  
#include "qlcs.h"  
#include "qlcl.h"
#include "qlcctrc.h"

/*****************************************************************************
** Function : q_trace
**
** Description:
**
**  This function is called by one of five macros which are imbedded in the
** QLLC code and which each provide tracing of a particular type of event.
** The five types of event to be traced are:
**
**   > Start  (Link Stations only)
**   > Halt   (Link Stations only)
**   > Xmit   (Transmits)
**   > Recv   (Receives)
**   > Timer
**
** The macros are defined in the header file qlcctrc.h which is the companion
** file to this module qlcctrc.c.
**
** Tracing is performed by calling the trcgenkt function:
**
**      int  trcgenkt(chan_adr, hook_word, data_word, buffer_len, buffer_ptr)
**
** The x_type arg passed to q_trace is only valid for the cases of
** a timeout hook_word and a transmit trace.
** In all other cases it should be passed as NULL.
**
***************************************************************************/
void  q_trace(
  correlator_type  correlator,
  unsigned int     hook_word,
  unsigned int     x_type,
  gen_buffer_type *buffer_ptr,
  int		   op_result
  )

{
  unsigned int   trace_buffer_address;
  unsigned int   trace_buffer_offset; 
  unsigned int   data_word; 
  unsigned short trace_buffer_seg_id;
  char           data_area[36];
  unsigned int   trace_length;
  station_type  *station_ptr;
  char          *trace_data;
  bool           free_trace_data;
  char          *local_buf_ptr;
  trace_start_block_type trace_start_block;
  trace_halt_block_type trace_halt_block;
  
  /***************************************************************************
   ** Form station_ptr from station correlator.
   ***************************************************************************/
  station_ptr = (station_type *)correlator;
 
  /***************************************************************************
   ** free_trace_data is set to TRUE if trace_data has been malloc()ed
   ***************************************************************************/
  free_trace_data = FALSE;
  
  switch (hook_word)
  {
  case HKWD_SYSX_DLC_START:

    /* set output strings to blanks */
    memset(trace_start_block.station_tag, ' ', DLC_MAX_DIAG);
    memset(trace_start_block.remote_addr, ' ', DLC_MAX_NAME);

    data_word = (DLC_DL_QLLC << 16) | DLC_PL_X25; 

    /* copy data into trace block */
    strncpy( trace_start_block.station_tag,
		station_ptr->station_tag, DLC_MAX_DIAG);
    strncpy( trace_start_block.remote_addr,
		station_ptr->remote_addr, DLC_MAX_NAME);

    trace_length = sizeof(struct trace_start_block_type);
    trace_data = (char *)&trace_start_block;
    break; 

  case HKWD_SYSX_DLC_HALT:

    /* set output strings to blanks */
    memset(trace_start_block.station_tag, ' ', DLC_MAX_DIAG);
    memset(trace_start_block.remote_addr, ' ', DLC_MAX_NAME);

    /* copy data into trace block */
    strncpy( trace_halt_block.station_tag,
		station_ptr->station_tag, DLC_MAX_DIAG);
    strncpy( trace_halt_block.remote_addr,
		station_ptr->remote_addr, DLC_MAX_NAME);

    data_word = (DLC_DL_QLLC << 16) | DLC_PL_X25; 
    trace_halt_block.result_code = op_result;
    trace_length = sizeof(struct trace_halt_block_type);
    trace_data = (char *)&trace_halt_block;
    break; 

  case HKWD_SYSX_DLC_XMIT:

    /*
    ** This is the sad story.  At the time this trace point is called,
    ** the QLLC XID information has not been put into the buffer.
    ** so we must fake it by putting the same information into
    ** the trace buffer that will later be add to the tx buffer
    */

    trace_length = JSMBUF_LENGTH(buffer_ptr) - X25_OFFSETOF_USER_DATA;

    if ((!(station_ptr->flags & DLC_TRCL)) && (trace_length > 80))
      trace_length = 80;

    data_word = (DLC_DL_QLLC << 16) | x_type; 
    
    trace_data = (char *) malloc(trace_length);
    if (trace_data == NULL) {
      trace_length = 0;
      break;
    }
    free_trace_data = TRUE;

    if (x_type == 0x01)
    {

	if (buffer_ptr->m_next)
	{
		/* copy the DLC bytes into the trace buffer */

		*trace_data = (byte)command;
		*(trace_data+1) = (byte)qxid_cmd;

		memcpy(trace_data+2,
			MTOD(buffer_ptr->m_next, char *), trace_length-2);
	}
	else
	{
		JSMBUF_READ_BLOCK(buffer_ptr, 
			X25_OFFSETOF_USER_DATA, trace_data, trace_length);
	}
    }
    else
    {
	JSMBUF_READ_BLOCK(buffer_ptr, 
		X25_OFFSETOF_USER_DATA, trace_data, trace_length);
    }
    break;

  case HKWD_SYSX_DLC_RECV:

    trace_length = JSMBUF_LENGTH(buffer_ptr) - X25_OFFSETOF_USER_DATA;
    if ((!(station_ptr->flags & DLC_TRCL)) && (trace_length > 80))
      trace_length = 80;

    trace_data = (char *) malloc(trace_length);
    if (trace_data == NULL) {
      trace_length = 0;
      break;
    }
    free_trace_data = TRUE;

    JSMBUF_READ_BLOCK(buffer_ptr, 
      X25_OFFSETOF_USER_DATA, trace_data, trace_length);

    if (trace_data[1] == (char) 0xBF)	/* Defect 105370 */
	x_type = 0x01;
    else
	x_type = 0x02;

    data_word = (DLC_DL_QLLC << 16) | x_type; 
    break; 

  case HKWD_SYSX_DLC_TIMER:

    data_word = (DLC_DL_QLLC << 16) | x_type; 
    trace_length = 0;
    trace_data = (char *) NULL;
    break; 
  }

  trcgenkt(
    station_ptr->trace_channel,
    hook_word,
    data_word,
    trace_length,
    trace_data
    );

  /*
   * free trace_data if it needs freeing
   */
  if (free_trace_data && trace_data != NULL)
  {
    free(trace_data);
  }
}


x25_num_in_chain(buf)
struct	mbuf	*buf;

{
	int		num_in_chain=0;
	struct mbuf	*m;

	m = buf;

	while (m != NULL)
	{
		++num_in_chain;
		m = m->m_next;
	}

	return(num_in_chain);
}
