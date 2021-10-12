static char sccsid[] = "@(#)45  1.7  src/bos/kernext/ent/en3com_util.c, sysxent, bos411, 9431A411a 8/2/94 13:01:00";
/*
 * COMPONENT_NAME: sysxent -- High Performance Ethernet Device Driver
 *
 * FUNCTIONS: 
 *		en3com_pio_retry
 *		en3com_gen_crc
 *		en3com_multi_add
 *		en3com_multi_del
 *		en3com_cdt_func
 *		en3com_cdt_add
 *		en3com_cdt_del
 *		en3com_trace
 *		en3com_logerr
 *		en3com_loopdelay
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 *
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <sys/lock_def.h>
#include <sys/errno.h>
#include <sys/device.h>
#include <sys/malloc.h>
#include <sys/adspace.h>
#include <sys/dump.h>
#include <sys/watchdog.h>
#include <sys/intr.h>
#include <sys/err_rec.h>
#include <sys/trcmacros.h>
#include <sys/mbuf.h>
#include <sys/ndd.h>
#include <sys/cdli.h>
#include <sys/ethernet_mibs.h>
#include <sys/cdli_entuser.h>
#include <net/spl.h>

#include "en3com_dds.h"
#include "en3com_mac.h"
#include "en3com_hw.h"
#include "en3com_pio.h"
#include "en3com.h"
#include "en3com_errids.h"


extern en3com_dd_ctl_t	en3com_dd_ctl;

void en3com_trace();
void en3com_logerr();

/*****************************************************************************/
/*
 * NAME:     en3com_pio_retry
 *
 * FUNCTION: This routine is called when a pio routine returns an exception.
 *		Retry the pio function and do error logging.
 *
 * EXECUTION ENVIRONMENT: process and interrupt.
 *
 * NOTES:
 *	This routine is called by using macros defined in en3com_pio.h
 *
 * CALLED FROM:
 *	en3com_stop
 *	en3com_getvpd
 *	en3com_fixvpd
 *	en3com_start 
 *	en3com_setpos 
 *	en3com_getcfg 
 *	en3com_setup 
 *	en3com_tx_setup 
 *	en3com_rv_setup 
 *	en3com_rv_start 
 *	en3com_xmit
 *	en3com_cmd
 *	en3com_getmib
 *	en3com_getstat
 *	en3com_clrstat
 *	en3com_intr
 *	en3com_rv_intr
 *	en3com_tx_intr
 *	en3com_exec_intr
 *	en3com_parity_err
 *	en3com_stimer
 *      
 * INPUT:
 *      p_dev_ctl       - device control structure.
 *      excpt_code      - exception code from first pio 
 *      iofunc          - io function to retry
 *      ioaddr          - io address of the exception
 *      ioparam         - parameter to PIO routine
 *      cnt             - for string copies
 *
 * RETURNS:  
 *	0 - retry successfully
 * 	exception code
 */
/*****************************************************************************/
en3com_pio_retry (
    en3com_dev_ctl_t	*p_dev_ctl,	/* adapter control structure        */
    int 		excpt_code,	/* exception code from original PIO */
    enum pio_func 	iofunc,		/* io function to retry	            */
    int 		ioaddr,		/* io address of the exception	    */
    long		ioparam,	/* parameter to PIO routine	    */
    int			cnt)		/* for string copies                */

