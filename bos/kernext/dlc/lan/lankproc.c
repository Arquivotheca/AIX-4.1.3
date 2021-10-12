static char sccsid[] = "@(#)02	1.63.3.22  src/bos/kernext/dlc/lan/lankproc.c, sysxdlcg, bos41J, 9512A_all 3/20/95 09:04:33";
/*************************************************************************
 * COMPONENT_NAME: (SYSXDLCG) Generic Data Link Control
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 ************************************************************************/

#include <sys/types.h>
#include <sys/user.h>
#include <sys/fp_io.h>
#include <sys/device.h>
#include <sys/mbuf.h>
#include "dlcadd.h"
#include "dlcrq.h"
#include <sys/gdlextcb.h>
/* <<< feature CDLI >>> */
#include <sys/ndd.h>
#include <sys/ndd_var.h>
#include <sys/cdli.h>
/* <<< end feature CDLI >>> */
#include <sys/trchkid.h>
#if defined(TRL) || defined(FDL)
#define TRLORFDDI
#endif

/* defect 122577 */
#include <sys/lock_def.h>
#include <sys/lock_alloc.h>
#include <sys/lockname.h>
/* end defect 122577 */

/* <<< feature CDLI >>> */
#ifdef   TRL
#include <sys/cdli_tokuser.h>
#include <sys/trlextcb.h>
#endif /* TRL */
#ifdef   FDL
#include <sys/cdli_fddiuser.h>
#include <sys/fdlextcb.h>
#endif /* FDDI */
#ifndef  TRLORFDDI
#include <sys/cdli_entuser.h>
#endif /* not TRLORFDDI */
/* <<< end feature CDLI >>> */
#include "lancomcb.h"
#include "lanhasht.h"
#include "lanstlst.h"
#include "lansplst.h"
#include "lanrstat.h"
#include "lanport.h"
#include "lanstlst.h"
#define  DLC_RCV 1
#define  DLC_STATUS 2
#define  DLC_DISC_DONE 3
#define  DLC_OFL 4
#define  ECB_RCV 0x40000000
#define  ECB_TIMER 0x20000000
#define  ECB_CLOSE 0x08000000          /* close ecb post code         */
#define  ECB_LOCAL 0x04000000          /* enter loop back packets     */
#define  ECB_STATUS 0x01000000         /* Device handler status post  */
/* <<< feature CDLI >>> */
#define  MAXSAPS 128                   /* maximum number of SAPS      */
/* <<< end feature CDLI >>> */
void lan_timeout();                    /* routine to set timer post   */
void sort_cache();                     /* sort cache                  */
int search_cache();                    /* search cache                */
struct cache *p1_to_cache;
init_proc(flag,parm,length)
  int      flag;
  void     *parm;
  int      length;

/*** start of specifications ******************************************
 *
 * module name  = init_proc
 *
 * descriptive name = process initialization routine
 *
 * input:   pointer to port control block and length
 *
 * output:
 *
 *** end of specifications ********************************************/

{
  register struct port_dcl *p;
  int      found,index,procno,rtn;
  int      rc;
  struct proc *p1;
  struct sigaction act;
  struct sigaction oact;
  struct   initparms
    {
      struct port_dcl *x;
    } 
  *ip;
  int      cache_mode;
  int      i;
/* defect 165426 */
  struct que_entry rq_data;
  int rqlock;
/* end defect 165426 */

  ip = (struct initparms *)parm;
  p = ip->x;

  static_trace(p,"Init",p);  /* defect 167068 */

  simple_lock(&p->dlc_port.lock);  /* defect 167068 */

  /********************************************************************/
  /* call setpinit to set process id to set the parent process id     */
  /********************************************************************/
/* <<< feature CDLI >>> */
  assert(rc = setpinit() == 0);

/* <<< THREADS >>> */
  /********************************************************************/
  /* call thread_self to get the current thread id of this kproc, and */
  /* save it where the head code can find it for et_post.             */
  /********************************************************************/

  assert((p->dlc_port.kproc_tid = thread_self()) != -1)
/* <<< end THREADS >>> */

  /********************************************************************/
  /* setup LS/X RIPL compare functional/multicast addresses           */
  /********************************************************************/

/* <<< removed  RIPL_FUNC_ADDR_MASK... >>> */

  p->common_cb.ripl_grp_addr[0] = RIPL_GRP_ADDR_MASK_0;
  p->common_cb.ripl_grp_addr[1] = RIPL_GRP_ADDR_MASK_1;
  p->common_cb.ripl_grp_addr[2] = RIPL_GRP_ADDR_MASK_2;
  p->common_cb.ripl_grp_addr[3] = RIPL_GRP_ADDR_MASK_3;
  p->common_cb.ripl_grp_addr[4] = RIPL_GRP_ADDR_MASK_4;
  p->common_cb.ripl_grp_addr[5] = RIPL_GRP_ADDR_MASK_5;

  /********************************************************************/
  /* get buffer to hold cache data                                    */
  /********************************************************************/

  assert((p1_to_cache = (struct cache *)palloc(CACHE_SIZE, 4)) != 0);
/* <<< end feature CDLI >>> */
  /********************************************************************/
  /* save pointer in port cb                                          */
  /********************************************************************/

  p->p2_to_cache = (u_char *)p1_to_cache;

  /********************************************************************/
  /* clear cache                                                      */
  /********************************************************************/
 

  for (i = 0; i < CACHE_SIZE; i++)
    {
      *p->p2_to_cache = 0;
      ++p->p2_to_cache;
    } 

  /********************************************************************/
  /* save pointer in port cb                                          */
  /********************************************************************/

  p->p2_to_cache = (u_char *)p1_to_cache;

  /********************************************************************/
  /* set number of entries in cache buffer to 0                       */
  /********************************************************************/

  p1_to_cache->n_entries = 0;

  /********************************************************************/
  /* setup beginning part of name cache data path name                */
  /********************************************************************/

  bcopy("/etc/", &p->name_cache, 5);

  /********************************************************************/
  /* append port name to path name to get full path name              */
  /********************************************************************/

  bcopy(p->dlc_port.namestr, &p->name_cache[5], sizeof
     (p->dlc_port.namestr));

  /********************************************************************/
  /* open if file exists, create it otherwise                         */
  /********************************************************************/

  rc = fp_open(p->name_cache, (O_RDWR|O_NDELAY|O_CREAT), FPP_CMODE, 
     NULL, SYS_ADSPACE, &p->fpp_cache);
 
  if (rc == DLC_OK)                    /* no error occurs ?           */
    {

      /****************************************************************/
      /* read complete cache data into buffer                         */
      /****************************************************************/

      rc = fp_read(p->fpp_cache, &p1_to_cache->n_entries, CACHE_SIZE, 
         0, SYS_ADSPACE, &p->fpp_cbytes);

      /****************************************************************/
      /* close file                                                   */
      /****************************************************************/

      rc = fp_close(p->fpp_cache);
    } 

/* <<< feature CDLI >>> */
  act.sa_handler = SIG_IGN;
  assert(rc = sigaction(SIGINT, &act, &oact) == 0);

  assert(rc = timeoutcf(10) == 0);
/* <<< end feature CDLI >>> */

  /********************************************************************/
  /* call link_manager to handle post interrupts                      */
  /********************************************************************/

  static_trace(p,"LMGb",p->dlc_port.kproc_tid);  /* defect 167068 */
  rtn = link_manager(p);
  static_trace(p,"LMGe",rtn);  /* defect 167068 */

/* defect 155341 */
  p1_to_cache = (struct cache *)p->p2_to_cache;

  if                       /* data in discovery name cache*/
     (p1_to_cache->n_entries > 0)
    {

      /****************************************************/
      /* open cache file and write cache data open file   */
      /****************************************************/

      rtn = fp_open(p->name_cache, (O_RDWR|O_NDELAY|
         O_CREAT), FPP_CMODE, NULL, SYS_ADSPACE,
         &p->fpp_cache);

      if (rtn == DLC_OK)   /* no error occurs ?           */
        {

          /************************************************/
          /* write test data to file                      */
          /************************************************/

          rtn = fp_write(p->fpp_cache,
             &p1_to_cache->n_entries, CACHE_SIZE, 0,
             SYS_ADSPACE, &p->fpp_cbytes);

          /************************************************/
          /* close file                                   */
          /************************************************/

          rtn = fp_close(p->fpp_cache);
        }
    }

  /********************************************************************/
  /* free cache data buffer                                           */
  /********************************************************************/

  assert(xmfree(p1_to_cache, kernel_heap) == 0);
/* end defect 155341 */

/* <<< THREADS >>> */
  untimeout(lan_timeout, p->dlc_port.kproc_tid);
/* <<< end THREADS >>> */

/* <<< feature CDLI >>> */
  assert(rc = timeoutcf(-10) == 0);
/* <<< end feature CDLI >>> */

/* defect 165426 */
  /* flush the receive ring queue */
  do
    {
    rqlock = disable_lock (PL_IMP, &(p->dlc_port.ringq_lock));
    rtn = dlc_rqget(p->dlc_port.rcv_ringq, &rq_data);
    /* unlock the ring queue */
    unlock_enable (rqlock, &(p->dlc_port.ringq_lock));

    static_trace(p,"RqPg",rtn);  /* defect 167068 */
    if (rtn != 0)            /* error or empty              */
       break;

    if (rq_data.entry[0] == DLC_RCV)
       m_freem ((struct mbuf *)rq_data.entry[1]);
    }

  while (1);
/* end defect 165426 */

  static_trace(p,"RqDP",0);  /* defect 167068 */
  dlc_rqdelete(p->dlc_port.rcv_ringq);
  static_trace(p,"RqDL",0);  /* defect 167068 */
  dlc_rqdelete(p->loopback.lb_ring_addr);

  /* free the ring queue lock - defect 127690 */
  lock_free (&(p->dlc_port.ringq_lock));
  /* end defect 127690 */
 
  simple_unlock(&p->dlc_port.lock);  /* defect 167068 */

  do
    {
      static_trace(p,"Dlay",p->dlc_port.kcid->proc_id);  /* defect 167068 */
      delay(10);
    } 

  while (p->dlc_port.kcid->proc_id == EVENT_NULL);

  static_trace(p,"Wake",p->dlc_port.kcid->proc_id);  /* defect 167068 */
  e_wakeup((int *)&p->dlc_port.kcid->proc_id);

  static_trace(p,"Exit",p->dlc_port.kproc_tid);  /* defect 167068 */
/* kexit is now implied                                               */
/* kexit(0);                                                          */

} 


/**********************************************************************
 *  NAME: Sort_cache
 *
 *  FUNCTION: Performs a sort on the array of indices of the name/address
 *              structure in the name cache.
 *
 *  EXECUTION ENVIRONMENT: Operates in DLC process.
 *
 *  NOTES:
 *
 *  DATA STRUCTURES:
 *       MODIFIED:
 *         cache -- Discovery name cache
 *
 *  RETURNS: None
 *
 **********************************************************************/

void sort_cache(item,left,right,p_to_cache)
  u_char   item[];                     /* array of indices to         
                                          name/address data in cache  */
  int      left,right;                 /* start and end index values  */
  struct dcache p_to_cache[];
{
  int      i,j;                        /* index values                */
  int      x,y;                        /* temporary index values      */

  i = left;
  j = right;                           /* save start and end search   
                                          values                      */

  /********************************************************************/
  /* calculate midpoint between start and end values and use this     */
  /* value as the beginning of sort point get first value into        */
  /* name/address array                                               */
  /********************************************************************/

  x = (int)item[(left+right)/2];

  /********************************************************************/
  /* begin sort                                                       */
  /********************************************************************/
 

  do
    {

      /****************************************************************/
      /* using start index value, get value from array of indexes and */
      /* use to index into discovery cache data area. Compare the name*/
      /* with the name pointed to by the midpoint index until the name*/
      /* is in greater sort order or until the start index equals the */
      /* end index                                                    */
      /****************************************************************/
 

      while ((memcmp(p_to_cache[item[i]].name, p_to_cache[x].name, 20)
         < 0) && (i < right))
        i++;

      /****************************************************************/
      /* compare the name pointed to by the end index to the name at  */
      /* the midpoint index until the name at the end is in greater   */
      /* sort order or the end index equals the midpoint index        */
      /****************************************************************/
 

      while ((memcmp(p_to_cache[item[j]].name, p_to_cache[x].name, 20)
         > 0) && (j > left))
        j--;
 
      if (i <= j)                      /* names found out of sort oder*/
        {

          /************************************************************/
          /* exchange names between left and right slots              */
          /************************************************************/

          y = item[i];
          item[i] = item[j];
          item[j] = y;
          i++;
          j--;
        } 
    } 
 
  while (i <= j);

  /********************************************************************/
  /* call sort recursively to sort left part of names array           */
  /********************************************************************/
 

  if (left < j)
    sort_cache(item, left, j, p_to_cache);

  /********************************************************************/
  /* call sort recursively if right part of names arry to be sorted   */
  /********************************************************************/
 

  if (i < right)
    sort_cache(item, i, right, p_to_cache);
} 


/**********************************************************************
 *  NAME: Search_cache
 *
 *  FUNCTION: Performs a search on the array of indices of the name/address
 *              structure in the name cache.
 *
 *  EXECUTION ENVIRONMENT: Operates in DLC process.
 *
 *  NOTES:
 *
 *  DATA STRUCTURES:
 *       MODIFIED:
 *         None
 *
 *  RETURNS:
 *           -1  -- Name not found
 *               -- name index into cache data area
 *
 **********************************************************************/

search_cache(item,count,key,p_to_cache)
  u_char   item[];                     /* array of indices to         
                                          name/address data in cache  */
  int      count;                      /* number of elements in array */
  u_char   *key;                       /* search string               */
  struct dcache p_to_cache[];
{
  int      low,high,mid;               /* index values                */
 
  if                                   /* number of items less than 1 */

     (count < 1)

    /******************************************************************/
    /* return with search not found indicator                         */
    /******************************************************************/

    return -1;
  low = 0;
  high = count-1;                      /* set boundary of search      */
 
  while (low <= high)                  /* not end of search           */
    {
      mid = (low+high)/2;              /* get midpoint as start of    
                                          search                      */
 
      if                               /* input name preceeds name of 
                                          target                      */
         (strcmp(key, p_to_cache[item[mid]].name) < 0)

        /**************************************************************/
        /* move end search boundary backward                          */
        /**************************************************************/

        high = mid-1;
 
      else
 
        if                             /* input name behind name of   
                                          target                      */
           (strcmp(key, p_to_cache[item[mid]].name) > 0)

          /************************************************************/
          /* move start search point forward                          */
          /************************************************************/

          low = mid+1;
 
        else
          return  mid;                 /* name found                  */
    } 

  return -1;                           /* indicate name not found     */
} 

/* <<< feature CDLI >>> */
 /********************************************************************
 * NAME: lan_rcv
 *
 * FUNCTION: Function Handler called by Device Handler Demuxer to
 *           handle incoming packet of data.
 *
 * EXECUTION ENVIRONMENT: Operates on Device Handler process/level.
 *
 * NOTES: This routine gets access to port control block from the address
 *        originally passed into the add_filter routine as isr_data.
 *        The received mbuf is put in the rinq queue and the kproc is posted
 *        to indicate that data was received from the device handler.
 *
 * DATA STRUCTURES:
 *        MODIFIED
 *               port cb ring queue
 *
 * RETURNS:
 *        ECB_RCV -- packet received posted to kproc
 *
 **********************************************************************/
