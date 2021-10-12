static char sccsid[] = "@(#)78  1.6  src/bos/kernext/mps_tok/mps_util.c, sysxmps, bos411, 9432B411a 8/10/94 07:30:41";
/*
 *   COMPONENT_NAME: sysxmps
 *
 *   FUNCTIONS: cdt_add
 *		cdt_del
 *		cdt_func
 *		hexdump
 *		mps_logerr
 *		mps_loopdelay
 *		mps_trace
 *		multi_add
 *		multi_del
 *		pio_retry
 *		re_multi_add
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/lock_def.h>
#include <sys/errno.h>
#include <sys/uio.h>
#include <sys/device.h>
#include <sys/mbuf.h>
#include <sys/malloc.h>
#include <sys/adspace.h>
#include <sys/ioacc.h>
#include <sys/dump.h>
#include <sys/watchdog.h>
#include <sys/intr.h>
#include <sys/err_rec.h>
#include <sys/trcmacros.h>
#include <sys/cdli.h>
#include <sys/ndd.h>

#include <sys/cdli_tokuser.h>
#include <sys/generic_mibs.h>
#include <sys/tokenring_mibs.h>

#include "mps_dslo.h"
#include "mps_mac.h"
#include "mps_dds.h"
#include "mps_dd.h"
#include "tr_mps_errids.h"
#ifdef KTD_DEBUG
#include "intercept_functions.h"
#endif

extern mps_dd_ctl_t  mps_dd_ctl;


/*****************************************************************************/
/*
 * NAME:     pio_retry
 *
 * FUNCTION: This routine is called when a pio routine returns an exception.
 *  	Retry the pio function and do error logging.
 *
 * EXECUTION ENVIRONMENT: process and interrupt.
 *
 * NOTES:
 *
 * CALLED FROM:
 *      
 * INPUT:
 *      p_dev_ctl       - device control structure.
 *      excpt_code      - exception code from first pio 
 *      iofunc          - io function to retry
 *      ioaddr          - io address of the exception
 *      ioparam         - parameter to PIO routine
 *      cnt             - for string copies
 *
 * RETURNS:  0 - retry successfully
 *        exception code
 */
/*****************************************************************************/
int pio_retry (
mps_dev_ctl_t  *p_dev_ctl,	/* adapter control structure        */
int   		excpt_code,	/* exception code from original PIO */
enum pio_func   iofunc,		/* io function to retry	            */
int   		ioaddr,		/* io address of the exception	    */
long  		ioparam,	/* parameter to PIO routine	    */
int  		cnt)		/* for string copies                */

{
  ndd_t         *p_ndd = &(NDD);
  ushort        param1;
  ushort        param2;
  int	        retry_count = PIO_RETRY_COUNT;
  ndd_statblk_t stat_blk;      /* status block               */
  xmit_elem_t   *xelm;         /* transmit element structure */
  int	        ipri;

  TRACE_SYS(MPS_OTHER, "NioB", p_dev_ctl, excpt_code, (ulong)iofunc);
      TRACE_SYS(MPS_OTHER, "NioC", (ulong)ioaddr, ioparam, 0);

  while (TRUE) {

  	/* 
         * Checks if out of retries
         */
  	if (retry_count <= 0) {
  		break;
  	}
  	retry_count--;

  	/* 
         * Retries the pio function, return if successful
         */
  	switch (iofunc)
  	{
  	case PUTCX:
  		excpt_code = BUS_PUTCX((char *)ioaddr, (char)ioparam);
  		break;
  	case PUTSX:
  		excpt_code = BUS_PUTSX((short *)ioaddr, (short)ioparam);
  		break;
  	case PUTSRX:
  		excpt_code = BUS_PUTSRX((short *)ioaddr,(short)ioparam);
  		break;
  	case PUTLX:
  		excpt_code = BUS_PUTLX((long *)ioaddr,(long)(ioparam));
  		break;
  	case PUTLRX:
  		excpt_code=BUS_PUTLRX((long *)ioaddr,(long)(ioparam));
  		break;
  	case GETCX:
  		excpt_code = BUS_GETCX((char *)ioaddr, (char *)ioparam);
  		break;
  	case GETSX:
  		excpt_code=BUS_GETSX((short *)ioaddr,(short *)ioparam);
  		break;
  	case GETSRX:
  		excpt_code=BUS_GETSRX((short *)ioaddr,(short *)ioparam);
  		break;
  	case GETLX:
  		excpt_code=BUS_GETLX((long *)ioaddr,(long *)ioparam);
  		break;
  	case GETLRX:
  		excpt_code=BUS_GETLRX((long *)ioaddr,(long *)ioparam);
  		break;
  	case PUTSTRX:
  		excpt_code = BUS_PUTSTRX((long *)ioaddr, 
  						(long *)ioparam, cnt);
  		break;
  	case GETSTRX:
  		excpt_code = BUS_GETSTRX((long *)ioaddr, 
  						(long *)ioparam, cnt);
  		break;
  	default:
  		ASSERT(0);
  	} /* End of switch (iofunc) */

  	if (excpt_code == 0) {
  		break;
  	}

  } /* End of while (TRUE) */

  TRACE_SYS(MPS_OTHER, "NioE", p_dev_ctl, (ulong)excpt_code, 0);
  return (excpt_code);

}


