static char sccsid[] = "@(#)03  1.28.1.11  src/bos/kernext/dlc/lan/lanmisc.c, sysxdlcg, bos41J, 9516B_all 4/14/95 18:01:50";

/*
 * COMPONENT_NAME: SYSXDLCG
 *
 * FUNCTIONS: lanmisc.c
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
                                                                      */

#include <fcntl.h>
#include <sys/types.h>
#include <sys/fp_io.h>
#include <net/spl.h>
#include <sys/mbuf.h>
#include <sys/file.h>
#include <sys/malloc.h>
#include <sys/errno.h>
#include "dlcadd.h"
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
#include "lanstlst.h"
#include "lansplst.h"
#include "lanrstat.h"
#include "lanport.h"
static int xcnt = 0;
int TSTBIT(field,b)
  int field,b;
{
  return ((field&(b))?1:0);
} 

/* defect 153969 */
dlc_trace(p,num,p1,p2,p3,p4)
  register struct port_dcl *p;
  register num;
  char     *p1;
  ulong    p2,p3,p4;
{
 
  if (num > 4)
    {
      /* Too many args */
      lanerrlg(p, ERRID_LAN8011, NON_ALERT, INFO_ERR, DLC_ERR_CODE, 
         FILEN, LINEN);
      /* force it to 4 args */
      num = 4;
    } 

  bcopy(p1, p->common_cb.tptr, 4);
  p->common_cb.tptr++;
 
  switch (num)
    {
      case 1 :
        *(p->common_cb.tptr)++ = 0;
        *(p->common_cb.tptr)++ = 0;
        *(p->common_cb.tptr)++ = 0;
        break;
      case 2 :
        *(p->common_cb.tptr)++ = p2;
        *(p->common_cb.tptr)++ = 0;
        *(p->common_cb.tptr)++ = 0;
        break;
      case 3 :
        *(p->common_cb.tptr)++ = p2;
        *(p->common_cb.tptr)++ = p3;
        *(p->common_cb.tptr)++ = 0;
        break;
      case 4 :
        *(p->common_cb.tptr)++ = p2;
        *(p->common_cb.tptr)++ = p3;
        *(p->common_cb.tptr)++ = p4;
        break;
    } 
  if (p->common_cb.tptr >= p->common_cb.tend)
    p->common_cb.tptr = p->common_cb.top;
 
} 
/* end defect 153969 */

lanerrlg(p,in_error,alert_flag,err_type,kill_rc,fileptr,line)
  register struct port_dcl *p;

/*** start of specifications ******************************************/
/*                                                                    */
/* module name:  lanerrlg                                             */
/*                                                                    */
/* descriptive name:  error log                                       */
/*                                                                    */
/* function:  logs the error entry to system error log                */
/*                                                                    */
/* input:  1) cause code                                              */
/*         2) error code                                              */
/*         3) additional status                                       */
/*         4) type of error (perm_plc, perm_sap, perm_sta, temp,info) */
/*         5) shutdown return code(or none)                           */
/*         6) common port declarations                                */
/*                                                                    */
/* output:  error log entry                                           */
/**********************************************************************/

  ulong    in_error;                   /* type of error encountered   */
  int      alert_flag;
  ushort   err_type;                   /* error type                  */
  int      kill_rc;                    /* shutdown return code        */
  char     *fileptr;
  int      line;
{
#ifdef   DEBUG
 
  if (p->debug)
    printf("LANERRLG %x\n", in_error);
#endif

  /********************************************************************/
  /* call the generic build errlog top portion routine                */
  /********************************************************************/

  bld_err_top(p, in_error, fileptr, line);
 
  if (alert_flag == ALERT)

    /******************************************************************/
    /* call the generic build alert detail data routine returns the   */
    /* log entry's next pointer location                              */
    /******************************************************************/

    alert_detail_data(p);

  /********************************************************************/
  /* call the errlog                                                  */
  /********************************************************************/

  lan_log(p);
 
  switch                               /* the error type              */
     (err_type)
    {
      case                             /* the log entry is a permanent
                                          physical link error.        */
         (PERM_PLC_ERR) :
          {

            /**********************************************************/
            /* set the common plc return code as indicated.           */
            /**********************************************************/

            p->common_cb.plc_retcode = kill_rc;
          } 
        break;
      case                             /* the log entry is a permanent
                                          sap error.                  */
         (PERM_SAP_ERR) :
          {

            /**********************************************************/
            /* set the sap return code as indicated.                  */
            /**********************************************************/

            p->sap_ptr->sap_retcode = kill_rc;

            /**********************************************************/
            /* set the common sap return code as indicated.           */
            /**********************************************************/

            p->common_cb.sap_retcode = kill_rc;
          } 
        break;
      case                             /* the log entry is a permanent
                                          link station error.         */
         (PERM_STA_ERR) :
          {

            /**********************************************************/
            /* set the station closing reason code as indicated.      */
            /**********************************************************/

            p->sta_ptr->closing_reason = kill_rc;
          } 
        break;
    } 
} 

