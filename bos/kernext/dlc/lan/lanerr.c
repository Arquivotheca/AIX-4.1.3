static char sccsid[] = "@(#)09  1.9.1.2  src/bos/kernext/dlc/lan/lanerr.c, sysxdlcg, bos411, 9428A410j 2/21/94 18:46:48";
/*************************************************************************
 * COMPONENT_NAME: (SYSXDLCG) Generic Data Link Control
 *
 * FUNCTIONS: Build Data Link Control Error Log Data
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1992
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 ************************************************************************/

#include <fcntl.h>
#include <sys/types.h>
#include <net/spl.h>
#include <sys/mbuf.h>
#include <sys/file.h>
#include <sys/malloc.h>
#include <sys/errno.h>
#include "dlcadd.h"
#include <sys/gdlextcb.h>
/* <<< defect 127819 >>> */
#ifdef   TRL
#include <sys/trlextcb.h>
#endif
#ifdef   FDL
#include <sys/fdlextcb.h>
#endif
/* removed fddi, tok, and comio header files >>> */
/* <<< end defect 127819 >>> */
#include "lancomcb.h"
#include "lanmem.h"
#include "lanstlst.h"
#include "lansplst.h"
#include "lanrstat.h"
#include "lanport.h"


/*-----------------------------------------------------------------------*/
/* FUNCTION: This routine builds the front end of all DLC errlog entries */
/* RETURNS:  A pointer to the next error buffer position where additional*/
/*           detailed data can be appended.                              */
/*-----------------------------------------------------------------------*/
bld_err_top(p, error, filenm, line)
register struct port_dcl *p;
int error;                  /* error id for this log */
char *filenm;               /* file name of module logging the error  */
int line;                   /* line number of module logging the error */
{
  int tempint;

  struct err_rec *rptr;

  p->errptr = p->build_err_rec;
  rptr = (struct err_rec *)p->errptr;

  /* Setup the Error ID field in the error record */
  rptr->error_id = error;
  /* Load the Resource Name into the error record */
  strcpy(rptr->resource_name,COMP_NAME);

  /* Build the generic part of the detailed data area */
  /* Initialize the Detail Data pointer, vector size counter and
					    total vector size counter */
     p->errptr = rptr->detail_data;

  /* (8019) Data Link Type */
     bcopy (DLC_TYPE, p->errptr, 16);
     p->errptr = p->errptr + 16;    /* fixed length */
  /* (8016) Communications Device Name */
     bcopy(p->dlc_port.namestr, p->errptr, 16);    /* fixed length */
     p->errptr = p->errptr + 16;
  /* (00A2) Detecting Module */
     bcopy("                ",p->errptr,16);
     bcopy(filenm, p->errptr, strlen(filenm));     /* current file name */
     p->errptr = p->errptr + 16;
     bcopy(&line, p->errptr, 4);         /* current line number */
     p->errptr = p->errptr + 4;
} /* end bld_err_top */


/*-----------------------------------------------------------------------*/
/* FUNCTION: This routine logs an error to the system or prints the log  */
/*           to standard out based on the DLCPLOG flag.                  */
/* RETURNS:  n/a                                                         */
/*-----------------------------------------------------------------------*/
lan_log(p)
register struct port_dcl *p;
{
#ifdef DLC_DEBUG
   if (p->debug)
      printf("p->errptr=%x\n",p->errptr);
      dump(p->build_err_rec, (int)p->errptr-(int)&p->build_err_rec[0]);
#endif

    p->rc = errsave (p->build_err_rec,
			       (int)p->errptr-(int)&p->build_err_rec[0]);
} /* end lan_log */