void lan_rcv(nddptr, mbufptr, macptr, p)
  register struct ndd      *nddptr;    /* ptr to ndd                  */
  register struct mbuf     *mbufptr;   /* ptr to received packet      */
  caddr_t                  macptr;     /* ptr to mac hdr (not used)   */
  register struct port_dcl *p;         /* ptr to port cb              */
{
  struct que_entry rq_data;            /* struct of ring queue entry  */
  ulong    rc;                         /* return code                 */
  int rqlock;                          /* defect 127690 */

  /* if the port cb address passed in is valid, ie. the ndd address
	matches the port cb's ndd address */
  assert(nddptr == p->nddp)
    {
      /* set up the ring queue element */
      rq_data.entry[0] = DLC_RCV;
      rq_data.entry[1] = (ulong)mbufptr;

      /* lock the ring queue - defect 127690 */
      rqlock = disable_lock (PL_IMP, &(p->dlc_port.ringq_lock));
      /* put receive buffer in receive ring queue */
      rc = dlc_rqput(p->dlc_port.rcv_ringq, &rq_data);
      /* unlock the ring queue - defect 127690 */
      unlock_enable (rqlock, &(p->dlc_port.ringq_lock));
 
      if /* the ring queue can't accept the mbuf */
	 (rc != 0)
	{
	  /* if the buffer address is valid, ie. not zero or negative */
	  assert((long)mbufptr > 0)
	    {
	       /* free the mbuf and any chained to it */
	       m_freem(mbufptr);
	    }

	  /* log a queue overflow error */
	  lanofflg(ERRID_LAN8012, LINEN);

	  return ;
	}

      /* post the lan manager kproc */
/* <<< THREADS >>> */
      et_post(ECB_RCV, p->dlc_port.kproc_tid);
/* <<< end THREADS >>> */

    }
} /* end lan_rcv */
/* <<< end feature CDLI >>> */

/*
 **********************************************************************
 *  NAME: lan_tx
 *
 *  FUNCTION: Function Handler called by device Handler to
 *              indicate the write transmit queue has slots
 *              available.
 *
 *  EXECUTION ENVIRONMENT: Operates under device handler interrupt
 *                          process.
 *
 *  NOTES:
 *
 *
 *  RETURNS:  None
 *
 **********************************************************************/

void lan_xmit(open_id)
  ulong    open_id;                    /* port control block address  */
{

/*  return to device handler                                          */

  ;
} 


/*
 **********************************************************************
 *  NAME: lan_timeout
 *
 *  FUNCTION: notify Interrupt handler that a unit of time has expired.
 *
 *  EXECUTION ENVIRONMENT: Operates in DLC Interrupt Handler process
 *
 *  NOTES:
 *
 *  DATA STRUCTURES:
 *                   port_dcl  -- Port control block
 *
 *  RETURNS:
 *           ECB_TIMER -- timer interrupt handler post code
 *
 **********************************************************************/
/* <<< THREADS >>> */
void lan_timeout(tid)
  tid_t      tid;                      /* thread id of the port that
					  needs to service timer     */
{

  /********************************************************************/
  /* set up address to port control block                             */
  /********************************************************************/

  timeout(lan_timeout, tid, HZ/2);

  /********************************************************************/
  /* post lan manager                                                 */
  /********************************************************************/

  et_post(ECB_TIMER, tid);
} 
/* <<< end THREADS >>> */



/* <<< feature CDLI >>> */
 /*********************************************************************
 *  NAME:  lan_status
 *
 * FUNCTION: Function Handler called by Device Handler Demuxer to
 *           handle incoming asynchronous status.
 *
 * EXECUTION ENVIRONMENT: Operates on Device Handler process/level.
 *
 * NOTES: This routine gets access to port control block from the address
 *        originally passed into the add_status routine as isr_data.
 *        The status is loaded directly into the ring queue and the
 *        kproc is posted to indicate that status was received from the
 *        device handler.
 *
 * DATA STRUCTURES:
 *        MODIFIED
 *               port cb ring queue
 *
 * RETURNS:
 *        ECB_STATUS -- status received posted to kproc
 *
 **********************************************************************/
void lan_status(nddptr, sblk_ptr, p)
  register struct ndd         *nddptr;   /* ptr to ndd                */
  register struct ndd_statblk *sblk_ptr; /* ptr to status block       */
  register struct port_dcl    *p;        /* ptr to port cb            */
{
  struct que_entry rq_data;            /* ring queue entry            */
  ulong    rc;                         /* return code                 */
  int rqlock;                          /* defect 127690 */

  /* if the port cb address passed in is valid, ie. the ndd address
	matches the port cb's ndd address */
  assert(nddptr == p->nddp)
    {
      /* set up the ring queue element */
      rq_data.entry[0] = DLC_STATUS;
      rq_data.entry[1] = sblk_ptr->code;
      rq_data.entry[2] = sblk_ptr->option[0];
      rq_data.entry[3] = sblk_ptr->option[1];
      rq_data.entry[4] = sblk_ptr->option[2];
      rq_data.entry[5] = sblk_ptr->option[3];
 
      /* lock the ring queue - defect 127690 */
      rqlock = disable_lock (PL_IMP, &(p->dlc_port.ringq_lock));
      /* put status into the receive ring queue */
      rc = dlc_rqput(p->dlc_port.rcv_ringq, &rq_data);
      /* unlock the ring queue - defect 127690 */
      unlock_enable (rqlock, &(p->dlc_port.ringq_lock));
 
      if /* the ring que cannot accept the status entry */
	 (rc != 0)
	{
	  /* log a queue overflow error */
	  lanofflg(ERRID_LAN8012, LINEN);
	  return ;
	}

      /* post the lan manager kproc */
/* <<< THREADS >>> */
      et_post(ECB_STATUS, p->dlc_port.kproc_tid);
/* <<< end THREADS >>> */

    }
} /* end lan_status */
/* <<< end feature CDLI >>> */


/*
 **********************************************************************
 *  NAME:  lanofflg
 *
 *  FUNCTION:  Logs the errors that occur on off-level
 *
 *  EXECUTION ENVIRONMENT: Operates under Device Handler Off-level
 *                          Interrupt process.
 *
 *  NOTES: This routine cannot alter the port control block like the
 *         lanerrlg routine, since we cannot lock during off-level.
 *
 *  INPUT: 1) Error ID
 *         2) source line number
 *
 *  OUTPUT: error log entry sent to the system log
 *
 *  RETURNS: nothing
 *
 **********************************************************************/

lanofflg(in_error,line)
  ulong    in_error;                   /* error ID encountered        */
  int      line;                       /* source line number          */
{
  register struct port_dcl *p;
  struct err_rec *rptr;
  struct err_rec err_rec;
  char     *errptr;
  char     bld_err_rec[100];
  int      rc;

  /********************************************************************/
  /* initialize the error buffer pointers                             */
  /********************************************************************/

  errptr = &bld_err_rec[0];
  rptr = (struct err_rec *)errptr;

  /********************************************************************/
  /* Setup the Error ID field in the error record                     */
  /********************************************************************/

  rptr->error_id = in_error;

  /********************************************************************/
  /* Load the Resource Name into the error record                     */
  /********************************************************************/

  bcopy(COMP_NAME, rptr->resource_name, sizeof(COMP_NAME));

  /********************************************************************/
  /* Build the generic part of the detailed data area Initialize the  */
  /* Detail Data pointer                                              */
  /********************************************************************/

  errptr = rptr->detail_data;

  /********************************************************************/
  /* (8019) Data Link Type                                            */
  /********************************************************************/

  bcopy(DLC_TYPE, errptr, 16);
  errptr = errptr+16;                  /* fixed length                */

  /********************************************************************/
  /* (8016) Communications Device Name                                */
  /********************************************************************/

  bcopy(p->dlc_port.namestr, errptr, 16);/* fixed length              */
  errptr = errptr+16;

  /********************************************************************/
  /* (00A2) Detecting Module                                          */
  /********************************************************************/

  bcopy("                ", errptr, 16);
  bcopy(FILEN, errptr, strlen(FILEN)); /* current file name           */
  errptr = errptr+16;
  bcopy(&line, errptr, 4);             /* current line number         */
  errptr = errptr+4;
#ifdef   DEBUG
 
  if (p->debug)
    printf("p->errptr=%x\n", errptr);
  dump(&err_rec[0], ((int)errptr-(int)&bld_err_rec[0]));
#endif

  /********************************************************************/
  /* call the system error log routine                                */
  /********************************************************************/

  rc = errsave(&bld_err_rec[0], ((int)errptr-(int)&bld_err_rec[0]));
} 
                                       /* end lanofflg                */

/*
 **********************************************************************
 *  NAME: lan_pclose
 *
 *  FUNCTION: Close all active stations and all active SAPs on a
 *              particular channel.
 *
 *  EXECUTION ENVIRONMENT: DLC Manager process
 *
 *  NOTES:
 *
 *  DATA STRUCTURES:
 *       MODIFIED
 *          sap_state -- SAP State
 *
 *  RETURNS: None
 *
 **********************************************************************/

lan_pclose(p)
  register struct port_dcl *p;         /* pointer to port control     
                                          block                       */
{
  struct dlc_chan *cptr;               /* pointer to channel control  
                                          block                       */
/* <<< feature CDLI >>> */
  ns_8022_t  ns_filter;
  ns_com_status_t  ns_statfilter;
  uint     i;                          /* sap list index variable     */
  uint     j;                          /* station list index variable */
  ulong    rtn;                        /* return code                 */
/* <<< end feature CDLI >>> */
/* defect 142249 */
  char tempaddr[6];
/* end defect 142249 */

  /********************************************************************/
  /* get accsss to channel control block                              */
  /********************************************************************/

  TRACE1(p, "KCLb");
  cptr = p->dlc_port.kcid;

  /********************************************************************/
  /* lock channel control block for exclusive use                     */
  /********************************************************************/

  /* defect 122577 */
  simple_lock(&cptr->lock);
  /* end defect 122577 */



    {

      /****************************************************************/
      /* search all saps for saps on channel to be closed             */
      /****************************************************************/
 

      for                              /* each sap on the channel     */
         (i = 0; i < MAXSAPS; i++)
        {
 
          if                           /* sap is active               */
             (p->sap_list[i].in_use == TRUE)
            {
#ifdef   DEBUG
 
              if (p->debug)
                printf("sap in use\n");
#endif

              /********************************************************/
              /* get sap control block                                */
              /********************************************************/

	      p->sap_ptr = (struct sap_cb *)p->sap_list[i].sap_cb_addr;
              p->sapno = i;

              /********************************************************/
              /* note: user_sap channel should be set after sap opened*/
              /********************************************************/

#ifdef   DEBUG
 
              if (p->debug)
                {
		  printf("channel=%x\n", p->sap_ptr->user_sap_channel);
                  printf("cid=%x\n", cptr);
                } 
#endif
 
              if                       /* sap on channel              */
                 (p->sap_ptr->user_sap_channel == cptr)
                {
/* LEHb defect 43788 */
		if                     /* sap local busy mode due to
					  network data                */
		   (p->sap_ptr->retry_rcvn_buf != 0)
		  {
				       /* then decrement the common sap
					  local busy counter.         */
		  p->common_cb.lbusy_ctr--;
				       /* indicate that a sap wakeup is
					  no longer needed */
		  p->sap_list[p->sapno].wakeup_needed = FALSE;
				       /* return the buffer to the
					  system buffer pool.         */
		  m_freem (p->sap_ptr->retry_rcvn_buf);
				       /* clear the netdata buffer ptr */
		  p->sap_ptr->retry_rcvn_buf = 0;
		  }
/* LEHe */
                  /****************************************************/
                  /* indicate link stations will not return * status  */
                  /* by indicating SAP is being ABORTED               */
                  /****************************************************/

                  p->sap_ptr->sap_retcode = ABORTED;

                  /****************************************************/
                  /* search for all link stations                     */
                  /****************************************************/
 

                  for (j = 0; j < MAX_SESSIONS; j++)
                    {
 
                      if ((p->station_list[j].sapnum == i) && 
                         (p->station_list[j].in_use == TRUE))
                        {
                          p->stano = j;
                          p->sta_ptr = (struct station_cb *)
                             p->station_list[j].sta_cb_addr;

                          /********************************************/
                          /* abort the link station                   */
                          /********************************************/

                          lansta(p, CLOSENO);

                          /********************************************/
                          /* free the link station cb                 */
                          /********************************************/
 

                          if (p->station_list[p->stano].sta_cb_addr !=
                             0)
/* <<< feature CDLI >>> */
			    assert(xmfree((caddr_t)p->station_list[j].
			       sta_cb_addr, (caddr_t)kernel_heap) == 0);
/* <<< end feature CDLI >>> */
                        } 
                    } 

                  /****************************************************/
                  /* halt the device driver netid (sap)               */
                  /****************************************************/
 
/* defect 155401 */
#ifndef EDL
		  if                   /* sap state is not SAP_CLOSE_PEND
					  ie. not already halting, and
					  not LS/X RIPL 0xFC          */

		     ((p->sap_ptr->sap_state != SAP_CLOSE_PEND) &&
		      ((p->sapno *2) != DISCOVERY_SAP))
                    {
/* <<< feature CDLI >>> */
		      /******************************************/
		      /* issue ns del filter to DH for user sap */
		      /******************************************/

		      bzero(&ns_filter, sizeof(ns_filter));
		      ns_filter.filtertype = NS_8022_LLC_DSAP;
		      ns_filter.dsap = p->sapno *2;

		      p->rc = ns_del_filter(p->nddp, &ns_filter,
						sizeof(ns_filter));

                      /************************************************/
		      /* ignore any return from ns_del_filter         */
                      /************************************************/

                    }
#endif
/* end defect 155401 */
/* defect 142249 */
                  /****************************************************/
                  /* call llget to fetch address and release storage  */
                  /* used in multicast (group) address list           */
                  /****************************************************/

                  while /* there are group addresses in the sap's list */
                        (llget (&tempaddr[0],p))
                    {
                      /* issue ndd disable address to the device driver */
                      p->rc = (*p->nddp->ndd_ctl)(p->nddp,
                                     NDD_DISABLE_ADDRESS, &tempaddr[0], 6);
                    }
/* end defect 142249 */

                  /****************************************************/
                  /* free the sap control block                       */
                  /****************************************************/

		  assert(xmfree(p->sap_ptr, kernel_heap) == 0);
/* <<< end feature CDLI >>> */

                  /****************************************************/
                  /* clear this sap entry from the sap list.          */
                  /****************************************************/

                  bzero(&p->sap_list[i], sizeof(p->sap_list[i]));
                  p->sap_list[i].t1_ctr = -1;
                  cptr->saps--;
                } 
            } 
        } 
#ifdef   DEBUG
 
      if (p->debug)
        printf("lan_close %d\n", p->common_cb.plc_state);
#endif
/* defect 155341 */
      if ((p->common_cb.plc_state != PLC_CLOSED) &&
          (p->nddp != 0))
/* end defect 155341 */ 
        {
 
          if                           /* last channel on port        */
             (p->dlc_port.chan_count == 1)
            {
#ifdef   DEBUG
 
              if (p->debug)
                printf("CLOSE THE DEVICE\n");
#endif

/* feature CDLI */

	      /********************************************************/
	      /* call halt_ndd to delete the dlc filters for saps     */
	      /* 0x00 and 0xFC and ethertype 0x80D5 and async status. */
	      /* Also frees the ndd.                                  */
	      /********************************************************/

	      halt_ndd(p);

/* end feature CDLI */

/* defect 155341 --- moved fp_write and xmfree of p1_to_cache to kexit */

            } 
        } 

      /****************************************************************/
      /* unlock channel control block                                 */
      /****************************************************************/

      /* defect 122577 */
      simple_unlock(&cptr->lock);
      /* end defect 122577 */


    } 
  TRACE1(p, "KCLe");
} 