lanfetch(p)
  register struct port_dcl *p;

/*** start of specifications ******************************************/
/*                                                                    */
/* module name:  lanfetch                                             */
/*                                                                    */
/* descriptive name:  fetch a buffer from the pool.                   */
/*                                                                    */
/* <<< feature CDLI >>> */
/* function:  obtains an mbuf of type=header from the kernel heap     */
/* <<< end feature CDLI >>> */
/*                                                                    */
/* input:  none                                                       */
/*                                                                    */
/* output:  buffer address is returned, or indication that none are   */
/*          available.                                                */
/*                                                                    */
/*** end of specifications ********************************************/

/* <<< feature CDLI >>> */
{
  struct mbuf *m0;

  /********************************************************************/
  /* Issue an m_gethdr from the mbuf pool.                            */
  /* Note: all mbuf chains must now begin with a "header" mbuf        */
  /********************************************************************/
  m0 = (struct mbuf *)m_gethdr(M_WAIT, MT_HEADER);
/* <<< end feature CDLI >>> */
 
  if (m0 == NULL)
    {
      lanerrlg(p, ERRID_LAN8010, NON_ALERT, INFO_ERR, DLC_SYS_ERR, 
         FILEN, LINEN);

      /****************************************************************/
      /* panic("Unable to get MBUF");                                 */
      /****************************************************************/

      return (NO_BUF_AVAIL);
    } 

  /********************************************************************/
  /* return from call with buffer address.                            */
  /********************************************************************/

  return ((int)m0);
}                                      /* end lanfetch;               */
lanfree(p,buf_addr)
  register struct port_dcl *p;

/*** start of specifications ******************************************/
/*                                                                    */
/* module name:  lanfree                                              */
/*                                                                    */
/* descriptive name:  free a buffer to the pool.                      */
/*                                                                    */
/* function:  returns a block i/o buffer to the proper buffer pool    */
/*                                                                    */
/* input:  buffer address                                             */
/*                                                                    */
/* output:  none                                                      */
/*                                                                    */
/*** end of specifications ********************************************/

  char     *buf_addr;
{
  TRACE2(p, "FREE", buf_addr);
 
  if                                   /* the buffer address is not   
                                          zero or negative            */
     ((long)buf_addr > 0)
    {

      /****************************************************************/
      /* return the buffer to the pool.                               */
      /****************************************************************/

      m_freem(buf_addr);
    } 
}                                      /* end lanfree;                */
lanrcvtr(p)
  register struct port_dcl *p;

/*** start of specifications ******************************************/
/*                                                                    */
/* module name:  lanrcvtr                                             */
/*                                                                    */
/* descriptive name:  receive link trace                              */
/*                                                                    */
/* function:  logs the system trace entry for a receive packet using  */
/*            generic trace.                                          */
/*                                                                    */
/* input:  received packet                                            */
/*                                                                    */
/* output:  trace entry                                               */
/*                                                                    */
/*** end of specifications ********************************************/

{
  ulong    dlctype;                    /* lan type                    */
  ulong    trace_length;               /* trace data length           */
  char     *tp;                        /* trace data pointer          */

/* setup generic trace hook id for specific lan                       */

#ifdef   TRL
  dlctype = DLC_TOKEN_RING;
#endif
#ifdef   FDL
  dlctype = DLC_FDDI;
#endif
#ifdef   EDL
  dlctype = DLC_ETHERNET;
#endif
#ifdef   E3L
  dlctype = DLC_IEEE_802_3;
#endif

  /********************************************************************/
  /* trace actual amount of data in packet                            */
  /********************************************************************/

  trace_length = p->m->m_len;
 
  if                                   /* short trace os specified and
                                          the data is longer than 80  
                                          bytes                       */
     (((p->sta_ptr->ls_profile.flags&DLC_TRCL) != DLC_TRCL) && 
     (trace_length > TRACE_SHORT_SIZE))

    /******************************************************************/
    /* set trace length size of data in buffer                        */
    /******************************************************************/

    trace_length = TRACE_SHORT_SIZE;

  /********************************************************************/
  /* get address of data                                              */
  /********************************************************************/

  tp = MTOD(p->m, char *);

  /********************************************************************/
  /* call trcgenkt to record generic trace entry                      */
  /********************************************************************/

  trcgenkt(p->sta_ptr->ls_profile.trace_chan, HKWD_SYSX_DLC_RECV, 
     dlctype, trace_length, tp);
}                                      /* end lanrcvtr;               */
lantoltr(p,input_to_type)
  register struct port_dcl *p;