{
    int			retry_count = PIO_RETRY_COUNT;
    int			ipri;
    ndd_statblk_t	stat_blk;   	/* Status block                      */
    
    
    TRACE_SYS(HKWD_EN3COM_OTHER, "NioB", (ulong)p_dev_ctl, excpt_code, 
	(ulong)iofunc);
    TRACE_SYS(HKWD_EN3COM_OTHER, "NioC", (ulong)ioaddr, ioparam, 0);

    while (TRUE) {
        /* 
         * check if out of retries
         */
        if (retry_count <= 0) {
          /* 
           * Log pio error and send a status block to user
           */
	  en3com_logerr(p_dev_ctl, ERRID_EN3COM_PIOFAIL, __LINE__,
		__FILE__, excpt_code, 0, 0); 
          if (p_dev_ctl->device_state == OPENED) { 
		bzero(&stat_blk, sizeof(ndd_statblk_t));
		stat_blk.code = NDD_HARD_FAIL;
		stat_blk.option[0] = NDD_PIO_FAIL;
		(*(NDD.nd_status))(&(NDD), &stat_blk);
          }
          break;
        }

        retry_count--;
	
        /* 
         * retry the pio function, return if successful
         */
        switch (iofunc) {
	  
	  case PUTSR:
	    	excpt_code = BUS_PUTSRX((short *)ioaddr, (short)ioparam);
	    	break;

	  case GETSR:
	    	excpt_code = BUS_GETSRX((short *)ioaddr, (short *)ioparam);
	    	break;

	  case PUTLR:
	    	excpt_code = BUS_PUTLRX((long *)ioaddr, (long)ioparam);
	    	break;

	  case GETLR:
	    	excpt_code = BUS_GETLRX((long *)ioaddr, (long *)ioparam);
	    	break;

	  case PUTC:
	    	excpt_code = BUS_PUTCX((char *)ioaddr, (char)ioparam);
	    	break;

	  case PUTS:
	    	excpt_code = BUS_PUTSX((short *)ioaddr, (short)ioparam);
	    	break;

	  case PUTL:
	    	excpt_code = BUS_PUTLX((long *)ioaddr, (long)ioparam);
	    	break;

	  case GETC:
	    	excpt_code = BUS_GETCX((char *)ioaddr, (char *)ioparam);
	    	break;

	  case GETS:
	    	excpt_code = BUS_GETSX((short *)ioaddr, (short *)ioparam);
	    	break;

	  case GETL:
	    	excpt_code = BUS_GETLX((long *)ioaddr, (long *)ioparam);
	    	break;

	  case PUTSTR:
	    	excpt_code = BUS_PUTSTRX((long *)ioaddr, (long *)ioparam, cnt);
	    	break;

	  case GETSTR:
	    	excpt_code = BUS_GETSTRX((long *)ioaddr, (long *)ioparam, cnt);
	    	break;

	  default:
	    	ASSERT(0);
        } 
	
        if (excpt_code == 0) 
          break;
	
    } 
    
    TRACE_SYS(HKWD_EN3COM_OTHER, "NioE", (ulong)excpt_code, 0, 0);
    return (excpt_code);
    
} 

/*****************************************************************************/
/*
 * NAME:     en3com_gen_crc
 *
 * FUNCTION: generate a 16-bit crc value (used to check VPD)
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      en3com_getvpd
 *
 * INPUT:
 *      buf             - starting address
 *      len             - length
 *
 * RETURNS:  
 *	a 16-bit crc value
 */
/*****************************************************************************/
ushort 
en3com_gen_crc (
   	uchar *buf,  	/* starting address of the area */
   	int   len)   	/* number of bytes is data area */

{
  uchar work_msb;
  uchar work_lsb;
  uchar value_msb;
  uchar value_lsb;
  ushort tempshort;


  TRACE_SYS(HKWD_EN3COM_OTHER, "crcB", (ulong)buf, len, 0);

  /* step through the caller's buffer */
  for (value_msb = 0xFF, value_lsb = 0xFF; len > 0; len--) {
      value_lsb ^= *buf++;
      value_lsb ^= (value_lsb << 4);

      work_msb = value_lsb >> 1;
      work_lsb = (value_lsb << 7) ^ value_lsb;

      work_lsb = (work_msb << 4) | (work_lsb >> 4);
      work_msb = ((work_msb >> 4) & 0x07) ^ value_lsb;

      value_lsb = work_lsb ^ value_msb;
      value_msb = work_msb;

  } /* end loop to step through the caller's buffer */

  tempshort = ((ushort)value_msb << 8) | value_lsb;
  TRACE_SYS(HKWD_EN3COM_OTHER, "crcE", (ulong)tempshort, 0, 0);
  return(tempshort);

} 