/**********************************************************************
 *  NAME: link_manager
 *
 *  FUNCTION: Initiates a loop to handle receive completion and status
 *              available posts from the device handler; timer posts
 *              from the system; and close port from DLC.
 *
 *  EXECUTION ENVIRONMENT: Operates in DLC process.
 *
 *  NOTES:
 *
 *  DATA STRUCTURES:
 *       MODIFIED:
 *         sap_rtncode -- SAP Return Code
 * defect 115819
 *         term_kproc -- Process Termination Indicator
 * end defect 115819
 *
 *  RETURNS: None
 *
 **********************************************************************/

link_manager(p)
  register struct port_dcl *p;
{
  struct status_block *sblk_ptr;       /* pointer to device handler   
                                          status cb                   */
  unsigned long t_rc;                  /* timer ID                    */
  ulong    rtn;                        /* return code                 */
  struct que_entry rq_data;
  ulong    dlctype;                    /* lan type                    */
  ulong    wait_rtn;
  struct dlc_getx_arg *getx;
  struct mbuf *mb;
  struct dlc_chan *c_ptr;
  int rqlock;                          /* defect 127690 */

  /********************************************************************/
  /* start the ticker timer for continuous post ecb's at 500 msec     */
  /* intervals. (note - there are 1024 timer ticks per second)        */
  /********************************************************************/
/* <<< THREADS >>> */
  timeout(lan_timeout, p->dlc_port.kproc_tid, HZ/2);
/* <<< end THREADS >>> */


  /********************************************************************/
  /* initialize the process termination indicator.                    */
  /********************************************************************/

  /* defect 167068 moved simple_lock(&p->dlc_port.lock) to init_proc */

/* defect 115819 */
  p->dlc_port.term_kproc = FALSE;
/* end defect 115819 */
 
  do
    {                                  /* loop on ecb's and input     
                                          queue elements.             */

      /****************************************************************/
      /* the process is to be deleted.                                */
      /****************************************************************/
 

/* defect 115819 */
      if (p->dlc_port.term_kproc == TRUE)
/* end defect 115819 */
        break;

      /****************************************************************/
      /* setup values for performance trace                           */
      /****************************************************************/

      dlctype = HKWD_SYSX_DLC_PERF|DLC_TRACE_WAITB;
#ifdef   TRL
      dlctype |= DLC_TOKEN_RING;
#endif
#ifdef   FDL
      dlctype |= DLC_FDDI;
#endif
#ifdef   EDL
      dlctype |= DLC_ETHERNET;
#endif
#ifdef   E3L
      dlctype |= DLC_IEEE_802_3;
#endif

/* call trchklt to record performance data                            */

      trchklt(dlctype, 0);

      /*--------------------------------------------------------------*/
      /* e_wait for a post ecb                                        */
      /*--------------------------------------------------------------*/
      /* issue e_wait for a receive ring queue post, a timer post, or */
      /* a write ring queue post. any or all of the above ecb's can be*/
      /* indicated concurrently. (note - always returns a 0 return    */
      /* code.) DYNA                                                  */
      /****************************************************************/

      /* defect 122577 */
      simple_unlock(&p->dlc_port.lock);
      /* end defect 122577 */

/* <<< THREADS >>> */
      wait_rtn = et_wait(0xff000000, 0xff000000, EVENT_SHORT);
/* <<< end THREADS >>> */


      /****************************************************************/
      /* setup values to record performance trace                     */
      /****************************************************************/

      dlctype = HKWD_SYSX_DLC_PERF|DLC_TRACE_WAITE;
#ifdef   TRL
      dlctype |= DLC_TOKEN_RING;
#endif
#ifdef   FDL
      dlctype |= DLC_FDDI;
#endif
#ifdef   EDL
      dlctype |= DLC_ETHERNET;
#endif
#ifdef   E3L
      dlctype |= DLC_IEEE_802_3;
#endif

/* call trchklt to record performance data                            */

      trchklt(dlctype, wait_rtn);

      /* defect 122577 */
      simple_lock(&p->dlc_port.lock);
      /* end defect 122577 */

  
      if                               /* a timer completion is posted*/
         (TSTBIT(wait_rtn, ECB_TIMER) == TRUE)

        /**************************************************************/
        /* call timeout handler.                                      */
        /**************************************************************/

        time_out(p);
 
      if                               /* close port requested        */
         (TSTBIT(wait_rtn, ECB_CLOSE) == TRUE)
        {

          /************************************************************/
          /* call pclose to close port                                */
          /************************************************************/

#ifdef   DEBUG
 
          if (p->debug)
            printf("call lan_pclose\n");
#endif
          lan_pclose(p);
 
          if                           /* this is the last close on   
                                          the port                    */
             ((p->common_cb.plc_state == PLC_CLOSED) && 
             (p->dlc_port.chan_count == 1))
            {

              /********************************************************/
              /* indicate that the kernel process needs to be freed   */
              /********************************************************/

/* defect 115819 */
	      p->dlc_port.term_kproc = TRUE;
/* end defect 115819 */
            } 
 
          else                         /* not the last close on this  
                                          port                        */
            {

              /********************************************************/
              /* wait for the head code to issue the sleep that       */
              /* directly follows its ECB_CLOSE post to lankproc      */
              /********************************************************/
 

              while (p->dlc_port.kcid->proc_id == EVENT_NULL)
                delay(10);

              /********************************************************/
              /* wake up the head code, who is waiting for close      */
              /* completion from lankproc                             */
              /********************************************************/

              e_wakeup((int *)&p->dlc_port.kcid->proc_id);
            } 
        } 
 
      if ((TSTBIT(wait_rtn, ECB_RCV) == TRUE) || (TSTBIT(wait_rtn, 
         ECB_STATUS) == TRUE))
        {
 
          do
            {
              /* lock the ring queue - defect 127690 */
              rqlock = disable_lock (PL_IMP, &(p->dlc_port.ringq_lock));
              rtn = dlc_rqget(p->dlc_port.rcv_ringq, &rq_data);
              /* unlock the ring queue - defect 127690 */
              unlock_enable (rqlock, &(p->dlc_port.ringq_lock));
 
              if (rtn != 0)            /* error or empty              */
                break;
#ifdef   DEBUG
 
              if (p->debug)
                printf("rq_data=%x q=%x\n", rq_data.entry[0], 
                   p->dlc_port.rcv_ringq);
#endif
 
              switch (rq_data.entry[0])
                {
                  case (DLC_RCV) :
/* <<< feature CDLI >>> */
/* removed case (DLC_OFL) : */
/* <<< end feature CDLI >>> */
                    if (p->common_cb.plc_state == PLC_OPENED)
                      rcv_completion(p, &rq_data);
                    else /* PLC no longer open (defect 165426) */
                      m_freem ((struct mbuf *)rq_data.entry[1]);
                    break;
                  case (DLC_STATUS) :
                    status(p, &rq_data);
                    break;
/* <<< feature CDLI >>> */
/* removed case (DLC_DISC_DONE) : */
/* <<< end feature CDLI >>> */
                  default  :

                    /**************************************************/
                    /* ERROR: unknown status type                     */
                    /**************************************************/

                    lanerrlg(p, ERRID_LAN8011, NON_ALERT, 
                       PERM_PLC_ERR, 0, FILEN, LINEN);
                } 
            } 
 
          while (1);
        } 
 
      if (TSTBIT(wait_rtn, ECB_LOCAL) == TRUE)
        {
 
          do
            {
              rtn = dlc_rqget(p->loopback.loopback_rq, &rq_data);
 
              if (rtn != 0)            /* error or empty              */
                break;
              TRACE3(p, "LPrc", rq_data.entry[0], rq_data.entry[1]);
 
              switch (rq_data.entry[0])
                {
                  case (DLC_RCV) :
 
                    if (p->common_cb.plc_state == PLC_OPENED)
                      rcv_completion(p, &rq_data);
                    break;
                  case (DLC_STAS_RES) :

                    /**************************************************/
                    /* now we need to get the mbuf address from the   */
                    /* ring queue and then we need to get the extended*/
                    /* structure exception structure from the mbuf,   */
                    /* and then just call the user-supplied exception */
                    /* handler routine                                */
                    /**************************************************/

                    mb = (struct mbuf *)rq_data.entry[1];
                    getx = MTOD(mb, struct dlc_getx_arg *);
                    c_ptr = (struct dlc_chan *)getx->result_code;
                    getx->result_code = 0;
                    p->rc = (*c_ptr->excp_fa)(getx, c_ptr);
                    m_free(mb);
 
                    if (p->rc != 0)
                      {
                        lanerrlg(p, ERRID_LAN8087, NON_ALERT, 
                           PERM_STA_ERR, DLC_ERR_CODE, FILEN, LINEN);
                        shutdown(p, HARD);
                      } 
                    break;

/* <<< feature CDLI >>> */
/* <<< removed  case (DLC_DISC_DONE) >>> */
/* <<< end feature CDLI >>> */

                  default  :

                    /**************************************************/
                    /* ERROR: unknown local rcv type                  */
                    /**************************************************/

                    lanerrlg(p, ERRID_LAN8011, NON_ALERT, 
                       PERM_PLC_ERR, 0, FILEN, LINEN);
                } 
            } 
 
          while (1);
        } 

      /*--------------------------------------------------------------*/
      /* check for plc failure                                        */
      /*--------------------------------------------------------------*/
 

      if                               /* a physical link failure has 
                                          occurred (ie. all saps)     */
         (p->common_cb.plc_retcode != DLC_OK)
        {
/* defect 155341 */
          if /* the port exists */ 
             (p->nddp)
            {
              /************************************************************/
              /* call the physical link blowout routine.                  */
              /************************************************************/

              plc_blowout(p);
            }
/* end defect 155341 */
        } 
 
      else                             /* no physical link failure    
                                          occurred.                   */
        {
 
          if                           /* a sap failure is indicated  
                                          in the common control block */
             (p->common_cb.sap_retcode != DLC_OK)
            {

              /********************************************************/
              /* reset the common sap failure indicator so that the   */
              /* next sap failure will be picked up.                  */
              /********************************************************/

              p->common_cb.sap_retcode = DLC_OK;
 
              if                       /* the current sap CORRELATOR  
                                          is within range (1-127)     */
                 ((p->sapno > 0) && (p->sapno < 128))

                /******************************************************/
                /* ok to address the sap list.                        */
                /******************************************************/

                {
 
                  if                   /* the current sap is "in use" */
                     (p->sap_list[p->sapno].in_use == TRUE)

                    /**************************************************/
                    /* ok to address the sap control block.           */
                    /**************************************************/

                    {

                      /************************************************/
                      /* get addressability to the sap control block. */
                      /************************************************/

                      p->sap_ptr = (struct sap_cb *)p->sap_list
                         [p->sapno].sap_cb_addr;
 
                      if               /* a sap failure has indeed    
                                          occurred                    */
                         (p->sap_ptr->sap_retcode != DLC_OK)
                        {

                          /********************************************/
                          /* call the sap shutdown routine.           */
                          /********************************************/

                          TRACE3(p, "KSSH", wait_rtn, p->sapno);
                          sap_shutdown(p);
                        } 
                    } 
                } 
            } 
        } 
    } 
 
  while (1);                           /* loop on _wait               */
  
/* defect 167068, moved simple_unlock(&p->dlc_port.lock) to end of init_proc */

  return (0);
} 

/* <<< feature CDLI >>> */
 /********************************************************************
 * NAME: status
 *
 * FUNCTION: Handle asynchronous status returned by Device Handler
 *
 * EXECUTION ENVIRONMENT: Operates in DLC process
 *
 * NOTES:
 *
 * RETURNS: None
 *
 **********************************************************************/

status(p,rq_data)
  register struct port_dcl *p;         /* port control block          */
  register struct que_entry *rq_data;
{
  TRACE2(p, "KSTb", rq_data->entry[1]);

  switch /* ndd status code */
	 (rq_data->entry[1])
    {
      case  NDD_CONNECTED :
          {
	    /* The device is now connected to the media.  This is the
	       asyncronous completion of a device driver open (ns_alloc)
	       whenever that device was not previously opened */

	    TRACE1(p, "NDDC");

	    /* if the physical link is opening */
	    if (p->common_cb.plc_state == OPENING_DEVICE)
	      {
		/* set the physical link state to STARTING_DEVICE */
		p->common_cb.plc_state = STARTING_DEVICE;

		/* Call start_filters to add the sap filters */
		start_filters(p);
	      }
          } 
        break;

      case  NDD_HARD_FAIL :
	  {
	    /* Device hard failure. This is the asyncronous notification
	       of a non-recoverable hard failure from the device driver */

	    TRACE1(p, "NDDF");

            /************************************************/
            /* ERROR: physical link blew out                */
            /************************************************/
            /* needs a new ERRID ???????                    */
            lanerrlg(p, ERRID_LAN802F, NON_ALERT, 
                         PERM_PLC_ERR, 0, FILEN, LINEN);
	  }
	break;

      default  :
	    TRACE1(p, "NDDU");

            /************************************************/
            /* ERROR: Unknown status from device handler    */
            /************************************************/

            lanerrlg(p, ERRID_LAN802F, NON_ALERT, 
                         INFO_ERR, 0, FILEN, LINEN);
	break;

    } /* end switch on status code */
  TRACE1(p, "KSTe");
} /* end status */
/* <<< end feature CDLI >>> */

/* <<< feature CDLI >>> */
/* <<< removed disc_completion(p,rq_data) >>> */
/* <<< end feature CDLI >>> */

/*
 **********************************************************************
 *  NAME: rcv_completion
 *
 *  FUNCTION:  Handles receive completion posted by device handler.
 *               This post indicates a packet has been received by the
 *               device handler and placed in the ring queue
 *
 *  EXECUTION ENVIRONMENT:  Operates under DLC process
 *
 *  NOTES:
 *
 *  DATA STRUCTURES:
 *       MODIFIED:
 *          m            -- Receive Buffer address
 *          rcv_data     -- Packet header data
 *          ring_head_out -- pointer to next ring queue entry
 *          routing_info -- Routing Information Present Flag
 *          sapno        -- Current SAP Number
 *
 *  RETURNS:  None
 *
 **********************************************************************/