/*** start of specifications **************************************** */
/*                                                                    */
/* module name:  lantoltr                                             */
/*                                                                    */
/* descriptive name:  timer link trace                                */
/*                                                                    */
/* function:  logs the system trace entry for a specified timeout     */
/*            using generic trace.                                    */
/*                                                                    */
/* input:  type of timeout                                            */
/*                                                                    */
/* output:  trace entry                                               */
/*                                                                    */
/*** end of specifications ****************************************** */

  int      input_to_type;
{
  ulong    dlctype;                    /* lan type                    */

/* setup generic hook id for a specific lan                           */

#ifdef   TRL
  dlctype = (DLC_TOKEN_RING<<16)|input_to_type;
#endif
#ifdef   FDL
  dlctype = (DLC_FDDI<<16)|input_to_type;
#endif
#ifdef   EDL
  dlctype = (DLC_ETHERNET<<16)|input_to_type;
#endif
#ifdef   E3L
  dlctype = (DLC_IEEE_802_3<<16)|input_to_type;
#endif

/* call trcgenkt to record timer generic trace entry                  */

  trcgenkt(p->sta_ptr->ls_profile.trace_chan, HKWD_SYSX_DLC_TIMER, 
  dlctype, 0, p->m);
}                                      /* end lantoltr;               */
lantrgen(p)
  register struct port_dcl *p;

/*** start of specifications ******************************************/
/*                                                                    */
/* module name:  lantrgen                                             */
/*                                                                    */
/* descriptive name:  test response generator                         */
/*                                                                    */
/* function:  generates and enqueues the test response packet.        */
/*                                                                    */
/* input:  test command received                                      */
/*                                                                    */
/* output:  test response                                             */
/*                                                                    */
/*** end of specifications ********************************************/

{

  /********************************************************************/
  /* call protocol specific test response gen                         */
  /********************************************************************/

  g_trgen(p);
}                                      /* end lantrgen;               */
lantxtr(p,in_buffer)
  register struct port_dcl *p;

/*** start of specifications ******************************************/
/*                                                                    */
/* module name:  lantxtr                                              */
/*                                                                    */
/* descriptive name:  transmit link trace                             */
/*                                                                    */
/* function:  logs the system trace entry for a transmit packet       */
/*            using generic trace.                                    */
/*                                                                    */
/* input:  packet to be transmitted                                   */
/*                                                                    */
/* output:  trace entry                                               */
/*                                                                    */
/*** end of specifications ********************************************/

  struct mbuf *in_buffer;
{
  ulong    dlctype;                    /* lan type                    */
  ulong    trace_len;                  /* size of trace data          */
  char     *trace_ptr;                 /* trace data pointer          */

/* setup generic trace hook id for the specific lan                   */

/* defect 127727 */
/* removed #define  DATA_ONLY */
/* iend defect 127727 */

#ifdef   TRL
  dlctype = DLC_TOKEN_RING;
#endif
#ifdef   FDL
  dlctype = DLC_FDDI;
#endif
#ifdef   EDL
  dlctype = DLC_ETHERNET;
#endif
#ifdef   E3L
  dlctype = DLC_IEEE_802_3;
#endif

  /********************************************************************/
  /* get addressability to the first buffer's data                    */
  /********************************************************************/

  trace_ptr = MTOD((struct mbuf *)in_buffer, char *);

  /********************************************************************/
  /* get the length of the first buffer's data                        */
  /********************************************************************/

  trace_len = (ulong)in_buffer->m_len;
 
  if                                   /* short trace is specified and
                                          the data is longer than 80  */
     (((p->sta_ptr->ls_profile.flags&DLC_TRCL) == 0) && (trace_len > 
     TRACE_SHORT_SIZE))

    /******************************************************************/
    /* set the trace length to 80 bytes                               */
    /******************************************************************/

    trace_len = TRACE_SHORT_SIZE;

  /********************************************************************/
  /* call trcgenkt to record a generic trace entry                    */
  /********************************************************************/

  trcgenkt(p->sta_ptr->ls_profile.trace_chan, HKWD_SYSX_DLC_XMIT, 
     dlctype, trace_len, trace_ptr);
 