/*****************************************************************************/
/*
 * NAME:     re_multi_add
 *
 * FUNCTION: Add multicast addressess to the multicast table.
 *
 * EXECUTION ENVIRONMENT: interrupt only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      mps_ctl
 *
 * INPUT:
 *      p_dev_ctl       - point to the dev_ctl area
 *      p_ndd           - pointer to the ndd in the dev_ctl area 
 *
 * RETURNS:
 *      0 - successful
 *      ENOMEM - unable to allocate required memory
 *      other  - error 
 */
/*****************************************************************************/
int re_multi_add(
  mps_dev_ctl_t   *p_dev_ctl,      /* point to the dev_ctl area              */
  ndd_t           *p_ndd)          /* pointer to the ndd in the dev_ctl area */


{

  mps_multi_t *p_multi = &WRK.multi_table;
  int  	i;
  int  	count = 0;

  TRACE_SYS(MPS_OTHER, "PraB", (ulong)p_dev_ctl, (ulong)p_ndd, 0);

  /*
   * Check if any group address net to set
   */
  while (p_multi) {
        for (i=0; i < p_multi->in_use; i++) {
    		set_group_address (p_dev_ctl, p_multi->m_slot[i].m_addr, TRUE);
  		count++;
    		if (count >= MAX_MULTI) {
  			break;
		}
        }

    	if (count >= MAX_MULTI) {
  		break;
	}
        p_multi = p_multi->next;
  }


  /*
   * Checks if we need to turn on the promiscuous mode 
   */
  if (WRK.multi_count > MAX_MULTI) {
         WRK.promiscuous_count++;	/* inc the reference counter */
         if (WRK.promiscuous_count == 1) {
       		p_ndd->ndd_flags |= NDD_PROMISC;
                MIB.Generic_mib.ifExtnsEntry.promiscuous = TRUE;
                if (modify_receive_options (p_dev_ctl, PROMIS_ON, TRUE)) {
                 	WRK.promiscuous_count--;
		}		 

         }
  }

  TRACE_SYS(MPS_OTHER, "PraE", p_dev_ctl, 0, 0);
  return(0);

}