/*****************************************************************************/
/*
 * NAME:     en3com_multi_add
 *
 * FUNCTION: Add a multicast address to the multicast table.
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      en3com_ctl
 *
 * INPUT:
 *	p_dev_ctl	- point to the dev_ctl area
 *      addr            - point to the multicast address to be added
 *
 * RETURNS:  
 *	0 - successful
 *	ENOMEM - unable to allocate required memory
 */
/*****************************************************************************/
en3com_multi_add(
  en3com_dev_ctl_t 	*p_dev_ctl,	/* point to the dev_ctl area */
  char 	*addr)				/* point to the multicast address */

{

  en3com_multi_t *p_multi;
  en3com_multi_t *p_new;
  int i;


  TRACE_SYS(HKWD_EN3COM_OTHER, "ImaB", (ulong)p_dev_ctl, (ulong)addr, 0);

  p_multi = &WRK.multi_table;
  /*
   * verify that the multicast address is a duplicate or not.
   * For a duplicate address, simply increment the ref_count and return
   */
  while (p_multi) {
	for (i=0; i < p_multi->in_use; i++) {
		if (SAME_NADR(addr, p_multi->m_slot[i].m_addr)) {
			p_multi->m_slot[i].ref_count++;
  			TRACE_SYS(HKWD_EN3COM_OTHER, "Ima1", 0, 0, 0);
			return(0);
		}
	}
	p_multi = p_multi->next;
  }

  /* get to the first table that has room for new entry */
  
  p_multi = &WRK.multi_table;
  while (p_multi->in_use >= MULTI_TABLE_SLOT) {
	p_multi = p_multi->next;
  }
  /*
   * if there is no room in existing tables,
   * Allocate memory for additional multicast table.
   */
  if (!p_multi) {
	p_multi = xmalloc(sizeof(en3com_multi_t), MEM_ALIGN, pinned_heap);
	if (!p_multi) {
  		TRACE_SYS(HKWD_EN3COM_ERR, "Ima2", ENOMEM, 0, 0);
		return(ENOMEM);
	}
	bzero(p_multi, sizeof(en3com_multi_t));

	/* link the new table to the beginning of the chain */
	p_multi->next = WRK.multi_table.next;
	WRK.multi_table.next = p_multi;
	p_multi->in_use = 0;
  }

  /* add the address into the table and increment counters */
  COPY_NADR(addr, p_multi->m_slot[p_multi->in_use].m_addr);
  p_multi->m_slot[p_multi->in_use].ref_count = 1;
  p_multi->in_use++;
  WRK.multi_count++;

  TRACE_SYS(HKWD_EN3COM_OTHER, "ImaE", 0, 0, 0);
  return(0);

}

/*****************************************************************************/
/*
 * NAME:     en3com_multi_del
 *
 * FUNCTION: Delete a multicast address from the multicast table.
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      en3com_ctl
 *
 * INPUT:
 *	p_dev_ctl	- point to the dev_ctl area
 *      addr            - point to the multicast address to be added
 *
 * RETURNS:  
 *	0 - successful
 *	EINVAL - invalid parameter
 */
/*****************************************************************************/
en3com_multi_del(
  en3com_dev_ctl_t 	*p_dev_ctl,	/* point to the dev_ctl area */
  char 	*addr)				/* point to the multicast address */