  if                                   /* there is a chained buffer   */
     ((struct mbuf *)in_buffer->m_next != 0)
    {

      /****************************************************************/
      /* get addressability to the next buffer's data                 */
      /****************************************************************/

      trace_ptr = MTOD((struct mbuf *)in_buffer->m_next, char *);

      /****************************************************************/
      /* get the length of the next buffer's data                     */
      /****************************************************************/

      trace_len = (ulong)in_buffer->m_next->m_len;
 
      if                               /* short trace is specified and
                                          the data is longer than 80  */
         (((p->sta_ptr->ls_profile.flags&DLC_TRCL) == 0) && (trace_len
         > TRACE_SHORT_SIZE))

        /**************************************************************/
        /* set the trace length to 80 bytes                           */
        /**************************************************************/

        trace_len = TRACE_SHORT_SIZE;

      /****************************************************************/
      /* call trcgenkt to record a generic trace entry for "data only"*/
      /****************************************************************/
/* defect 127727 */
      trcgenkt(p->sta_ptr->ls_profile.trace_chan, HKWD_SYSX_DLC_XMIT, 
         DLC_DL_DATA_ONLY, trace_len, trace_ptr);
/* end defect 127727 */
    }                                  /* endif - chained buffer      */
}                                      /* end lantxtr;                */
/* feature CDLI */
extern struct mbstat mbstat;
lanwrtsg(p, buf)
  register struct port_dcl *p;

/*** start of specifications ******************************************/
/*                                                                    */
/* module name:  lanwrtsg                                             */
/*                                                                    */
/* descriptive name:  write send command generator                    */
/*                                                                    */
/* function:  issues the ndd_output call to the device handler, and   */
/*            insures that the first mbuf of a chain is type=header   */
/*                                                                    */
/* input:  mbuf address                                               */
/*                                                                    */
/* output: 1) link and performance trace entries.                     */
/*         2) mbuf chain sent to the device handler.                  */
/*                                                                    */
/*** end of specifications ********************************************/

  struct mbuf *buf;                    /* input mbuf address          */
{
  struct mbuf *tempbuf;                /* temporary mbuf pointer      */
  ulong    dlctype;                    /* trace lan type              */
  ulong    info;                       /* trace information           */
  ulong    port_sta;                   /* trace station and port #s   */

/* defect 82006     */
#define NO_SAP_CB  0                  /* no SAP control block present */
/* end defect 82006  */

  int      rc;
  char     *t_ptr;                     /* temp routing info pointer   */
  struct send_nbuf_data *datap;        /* pointer to packet data      */
  struct ri_control_field *p_to_ri;    /* pointer to routing info     */

  /********************************************************************/
  /* Insure that the first mbuf in the chain is of type=header.       */
  /********************************************************************/
  if /* the mbuf is not of type=header */
     (buf->m_type != MT_HEADER)
    {
      /* change the mbuf to type=header */
      MCHTYPE(buf, MT_HEADER);
    }

  /********************************************************************/
  /* Insure that the total chain length is set correctly              */
  /********************************************************************/
  buf->m_pkthdr.len = 0;                  /* initialize total length  */
  tempbuf = buf;                          /* current buf = chain head */

  while /* there are mbufs in the chain */
    (tempbuf != (struct mbuf *) NULL)
    {
      buf->m_pkthdr.len += tempbuf->m_len;       /* add in each m_len */
      tempbuf = tempbuf->m_next;                 /* goto next mbuf    */
    };

  /********************************************************************/
  /* Get addressability to the packet data in the mbuf                */
  /********************************************************************/
  datap = MTOD((struct mbuf *)buf, struct send_nbuf_data *);
  p->d.data_ptr = (char *)datap;
 
#ifdef FDL
				       /* clear the fddi prefix area  */
  p->d.data_ptr[0] = 0;
  p->d.data_ptr[1] = 0;
  p->d.data_ptr[2] = 0;
#endif                                 /* FDL */

  if                                   /* station number is valid, ie.
                                          a station is doing the      
                                          transmit                    */
     (p->stano != NO_MATCH)
    {
 
      if                               /* link trace enabled          */
         ((p->sta_ptr->ls_profile.flags&DLC_TRCO) == DLC_TRCO)

        /**************************************************************/
        /* call lantxtr to record link trace data                     */
        /**************************************************************/

	lantxtr(p, buf);
    }                                  /* endif - station number is   
                                          valid                       */

  /********************************************************************/
  /* setup lan and monitor types for performance trace                */
  /********************************************************************/

  dlctype = DLC_TRACE_SNDC<<8;
#ifdef   TRLORFDDI
#ifdef TRL
  dlctype |= DLC_TOKEN_RING;
#elif FDL
  dlctype |= DLC_FDDI;
#endif
  p->ri.ri_sd = (struct ri_sd *)p->d.send_data->ri_field;
 