rcv_completion(p,rq_data)
  register struct port_dcl *p;
  register struct que_entry *rq_data;
{
/* LEHb defect 44499 */
  int      discard_buf = 0;
/* LEHe */
  ulong    dlctype;                    /* lan type                    */
  int      len;

  p->stano = NO_MATCH;
  p->sta_ptr = DLC_NULL;
  p->m = (struct mbuf *)rq_data->entry[1];
  p->d.data_ptr = MTOD(p->m, caddr_t);
  TRACE3(p, "KRCb", p->m, p->d.data_ptr);

/* <<< feature CDLI >>> */
/* removed  if (rq_data->entry[0] == DLC_OFL) */
/* <<< end feature CDLI >>> */

  /********************************************************************/
  /* setup values to record monitor trace                             */
  /********************************************************************/

  dlctype = HKWD_SYSX_DLC_PERF|DLC_TRACE_RCVBB;
#ifdef   TRL
  dlctype |= DLC_TOKEN_RING;
#endif
#ifdef   FDL
  dlctype |= DLC_FDDI;
#endif
#ifdef   EDL
  dlctype |= DLC_ETHERNET;
#endif
#ifdef   E3L
  dlctype |= DLC_IEEE_802_3;
#endif

  /********************************************************************/
  /* call trchklt to record performance trace data                    */
  /********************************************************************/

  trchklt(dlctype, p->m->m_len);

  /********************************************************************/
  /* copy receive data to a word aligned area                         */
  /********************************************************************/

/* <<< feature CDLI >>> */
#ifndef TRLORFDDI
    bcopy(p->d.data_ptr, &(p->rcv_data), sizeof(struct rcv_data));
#endif /* not TRLORFDDI */

#ifdef TRL
#define ROUTING_OFFSET TRL_ROUTING_OFFSET
#elif FDL
#define ROUTING_OFFSET FDL_ROUTING_OFFSET
#endif

#ifdef TRLORFDDI

    if                                 /* routing is present         */
/* <<< defect 129185 >>> */
       (TSTBIT(*(p->d.data_ptr + (ROUTING_OFFSET - 6)), RI_PRESENT) == TRUE )
/* <<< end defect 129185 >>> */
				       /* determine the size of the
					  routing information        */
      len = (int)(*(p->d.data_ptr + ROUTING_OFFSET) & 0x1f);
    else
				       /* set routing length to zero */
      len = 0;

				       /* pre-zero the routing control to
					  remove any old route length */
				       /* note - required for trgen  */
    bzero(&(p->rcv_data.ri_field[0]), 2);
				       /* get fc, daddr, saddr, ri   */
    bcopy(p->d.data_ptr, &(p->rcv_data), (ROUTING_OFFSET + len));
				       /* get dlc header             */
    bcopy((p->d.data_ptr + ROUTING_OFFSET + len),
	   &(p->rcv_data.lsap), 4);
				       /* set the i-field ptr        */
    p->i.i_field_ptr = (p->d.data_ptr + ROUTING_OFFSET + 3 + len);

#endif /* TRLORFDDI */
/* <<< end feature CDLI >>> */

  /********************************************************************/
  /* get current sap number from dlc header                           */
  /********************************************************************/

  p->sapno = p->rcv_data.lsap/2;

/* defect 82006    */
      /****************************************************************/
      /* get addressability to the sap                                */
      /****************************************************************/

      p->sap_ptr = (struct sap_cb *)p->sap_list[p->sapno].sap_cb_addr;
 

/* end defect 82006   */


#ifdef   DEBUG
 
  if (p->debug)
    printf("rcv sapno=%d\n", p->sapno);
#endif
#ifndef  TRLORFDDI
  p->lpdu_length = p->rcv_data.lpdu_length;
#endif                                  /* EDL E3L                     */
#ifdef   TRLORFDDI
/* defect 141275 */
  p->lpdu_length = (p->m->m_pkthdr.len - ROUTING_OFFSET - len);
/* end defect 141275 */

  /* LEHb defect XXX */
  /* deleted phy_ctl1 and phy_ctl2 */
  /* LEHe */

  /* LEH defect 44499 */
  /* deleted 5 lines */
  /* LEHe */
 
  if                                   /* routing information present */
     (TSTBIT(p->d.rcv_data->raddr[0], RI_PRESENT) == TRUE)
    {

      /****************************************************************/
      /* save routing information present info                        */
      /****************************************************************/

      p->routing_info = 1;

      /****************************************************************/
      /* reset routing information flag to get remote address         */
      /****************************************************************/

      CLRBIT(p->rcv_data.raddr[0], RI_PRESENT);

/* LEH defect 44499 */
      /****************************************************************/
      /* call test_routing to check and save routing information data */
      /****************************************************************/

      discard_buf = test_routing(p);

    }
  else                                 /* no routing present          */
    {
      /****************************************************************/
      /* reset routing information present info                        */
      /****************************************************************/

      p->routing_info = 0;

      /****************************************************************/
      /* clear routing information length                             */
      /****************************************************************/

      p->common_cb.ri_length = 0;
    }

#endif /* TRLORFDDI */
  if                                   /* buffer is still valid, and
					  the physical link is open   */
     ((discard_buf == 0) &&
      (p->common_cb.plc_state == PLC_OPENED))
/* LEHe */
    {
/* Defect 121116 moved defect 82006 test_grpaddr below */
      if                              /* for the discovery manager   */
         (p->rcv_data.lsap == DISCOVERY_SAP)
        {
          landiscv(p, RCVNO);
        } 
 
      else                             /* not for discovery           */
        {
 
          if                           /* for the null sap manager    */
             (p->rcv_data.lsap == NULL_SAP)
	    null_sap_mgr(p);
 
          else                         /* not for discovery or the    
                                          null sap manager.           */
            {

/* <<< feature CDLI >>> */
/* defect 82006 */
				      /* call test_grpaddr to check if
					 received packet is a group address
					 or token ring functional address  */
	      if ( test_grpaddr(p) != TRUE)
		{
		  /* NOTE: call to test_grpaddr routine returns
		     TRUE if it contains a group address, ie. the
		     address is for this SAP */

		  /* free invalid group address buffer  */
		  lanfree(p, p->m);

		  return (0);
		}
/* end defect 82006 */
/* <<< end feature CDLI >>> */

              /********************************************************/
              /* setup values to record performance trace             */
              /********************************************************/

              dlctype = HKWD_SYSX_DLC_PERF|DLC_TRACE_HASHB;
#ifdef   TRL
              dlctype |= DLC_TOKEN_RING;
#endif
#ifdef   FDL
              dlctype |= DLC_FDDI;
#endif
#ifdef   EDL
              dlctype |= DLC_ETHERNET;
#endif
#ifdef   E3L
	      dlctype |= DLC_IEEE_802_3;
#endif

              /********************************************************/
              /* call trchklt to record performance data              */
              /********************************************************/

              trchklt(dlctype, 0);


              /********************************************************/
              /* call the "find station in receive hash table"        */
              /* routine, with the hashing string received, to get the*/
              /* station list station number and receive hash table   */
              /* hashno.                                              */
              /********************************************************/

              find_sta_in_hash(p);

              /********************************************************/
              /* setup values to record performance trace             */
              /********************************************************/

              dlctype = HKWD_SYSX_DLC_PERF|DLC_TRACE_HASHE;
#ifdef   TRL
              dlctype |= DLC_TOKEN_RING;
#endif
#ifdef   FDL
              dlctype |= DLC_FDDI;
#endif
#ifdef   EDL
              dlctype |= DLC_ETHERNET;
#endif
#ifdef   E3L
              dlctype |= DLC_IEEE_802_3;
#endif

              /********************************************************/
              /* call trchklt to record performance data              */
              /********************************************************/

              trchklt(dlctype, 0);
 
              if                       /* a station was found in the  
                                          hash                        */
                 (p->stano != NO_MATCH)
                {

                  /****************************************************/
                  /* call the link station with cmd = receive         */
                  /* completion.                                      */
                  /****************************************************/

                  lansta(p, RCV_CMPLNO);
                } 
 
              else                     /* no station was found in the 
                                          hash                        */
                {

                  /****************************************************/
                  /* call rcv_di_data to see if packet is for direct  */
                  /* interface this will free the buffer otherwise    */
                  /****************************************************/

                  rcv_di_data(p);
                } 
            } 
        }
/* <<< feature CDLI >>> */
/* <<< moved defect 82006 above >>> */
/* <<< end feature CDLI >>> */
    } 
 
  else                                 /* error - the plc is not open 
                                          or adding its name          */
    {

      /****************************************************************/
      /* return the buffer to the pool                                */
      /****************************************************************/

      lanfree(p, p->m);
    } 
  TRACE1(p, "KRCe");
} 