{

  en3com_multi_t *p_multi = &WRK.multi_table;
  en3com_multi_t *p_prev = NULL;
  int i;


  TRACE_SYS(HKWD_EN3COM_OTHER, "ImdB", (ulong)p_dev_ctl, (ulong)addr, 0);

  /* search for the address */
  while (p_multi) {
	for (i = 0; i < p_multi->in_use; i++) {
		if (SAME_NADR(addr, p_multi->m_slot[i].m_addr))  {
			break;
		}
	}
	if (i < p_multi->in_use) 
		break;			/* found */
	p_prev = p_multi;
	p_multi = p_multi->next;
  }

  /* if found the address */
  if (p_multi) {		
	/*
	 * If the ref_count is greater than 1, simply decrement it.
	 * Otherwise, remove the address from the table by consolidate 
	 * the rest of the table 
	 */
	if (!(--p_multi->m_slot[i].ref_count)) {
	  for (; i < p_multi->in_use - 1; i++) {
		COPY_NADR(p_multi->m_slot[i+1].m_addr, 
			p_multi->m_slot[i].m_addr);
		p_multi->m_slot[i].ref_count = p_multi->m_slot[i+1].ref_count;
		
	  }
	  bzero(&p_multi->m_slot[i], sizeof(multi_slot_t));

	  /* update counters */
	  p_multi->in_use--;
	  WRK.multi_count--;
	
	  /* if the table is empty and it is an expansion, free the space */
	  if (!p_multi->in_use) {
		if (p_prev) {
			p_prev->next = p_multi->next;
			xmfree(p_multi, pinned_heap);
		}
	  }
	}
  	TRACE_SYS(HKWD_EN3COM_OTHER, "ImdE", 0, 0, 0);
  	return(0);
  }
  else {
  	TRACE_SYS(HKWD_EN3COM_ERR, "Imd1", EINVAL, 0, 0);
	return(EINVAL);
  }


}

/*****************************************************************************/
/*
 * NAME:     en3com_cdt_func
 *
 * FUNCTION: return the address of the component dump table 
 *
 * EXECUTION ENVIRONMENT: 
 *
 * NOTES:
 *
 * CALLED FROM:
 *
 * INPUT:
 *	none.
 *
 * RETURNS:  
 *	the address of the component dump table
 */
/*****************************************************************************/
struct cdt *en3com_cdt_func()
{

   return((struct cdt *)(&en3com_dd_ctl.cdt.head));

}

/*****************************************************************************/
/*
 * NAME:     en3com_cdt_add
 *
 * FUNCTION: add an entry to the component dump table
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:
 *	en3com_open
 *
 * INPUT:
 *	name		- character string of the name of the data structure
 *	addr		- address of the data structure
 *	len		- length of the data structure
 *
 * RETURNS:  
 *	none.
 */
/*****************************************************************************/
void
en3com_cdt_add(
  char *name,  	/* label string for area dumped */
  char *addr,   	/* address of the area to be dumped */
  int   len)   	/* amount of data to be dumped */

{
  struct cdt_entry *p_entry;


  TRACE_SYS(HKWD_EN3COM_OTHER, "OdaB", (ulong)name, (ulong)addr, len);
   
  p_entry = &en3com_dd_ctl.cdt.entry[en3com_dd_ctl.cdt.count];
  strcpy(p_entry->d_name, name);
  p_entry->d_len = len;
  p_entry->d_ptr = addr;
  p_entry->d_xmemdp = NULL;

  en3com_dd_ctl.cdt.count++;
  en3com_dd_ctl.cdt.head._cdt_len += sizeof(struct cdt_entry);

  TRACE_SYS(HKWD_EN3COM_OTHER, "OdaE", 0, 0, 0);

}

/*****************************************************************************/
/*
 * NAME:     en3com_cdt_del
 *
 * FUNCTION: delete an entry to the component dump table
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:
 *	en3com_open
 *	en3com_close
 *
 * INPUT:
 *	name		- character string of the name of the data structure
 *	addr		- address of the data structure
 *	len		- length of the data structure
 *
 * RETURNS:  
 *	none.
 */
/*****************************************************************************/
void
en3com_cdt_del(
  char *name,  	/* label string for area dumped */
  char *addr,   	/* address of the area to be dumped */
  int   len)   	/* amount of data to be dumped */