  if                                   /* routing information present */
     (TSTBIT(p->d.send_data->laddr[0], RI_PRESENT) == TRUE)
    {

      /****************************************************************/
      /* get apddress to routing information data                      */
      /****************************************************************/

      p_to_ri = (struct ri_control_field *)p->d.send_data->ri_field;

      /****************************************************************/
      /* move pointer to saps, past routing information data          */
      /****************************************************************/
      t_ptr =  (char *) p->ri.ri_sd ;
      t_ptr += p_to_ri->ri_lth;
      p->ri.ri_sd =  (struct ri_sd *) t_ptr;
    } 

  /********************************************************************/
  /* get control bytes and sap numbers                                */
  /********************************************************************/

  info = p->ri.ri_sd->ctl1;
  info = (info<<8)|p->ri.ri_sd->ctl2;
  info = (info<<8)|p->ri.ri_sd->rsap;
  info = (info<<8)|p->ri.ri_sd->lsap;
#endif                                  /* TRLORFDDI                         */
#ifdef   EDL
  dlctype |= DLC_ETHERNET;
#endif
#ifdef   E3L
  dlctype |= DLC_IEEE_802_3;
#endif
#ifndef  TRLORFDDI
  info = datap->ctl1;
  info = (info<<8)|datap->ctl2;
  info = (info<<8)|datap->rsap;
  info = (info<<8)|datap->lsap;
#endif                                  /* not TRLORFDDI                     */

  /********************************************************************/
  /* get station number in upper half word and get number from port   */
  /* name in lower half word                                          */
  /********************************************************************/

#ifndef FDL
  port_sta = ((p->stano<<16)|p->dlc_port.namestr[3]);
#elif FDL
  port_sta = ((p->stano<<16)|p->dlc_port.namestr[4]);
#endif

  /********************************************************************/
  /* call trchkgt to record generic trace data                        */
  /********************************************************************/

  trchkgt(HKWD_SYSX_DLC_MONITOR|dlctype, 0x09, datap, buf->m_pkthdr.len,
     info, port_sta);    /* 175640 */

  /********************************************************************/
  /* setup hook id and type for performance hook                      */
  /********************************************************************/

  dlctype = HKWD_SYSX_DLC_PERF|DLC_TRACE_SNOIF;
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
  /* call trchklt to record performance hook id                       */
  /********************************************************************/

  trchklt(dlctype, buf->m_len);

  TRACE3(p, "LWRT", buf, MTOD((struct mbuf *)buf, caddr_t));
 
  if ((!bcmp(p->d.send_data->raddr, p->common_cb.local_addr, 6)) || 
     ((p->stano != NO_MATCH) && (p->sta_ptr->loopback != 0)))
    {
      rc = local_write(p, buf);
      return (rc);
    } 
  
  /**********************************************************************/
  /* call ndd_output to pass data to the device handler                 */
  /**********************************************************************/
  rc = (*p->nddp->ndd_output)(p->nddp, buf);
/* end feature CDLI */
      
  /**********************************************************************/
  /* Beginning ix30566 Defect 67399                                     */
  /**********************************************************************/

  if (rc != 0)		/*  if the write to the device handler failed   */
  {									
#ifdef   DEBUG
      if (p->debug)
        printf("rc = %d\n", rc);
#endif

     lanfree(p, buf);                     /* free the mbuf chain        */
     if (p->stano != NO_MATCH)            /* if station number if valid */
				          /* (a station doing the xmit) */
     {
        /* bump the station's adapter transmit error counter            */
        ++p->sta_ptr->ras_counters.counters.adp_send_err;
     } 
 
     if (rc == ENETUNREACH)   /* if device is in temporary "limbo" mode */
     {
        if (p->stano != NO_MATCH)          /* if a station did transmit */
        {
           p->sta_ptr->sta_limbo = 1;      /* enter station limbo mode  */
        } 
        else                               /* if this is a sap transmit */
        {
/* defect 82006      */
           if                              /* SAP has a control block    */
             (p->sap_ptr != NO_SAP_CB)
/* end defect 82006   */
           
           p->sap_ptr->sap_limbo = 1;      /* enter sap limbo mode      */
        } 
        /****************************************************************/
        /* log a temporary error - unnusual network condition           */
        /****************************************************************/
        lanerrlg(p, ERRID_LAN8031, NON_ALERT, TEMP_ERR, 
         			    DLC_SAP_NT_COND, FILEN, LINEN);
     }  

     else	/* not in temporary "limbo" mode			*/	
     {	
        if (rc == EAGAIN)     /* if busy... try again later... (EAGAIN) */
	{
            /************************************************************/
            /* log a temporary error - unnusual network condition     	*/
            /************************************************************/
            lanerrlg(p, ERRID_LAN8031, NON_ALERT, TEMP_ERR, 
               DLC_SAP_NT_COND, FILEN, LINEN);
	}
        else  /* not a temporary "limbo" mode failure - log PERM error	*/
        {     
           /*************************************************************/
           /* log a permanent error and shutdown the port               */
           /*************************************************************/
           lanerrlg(p, ERRID_LAN8031, NON_ALERT, PERM_PLC_ERR, 
         			       DLC_SAP_NT_COND, FILEN, LINEN);
        } 
     }
  } 
  /**********************************************************************/
  /* End of ix30566 Def 67399                                           */
  /**********************************************************************/
 