/* defect 82006 */
 /********************************************************************
 * NAME: test_grpaddr
 *
 * FUNCTION: tests received packet destination address for a group address
 *            or token ring functional address. If found, performs test to
 *            determine if packet for this SAP. Discard packet if not for
 *            this SAP.
 *
 * EXECUTION ENVIRONMENT: Operates under DLC process
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *      MODIFIED:
 *          None
 *
 * RETURNS: 1 = if group or functional address present, it is for this SAP
 *          0 = group address present is not for this SAP
 *
 **********************************************************************/

 int test_grpaddr(p)
  register struct port_dcl *p;         /* pointer to port control
                                          block                       */
{

  ulong_t *p_to_ifuncaddr;  /* address in SAP control block             */
  ulong_t *p_to_funcaddr;  /* address in current packet                */
  ulong_t func_addr;       /* last 4 bytes of address in packet        */
  ulong_t in_funcaddr;     /* last 4 bytes of input address            */

  char *p_to_grpaddr;      /* first byte of destination address */
  char *p_to_grpbit;       /* group address bit in byte 3 of dest addr */
  int  func_address_packet;
  int  packet_sap;         /* packet for this SAP                      */



   if (p->sap_list[p->rcv_data.lsap/2].in_use != TRUE)  /* Defect 121116 */
      return(FALSE) ;
			 /* indicate packet for this SAP as default   */
   packet_sap = TRUE;

                        /* get address of first byte of destination address
                           and indicate default is not functional address  */
/* <<< feature CDLI >>> */
   p_to_grpaddr = (char *)&p->rcv_data.laddr;
/* <<< end feature CDLI >>> */
   func_address_packet = 0;

#ifdef TRLORFDDI
                       /* set address to group address bit  */
   p_to_grpbit = &p->rcv_data.laddr[2];

   if                  /* locally administered or globally administrated 
                           group address */
     ((*p_to_grpaddr == 0xc0) || (*p_to_grpaddr == 0x80)) {
#ifdef TRL
/* <<< feature CDLI >>> */
     if /* it's a functional address */
	( (*p_to_grpbit & 0x80) != 0x80)
       {
	 /* set functional address indicator */
	 func_address_packet  =  TRUE;

	 /* get the low order 4 bytes of the enabled functional address
	    bits from the sap control block */
	 p_to_ifuncaddr =
		    (ulong_t)&p->sap_ptr->sap_profile.func_addr_mask[2];

	 /* get the low order 4 bytes of the received functional address
	    bits */
	 p_to_funcaddr = (ulong_t) p_to_grpbit;
	 in_funcaddr = (ulong_t) *p_to_ifuncaddr;
	 func_addr = (ulong_t) *p_to_funcaddr;

	 if /* the received functional address bits are not enabled */
	    ( (in_funcaddr & func_addr) == 0 )   /* defect 160066 */
	   {
	     /* indicate invalid group packet for this port */
	     return (FALSE);
	   }
       }
  
#endif /* TRL */
#endif /* TRLORFDDI */
/* <<< end feature CDLI >>> */

#if defined(EDL) || defined(E3L)

      if           /* locally administered or globally administrated 
                      Ethernet group address */ 
        ((*p_to_grpaddr == 0x03) || (*p_to_grpaddr == 0x01)) {   


#endif
       

        if (!func_address_packet)  {
                      
     
          if              /* receive packet for a group address 
                                       not for this port */
             (!llcheck(&p->rcv_data.laddr,p)) {
                  

                        /* indicate invalid group packet for this port */
                       packet_sap = FALSE;
          }
	}
    }
                      /* packet for this sap */
    return  (packet_sap);


}

/* end defect 82006   */                      
        

 /********************************************************************
 * NAME: complete_listen
 *
 * FUNCTION: completes processing a listen pending request
 *
 * EXECUTION ENVIRONMENT: Operates under DLC process
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *      MODIFIED:
 *          listen_pending -- Listen pending
 *          rcv_data       -- Packet header data
 *
 * RETURNS: None
 *
 **********************************************************************/

complete_listen(p,input_func)
  register struct port_dcl *p;         /* pointer to port control     
                                          block                       */
  uint     input_func;                 /* flag indicating name        
                                          response packet * should be 
                                          sent                        */
{
  TRACE1(p, "KLIb");

  /********************************************************************/
  /* reset the listen pending indicator.                              */
  /********************************************************************/

  p->sap_list[p->sapno].listen_pend = FALSE;

  /********************************************************************/
  /* save the incomming remote's address, sap, and name values.       */
  /********************************************************************/

  bcopy(p->rcv_data.raddr, p->sap_ptr->listen_raddr, 6);
  p->sap_ptr->incomming_rsap = (p->rcv_data.rsap&RESP_OFF);
 
  if                                   /* discovery procedure         */
     (p->rcv_data.lsap == DISCOVERY_SAP)
    {

      /****************************************************************/
      /* save remote name length                                      */
      /****************************************************************/

      p->sap_ptr->listen_rname_length = p->object_vector.length-
         SIZE_VECTOR_HEADER;

      /****************************************************************/
      /* save remote name                                             */
      /****************************************************************/

      bcopy(p->object_vector.value, p->sap_ptr->listen_rname, 
         p->sap_ptr->listen_rname_length);
      p->sap_ptr->incomming_rsap = (p->lsap_vector.value&RESP_OFF);
    } 
 
  if                                   /* a name found response should
                                          be sent                     */
     (input_func == 1)
    {

      /****************************************************************/
      /* send the "name found" response.                              */
      /****************************************************************/

      found_vector_gen(p, p->sap_ptr->sap_profile.local_sap);
    } 

  /********************************************************************/
  /* build hashing string for the remote station address, the local   */
  /* sap, and the remote sap values.                                  */
  /********************************************************************/

  bcopy(p->sap_ptr->listen_raddr,
     p->common_cb.u_h.s_h.hash_string_raddr, 6);
  p->common_cb.u_h.s_h.hash_string_rsap = p->sap_ptr->incomming_rsap;
  p->common_cb.u_h.s_h.hash_string_lsap =
     p->sap_ptr->sap_profile.local_sap;

  /********************************************************************/
  /* call the "add station to receive hash table" routine, with the   */
  /* hashing string and station slotno.                               */
  /********************************************************************/

  add_sta_to_hash(p);

  /********************************************************************/
  /* call the link station with command =listen completion.           */
  /********************************************************************/

  lansta(p, LISTEN_CMPLNO);
  TRACE1(p, "KLIe");
} 


 /********************************************************************
 * NAME: null_sap_mgr
 *
 * FUNCTION: Determines response to a packet sent to Null Sap.
 *            Sends either XID or Test response.
 *
 * EXECUTION ENVIRONMENT: Operates in DLC process
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * RETURNS: None
 *
 **********************************************************************/

null_sap_mgr(p)
  register struct port_dcl *p;
{
  uint     discard_buf;                /* flag to return buffer to    
                                          pool                        */
  uchar    ctl1_masked;                /* control byte                */
  ulong    dlctype;                    /* lan type                    */
  ulong_t  port_sta;                   /* station number and port       
                                        number                       */
#ifdef   DEBUG
 

  if (p->debug)
    printf("null_sap_mgr\n");
#endif

  /*------------------------------------------------------------------*/
  /* receive post monitor trace routine                               */
  /*------------------------------------------------------------------*/
  /* setup values to record monitor trace                             */
  /********************************************************************/

  dlctype = DLC_TRACE_RNONI<<8;
#ifdef   TRL
  dlctype |= DLC_TOKEN_RING;
#endif
#ifdef   FDL
  dlctype |= DLC_FDDI;
#endif
#ifdef   EDL
  dlctype |= DLC_ETHERNET;
#endif
#ifdef   E3L
  dlctype |= DLC_IEEE_802_3;
#endif

  /********************************************************************/
  /* get station number in upper half word and get number from port   */
  /* name in lower half word                                          */
  /********************************************************************/
/* <<< feature CDLI >>> */
#ifndef FDL
  port_sta = ((p->stano<<16)|p->dlc_port.namestr[3]);
#elif FDL
  port_sta = ((p->stano<<16)|p->dlc_port.namestr[4]);
#endif
/* <<< end feature CDLI >>> */

  /********************************************************************/
  /* call trchkgt to record monitor trace data                        */
  /********************************************************************/

  trchkgt(HKWD_SYSX_DLC_MONITOR|dlctype, p->rcv_data.ctl1, p->m, 
     p->m->m_len, ((p->rcv_data.lsap<<16)|p->rcv_data.rsap), port_sta);

  /********************************************************************/
  /* build a copy of control byte #1 with the poll/final bit masked on*/
  /* for faster compare.                                              */
  /********************************************************************/

  ctl1_masked = (p->rcv_data.ctl1|PF1_MASK);

/* LEHb defect 44499 */
/* deleted 17 lines - test_routing call */

  /********************************************************************/
  /* call test_pending to test for listen or call pending modes to    */
  /* determine if a link station start should be completed            */
  /********************************************************************/

  discard_buf = test_pending(p);
/* LEHe */

      if                               /* the buffer was returned from
                                          test_pend, ie. no link      
                                          station                     */

      /****************************************************************/
      /* was able to complete a call or listen, or the packet was for */
      /* the null sap and may need a response.                        */
      /****************************************************************/

         (discard_buf == 1)
        {
 
          if                           /* command packet received     */
             (TSTBIT(p->rcv_data.rsap, RESPONSE) == FALSE)
            {
 
              switch (ctl1_masked)
                {
                  case  XID :
#ifdef   DEBUG
 
                    if (p->debug)
                      printf("call send_ieee\n");
#endif

                    /**************************************************/
                    /* call Send IEEE XID to send response            */
                    /**************************************************/

                    send_ieee_xid(p);

                    /**************************************************/
                    /* do not free the buffer                         */
                    /**************************************************/

                    discard_buf = 0;
                    break;
                  case  TEST :
#ifdef   DEBUG
 
                    if (p->debug)
                      printf("call lantrgen\n");
#endif

                    /**************************************************/
                    /* call test response generator                   */
                    /**************************************************/

                    lantrgen(p);

                    /**************************************************/
                    /* do not free the buffer                         */
                    /**************************************************/

                    discard_buf = 0;
                    break;
                  default  :           /* not XID or TEST             */
#ifdef   DEBUG
 
                    if (p->debug)
                      printf("error - rcvd bad cmd for null sap\n");
#endif

                    /**************************************************/
                    /* call error log - non supported control for the */
                    /* null sap                                       */
                    /**************************************************/

                    lanerrlg(p, ERRID_LAN0031, NON_ALERT, INFO_ERR, 0,
                    FILEN, LINEN);
                }                      /* end switch                  */
            }                          /* else this packet was a      
                                          response packet             */
        }                              /* a link station was started    
                                        on this packet               */
/* LEHb defect 44499 */
/* deleted 4 lines */
/* LEHe */
 
    if                                 /* the buffer is to be         
                                          discarded                   */
       (discard_buf == 1)
      {

        /**************************************************************/
        /* return the buffer to the pool                              */
        /**************************************************************/

        lanfree(p, p->m);
      } 
}                                      /* end null_sap_mgr            */

 /********************************************************************
 * NAME: rcv_di_data
 *
 * FUNCTION: Handles packet send as direct interface data.
 *
 * EXECUTION ENVIRONMENT: Operates in DLC process
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *      MODIFIED:
 *         sapno  -- SAP Number
 *         flags  -- I/O Extension Result Flag
 *         corr   -- I/O Extension Correlator
 *         m_len  -- Receive Buffer Length
 *
 * RETURNS: None
 *
 **********************************************************************/

rcv_di_data(p)
  register struct port_dcl *p;
{
  uint     discard_buf;                /* flag to return buffer to    
                                          pool                        */
  struct que_entry rq_data;            /* ring queue data             */
  struct dlc_chan *c_ptr;
  ulong    dlctype;                    /* lan type                    */
  ulong_t  port_sta;                   /* station number and port       
                                        number                       */
  ulong_t  header_length;              /* packet header length 173162 */
#ifdef   DEBUG
 

  if (p->debug)
    printf("rcv_di_data %x\n", p);
#endif

  /********************************************************************/
  /* set flag to discard buffer as default                            */
  /********************************************************************/

  discard_buf = 1;

  /********************************************************************/
  /* get the sap index from the incomming destination sap             */
  /********************************************************************/

  p->sapno = p->rcv_data.lsap/2;
 
  if                                   /* the incomming destination   
                                          sap identifies a sap that is
                                          in-use                      */
     (p->sap_list[p->sapno].in_use == TRUE)
    {

      /****************************************************************/
      /* get addressability to the sap                                */
      /****************************************************************/

      p->sap_ptr = (struct sap_cb *)p->sap_list[p->sapno].sap_cb_addr;
 
      if                               /* the in-use sap is open      */
         (p->sap_ptr->sap_state == SAP_OPEN_STATE)
        {

/* LEHb defect 44499 */
/* deleted 17 lines - test_routing call */

	      /********************************************************/
	      /* call test_pending to test for listen or call pending */
	      /* modes to determine if a link station start should be */
	      /* completed.                                           */
	      /********************************************************/

	      discard_buf = test_pending(p);
/* LEHe */
 
              if                       /* the buffer was returned from
                                          test_pend, ie. no link      
                                          station                     */

              /********************************************************/
              /* was able to complete a call or listen                */
              /********************************************************/

                 (discard_buf != 0)
                {
 
                  if ((p->rcv_data.ctl1 == UI) || (p->rcv_data.ctl1 ==
                     UI_NO_PF))
		    {
/* LEHb defect 43788 */
		    /* if the user does not already have a busy condition
					       on receive network data */
		    if (p->sap_ptr->retry_rcvn_buf == 0)
		      {
/* LEHe */

#ifdef   TRLORFDDI
#ifdef   DEBUG
 
                      if (p->debug)
                        printf("routing_info=%d\n", p->routing_info);
#endif
 
                      if               /* routing information present */
                         (p->routing_info == 1)
                        {

                          /********************************************/
                          /* set routing information flag in token    */
                          /* ring header                              */
                          /********************************************/

                          SETBIT(p->rcv_data.laddr[0], RI_PRESENT);
                        } 
#endif                                  /* TRLORFDDI                         */

                      /************************************************/
                      /* set results to network data received         */
                      /************************************************/

                      p->dlc_io_ext.flags = DLC_NETD;

                      /************************************************/
                      /* return sap user correlator                   */
                      /************************************************/

                      p->dlc_io_ext.sap_corr = 
                         p->sap_ptr->sap_profile.user_sap_corr;

                      /************************************************/
                      /* setup data header length and adjust the mbuf */
                      /* data pointer and length (173162)             */
                      /************************************************/
#ifndef TRLORFDDI
                      p->dlc_io_ext.dlh_len = UN_HDR_LENGTH;
                      p->m->m_len = p->lpdu_length-3;
                      p->m->m_data += UN_HDR_LENGTH;
#endif /* not TRLORFDDI */
#ifdef TRLORFDDI
                      header_length = 
			(UN_HDR_LENGTH + (p->rcv_data.ri_field[0] & 0x1f));
		      p->m->m_len -= header_length;
		      p->dlc_io_ext.dlh_len = header_length;
                      p->m->m_data += header_length;
#endif

c_ptr = p->sap_ptr->user_sap_channel;

                      /************************************************/
                      /* setup values to record monitor trace         */
                      /************************************************/

                      dlctype = DLC_TRACE_RNETD<<8;
#ifdef   TRL
                      dlctype |= DLC_TOKEN_RING;
#endif
#ifdef   FDL
                      dlctype |= DLC_FDDI;
#endif
#ifdef   EDL
                      dlctype |= DLC_ETHERNET;
#endif
#ifdef   E3L
                      dlctype |= DLC_IEEE_802_3;
#endif

                      /************************************************/
                      /* get station number in upper half word and get*/
                      /* number from port name in lower half word     */
                      /************************************************/
/* <<< feature CDLI >>> */
#ifndef FDL
                      port_sta = ((p->stano<<16)|p->dlc_port.namestr[3]);
#elif FDL
		      port_sta = ((p->stano<<16)|p->dlc_port.namestr[4]);
#endif
/* <<< end feature CDLI >>> */

                      /************************************************/
                      /* call trchkgt to record monitor trace data    */
                      /************************************************/

                      trchkgt(HKWD_SYSX_DLC_MONITOR|dlctype, p->m, 
                         p->m->m_len, p->dlc_io_ext.sap_corr, 0, 
                         port_sta);

                      /************************************************/
                      /* setup values for performance trace           */
                      /************************************************/

                      dlctype = HKWD_SYSX_DLC_PERF|DLC_TRACE_RDIDA;
#ifdef   TRL
                      dlctype |= DLC_TOKEN_RING;
#endif
#ifdef   FDL
                      dlctype |= DLC_FDDI;
#endif
#ifdef   EDL
                      dlctype |= DLC_ETHERNET;
#endif
#ifdef   E3L
                      dlctype |= DLC_IEEE_802_3;
#endif

                      /************************************************/
                      /* call trchklt to record performance data      */
                      /************************************************/

                      trchklt(dlctype, p->m->m_len);

                      /************************************************/
                      /* call the user RCV network data routine       */
                      /************************************************/

                      p->rc = (*c_ptr->rcvn_fa)(p->m, &(p->dlc_io_ext), c_ptr);
 
                      switch (p->rc)
                        {
                          case  DLC_FUNC_OK :/* normal case           */
                            discard_buf = 0;
                            break;
                          case  DLC_FUNC_RETRY :
/* LEHb defect 43788 */
/* delete 1 line /
			    /* save the netd pointer in the sap cb    */
			    p->sap_ptr->retry_rcvn_buf = p->m;
			    /* save the receive extension in sap cb   */
			    bcopy (&(p->dlc_io_ext),
				   &(p->sap_ptr->retry_rcvn_ext),
				   sizeof(p->dlc_io_ext));
			    /* indicate that a sap wakeup is needed   */
			    p->sap_list[p->sapno].wakeup_needed = TRUE;
			    /* bump the common local busy counter     */
			    p->common_cb.lbusy_ctr++;
/* LEHe */
                            /******************************************/
                            /* indicate buffer to be saved            */
                            /******************************************/

                            discard_buf = 0;
                            break;
                          default  :

                            /******************************************/
                            /* call error log - user network data     */
                            /* routine failed                         */
                            /******************************************/

			    lanerrlg(p, ERRID_LAN8086, NON_ALERT,
			       PERM_SAP_ERR, DLC_USR_INTRF, FILEN, LINEN);
                        }              /* end switch                  */
/* LEHb defect 43788 */
		      }
		      /* else the user already has a busy condition on
						 receive network data */
		    else
		      {
		      /* ignore the received network data */
		      discard_buf = 1;
		      }
/* LEHe */
                    }                  /* else control byte received  
                                          is not UI data              */
                }                      /* else a link station was     
                                          started                     */
            }                          /* else the target sap is not  
                                          open                        */
        }                              /* else the target sap is not    
                                        in use                       */
/* LEHb defect 44499 */
/* deleted 3 lines */
/* LEHe */
 
    if                                 /* the buffer is to be         
                                          discarded                   */
       (discard_buf == 1)
      {

        /**************************************************************/
        /* return the buffer to the pool                              */
        /**************************************************************/

        lanfree(p, p->m);
      } 
}                                      /* end rcv_di_data             */

 /********************************************************************
 * NAME: test_pending
 *
 * FUNCTION: Check for a call pending to the exact sap/address or a listen
 *           pending to the sap range.  Only special command packets can
 *           complete a call or a listen (XID, TEST, SABME, and UI).
 *           Any token-ring routing is assumed to be good.
 *
 * EXECUTION ENVIRONMENT: Operates in DLC process
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *      MODIFIED:
 *         stano   -- Station Number
 *         p->sta_ptr -- Pointer to Station Control Block
 *
 * RETURNS: Buffer Status : 0 -> do not free buffer, it has been passed
 *                               to a link station.
 *                          1 -> buffer is being returned.
 *
 **********************************************************************/

test_pending(p)
  register struct port_dcl *p;
{
  uint     rtn;                        /* return status               */
  uint     buf_status;                 /* buffer status               */
  uint     found;                      /* flag indicating find        
                                          successful                  */
  uint     index;                      /* loop value                  */
  uint     no_response;                /* do not send name found rsp    
                                        flag                         */
#ifdef   DEBUG
 

  if (p->debug)
    printf("test_pending entered\n");
#endif

  /********************************************************************/
  /* check for a valid command frame (may be a test response also)    */
  /********************************************************************/

				       /* check on the packet type    */
    switch (p->rcv_data.ctl1|PF1_MASK)
      {
      case  XID :
      case  TEST :
      case  SABME :
      case  UI :
				       /* OK it's valid for call or
					  listen                      */
				       /* indicate buffer to be returned
					  as default action, and fall
					  through                     */
	buf_status = 1;
	break;

      default :
				       /* it's not the correct type   */
				       /* so don't check any further  */
	return (1);
	break;
      }                                /* end switch                  */

  /********************************************************************/
  /* scan for possible CALL IN PROGRESS                               */
  /********************************************************************/
  /* clear search indicator                                           */
  /********************************************************************/

  found = 0;
 
  for (index = 0; index < MAX_SESSIONS; index++)
    {
 
      if (found == 1)
        break;
 
      if                               /* the station in the station  
                                          list has a call pending     */

      /****************************************************************/
      /* or the station is active                                     */
      /****************************************************************/

         ((p->station_list[index].call_pend == TRUE) || 
         (p->station_list[index].sta_active == TRUE))
        {

          /************************************************************/
          /* get address to station control block                     */
          /************************************************************/

          p->sta_ptr = (struct station_cb *)p->station_list[index].
             sta_cb_addr;
          p->stano = index;
 
          if                           /* this station uses resolve   
                                          procedures, and             */

          /************************************************************/
          /* the packet came from the called station address, and the */
          /* packet came from the called sap address or the null sap  */
          /* address, and the targeted local sap is either null or    */
          /* this station's sap.                                      */
          /************************************************************/
/* defect 156006 */
             (((p->sta_ptr->ls_profile.flags&DLC_SLS_ADDR) != 0) &&
             (bcmp(p->sta_ptr->raddr, p->rcv_data.raddr, 6) == 0) &&
             (((p->rcv_data.rsap&RESP_OFF) ==
             p->sta_ptr->ls_profile.rsap) || ((p->rcv_data.rsap
             &RESP_OFF) == NULL_SAP)) &&
             (p->rcv_data.lsap/2 == p->station_list[p->stano].sapnum))
/* end defect 156006 */
            {
#ifdef   DEBUG
 
              if (p->debug)
                printf("call_pending located: stano=%d\n", p->stano);
#endif
 
              if                       /* station is in call pending  
                                          state                       */
                 (p->station_list[index].call_pend == TRUE)
                {

                  /****************************************************/
                  /* get addressability to the sap control block      */
                  /****************************************************/

                  p->sapno = p->station_list[p->stano].sapnum;
                  p->sap_ptr = (struct sap_cb *)p->sap_list[p->sapno].
                     sap_cb_addr;

                  /****************************************************/
                  /* set the incomming rsap to the station profile    */
                  /* rsap                                             */
                  /****************************************************/

		  p->sap_ptr->incomming_rsap =
		     p->sta_ptr->ls_profile.rsap;

/* LEHb defect 44499 */
#ifdef   TRLORFDDI
		  /****************************************************/
		  /* call set station route routine to save any       */
		  /* bridge routing that was received.                */
		  /****************************************************/

		  set_sta_route (p);

#endif /* TRLORFDDI */
/* LEHe */
                  /****************************************************/
                  /* call new_call_response routine to complete call  */
                  /****************************************************/

                  new_call_response(p);
                } 

              /********************************************************/
              /* set the flag to end search                           */
              /********************************************************/

              found = 1;
 
              if                       /* this packet was not to or   
                                          from the null sap           */
                 ((p->rcv_data.lsap != NULL_SAP) && ((p->rcv_data.rsap
                 &RESP_OFF) != NULL_SAP))
                {

                  /****************************************************/
                  /* call the link station to handle received packet  */
                  /****************************************************/

                  lansta(p, RCV_CMPLNO);

                  /****************************************************/
                  /* indicate that the buffer is not being returned   */
                  /****************************************************/

                  buf_status = 0;
                }                      /* else the buffer is for the  
                                          null sap manager            */
            }                          /* else not calling that       
                                          address via resolve         */
        }                              /* else no call pending        */
    }                                  /* end do index                */
 
  if                                   /* no calls were in progress to
                                          the remote sap and address  */
     (found == 0)
    {
      if                               /* the received packet is a    
                                          command packet              */
         ((TSTBIT(p->rcv_data.rsap, RESPONSE) == FALSE))
        {

          /************************************************************/
          /* scan for possible LISTEN PENDING                         */
          /************************************************************/
          /* clear search indicator                                   */
          /************************************************************/

          found = 0;
 
          for (index = 0; index < 128; index++)
            {
 
              if (found == 1)
                break;

/* defect 156006 */
              if /* the indexed sap has a listen pending, and the
                    received destination sap is the indexed sap       */
                 ((p->sap_list[index].listen_pend == TRUE) &&
                  (p->rcv_data.lsap/ 2 == index))
/* end defect 156006 */
                {

                  /****************************************************/
                  /* retrieve the sap pointer                         */
                  /****************************************************/

                  p->sapno = index;
                  p->sap_ptr = (struct sap_cb *)p->sap_list[p->sapno].
                     sap_cb_addr;

                  /****************************************************/
                  /* retrieve the pending station's slot#             */
                  /****************************************************/

                  p->stano = p->sap_ptr->listen_stano;
#ifdef   DEBUG
 
                  if (p->debug)
                    printf("listen_pending located: stano=%d\n", 
                       p->stano);
#endif

                  /****************************************************/
                  /* get addressability to the station's control block*/
                  /****************************************************/

                  p->sta_ptr = (struct station_cb *)p->station_list
                     [p->stano].sta_cb_addr;

                  /****************************************************/
                  /* set the incomming rsap to the station profile    */
                  /* rsap                                             */
                  /****************************************************/

		  p->sap_ptr->incomming_rsap =
		     p->sta_ptr->ls_profile.rsap;
 
                  if 

                  /****************************************************/
                  /* the remote's sap is in range for this listen     */
                  /****************************************************/

                     ((p->rcv_data.rsap >= 
                     p->sta_ptr->ls_profile.rsap_low) && 
                     (p->rcv_data.rsap <= 
                     p->sta_ptr->ls_profile.rsap_high))
                    {
                      found = 1 ;       /* Defect 149322 */
/* LEHb defect 44499 */
#ifdef   TRLORFDDI
		      /************************************************/
		      /* call set station route routine to save any   */
		      /* bridge routing that was received.            */
		      /************************************************/

		      set_sta_route (p);

#endif /* TRLORFDDI */
/* LEHe */
                      /************************************************/
                      /* call complete_listen to complete opening a   */
                      /* link station                                 */
                      /************************************************/

                      no_response = 0;
                      complete_listen(p, no_response);
 
                      if               /* the target sap is not the   
                                          null sap                    */
                         (p->rcv_data.lsap != NULL_SAP)
                        {

                          /********************************************/
                          /* call the link station to handle received */
                          /* packet                                   */
                          /********************************************/

                          lansta(p, RCV_CMPLNO);

                          /********************************************/
                          /* indicate that the buffer is not being    */
                          /* returned                                 */
                          /********************************************/

                          buf_status = 0;
                        }              /* else the buffer is for the  
                                          null sap manager            */
                    }                  /* else the remote's sap is not
                                          in range for this listen    */
                }                      /* else no listen is pending   
                                          for this sap                */
            }                          /* end sap list index          */
        }                              /* else the packet was a       
                                          response                    */
    }                                  /* else a pending call was     
                                          already found               */
  return (buf_status);
}                                      /* end test_pending            */

#ifdef   TRLORFDDI
/* LEHb defect 44499 */
 /********************************************************************
 * NAME: test_routing
 *
 * FUNCTION: Test Token Ring routing information field
 *
 * EXECUTION ENVIRONMENT: Operates in DLC process
 *
 * NOTES: Token Ring Only
 *
 * DATA STRUCTURES:
 *      MODIFIED:
 *          ri_length  -- Length of routing information field
 *          direction  -- Routing information direction flag
 *          all_rings  -- Routing information all rings field
 *          path_trace -- Routing information path trace field
 *
 * RETURNS: routing status: 0 -> routing field validated
 *                          1 -> discard the buffer
 *
 **********************************************************************/

test_routing(p)
  register struct port_dcl *p;
{
  uint     rtn = 0;                    /* return code                 */
  struct ri_control_field ri_control;
/* <<< feature CDLI >>> */
/* <<< removed ctl1_masked >>> */
/* <<< end feature CDLI >>> */

#ifdef   DEBUG
  if (p->debug)
    printf("test routing %x\n", p);
#endif

  /********************************************************************/
  /* presave routing information length                               */
  /********************************************************************/

  p->common_cb.ri_length = 0;

/* deleted 16 lines */

  /********************************************************************/
  /* get routing control information                                  */
  /********************************************************************/

  bcopy(&(p->d.rcv_data->ri_field[0]), &ri_control, 2);

  if                                 /* routing length is greater than
					2 bytes and less than the max
					route length and a multiple of 2 */
/* <<< feature CDLI >>> */
     ((ri_control.ri_lth > 2) &&
#ifdef TRL
      (ri_control.ri_lth < (TRL_ROUTING_LEN + 1)) &&
#elif FDL
      (ri_control.ri_lth < (FDL_ROUTING_LEN + 1)) &&
#endif
      ((ri_control.ri_lth & 1) == 0))
/* <<< end feature CDLI >>> */
    {
    /******************************************************************/
    /* save routing information length                                */
    /******************************************************************/

    p->common_cb.ri_length = ri_control.ri_lth;

/* deleted 6 lines */

    /******************************************************************/
    /* save routing information in save area                          */
    /******************************************************************/

    bcopy(&(p->rcv_data.ri_field[0]), &(p->common_cb.ri_field),
	  p->common_cb.ri_length);

    /******************************************************************/
    /* toggle direction indicator to return frame                     */
    /******************************************************************/

    p->common_cb.ri_field.direction ^= 1;

    /******************************************************************/
    /* reset all rings and limited broadcast                          */
    /******************************************************************/
    if ((TSTBIT(p->rcv_data.rsap, RESPONSE) == FALSE) &&
        ((p->rcv_data.ctl1|PF1_MASK) != UI))

      {
                                     /* it's a command frame            */
                                     /* and  not datagram or direct data */
        if                           /* single (limited) route broadcast
	     			         packet received                */
           ((p->common_cb.ri_field.single_route == TRUE) &&
	    (p->common_cb.ri_field.all_route == TRUE))
          {

          /****************************************************************/
          /* set route to all route broadcast to allow returning          */
          /* packet to find the best route                                */
          /****************************************************************/

          p->common_cb.ri_field.single_route = FALSE;

          /****************************************************************/
          /* set routing length to allow bridge segments to be            */
          /* appended as the packet passes thru bridges                   */
          /****************************************************************/

          p->common_cb.ri_length = 2;
          p->common_cb.ri_field.ri_lth = 2;

          /****************************************************************/
          /* clear direction bit                                          */
          /****************************************************************/

          p->common_cb.ri_field.direction = 0;
          }

        else                             /* not single route broadcast    */
          {
          if                               /* all route broadcast packet
		        		      received                    */
	     ((p->common_cb.ri_field.single_route == FALSE) &&
	        (p->common_cb.ri_field.all_route == TRUE))
	    {
	    if                           /* routing length is 2 bytes     */
		          	       /* ie, no routes are appended    */
	        (p->common_cb.ri_length == 2)
	      {
	      /************************************************************/
	      /* reset the routing length to zero (don't need routing)    */
	      /************************************************************/

	      p->common_cb.ri_length = 0;
	      }
	    else
	      {
	      /************************************************************/
	      /* reset all route broadcast indicator                      */
	      /************************************************************/

	      p->common_cb.ri_field.all_route = FALSE;
	      }
	    }
          }
        }
     else
                                     /* do not response with all      */
                                     /* route broadcast               */


      {
         p->common_cb.ri_field.all_route = FALSE;
         p->common_cb.ri_field.single_route = FALSE;
      }
    }
  else                               /* no bridge route on broadcast
					or routing length not correct */
    {

    if                               /* routing received is not 2-bytes
				       (length for no bridge present) */
       (ri_control.ri_lth != 2)
      {

	/**************************************************************/
	/* call error log - routing length error received             */
	/**************************************************************/

	lanerrlg(p, ERRID_LAN0038, NON_ALERT, INFO_ERR, 0, FILEN, LINEN);

	/**************************************************************/
	/* notify caller to discard the buffer                        */
	/**************************************************************/

	rtn = 1;
      }
    }
  return (rtn);
}                                      /* end test_routing            */
/* LEHe */
#endif                                 /* TRLORFDDI                   */

 /********************************************************************
 * NAME: send_ieee_xid
 *
 * FUNCTION: Send IEEE Xid response to remote SAP.
 *
 * EXECUTION ENVIRONMENT: Operates in DLC process
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *                  port_dcl -- Port control block
 *
 * RETURNS: None
 *
 **********************************************************************/

send_ieee_xid(p)
  register struct port_dcl *p;
{
  uchar    temp_rsap;
  uchar    temp_ctl1;
  uchar    ieee_xid[] = 
    {
      0x81,0x03,0xfe
    } 
  ;                                    /* IEEE XID DATA               */
#ifdef   DEBUG
 
  if (p->debug)
    printf("send ieee xid %x\n", p);
#endif

  /********************************************************************/
  /* save remote sap and input control byte                           */
  /********************************************************************/

  temp_rsap = p->rcv_data.rsap;
  temp_ctl1 = p->rcv_data.ctl1;

  /********************************************************************/
  /* move the remote source address to the destination address field. */
  /********************************************************************/

  bcopy(p->rcv_data.raddr, p->d.send_buf_rsp->raddr, 6);

  /********************************************************************/
  /* fill in the local address in the source address field.           */
  /********************************************************************/

  bcopy(p->common_cb.local_addr, p->d.send_buf_rsp->laddr, 6);
#ifndef  TRLORFDDI

  /********************************************************************/
  /* build the remainder of the data link header.                     */
  /********************************************************************/

  p->d.send_buf_rsp->lsap = p->rcv_data.lsap|RESP_ON;
  p->d.send_buf_rsp->rsap = temp_rsap;
  bcopy(ieee_xid, &(p->d.send_buf_rsp->ctl2), 3);
  p->d.send_buf_rsp->lpdu_length = 6;  /* lsap,rsap,ctl1,8103fe       */
  p->m->m_len = MIN_PACKET;
#endif                                  /* EDL or E3L                  */
#ifdef   TRLORFDDI

  /********************************************************************/
  /* save pointer to end of routing information data                  */
  /********************************************************************/

/* LEHb defect 44499 */
  p->ri.ptr_ri = p->d.send_buf_rsp->ri_field + p->common_cb.ri_length;
/* LEHe */

  if                                   /* routing information present */
     (p->common_cb.ri_length > 0)
    {

      /****************************************************************/
      /* move routing information to packet                           */
      /****************************************************************/
/* LEHb defect 44499 */
      bcopy(&(p->common_cb.ri_field), &(p->d.send_buf_rsp->ri_field[0]),
	 p->common_cb.ri_length);

      /****************************************************************/
      /* set routing information flag in source address               */
      /****************************************************************/

      SETBIT(p->d.send_buf_rsp->laddr[0], RI_PRESENT);
/* LEHe */
    } 

  /********************************************************************/
  /* build the remainder of the data link header.                     */
  /********************************************************************/

  p->ri.ri_sbp->lsap = p->rcv_data.lsap|RESP_ON;
  p->ri.ri_sbp->rsap = temp_rsap;
  p->ri.ri_sbp->ctl1 = temp_ctl1;
  bcopy(ieee_xid, &(p->ri.ri_sbp->ctl2), 3);
/* LEHb defect XXX */
/* HBTb - defect 76763 - force priority reset to 10 */
  p->d.send_buf_rsp->phy_ctl_1 = 0x10;
/* HBTe - defect 76763 */
/* <<< feature CDLI >>> */
/* LEHe */
#ifdef TRL
  p->d.send_buf_rsp->phy_ctl_2 = 0x40;
#endif /* TRL */
#ifdef FDL
/* defect 94039 */
  p->d.send_buf_rsp->reserved[0] = 0;
  p->d.send_buf_rsp->reserved[1] = 0;
  p->d.send_buf_rsp->reserved[2] = 0;
  p->d.send_buf_rsp->phy_ctl_1 = 0x50;
/* end defect 94039 */
#endif /* FDL */
  p->m->m_len = ROUTING_OFFSET +3 +sizeof(ieee_xid) +p->common_cb.ri_length;
#endif /* TRLORFDDI */

  /********************************************************************/
  /* call the write send command generator, with the add name         */
  /* response buffer address, disabling link trace.                   */
  /********************************************************************/

p->stano = NO_MATCH;                   /* disables trace              */
  lanwrtsg(p, p->m);
/* <<< end feature CDLI >>> */
} 

/* <<< feature CDLI >>> */
/* <<< moved device_started to lan.c as filter_started >>>*/
/* <<< end feature CDLI >>> */

/* <<< feature CDLI >>> */
/* <<< moved device_halted to lan.c as filter_halted >>>*/
/* <<< end feature CDLI >>> */

 /********************************************************************
 * NAME: timeout
 *
 * FUNCTION: Timer post handler.
 *
 * EXECUTION ENVIRONMENT: Operates in DLC process
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *      MODIFIED:
 *          t1_ena, t2_ena, t3_ena  -- timer enable fields
 *          t1_ctr, t2_ctr, t3_ctr  -- timer counter fields
 *          t_busy_ctr              -- timer busy counter
 *
 * RETURNS: none
 *
 **********************************************************************/

time_out(p)
  register struct port_dcl *p;
{
  register n;
  uchar    t_ena;
/* LEHb defect 43788 */
  struct dlc_chan *c_ptr;
/* LEHe */

  /********************************************************************/
  /* for index = 0 to maximum number of stations - 1                  */
  /********************************************************************/
 

  for (n = 0; n < MAX_SESSIONS; n++)
    {
      t_ena = p->station_list[n].t1_ena+p->station_list[n].t2_ena+
         p->station_list[n].t3_ena;
 
      if (t_ena != 0)
        {

          /*----------------------------------------------------------*/
          /* station local busy check                                 */
          /*----------------------------------------------------------*/
 

          if                           /* the indexed station needs a 
                                          wakeup                      */
             (p->station_list[n].wakeup_needed == TRUE)
            {

              /********************************************************/
              /* set the stationno to the current index;              */
              /********************************************************/

              p->stano = n;

              /********************************************************/
              /* call the link station with command = local busy      */
              /* wakeup                                               */
              /********************************************************/

              lansta(p, LBUSY_WAKEUPNO);
            } 

          /*----------------------------------------------------------*/
          /* station repoll timer check                               */
          /*----------------------------------------------------------*/
 

          if                           /* the station repoll timer is 
                                          endabled                    */
             (p->station_list[n].t1_ena != 0)
            {

              /********************************************************/
              /* decrement the station's repoll timer value in the    */
              /* station list.                                        */
              /********************************************************/

              p->station_list[n].t1_ctr--;
 
              if                       /* the T1 timer value has      
                                          decremented to zero         */
                 (p->station_list[n].t1_ctr == 0)
                {

                  /****************************************************/
                  /* set the stationno to the current index;          */
                  /****************************************************/

                  p->stano = n;
 
                  if                   /* a call is in progress for   
                                          this station                */
                     (p->station_list[p->stano].call_pend == TRUE)

                    /**************************************************/
                    /* call the dlc manager call timeout completion   */
                    /* routine.                                       */
                    /**************************************************/

                    mgr_call_timeout(p);
 
                  else                 /* no call is in progress for  
                                          this station.               */
                    {

                      /************************************************/
                      /* call the link station with command           */
                      /* =t1_timeout_completion.                      */
                      /************************************************/

                      lansta(p, T1_TIMEOUT_CMPLNO);
                    } 
                } 
            } 

          /*----------------------------------------------------------*/
          /* acknowledge timer check                                  */
          /*----------------------------------------------------------*/
 

          if                           /* stations acknowledgement    
                                          timer is enabled            */
             (p->station_list[n].t2_ena != 0)
            {

              /********************************************************/
              /* decrement the station's acknowledgement timer value  */
              /* in the station list.                                 */
              /********************************************************/

              p->station_list[n].t2_ctr--;
 
              if                       /* the timer value has         
                                          decremented to zero         */
                 (p->station_list[n].t2_ctr == 0)
                {

                  /****************************************************/
                  /* set the stationno to the current index;          */
                  /****************************************************/

                  p->stano = n;

                  /****************************************************/
                  /* call the link station with command               */
                  /* =t2_timeout_completion.                          */
                  /****************************************************/

                  lansta(p, T2_TIMEOUT_CMPLNO);
                } 
            } 

          /*----------------------------------------------------------*/
          /* inact/abort timer check                                  */
          /*----------------------------------------------------------*/
 

          if                           /* the stations                
                                          inactivity/abort timer is   
                                          enabled                     */
             (p->station_list[n].t3_ena != 0)
            {

              /********************************************************/
              /* decrement the station's inactivity/abort timer value */
              /* in the station list.                                 */
              /********************************************************/

              p->station_list[n].t3_ctr--;
 
              if                       /* the timer value has         
                                          decremented to zero         */
                 (p->station_list[n].t3_ctr == 0)
                {
                  p->stano = n;

                  /****************************************************/
                  /* call the link station with command=              */
                  /* t3_timeout_completion.                           */
                  /****************************************************/

                  lansta(p, T3_TIMEOUT_CMPLNO);
                } 
            } 
        } 
    }                                  /* end do n;                   */

  /*------------------------------------------------------------------*/
  /* sap add name timer check                                         */
  /*------------------------------------------------------------------*/
 

  if                                   /* there is at least one SAP   
                                          adding its name             */
     (p->common_cb.addn_ctr != 0)
    {

      /****************************************************************/
      /* for index = 0 to 127 maximum saps                            */
      /****************************************************************/
 

      for (n = 0; n < 128; n++)
        {
 
          if                           /* the sap's t1 add name repoll
                                          timer is enabled            */
             (p->sap_list[n].t1_ena != 0)
            {
#ifdef   DEBUG
 
              if (p->debug)
                printf("timer enabled n=%d\n", n);
#endif

              /********************************************************/
              /* decrement the sap's t1 repoll timer value in the sap */
              /* list.                                                */
              /********************************************************/

              p->sap_list[n].t1_ctr--;
 
              if                       /* the timer value has         
                                          decremented to zero         */
                 (p->sap_list[n].t1_ctr == 0)
                {
#ifdef   DEBUG
 
                  if (p->debug)
                    printf("timer 0 for %d\n", n);
#endif

                  /****************************************************/
                  /* get addressability to the sap at the current     */
                  /* index.                                           */
                  /****************************************************/

                  p->sapno = n;
                  p->sap_ptr = (struct sap_cb *)p->sap_list[p->sapno].
                     sap_cb_addr;
 
                  if                   /* the sap is not in use, ie.  
                                          the sap pointer is not valid*/
                     (p->sap_list[p->sapno].in_use == FALSE)
                    {

                      /************************************************/
                      /* ERROR: sap pointer is not valid              */
                      /************************************************/

                      lanerrlg(p, ERRID_LAN8011, NON_ALERT, 
                         PERM_PLC_ERR, 0, FILEN, LINEN);
                      return ;
                    } 
 
                  if                   /* this sap is adding the local
                                          name to the NETWORK         */
                     (p->sap_ptr->sap_state == ADDING_NAME)
                    {

                      /************************************************/
                      /* call the dlc manager add name timeout        */
                      /* completion routine.                          */
                      /************************************************/

                      add_name_timeout(p);
                    } 
                } 
            } 
        }                              /* end do n;                   */
    } 

/* LEHb defect 43788 */
  /*------------------------------------------------------------------*/
  /* sap local busy (network data) timer check                        */
  /*------------------------------------------------------------------*/
  if                                   /* there is at least one SAP
					  holding lbusy network data  */
     (p->common_cb.lbusy_ctr != 0)
    {
      /****************************************************************/
      /* for index = 0 to 127 maximum saps                            */
      /****************************************************************/
      for (n = 0; n < 128; n++)
	{
	  if                           /* the sap needs a wakeup       */
	     (p->sap_list[n].wakeup_needed == TRUE)
	    {
				       /* then reset the sap local
					  busy wakeup needed flag.     */
	    p->sap_list[n].wakeup_needed = FALSE;
				       /* decrement the local busy ctr */
	    p->common_cb.lbusy_ctr--;
				       /* set the sap pointer and sap
						      index number     */
	    p->sap_ptr = (struct sap_cb *)p->sap_list[n].sap_cb_addr;
	    p->sapno = n;

	    /*********************************************************/
	    /* call the user RCV network data routine                */
	    /*********************************************************/
	    c_ptr = p->sap_ptr->user_sap_channel;
	    p->rc = (*c_ptr->rcvn_fa)(p->sap_ptr->retry_rcvn_buf,
		     &(p->sap_ptr->retry_rcvn_ext), c_ptr);

	    switch (p->rc)
		{
		case  DLC_FUNC_OK :      /* normal case                */
					 /* reset the save buffer addr */
		   p->sap_ptr->retry_rcvn_buf = 0;
		   break;

		case  DLC_FUNC_RETRY :
					 /* note: leave the save buffer
						  address as is.      */
					 /* indicate that a sap wakeup
					    is still needed.          */
		   p->sap_list[p->sapno].wakeup_needed = TRUE;
					 /* bump the common local busy
					    counter.                  */
		   p->common_cb.lbusy_ctr++;
		   break;

		default  :
		   /* call error log - user network data routine failed */
		   lanerrlg(p, ERRID_LAN8086, NON_ALERT,
					    INFO_ERR, 0, FILEN, LINEN);
		   /* free the mbuf                                   */
		   m_freem (p->sap_ptr->retry_rcvn_buf);
					 /* reset the save buffer addr */
		   p->sap_ptr->retry_rcvn_buf = 0;
		}                        /* end switch rc             */
	    }                            /* end if wakeup needed      */
	}                                /* end do n                  */
    }                                    /* end if any lbusy netd     */
}

/* delete  manager busy timer check routine */
/* LEHe */

 /********************************************************************
 * NAME: add_name_timeout
 *
 * FUNCTION: Retransmits the Discovery Add name query packet
 *            up to 6 times before indicating the local name
 *            is unique if no name found response packet
 *            received.
 *
 * EXECUTION ENVIRONMENT: Operates in DLC process
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *      MODIFIED:
 *         addn_retries  -- Add Name Retries
 *         sap_state     -- SAP State
 *         m             -- Pointer to Packet Buffer
 *         t1_ctr        -- Timer T1 Repoll Counter
 *         find_self_echo -- Flag indicating packet received by sending
 *                            station
 *         find_self_addr -- Address of buffer containing packet
 *
 * RETURNS: None
 *
 **********************************************************************/

add_name_timeout(p)
  register struct port_dcl *p;
{
  struct ri_control_field *dp;         /* pointer to routing info     */
  ulong    dlctype;                    /* lan type                    */
  ulong_t  port_sta;                   /* station number and port     
                                          number                      */

  TRACE1(p, "tim1");

  /********************************************************************/
  /* setup values to record monitor data                              */
  /********************************************************************/

  dlctype = DLC_TRACE_TIMER<<8;
#ifdef   TRL
  dlctype |= DLC_TOKEN_RING;
#endif
#ifdef   FDL
  dlctype |= DLC_FDDI;
#endif
#ifdef   EDL
  dlctype |= DLC_ETHERNET;
#endif
#ifdef   E3L
  dlctype |= DLC_IEEE_802_3;
#endif

  /********************************************************************/
  /* get station number in upper half word and get number from port   */
  /* name in lower half word                                          */
  /********************************************************************/
/* <<< feature CDLI >>> */
#ifndef FDL
  port_sta = ((p->stano<<16)|p->dlc_port.namestr[3]);
#elif FDL
  port_sta = ((p->stano<<16)|p->dlc_port.namestr[4]);
#endif
/* <<< end feature CDLI >>> */

/* call trchkgt to record monitor data                                */

  trchkgt(HKWD_SYSX_DLC_MONITOR|dlctype, 0x01, 0, 0, 0, port_sta);

  /********************************************************************/
  /* decrement the add name query retries counter.                    */
  /********************************************************************/

  p->sap_list[p->sapno].addn_retries--;
 
  if                                   /* the number of retries has   
                                          reached the limit           */
     (p->sap_list[p->sapno].addn_retries <= 0)
    {

      /****************************************************************/
      /* disable the sap's t1 add name repoll timeout.                */
      /****************************************************************/

      p->sap_list[p->sapno].t1_ena = FALSE;
      p->sap_list[p->sapno].t1_ctr = -1;
 
      if                               /* the query buffer (new       
                                          discovery) has not already  
                                          been returned               */
         (p->sap_list[p->sapno].find_self_addr != 0)
        {

          /************************************************************/
          /* return the query command buffer to the pool.             */
          /************************************************************/

#ifdef   DEBUG
 
          if (p->debug)
            printf("free ptr=%x\n", p->sap_list[p->sapno].
               find_self_addr);
#endif
          lanfree(p, p->sap_list[p->sapno].find_self_addr);

          /************************************************************/
          /* set the query buffer address to zero to indicate that it */
          /* is returned.                                             */
          /************************************************************/

          p->sap_list[p->sapno].find_self_addr = 0;
        } 

      /****************************************************************/
      /* set the sap state = opened.                                  */
      /****************************************************************/

      p->sap_ptr->sap_state = SAP_OPEN_STATE;

      /****************************************************************/
      /* call the add name complete routine.                          */
      /****************************************************************/

      add_name_cmpl(p);
    } 
 
  else                                 /* the maximum query retries   
                                          have not been completed.    */
    {

      /****************************************************************/
      /* set the manager t1 timer for another repoll timeout.         */
      /****************************************************************/

      p->sap_list[p->sapno].t1_ctr = ADDN_RETRY_VAL;

      /****************************************************************/
      /* indicate that an add name echo will occur.                   */
      /****************************************************************/

      p->sap_list[p->sapno].find_self_echo = TRUE;

      /****************************************************************/
      /* call find_self_gen to build and send add local name find     */
      /* packet                                                       */
      /****************************************************************/

      find_self_gen(p);
    } 
} 


 /********************************************************************
 * NAME: add_name_cmpl
 *
 * FUNCTION: Notifies user that the physical link is open,
 *            following successful completion of add name query.
 *
 * EXECUTION ENVIRONMENT: Operates in DLC process
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *      MODIFIED:
 *         addn_ctr    -- Add Name Counter
 *         addn_pend   -- Add Name pending
 *         result_ind  -- I/O Extension Result indicator
 *         result_corr -- I/O Extension Result correlator
 *         result_code -- I/O Extension Result code
 *
 * RETURNS: None
 *
 **********************************************************************/

add_name_cmpl(p)
  register struct port_dcl *p;
{
  uint     rc;                         /* function handler return code*/
  struct dlc_chan *c_ptr;              /* pointer to channel          */
  struct dlc_getx_arg dlc_getx_arg;    /* function handler SAP OPENED 
                                          * return parameters area    */
  struct dlc_sape_res *q;              /* sap opened extension        */
  ulong    dlctype;                    /* lan type                    */
  ulong_t  port_sta;                   /* station number and port     
                                          number                      */
  char *p_to_grpbit;                   /* pointer to group address bit */

/* <<< feature CDLI >>> */
  char add_grp_arg[6];
/* <<< end feature CDLI >>> */

  /********************************************************************/
  /* setup values to record monitor trace                             */
  /********************************************************************/

  dlctype = DLC_TRACE_TIMER<<8;
#ifdef   TRL
  dlctype |= DLC_TOKEN_RING;
#endif
#ifdef   FDL
  dlctype |= DLC_FDDI;
#endif
#ifdef   EDL
  dlctype |= DLC_ETHERNET;
#endif
#ifdef   E3L
  dlctype |= DLC_IEEE_802_3;
#endif

  /********************************************************************/
  /* get station number in upper half word and get number from port   */
  /* name in lower half word                                          */
  /********************************************************************/
/* <<< feature CDLI >>> */
#ifndef FDL
  port_sta = ((p->stano<<16)|p->dlc_port.namestr[3]);
#elif FDL
  port_sta = ((p->stano<<16)|p->dlc_port.namestr[4]);
#endif
/* <<< end feature CDLI >>> */

/* call trchgkt to record monitor data                                */

  trchkgt(HKWD_SYSX_DLC_MONITOR|dlctype, 0x08, 0, 0, 0, port_sta);
#ifdef   DEBUG
 
  if (p->debug)
    printf("add name cmpl\n");
#endif
/* <<< feature CDLI >>> */
/* #ifndef  TRL */

  /********************************************************************/
  /* the INTEL chip does no echo back to itself                       */
  /********************************************************************/

  p->sap_list[p->sapno].find_self_echo = FALSE;
/* #endif */
/* <<< end feature CDLI >>> */
#ifdef   DEBUG
 
  if (p->debug)
    printf("sapno=%x flag=%d addr=%x\n", p->sapno, p->sap_list
       [p->sapno].find_self_echo, &(p->sap_list[p->sapno].
       find_self_echo));
#endif
 
  if                                   /* no "add name" echo is still 
                                          pending receive (ie. the    
                                          adapter                     */

  /********************************************************************/
  /* interrupts are working, or the name was already added)           */
  /********************************************************************/

     (p->sap_list[p->sapno].find_self_echo == FALSE)
    {

/* <<< feature CDLI >>> */
#ifdef   TRL
    /******************************************************************/
    /* send any functional addresses to the device handler            */
    /******************************************************************/

    if /* there is a functional address in the sap profile */
       (p->sap_ptr->sap_profile.len_func_addr_mask != 0)
      {
      /* call the token ring set functional address routine with the "add"
	 parameter and the functional address from the sap profile */

      trl_setfa(p, DLC_ADD_FUNC_ADDR,
		p->sap_ptr->sap_profile.len_func_addr_mask,
		&p->sap_ptr->sap_profile.func_addr_mask);
      }
#endif /* TRL */

    /******************************************************************/
    /* send any group addresses to the device handler                 */
    /******************************************************************/

    if /* there is a group address in the sap profile */
       (p->sap_ptr->sap_profile.len_grp_addr != 0)
      {
	/* note: the input group addr length is already validated to 6 */

	/* build the 6-byte add_grp_arg for the ndd_ctl call */
	bcopy(&p->sap_ptr->sap_profile.grp_addr, &add_grp_arg[0], 6);

#ifdef   TRL
	/* set group address bit on (ie. non-functional addr) */
	add_grp_arg[2] |= 0x80;
#endif /* TRL */

	/* issue ndd enable address to the device driver */
	rc = (*p->nddp->ndd_ctl)(p->nddp, NDD_ENABLE_ADDRESS,
							 &add_grp_arg, 6);
	if (rc == 0)
	  {
	    /* call lladd to add address to list of valid group addresses
	       for this sap */
	    lladd(&add_grp_arg, p);

	    if /* the add to address list failed */
	       (rc != 0)
	      {
		/* Note: already error logged in lladd routine */

		/* issue ndd disable address to the device driver,
		   and ignore any return code */
		rc = (*p->nddp->ndd_ctl)(p->nddp, NDD_DISABLE_ADDRESS,
							 &add_grp_arg, 6);
	      }
	  }

	else /* ndd add group address failed */
	  {
	  /* call error log - adapter group address not working.
	     sap shutdown = unusual NETWORK condition. */

	  lanerrlg(p, ERRID_LAN8045, NON_ALERT, PERM_SAP_ERR,
					 DLC_SAP_NT_COND, FILEN, LINEN);
	  }
      }
/* <<< end feature CDLI >>> */

      /****************************************************************/
      /* build the "sap opened" result. setup sap correlator in sap   */
      /* opened extension area                                        */
      /****************************************************************/

#ifdef   DEBUG
 
      if (p->debug)
        printf("user sap corr=%d\n", 
           p->sap_ptr->sap_profile.user_sap_corr);
#endif
      dlc_getx_arg.user_sap_corr =
	 p->sap_ptr->sap_profile.user_sap_corr;

      /****************************************************************/
      /* setup SAP OPENED result in sap opened extension area         */
      /****************************************************************/

      dlc_getx_arg.result_ind = DLC_SAPE_RES;
      dlc_getx_arg.user_ls_corr = 0;
      dlc_getx_arg.result_code = 0;

      /****************************************************************/
      /* setup address to sap opened extension                        */
      /****************************************************************/

      q = (struct dlc_sape_res *)&dlc_getx_arg.result_ext[0];
#ifndef  TRLORFDDI
      q->max_net_send = MAX_PACKET-NORM_HDR_LENGTH;
      p->common_cb.maxif = MAX_PACKET-NORM_HDR_LENGTH;
#endif
#ifdef   TRLORFDDI
      q->max_net_send = MAX_PACKET;
      p->common_cb.maxif = MAX_PACKET-NORM_HDR_LENGTH;
#endif
      q->lport_addr_len = 6;
      bcopy(p->common_cb.local_addr, q->lport_addr, 6);

      /****************************************************************/
      /* call the exception handler to return sap opened data         */
      /****************************************************************/

      c_ptr = p->sap_ptr->user_sap_channel;
      TRACE3(p, "XFAb", DLC_SAPE_RES, 0);
      rc = (*c_ptr->excp_fa)(&dlc_getx_arg, 
         p->sap_ptr->user_sap_channel);
      TRACE1(p, "XFAe");
#ifdef   DEBUG
 
      if (p->debug)
        printf("return from exception routine rc=%x\n", rc);
#endif
    } 
 
  else                                 /* error - no "add name" echoes
                                          are being received.         */
    {
#ifdef   DEBUG
 
      if (p->debug)
        printf("add name errlog\n");
#endif

      /****************************************************************/
      /* call error log - adapter receive function not working. sap   */
      /* shutdown = unusual NETWORK condition.                        */
      /****************************************************************/

      lanerrlg(p, ERRID_LAN8016, NON_ALERT, PERM_SAP_ERR, 
      DLC_SAP_NT_COND, FILEN, LINEN);
    } 

  /********************************************************************/
  /* decrement the common counter for SAPs adding their name          */
  /********************************************************************/

  p->common_cb.addn_ctr--;

  /********************************************************************/
  /* reset the add name pending indicator in the sap list.            */
  /********************************************************************/

  p->sap_list[p->sapno].addn_pend = FALSE;
}

/* LEHb defect 43788 */
/* delete local_busy_timeout routine */
/* LEHe */

 /********************************************************************
 * NAME: mgr_call_timeout
 *
 * FUNCTION: Handles the repolling of connect_out "find_name"
 *            and closes the logical link if retries exceeded.
 *
 * EXECUTION ENVIRONMENT: Operates in DLC process
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *      MODIFIED:
 *          sapno    -- SAP Number
 *          p->sta_ptr  -- Pointer to Station
 *          p->sap_ptr  -- Pointer to SAP
 *          call_retries -- number of times packet sent
 *          call_pend  -- A call is waiting for a response
 *          t1_ena     -- T1 (Repoll) Timer
 *          t1_ctr     -- T1 (Repoll) Timer Counter
 *          m          -- Pointer to buffe containing packet
 *
 * RETURNS: None
 *
 **********************************************************************/

mgr_call_timeout(p)
  register struct port_dcl *p;         /* pointer to port control     
                                          block                       */
{
  struct ri_control_field *dp;         /* pointer to routing info     */
  ulong    dlctype;                    /* lan type                    */
  ulong_t  port_sta;                   /* station number and port       
                                        number                       */
#ifdef   DEBUG
 

  if (p->debug)
    printf("mgr_call_timeout\n");
#endif

  /********************************************************************/
  /* setup values to record monitor data                              */
  /********************************************************************/

  dlctype = DLC_TRACE_TIMER<<8;
#ifdef   TRL
  dlctype |= DLC_TOKEN_RING;
#endif
#ifdef   FDL
  dlctype |= DLC_FDDI;
#endif
#ifdef   EDL
  dlctype |= DLC_ETHERNET;
#endif
#ifdef   E3L
  dlctype |= DLC_IEEE_802_3;
#endif

  /********************************************************************/
  /* get station number in upper half word and get number from port   */
  /* name in lower half word                                          */
  /********************************************************************/
/* <<< feature CDLI >>> */
#ifndef FDL
  port_sta = ((p->stano<<16)|p->dlc_port.namestr[3]);
#elif FDL
  port_sta = ((p->stano<<16)|p->dlc_port.namestr[4]);
#endif
/* <<< end feature CDLI >>> */

/* call trchkgt to record monitor data                                */

  trchkgt(HKWD_SYSX_DLC_MONITOR|dlctype, 0x03, 0, 0, 0, port_sta);

  /********************************************************************/
  /* get addressability to the station's control block.               */
  /********************************************************************/

  p->sta_ptr = (struct station_cb *)p->station_list[p->stano].
     sta_cb_addr;

  /********************************************************************/
  /* get addressability to the sap control block.                     */
  /********************************************************************/

  p->sapno = p->station_list[p->stano].sapnum;
  p->sap_ptr = (struct sap_cb *)p->sap_list[p->sapno].sap_cb_addr;

  /********************************************************************/
  /* decrement the call retries counter.                              */
  /********************************************************************/

#ifdef   DEBUG
 
  if (p->debug)
    printf("retries=%d\n", p->station_list[p->stano].call_retries);
#endif
  p->station_list[p->stano].call_retries--;
 
  if                                   /* the number of retries has   
                                          reached the limit           */
     (p->station_list[p->stano].call_retries == 0)
    {
 
      if                               /* discovery and remote name in
                                          cache                       */
	 (((p->sta_ptr->ls_profile.flags&DLC_SLS_ADDR) != DLC_SLS_ADDR)
	   && ((p->sta_ptr->sta_cache&CACHE_NAME) == CACHE_NAME))
        {

          /************************************************************/
          /* indicate cache entry contains wrong address              */
          /************************************************************/

          p->sta_ptr->sta_cache = CACHE_WRONG_NAME;

          /************************************************************/
          /* call discovery to resent packet using a broadcast address*/
          /************************************************************/

          landiscv(p, CALLNO);
        } 
 
      else
        {
 
          if                           /* not using all route         
                                          broadcast to find remote    
                                          station                     */
             (p->station_list[p->stano].discv_allr == FALSE)
            {

              /********************************************************/
              /* indicate using all route broadcast in discovery      */
              /********************************************************/

              p->station_list[p->stano].discv_allr = TRUE;

              /********************************************************/
              /* call discovery to resend packet using a broadcast    */
              /* address                                              */
              /********************************************************/

              landiscv(p, CALLNO);
            } 
 
          else
            {

              /********************************************************/
              /* disable the station's t1 timer.                      */
              /********************************************************/

              p->station_list[p->stano].t1_ena = FALSE;

              /********************************************************/
              /* reset the call pending indicator in the station list */
              /* and the receive station table.                       */
              /********************************************************/

              p->station_list[p->stano].call_pend = FALSE;
	      p->common_cb.hashno = p->station_list[p->stano].sta_hash;
              p->rcv_sta_tbl[p->common_cb.hashno].call_pend = FALSE;

              /********************************************************/
              /* call error log - "cannot find remote"                */
              /********************************************************/

              lanerrlg(p, ERRID_LAN0026, NON_ALERT, PERM_STA_ERR, 
                 DLC_NO_FIND, FILEN, LINEN);

              /********************************************************/
              /* call the link station with command = close.          */
              /********************************************************/

#ifdef   DEBUG
 
              if (p->debug)
                printf("call lansta close\n");
#endif
              lansta(p, CLOSENO);
            } 
        } 
    } 
 
  else                                 /* the maximum call retries    
                                          have not been completed.    */
    {
#ifdef   DEBUG
 
      if (p->debug)
        printf("retry call\n");
#endif

      /****************************************************************/
      /* set the timer t1 counter to the station's response timeout   */
      /* value.                                                       */
      /****************************************************************/

      p->station_list[p->stano].t1_ctr = p->sta_ptr->resp_to_val;
      find_remote_gen(p);
    } 

  /********************************************************************/
  /* end of mgr_call_timeout                                          */
  /********************************************************************/

} 


/* loopback                                                           */
/**********************************************************************/
/* function name: post_stares                                         */
/* description:  this function simulates a listen                     */
/*   or call completion.  It formats the
   getx_arg structure and puts the
   address of the structure into an mbuf
   which it then puts on the loopback
   ring queue--then it posts the linkmgr
   to simulate an adapter interrupt                                   */
/**********************************************************************/

post_stares(p)
  register struct port_dcl *p;
{
  struct dlc_getx_arg *getx;
  struct mbuf *buf;
  struct dlc_stas_res *q;
  struct que_entry rq_data;

  TRACE1(p, "PoSb");
 
  if ((ulong_t)(buf = (struct mbuf *)lanfetch(p)) == NO_BUF_AVAIL)
    {
      lanerrlg(p, ERRID_LAN8010, NON_ALERT, PERM_STA_ERR, DLC_LS_ROUT,
         FILEN, LINEN);
      return (-1);
    } 
  getx = MTOD(buf, struct dlc_getx_arg *);
  getx->result_ind = DLC_STAS_RES;
  getx->user_sap_corr = p->sap_ptr->sap_profile.user_sap_corr;
  getx->user_ls_corr = p->sta_ptr->ls_profile.user_ls_corr;
  getx->result_code = (int)p->sap_ptr->user_sap_channel;

  /********************************************************************/
  /* return the extension portion                                     */
  /********************************************************************/

  q = (struct dlc_stas_res *)&(getx->result_ext[0]);
  q->maxif = p->sta_ptr->ls_profile.maxif;
  q->rport_addr_len = 6;

  /********************************************************************/
  /* hard code the remote address to be our local address             */
  /********************************************************************/

  bcopy(p->common_cb.local_addr, q->rport_addr, q->rport_addr_len);
  q->rname_len = p->sta_ptr->ls_profile.len_raddr_name;
  bcopy(p->sta_ptr->ls_profile.raddr_name, q->rname, q->rname_len);
  q->rsap = p->sta_ptr->ls_profile.rsap;
  q->max_data_off = 0;
  rq_data.entry[0] = DLC_STAS_RES;
  rq_data.entry[1] = (ulong)buf;
 
  if (dlc_rqput(p->loopback.loopback_rq, &rq_data) != 0)
    {
      lanfree(p, buf);
      return (-1);
    } 
/* <<< THREADS >>> */
  et_post(ECB_LOCAL, p->dlc_port.kproc_tid);
/* <<< end THREADS >>> */

 
  if                                   /* link trace is enabled       */
     ((p->sta_ptr->ls_profile.flags&DLC_TRCO) == DLC_TRCO)

    /******************************************************************/
    /* call the session trace routine (open, length).                 */
    /******************************************************************/

    {
      session_trace(p, HKWD_SYSX_DLC_START, 0);
    } 
 
  if                                   /* t3 timer state = inactivity */
     (p->sta_ptr->t3_state == T3_INACT)
    {
      p->station_list[p->stano].t3_ctr = p->sta_ptr->inact_to_val;
      p->station_list[p->stano].t3_ena = TRUE;
    } 

#ifdef   TRLORFDDI
/* LEHb defect XXX */
/* deleted phy_ctl1 and phy_ctl2 and associated pkt_prty */
/* LEHe */

  /********************************************************************/
  /* save routing information length                                  */
  /********************************************************************/

  p->sta_ptr->ri_length = p->common_cb.ri_length;
 
  if                                   /* routing information field   
                                          present                     */
     (p->common_cb.ri_length > 0)
    {

      /****************************************************************/
      /* save routing information in control block                    */
      /****************************************************************/

/* LEHb defect 44499 */
      bcopy(&(p->common_cb.ri_field), &(p->sta_ptr->ri_field),
	 p->common_cb.ri_length);
/* LEHe */
    } 
#endif                                 /* TRLORFDDI                   */
TRACE1(p, "SoPe");
}                                      /* post_stares                 */

/* <<< feature CDLI >>> */
local_write(p,in_addr)
  struct port_dcl *p;
  struct mbuf *in_addr;
/* <<< removed in_length >>> */
/* <<< end feature CDLI >>> */
{
  struct que_entry rq_data;
  int      rc;
  struct mbuf *new_buf;
  struct STA_CMD_BUF *sta_cmd_buf;
  struct ri_sd *ri;

  TRACE3(p, "LOWR", in_addr, p->stano);

/* another major hack-- for all local writes, we have already
   established a link station between our two local link
   stations.  in order to imitate normal data trasnmission,
   we had to setup two unique hash table entries:  we did
   this by using the normal sap numbers, but for one station, 
   we changed the remote address to zero, and the other station,
   we used our local address--- this let us have two unique
   entries in the hash table -- so now one station thinks
   that its remote address is the local address while the other
   station thinks that *its* remote address is 0.
 the gist of it all is that all network packets are built
 using our local network hardware address--- we need
 to change this to a local address of 0 for all packets
 sent from only one of the link stations (the other station's
 packets are fine)
 so, we need to go through the mbuf and look at the
 remote address of this packet-- if the remote address
 is 0, then we leave the packet alone--however if
 the remote address is our local address, then we know
 that we need to change our local address for this
 packet to 0                                                          */

/* <<< feature CDLI >>> */
/* <<< removed  if (options&CIO_NOFREE_MBUF) >>> */

#ifdef TRLORFDDI
  sta_cmd_buf = MTOD(in_addr, struct STA_CMD_BUF *);
  p->d.rcv_data = (struct rcv_data *)sta_cmd_buf;
  ri = (struct ri_sd *)&p->d.rcv_data->lsap;
  /* determine the length of the routing information field */
  if( TSTBIT( p->d.rcv_data->raddr[0], RI_PRESENT ) == TRUE )
	  rc = (p->d.rcv_data->ri_field[0] & 0x1f);
  else
	  rc = 0;
  bcopy(&ri->rsap, &p->d.rcv_data->lsap, in_addr->m_len - ROUTING_OFFSET);
  in_addr->m_len += rc; /* include the length of the routing information */
#endif
/* <<< end feature CDLI >>> */

  /********************************************************************/
  /* now another kludge -- normally we can write to the device driver */
  /* using chained mbufs -- which is what we always do.. however, when*/
  /* the device driver sends us an mbuf, it is a single cluster mbuf  */
  /* --- it is never chained -- so now we need to convert our chain to*/
  /* a single cluster mbuf so we can read it later                    */
  /********************************************************************/

  new_buf = (struct mbuf *)m_collapse(in_addr, 1);
 
  if (new_buf == NULL)
    {
      return (ENOMEM);
    } 
  TRACE2(p, "LOco", new_buf);
  rq_data.entry[0] = DLC_RCV;
  rq_data.entry[1] = (ulong)new_buf;
  rc = dlc_rqput(p->loopback.loopback_rq, &rq_data);
 
  if (rc != 0)
    {
      lanfree(p, new_buf);
      return (EAGAIN);
    } 
/* <<< THREADS >>> */
  et_post(ECB_LOCAL, p->dlc_port.kproc_tid);
/* <<< end THREADS >>> */

 
/* <<< feature CDLI >>> */
/* removed  if (options&CIO_ACK_TX_DONE) */
/* <<< end feature CDLI >>> */

} 


/* loopback                                                           */