{
  struct cdt_entry *p_entry;
  int i;


   
  TRACE_SYS(HKWD_EN3COM_OTHER, "OddB", (ulong)name, (ulong)addr, len);

  /* find the entry in the table (match only the memory pointer) */
  for (p_entry = &en3com_dd_ctl.cdt.entry[0], i = 0;
        i < en3com_dd_ctl.cdt.count; p_entry++, i++) {
	if (p_entry->d_ptr == addr) 
		break;
  }

  /* if found the entry, remove the entry by re-arrange the table */
  if (i < en3com_dd_ctl.cdt.count) {
      	for (; i < en3com_dd_ctl.cdt.count; p_entry++, i++) {
		strcpy(p_entry->d_name, en3com_dd_ctl.cdt.entry[i+1].d_name);
		p_entry->d_len = en3com_dd_ctl.cdt.entry[i+1].d_len;
		p_entry->d_ptr = en3com_dd_ctl.cdt.entry[i+1].d_ptr;
	}
	en3com_dd_ctl.cdt.count--;
	en3com_dd_ctl.cdt.head._cdt_len -= sizeof(struct cdt_entry);

  }
  TRACE_SYS(HKWD_EN3COM_OTHER, "OddE", 0, 0, 0);

}

/*****************************************************************************/
/*
 * NAME:     en3com_trace
 *
 * FUNCTION: Put a trace into the internal trace table and the external
 *	     system trace.
 *
 * EXECUTION ENVIRONMENT: process or interrupt
 *
 * NOTES:
 *	This routine is only called through macros when DEBUG is defined.
 *	
 * CALLED FROM:
 *	every routine in the driver 
 *
 * INPUT:
 *	hook		- trace hook 
 *	tag		- four letter trace ID
 *	arg1 to arg 3   - arguments to trace
 *
 * RETURNS:  
 *	none.
 */
/*****************************************************************************/
void
en3com_trace(
   ulong	hook,	/* trace hook */
   char		*tag,	/* trace ID */
   ulong	arg1,	/* 1st argument */
   ulong	arg2,	/* 2nd argument */
   ulong	arg3)	/* 3rd argument */

{	 

	int i;
	int ipri;

	ipri = disable_lock(PL_IMP, &TRACE_LOCK);

	/*
	 * Copy the trace point into the internal trace table
	 */
	i = en3com_dd_ctl.trace.next_entry;
	en3com_dd_ctl.trace.table[i] = *(ulong *)tag;
	en3com_dd_ctl.trace.table[i+1] = arg1;
	en3com_dd_ctl.trace.table[i+2] = arg2;
	en3com_dd_ctl.trace.table[i+3] = arg3;
	
	if ((i += 4) < EN3COM_TRACE_SIZE) {
		en3com_dd_ctl.trace.table[i] = EN3COM_TRACE_END;
		en3com_dd_ctl.trace.next_entry = i;
	}
	else {
		en3com_dd_ctl.trace.table[0] = EN3COM_TRACE_END;
		en3com_dd_ctl.trace.next_entry = 0;
	}
 
	unlock_enable(ipri, &TRACE_LOCK);

	/*
	 * Call the external trace routine
	 */
	TRCHKGT((hook << 20) | HKTY_GT | 4, *(ulong *)tag, arg1, arg2, arg3, 0);
	

}

/*****************************************************************************/
/*
 * NAME:     en3com_logerr
 *
 * FUNCTION: Collect information for making system error log entry.
 *
 * EXECUTION ENVIRONMENT: process or interrupt 
 *
 * NOTES:
 *	
 * CALLED FROM:
 *	en3com_setup
 *	en3com_tx_setup
 *	en3com_rv_setup
 *	en3com_tx_free
 *	en3com_rv_free
 *	en3com_init
 *	en3com_start
 *	en3com_getcfg
 *	en3com_intr
 *	en3com_rv_intr
 *	en3com_tx_intr
 *	en3com_tx_timeout
 *	en3com_ctl_timeout
 *	en3com_hard_fail
 *
 * INPUT:
 *	p_dev_ctl	- pointer to device control area
 *	errid		- error id for logging 
 *	line		- code line number
 *	fname		- code file name
 *	parm1		- log 4 bytes data 1
 *	parm2		- log 4 bytes data 2
 *	parm3		- log 4 bytes data 3
 *
 * RETURNS:  
 *	none.
 */