  else                                 /* the write went ok           */
    {
      if                               /* station number is valid, ie.
                                          a station is doing the      
                                          transmit                    */
         (p->stano != NO_MATCH)
        {

          /************************************************************/
          /* reset any previous station limbo mode                    */
          /************************************************************/

          p->sta_ptr->sta_limbo = 0;
/* LEHb defect 44499 */
#ifdef TRLORFDDI
	  if                            /* just transmitted first packet
					   after single route broadcast
					   was received               */
	     (p->sta_ptr->sri_length != 0)
	    {
	      /********************************************************/
	      /* replace station's 2-byte all route broadcast with    */
	      /* single route broadcast value saved at receive time   */
	      /********************************************************/

	      p->sta_ptr->ri_length = p->sta_ptr->sri_length;
	      bcopy(&(p->sta_ptr->sri_field[0]), &(p->sta_ptr->ri_field),
		    p->sta_ptr->sri_length);

	      /********************************************************/
	      /* reset the saved single route broadcast length to     */
	      /* indicate that the copy operation is complete         */
	      /********************************************************/

	      p->sta_ptr->sri_length = 0;

	    }                           /* endif tx after single route */
#endif
	}                               /* endif valid station         */
/* LEHe */

/* defect 82006   */
      if                                /* SAP has a control block    */
         (p->sap_ptr != NO_SAP_CB)
/* end defect 82006   */

      /****************************************************************/
      /* always reset any previous sap limbo mode                     */
      /****************************************************************/

      p->sap_ptr->sap_limbo = 0;
    } 
}                                      /* end lanwrtsg;               */

lansrslt(p,in_op_res,in_results,in_user_sap_corr,in_user_ls_corr)

/*** start of specifications ******************************************/
/*                                                                    */
/* module name:  lansrslt                                             */
/*                                                                    */
/* descriptive name:  send short result to user                       */
/*                                                                    */
/* function:  builds the short result exception.                      */
/*                                                                    */
/* input:  1) operation results                                       */
/*         2) result flags                                            */
/*         3) user correlator                                         */
/*                                                                    */
/* output:                                                            */
/*                                                                    */
/*** end of specifications ********************************************/

  struct port_dcl *p;
  int      in_op_res;
  int      in_results;
  int      in_user_sap_corr;
  int      in_user_ls_corr;
{
  struct dlc_getx_arg dlc_getx_arg;
  struct dlc_chan *c_ptr;
 

  if (p->debug)
    printf(
      "lansrslt: in_op_res=%x, in_results=%x, user_sap=%d user_ls=%d\n"
       , in_op_res, in_results, in_user_sap_corr, in_user_ls_corr);
  dlc_getx_arg.result_ind = in_results;
  dlc_getx_arg.user_sap_corr = in_user_sap_corr;
  dlc_getx_arg.user_ls_corr = in_user_ls_corr;
  dlc_getx_arg.result_code = in_op_res;

  /********************************************************************/
  /* call the exception handler to return data                        */
  /********************************************************************/

  c_ptr = p->sap_ptr->user_sap_channel;
  TRACE3(p, "XFAb", in_results, in_op_res);
  p->rc = (*c_ptr->excp_fa)(&dlc_getx_arg, c_ptr);
  TRACE2(p, "XFAe", p->rc);
 
  if (p->rc != 0)

    /******************************************************************/
    /* error - Bad return code from user's exception routine          */
    /******************************************************************/

    lanerrlg(p, ERRID_LAN8087, NON_ALERT, PERM_SAP_ERR, DLC_SYS_ERR, 
       FILEN, LINEN);
}                                      /* end lansrslt                */

/* defect 82006  */

