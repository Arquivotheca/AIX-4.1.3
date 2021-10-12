static char sccsid[] = "@(#)66	1.10  src/bos/kernext/dlc/head/dlcopen.c, sysxdlcg, bos412, 9446B 11/15/94 16:47:24";
/*
 * COMPONENT_NAME: (SYSXDLCG) Generic Data Link Control
 *
 * FUNCTIONS: dlcopen
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

/*
 * NAME: dlcopen
 *                                                                    
 * FUNCTION: Dlcopen, in the case of an open from the kernel, puts the
 *  functional addresses in the channel id.  In the case of an application call
 *  the dlcintr and dlcinte addresses are put in the channel id.  Then pr_open
 *  is called.
 *                                                                    
 * EXECUTION ENVIRONMENT: Dlcopen is called by a user to start a protocol.
 *                                                                     
 * RETURNS: DLC_OK
 */  

/* defect 122577 */
#include <sys/lock_def.h>
#include <sys/lock_alloc.h>
#include <sys/lockname.h>
/* end defect 122577 */

#include        "dlcadd.h"

struct s_mpx_tab   dlcsum;      /* global table of the channels and adapters */

extern int dlcintr();
extern int dlcinte();

/* START MULT_PU */
extern init_proc();
/* END MULT_PU */

/*ARGSUSED*/
dlcopen(dev, flag, mpx, ext)
dev_t   dev;
ulong   flag;
struct dlc_chan *mpx;
struct dlc_open_ext *ext;
{
#define STANZA_LEN 30
char 		device[STANZA_LEN + 5]; /* extra space to be extra careful */
ulong		tempdev,cb_index,slotnum,rc;
struct dlc_open_ext tempext;
int		i;
/* defect 101311 */
ulong rc_temp;
/* end defect 101311 */

/* defect 122577 */
  simple_lock(&mpx->lock);
/* end defect 122577 */

static_trace(mpx->cb,"Open",flag);  /* defect 167068 */

/****************************************************************************/
/*                                                                          */
/*  Check the flag that was supplied on the OPEN system call.  If the       */
/*  flag does have the DNDELAY bit set, then we are to perform BLOCKED      */
/*  READs.                                                                  */
/*                                                                          */
/****************************************************************************/

if ((flag & DKERNEL))
	mpx->state |= KERN;

/****************************************************************************/
/*                                                                          */
/*     If the call comes from the kernel then the extension holds the       */
/* return addresses, otherwise put the dlc's return addresses in the channel*/
/*                                                                          */
/****************************************************************************/

if (flag & DKERNEL)
{
	if (ext -> maxsaps == 0)
		mpx -> maxsaps = 1;
		else mpx -> maxsaps = ext-> maxsaps;

	mpx -> rcvi_fa = ext-> rcvi_fa;
	mpx -> rcvx_fa = ext-> rcvx_fa;
	mpx -> rcvd_fa = ext-> rcvd_fa;
	mpx -> rcvn_fa = ext-> rcvn_fa;
	mpx -> excp_fa = ext-> excp_fa;
}
else 
{

	if (ext != 0)
	{
		if (copyin(ext, &tempext, sizeof(struct dlc_open_ext)) 
                    == DLC_ERR)
		{

			/* defect 122577 */
			simple_unlock(&mpx->lock);
			/* end defect 122577 */

			static_trace(mpx->cb,"Opn1",EFAULT); /* defect 167068 */
			return(EFAULT);
		}

		if (tempext.maxsaps == 0)
			mpx -> maxsaps = 1;
			else mpx -> maxsaps = tempext.maxsaps;
	}
	else mpx -> maxsaps = 1;

	mpx -> rcvi_fa = dlcintr; 
	mpx -> rcvx_fa = dlcintr;
	mpx -> rcvd_fa = dlcintr;
	mpx -> rcvn_fa = dlcintr;
	mpx -> excp_fa = dlcinte;
	
}
	if ((mpx -> maxsaps > 127) || (mpx -> maxsaps < 1)) 
	{
		mpx->maxsaps = 1;
	}
/* START MULT_PU */
/******************************************/
/* THIS IS ADDED CODE AS PART OF MPU WORK */
/*                                        */
/* If first open, call pr_open.           */
/******************************************/
if (mpx->cb->chan_count == 1)
    if ((rc = pr_open(mpx->cb)) != DLC_OK)
    {

#ifdef DLC_DEBUG
        printf("ERROR : in dlcopen pr_open failed, rc = %d\n",rc);
#endif

#define ECB_CLOSE 0x08000000
        mpx->proc_id = EVENT_NULL;
        mpx -> cb -> kcid = mpx;
/* <<< THREADS >>> */
	static_trace(mpx->cb,"Post",mpx->cb->kproc_tid); /* defect 167068 */
	et_post(ECB_CLOSE,mpx->cb->kproc_tid);
/* <<< end THREADS >>> */
/* defect 122577    */
/* defect 155341 --- removed cntl-c interrupt capability */
	static_trace(mpx->cb,"Slpb",mpx->proc_id);  /* defect 167068 */
        rc_temp = e_sleep_thread( (int *)&mpx->proc_id, (int *)&mpx->lock,
                    LOCK_SIMPLE );
	static_trace(mpx->cb,"Slpe",rc_temp);  /* defect 167068 */

        delay(10); 
/* end 155341 --- added delay to insure that the kproc gets a chance to run kexit */

/* defect 101311 */

/*
# removed xmfree(mpx->cb,pinned_heap);
# removed free(mpx);
# removed unpincode(init_proc);
*/

/* end defect 101311 */

/* defect 122577 */
  simple_unlock(&mpx->lock);
/* end defect 122577 */

	static_trace(mpx->cb,"Opn2",rc);  /* defect 167068 */
        return(rc);
    }
/* END MULT_PU */

static_trace(mpx->cb,"Opne",DLC_OK);  /* defect 167068 */

/* defect 122577 */
  simple_unlock(&mpx->lock);
/* end defect 122577 */


return(DLC_OK);
}