/*****************************************************************************/
/*
 * NAME:     multi_add
 *
 * FUNCTION: Add a multicast address to the multicast table.
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      mps_ctl
 *
 * INPUT:
 *      p_dev_ctl       - point to the dev_ctl area
 *      p_ndd           - pointer to the ndd in the dev_ctl area 
 *      addr            - point to the multicast address to be added
 *
 * RETURNS:
 *      0 - successful
 *      ENOMEM - unable to allocate required memory
 *      EINVAL - invalid parameter
 *
 * Note :
 *   	The new group address will always be added to the end of the table.  
 *      The Wildwood adapter only support 256 group addresses.  When there 
 *	are less than 256 group addresses in the table, update the adapter's 
 *	group register as required. When there are more than 256 group 
 *	addresses in the table, the driver will enable the promiscuous mode 
 *      on the adapter.
 * 	The driver will keep the group address in its table and expand the
 * 	table when more space is needed.
 *
 */
/*****************************************************************************/
int multi_add(
  mps_dev_ctl_t   *p_dev_ctl,      /* point to the dev_ctl area              */
  ndd_t           *p_ndd,          /* pointer to the ndd in the dev_ctl area */
  char  	  *addr)           /* point to the multicast address         */

{

  mps_multi_t *p_multi = &WRK.multi_table;
  int i;
  int rc;

  TRACE_SYS(MPS_OTHER, "ImaB", (ulong)p_dev_ctl, (ulong)addr, (ulong)p_multi);

  /*
   * Determines if the multicast address is a duplicate or not.
   * For a duplicate address, simply increment the ref_count and return
   */
  while (p_multi) {
        for (i=0; i < p_multi->in_use; i++) {
                if (SAME_NADR(addr, p_multi->m_slot[i].m_addr)) {
                        p_multi->m_slot[i].ref_count++;
                        TRACE_SYS(MPS_OTHER, "Ima1", p_dev_ctl, 0, 0);
                        return(0);
                }
        }
        p_multi = p_multi->next;
  }

  /* 
   * Gets to the table that has room for new entry 
   */
  if (WRK.multi_last->in_use >= MULTI_ENTRY) {
  	/*
  	 * No room in existing tables, allocate
  	 * memory for additional multicast table.
  	 */
  	if (!p_multi) {
        	if (!WRK.new_multi) {
                	WRK.new_multi = (mps_multi_t *)
                     	xmalloc(sizeof(mps_multi_t),MEM_ALIGN,pinned_heap);
        		if (!WRK.new_multi) {
                		TRACE_SYS(MPS_OTHER,"Ima2",p_dev_ctl,ENOMEM,0);
                		return(ENOMEM);
        		}
        	}
		/*
        	 * Links the new table to the end of the chain 
		 */
  		WRK.multi_last->next = p_multi = WRK.new_multi;
        	p_multi->next = NULL;
        	p_multi->in_use = 0;
  		WRK.new_multi = NULL;
  	}
  	WRK.multi_last = p_multi;

  } else {
  	p_multi = WRK.multi_last;
  }

  /* 
   * Adds the address into the table and increment counters 
   */
  COPY_NADR(addr, p_multi->m_slot[p_multi->in_use].m_addr);

  /* 
   * Updates counters
   */
  p_multi->m_slot[p_multi->in_use].ref_count = 1;
  p_multi->in_use++;
  WRK.multi_count++;

 /* 
  * Checks if any room to add the group address to hardware register.
  * the hardware group register is limit to 256 
  */
 if (WRK.multi_count <= MAX_MULTI) { 
	if (p_dev_ctl->device_state == OPENED) {
 		if (rc = set_group_address (p_dev_ctl, addr, FALSE)) {
			TRACE_BOTH(MPS_OTHER, "Ima4", p_dev_ctl, addr, rc);
  			p_multi->m_slot[p_multi->in_use].ref_count = 0;
  			p_multi->in_use--;
  			WRK.multi_count--;
  			return (EINVAL);
		}
	}
  } else {
 	if (WRK.multi_count == (MAX_MULTI + 1)) { 
  		/* 
 	 	 * we need to trun on the promiscuous mode 
 		 */
       		WRK.promiscuous_count++; /* inc the reference counter */
       		if (WRK.promiscuous_count == 1) {
			p_ndd->ndd_flags |= NDD_PROMISC;
               		MIB.Generic_mib.ifExtnsEntry.promiscuous = TRUE;
			if (p_dev_ctl->device_state == OPENED) {
               			modify_receive_options(p_dev_ctl, PROMIS_ON,
								FALSE);
			}
		}

       	}
  	TRACE_SYS(MPS_OTHER, "ImaE", p_dev_ctl, 0, 0);
  	return(0);
  }
}