/*****************************************************************************/
void
en3com_logerr(
  en3com_dev_ctl_t	*p_dev_ctl,	/* pointer to dev_ctl area */
  ulong  errid,              /* Error id for logging */
  int    line,		     /* line number */
  char   *p_fname,	     /* file name */
  ulong  parm1,              /* log 4 bytes data 1 */
  ulong  parm2,              /* log 4 bytes data 2 */
  ulong  parm3)              /* log 4 bytes data 3 */

{
	struct  error_log_def   log;
	uchar   lbuf[64];
	int i;


  	TRACE_SYS(HKWD_EN3COM_OTHER, "NlgB", (ulong)p_dev_ctl, (ulong)errid, 
		line);
  	TRACE_SYS(HKWD_EN3COM_OTHER, "NlgC", parm1, parm2, parm3);

   	/* Load the error id and device driver name into the log entry */
   	log.errhead.error_id = errid;
        strncpy(log.errhead.resource_name, DDS.lname, ERR_NAMESIZE);

        /* put the line number and filename in the table */
        sprintf(lbuf, "line: %d file: %s", line, p_fname);
        strncpy(log.fname, lbuf, sizeof(log.fname));

   	/* Load POS register value in the table   */
   	for (i = 0; i < NUM_POS_REG; i++)
      		log.pos_reg[i] = WRK.pos_reg[i];

   	/* Load Network address in use value into the table  */
   	for (i = 0; i < ENT_NADR_LENGTH; i++) 
      		log.ent_addr[i] = WRK.net_addr[i];

   	/* Start filling in the table with data  */
   	log.parm1 = parm1;
   	log.parm2 = parm2;
   	log.parm3 = parm3;

   	/* log the error message */
   	errsave(&log, sizeof(struct error_log_def));

  	TRACE_SYS(HKWD_EN3COM_OTHER, "NlgE", 0, 0, 0);

}  

/*****************************************************************************/
/*
 * NAME:     en3com_loopdelay
 *
 * FUNCTION: Simulate delay at interrupt level by using the IOCC delay
 *	     register. 
 *
 * EXECUTION ENVIRONMENT: interrupt only
 *
 * NOTES:
 *	This routine is called only when the command register is not ready
 *	before a command needs to be issued.
 *	
 * CALLED FROM:
 *	en3com_cmd
 *	en3com_rv_start
 *
 * INPUT:
 *	p_dev_ctl	- pointer to device control area
 *	ticks		- number of timer ticks to delay
 *
 * RETURNS:  
 *	none.
 */
/*****************************************************************************/
void
en3com_loopdelay(
  en3com_dev_ctl_t	*p_dev_ctl,	/* pointer to dev_ctl area */
  int	ticks)			/* number of timer ticks to delay */

{

  int delay_seg;
  int i, j;
  uchar tmp_char;


  	TRACE_SYS(HKWD_EN3COM_OTHER, "NldB", (ulong)p_dev_ctl, (ulong)ticks, 0);

	delay_seg = (uint)IOCC_ATT(DDS.bus_id, IOCC_DELAY_REG); 
	for (i = 0; i < ticks; i++) {
		/*
	 	 * delay 10000 usec = 1 tick
	 	 */
		j = 0;
		while ( ++j < 10000) 
			/* delay 1 microsecond */
			tmp_char = BUSIO_GETC(delay_seg); 
	}
	IOCC_DET(delay_seg);

  	TRACE_SYS(HKWD_EN3COM_OTHER, "NldE", 0, 0, 0);
}