/*
 *   COMPONENT_NAME: (SYSXDLCG) Generic Data Link Control
 *
 *   FUNCTIONS: Provide link list functions to manage multicast address
 *              list. The functions are:
 *                 llhead -- set current link pointer to start of list
 *                 llinit -- setup multicast link list
 *                 llcheck -- find an element in the list
 * <<< feature CDLI >>>
 *     remove  llcrlink -- create a link list element
 * <<< end feature CDLI >>>
 *                 llnext -- move to next element in the list
 *                 lltail -- move to last element in list
 *                 lldelete -- remove an element from the list
 *                 lladd  --  add an element to the list 
 * <<< defect 142249 >>>
 *                 llget  --  get and remove the group address at the head of the list
 * <<< end defect 142249 >>>
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *  
 *   EXECUTION ENVIRONMENT: Executes under a process.
 *
 *   PRE CONDITIONS:
 *
 *   POST CONDITIONS:
 *
 *   PARAMETERS:
 *
 *   NOTES:
 *
 *   RECOVERY OPERATION:
 *
 *   DATA STRUCTURES:
 *      Link List entry:
 *                       char *nextentry;
 *                       char *previousentry;
 *                       char multicast_address[6];
 *
 *      Link list anchor entry in port control block:
 *                       char *head;
 *                       char *tail;
 *                       char *current_entry;
 *                       int  items_in_list;
 *                       int  entry_size;
 *                        
 *
 *   RETURNS:
 * 
 *
 */


#define NAME_SIZE 6    /* multicast address length  */



int llcheck(lookfor,p)
  register struct port_dcl *p;
char *lookfor;        /* multicast address to find   */


{
  int rc;           /* return code                 */
  int localrc;      /* llnext return code           */


  rc = 0;
  
  if   /* at least one element in mulitcast address list  */
    (p->sap_ptr->mc_addr.listlength > 0) {


    if   /* current pointer points to input group address */
       (memcmp(p->sap_ptr->mc_addr.clp->grpaddr, lookfor, NAME_SIZE) == 0 ) {
      
       /* indicate successful find */
         rc = 1;


     } else
        
     {

       /* setup search pointer to head of group address list */
       llhead(p);
       
/* defect 147857 */
       /* search multicast address list for group address  */
       while (1) {
         if (memcmp(p->sap_ptr->mc_addr.clp->grpaddr, lookfor,
             NAME_SIZE) == 0 ) {

            /* indicate successful find */
            rc = 1;
            break;
         }

         /* move to next address in search list or exit if at the end */
         localrc = llnext(p);
         if (localrc == NULL)
            break;
       }
/* end defect 147857 */

    }

  }

 return (rc);    
}

/* <<< feature CDLI >>> */
/* moved contents of llcrlink (one line) to lladd */
/* <<< end feature CDLI >>> */
    

llinit(p)
  register struct port_dcl *p;
{
    
                             /* setup head, tail, current address pointers to
                                indicate address list empty; initiate counter
                                and item size variables                */
                      
    p->sap_ptr->mc_addr.head = NULL; 
    p->sap_ptr->mc_addr.tail= p->sap_ptr->mc_addr.clp = NULL;
    p->sap_ptr->mc_addr.itemlength = sizeof(struct linktype);
    p->sap_ptr->mc_addr.listlength = 0;
    
}

llhead (p)
  register struct port_dcl *p;
{
                             /* move current address pointer to start of 
                                 address list                           */    
    p->sap_ptr->mc_addr.clp = p->sap_ptr->mc_addr.head;
}

lltail(p)
  register struct port_dcl *p;
{
                            /* move current address pointer to tail of 
                                 address                               */

    p->sap_ptr->mc_addr.clp = p->sap_ptr->mc_addr.tail;

    
}

int llnext(p)
  register struct port_dcl *p;
{
  int    rc;

                           /* move current address pointer to next address
                              in list if not at end of list                */  
  if (p->sap_ptr->mc_addr.clp->next == NULL)
	rc = 0;
    else {
	p->sap_ptr->mc_addr.clp = p->sap_ptr->mc_addr.clp->next;
	rc = 1;
    }
    
    return rc;
    
}



lladd(newitem,p)
char *newitem;
  register struct port_dcl *p;