/*****************************************************************************/
/*
 * NAME:     multi_del
 *
 * FUNCTION: Delete a multicast address from the multicast table.
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      mps_ctl
 *
 * INPUT:
 *      p_dev_ctl       - point to the dev_ctl area
 *      p_ndd           - pointer to the ndd in the dev_ctl area 
 *      addr            - point to the multicast address to be added
 *
 * RETURNS:
 *      0 - successful
 *      EINVAL - invalid parameter
 *
 * Note :
 *	 The Multicast address table will not leave any hole in the table.  
 *	 After each delete, the table will repack by moving the last entry in 
 *	 the table to the entry that just have been removed.  The promiscuous 
 *	 mode will be turned off if number of entry in the table is not exceed 
 *	 the hardware group registers limit(256).
 *	 The driver will maintain its multicast table and shrink the table when 
 *	 spaces freed up.
 */
/*****************************************************************************/
int multi_del(
  mps_dev_ctl_t   *p_dev_ctl,      /* point to the dev_ctl area              */
  ndd_t           *p_ndd,          /* pointer to the ndd in the dev_ctl area */
  char  *addr)                     /* point to the multicast address         */

{

  mps_multi_t *p_multi = &WRK.multi_table;
  mps_multi_t *l_table;
  mps_multi_t *p_prev  = NULL;
  char *p;
  int i;
  int count = 0;
  int FOUND = FALSE;


  TRACE_SYS(MPS_OTHER, "ImdB", (ulong)p_dev_ctl, (ulong)addr, 0);

  /* 
   * Searchs for the address 
   */
  while (p_multi) {
        for (i = 0; i < p_multi->in_use; i++) {
  		count++;
                if (SAME_NADR(addr, p_multi->m_slot[i].m_addr))  {
  			FOUND = TRUE;
                        break;
                }
        }
  	if (FOUND) { 
  		break;
        }
        p_multi = p_multi->next;
  }

  /* 
   * If can not find the address
   */
  if (!p_multi) {
        TRACE_SYS(MPS_OTHER, "Imd1", p_dev_ctl, EINVAL, 0);
        return(EINVAL);
  }

  /*
   * If the ref_count is greater than 1, simply decrement it.
   * Otherwise, remove the address from the table 
   */
  p_multi->m_slot[i].ref_count--;
  if (!(p_multi->m_slot[i].ref_count)) {

  	/* 
   	 * we need to delete the group address from the hardware reg
   	 */
  	if ((count < MAX_MULTI) && (p_dev_ctl->device_state == OPENED)) { 
   		if (unset_group_address (p_dev_ctl, addr)) {
  			TRACE_SYS(MPS_OTHER, "Imd1", p_dev_ctl, addr, count);
		}
	}

  	l_table = WRK.multi_last;
  	/* Updates counters */
  	WRK.multi_count--;
  	l_table->in_use--;

  	/* Updates the multi address table */
  	if (WRK.multi_count) {
		COPY_NADR(l_table->m_slot[l_table->in_use].m_addr,
       				p_multi->m_slot[i].m_addr);
  		p_multi->m_slot[i].ref_count = 
				l_table->m_slot[l_table->in_use].ref_count;
	}

  	/* 
   	 * Puts the last added group address in the multi table but not 
   	 * in the hardware register to the hardware register.
   	 */
  	if ((count < MAX_MULTI) && (WRK.multi_count >= MAX_MULTI) &&
	     (p_dev_ctl->device_state == OPENED)) {
   		if (set_group_address (p_dev_ctl,  
         	     		  l_table->m_slot[l_table->in_use].m_addr, 
				  FALSE)) {
		}
  	}

 	/*
  	 * If the table is empty and it is an expansion, free the space
  	 */
  	if (!l_table->in_use) {
		/*
  	 	* Searchs for the previous table 
	 	*/
  		p_multi = &WRK.multi_table;
    		while (p_multi != l_table) {
         		p_prev = p_multi;
         		p_multi = p_multi->next;
    		}

        	if (p_prev) {
    			WRK.multi_last = p_prev;
                	p_prev->next = p_multi->next;
  			WRK.free_multi = p_multi;
        	}
  	}

  	/* 
   	 * Checks if we need to trun off the promiscuous mode 
   	 */
  	if (WRK.multi_count == MAX_MULTI) {
       	   	WRK.promiscuous_count--;/* dev the reference counter */
          	if (!WRK.promiscuous_count) {
                       p_ndd->ndd_flags &= ~NDD_PROMISC;
                       MIB.Generic_mib.ifExtnsEntry.promiscuous = FALSE;
	     		if (p_dev_ctl->device_state == OPENED) {
                       		modify_receive_options (p_dev_ctl, 0, FALSE);
			}
         	}
         }

  }
  TRACE_SYS(MPS_OTHER, "ImdE", p_dev_ctl, 0, 0);
  return(0);

}