alert_detail_data(p)
register struct port_dcl *p;
{
   char   *vlptr;                   /* vector length field pointer */
   char   temp_va;
/* LEHb defect 44499 */
   ushort tempint;
/* LEHe */

  /* Builds the generic Detail_Data area for alerts */

  /* (8001) Additional Subvectors */
     tempint = ADD_SUBVECS;
     bcopy (&tempint, p->errptr, 2);
     p->errptr = p->errptr + 2;
/*--------------------------------------------------------------------*/
     /* (51) LAN Link Conn Subsystem Data vector */
	 /* save the position of vector length field and bump past */
	 vlptr = p->errptr;
	 p->errptr++;
	 /* load the subvector value and bump */
	 *p->errptr = SV_LCS_DATA;
	 p->errptr++;
	 /* (03) Local MAC address */
	    /* load the subfield length and bump */
	    *p->errptr = 8;
	    p->errptr++;
	    /* load the subvector value and bump */
	    *p->errptr = SF51_LOCAL_MAC;
	    p->errptr++;
	    /* load the local MAC address value and bump */
	    bcopy (p->sta_ptr->laddr, p->errptr, 6);
	    p->errptr = p->errptr + 6;
	 /* Remote MAC address                */
	    /* load the subfield length and bump */
	    *p->errptr = 8;
	    p->errptr++;
	    /* load the subvector value and bump */
	    *p->errptr = SF51_REMOTE_MAC;
	    p->errptr++;
	    /* load the remote MAC address value and bump */
	    bcopy (p->sta_ptr->raddr, p->errptr, 6);
	    p->errptr = p->errptr + 6;
#ifdef TRLORFDDI
	 /* Routing Information               */
	    /* load the subfield length and bump */
	    *p->errptr = 2 + p->sta_ptr->ri_length;
	    p->errptr++;
	    /* load the subvector value and bump */
	    *p->errptr = SF51_ROUTING;
	    p->errptr++;
/* LEHb defect 44499 */
	    /* load the routing information value and bump */
	    bcopy (&(p->sta_ptr->ri_field), p->errptr,
		   p->sta_ptr->ri_length);
/* LEHe */
	    p->errptr = p->errptr + p->sta_ptr->ri_length;
#endif /* TRLORFDDI */
	 /* load the length field back at the top of the vector */
	 *vlptr = p->errptr - vlptr;
/*--------------------------------------------------------------------*/
     /* (52) LAN Connection Subsystem Config */
	 /* save the position of vector length field and bump past */
	 vlptr = p->errptr;
	 p->errptr++;
	 /* load the subvector value and bump */
	 *p->errptr = SV_LCS_CONFIG;
	 p->errptr++;
	 /* (02) Remote link address/SAP */
	    /* load the subfield length and bump */
	    *p->errptr = 3;
	    p->errptr++;
	    /* load the subvector value and bump */
	    *p->errptr = SF52_REMOTE_SAP;
	    p->errptr++;
	    /* load the remote SAP address value and bump */
	    *p->errptr = p->sta_ptr->rsap;
	    p->errptr++;
	 /* (04) Local link address/SAP */
	    /* load the subfield length and bump */
	    *p->errptr = 3;
	    p->errptr++;
	    /* load the subvector value and bump */
	    *p->errptr = SF52_LOCAL_SAP;
	    p->errptr++;
	    /* load the local SAP address value and bump */
	    *p->errptr = p->sta_ptr->lsap;
	    p->errptr++;
	 /* load the length field back at the top of the vector */
	 *vlptr = p->errptr - vlptr;
/*--------------------------------------------------------------------*/
     /* (8C) Link Station Data */
	 /* save the position of vector length field and bump past */
	 vlptr = p->errptr;
	 p->errptr++;
	 /* load the subvector value and bump */
	 *p->errptr = SV_LS_DATA;
	 p->errptr++;
	 /* (01) Current NS/NR Counts */
	    /* load the subfield length and bump */
	    *p->errptr = 4;
	    p->errptr++;
	    /* load the subvector value and bump */
	    *p->errptr = SF8C_NS_NR;
	    p->errptr++;
	    /* load the current NS (sent) count */
	    *p->errptr = p->sta_ptr->vs;
	    p->errptr++;
	    /* load the current NR (received) count */
	    *p->errptr = p->sta_ptr->vr;
	    p->errptr++;
	 /* (02) Outstanding Frame Count */
	    /* load the subfield length and bump */
	    *p->errptr = 3;
	    p->errptr++;
	    /* load the subvector value and bump */
	    *p->errptr = SF8C_FRAMES_OUT;
	    p->errptr++;
	    /* load the number of sent I-frames outstanding */
	    if (p->sta_ptr->va > p->sta_ptr->vs)
	       temp_va = p->sta_ptr->va +128;
	    else
	       temp_va = p->sta_ptr->va;
	    *p->errptr = temp_va - p->sta_ptr->vs;
	    p->errptr++;
	 /* (03) Last Control Received */
	    /* load the subfield length and bump */
	    *p->errptr = 4;
	    p->errptr++;
	    /* load the subvector value and bump */
	    *p->errptr = SF8C_CNTL_RCVD;
	    p->errptr++;
	    /* load the last received control byte values */
	    *p->errptr = p->sta_ptr->rcv_ctl1;
	    p->errptr++;
	    *p->errptr = p->sta_ptr->rcv_ctl2;
	    p->errptr++;
	 /* (04) Last Control Sent */
	    /* load the subfield length and bump */
	    *p->errptr = 4;
	    p->errptr++;
	    /* load the subvector value and bump */
	    *p->errptr = SF8C_CNTL_SENT;
	    p->errptr++;
	    /* load the last sent control byte values */
	    *p->errptr = p->sta_ptr->last_cmd_1;
	    p->errptr++;
	    *p->errptr = p->sta_ptr->vr; /* may need a last_cmd_2 */
	    p->errptr++;
	 /* (05) Sequence Number Modulus */
	    /* load the subfield length and bump */
	    *p->errptr = 3;
	    p->errptr++;
	    /* load the subvector value and bump */
	    *p->errptr = SF8C_MODULUS;
	    p->errptr++;
	    /* load the modulus value */
	    *p->errptr = 128;
	    p->errptr++;
	 /* (06) Link Station State */
	    /* load the subfield length and bump */
	    *p->errptr = 3;
	    p->errptr++;
	    /* load the subvector value and bump */
	    *p->errptr = SF8C_LS_STATE;
	    p->errptr++;
	    /* default the link station state to NOT busy;
	    *p->errptr = 0;
/* LEHb  defect 43788 */
/* delete 4 lines */
	    if (p->sta_ptr->local_busy == TRUE)
/* LEHe */
	    {
	       *p->errptr |= SF8C_LS_LBUSY;
	    }
	    if (p->sta_ptr->remote_busy == TRUE)
	    {
	       *p->errptr |= SF8C_LS_RBUSY;
	    }
	    p->errptr++;
	 /* (07) LLC Reply Timer Expiration Count */
	    /* load the subfield length and bump */
	    *p->errptr = 4;
	    p->errptr++;
	    /* load the subvector value and bump */
	    *p->errptr = SF8C_REPOLLS;
	    p->errptr++;
	    /* load the number of T1 repoll timeouts (?? mismatch ???)*/
	    bcopy (&(p->station_list[p->stano].t1_ctr)+2, p->errptr, 2);
	    p->errptr = p->errptr + 2;
	 /* (08) Last Received NR Count */
	    /* load the subfield length and bump */
	    *p->errptr = 3;
	    p->errptr++;
	    /* load the subvector value and bump */
	    *p->errptr = SF8C_RCVD_NR;
	    p->errptr++;
	    /* load the link station's recieved ack value */
	    *p->errptr = p->sta_ptr->va;
	    p->errptr++;
	 /* load the length field back at the top of the vector */
	 *vlptr = p->errptr - vlptr;
} /* end alert_detail_data */