{
    struct linktype *newlink;

/* <<< feature CDLI >>> */
    /* get storage for new link to be inserted into group address list */
    newlink = (struct linktype *) palloc(sizeof(struct linktype),4);

    if /* palloc failed */
       (newlink == 0)
      {
	/* call error log - palloc for group address failed.
	   sap shutdown = system error */
	lanerrlg(p, ERRID_LAN8045, NON_ALERT, PERM_SAP_ERR,
					     DLC_SYS_ERR, FILEN, LINEN);

	return (ENOMEM);
      }
/* <<< end feature CDLI >>> */

    bcopy(newitem, newlink->grpaddr, 6);

    p->sap_ptr->mc_addr.listlength++;
	
/* defect 147857 */
    /* setup link pointers in the new element */
    if /* first element to be added to list */
      (p->sap_ptr->mc_addr.clp == NULL) {

        /* set next and previous pointers in new element to NULL */
        newlink->next = NULL;
        newlink->previous = NULL;

        /* set tail to new element */
	p->sap_ptr->mc_addr.tail = newlink;
       }

     else /* not the first element */
        {
        /* push the new element onto the front of the list */
        p->sap_ptr->mc_addr.head->previous = newlink;
        newlink->next = p->sap_ptr->mc_addr.head;
        newlink->previous = NULL;
        }

    /* set the head pointer to the new element */ 
    p->sap_ptr->mc_addr.head = newlink;

    /* set the current pointer to the new element */ 
    p->sap_ptr->mc_addr.clp = newlink;
/* end defect 147857 */
    
/* <<< feature CDLI >>> */
    return (0);
/* <<< end feature CDLI >>> */
}



lldelete(p)
  register struct port_dcl *p;
{
    struct linktype *before, *after;
    int    rc;
    
    /* is this the only address in list */
/*
    if ((p->sap_ptr->mc_addr.head == p->sap_ptr->mc_addr.clp) &&
           (p->sap_ptr->mc_addr.tail == p->sap_ptr->mc_addr.clp))
*/
    if
       (p->sap_ptr->mc_addr.listlength == 1)
    {
	p->sap_ptr->mc_addr.head = p->sap_ptr->mc_addr.tail = NULL;
    }
    /* is this the head of list */
    else
	if (p->sap_ptr->mc_addr.head == p->sap_ptr->mc_addr.clp) {
	    p->sap_ptr->mc_addr.head = p->sap_ptr->mc_addr.head->next;
	    p->sap_ptr->mc_addr.head->previous = NULL;
	}
    /* is this the tail  */
	else 
	    if (p->sap_ptr->mc_addr.tail == p->sap_ptr->mc_addr.clp) {
		p->sap_ptr->mc_addr.tail = p->sap_ptr->mc_addr.tail->previous;
		p->sap_ptr->mc_addr.tail->next = NULL;
	    }
    /* otherwise it must be inside list  */
	    else 
	    {
		before = p->sap_ptr->mc_addr.clp->previous;
		after = p->sap_ptr->mc_addr.clp->next;
		
		before->next = after;
		after->previous = before;
	    }
    /* delete current multicast address item */
/* <<< feature CDLI >>> */
    assert(xmfree(p->sap_ptr->mc_addr.clp, kernel_heap) == 0);
/* <<< end feature CDLI >>> */

    /* reset current multicats address to first address in list */
    p->sap_ptr->mc_addr.clp = p->sap_ptr->mc_addr.head;
    p->sap_ptr->mc_addr.listlength--;
    
}

/* end  defect 82006  */

/* defect 142249 */
llget(addr,p)
  char *addr;
  register struct port_dcl *p;
{
    struct linktype *tempaddr;
    int    rc;

    rc = 0; /* set default to "no more addresses" */    
    if /* at least one group address in the list */   
       (p->sap_ptr->mc_addr.listlength != 0)
      {
        /* point to the head element */
        tempaddr = p->sap_ptr->mc_addr.head;

        if /* there is only one address in list */
           (p->sap_ptr->mc_addr.listlength == 1)
          {
            /* null out the head and tail pointers */
	    p->sap_ptr->mc_addr.head = NULL;
	    p->sap_ptr->mc_addr.tail = NULL;
          }

        else /* more than one address in the list */
          {
            /* set head to next, and previous to null */
	    p->sap_ptr->mc_addr.head = p->sap_ptr->mc_addr.head->next;
	    p->sap_ptr->mc_addr.head->previous = NULL;
	  }

        /* set current to head (may be null) */
        p->sap_ptr->mc_addr.clp = p->sap_ptr->mc_addr.head;

        /* decrement the list length */
        p->sap_ptr->mc_addr.listlength--;

        /* copy out the address to the caller */ 
        bcopy (&(tempaddr->grpaddr[0]), addr, 6);

        /* delete the multicast address element */
        assert(xmfree(tempaddr, kernel_heap) == 0);
 
        rc = 1;  /* indicate that there is an address being returned */
      }    
    return (rc);
}
/* end defect 142249 */