/*****************************************************************************/
/*
 * NAME:     cdt_func
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
 *  none.
 *
 * RETURNS:  the address of the component dump table
 */
/*****************************************************************************/
struct cdt *cdt_func()
{

  return((struct cdt *)&mps_dd_ctl.cdt.head);

}
/*****************************************************************************/
/*
 * NAME:     cdt_add
 *
 * FUNCTION: add an entry to the component dump table
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:
 *  	ndd_open
 *
 * INPUT:
 *  name		- character string of the name of the data structure
 *  addr		- address of the data structure
 *  len		- length of the data structure
 *
 * RETURNS:  none.
 *
 */
/*****************************************************************************/
void cdt_add(
char *name,    /* label string for area dumped */
char *addr,    /* address of the area to be dumped */
int   len)     /* amount of data to be dumped */

{
  struct cdt_entry *p_entry;


  TRACE_SYS(MPS_OTHER, "OdaB", (ulong)name, (ulong)addr, len);

  p_entry = &mps_dd_ctl.cdt.entry[mps_dd_ctl.cdt.count];
  strcpy(p_entry->d_name, name);
  p_entry->d_len = len;
  p_entry->d_ptr = addr;
  p_entry->d_xmemdp = NULL;

  mps_dd_ctl.cdt.count++;
  mps_dd_ctl.cdt.head._cdt_len += sizeof(struct cdt_entry);

  TRACE_SYS(MPS_OTHER, "OdaE", 0, 0, 0);

}
/*****************************************************************************/
/*
 * NAME:     cdt_del
 *
 * FUNCTION: delete an entry to the component dump table
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:
 *  	ndd_open
 *  	ndd_close
 *
 * INPUT:
 *  name		- character string of the name of the data structure
 *  addr		- address of the data structure
 *  len		- length of the data structure
 *
 * RETURNS:  none.
 *
 */
/*****************************************************************************/
void cdt_del(
char *name,    /* label string for area dumped */
char *addr,    /* address of the area to be dumped */
int   len)     /* amount of data to be dumped */

