static char sccsid[] = "@(#)61	1.16  src/bos/kernext/dlc/head/dlcconfig.c, sysxdlcg, bos412, 9446B 11/15/94 16:45:55";
/*
 * COMPONENT_NAME: (SYSXDLCG) Generic Data Link Control
 *
 * FUNCTIONS: dlccconfig
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
 * NAME: dlcconfig
 *                                                                    
 * FUNCTION: Dlccconfig, if called with the INIT command, adds the protocol's 
 *  entry points into the switch table and initializes the head of the channel 
 *  id list.  If called with the TERM command and the protocol does not have any
 *  active channels, then it removes the entry points from the switch table.
 *                                                                    
 * EXECUTION ENVIRONMENT: Dlcconfig is called by the system to add and delete
 *  the protocol.  The routine must be the first in the extension.
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
struct dlcconfig 
{
	dev_t	dev;
	int	maxq;
} config;

/* defect 167068 */
#define STRACEMAX	100
Simple_lock	trace_lock;
ulong_t		*trace_ptr;
ulong_t		*trace_top;
ulong_t		*trace_end;
ulong_t		trace_table[STRACEMAX];

extern int dlcopen();
extern int dlcclose();
extern int dlcread();
extern int dlcwrite();
extern int dlcioctl();
extern int dlcselect();
extern int dlcmpx();
extern int dlc_rqcreate();
extern int nodev();

/*ARGSUSED*/
dlcconfig(devno,cmd, uiop)
dev_t devno;
int cmd;      
struct uio *uiop;     
{ 
int rc;
struct devsw dlcdev;
struct dlc_chan *tmpx;

#ifdef DLC_DEBUG
printf("dlcconfig : cmd=%d\n",cmd);
#endif
switch(cmd)
{

/**************************************************************************/
/*                                                                        */
/*                                                                        */
/**************************************************************************/

case CFG_INIT :

#define E_SOFTWARE 1
#define E_UNIX 2
#define E_UNIXDD1 3
#define E_INFO 4

	rc = uiomove((caddr_t)&config, sizeof (struct dlcconfig), UIO_WRITE, uiop);
#ifdef cw
	devno = makedev(21,0);
#endif

	if ((rc=pincode(dlcconfig)) != 0) 
		return(rc);

	/* defect 166894, removed rc=pincode(dlc_rqcreate)
           since it's already pinned above */

      /* init the channel list lock.  This must be done in this routine as it */
      /*  the only one called but once per IPL before any other calls are made*/

/* defect 122577   */
        lock_alloc(&dlcsum.lock, LOCK_ALLOC_PIN, CHANNEL_LIST_LOCK, -1);
        
        simple_lock_init(&dlcsum.lock);
/* end defect 122577 */

	dlcsum.chanp = (struct dlc_chan *) DLC_NULL;
	dlcsum.maxq = config.maxq;

	dlcdev.d_open = dlcopen;
	dlcdev.d_close = dlcclose;
	dlcdev.d_read = dlcread;
	dlcdev.d_write = dlcwrite;
	dlcdev.d_ioctl = dlcioctl;
	dlcdev.d_strategy = nodev;
	dlcdev.d_select = dlcselect;
	dlcdev.d_ttys = DLC_NULL;
	dlcdev.d_config = dlcconfig;
	dlcdev.d_print = nodev;
	dlcdev.d_dump = nodev;
	dlcdev.d_mpx = dlcmpx;
	dlcdev.d_revoke = nodev;
	dlcdev.d_opts = DEV_MPSAFE;
	
	rc = devswadd(devno,&dlcdev);

	if (rc != DLC_OK)
	{
		unpincode(dlcconfig);   /* added, defect 166894 */
		return(rc);
	}

/* defect 167068 */
	/* initialize the static in-core trace */
        lock_alloc(&trace_lock, LOCK_ALLOC_PIN, DLCTRACE_LIST_LOCK, -1);
        simple_lock_init(&trace_lock);

	trace_top = &trace_table[0];
	trace_ptr = trace_top;
	trace_end = &trace_table[STRACEMAX];
/* end defect 167068 */
	
	return(DLC_OK);
	break;

/**************************************************************************/
/*                                                                        */
/*                                                                        */
/**************************************************************************/

case CFG_TERM :

  
/* defect 122577 */
  simple_lock(&dlcsum.lock);
/* end defect 122577 */

 
	if (dlcsum.chanp != EMPTY)
	{

/* defect 122577 */
  simple_unlock(&dlcsum.lock);
/* end defect 122577 */

		return(EBUSY);

	}

/* defect 122577 */
  simple_unlock(&dlcsum.lock);

  lock_free(&dlcsum.lock);

/* end defect 122577 */

	lock_free(&trace_lock);  /* defect 167068 */
	
	devswdel(devno);
	unpincode(dlcconfig);
	/* defect 166894, removed unpincode(dlc_rqcreate); */

	return(DLC_OK);
	break;


/**************************************************************************/
/*                                                                        */
/*                                                                        */
/**************************************************************************/

default:

#ifdef DLC_DEBUG
printf("dlcconfig : ERROR -->OUT OF SWITCH.\n");
#endif

	return(EINVAL);

}; /* end switch(cmd) */

return(DLC_OK);

}

/* defect 167068 */
static_trace(p,ascii_word,hex_data)
  register struct dlc_port *p;
  char  	*ascii_word;
  ulong  	hex_data;
{

  simple_lock(&trace_lock);

  if (p != 0)
    {
      bcopy(&p->namestr[0], trace_ptr, 8);
      trace_ptr = trace_ptr + 2;  /* bump 2 words */
    }
  else
    {
      *(trace_ptr) = 0;
      trace_ptr = trace_ptr + 1;  /* bump 1 word */
      *(trace_ptr) = 0;
      trace_ptr = trace_ptr + 1;  /* bump 1 word */
    }

  bcopy(ascii_word, trace_ptr, 4);
  trace_ptr = trace_ptr + 1;  /* bump 1 word */
  *(trace_ptr) = hex_data; 
  trace_ptr = trace_ptr + 1;  /* bump 1 word */

  if (trace_ptr >= trace_end)
    trace_ptr = trace_top;

  simple_unlock(&trace_lock);

  return;
/* end defect 167068 */
}