{
  struct cdt_entry *p_entry;
  int i;



  TRACE_SYS(MPS_OTHER, "OddB", (ulong)name, (ulong)addr, len);

  /* 
   * Searchs the entry in the table (match only the memory pointer) 
   */
  for (p_entry = &mps_dd_ctl.cdt.entry[0], i = 0;
      i < mps_dd_ctl.cdt.count; p_entry++, i++) {
  	if (p_entry->d_ptr == addr)
  		break;
  }

  /* 
   * If found the entry, remove the entry by re-arrange the table 
   */
  if (i < mps_dd_ctl.cdt.count) {
  	for (; i < mps_dd_ctl.cdt.count; p_entry++, i++) {
  		strcpy(p_entry->d_name, 
  				mps_dd_ctl.cdt.entry[i+1].d_name);
  		p_entry->d_len = mps_dd_ctl.cdt.entry[i+1].d_len;
  		p_entry->d_ptr = mps_dd_ctl.cdt.entry[i+1].d_ptr;
  	}
  	mps_dd_ctl.cdt.count--;
  	mps_dd_ctl.cdt.head._cdt_len -= sizeof(struct cdt_entry);

  }
  TRACE_SYS(MPS_OTHER, "OddE", 0, 0, 0);

}
/*****************************************************************************/
/*
 * NAME:     mps_trace
 *
 * FUNCTION: Put a trace into the internal trace table and the external
 *       system trace.
 *
 * EXECUTION ENVIRONMENT: process or interrupt
 *
 * NOTES:
 *  This routine is only called through macros when the DEBUG is defined.
 *  
 * CALLED FROM:
 *
 * INPUT:
 *  hook		- trace hook 
 *  tag		- four letter trace ID
 *  arg1 to arg 3   - arguments to trace
 *
 * RETURNS:  none.
 *
 */
/*****************************************************************************/
mps_trace(
ulong  hook,	/* trace hook */
char  	*tag,	/* trace ID */
ulong  arg1,	/* 1st argument */
ulong  arg2,	/* 2nd argument */
ulong  arg3)	/* 3rd argument */

{

  int i;
  int ipri;

  ipri = disable_lock(PL_IMP, &TRACE_LOCK);

  /*
   * Copys the trace point into the internal trace table
   */
  i = mps_dd_ctl.trace.next_entry;
  mps_dd_ctl.trace.table[i] = *(ulong *)tag;
  mps_dd_ctl.trace.table[i+1] = arg1;
  mps_dd_ctl.trace.table[i+2] = arg2;
  mps_dd_ctl.trace.table[i+3] = arg3;

  if ((i += 4 ) >= ((MPS_TRACE_SIZE) - 4)) {
        i = 4;
  }
  mps_dd_ctl.trace.next_entry = i;

  strcpy(((char *)(&mps_dd_ctl.trace.table[i])), MPS_TRACE_CUR);

  unlock_enable(ipri, &TRACE_LOCK);

  /*
   * Calls the external trace routine
   */
  TRCHKGT(hook | HKTY_GT | 4, *(ulong *)tag, arg1, arg2, arg3, 0);

}

/*****************************************************************************/
/*
 * NAME:     mps_loopdelay
 *
 * FUNCTION: Simulate delay at interrupt level by using the IOCC delay
 *           register.
 *
 * EXECUTION ENVIRONMENT: interrupt only
 *
 * NOTES:
 *      This routine is called only when we are trying to recover the
 *      adapter in interrupt level code.
 *
 * CALLED FROM:
 *
 * INPUT:
 *      p_dev_ctl       - pointer to device control area
 *      ticks           - number of timer ticks to delay
 *
 * RETURNS:
 *      none.
 */
/*****************************************************************************/
void mps_loopdelay(
  mps_dev_ctl_t *p_dev_ctl,     /* pointer to dev_ctl area */
  int     ticks)          /* number of timer ticks to delay */

{

  #define IOCC_DELAY_REG              0xE0    /* hardware delay register */
  int delay_seg;
  int i, j;
  uchar tmp_char;


        TRACE_SYS(MPS_OTHER, "NldB", (ulong)p_dev_ctl, (ulong)ticks, 0);

        delay_seg = (uint)IOCC_ATT(DDS.bus_id, IOCC_DELAY_REG);
        for (i = 0; i < ticks; i++) {
                /*
                 * Delays 10000 usec = 1 tick
                 */
                j = 0;
                while ( ++j < 10000)
			/*
                         * Delays 1 microsecond 
			 */
                        tmp_char = BUSIO_GETC(delay_seg);
        }
        IOCC_DET(delay_seg);

        TRACE_SYS(MPS_OTHER, "NldE", (ulong)p_dev_ctl, (ulong)ticks, 0);
}


/*****************************************************************************/
/*
 * NAME:     mps_logerr
 *
 * FUNCTION: Collect information for making system error log entry.
 *
 * EXECUTION ENVIRONMENT: process or interrupt
 *
 * NOTES:
 *
 * CALLED FROM:
 *
 * INPUT:
 *      p_dev_ctl       - pointer to device control area
 *      errid           - error id for logging
 *      line            - code line number
 *      fname           - code file name
 *      parm1           - log 4 bytes data 1
 *      parm2           - log 4 bytes data 2
 *      parm3           - log 4 bytes data 3
 *
 * RETURNS:
 *      none.
 */
/*****************************************************************************/
void mps_logerr(
  mps_dev_ctl_t *p_dev_ctl,       /* pointer to dev_ctl area */
  ulong    errid,              	/* Error id for logging */
  int      line,               	/* line number */
  char     *p_fname,           	/* file name */
  ulong    parm1,              	/* log 4 bytes data 1 */
  ulong    parm2,              	/* log 4 bytes data 2 */
  ulong    parm3)              	/* log 4 bytes data 3 */

{
  struct  error_log_def   log;
  uchar   lbuf[80];
  int i;


  TRACE_SYS(MPS_OTHER, "NlgB", (ulong)p_dev_ctl, (ulong)errid, line);
  TRACE_SYS(MPS_OTHER, "NlgC", parm1, parm2, parm3);

  /* 
   * Loads the error id and device driver name into the log entry 
   */
  log.errhead.error_id = errid;
  strncpy(log.errhead.resource_name, DDS.dev_name, ERR_NAMESIZE);

  /* 
   * Puts the line number and filename in the table 
   */
  sprintf(lbuf, "line: %d file: %s", line, p_fname);
  strncpy(log.fname, lbuf, sizeof(log.fname));

  /* 
   * Loads POS register value in the table   
   */
  for (i = 0; i < NUM_POS_REG; i++)
          log.pos_reg[i] = WRK.pos_reg[i];

  /* 
   * Loads Network address in use value into the table  
   */
  for (i = 0; i < CTOK_NADR_LENGTH; i++)
          log.mps_addr[i] = WRK.mps_addr[i];

  /* 
   * Starts fill in the table with data  
   */
  log.parm1 = parm1;
  log.parm2 = parm2;
  log.parm3 = parm3;

  /* 
   * Logs the error message 
   */
  errsave(&log, sizeof(struct error_log_def));

  TRACE_SYS(MPS_OTHER, "NlgE", (ulong)p_dev_ctl, 0, 0);

}


/*****************************************************************************/
/*
 * NAME: hexdump
 *
 * FUNCTION: Display an array of type char in ASCII, and HEX.
 *
 * EXECUTION ENVIRONMENT: process or interrupt
 *
 * NOTES:
 *
 * CALLED FROM:
 *
 * INPUT:
 *      data       - data pointer 
 *      len        - length of data 
 *
 * RETURNS:
 *      none.
 */
/*****************************************************************************/

hexdump(data,len)
char *data;
long len;
{

  int     i,j,k;
  char    str[18];

  printf("hexdump(): length = %x\n",len);
  i=j=k=0;
  while(i<len)
  {
          j=(int) data[i++];
          if(j>=32 && j<=126)
                  str[k++]=(char) j;
          else
                  str[k++]='.';
          printf("%02x ",j);
          if(!(i%8)) {
                  printf("  ");
                  str[k++]=' ';
          }
          if(!(i%16)) {
                  str[k]='\0';
                  printf("     %s\n",str);
                  k=0;
          }
  }
  while(i%16)
  {
          if(!(i%8)) {
                  printf("  ");
	  }
          printf("   ");
          i++;
  }
  str[k]='\0';
  printf("       %s\n\n",str);
  ;
}

