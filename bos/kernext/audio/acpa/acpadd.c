/* @(#)05 1.36.2.1 src/bos/kernext/audio/acpa/acpadd.c, sysxacpa, bos411, 9428A410j 92/07/22 21:46:11 */

/*
 * COMPONENT_NAME: SYSXACPA     Multimedia Audio Capture and Playback Adapter
 *
 * FUNCTIONS: acpaopen, acpaclose, acpaconfig, acpaioctl, acpaintr, etc.
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*****	Audio Support Software	*****************************************/
/*									*/
/*  DESCRIPTION:							*/
/*									*/
/*	The module contains the	ACPA device driver functions.		*/
/*									*/
/************************************************************************/
/*									*/
/*  EXECUTION ENVIRONMENT:						*/
/*									*/
/*	This module is for the RISC System/6000	only.			*/
/*									*/
/************************************************************************/
/*									*/
/*  INVOCATION:								*/
/*    example:								*/
/*	rc = acpaconfig(devno, cmd, uiop)				*/
/*	dev_t devno;		 device	major and minor	numbers		*/
/*	int cmd;		 the config command			*/
/*	struct uio* uiop;	 pointer to user space			*/
/*									*/
/************************************************************************/
/*									*/
/*  INPUT PARAMETERS:							*/
/*									*/
/*	See above							*/
/*									*/
/************************************************************************/
/*									*/
/*  OUTPUT PARAMETERS:							*/
/*									*/
/*	N/A								*/
/*									*/
/************************************************************************/
/*									*/
/*  RETURN VALUES:							*/
/*									*/
/*	0 if successful							*/
/*	various	nonzero	numbers	if unsuccessful				*/
/*									*/
/*****	System Include Files  *******************************************/

#ifdef i386

/* Include files for AIX PS/2 1.1 device drivers */

#include <sys/param.h>
#include <sys/conf.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/devinfo.h>
#include <sys/errno.h>
#include <sys/conf.h>
#include <sys/proc.h>
#include <sys/buf.h>
#include <sys/file.h>
#include <sys/dir.h>
#include <sys/iobuf.h>
#include <sys/ioctl.h>
#include <sys/ioctlcmd.h>
#include <sys/systm.h>
#include <sys/machinfo.h>
#include <sys/erec.h>
#include <sys/select.h>
#include <sys/mbuf.h>
#include <sys/i386/pos.h>	/* POS information in nvram */
#include <sys/i386/cmos.h>	/* CMOS	database */
#include <sys/i386/intr86.h>	/* "intrattach"	level request masks */
#include <sys/i386/mmu386.h>	/* Memory mapping macros */
#include <vmalloc.h>		/* Kernel memory allocators */
#else

/* Includes files for AIX v3 */
#include <sys/device.h>		/* Device driver stuff */
#include <sys/uio.h>		/* Details of user's I/O request */
#include <sys/mdio.h>		/* Macros for accessing	I/O hardware */
#include <sys/sleep.h>		/* Post/wait and other routines	*/
#include <sys/ldr.h>		/* Kernel extension loader et al */
#include <sys/pin.h>		/* Pin/unpin calls */
#include <sys/malloc.h>		/* Memory allocation routines */
#include <sys/pri.h>		/* Interrupt priorities	*/
#include <sys/poll.h>		/* Select */
#include <sys/lockl.h>		/* preemption synchronization */
#include <sys/errno.h>
#include <sys/syspest.h>
#include <sys/watchdog.h>	/* watchdog timers */
#include <sys/iocc.h>
#include <sys/ioacc.h>
#include <sys/adspace.h>
#endif /* i386 */

/*********************************************************************/
/* Include and defines for acpaerr				     */
/*********************************************************************/
#include <sys/errids.h>
ERR_REC(230) ER;

/*****	Global Variables  ***********************************************/

#ifdef ACPA_MEMORY_TRACE
int *xxptr[16];
int *txxptr[16];
int extraflag = 0;
#endif

static config_count = 0;      /* Counts	number of times	config was done	*/
int msgdebug = 1;	      /* Flags whether error output is to be done */
int acpabug = 1;	      /* Flags whether error output is to be done */

/*****	Main Entry Point  ***********************************************/

int acpainit();
int acpareset();
int acpaopen();
int acpaclose();
int acparead();
int acpawrite();
int acpaioctl();
int acpaselect();
int acpaintr();
#ifdef i386
int notty();			/* Perhaps */
int cpass();			/* Input in From user buffer */
int passc();			/* Output to user buffer */
#else
void timed_out(struct wdog_info	*);
long load_ucode_into_memory();
#ifdef ORIGINAL
#define	BUS_ID	0x820C0020
#else
#define	BUS_ID	0x820C0020
#endif /* ORIGINAL */
/*#define IOCC_BID	0x820C00A0*/
extern int nodev();
#endif /* i386 */
void acpaput( int adr, unsigned	short dat, int min );

#ifdef i386
extern	struct devdata devdata[]; /* One struct	for each MCA bus slot */
#endif /* i386 */

#include <sys/audio.h>
#include <sys/acpa.h>
#include "oldacpa.h"
#include "oldacpahw.h"		   /* Include hardware information */
#include "acpadiags.h"

#include "trace.h"

#include <sys/disptrc.h>

/* See page 10 of the DSP documentation	for this information */
/* 0x0E48: Host	to DMS music buffer track 1 block 0 start address */
/* 0x1718: Host	to DMS music buffer track 2 block 0 start address */
unsigned short acpa_musicbufbases[2] = {0x0e48,	0x1718};
/* 0x0E48: Host	to DMS voice buffer track 1 block 0 start address */
/* 0x1B80: Host	to DMS voice buffer track 2 block 0 start address */
unsigned short acpa_voicebufbases[2] = {0x0e48,	0x1b80};
/* 0x0E48: Host	to DMS stereo buffer track 1 block 0 start address */
/* 0x1B80: Host	to DMS stereo buffer track 2 block 0 start address */
unsigned short acpa_stereobufbases[2] =	{0x0e48, 0x0e48};
/* There are 8 blocks for mono playback, and 16	blocks for stereo. */
/* Recording always uses 16 blocks. */
unsigned short acpa_8_pcmbases[2] = { 0x0e70, 0x1730 };
unsigned short acpa_16_pcmbases[2] = { 0x0e70, 0x0e70 };

/* The possible	interrupt levels; see page 6-28	of ACPA	Tech Ref */
static int irqlevels[8]	= {3, 4, 5, 6, 2, 10, 11, 12};
/* The possible	start addresses	for first ACPA register	for PIO	*/
/* See page 6-10 of ACPA Tech Ref */
static unsigned	short iobases[4] = {0xfdc0, 0xfdc8, 0xfdd0, 0xfdd8};

#ifndef	i386
BUGVDEF(dbg_avc, 1);
#endif 

/************************************************************************/
/*									*/
/*  FUNCTION NAME: acpaconfig						*/
/*									*/
/*  DESCRIPTION:							*/
/*									*/
/*	This routine is	used to	configure the device driver.  It is	*/
/*	called at boot time from cfgdevice or when the device is	*/
/*	configured dynamically.						*/
/*									*/
/*  INVOCATION:								*/
/*									*/
/*	rc = acpaconfig( dev_t devno, int cmd, struct uio *uiop	)	*/
/*									*/
/*  INPUT PARAMETERS:							*/
/*									*/
/*	devno contains the major and minor numbers of the adapter.	*/
/*	cmd specifies which config command is to be executed.		*/
/*	uiop is	a pointer to the u area	of the calling process.		*/
/*									*/
/*  OUTPUT PARAMETERS:							*/
/*									*/
/*	N/A								*/
/*									*/
/*  RETURN VALUES:							*/
/*									*/
/*	0 if successful							*/
/*	-1 if unsuccessful.  In	this case errno	will also be set.	*/
/*									*/
/************************************************************************/

int acpaconfig(devno, cmd, uiop)
dev_t devno;
int cmd;
struct uio* uiop;
{
  int our_rc;			/* This is the return code value */	
  int i, j;
  struct acpa_dds *ddsptr;
  volatile unsigned char *pptr;
  unsigned char	pos2;
  unsigned char	test;
  caddr_t iocc_addr;
  uint	err_flag,offet;
  int min;
  register struct acpa_status *aptr; /*	ptr to current adapter's status	*/
  register Track *tptr1;	/* pointer to track 1 info */
  register Track *tptr2;	/* pointer to track 2 info */
  extern int acpampx();

  our_rc = 0;			/* Assume everything's OK until	proven */
				/* otherwise */

  min =	minor( devno );		/* Get the minor number	of the device */

  aptr = (struct acpa_status *)	&( acpa[min] );
  tptr1	= (Track *) &( acpa[min].track[0] );
  tptr2	= (Track *) &( acpa[min].track[1] );

  /* This routine is called to initialize, query and delete the	device */
  /* driver.  Determine	which request and handle it accordingly. */

  switch(cmd) {

  case CFG_INIT:

    ENTER_ACPA_PTRACE( CFGINIT,	ACPA_MAIN, min );

    /* We have been called to initialize the ACPA device driver. */
    /* This routine must add the other device driver routines to the */
    /* device switch via the devswadd routine and must copy the	*/
    /* device-dependent	structure (dds)	provided by the	caller.	 We */
    /* perform no hardware specific operations at this point, */
    /* deferring them until open time. */

    /* Copy the	device-dependent structure from	our caller and save it */
    /* for later use */
    if ((our_rc	= uiomove(&(aptr->dds),	sizeof(aptr->dds), UIO_WRITE,
			  uiop)) != 0) {
      our_rc = EIO;
      break;			/* Quit	right now if uiomove failed */
    }

    /* THE FOLLOWING LEVEL/BASE CHECKS MAY BE UNNECESSARY */
    ddsptr = &(	aptr->dds );
    /* Get the current interrupt level and pio base address to construct */
    /* the contents of POS2 for	subsequent verification.  There	is no	 */
    /* checking	for out-of-loop	conditions because they	"can't"	happen	 */
    /* with ODM.  This code won't be reached if	something is wrong.	 */
    for (i=0; i<8; i++)
	if (aptr->dds.int_level	== irqlevels[i])
          break;
    for (j=0; j<4; j++)
      if (aptr->dds.bus_addr ==	iobases[j])
	break;
    if ( ( i == 8 ) || ( j == 4 ) )
    {
      our_rc = EINVAL;
      break;
    } 

    /* This is explained on page 6-10 of the ACPA Technical Reference. */
    pos2 = (i << 3) | (j << 1) | 1;

    /* Use IOCC_ATT to talk to the bus,	and BUSIO_ATT to talk to an */
    /* adapter on the bus.  Here we will talk with the bus. */
    iocc_addr =	IOCC_ATT(IOCC_BID,0);

    /* Set POS2	to the desired value. */
    /* Unlike PS/2s, our machines set POS2; it's not done for us. */
    BUSIO_PUTC(iocc_addr+IO_IOCC+POSREG(2,aptr->dds.slot_number),pos2);

    /* Get the contents	of POS2. */
    test = BUSIO_GETC(iocc_addr+IO_IOCC+POSREG(2,aptr->dds.slot_number));

    IOCC_DET(iocc_addr);

    /* Verify that the contents	of POS2	were set correctly. */
    if ((test &	0x3f) != pos2) {
      TRACE(("POS2 corrupted, should be %x, is %x\n", (int) pos2, (int) test));
      our_rc = EINVAL;
      break;
    }

    /* Now save	a pointer to this device's DDS structure. */
    aptr->dev.d_dsdptr = (void *)&aptr->dds;
    /* Save the	device's base bus address. */
    aptr->base = aptr->dds.bus_addr;

    /* Initialize the ACPA card	and the	acpa structure.	 The ACPA card */
    /* should be reset (not running), unable to	interrupt the host, and	*/
    /* the internal speaker (if	any) should be disabled. */
    if ( acpainitlz( &(	acpa[min] ) ) != 0 )
    {
      /* Perform host independent initialization */
      our_rc = EINVAL;
      acpaerr(ERRID_ACPA_INITZ,	"sysxacpa", min, __LINE__,"cfginit","acpainitlz",our_rc,NULL,NULL);
      break;
    }

    if ( config_count == 0 )
    {
      /* Get the microcode images from the files */
      our_rc = load_ucode_into_memory(aptr->dds.mcode[1],&record_ucode);
      if ( our_rc ) {
	acpaerr(ERRID_ACPA_LOAD, "sysxacpa", min, __LINE__,"cfginit","load_ucode_into_memory",our_rc,aptr->dds.mcode[1],"ADPCM record");
	switch( our_rc )
	{
	  /* Deallocate any previously malloced memory if any microcode load */
	  /* failed. */
	  case -3 :
	    our_rc = ENOMEM;
	    break;
          case -4 :
          case -5 :
            /* Free up the memory allocated to the microcode image. */
            xmfree( record_ucode.data, kernel_heap );
	  default :
	    our_rc = EIO;
	    break;
        }
	break;
      }
      our_rc = load_ucode_into_memory(aptr->dds.mcode[0],&playback_ucode);
      if ( our_rc ) {
	acpaerr(ERRID_ACPA_LOAD, "sysxacpa", min, __LINE__,"cfginit","load_ucode_into_memory",our_rc,aptr->dds.mcode[0],"ADPCM playback");
	switch( our_rc )
	{
	  /* Deallocate any previously malloced memory if any microcode load */
	  /* failed. */
	  case -3 :
	    our_rc = ENOMEM;
	    break;
          case -4 :
          case -5 :
            /* Free up the memory allocated to the microcode image. */
            xmfree( playback_ucode.data, kernel_heap );
	  default :
	    our_rc = EIO;
	    break;
        }
        /* Free up the memory allocated to the microcode images. */
        xmfree( record_ucode.data, kernel_heap );
	break;
      }
      our_rc = load_ucode_into_memory(aptr->dds.mcode[3],&record_pcm_ucode);
      if ( our_rc ) {
	acpaerr(ERRID_ACPA_LOAD, "sysxacpa", min, __LINE__,"cfginit","load_ucode_into_memory",our_rc,aptr->dds.mcode[3],"PCM record");
	switch( our_rc )
	{
	  /* Deallocate any previously malloced memory if any microcode load */
	  /* failed. */
	  case -3 :
	    our_rc = ENOMEM;
	    break;
          case -4 :
          case -5 :
            /* Free up the memory allocated to the microcode image. */
            xmfree( record_pcm_ucode.data, kernel_heap );
	  default :
	    our_rc = EIO;
	    break;
        }
        /* Free up the memory allocated to the microcode images. */
        xmfree( record_ucode.data, kernel_heap );
        xmfree( playback_ucode.data, kernel_heap );
	break;
      }
      our_rc = load_ucode_into_memory(aptr->dds.mcode[2],&playback_pcm_ucode);
      if ( our_rc ) {
	acpaerr(ERRID_ACPA_LOAD, "sysxacpa", min, __LINE__,"cfginit","load_ucode_into_memory",our_rc,aptr->dds.mcode[2],"PCM playback");
	switch( our_rc )
	{
	  /* Deallocate any previously malloced memory if any microcode load */
	  /* failed. */
	  case -3 :
	    our_rc = ENOMEM;
	    break;
          case -4 :
          case -5 :
            /* Free up the memory allocated to the microcode image. */
            xmfree( playback_pcm_ucode.data, kernel_heap );
	  default :
	    our_rc = EIO;
	    break;
        }
        /* Free up the memory allocated to the microcode images. */
        xmfree( record_ucode.data, kernel_heap );
        xmfree( playback_ucode.data, kernel_heap );
        xmfree( record_pcm_ucode.data, kernel_heap );
	break;
      }
      our_rc = load_ucode_into_memory(aptr->dds.mcode[5],&record_22_ucode);
      if ( our_rc ) {
	acpaerr(ERRID_ACPA_LOAD, "sysxacpa", min, __LINE__,"cfginit","load_ucode_into_memory",our_rc,aptr->dds.mcode[5],"22 ADPCM record");
	switch( our_rc )
	{
	  /* Deallocate any previously malloced memory if any microcode load */
	  /* failed. */
	  case -3 :
	    our_rc = ENOMEM;
	    break;
          case -4 :
          case -5 :
            /* Free up the memory allocated to the microcode image. */
            xmfree( record_22_ucode.data, kernel_heap );
	  default :
	    our_rc = EIO;
	    break;
        }
        /* Free up the memory allocated to the microcode images. */
        xmfree( record_ucode.data, kernel_heap );
        xmfree( playback_ucode.data, kernel_heap );
        xmfree( record_pcm_ucode.data, kernel_heap );
        xmfree( playback_pcm_ucode.data, kernel_heap );
	break;
      }
      our_rc = load_ucode_into_memory(aptr->dds.mcode[4],&playback_22_ucode);
      if ( our_rc ) {
	acpaerr(ERRID_ACPA_LOAD, "sysxacpa", min, __LINE__,"cfginit","load_ucode_into_memory",our_rc,aptr->dds.mcode[4],"22 ADPCM playback");
	switch( our_rc )
	{
	  /* Deallocate any previously malloced memory if any microcode load */
	  /* failed. */
	  case -3 :
	    our_rc = ENOMEM;
	    break;
          case -4 :
          case -5 :
            /* Free up the memory allocated to the microcode image. */
            xmfree( playback_22_ucode.data, kernel_heap );
	  default :
	    our_rc = EIO;
	    break;
        }
        /* Free up the memory allocated to the microcode images. */
        xmfree( record_ucode.data, kernel_heap );
        xmfree( playback_ucode.data, kernel_heap );
        xmfree( record_pcm_ucode.data, kernel_heap );
        xmfree( playback_pcm_ucode.data, kernel_heap );
        xmfree( record_22_ucode.data, kernel_heap );
	break;
      }

      /* Create	and add	the switch table if this has not been done */
      /* already. */
      aptr->dev.d_open = acpaopen;     /* Called to handle open	calls */
      aptr->dev.d_close	= acpaclose;   /* Called to handle close calls */
      aptr->dev.d_read = acparead;     /* Called to handle read	calls */
      aptr->dev.d_write	= acpawrite;   /* Called to handle write calle */
      aptr->dev.d_ioctl	= acpaioctl;   /* Called to handle ioctl calls */
      aptr->dev.d_strategy = nodev;    /* No strategy routine, char mode only */
      aptr->dev.d_select = nodev;      /* No select routine, lazy */
      aptr->dev.d_config = acpaconfig; /* This routine */
      aptr->dev.d_print	= nodev;       /* No print routine */
      aptr->dev.d_dump = nodev;	       /* Can't	dump system to voice */
      aptr->dev.d_mpx =	acpampx;       /* Called to handle multiplexing	*/
      aptr->dev.d_revoke = nodev;      /* ACPA not in TCB */
      if ( ( our_rc = devswadd(	devno, &aptr->dev ) ) != 0 )
      {
        /* Free up the memory allocated to the microcode images. */
        xmfree( record_ucode.data, kernel_heap );
        xmfree( playback_ucode.data, kernel_heap );
        xmfree( record_pcm_ucode.data, kernel_heap );
        xmfree( playback_pcm_ucode.data, kernel_heap );
        xmfree( record_22_ucode.data, kernel_heap );
        xmfree( playback_22_ucode.data, kernel_heap );
        our_rc = EINVAL;
	break;			  /* Couldn't add to system, quit now */
      }
    }

    /* Ensure the initial state	of the tracks. */
    tptr2->in_use = tptr1->in_use = FALSE;
    tptr2->current_state = tptr1->current_state	= Uninitialized;
    tptr2->rinfo.srate = tptr1->rinfo.srate = 5500;
    tptr2->rinfo.bits_per_sample =tptr1->rinfo.bits_per_sample = 8;
    tptr2->rinfo.bsize = tptr1->rinfo.bsize = 576;
    tptr2->rinfo.mode =	tptr1->rinfo.mode = ADPCM;
    tptr2->rinfo.channels = tptr1->rinfo.channels = 1;
    tptr2->rinfo.position_resolution = tptr1->rinfo.position_resolution	=
      100;	/* assume 5.5 kHz ADPCM	*/
    tptr2->rinfo.flags = tptr1->rinfo.flags = FIXED;
    tptr2->rinfo.operation = tptr1->rinfo.operation = UNINITIALIZED;
    tptr2->ainfo.change.input =	tptr1->ainfo.change.input = HIGH_GAIN_MIKE;
    tptr2->ainfo.change.output = tptr1->ainfo.change.output = OUTPUT_1;
    tptr2->ainfo.change.monitor	= tptr1->ainfo.change.monitor =
      MONITOR_UNCOMPRESSED;
    tptr2->ainfo.change.volume = tptr1->ainfo.change.volume = 0x7fffffff;
    tptr2->ainfo.change.volume_delay = tptr1->ainfo.change.volume_delay	= 0;
    tptr2->ainfo.change.balance	= tptr1->ainfo.change.balance =	0x3fffffff;
    tptr2->ainfo.change.balance_delay =	tptr1->ainfo.change.balance_delay = 0;
    tptr1->ainfo.change.dev_info = &( tptr1->tinfo.master_volume );
    tptr2->ainfo.change.dev_info = &( tptr2->tinfo.master_volume );
    tptr1->tinfo.master_volume = 0x7fff;
    tptr2->tinfo.master_volume = 0x7fff;
    tptr1->tinfo.dither_percent	= tptr2->tinfo.dither_percent =	33;
    tptr1->rnkb	= aptr->dds.MACPA_5r_secs * 10;
    tptr1->wnkb	= aptr->dds.MACPA_5w_secs * 10;
    tptr2->rnkb	= aptr->dds.MACPA_5r_secs * 10;
    tptr2->wnkb	= aptr->dds.MACPA_5w_secs * 10;

    aptr->secondtime = FALSE;
    aptr->doing_what = Nothing;
    aptr->running = FALSE;

    for	( i=0; i<MAXNKB; i++ )
    {
      tptr1->wbuf[i] = NULL;
      tptr2->wbuf[i] = NULL;
      tptr1->rbuf[i] = NULL;
      tptr2->rbuf[i] = NULL;
    }

    /* Flag that a successful configuration INIT has been done.	*/
    config_count++;

    EXIT_ACPA_PTRACE( CFGINIT, ACPA_MAIN, min );

    break;

  case CFG_TERM:

    ENTER_ACPA_PTRACE( CFGTERM,	ACPA_MAIN, min );

    /* We have been called to terminate	the ACPA device	driver.	*/

    if (aptr->base == 0)
      break;  /* Not in	the system, nothing to do to delete it.	*/

    /* Check to	see if any devices are currently open.	If so, exit */
    /* with an error. */
    if ( ( acpa[min].track[0].in_use !=	FALSE )	||
	 ( acpa[min].track[1].in_use !=	FALSE )	)
      /* Because the device is in use, refuse to deconfigure. */
      return( EBUSY );

    /* Shut down the DSP on the	ACPA card if it's running. */

    (void) acpastop( &(	acpa[min] ) );	    /* Stop the	DSP on the ACPA	card */
    /* No devices in use, delete the driver from the system's device */
    /* switch tables via devswdel. */

    if ( config_count == 1 )
    {
      /* Delete	device from system switch table	*/
      j = devswdel( devno );
      if ( j != 0 )
        our_rc = EINVAL;
      /* Free up the memory allocated to the microcode images. */
      j = xmfree( record_ucode.data, kernel_heap );
      if ( j != 0 )
        our_rc = EFAULT;
      j = xmfree( playback_ucode.data, kernel_heap );
      if ( j != 0 )
        our_rc = EFAULT;
      j = xmfree( record_pcm_ucode.data, kernel_heap );
      if ( j != 0 )
        our_rc = EFAULT;
      j = xmfree( playback_pcm_ucode.data, kernel_heap );
      if ( j != 0 )
        our_rc = EFAULT;
      j = xmfree( playback_22_ucode.data, kernel_heap );
      if ( j != 0 )
        our_rc = EFAULT;
      j = xmfree( record_22_ucode.data, kernel_heap );
      if ( j != 0 )
        our_rc = EFAULT;
    }

    /* Ensure that all kernel memory is	returned to the	system.	*/
    for	( i=0; i<MAXNKB; i++ )
    {
      if ( tptr1->wbuf[i] != NULL )
      {
	(void) FREE_MEMORY( tptr1->wbuf[i] );
	tptr1->wbuf[i] = NULL;
      }
      if ( tptr2->wbuf[i] != NULL )
      {
	(void) FREE_MEMORY( tptr2->wbuf[i] );
	tptr2->wbuf[i] = NULL;
      }
      if ( tptr1->rbuf[i] != NULL )
      {
	(void) FREE_MEMORY( tptr1->rbuf[i] );
	tptr1->rbuf[i] = NULL;
      }
      if ( tptr2->rbuf[i] != NULL )
      {
	(void) FREE_MEMORY( tptr2->rbuf[i] );
	tptr2->rbuf[i] = NULL;
      }
    }

    config_count--;

    EXIT_ACPA_PTRACE( CFGTERM, ACPA_MAIN, min );

    break;

#ifdef MORE
  /* THIS IS NO	LONGER NEEDED BECAUSE THERE IS NO VPD */
  case CFG_QVPD:

    /* We have been called to query the	device driver, but ... */
    /* there is	no vital product data on the ACPA. */
    break;
#endif

  default:
    break;
  }

  return our_rc;		/* Return error	to caller if one occurred */

}

/*----------------------------------------------------------------------
 * Function name:  acpaopen
 *
 * Function:	   Process the open system call	
 * 
 * Operation:
 *
 * 1.  If no ACPA card is present, return ENXIO	error.
 * 2.  If an unsupported option	is specified return an EINVAL error.
 * 3.  If trying to read a write-only device or	vice versa, return an
 *     EINVAL error.
 * 4.  If the combination of options is	not supported, return EINVAL.
 * 
 * 7.  If the channel(s) are already in	use, return EBUSY error.
 * 8.  If the wrong microcode is loaded	and in use, return EBUSY error.
 *     (Can't open the right channel for recording if the left channel
 *	is already open	for playback, for example).
 *
 * 20. Mark the	channel(s) in use.
 * 21. 
 *     
 * 
 *----------------------------------------------------------------------*/

#ifdef i386
int acpaopen(dev, flag,	ext)
dev_t dev;			/* Major and minor device numbers */
int flag;			/* FREAD, FWRITE or similar from open */
caddr_t	ext;			/* Parameter from openx	if any */
#else
int acpaopen(dev, flag,	mpxchan, ext)
dev_t dev;			/* Major and minor device numbers */
ulong flag;			/* DREAD, DWRITE or similar from open */
int mpxchan;			/* Not used, not a multiplexed device */
int ext;			/* Extension, not supported */
#endif /* i386 */
{
  int min;			/* Minor device	number */
  Shorts* code_to_load = NULL;	/* What	set of microcode must we load? */
  int rc;			/* Return code from called routines */
  int i;
  int j;
  register int mpxchannel;	/* the transformed mpx channel number */
  int otherchan;
  int ctlflag;			/* flags whether this is a ctl file */
  register struct acpa_status *aptr; /*	ptr to current adapter's status	*/
  register Track *tptr;
  int pid;			/* process ID of caller	*/
  unsigned int test_flag = ~( O_NDELAY | FREAD | FWRITE	);
  int oldnkb;
  extern int acpa_diag_handler();	/* diagnostics interrupt handler */

  ENTER_ACPA_PTRACE( OPEN, ACPA_MAIN, minor(dev) );

  mpxchannel = ( mpxchan & 3 ) - 1;	/* this	is used	as an index */
  ctlflag = mpxchan & ACPA_CTL;		  /* is	this a ctl request? */
  min =	minor(dev);		/* Get minor device number */

  aptr = (struct acpa_status *)	&( acpa[min] );
  tptr = &( acpa[min].track[mpxchannel]	);

  /* If	this is	a diagnostics call, register its interrupt, etc. */
  if ( mpxchan & ACPA_DIAGS )
  {
    tptr->in_use = TRUE;
    tptr->current_state	= Opened;
    tptr->pid =	getpid();

    /* The interrupt handler needs to be installed for this adapter. */

    /* In the V3 environment, we have to make sure this	code is	all */
    /* pinned in real memory. */
    if ( (rc = pincode(	aptr ))	!= 0 )
    {
      RETURN(ENOMEM);		/* No memory, can't open the device */
    }

    /* Fill in the intr	structure.  See	sys/intr.h for a description of	*/
    /* the fields. */
    aptr->intr_info.handler.next = 0;
    /* Set the address of the interrupt	routine. */
    aptr->intr_info.handler.handler = acpa_diag_handler;
    /* ACPA card is on MCA bus.	*/
    aptr->intr_info.handler.bus_type = BUS_MICRO_CHANNEL;
    aptr->intr_info.handler.flags = 0;
    /* Specify the interrupt level. */
    aptr->intr_info.handler.level = aptr->dds.int_level;
    /* Very high interrupt priority! */
    aptr->intr_info.handler.priority = aptr->dds.priority;
    /* Setup bus access	*/
    aptr->intr_info.handler.bid	= BUS_ID;
    aptr->intr_info.min	= min;	    /* Remember	the minor number */

    /* Register	the interrupt handler */
    rc = i_init( &(aptr->intr_info) );
    if ( rc != INTR_SUCC )
    {
      /* Severe	error -- the interrupt handler can't be	defined. */
      RETURN( EIO );
    }
#ifdef ACPA_MEMORY_TRACE
    if ( ( xxptr[min] =	GET_MEMORY( sizeof(int)*0x1000 ) ) == NULL )
    {
      printf("Failed getting xxptr[min]\n");
      RETURN( -1 );
    }
    txxptr[min]	= xxptr[min];
    *xxptr[min]++ = 0xaaaabbbb;
    *xxptr[min]++ = 0;
    for	( i=0; i<(0x1000-2); i++ )
      *xxptr[min]++ = -1;
    xxptr[min] = txxptr[min] + 2;
    *xxptr[min]++ = min;
    *xxptr[min]++ = mpxchannel;
    /*printf("debug address is %x\n", txxptr[min]	); */
#endif

    RETURN( 0 );
  }

  /* If	this is	a ctl file, just do the	necessary tests	for it.	*/
  if ( ctlflag )
  {
    /* Determine whether any invalid flags have	been specified */

    if ( flag &	test_flag )
      /* Flags other than O_NDELAY, FREAD, and FWRITE were specified. */
      RETURN( EINVAL );

    if ( ( flag	& FREAD	) && ( flag & FWRITE ) )
      /* O_RDWR	is not permitted yet. */
      RETURN( EINVAL );

    if ( !( flag & FWRITE ) )
      /* The application must specify write. */
      RETURN( EINVAL );

    RETURN( 0 );
  }

  /* Reject the	open if	the device has already been opened. */
  if ( tptr->in_use != FALSE )
    RETURN( EINVAL );

  /* Determine whether any invalid flags have been specified */

  if ( flag & test_flag	)
    /* Flags other than	O_NDELAY, FREAD, and FWRITE were specified. */
    RETURN( EINVAL );

  if ( !( flag & FREAD ) && !( flag & FWRITE ) )
    /* The application must specify either read	or write. */
    RETURN( EINVAL );

  if ( ( flag &	FREAD )	&& ( flag & FWRITE ) )
    /* O_RDWR is not permitted yet. */
    RETURN( EINVAL );
  else
    /* store the flags used to open the	device */
    tptr->open_flags = flag;

  /* Set the state of the adapter.  Ensure that	a request for two */
  /* tracks simultaneously uses	the same mode.	But only one record */
  /* operation at a time is allowed. */
  if ( flag & FREAD )
    if ( ( aptr->doing_what == Playing ) ||
       ( aptr->doing_what == Recording ) )
      RETURN( EINVAL );
    else
      aptr->doing_what = Recording;
  else if ( flag & FWRITE )
    if ( ( aptr->doing_what != Nothing ) &&
       ( aptr->doing_what != Playing ) )
      RETURN( EINVAL );
    else
      aptr->doing_what = Playing;

  /* Ensure that the system knows that no microcode is loaded */
  /* on	the adapter at this time. */
  /* THIS CONFLICTS WITH INITING AT CONFIG TIME, WHERE NO UCODE	IS */
  /* LOADED.  WHERE SHOULD IT BE LOADED?  OTHERWISE THERE'S A PROBLEM */
  /* IN	ACPA_START (IF INIT WASN'T CALLED). */
  if ( aptr->secondtime	!= TRUE	)
    aptr->ucode_loaded = NULL;

  /* If no interrupts occur within 5 seconds, something is seriously */
  /* wrong with	the card, since	it interrupts at least every 1/10th of */
  /* a second. */
  aptr->timeout = 5;

  if ( ( aptr->track[0].in_use == FALSE	) &&
       ( aptr->track[1].in_use == FALSE	) )
  {
    /* The interrupt handler needs to be installed for this adapter. */

#ifdef i386
    TRACE(("ACPAopen defining interrupt	handler, irq=%d\n",
		acpa.irqlevel));
    (void) intrattach(acpaintr,	acpa.irqlevel, SPL_CLKONLY);
#else
    /* In the V3 environment, we have to make sure this	code is	all */
    /* pinned in real memory. */

    if ( (rc = pincode(	aptr ))	!= 0 )
    {
      acpaerr(ERRID_ACPA_MEM, "sysxacpa", min, __LINE__, "acpaopen","pincode",rc,NULL,NULL);
      RETURN(ENOMEM);		/* No memory, can't open the device */
    }

    /* Fill in the intr	structure.  See	sys/intr.h for a description of	*/
    /* the fields. */
    aptr->intr_info.handler.next = 0;
    /* Set the address of the interrupt	routine. */
    aptr->intr_info.handler.handler = acpaintr;
    /* ACPA card is on MCA bus.	*/
    aptr->intr_info.handler.bus_type = BUS_MICRO_CHANNEL;
    /* No flags?  Maybe	INTR_NOT_SHARED? */
    aptr->intr_info.handler.flags = 0;
    /* Specify the interrupt level. */
    aptr->intr_info.handler.level = aptr->dds.int_level;
    /* Very high interrupt priority! */
    aptr->intr_info.handler.priority = aptr->dds.priority;
    /* Setup bus access	*/
    aptr->intr_info.handler.bid	= BUS_ID;
    aptr->intr_info.min	= min;	    /* Remember	the minor number */

    /* Register	the interrupt handler */
    rc = i_init( &(aptr->intr_info) );
    if ( rc != INTR_SUCC )
    {
      /* Severe	error -- the interrupt handler can't be	defined. */
      acpaerr(ERRID_ACPA_INTR1,	"sysxacpa", min, __LINE__, "acpaopen","i_init",rc,NULL,NULL);
      RETURN( EIO );
    }
  }

  /* initialize	the sleep channel and the watchdog timer */
  tptr->watch_it.wd.next = NULL;
  tptr->watch_it.wd.prev = NULL;
  tptr->watch_it.wd.func = timed_out;
  tptr->watch_it.wd.restart = aptr->timeout;	  /* N.	seconds	for watchdog */
  tptr->watch_it.wd.count = 0;
  tptr->watch_it.min = min;
  tptr->watch_it.mpxchannel = mpxchannel;
  w_init (&tptr->watch_it);

  /* initialize write flag to indicate no current write activity */
  tptr->write_flag = FALSE;

#endif /* i386 */

  /* Ensure that the system knows that no queue	buffers	have been */
  /* allocated yet for this track. */
  for (	i=0; i<MAXNKB; i++ )
  {
    if ( tptr->wbuf[i] != NULL )
    {
      (void) FREE_MEMORY( tptr->wbuf[i]	);
      tptr->wbuf[i] = NULL;
    }
    if ( tptr->rbuf[i] != NULL )
    {
      (void) FREE_MEMORY( tptr->rbuf[i]	);
      tptr->rbuf[i] = NULL;
    }
  }

  /* The track is now in use, so mark it appropriately.	*/
  tptr->in_use = TRUE;
  tptr->current_state =	Opened;

  /* Get the process id	of the process performing this open. */
  tptr->pid = getpid();

#ifdef ACPA_MEMORY_TRACE
    if ( ( xxptr[min] =	GET_MEMORY( sizeof(int)*0x1000 ) ) == NULL )
    {
      printf("Failed getting xxptr[min]\n");
      RETURN( -1 );
    }
    txxptr[min]	= xxptr[min];
    *xxptr[min]++ = 0xaaaabbbb;
    *xxptr[min]++ = 0;
    for	( i=0; i<(0x1000-2); i++ )
      *xxptr[min]++ = -1;
    xxptr[min] = txxptr[min] + 2;
    *xxptr[min]++ = min;
    *xxptr[min]++ = mpxchannel;
    printf("debug address is %x\n", txxptr[min]       );
    /*brkpoint(	txxptr[min] );	     */
#endif

  EXIT_ACPA_PTRACE( OPEN, ACPA_MAIN, min );

  /* All done, return to caller	in triumph */
  RETURN( 0 );

}



/*----------------------------------------------------------------------
 * Function name:  acpaclose
 *
 * Function:	   Process the close system call
 * 
 *----------------------------------------------------------------------*/

#ifdef i386
int acpaclose(dev, flag, ext)
dev_t dev;			/* Major and minor device numbers */
int flag;			/* FREAD or similar from the last open */
caddr_t	ext;			/* Parameter from openx	if any */
#else
int acpaclose(dev, mpxchan, ext)
dev_t dev;			/* Major and minor device numbers */
int mpxchan;			/* Ignored, not	a multiplexed device */
int ext;			/* Extension from openx	(not supported)	*/
#endif /* i386 */
{
  int min;			/* Minor device	number */
  int channel;			/* Channel number from the minor device	*/
  int source;			/* In/out source from the minor	device */
  int speed;			/* Speed from the minor	device */
  int card;			/* Card	from the minor device */
  int i, j;			/* Loop	counters */
  int oldspl;			/* Old interrupt enable	level */
  int timedout;			/* Timeout counter */
  int rc;			/* Return code from called routines */
  Track	*tptr;
  Track	*tptr2;			/* Track definition block ptr */
  register int mpxchannel;	/* the transformed mpx channel number */
  int ctlflag;			/* flags whether this is a ctl file */
  void acpa_remove_leftover_requests();

  ENTER_ACPA_PTRACE( CLOSE, ACPA_MAIN, minor(dev) );

  mpxchannel = ( mpxchan & 3 ) - 1;	/* this	is used	as an index */
  ctlflag = mpxchan & ACPA_CTL;		/* is this a ctl flag? */
  min =	minor(dev);			/* Get the minor device	number */

  if ( ctlflag )
    RETURN( 0 );

  /* Figure out	which channel is being closed and find its block. */
  tptr = (Track	*) &( acpa[min].track[mpxchannel] );
  if ( mpxchannel == 0 )
    tptr2 = (Track *) &( acpa[min].track[1] );
  else
    tptr2 = (Track *) &( acpa[min].track[0] );

  /* Make sure that the	channel	was really in use, if not abend. */
  
  if (tptr->in_use != TRUE) {
    TRACE(("ACPAclose closing channel not in use!\n"));
    RETURN(0);
  }

  /* If	this is	a diagnostics call, just set the flags,	stop the adapter, */
  /* and return. */
  if ( mpxchan & ACPA_DIAGS )
  {
    (void) acpastop( &(	acpa[min] ) );	      /* Reset the DSP */
    (void) i_clear( &(acpa[min].intr_info) );
    /* Unpin all the code and data in this module */
    (void) unpincode( &(acpa[min]) );
    tptr->in_use = FALSE;
    tptr->current_state	= Uninitialized;
#ifdef ACPA_MEMORY_TRACE
  (void) FREE_MEMORY( txxptr[min] );
#endif
    RETURN( 0 );
  }

  /* Only the process that opened a track can close it.	*/
  if ( tptr->pid != getpid() )
    RETURN( EINVAL );

  /* Operation is different for	playback and for recording.  */

  if (acpa[min].doing_what == Playing) {

    /* One odd case is if the device was opened	for output, but	*/
    /* nothing was ever	written	to it.	Since on playback we defer */
    /* starting	the DSP	until the first	write, an open followed	by a */
    /* close must be treated specially (the DSP	won't be running yet). */
    /* In this case just mark the DSP as stopped and not in use	and */
    /* exit. */

    if ( tptr->current_state <=	Almost_Started )
    {
      /* Stop the DSP only if neither track is running.	*/
      if ( tptr2->current_state	<= Almost_Started )
	(void) acpastop( &( acpa[min] )	);	  /* Reset the DSP */
    }

    else {
      
      /* If source mix is in use on this track, turn it off. */
      if ( tptr->rinfo.mode == SOURCE_MIX )
	if ( mpxchannel == 0 )
	  acpaput( PCM_T1_CONTROL_PARMS + PCM_SOURCE_MIX,
		   (unsigned short) 0, min );
	else
	  acpaput( PCM_T2_CONTROL_PARMS + PCM_SOURCE_MIX,
		   (unsigned short) 0, min );

      /* During playback, see if there are samples waiting to be played. */
      /* If not, shut down the DSP immediately.	 If so,	just set the */
      /* "closing" flag; the interrupt handler will stop the DSP when */
      /* the remaining samples are played. */
      
      if ( tptr->current_state == Pausing )
      {
	/* Stop the adapter only if the other track is not in use. */
	if ( tptr2->current_state == Uninitialized )
	  /* A signal might have been received while pausing, so stop */
	  /* the adapter if the other track is not in use. */
	  (void) acpastop( &( acpa[min] ) );
	/* Prevent the close from never completing. */
	acpa[min].closing = FALSE;
      }
      else
	acpa[min].closing = TRUE;

      /* Wait here until the DSP is stopped by the interrupt handler */

      oldspl = DISABLE_INTERRUPTS(); /*	Disable	all interrupts */
      acpa[min].recursion = 1;	       /* If intrupt occurs scream very	loudly */
#ifdef ACPA_MEMORY_TRACE
#endif
      for (;;) {
	timedout = 0;
	if (acpa[min].closing == FALSE)
	  break;	/* Don't sleep,	DSP already stopped */
	/* Sleep waiting on the	stop event word	*/
	tptr->waiting =	TRUE;		    /* We're waiting, if anyone	asks...	*/
#ifdef ACPA_MEMORY_TRACE
#endif
	TIMED_SLEEP(&tptr->stop_event, rc, STOP_EVENT);	 /* Wait for event or */
#ifdef ACPA_MEMORY_TRACE
#endif
						       /* for several seconds*/
	tptr->waiting =	FALSE;	     /*	Not waiting any	more, if anyone	asks...	*/
	if (rc == SLEEP_BROKEN_BY_SIGNAL) {
	  timedout++;
	  break;
	}
	if (rc == SLEEP_TIMED_OUT) {
	  timedout++;
	  break;
	}
      }

      if ( timedout )
      {
	/* Don't turn off adapter if the other track is in use. */
	if ( tptr2->current_state == Uninitialized )
	  /* The adapter may not have been stopped, so ensure it is now. */
	  (void) acpastop( &( acpa[min] ) );
	acpa[min].closing = FALSE;
      }

      acpa[min].recursion = FALSE;        /* Interrupts are OK again */
      (void) ENABLE_INTERRUPTS(oldspl);	/* Enable interrupts again */

    }

  }
    
  else {
    /* If we are recording, just stop the DSP immediately. */
    (void) acpastop( &(	acpa[min] ) );
  }

  /* Ensure that all queue buffers are released	and the	pointers NULLed. */
  for (	i=0; i<MAXNKB; i++ )
  {
    if ( acpa[min].track[mpxchannel].wbuf[i] !=	NULL )
    {
      (void) FREE_MEMORY( acpa[min].track[mpxchannel].wbuf[i] );
      acpa[min].track[mpxchannel].wbuf[i] = NULL;
    }
    if ( acpa[min].track[mpxchannel].rbuf[i] !=	NULL )
    {
      (void) FREE_MEMORY( acpa[min].track[mpxchannel].rbuf[i] );
      acpa[min].track[mpxchannel].rbuf[i] = NULL;
    }
  }

  /* De-register the interrupt handler from the	kernel */

#ifdef i386
  (void) intrdetach(acpaintr, SPL_CLKONLY);
#else

  /* clear the watchdog	timer */
  w_clear (&tptr->watch_it);

  /* Get rid of	the interrupt handler and disable the h/w interrupt level */
  /* (if this is the only use of it). */
  if ( ( tptr->in_use == FALSE ) ||
       ( tptr2->in_use == FALSE	) )
  {
    /* Because only one	track is currently in use, the handler can be */
    /* deregistered now. */
    (void) i_clear( &(acpa[min].intr_info) );
    /* Unpin all the code and data in this module */
    (void) unpincode( &(acpa[min]) );

    /* Because both tracks are now closed, the state can be reset. */
    acpa[min].doing_what = Nothing;

    /* CORRECT HERE? */
    acpa[min].ps2_speaker_enabled = FALSE;    /* The internal speaker isn't working */
  }
#endif /* i386 */

  tptr->in_use = FALSE;
  tptr->current_state =	Uninitialized;
  tptr->rinfo.operation	= UNINITIALIZED;

  /* Get rid of	any enqueued requests. */
  acpa_remove_leftover_requests( min, mpxchannel );

  EXIT_ACPA_PTRACE( CLOSE, ACPA_MAIN, min );

#ifdef ACPA_MEMORY_TRACE
  /*brkpoint( txxptr[min], 0xabcd );    */
  (void) FREE_MEMORY( txxptr[min] );
#endif

  RETURN( 0 );			/* Return to caller, close is complete */

}

int acpa_diag_handler(status)
struct intr_status *status;	/* interrupt handler information */
{
  int min;			/* the device's	minor number */
  register struct acpa_status *aptr; /*	ptr to current adapter's status	*/
  int stat_reg;			/* Contents of the status register */
  unsigned char	cmd;		/* Command to send to the ACPA card */
  int i;

  ENTER_ACPA_PTRACE( INTERRUPT,	ACPA_MAIN, min );

  min =	status->min;
  aptr = (struct acpa_status *)	&( acpa[min] );
#ifdef ACPA_MEMORY_TRACE
  if ( xxptr[min] > ( txxptr[min] + 0x1000 - 400 ) )
  {
    xxptr[min] = txxptr[min] + 1;
    *xxptr[min]++ += 1;	/* This	is the restart count */
    for	( i=0; i<(0x1000-2); i++ )
      *xxptr[min]++ = -1;
    xxptr[min] = txxptr[min] + 2; /* skip the restart count */
    *xxptr[min]++ = 0xabcd1111;
  }
  *xxptr[min]++	= aptr->int_count;
  *xxptr[min]++	= 0xf1f3;
#endif

  if (aptr->base == 0)
    RETURN(INTR_FAIL); /* No ACPA card in system */
 
#ifdef ACPA_MEMORY_TRACE
  *xxptr[min]++	= 0xf1f5;
#endif
  /*----------------------------------------------------------------------
   * Determine if the interrupt	is due to the ACPA card.
   * If	not, just exit immediately.
   *----------------------------------------------------------------------*/

  stat_reg = acpainb(aptr->cmd_stat_reg);    /*	Get status reg contents	*/

  if ((stat_reg	& HST_REQ) == 0) RETURN(INTR_FAIL);  /*	Interrupt */
						     /*	wasn't from */
						     /*	the ACPA card */
#ifdef ACPA_MEMORY_TRACE
  *xxptr[min]++	= 0xf1f6;
#endif

  aptr->int_count++;		  /* One more interrupt	has occurred */

  /*----------------------------------------------------------------------
   * Clear the interrupt from the ACPA card.
   * This tells	the ACPA card that we have serviced the	interrupt and
   * insures that we don't confuse ourselves the next time we enter
   * (we would think that an interrupt is still	pending	which has been
   * serviced).
   * Clear the interrupt by setting HREQACK to zero in the Command register.
   *----------------------------------------------------------------------*/

  cmd =	0x00;				       /* No bits on in	command	*/
  cmd |= TMS_RES;		    /* Keep the	TMS processor running */
  cmd |= HINTENA;		 /* ACPA can still interrupt the host */

  acpaoutb(aptr->cmd_stat_reg, cmd);	     /*	Clear the interrupt */
#ifdef ACPA_MEMORY_TRACE
  *xxptr[min]++	= 0xf1f7;
#endif

  /* Clear the interrupt level on the bus (V3 only) */
  i_reset(status);	       /* Reset	the interrupt on the bus */
#ifdef ACPA_MEMORY_TRACE
  *xxptr[min]++	= 0xf1f8;
#endif

  EXIT_ACPA_PTRACE( INTERRUPT, ACPA_MAIN, min );

#ifdef ACPA_MEMORY_TRACE
  *xxptr[min]++	= aptr->int_count;
  *xxptr[min]++	= 0xf1f9;
#endif
  RETURN(INTR_SUCC);		/* Return to kernel successfully */

}

/*----------------------------------------------------------------------
 * Function name:  acpaintr
 *
 * Function:	   ACPA	Interrupt routine
 *
 * Environment:
 *		   An interrupt	has occurred on	the IRQ	level
 *		   that	the ACPA card is using.	 This does NOT
 *		   guarantee that the interrupt	came from the
 *		   ACPA	card, as the IRQ level may be shared.
 *
 *
 * Operation:
 *
 * 1.  Verify that the interrupt came from the ACPA card.  Examine
 *     the Host	Request	bit of the Host	Status Register	to see if
 *     the interrupt came from the ACPA	Card.
 *
 * nn. Clear the interrupt from	the ACPA card and exit.
 *
 *----------------------------------------------------------------------*/

#ifdef i386
int acpaintr(vec_num)
int vec_num;			/* Interrupt vector number */
#else
int acpaintr(status)
struct intr_status *status;    /* Interrupt handler information	*/
#endif /* i386 */
{
  int stat_reg;			/* Contents of the status register */
  unsigned char	cmd;		/* Command to send to the ACPA card */
  int trk;			/* Which track interrupted us (0 or 1) */
  int trkbox;			/* Mailbox base	address	in DSP memory */
  int count;			/* Buffer count	*/
  int hostbufptr;		/* Host	buffer ptr */
  int dspbufptr;		/* DSP's buffer	pointer	*/
  int temp;			/* Temporary */
  int temp2;			/* Another temporary */
  int remain;			/* N. remaining	*/
  int ntofill;			/* N of	buffers	to fill	*/
  unsigned short* t;
  int i;			/* Loop	counter	*/
  int min;			/* the device's	minor number */
  register struct acpa_status *aptr; /*	ptr to current adapter's status	*/
  register Track *tptr;		/* pointer to current track info */
  int counter;
  void acpain_mult();
  extern void acpaout_mult();
  unsigned short dsp_buf_count;
  int done;
  int loaded = FALSE;		/* specifies whether preloading	was done */
  int buffer_overflow =	FALSE;
  int old_dsp_ptr;              /* holds previous value of DSP pointer */
  extern void acpa_write_block();
  extern void acpa_update_position_info();

  ENTER_ACPA_PTRACE( INTERRUPT,	ACPA_MAIN, min );

  min =	status->min;

  aptr = (struct acpa_status *)	&( acpa[min] );

  if (aptr->base == 0) RETURN(INTR_FAIL); /* No	ACPA card in system */

  /*----------------------------------------------------------------------
   * Determine if the interrupt	is due to the ACPA card.
   * If	not, just exit immediately.
   *----------------------------------------------------------------------*/

  stat_reg = acpainb(aptr->cmd_stat_reg);    /*	Get status reg contents	*/

  if ((stat_reg	& HST_REQ) == 0) RETURN(INTR_FAIL);  /*	Interrupt */
						     /*	wasn't from */
						     /*	the ACPA card */

  acpaput( DSP_TMS_INT_INHIBIT_FLAG, (unsigned short) TMS_CAN_NOT_INTERRUPT, min );

  aptr->int_count++;              /* One more interrupt has occurred */

#ifdef ACPA_MEMORY_TRACE
  if ( xxptr[min] > ( txxptr[min] + 0x1000 - 400 ) )
  {
    xxptr[min] = txxptr[min] + 1;
    *xxptr[min]++ += 1;	/* This	is the restart count */
    for	( i=0; i<(0x1000-2); i++ )
      *xxptr[min]++ = -1;
    xxptr[min] = txxptr[min] + 2; /* skip the restart count */
    *xxptr[min]++ = 0xabcd1111;
  }
#endif

  /* Make sure we haven't been entered at an uncomfortable time	*/

  if (aptr->recursion != FALSE &&
      aptr->recursion != 1 &&
      aptr->recursion != 200) {
    acpaerr(ERRID_ACPA_INTR2, "sysxacpa", min, __LINE__,"acpaintr","aptr->recursion",0,NULL,NULL);
#ifdef ACPA_MEMORY_TRACE
    *xxptr[min]++ = aptr->recursion;
    *xxptr[min]++ = min;
    *xxptr[min]++ = counter;
    *xxptr[min]++ = 0xaaa2;
#endif
  }

  /* Check each	track for possible data. */
  for (	counter=0; counter<2; counter++	)
  {
    if ( counter == 0 )
       trkbox =	DSP_T1_MAILBOX_BASE;		  /* Get DSP addr of mailbox */
    else
      trkbox = DSP_T2_MAILBOX_BASE;

    tptr = (Track *) &(	aptr->track[counter] );

    /* Don't bother with a track if it is not in Started state.	*/
    if ( ( tptr->current_state == Started ) ||
	 ( tptr->current_state == Stopped ) ||
	 ( tptr->current_state == Pausing ) )
    {
#ifdef ACPA_MEMORY_TRACE
#endif
      /* If we get an interrupt	from a track that's not	in use,	something */
      /* is Very VVEERRYY bizarre.  */
      /* As a possible fixup, assume the interrupt should have come from */
      /* the other track and go	on.  If	the interrupt is REALLY	unusual	*/
      /* print out a bunch of statistics about it. */

      if (tptr->in_use != TRUE)	{
#ifdef ACPA_MEMORY_TRACE
#endif
	acpaerr(ERRID_ACPA_INTR3, "sysxacpa", min, __LINE__,"acpaintr","tptr->in_use",0,NULL,NULL);
	TRACE(("INTERRUPT FROM IDLE CHANNEL!\n"));
	TRACE(("Channel	mailbox	at %x\n", trkbox));
	TRACE(("CMD_FROM_HOST: %x\n", acpaget(trkbox + DSP_TX_CMD_FROM_HOST,min)));
	TRACE(("DSP_BUF_PTR:   %x\n", acpaget(trkbox + DSP_TX_DSP_BUF_PTR,min)));
	TRACE(("CMD_FROM_DSP:  %x\n", acpaget(trkbox + DSP_TX_CMD_FROM_DSP,min)));
	TRACE(("HOST_BUF_PTR:  %x\n", acpaget(trkbox + DSP_TX_HOST_BUF_PTR,min)));
	TRACE(("BUFFER_CNT:    %x\n", acpaget(trkbox + DSP_TX_BUFFER_CNT,min)));
	TRACE(("TMS_STATUS:    %x\n", acpaget(trkbox + DSP_TX_TMS_STATUS,min)));
	TRACE(("LEVEL:	       %x\n", acpaget(trkbox + DSP_TX_LEVEL,min)));
	goto out;
      }

      /* Behavior is different if we're	recording or playing data */

      /* THIS CODE DOES NOT FUNCTION LIKE THE PLAYBACK CODE; IT DOES NOT */
      /* INCREMENT HOST_BUF_PTR, BUT INSTEAD STILL RELIES ON THE CARD */
      /* TO GET THE VALUES.  THIS NEEDS TO CHANGE. */
      if (aptr->doing_what == Recording) {

	/*----------------------------------------*/
	/* RECORDING				  */
	/*----------------------------------------*/

	/* Logic:

	   get # of DSP	slots with unread data
	   for each buffer
	     if	kernel queue is	full then break
	     temp = host buffer	ptr + 1	modulo 4
	     move tempth DSP buffer into the kernel queue
	     set host buffer ptr = temp	in the DSP memory
	     wakeup anyone who is waiting for more data	in the kernel queue
	     end for

	 */

	done = FALSE;
	while (	! done )
	{
	  /* Determine if there	is anything else to do.	*/
	  /* Note that dsp_buf_count and tptr->host_buf_count are both */
	  /* unsigned shorts.  This is very important, because the */
	  /* adapter register turns over from 0xffff to	0.  Therefore the */
	  /* variable must exhibit the same behavior. */
	  dsp_buf_count	= (int)	acpaget( trkbox	+ DSP_TX_BUFFER_CNT, min );
#ifdef ACPA_MEMORY_TRACE
#endif
	  if ( dsp_buf_count !=	tptr->host_buf_count )
	  {
	    /* Ask the adapter which buffer should be read next. */
	    temp = (int) acpaget( trkbox + DSP_TX_HOST_BUF_PTR,	min );
	    if ( ( tptr->current_state != Pausing ) && 
		 ( tptr->current_state != Stopped ) )
	    {
	      /* Read the data.	*/

	      /* If the	kernel queue is	full, reset queue to overwrite data */
	      if (tptr->host_n_bufs >= tptr->rnkb)
	      {
#ifdef ACPA_MEMORY_TRACE
#endif
		/* turn	on the overflow	flag */
		tptr->buffer.flags |= AUDIO_OVERRUN;

		buffer_overflow	= TRUE;
	      }

	      /* Determine which kernel	queue slot we will fill	(put in	temp2)	 */
	      /* If the	buffer has overflowed then reuse the current buffer else */
	      /* use the next buffer.						 */

	      if (buffer_overflow)
		temp2 =	tptr->host_head;
	      else
		temp2 =	(tptr->host_head) + 1;

	      if (temp2	>= tptr->rnkb)
		temp2 -= tptr->rnkb;		   /* Modulo rnkb */

	      /* Move data from	DSP queue into kernel queue */

	      t	= tptr->rbuf[temp2];	/* Now t is the	address	of the */
					/* kernel buffer */
	      acpaout(aptr->addr_low_reg,
		    (unsigned short) tptr->buf_loc[temp]);
#ifdef ACPA_MEMORY_TRACE
#endif
	      /* Transfer one block at a time. */
	      acpain_mult( aptr->data_low_reg, tptr->buf_size, t );

	      /* If the	buffer has overflowed then do NOT update variables.	*/
	      /* Since the overflow is an error	condition there	is no reason to	*/
	      /* try to	keep the position and size information correct.		*/

	      if ( !buffer_overflow)
	      {
		/* Increment number of kernel slots in use */

		tptr->host_n_bufs++;	  /* One more filled read buffer */

		/* Update the max buffer size if this is a bigger value. */
		if ( tptr->host_n_bufs > tptr->buffer.read_buf_max )
		  tptr->buffer.read_buf_max = tptr->host_n_bufs;

		/* Increment the buffer's read buffer size by the number of */
		/* bytes in the	block of data. */
		tptr->buffer.read_buf_size += tptr->rinfo.bsize;

		/* Now update the buffer time field.  Note that	the non-ADPCM */
		/* field contains the number of	bytes, while the ADPCM */
		/* field contains the number of	milliseconds. */
		if ( tptr->rinfo.mode != ADPCM )
		  tptr->buffer.position	+= 560;
		else  /* ADPCM */
                {
                  if (tptr->rinfo.srate == 22050)
		    /* Interrupts occur every 1/20th of a second, */
		    /* so each block contains 50 milliseconds. */
		    tptr->buffer.position += 50;
                  else
		    /* Interrupts occur every 1/10th of a second, */
		    /* so each block contains 100 milliseconds. */
		    tptr->buffer.position += 100;
                }

		/* Bump	kernel queue head pointer */
		tptr->host_head	= temp2;	  /* Move on to	the next slot */
	      }

	      /* Wake up any processes waiting for new data in the kernel */
	      /* queue */

	      if (tptr->waiting	== TRUE) {
		/* If anyone was waiting for space, */
		WAKEUP2(&(tptr->ring_event), RING_EVENT);
					 /* wake 'em up	now */
	      }
	    }	/* end of if not pausing */

	    /* Increment both counters and continue. */
	    tptr->host_buf_count++;
	    /* Increment host buffer pointer in	DSP memory */
	    temp++;

	    if ( temp >= tptr->modulo_factor )
	      temp = 0;
	    (void) acpaput(trkbox+DSP_TX_HOST_BUF_PTR,
			   (unsigned short) temp, min );
	  }
	  else
	  {
	    done = TRUE;
#ifdef ACPA_MEMORY_TRACE
#endif
	  }
	}	/* end while ! done */

      }	/* END OF THE RECORDING	INTERRUPT HANDLER */


      /*----------------------------------------*/
      /* PLAYBACK				*/
      /*----------------------------------------*/


      else {

#ifdef ACPA_MEMORY_TRACE
#endif
	/* See if the ACPA has detected	an underrun.  The command-to-host */
	/* will	be zero	if so.	On the first underrun after a close, stop */
	/* the DSP.  This is done before the pause check to ensure that	*/
	/* the underflow flag will be turned on	(if required) as soon as */
	/* the pause concludes.	*/

	/* Determine whether underflow has occurred. */
	temp = acpaget(trkbox +	DSP_TX_CMD_FROM_DSP, min);

	if ( temp == 0 )
	{
	  /* Underflow has occurred. */
	  tptr->buffer.flags |=	AUDIO_UNDERRUN;
#ifdef ACPA_MEMORY_TRACE
  *xxptr[min]++ = 0x12345678;
  *xxptr[min]++ = ( acpaget(trkbox + DSP_TX_DSP_BUF_PTR, min) << 16 ) 
   | acpaget(trkbox + DSP_TX_HOST_BUF_PTR, min);
  *xxptr[min]++ = ( acpaget(trkbox + DSP_TX_BUFFER_CNT, min) << 16 )
   |  acpaget( PCM_T1_CONTROL_PARMS + PCM_INTERRUPT_COUNTER, min);
  *xxptr[min]++ = ( tptr->host_buf_count << 16 ) | tptr->preload_count; 
#endif
	}
	else
	{
	  tptr->buffer.flags &=	~AUDIO_UNDERRUN;
	}

	/* If the track	is pausing or stopped, do nothing except ensure	that */
	/* the preload counter is updated correctly.  (Since any preloaded */
	/* buffers will	be played during interrupts while the adapter */
	/* is paused, the counter and the buffer position information */
	/* must	be updated.) */
	if ( ( tptr->current_state == Pausing )	|| 
	     ( tptr->current_state == Stopped )	)
	{
	  if ( tptr->preload_count > 0 )
	  {
	    tptr->preload_count--;
	    acpa_update_position_info( tptr, min, counter );
	    tptr->host_buf_count++;     /* note that one more block was  */
					/* processed */
#ifdef ACPA_MEMORY_TRACE
#endif
	  }
	  continue;
	}

	if ( temp == 0 )
	{
	  /* Underflow has occurred. */
	  if ( aptr->closing ==	TRUE )
	  {
#ifdef ACPA_MEMORY_TRACE
#endif
	    if ( counter == 0 )
	    {
	      if ( acpa[min].track[1].current_state <= Stopped )
		(void) acpastop( &( acpa[min] )	);	/* Stop	the DSP	*/
	    }
	    else
	      if ( acpa[min].track[0].current_state <= Stopped )
		(void) acpastop( &( acpa[min] )	);	/* Stop	the DSP	*/
	    aptr->closing = FALSE; /* No longer	closing	*/
	    /* Tell anyone who might care that the DSP has stopped. */
	    /* The close routine is waiting for	this */
	    WAKEUP2(&(tptr->stop_event), STOP_EVENT );
	    continue;
	  }

	  /* Is	there any data in the kernel queue? */
	  if ((	tptr->host_n_bufs > 0 )	&&
	      !((tptr->host_n_bufs == 1) && (tptr->partial_block_left_over)))
	  {
#ifdef ACPA_MEMORY_TRACE
#endif
	    /* Try to preload the adapter again, as is done in write().	*/
	    while ( ( tptr->host_n_bufs	> 0 ) &&
		   !((tptr->host_n_bufs==1)&&(tptr->partial_block_left_over)) &&
		    ( tptr->preload_count < tptr->modulo_factor ) )
	    {
	      acpa_write_block( tptr, aptr, trkbox, min );
	      tptr->preload_count++;
	      /* Because the adapter is in an underrun state, no blocks are */
	      /* processed as interrupts occur.  Therefore host_buf_count */
	      /* is not incremented here. */
	    }
	    if ( tptr->preload_count > 0 )
	      tptr->preload_count -= 1; /* Account for the fact that one */
					/* preloaded buffer will be played */
					/* by the next interrupt. */
	    /* tptr->host_buf_count is not incremented by one here because */
	    /* it is getting reset immediately following. */
#ifdef ACPA_MEMORY_TRACE
#endif

	    /* Now that	data has been preloaded, the host buffer count */
	    /* must be reset to	the value of the DSP buffer counter. */
	    /* This way	the DSP	buffer count will be exactly one greater */
	    /* than the	host buffer count when the next	interrupt occurs, */
	    /* which is	the initial state after	write()	starts the card. */
	    tptr->host_buf_count = (int) acpaget( trkbox + DSP_TX_BUFFER_CNT,
						  min );

            /* ESD CLAIMS THAT THE FIRST UNDERRUN SHOULD INCREMENT THE */
            /* POSITION TO THE FINAL VALUE.  HOWEVER, WAIT RETURNS BEFORE */
            /* THE UNDERRUN OCCURS, SO THE POSITION VALUE IS TOO SMALL. */
            /* NEED TO TRY PUTTING IN FIRST_UNDERRUN FLAG, WHICH WHEN */
            /* HIT UPDATES THE POSITION FIELD, AND MAKING IT WORK WITH WAIT. */
            /* Now update the buffer time field.  (Note that the non-ADPCM */
            /* field contains the number of bytes, while the ADPCM */
            /* field contains the number of milliseconds.)  This is done */
            /* because one block will have played by the next interrupt. */
            if ( tptr->rinfo.mode != ADPCM )
              tptr->buffer.position += 560;
            else  /* ADPCM */
            {
              if (tptr->rinfo.srate == 22050)
	        /* Interrupts occur every 1/20th of a second, */
	        /* so each block contains 50 milliseconds. */
	        tptr->buffer.position += 50;
              else
	        /* Interrupts occur every 1/10th of a second, */
	        /* so each block contains 100 milliseconds. */
	        tptr->buffer.position += 100;
            }

	    /* Restart the adapter from	an underrun state. */
	    (void) acpaput( trkbox + DSP_TX_CMD_FROM_DSP, 5, min );
#ifdef ACPA_MEMORY_TRACE
#endif
	  }
	  else /* DEBUG	*/
	  {
	    /* There is	no data	in the queue, but any preloaded	buffers	*/
	    /* will still play as interrupts occur, so decrement the count */
	    /* and update the buffer position information. */
	    if ( tptr->preload_count > 0 )
	    {
	      tptr->preload_count--;
	      acpa_update_position_info( tptr, min, counter );
	      tptr->host_buf_count++;   /* note that one more block was */
					/* processed */
#ifdef ACPA_MEMORY_TRACE
#endif
	    }

	  }
#ifdef ACPA_MEMORY_TRACE
#endif
	  continue;
	}       /* end of if underflow */

	/* Preload the buffers if they aren't full now.	*/
	if ( tptr->preload_count < tptr->preload_max )
	{
#ifdef ACPA_MEMORY_TRACE
#endif
	  old_dsp_ptr = acpaget( trkbox + DSP_TX_DSP_BUF_PTR, min );
	  /* Try to preload the	adapter	again, as is done in write(). */
	  while	( ( tptr->host_n_bufs >	0 ) &&
		 !((tptr->host_n_bufs==1)&&(tptr->partial_block_left_over)) &&
		  ( tptr->preload_count	< tptr->preload_max ) )
	  {
	    acpa_write_block( tptr, aptr, trkbox, min );
	    tptr->preload_count++;
	    /* tptr->host_buf_count is not incremented here because the */
	    /* blocks are just getting preloaded; they are not getting */
	    /* processed at this time. */
	  }
	  /* Determine if any interrupts occurred while preloading.  Although */          /* ACPA interrupts are turned off while in the interrupt handler, */
	  /* the card still processes blocks.  One or more blocks may have */
	  /* occurred during the above preloading, so that must be taken into */
	  /* account now. */
	  old_dsp_ptr = acpaget( trkbox + DSP_TX_DSP_BUF_PTR, min ) -
			old_dsp_ptr;
	  /* If interrupts occurred and data was preloaded, then reduce the */
	  /* number of preloaded buffers by the number of interrupts.  This */
	  /* is done because one block of preloaded data was played for each */
	  /* interrupt that occurred, even though interrupts were turned off */
	  /* for the duration of the interrupt handler.  Note that this is */
	  /* not done when preloading during an underrun state, since no */
	  /* blocks are being played then (not until the 5 is written to the */
	  /* adapter. */
	  if ( old_dsp_ptr > 0 )
	  {
#ifdef ACPA_MEMORY_TRACE
#endif
	    if ( old_dsp_ptr < tptr->preload_count )
	    {
	      tptr->host_buf_count += old_dsp_ptr;
	      tptr->preload_count -= old_dsp_ptr;
	    }
	  }
	}

	done = FALSE;
	while (	! done )
	{
	  /* Determine whether any more	action should be taken.	*/
	  dsp_buf_count	= (int)	acpaget( trkbox	+ DSP_TX_BUFFER_CNT, min );
#ifdef ACPA_MEMORY_TRACE
#endif
	  if ( dsp_buf_count !=	tptr->host_buf_count )
	  {
	    if ( ( tptr->host_n_bufs > 0 ) &&
		!((tptr->host_n_bufs ==	1) && (tptr->partial_block_left_over)))
	    {
	      /* Data is available, so move one	block onto the adapter.	*/
	      acpa_write_block( tptr, aptr, trkbox, min );
	    }
	    else
	    {
	      /* The next interrupt will have used one preloaded block.	*/
	      tptr->preload_count--;
#ifdef ACPA_MEMORY_TRACE
#endif
	      done = TRUE;
	    }
	    tptr->host_buf_count++;     /* show that one more intr occurred */

	    acpa_update_position_info( tptr, min, counter );

	  }	 /* end	of if counters aren't equal */
	  else
	    done = TRUE;
#ifdef ACPA_MEMORY_TRACE
#endif
	} /* end of while not done */
      }
    }
  }

  /*----------------------------------------------------------------------
   * Clear the interrupt from the ACPA card.
   * This tells	the ACPA card that we have serviced the	interrupt and
   * insures that we don't confuse ourselves the next time we enter
   * (we would think that an interrupt is still	pending	which has been
   * serviced).
   * Clear the interrupt by setting HREQACK to zero in the Command register.
   *----------------------------------------------------------------------*/

out:
#ifdef ACPA_MEMORY_TRACE
#endif
  cmd =	0x00;				       /* No bits on in	command	*/
  if (aptr->running == TRUE)
    cmd	|= TMS_RES;		      /* Keep the TMS processor	running	*/
  if (aptr->ps2_speaker_enabled	== TRUE)
    cmd	|= SKPR_EN;		       /* Keep the PS/2	speaker	enabled	*/
  if (aptr->can_interrupt_host == TRUE)
    cmd	|= HINTENA;		   /* ACPA can still interrupt the host	*/
  /* cmd |= TMS_INT_CMD;	   -- 0	interrupts the TMS */
  /* HREQACK = 0     --	Clear interrupt	from TMS to host */

  acpaoutb(aptr->cmd_stat_reg, cmd);	     /*	Clear the interrupt */

#ifndef	i386
  /* Clear the interrupt level on the bus (V3 only) */

  i_reset(status);	       /* Reset	the interrupt on the bus */
#endif /*  not i386 */

  acpaput( DSP_TMS_INT_INHIBIT_FLAG, (unsigned short) TMS_CAN_INTERRUPT, min );
 
  EXIT_ACPA_PTRACE( INTERRUPT, ACPA_MAIN, min );

  RETURN(INTR_SUCC);		/* Return to kernel successfully */

}

#define	ACPA_READ_TYPE	     1
#define	ACPA_WRITE_TYPE	     2

/* This	routine	updates	the buffer position and	handles	any enqueued */
/* request whose time has come.	*/

void acpa_update_position_info(	Track *tptr, int min, int counter )
{
  struct audio_request *rptr;	/* pointer to request list */
  extern void acpa_handle_request();

  /* Now update	the buffer time	field. */
  if ( tptr->rinfo.mode	!= ADPCM )
    /* Each block of data contains 560 bytes. */
    tptr->buffer.position += 560;
  else  /* ADPCM */
  {
    if (tptr->rinfo.srate == 22050)
      /* Interrupts occur every 1/20th of a second, */
      /* so each block contains 50 milliseconds. */
      tptr->buffer.position += 50;
    else
      /* Interrupts occur every 1/10th of a second, */
      /* so each block contains 100 milliseconds. */
      tptr->buffer.position += 100;
  }

#ifdef ACPA_MEMORY_TRACE
#endif
  /* Handle any	enqueued requests whose	time has come. */
  if ( tptr->request_list != NULL )
  {
    rptr = tptr->request_list;
    if ( tptr->rinfo.mode == ADPCM )
    {
#ifdef ACPA_MEMORY_TRACE
#endif
      if ( rptr->position <= tptr->buffer.position )
      {
	acpa_handle_request( tptr, min );
      }
    }
    else
    {
#ifdef ACPA_MEMORY_TRACE
#endif
      if ( rptr->position <= (unsigned long) acpa_convert_position
		   ( tptr, ACPA_WRITE_TYPE, tptr->buffer.position ) )
      {
	acpa_handle_request( tptr, min );
      }
    }
  }
#ifdef ACPA_MEMORY_TRACE
#endif

}

/*----------------------------------------------------------------------
 * Function name:  acparead
 *
 * Function:	   Handle the read system call
 *
 *----------------------------------------------------------------------*/

#ifdef i386
int acparead(dev, ext)
dev_t dev;			/* Major and minor device numbers */
caddr_t	ext;			/* Parameter from readx	if any */
#else
int acparead(dev, uiop,	mpxchan, ext)
dev_t dev;			/* Major and minor device numbers */
struct uio* uiop;		/* I/o request from user process */
int mpxchan;			/* Ignored, not	a multiplexed device */
int ext;			/* Ignored, parameter from readx */
#endif /* i386 */
{
  int min;			/* Minor device	number */
  int i;
  unsigned short dat;

  int oldspl;			/* Old interrupt level */
  int ptr;			/* Ptr to buffer */
  int timedout;			/* Timed out waiting for data */
  int rc;			/* Return code from called routines */
  int bufsizeinbytes;		/* Size	of buffer at the current speed */
  int nbufs;			/* Number of samples in	the current read */
  int bufct;			/* Buffer count	*/
  int uiomove_rc;
  int mpxchannel;
  int extra_data_expected;
  int new_extra_data_size;
  int new_user_count;
  int extra_data_bytes_to_send;
  int back_in_sync;
  int user_count;
  char *extra_data_ptr;
  Track	*tptr;

  ENTER_ACPA_PTRACE( READ, ACPA_MAIN, min );

  /* No	ctl processes are allowed to perform reads. */
  if ( mpxchan & ACPA_CTL )
    RETURN( EINVAL);

  mpxchannel = ( mpxchan & 3 ) - 1;	/* this	is used	as an index */

  min =	minor(dev);		/* Get the minor device	number */

  tptr = (Track	*) &( acpa[min].track[mpxchannel] );

  /* Reads are not allowed if source mix is in use. */
  if ( tptr->rinfo.mode == SOURCE_MIX )
    RETURN( EINVAL );

  /* See if the	read request is	out of line (if	the ACPA is running */
  /* the playback code then reads are impossible)		  */
  if ( acpa[min].doing_what != Recording )
    RETURN(EINVAL);		/* Can't read while writing */

  /* Do	not allow reads	or writes if the adapter is pausing. */
  if ( tptr->current_state == Pausing )
    RETURN( EINVAL );

  /* Only the process that opened a track can read from	it. */
  if ( tptr->pid != getpid() )
    RETURN( EINVAL );

  /* Handle reads from normal ACPA devices. */

  /* Here to handle reads from the ACPA	card.  We are recording	sounds */
  /* from one of the inputs.  Data from	the card is placed into	the */
  /* channel's kernel queue by the interrupt routine.  This routine */
  /* takes data	out of the kernel queue.  The queue head points	to the */
  /* most recently filled buffer, while	the queue tail points BEHIND */
  /* the least recently	filled buffer.	So our general operation is to */
  /* bump the tail pointer over	(increment it) and take	data from that */
  /* buffer.	Things aren't that simple, though; we may read ahead */
  /* of	the interrupt routine (the queue may be	empty).	 In such cases */
  /* we	sleep on the buffer we want to be filled; the interrupt	*/
  /* routine will wakeup each buffer every time	it fills one.  */

  /* Restrictions: As many samples will	be returned as will fit	into the  */
  /* caller's buffer.  The application must allow for receiving	fewer bytes */
  /* than the read was for. */

  nbufs	= 0;
  bufsizeinbytes = 2 * tptr->buf_size;
  extra_data_expected =	FALSE;
  new_extra_data_size =	0;
  new_user_count = 0;
  extra_data_bytes_to_send = 0;
  back_in_sync = FALSE;
  user_count = USER_COUNT;

  /* If	the user specified O_NDELAY and	there are not enough free buffers to */
  /* be	able to	perform	the read without blocking then set user_count equal  */
  /* to	the amount of data that	will fit into the buffers.  Otherwise it is  */
  /* the same as USER_COUNT.						     */

  if (tptr->open_flags & O_NDELAY)
  {
    if (tptr->extra_data_left_over)
    {
      if (USER_COUNT > (tptr->host_n_bufs * bufsizeinbytes)
		       + tptr->extra_data_size)
	user_count = (tptr->host_n_bufs	* bufsizeinbytes)
		     + tptr->extra_data_size;
    }
    else
    {
      if (USER_COUNT > tptr->host_n_bufs * bufsizeinbytes)
	user_count = tptr->host_n_bufs * bufsizeinbytes;
    }
  }

  /* if a partial block is not left over from the previous read */

  if ( !tptr->extra_data_left_over)
  {
    nbufs = user_count / bufsizeinbytes;

    /* if there	will be	extra data left	over from this read */

    if (nbufs *	bufsizeinbytes < user_count)
    {
      extra_data_expected = TRUE;
      new_extra_data_size = bufsizeinbytes -
			    (user_count	- (nbufs * bufsizeinbytes));
      ++nbufs;
    }
  }
  else /* partial block left over so use it first */
  {
    /* if the extra data left over from	the previous read is enough to	*/
    /* satisfy the current request then	we don't need to read any more.	*/

    if (tptr->extra_data_size >= user_count)
    {
      extra_data_bytes_to_send = user_count;
    }
    else   /* need to read more	data */
    {
      extra_data_bytes_to_send = tptr->extra_data_size;
      new_user_count = user_count - tptr->extra_data_size;

      nbufs = new_user_count / bufsizeinbytes;

      /* if there will be extra data left over from this read */

      if (nbufs	* bufsizeinbytes < new_user_count)
      {
	extra_data_expected = TRUE;
	new_extra_data_size = bufsizeinbytes -
			      (new_user_count -	(nbufs * bufsizeinbytes));
	++nbufs;
      }
    }

    /* Point to	the buffer containing the extra	data */

    if ((tptr->host_tail + 1) >= tptr->rnkb)
      extra_data_ptr = (char *)	(tptr->rbuf[(tptr->host_tail+1)	- tptr->rnkb]);
    else
      extra_data_ptr = (char *)	(tptr->rbuf[tptr->host_tail + 1]);

    /* Now calculate how far into the buffer the extra data begins */

    extra_data_ptr += bufsizeinbytes - tptr->extra_data_size;

    /* Move data left over from	the last read into user	memory */

    IO_MOVE(extra_data_ptr, extra_data_bytes_to_send, IO_MOVE_READ);

    if (IO_MOVE_RC != 0)
       RETURN(IO_MOVE_RC);

    /* if sent all the extra data left over from the last read */

    if (extra_data_bytes_to_send == tptr->extra_data_size)
    {
      /* if the user requested exactly the amount of data left over */

      if (user_count ==	tptr->extra_data_size)
	back_in_sync = TRUE;  /* we can update variables and return */

      tptr->extra_data_size = 0;
      tptr->extra_data_left_over = FALSE;

      oldspl = DISABLE_INTERRUPTS();		       /* Disable interrupts */

      ++tptr->host_tail;

      if (tptr->host_tail >= tptr->rnkb)
	tptr->host_tail	-= tptr->rnkb;	/* modulo rnkb */

      tptr->host_n_bufs--;	     /*	One fewer buffers on the kernel	queue */

      /* Now that a block has been transferred,	decrease the buffer's */
      /* read buffer size and turn off the AUDIO_OVERRUN flag. */
      tptr->buffer.read_buf_size -= extra_data_bytes_to_send;
      tptr->buffer.flags &= ~AUDIO_OVERRUN;

      (void) ENABLE_INTERRUPTS(oldspl);		   /* Enable interrupts	again */

      /* if user asked to read exactly the same	number of bytes	as the number */
      /* of extra data bytes left over from the	last read then done	      */

      if (back_in_sync)
	 RETURN(IO_MOVE_RC);				/* Normal completion */
    }
    else   /* more than enough data left over to satisfy request */
    {
      tptr->extra_data_size -= user_count;

      oldspl = DISABLE_INTERRUPTS();			/* Disable interrupts */

      /* Now that a partial block has been transferred,	decrease the buffer's */
      /* read buffer size and turn off the AUDIO_OVERRUN flag. */
      tptr->buffer.read_buf_size -= extra_data_bytes_to_send;
      tptr->buffer.flags &= ~AUDIO_OVERRUN;

      (void) ENABLE_INTERRUPTS(oldspl);		   /* Enable interrupts	again */

      RETURN(IO_MOVE_RC);				/* Normal completion */
    }
  }

  /* Logic: ************************************************************

  for the number of buffers we need to fill do
     disable interrupts

     do forever
        if there is data to read
           break out and do not go to sleep

        sleep for 4 seconds or until interrupt routine wakes us up

        if the sleep is broken by a signal
           set timedout flag and break out

        if the sleep timed out
           the ACPA card has a problem so set timedout flag and break out
     end do

     enable interrupts

     if the timedout flag is set
        return an I/O error since no interrupts occurred

     send data to the user's buffer

     disable interrupts
     update tail pointer
     enable interrupts
  end for

  **********************************************************************/

#ifdef ACPA_MEMORY_TRACE
  if ( xxptr[min] > ( txxptr[min] + 0x1000 - 400 ) )
  {
    xxptr[min] = txxptr[min] + 1;
    *xxptr[min]++ += 1;
    for ( i=0; i<(0x1000-2); i++ )
      *xxptr[min]++ = -1;
    xxptr[min] = txxptr[min] + 2;
  }
  *xxptr[min]++ = nbufs;
  *xxptr[min]++ = 6;
#endif
  for (bufct = 0; bufct < nbufs; bufct++) {
#ifdef ACPA_MEMORY_TRACE
#endif

#ifdef ACPA_MEMORY_TRACE
#endif
    oldspl = DISABLE_INTERRUPTS(); /* Disable all interrupts */
    acpa[min].recursion	= 1;	       /* If intrupt occurs scream very	loudly */
    for	(;;) {
      timedout = 0;
      if (tptr->host_n_bufs > 0) break;	  /* Don't sleep, there's data to read */
      /* Sleep waiting on the channel block */
      tptr->waiting = TRUE;		   /* We're waiting, if	anyone asks... */
#ifdef ACPA_MEMORY_TRACE
#endif
      TIMED_SLEEP(&tptr->ring_event, rc, RING_EVENT); /* Wait for event	or*/
						    /* for several seconds*/
      tptr->waiting = FALSE;	    /* Not waiting any more, if	anyone asks... */
#ifdef ACPA_MEMORY_TRACE
#endif
      if (rc == SLEEP_BROKEN_BY_SIGNAL) {
#ifdef ACPA_MEMORY_TRACE
#endif
	TRACE(("SLEEP BROKEN BY	SIGNAL IN READ\n"));
	timedout++;
	break;
      }
      if (rc ==	SLEEP_TIMED_OUT) {
#ifdef ACPA_MEMORY_TRACE
#endif
	timedout++;
	break;
      }
    }

    /*
     * If we got anything, update queue	pointers
     */
    if (!timedout) {
#ifdef ACPA_MEMORY_TRACE
#endif
      ptr = tptr->host_tail + 1;
      if (ptr >= tptr->rnkb)
	ptr -= tptr->rnkb;	   /* ptr = host_tail +	1 modulo rnkb */
    }

    acpa[min].recursion	= FALSE; /* Interrupts aren't fatal any	more */
    (void) ENABLE_INTERRUPTS(oldspl); /* Enable	interrupts again */
    /*
     * If no interrupt happened, return	I/O error
     */
    if (timedout) {
      if (rc ==	SLEEP_BROKEN_BY_SIGNAL)	{
#ifdef ACPA_MEMORY_TRACE
#endif
	RETURN(EINTR);
      }
      else {
	acpaerr(ERRID_ACPA_INTR4, "sysxacpa", min, __LINE__,"acparead","timedout",rc,NULL,NULL);
	RETURN(EIO);
      }
    }

    /* if we are reading the last block	of data	and extra data is expected */
    /* we need to only send the	part of	the data the user requested and	we */
    /* have to keep track of the remaining data.			   */

    if ((bufct == nbufs	- 1) &&	extra_data_expected)
    {
      /* move partial block of buffer into user	memory to complete the */
      /* number	of bytes requested				       */

      IO_MOVE(tptr->rbuf[ptr], bufsizeinbytes -	new_extra_data_size,
	      IO_MOVE_READ);

      oldspl = DISABLE_INTERRUPTS();			/* Disable interrupts */
      acpa[min].recursion = 2;			      /* Interrupts are	fatal */

      tptr->extra_data_left_over = TRUE;
      tptr->extra_data_size = new_extra_data_size;

      /* Now that a block has been transferred,	decrease the buffer's */
      /* read buffer size and turn off the AUDIO_OVERRUN flag. */
      tptr->buffer.read_buf_size -= bufsizeinbytes - new_extra_data_size;
      tptr->buffer.flags &= ~AUDIO_OVERRUN;

      acpa[min].recursion = FALSE;		   /* Interrupts are OK	again */

      (void) ENABLE_INTERRUPTS(oldspl);		   /* Enable interrupts	again */
    }
    else
    {
      /* Move contents of buffer "ptr" into user memory	*/
      /* Actually the buffer is	pointed	to by tptr->rbuf[ptr] */

      IO_MOVE(tptr->rbuf[ptr], bufsizeinbytes, IO_MOVE_READ);

      /* Now mark the tail buffer as available for the interrupt handler */

#ifdef ACPA_MEMORY_TRACE
#endif
      oldspl = DISABLE_INTERRUPTS();			/* Disable interrupts */
      acpa[min].recursion = 2;			      /* Interrupts are	fatal */
      tptr->host_tail =	ptr;			  /* Bump up the tail pointer */
      tptr->host_n_bufs--;	     /*	One fewer buffers on the kernel	queue */
      /* Now that a block has been transferred,	decrease the buffer's */
      /* read buffer size and turn off the AUDIO_OVERRUN flag. */
      tptr->buffer.read_buf_size -= tptr->rinfo.bsize;
      tptr->buffer.flags &= ~AUDIO_OVERRUN;

      acpa[min].recursion = FALSE;		   /* Interrupts are OK	again */
      (void) ENABLE_INTERRUPTS(oldspl);		  /* Enable interrupts again */
    }
  }				/* Do it for next sample that will fit.	*/

  EXIT_ACPA_PTRACE( READ, ACPA_MAIN, min );

#ifdef ACPA_MEMORY_TRACE
#endif

  RETURN(IO_MOVE_RC);					/* Normal completion */

}

/*----------------------------------------------------------------------
 * Function name:  acpawrite
 *
 * Function:	   Handle the write system call
 *
 *----------------------------------------------------------------------*/

#ifdef i386
int acpawrite(dev, ext)
dev_t dev;			/* Major and minor device numbers */
caddr_t	ext;			/* Parameter from writex if any	*/
#else
int acpawrite(dev, uiop, mpxchan, ext)
dev_t dev;			/* Major and minor device numbers */
struct uio* uiop;		/* I/O block from user process */
int mpxchan;			/* Multiplex device channel (ignored) */
int ext;			/* Extension from writex (ignored) */
#endif /* i386 */
{
  int min;			/* Minor device	number */
  Shorts* image;		/* Ptr to the microcode	image we are */
				/* going to affect */
  char*	sink;			/* Where to put	the data in kernel memory */
  int source;			/* Source or sink from minor number */
  int speed;			/* Speed from minor number */
  int channel;			/* Channel(s) from minor number	*/
  int trk;			/* Channel subscript */
  int card;			/* What	card are we using? (0-3) */
  Track* ch;		      /* Ptr to	current	channel	*/
  int oldspl;			/* Old interrupt level */
  int timedout;			/* Did our tsleep time out or wake up? */
  int rc;			/* Return code from called routines */
  int bufcount;			/* Loop	counter	*/
  int nbufs;			/* Number of buffers in	the current write */
  int bufsizeinbytes;
  int trkbox;			/* Base	addr of	track mailbox in DSP memory */
  int uiomove_rc;		/* return code for R2 */
  int mpxchannel;		/* the transformed mpx channel number */
  register Track *tptr;		/* pointer to current track info */
  int temp, temp2;
  int partial_block_expected;
  int new_partial_block_size;
  int new_data_bytes_to_write;
  int new_user_count;
  int user_count;
  char *partial_block_ptr;
  register struct acpa_status *aptr;
  extern void acpa_write_block();
#ifdef ACPA_MEMORY_TRACE
  void dbg_print_regs();
#endif

  /* No	ctl processes are allowed to perform reads. */
  if ( mpxchan & ACPA_CTL )
    RETURN( EINVAL);

  mpxchannel = ( mpxchan & 3 ) - 1;	/* this	is used	as an index */

  min =	minor(dev);		/* Get the minor device	number */

  ENTER_ACPA_PTRACE( WRITE, ACPA_MAIN, min );

  aptr = (struct acpa_status *)	&( acpa[min] );
  tptr = (Track	*) &( acpa[min].track[mpxchannel] );

  /* Reads are not allowed if source mix is in use. */
  if ( tptr->rinfo.mode == SOURCE_MIX )
    RETURN( EINVAL );

  /* Do not allow playing if we are not in playback mode. */
  if ( aptr->doing_what	!= Playing )
    RETURN(EINVAL);

  /* Do	not allow reads	or writes if the adapter is pausing. */
  if ( tptr->current_state == Pausing )
    RETURN( EINVAL );

  /* Only the process that opened a track can write to it. */
  if ( tptr->pid != getpid() )
    RETURN( EINVAL );

  nbufs	= 0;
  bufsizeinbytes = 2 * tptr->buf_size;
  partial_block_expected = FALSE;
  new_partial_block_size = 0;
  new_user_count = 0;
  new_data_bytes_to_write = 0;
  user_count = USER_COUNT;

  /* If	the user specified O_NDELAY and	there are not enough free buffers to */
  /* be	able to	perform	the write without blocking then	set user_count equal */
  /* to	the amount of data that	will fit into the buffers.  Otherwise it is  */
  /* the same as USER_COUNT.						     */

  if (tptr->open_flags & O_NDELAY)
  {
    if (tptr->partial_block_left_over)
    {
      if (USER_COUNT > ((tptr->wnkb - tptr->host_n_bufs) * bufsizeinbytes)
			+ tptr->partial_block_size)
	user_count = ((tptr->wnkb - tptr->host_n_bufs) * bufsizeinbytes)
		      +	tptr->partial_block_size;
    }
    else
    {
      if (USER_COUNT > (tptr->wnkb - tptr->host_n_bufs)	* bufsizeinbytes)
	user_count = (tptr->wnkb - tptr->host_n_bufs) *	bufsizeinbytes;
    }
  }

  /* if a partial block is not left over from the previous write */

  if ( !tptr->partial_block_left_over)
  {
    nbufs = user_count / bufsizeinbytes;

    /* if there	will be	a partial block	left over from this write */

    if (nbufs *	bufsizeinbytes < user_count)
    {
      partial_block_expected = TRUE;
      new_partial_block_size = user_count - (nbufs * bufsizeinbytes);
      ++nbufs;
    }
  }
  else /* partial block	left over so add to it first */
  {
    /* if adding the data sent for this	write to the partial block left	*/
    /* over from the previous write does NOT result in a full block	*/

    if (tptr->partial_block_size + user_count <	bufsizeinbytes)
    {
      new_data_bytes_to_write =	user_count;
    }
    else   /* new data will at least fill the partial block */
    {
      new_data_bytes_to_write =	bufsizeinbytes - tptr->partial_block_size;
      new_user_count = user_count - new_data_bytes_to_write;

      nbufs = new_user_count / bufsizeinbytes;

      /* if there will be a partial block left over from this write */

      if (nbufs	* bufsizeinbytes < new_user_count)
      {
	partial_block_expected = TRUE;
	new_partial_block_size = new_user_count	- (nbufs * bufsizeinbytes);
	++nbufs;
      }
    }

    /* Point to	the buffer containing the partial block	*/

    partial_block_ptr =	(char *) (tptr->wbuf[tptr->host_head]);

    /* Now calculate where the new data	should be put in the block */

    partial_block_ptr += tptr->partial_block_size;

    /* get data	from user's buffer */

    IO_MOVE(partial_block_ptr, new_data_bytes_to_write,	IO_MOVE_WRITE);

    if (IO_MOVE_RC != 0)
      RETURN(IO_MOVE_RC);	      /* Error occurred	*/

    oldspl = DISABLE_INTERRUPTS();		       /* Disable interrupts */

    tptr->buffer.write_buf_size	+= new_data_bytes_to_write;

    /* if there	is not enough data to make the partial block complete */

    if ((tptr->partial_block_size + new_data_bytes_to_write) < bufsizeinbytes)
    {
      tptr->partial_block_size += new_data_bytes_to_write;
    }
    else
    {
      tptr->partial_block_size = 0;
      tptr->partial_block_left_over = FALSE;
    }

    (void) ENABLE_INTERRUPTS(oldspl);		   /* Re-enable	interrupts */
  }

  /* Logic:  For each sample-sized chunk in the	write buffer:

     for the number of buffers we will be writing
        disable interrupts

        if kernel queue is full
           do forever
              if there is an empty slot in the queue
                 break out and do not go to sleep

              sleep for 4 seconds or until interrupt routine wakes us up

              if the sleep is broken by a signal
                 set timedout flag and break out

              if the sleep timed out
                 the ACPA card has a problem so set timedout flag and break out
           end do

        if we did not timeout
           enable interrupts

           get data from the user's buffer

           disable interrupts
           update head pointer
           enable interrupts
           
        else
           return an I/O error since no interrupts occurred


        if this is the first write after an open
           start the ACPA card
     end for

     NOTE THAT IT IS ESSENTIAL THAT INTERRUPTS ARE ALWAYS DISABLED
     WHENEVER THE ADAPTER IS WRITTEN TO OR READ FROM.  WHEN TWO TRACKS
     ARE PLAYING SIMULTANEOUSLY, THE MICROCODE MUST ALWAYS BE USED
     SERIALLY.  OTHERWISE ITS I/O REGISTERS WILL BE TRASHED.
  */

#ifdef ACPA_MEMORY_TRACE
  if ( xxptr[min] > ( txxptr[min] + 0x1000 - 400 ) )
  {
    int i;
    xxptr[min] = txxptr[min] + 1;
    *xxptr[min]++ += 1;
    for ( i=0; i<(0x1000-2); i++ )
      *xxptr[min]++ = -1;
    xxptr[min] = txxptr[min] + 2;
  }
  /**xxptr[min]++ = 0x55550000 | mpxchannel;*/
  /*dbg_print_regs( 0x0e10, min, 0xaaab0000 );*/
  /*if ( mpxchannel == 0 )
    dbg_print_regs( 0x0e18, min, 0xaaac0000 );
  else
    dbg_print_regs( 0x0e20, min, 0xaaac0000 );*/
#endif
  for (bufcount = 0; bufcount < nbufs; bufcount++) {
    oldspl = DISABLE_INTERRUPTS();		       /* Disable interrupts */
    acpa[min].recursion	= 200;			       /* Interrupts are fatal */

    timedout = 0;

    if (tptr->host_n_bufs == tptr->wnkb) {   /*	Kernel queue is	full */
      for (;;) {
	timedout = 0;
	if (tptr->host_n_bufs <	tptr->wnkb) break;   /*	Don't sleep, there's an	*/
					/* empty slot */
	tptr->waiting =	TRUE;	  /* We're waiting */
#ifdef ACPA_MEMORY_TRACE
#endif
	TIMED_SLEEP(&tptr->ring_event, rc, RING_EVENT);	/* Wait	for a bit */
	tptr->waiting =	FALSE;	  /* Not waiting any more */
	if (rc == SLEEP_BROKEN_BY_SIGNAL) {
#ifdef ACPA_MEMORY_TRACE
#endif
	  timedout++;
	  break;
	}
	if (rc == SLEEP_TIMED_OUT) {
#ifdef ACPA_MEMORY_TRACE
#endif
	  timedout++;		/* We timed out	*/
	  break;			/* Bail	out */
	}
      }
    }

    /* If a buffer is available	(we didn't time	out), update the queue */
    /* pointers	*/

    if (!timedout) {
      int temp_head;

      temp_head	= tptr->host_head + 1;

      if (temp_head >= tptr->wnkb)
	temp_head -= tptr->wnkb;

      acpa[min].recursion = FALSE;		   /* Interrupts are OK	again */
      (void) ENABLE_INTERRUPTS(oldspl);		     /*	Re-enable interrupts */

      if (bufcount == (nbufs - 1) && partial_block_expected)
      {
	 IO_MOVE(tptr->wbuf[temp_head],	new_partial_block_size,	IO_MOVE_WRITE);
      }
      else
      {
	 IO_MOVE(tptr->wbuf[temp_head],	bufsizeinbytes,	IO_MOVE_WRITE);
      }

      if (IO_MOVE_RC !=	0)
	RETURN(IO_MOVE_RC);		/* Error occurred */

      oldspl = DISABLE_INTERRUPTS();		       /* Disable interrupts */

      tptr->host_head++;

      if (tptr->host_head >= tptr->wnkb)
	tptr->host_head	-= tptr->wnkb;	     /*	host_head++ modulo wnkb	*/

      tptr->host_n_bufs++;	  /* One more buffer in	the queue is full */

      /* Now that the block has	been transferred, increase the buffer's	*/
      /* write buffer size. */

      if (bufcount == (nbufs - 1) && partial_block_expected)
      {
	tptr->buffer.write_buf_size += new_partial_block_size;
	tptr->partial_block_left_over =	TRUE;
	tptr->partial_block_size = new_partial_block_size;
      }
      else
      {
	tptr->buffer.write_buf_size += tptr->rinfo.bsize;
      }

      /* Update	the max	buffer size if this is a bigger	value. */
      if ( tptr->host_n_bufs > tptr->buffer.write_buf_max )
	tptr->buffer.write_buf_max = tptr->host_n_bufs;

      acpa[min].recursion = FALSE;		  /* Interrupts	are OK again */
      (void) ENABLE_INTERRUPTS(oldspl);	      /* Re-enable interrupts and... */
    }

    else {					     /*	Timed out, just	punt */
      (void) ENABLE_INTERRUPTS(oldspl);	      /* Re-enable interrupts and... */

      if (rc ==	SLEEP_BROKEN_BY_SIGNAL)	{
	RETURN(EINTR);		/* Interrupted system call */
      }
      else {
	acpaerr(ERRID_ACPA_INTR4, "sysxacpa", min, __LINE__,"acpawrite","timedout",rc,NULL,NULL);
	RETURN(EIO);			 /* Return an I/O error	on the write */
      }
    }

    /* If this is the first write after	open, we must start the	DSP. */
    /* This was	deferred from open in order to allow us	to accumulate */
    /* data to be played.  So start it now */

    if ( tptr->current_state ==	Almost_Started )
    {
      /* Find the track	control	block and send the PLAYBACK instruction	*/
      /* to the	DSP microcode. */
      if (mpxchannel ==	0)
	trkbox = DSP_T1_MAILBOX_BASE;
      else
	trkbox = DSP_T2_MAILBOX_BASE;

      /* For the initial preloading of buffers, the maximum number of buffers */
      /* need to be loaded.  Since the preload maximum was reduced by one for */
      /* most uses, this one adds on to it to ensure all the buffers get */
      /* loaded. */
      if ( tptr->preload_count < tptr->modulo_factor )
      {
	if ( !(tptr->partial_block_left_over && (tptr->host_n_bufs == 1)))
	{
	  /* Need to fill up another DSP buffer before starting the card. */
	  oldspl = DISABLE_INTERRUPTS();        /* Disable interrupts */
	  acpa_write_block( tptr, aptr, trkbox, min );
	  (void) ENABLE_INTERRUPTS(oldspl);     /* Reenable interrupts */
	  tptr->preload_count++;  /* One more buffer has been preloaded. */
	}
      }
      else      /* All preloads have been done; start the adapter. */
      {
	tptr->preload_count -= 1;       /* When the first interrupt occurs, */
					/* one of the preloaded blocks will */
					/* have been played, so account for */
					/* that now. */
	tptr->current_state = Started;
	/* The position field needs to be initialized correctly.  Because */
	/* one block will have already played when the first interrupt */
	/* occurs, that block needs to be represented in the position. */
        if ( tptr->rinfo.mode != ADPCM )
	  tptr->buffer.position += 560;
	else  /* ADPCM */
        {
          if (tptr->rinfo.srate == 22050)
            /* Interrupts occur every 1/20th of a second, */
            /* so each block contains 50 milliseconds. */
            tptr->buffer.position += 50;
          else
            /* Interrupts occur every 1/10th of a second, */
            /* so each block contains 100 milliseconds. */
            tptr->buffer.position += 100;
        }

	if (acpa[min].running != TRUE)
	{
	  acpa[min].can_interrupt_host = FALSE;	   /* Disable DSP interrupts */
	  (void) acpastart( min	);				       /* Start	DSP */
	  (void) DELAYTICKS(2);		 /* Wait a moment for the DSP to start */

	  /* Enable interrupts from the	DSP */
	  acpa[min].can_interrupt_host = TRUE;
	  (void) acpastart( min	);
	}

	/* Start the operation. */
	oldspl = DISABLE_INTERRUPTS();          /* Disable interrupts */
	acpaput(trkbox + DSP_TX_CMD_FROM_HOST, (unsigned short)	5, min );
	(void) ENABLE_INTERRUPTS(oldspl);       /* Re-enable interrupts */
      }
    }

  }				/* End of one sample */

  tptr->write_flag = TRUE;

  EXIT_ACPA_PTRACE( WRITE, ACPA_MAIN, min );

  RETURN(IO_MOVE_RC);		/* Return error	from uiomove if	any */

}

/*----------------------------------------------------------------------
 * Function name:  acpaioctl
 *
 * Function:	   Handle the IOCTL system call
 * 
 *----------------------------------------------------------------------*/

/* MUST	respond	to IOCTYPE and IOCINFO */

#ifdef i386
int acpaioctl(dev, cmd,	arg, flag, ext)
dev_t dev;			/* Major and minor device numbers */
int cmd;			/* Command parameter from ioctl	call */
caddr_t	arg;			/* Argument from the system call */
int flag;			/* Flags passed	to open	system call */
caddr_t	ext;			/* Parameter from ioctlx if any	*/
#else
int acpaioctl(dev, cmd,	arg, devflag, mpxchan, ext)
dev_t dev;			/* Major and minor device numbers */
int   cmd;			/* Command parameter from ioctl	call */
int   arg;			/* Argument from the system call */
ulong devflag;			/* Flags passed	to open	system call */
int   mpxchan;			/* Ignored, not	a multiplexed device */
int   ext;			/* Parameter from ioctlx if any	(ignored) */
#endif /* i386 */
{
  register int min;		/* Minor device	number */
  int rc = 0;			/* Contains return code	value */
  audio_control	control;	/* local copy of user's	audio_control info */
  int ctlflag;			/* flags whether this is a ctl file */
  acpa_regs8 *dptr8;		/* pointer to 8-bit diagnostics	data */
  acpa_regs16 *dptr16;		/* pointer to 16-bit diagnostics data */
  acpa_regs8 reg8;		/* local copy of user's	8-bit request */
  acpa_regs16 reg16;		/* local copy of user's	16-bit request */
  unsigned int ioctl_request;	/* the specific	AUDIO_CONTROL request */
  int new_position;
  extern int acpa_init_request();
  extern int acpa_change_request();
  extern int acpa_start();
  extern int acpa_stop();
  extern int acpa_pause();
  extern int acpa_resume();
  extern int acpa_status_request();
  extern int acpa_wait_request();
  extern int acpa_buffer_request();
  extern int acpa_load_request();
  extern unsigned short	acpain();
  extern unsigned char acpainb();
  extern int acpa_add_request_to_q();

  min =	minor(dev);		/* Get minor device number */

  ENTER_ACPA_PTRACE( IOCTL, ACPA_MAIN, min );

  /* If	minor number is	invalid	return EINVAL */
  if ( ( min < 0) || ( min > 15) )
    RETURN( EINVAL );

  ctlflag = mpxchan & ACPA_CTL;

  /* Call the appropriate ioctl	request	code based on the command. */
  /* Ctl files should only be allowed to progress to AUDIO_CHANGE. */
  /* Return an error if	anything is the	case. */
  if ( ( ctlflag ) && (	cmd != AUDIO_CONTROL ) )
    RETURN( EINVAL );

  /* Only the process that opened a track can do any ioctls except */
  /* for AUDIO_CHANGE. */
  if ( ( acpa[min].track[(mpxchan & 3)-1].pid != getpid() ) &&
       ( cmd !=	AUDIO_CONTROL )	)
    RETURN( EINVAL );

  /* Only diagnostics requests can use the diagnostics ioctl commands. */
  if ( !( mpxchan & ACPA_DIAGS ) )
    if ( ( cmd >= AUDIO_DIAGS8_READ ) && ( cmd <= AUDIO_DIAGS16_WRITE )	)
      RETURN( EINVAL );

  /* Copy the user's buffer into the kernel space if it	is an */
  /* AUDIO_CONTROL command. */
  /* DOES THIS NEED TO CHECK FOR NULL OR WILL THE COPY JUST FAIL? */
  if ( cmd == AUDIO_CONTROL )
    if ( ( rc =	copyin(	(char *) arg, (char *) &control,
			sizeof(audio_control) )	) != 0 )
      RETURN( EFAULT );
    else
      /* Only AUDIO_RESUME can be done if the adapter's	current	state is */
      /* paused. */
      if ( ( acpa[min].track[(mpxchan &	3)-1].current_state == Pausing ) &&
	   ( control.ioctl_request != AUDIO_RESUME ) )
	RETURN(	EINVAL );

#ifdef ACPA_MEMORY_TRACE
#endif
  /* The only ioctls that can be used on a track that is set to */
  /* source mix are:  AUDIO_CHANGE, AUDIO_INIT, AUDIO_START, and */
  /* AUDIO_STOP. */
  if ( acpa[min].track[(mpxchan&3)-1].rinfo.mode == SOURCE_MIX )
    switch( cmd )
    {
      case AUDIO_INIT :
	break;
      case AUDIO_CONTROL :
	switch( control.ioctl_request )
	{
	  case AUDIO_CHANGE :
	  case AUDIO_START :
	  case AUDIO_STOP :
	    break;
	  default :
	    RETURN( EINVAL );
	}
        break;
      default :
	RETURN(EINVAL );
    }

  switch ( cmd )
  {
    case AUDIO_INIT :
      /* Allow the init	request	only if	the file descriptor is a track */
      switch( mpxchan )
      {
	case ACPA_TRACK1 :
	case ACPA_TRACK2 :
	  rc = acpa_init_request( arg, min, devflag, mpxchan );
	  break;
	/* Any other values are	invalid	*/
	default	:
	  RETURN( EINVAL );
	  break;
      }
      break;
    case AUDIO_CONTROL :
      if ( ( ctlflag ) && ( control.ioctl_request != AUDIO_CHANGE ) )
	RETURN(	EINVAL );
      if ( ( acpa[min].track[(mpxchan &	3)-1].pid != getpid() )	&&
	   ( control.ioctl_request != AUDIO_CHANGE ) )
	RETURN(	EINVAL );
      if ( ( acpa[min].track[(mpxchan &	3)-1].current_state == Pausing ) &&
	   ( control.ioctl_request != AUDIO_RESUME ) )
	RETURN(	EINVAL );
      /* If the	user-specified position	is already past, make the */
      /* request get executed immediately. */
      if ( acpa[min].track[(mpxchan&3)-1].rinfo.mode ==	ADPCM )
      {
	if ( control.position <= acpa[min].track[(mpxchan&3)-1].buffer.position	)
	  control.position = 0;
      }
      else
      {
	new_position = (unsigned long) acpa_convert_position
		       ( &(acpa[min].track[(mpxchan&3)-1]), ACPA_WRITE_TYPE,
			 acpa[min].track[(mpxchan&3)-1].buffer.position	);
	if ( control.position <= new_position )
	  control.position = 0;
      }
#ifdef ACPA_MEMORY_TRACE
#endif
      /* If the	request	is supposed to be executed at a	later point in */
      /* time, enqueue the request.  Otherwise execute it. */
      if ( control.position )
      {
	if ( acpa[min].doing_what == Recording )
	  /* Enqueued requests are for playback	mode only. */
	  RETURN( EINVAL );
	else
	  switch( control.ioctl_request	)
	  {
	    case AUDIO_START :
	      /* START is always executed immediately. */
	      break;
	    case AUDIO_RESUME :
	      /* RESUME	cannot be enqueued. */
	      RETURN( EINVAL );
	      break;
	    default :
	      /* Any other request can be enqueued. */
	      rc = acpa_add_request_to_q( &control, min, mpxchan );
	      RETURN( rc );
	      break;
	  }
      }
      switch( control.ioctl_request )
      {
	case( AUDIO_CHANGE ) :
	  rc = acpa_change_request( control.request_info, min, mpxchan,
				    NOT_ENQUEUED, -1 );
	  break;
	case( AUDIO_START ) :
	  rc = acpa_start( mpxchan, min, control.position );
	  break;
	case( AUDIO_STOP ) :
	  rc = acpa_stop( mpxchan, min );
	  break;
	case( AUDIO_PAUSE ) :
	  rc = acpa_pause( mpxchan, min	);
	  break;
	case( AUDIO_RESUME ) :
	  rc = acpa_resume( mpxchan, min );
	  break;
	default	:
	  acpaerr(ERRID_ACPA_IOCTL1, "sysxacpa", min, __LINE__,"acpaioctl","control.ioctl_request",rc,NULL,NULL);
	  rc = EINVAL;
	  break;
      }
      break;
    case AUDIO_STATUS :
      rc = acpa_status_request(	arg, min, mpxchan );
      break;
    case AUDIO_WAIT :
      rc = acpa_wait_request( min, mpxchan );
      break;
    case AUDIO_BUFFER :
      rc = acpa_buffer_request(	arg, min, mpxchan );
      break;
    case AUDIO_LOAD :
      rc = acpa_load_request( arg, min,	mpxchan	);
      break;
    case AUDIO_DIAGS_PUT :
      /* Copy the user's buffer	into the kernel	space. */
      if ( ( rc	= copyin( (char	*) arg,	(char *) &reg16,
			  sizeof(acpa_regs16) )	) != 0 )
	RETURN(	EFAULT );
      acpa[min].int_count = (int) reg16.data;
      break;
    case AUDIO_DIAGS_GET :
      reg16.data = acpa[min].int_count;
      dptr16 = (acpa_regs16 *) arg;
      /* Copy the kernel data back into	the user's buffer. */
      if ( ( rc	= copyout( (char *) &(reg16.data), (char *) &(dptr16->data),
			   sizeof(dptr16->data)	) ) != 0 )
	RETURN(	EFAULT );
      break;
    case AUDIO_DIAGS8_READ :
      /* Copy the user's buffer	into the kernel	space. */
      if ( ( rc	= copyin( (char	*) arg,	(char *) &reg8,
			  sizeof(acpa_regs8) ) ) != 0 )
	RETURN(	EFAULT );
      /* issue the command to the adapter */
      reg8.data	= acpainb( (int) ( acpa[min].base+reg8.offset )	);
      dptr8 = (acpa_regs8 *) arg;
      /* Copy the kernel data back into	the user's buffer. */
      if ( ( rc	= copyout( (char *) &(reg8.data), (char	*) &(dptr8->data),
			   sizeof(dptr8->data) ) ) != 0	)
	RETURN(	EFAULT );
      break;
    case AUDIO_DIAGS8_WRITE :
      /* Copy the user's buffer	into the kernel	space. */
      if ( ( rc	= copyin( (char	*) arg,	(char *) &reg8,
			  sizeof(acpa_regs8) ) ) != 0 )
	RETURN(	EFAULT );
      /* issue the command to the adapter */
      acpaoutb(	(int) (	acpa[min].base+reg8.offset ), reg8.data	);
      break;
    case AUDIO_DIAGS16_READ :
      /* Copy the user's buffer	into the kernel	space. */
      if ( ( rc	= copyin( (char	*) arg,	(char *) &reg16,
			  sizeof(acpa_regs16) )	) != 0 )
	RETURN(	EFAULT );
      /* issue the command to the adapter */
      reg16.data = acpain( (int) ( acpa[min].base+reg16.offset ) );
      dptr16 = (acpa_regs16 *) arg;
      /* Copy the kernel data back into	the user's buffer. */
      if ( ( rc	= copyout( (char *) &(reg16.data), (char *) &(dptr16->data),
			   sizeof(dptr16->data)	) ) != 0 )
	RETURN(	EFAULT );
      break;
    case AUDIO_DIAGS16_WRITE :
      /* Copy the user's buffer	into the kernel	space. */
      if ( ( rc	= copyin( (char	*) arg,	(char *) &reg16,
			  sizeof(acpa_regs16) )	) != 0 )
	RETURN(	EFAULT );
      /* issue the command to the adapter */
      acpaout( (int) ( acpa[min].base+reg16.offset ), reg16.data );
      break;
    default :
      acpaerr(ERRID_ACPA_IOCTL2, "sysxacpa", min, __LINE__,"acpaioctl","cmd",EINVAL,NULL,NULL);
      RETURN( EINVAL );
      break;
  }

  EXIT_ACPA_PTRACE( IOCTL, ACPA_MAIN, min );

  RETURN( rc );
}

int acpampx( devno, chanp, channame )
dev_t devno;		/* major and minor number of device */
chan_t *chanp;		/* converted channel number */
char *channame;		/* string suffix to device special file	name */
{
  int min;		/* minor number	of device */
  int i;		/* all-purpose variable	*/

  ENTER_ACPA_PTRACE( MPX, ACPA_MAIN, minor( devno ) );

  /* Use the generic channel if	no specific channel is specified. */
  if ( channame	== NULL	)
  {
    /* This is the close() pass	through	mpx */
    RETURN( 0 );
  }

  if ( *channame == 0 )
  {
    /* This is the open() pass through mpx */
    /* Because no path beyond /dev/audioX has been specified, the call */
    /* is invalid. */
    RETURN( EINVAL );
  }

  min =	minor( devno );		/* Which one of	4 cards	is this? */

  /* Otherwise,	set the	channel	ID if the channel name is valid. */
  /* To	cut down on the	executed logic,	base name testing on the */
  /* first character in	the channel name. */

  /* If	minor number is	invalid	return ENXIO */
  if ( min < 0 || min >	15 )
      RETURN( ENXIO );
  else
  {
    i =	strlen(	channame );	/* get the length of the suffix	*/

    switch ( i )
    {
      case 1 :		/* there is one	character in the suffix	*/
	if ( *channame == '1' )
	  *chanp = ACPA_TRACK1;
	else if	( *channame == '2' )
	  *chanp = ACPA_TRACK2;
	else
	  *chanp = -1;	/* this	is invalid */
	break;
      case 4 :		/* there are four chars	in the suffix */
	if ( strcmp( "ctl1", channame	) == 0 )
	  *chanp = ACPA_CTL1;
	else if	( strcmp( "ctl2", channame ) ==	0 )
	  *chanp = ACPA_CTL2;
	else if	( strcmp( "diag", channame ) ==	0 )
	  *chanp = ACPA_DIAG1;
	else
	  *chanp = -1;	/* this	is invalid */
	break;
      default :
	*chanp = -1;
	break;
    }
  }

  if ( *chanp == -1 )
    RETURN( EINVAL );

  EXIT_ACPA_PTRACE( MPX, ACPA_MAIN, min	);

  RETURN( 0 );
}





/*----------------------------------------------------------------------
 * Function name:  acpaselect
 *
 * Function:	   Handle the select system call
 * 
 * Returns:
 * 0 ==> Device	is not ready.  Collision occurred; this	is not the
 *	 first process waiting on the event
 * 1 ==> Selection criteria is TRUE.
 * 2 ==> Device	is not ready.  This is the first process waiting on the
 *	 event.
 *
 *----------------------------------------------------------------------*/

#ifdef i386
int acpaselect(dev, seltype)
dev_t dev;			/* Major and minor device numbers */
int seltype;			/* Either FREAD, FWRITE	or 0 (exception) */
#else
int acpaselect(dev, events, reventp, chan)
dev_t dev;
ushort events;
ushort*	reventp;
int chan;
#endif /* i386 */
{

  ENTER_ACPA_PTRACE( SELECT, ACPA_MAIN,	-1 );

  EXIT_ACPA_PTRACE( SELECT, ACPA_MAIN, -1 );

  return (1);			/* Select is always ready (for now) */

}


/*----------------------------------------------------------------------
 * Function name:  acpaload
 *
 * Function:	   Load	a DSP program into the ACPA's memory
 * 
 *----------------------------------------------------------------------*/

int acpaload( Shorts *code, int	min )
/* code	is the DSP memory image	to load	*/
{
  int i;
  int j; /* DEBUG */
  unsigned short dat;
  unsigned short* c;
  unsigned short len;
#ifndef	i386
  unsigned short reversed_dat;
#endif /* not i386 */

  ENTER_ACPA_PTRACE( LOAD, ACPA_MAIN, min );

  if (code->len	<= 0 ||	code->data == NULL)
    return(-1);			/* Can't load it, no buffer available */
  else
    len	= code->len;		/* Put length in variable to cut down */
				/* on register use */

  acpaout(acpa[min].addr_low_reg, (unsigned short) 0);

  /* Write the data into the ACPA's Shared Memory */
  /* If	we discover that the endian-ness of the	data is	wrong, this */
  /* is	the place to fix it. */

  c = code->data;		/* Beginning of	data to	write */

  for (i = 0; i	< len; i++) {
    dat	= c[i];			/* Data	to write */
#ifdef i386
    acpaout(acpa[min].data_low_reg, dat);
#else
    reversed_dat = ((dat & 0x00ff) << 8) | ((dat & 0xff00) >> 8);
    acpaout(acpa[min].data_low_reg, reversed_dat);
#endif 

  }

  EXIT_ACPA_PTRACE( LOAD, ACPA_MAIN, min );

  return(0);			/* No errors, return to	caller */
}

/*----------------------------------------------------------------------
 * Function name:  acpaget
 *
 * Function:	   Get a word from the DSP memory
 * 
 *----------------------------------------------------------------------*/

int acpaget( int adr, int min )
/* adr is the address to read in ACPA memory */
{
  int rc;
  int oldspl;			/* Old interrupt level */

  oldspl = DISABLE_INTERRUPTS();		/* Disable interrupts */
  acpa[min].recursion =	4;   /*	Interrupts are fatal */

  acpaout(acpa[min].addr_low_reg,
	(unsigned short) adr); /* Tell card where we will read from */

  rc = acpain(acpa[min].data_low_reg); /* Read the data	from the DSP memory */

  acpa[min].recursion =	FALSE; /* Interrupts are OK now	*/
  (void) ENABLE_INTERRUPTS(oldspl);   /* Re-enable interrupts if appropriate */

  return(rc);			/* Return the data to the caller */

}


/*----------------------------------------------------------------------
 * Function name:  acpaput
 *
 * Function:	   Put a word into the DSP memory
 *
 *----------------------------------------------------------------------*/

void acpaput( int adr, unsigned	short dat, int min )
/* adr is the address to write in ACPA memory */
/* dat is the data to write into ACPA memory */
{
  int rc;
  int oldspl;			/* Old interrupt level */

  oldspl = DISABLE_INTERRUPTS();		       /* Disable interrupts */
  acpa[min].recursion =	3;   /*	Interrupts are fatal */

  acpaout(acpa[min].addr_low_reg,
	(unsigned short) adr);	     /*	Tell card where	we will	read from */

  (void) acpaout(acpa[min].data_low_reg, dat); /* Write	to the DSP memory */

  acpa[min].recursion =	FALSE; /* Interrupts are OK now	*/
  (void) ENABLE_INTERRUPTS(oldspl);	/* Re-enable interrupts	if necessary */

  return;

}


/*----------------------------------------------------------------------
 * Function name:  acpastop
 *
 * Function:	   Stop	the DSP	on the ACPA card
 * 
 *----------------------------------------------------------------------*/

int acpastop( struct acpa_status *acpa )
{

  ENTER_ACPA_PTRACE( STOP, ACPA_MAIN, -1 );

  /* See if an ACPA card exists	in the system; if so, reset the	TMS processor*/

  if (acpa->base != 0)
  {
    /* See p. 6-14 of the Technical Reference for information about this. */
    acpaoutb(acpa->cmd_stat_reg,
	     (unsigned char) 0);       /* Reset	the TMS, disable   */
				  /* host interrupts, disable internal spkr  */

    acpa->running = FALSE; /* DSP is stopped */
    acpa->ps2_speaker_enabled =	FALSE; /* PS/2 speaker not in use */
    acpa->can_interrupt_host = FALSE; /* Can't interrupt host */
  }

  EXIT_ACPA_PTRACE( STOP, ACPA_MAIN, -1	);

  return(0);			/* Return with no errors */
}

/*----------------------------------------------------------------------
 * Function name:  acpastart
 *
 * Function:	   Start the DSP on the	ACPA card
 * 
 * This	routine	is also	used to	turn on	the internal PS/2 speaker
 * and to enable and disable DSP interrupts. 
 *
 *----------------------------------------------------------------------*/

int acpastart( int min )
{
	unsigned char cmd;		/* Command to send to ACPA card	*/
	int oldspl;			/* Old interrupt level */

	ENTER_ACPA_PTRACE( START, ACPA_MAIN, min );

	/* IS THIS STILL NECESSARY? */
	/* Don't try this if the ACPA is not in	the system. */
	if ( acpa[min].base != 0 )
	{
		/* Disable all interrupts. */
		oldspl = DISABLE_INTERRUPTS();
		/* Interrupts are fatal	*/
		acpa[min].recursion = 5;

		/* Create the command to send to the DSP. */

		/* TMS_RES restarts the	DSP to operating state;	*/
		/* TMS_INT_CMD means that the host does	NOT interrupt */
		/* the DSP. */
		cmd = TMS_RES |	TMS_INT_CMD;
		if (acpa[min].can_interrupt_host == TRUE)
		{
#ifdef ACPA_MEMORY_TRACE
#endif
			/* Let DSP interrupt the host */
			cmd |= HINTENA;
		}
		if (acpa[min].ps2_speaker_enabled == TRUE)
			/* Enable the PS/2 speaker */
			cmd |= SKPR_EN;

		/* Issue the command. */
#ifdef ACPA_MEMORY_TRACE
#endif
		acpaoutb(acpa[min].cmd_stat_reg, cmd);

		/* Now the DSP is running. */
		acpa[min].running = TRUE;

		/* Interrupts are now permissable */
		acpa[min].recursion = FALSE;
		/* Enable interrupts again */
		(void) ENABLE_INTERRUPTS(oldspl);
#ifdef ACPA_MEMORY_TRACE
#endif
	}

	EXIT_ACPA_PTRACE( START, ACPA_MAIN, min	);

	return(0);
}



/*----------------------------------------------------------------------
 * Function name:  acpainitlz
 *
 * Function:	   Initialize the acpa structure and reset the DSP to
 *		   a known state
 * 
 * This	routine	is used	by both	the PS/2 and V3	versions
 *
 *----------------------------------------------------------------------*/

int acpainitlz(acpa)
struct acpa_status* acpa;	/* Ptr to the main data	structure */
{
  int i;			/* Loop	counter	*/
  int j;			/* Loop	counter	*/
  int rc;			/* Return code from called routines */
  extern int acpastop();
  
  ENTER_ACPA_PTRACE( INITZ, ACPA_MAIN, -1 );

  /* Precalculate addresses of all the I/O ports */
  
  acpa->data_low_reg = acpa->base + DATA_LOW_REG;
  acpa->data_hi_reg = acpa->base + DATA_HI_REG;
  acpa->addr_low_reg = acpa->base + ADDR_LOW_REG;
  acpa->addr_hi_reg = acpa->base + ADDR_HI_REG;
  acpa->cmd_stat_reg = acpa->base + CMD_REG;
  
  /* Stop the DSP and reset its	state */
  acpastop( acpa );
  
#ifdef i386
  acpa->ucode_loaded = NULL;		/* No microcode	loaded yet */
  acpa->playback_ucode.len = 0;/* No play microcode image avail	yet*/
  acpa->playback_ucode.data = NULL;    /* Really no play microcode */
  acpa->record_ucode.data = NULL;/* No record microcode	image avail*/
  acpa->record_ucode.len = 0;	  /* Really no recording microcode */
  acpa->playback_pcm_code.len =	0;   /*	No play	ucode image avail yet*/
  acpa->playback_pcm_code.data = NULL;	 /* Really no play microcode */
  acpa->record_pcm_code.data = NULL;   /* No record ucode image	avail*/
  acpa->record_pcm_code.len = 0;    /* Really no recording microcode */
#endif
  
  acpa->int_count = 0;		      /* No interrupts from ACPA yet */
  
  acpa->recursion = FALSE;	/* No illegal recursion	has occurred */
  
  /* Set all memory in the ACPA	card to	a test pattern.	*/
  
  acpaout(acpa->addr_low_reg, 
	(unsigned short) 0x0000); /* DSP address to start with */
  
  for (i=0; i <	8*1024;	i++) {
    acpaout(acpa->data_low_reg,	
	  (unsigned short) i);
  }
 
  /* Read the test pattern back	in */

  acpaout(acpa->addr_low_reg, (unsigned	short) 0x0000);
  for (i = 0 ; i < 8 * 1024; i ++) {
   rc =	acpain(acpa->data_low_reg);
   if (rc != i)	{
     TRACE(("Error reading back	test pattern, should be	%x, is %x\n",
		i, rc));
     return(-1);
   }	  
  }

  acpaout(acpa->addr_low_reg, (unsigned	short) 0x0000);
  for (i=0; i <	8*1024;	i++) {
    acpaout(acpa->data_low_reg,
	  (unsigned short) 0);
  }

  /* Note that the buffers in the kernel ring have not been */
  /* acquired.	The necessary buffers will be acquired at open time */
  
  for (i = 0; i	< 2; i++) {	/* Once	for each channel */
    for	(j = 0;	j < MAXNKB; j++) { /* Once for each kernel buffer */
      acpa->track[i].rbuf[j] = NULL;
      acpa->track[i].wbuf[j] = NULL;
    }
  }
 
  EXIT_ACPA_PTRACE( INITZ, ACPA_MAIN, -1 );

  return(0);
 
}

/* The following routines are AIXv3 specific */

#ifndef	i386
/*----------------------------------------------------------------------
 * Function name:  acpaout
 *
 * Function:	   Write a short to a given I/O	port on	a V3 system
 *
 *----------------------------------------------------------------------*/

void acpaout(int port, unsigned	short data)
{
  unsigned char	*c;
  unsigned short *p;
  ulong	bus_val;

  bus_val = BUSIO_ATT( BUS_ID, 0 );
  c = (char *) bus_val + port;
  p = (unsigned	short *) c;
  BUSIO_PUTS( p, ((data	>> 8) |	(data << 8)) );
  BUSIO_DET(bus_val);
#ifdef MORE
  acpaoutb(port, (unsigned char) (data & 0x000000ff));	/* Low order byte */
  acpaoutb(port+1, (unsigned char) ((data & 0x0000ff00)	>> 8));	/* Hi byte */
#endif
}

/* This	routine	write 8	bits to	the specified bus address */
void acpaoutb(int port,	unsigned char data)
{
	volatile unsigned char*	p;
	ulong bus_val;

	/* BUSIO_ATT and BUS_ID	are used to talk to the	adapter. */
	/* IOCC_ATT and	IOCC_ID	are used to talk to POS	registers/bus. */
	bus_val	= BUSIO_ATT(BUS_ID,0);	/* setup segment register */
	p = bus_val + port;		/* Addr	of I/O port */
	BUSIO_PUTC(p, data);		/* Write data to the port */
	BUSIO_DET(bus_val);		/* Restore bus segment reg value */
}

void acpain_mult( int port, int	size, unsigned short *dest )
{
  volatile unsigned short *p;
  volatile unsigned short rc;
  volatile int i;

  p = (unsigned	short *) BUSIO_ATT( BUS_ID, 0 );
  for (	i=0; i<size; i++ )
  {
    rc = *(p + (port/2));
    *dest++ = rc;
  }
  BUSIO_DET((ulong) p);
}

unsigned short acpain(int port)
{
  unsigned short *p;
  unsigned short rc;

  p = (unsigned	short *) BUSIO_ATT( BUS_ID, 0 );
  rc = *(p + (port/2));
  BUSIO_DET((ulong) p);
  return( (rc>>8) | (rc<<8));
#ifdef MORE
  return ((unsigned short) ((acpainb(port)) + (acpainb(port+1) << 8)));
#endif

}


/*---------------------*/
/* acpa	i n b	    */
/* --------------------------------------------------------------------
** Function name:  acpainb
**
** Function:	   Read	a character from a given I/O port on V3	system
**
**---------------------------------------------------------------------
*/

unsigned char
acpainb(int port)
{
volatile unsigned char*	p;
caddr_t	bus_val;
unsigned char rc;

bus_val	= BUSIO_ATT(IOCC_BID,0);  /* setup segment register */
p = bus_val + port; /* Addr of I/O port	*/
rc = BUSIO_GETC(p);	  /* Read data from the	port */
BUSIO_DET(bus_val);	      /* Restore bus segment register value*/
return(rc);	      /* Return	data to	caller */
}





/*-----------------------------------------------*/
/*						 */
/*						 */
/* Load	the audio microcode into system	memory.	*/
/*						 */
/*						 */
/*-----------------------------------------------*/

long load_ucode_into_memory(fname, ucode_base)
char *fname;		/*pointer to file name	of the ucode */
Shorts *ucode_base;	/* ucode descriptor */
{

char		*bufp;
char		diagbuf[12];
int		ucfd;
struct		file	*acpa_ucode_file_ptr;
int		numbytes;		/* number of bytes transferred */

  ENTER_ACPA_PTRACE( CFGINIT, ACPA_SUBR_ONE, -1	);

    /*----------------------------------------------------*/
    /* attempt to open the microcode file, then verify,	  */
    /* using the magic number that it contains microcode. */
    /*----------------------------------------------------*/
    if ( ( ucfd = fp_open( fname, O_RDONLY, 0, 0, SYS_ADSPACE,
                           &acpa_ucode_file_ptr) ) != 0 )
       return(-1);

    /* Determine whether this is a diagnostics call at config time. */
    /* If it is, the first 11 bytes will contain the string */
    /* "ACPA977DIAG". */
    if ((ucfd =	fp_read(acpa_ucode_file_ptr, &diagbuf[0], 11, 0,
			UIO_SYSSPACE, &numbytes)) != 0)
      return( - 2 );
    else
    {
      diagbuf[11] = 0;	/* end the string with a null */
      if ( ( ucfd = strncmp( bufp, "ACPA977DIAG", 11 ) ) == 0 )
        /* The string was found; return without doing anything else. */
        return( 0 );
      else /* rewind back to the start of the file */
        if ((ucfd = fp_lseek(acpa_ucode_file_ptr, 0, 0 )) == -1)
          return(-3);
    }

#define	COPYRIGHT_LEN (81)		   /* Length of	copyright notice at */

    if ((ucfd =	fp_lseek(acpa_ucode_file_ptr, COPYRIGHT_LEN, 0 )) == -1)
      return(-4);

    bufp = (char *)xmalloc( 16*1024, 3,	kernel_heap );
    if (bufp ==	NULL)
      return(-5);

    if ((ucfd =	fp_read(acpa_ucode_file_ptr, bufp, 16*1024, 0,
			UIO_SYSSPACE, &numbytes)) != 0)
      return(-6);

    if ((ucfd =	fp_close(acpa_ucode_file_ptr)) != 0 )
      return(-7);

    ucode_base->data = (unsigned short *)bufp;
    ucode_base->len = numbytes;

    EXIT_ACPA_PTRACE( CFGINIT, ACPA_SUBR_ONE, -1 );

    return( 0 );		/* end of load_ucode_into_memory */
}

/* watch dog timer */
/* 
   This	function gets invoked if the watchdog timer expires.
   It must stop	the timer and break the	sleep for the device
   interrupt.
*/

void timed_out (w)
struct wdog_info *w;

{
  int event;
  int min;
  int mpxchannel;

  ENTER_ACPA_PTRACE( TIMEDOUT, ACPA_MAIN, -1 );

  event	= w->event_type;
  min =	w->min;
  mpxchannel = w->mpxchannel;

  /* Set an unsuccessful return code for the timed sleep */
  acpa[min].sleep_code = SLEEP_TIMED_OUT;

  /* break the sleep */
  if ( event ==	RING_EVENT )
    e_wakeup( &acpa[min].track[mpxchannel].ring_event );
  if ( event ==	STOP_EVENT )
    e_wakeup( &acpa[min].track[mpxchannel].stop_event );

  /* There is no reason to call w_stop here because the system has */
  /* already done it for us by virtue of this routine being called. */

  EXIT_ACPA_PTRACE( TIMEDOUT, ACPA_MAIN, -1 );

}

#endif /* not i386 */

static int debug_line =	-1;

debug_setlocation( line	)
int line;
{
	debug_line = line;
}

debug_print( f,	s0, s1,	s2, s3,	s4, s5,	s6, s7,	s8, s9)	/* limit of ten	args */
char *f;
char *s0, *s1, *s2, *s3, *s4, *s5, *s6,	*s7, *s8, *s9;
{
	static char  buff[256];

	/* Output the line number of the driver	message	*/
	sprintf(buff, "Audio DD	line %d: ", debug_line);

	sprintf(&(buff[strlen(buff)]), f, s0, s1, s2, s3, s4, s5, s6, s7, s8, s9);

	printf(	buff );

}

/* short version that doesn't print out	the line number	*/
debug_prints( f, s0, s1, s2, s3, s4, s5, s6, s7, s8, s9) /* limit of ten args */
char *f;
char *s0, *s1, *s2, *s3, *s4, *s5, *s6,	*s7, *s8, *s9;
{
	static char  buff[256];

	sprintf(&(buff[0]), f, s0, s1, s2, s3, s4, s5, s6, s7, s8, s9);

	printf(	buff );

}



void acpa_bus_delay(delay_time)
long delay_time;		   /* number of	microseconds to	delay */
{
	volatile	long	micro_s;
			ulong   bus_val;   /* value read from BUSACC() */
			uchar	*delay_ptr;
        		uchar   val;


	bus_val = BUSACC(0x820C0060);   /* get access to the I/O bus */

	delay_ptr = (uchar *)(BUSIO() +	0x80E7 /* IO_DELAY */);

	for (micro_s = 0; micro_s < delay_time;	micro_s++)
		val = BUSIO_GETC(delay_ptr);

	BUSREST(bus_val);   /* release access to the I/O bus */

} /* end of acpa_bus_delay */



/************************************************************************/
/*									*/
/*  FUNCTION NAME: acpa_init_request					*/
/*									*/
/*  DESCRIPTION:							*/
/*									*/
/*	This function implements the AUDIO_INIT	request	for the		*/
/*	AUDIO_CONTROL ioctl.						*/
/*									*/
/*  INVOCATION:								*/
/*									*/
/*	rc = acpa_init_request(	audio_init *init, int min,		*/
/*				ulong openflags, int mpxchan )		*/
/*									*/
/*  INPUT PARAMETERS:							*/
/*									*/
/*	init is	a pointer to the AUDIO_INIT request specifics.		*/
/*	min is the minor number	for the	adapter	being used.		*/
/*	openflags contains the flags used with the open() system	*/
/*		used on	this adapter.					*/
/*	mpxchan	is the channel id for the adapter being	used.		*/
/*									*/
/*  OUTPUT PARAMETERS:							*/
/*									*/
/*	N/A								*/
/*									*/
/*  RETURN VALUES:							*/
/*									*/
/*	0 if successful							*/
/*	-1 if unsuccessful.  In	this case errno	will also be set.	*/
/*									*/
/************************************************************************/

#define	TURN_BESTFIT_ON	local_flags |= BESTFIT_PROVIDED

/* This	routine	satisfies the AUDIO_INIT request */
int acpa_init_request( audio_init *init, int min, ulong	openflags,
  int mpxchan )
{
  int rc = 0;
  int i, j;
  struct xmem dp[2];		/* used	to attach to user memory */
  track_info tinfo;		/* local copy of track information */
  audio_init iinfo;		/* local copy of init information */
  register int mpxchannel;	/* the transformed mpx channel number */
  Shorts* code_to_load = NULL;	/* What	set of microcode must we load? */
  int otherchan;		/* the other track */
  register struct acpa_status *aptr; /*	ptr to current adapter's status	*/
  register Track *tptr;		/* pointer to current track info */
  int oldrnkb;			/* previous number of read buffers */
  int oldwnkb;			/* previous number of write buffers */
  unsigned long	local_flags;	/* local copy of user's	flag field */
  extern void init_audio_buffer(); /* initializes audio_buffer fields */
  void acpa_bus_delay();

  ENTER_ACPA_PTRACE( INIT, ACPA_MAIN, min );

  mpxchannel = ( mpxchan & 3 ) - 1;	/* this	is used	as an index */

  aptr = (struct acpa_status *)	&( acpa[min] );
  tptr = (Track	*) &( acpa[min].track[mpxchannel] );

  /* Copy the user's buffer into the kernel space. */
  if ( ( rc = copyin( (char *) init, (char *) &iinfo,
	 sizeof(audio_init) ) )	!= 0 )
    return( EFAULT );

  iinfo.rc = 0;

  /* There is no support for read/write	operations yet (WAIT FOR MIDI).	*/
  if ( ( openflags & FREAD ) &&	( openflags & FWRITE ) )
    iinfo.rc = CONFLICT;
  /* Verify that the requested operation is supported and not */
  /* contradictory. */
  if ( ! iinfo.rc )
  {
    switch( iinfo.operation )
    {
      case PLAY	:
	if ( openflags & FREAD )
	  iinfo.rc = CONFLICT;
	else
	  iinfo.rc = 0;
	break;
      case RECORD :
	if ( openflags & FWRITE	)
	  iinfo.rc = CONFLICT;
	else
	  iinfo.rc = 0;
	break;
      default :
	iinfo.rc = INVALID_REQUEST;
	break;
    }
  }
  if ( iinfo.rc	)
    /* Need to return the error	code to	the user. */
    if ( ( rc =	copyout( (char *) &(iinfo.rc), (char *)	&(init->rc),
	   sizeof(init->rc) ) )	!= 0 )
      return( EFAULT );
    else
      return( EINVAL );

  local_flags =	0;			/* Turn	off all	flags. */

  /* Now set the initialization	fields up according to the user's */
  /* specifications. */

  /* The order of precedence --	from highest to	lowest -- is mode, flags, */
  /* sampling rate, bits per sample, and number	of channels. */
  switch( iinfo.mode )
  {
    case SOURCE_MIX :
      /* Reject source mix if request was for stereo - not supported */

      if (iinfo.channels == 2)
        return(EINVAL);

      TURN_BESTFIT_ON;
      /* Signed format is used because its silence value is 0, */
      /* which must be preloaded on the adapter's buffers before starting */
      /* the adapter in source mix mode. */
      local_flags = FIXED | SIGNED;
      iinfo.srate = 8000;
      iinfo.bits_per_sample = 16;
      iinfo.channels = 1;
      break;
    case ADPCM :
      /* Examine the flags field. */
      if ( iinfo.flags & ~( FIXED ) )
      {
	/* Remove any flags that are not supported. */
	iinfo.flags &= ( FIXED );
	TURN_BESTFIT_ON;
      }
      local_flags = iinfo.flags;
      if ( !( local_flags | FIXED ) )
      {
	local_flags = iinfo.flags | FIXED;    /* Ensure	that FIXED is on. */
	TURN_BESTFIT_ON;
      }

      /* Now examine the sampling rate field. */
      if ( iinfo.srate != AUDIO_IGNORE )
	/* Determine whether we	are making an approximation. */
	switch ( iinfo.srate )
	{
	  /* These values are not approximations. */
	  case 5500 :
	  case 11025 :
	  case 22050 :
	    break;
	  case 44100 :  /* This rate is invalid for ADPCM */
            iinfo.srate = 22050;
	    TURN_BESTFIT_ON;
	    break;
	  default :
	    /* Any other value is an approximation. */
	    TURN_BESTFIT_ON;
	    /* is user's rate <= ( 5500	+ 11025	) / 2 ?	*/
	    if ( iinfo.srate <=	8262 )
	      iinfo.srate = 5500;
	    /* is user's rate <= ( 11025 + 22050 ) / 2 ? */
	    else if ( iinfo.srate <= 16537 )
	      iinfo.srate = 11025;
            else
              iinfo.srate = 22050;
	}
      else
      {
	/* Use the default setup -- 5.5	kHz. */
	iinfo.srate = 5500;
	TURN_BESTFIT_ON;
      }

      /* Now examine the bits per sample field.	*/
      /* Currently only	16 bits	per sample are supported. */
      switch( iinfo.bits_per_sample )
      {
	case 16	:
	  break;
	default	:
	  iinfo.bits_per_sample	= 16;
	  TURN_BESTFIT_ON;
      }

      /* Now examine the channels field. */
      /* Stereo	is not supported at 5.5 and 22 kHz ADPCM. */
      switch( iinfo.channels )
      {
	case 1 :
	  break;
	case 2 :
	  if ( iinfo.srate != 11025 )
	  {
	    iinfo.channels = 1;
	    TURN_BESTFIT_ON;
	  }
	  break;
	default	:
	  iinfo.channels = 1;
	  TURN_BESTFIT_ON;
      }
      break;
    case MU_LAW	:
    case A_LAW :
      if ( iinfo.flags & ~( FIXED | BIG_ENDIAN ) )
      {
	/* Remove any flags that are not supported. */
	iinfo.flags &= ( FIXED | BIG_ENDIAN );
	TURN_BESTFIT_ON;
      }
      local_flags = iinfo.flags;
      if ( !( local_flags | FIXED ) )
      {
	local_flags = iinfo.flags | FIXED;    /* Ensure that FIXED is on. */
	TURN_BESTFIT_ON;
      }

      /* Now examine the sampling rate field. */
      if ( iinfo.srate != AUDIO_IGNORE )
	/* Determine whether we	are making an approximation. */
	switch ( iinfo.srate )
	{
	  case 8000 :
	  case 11025 :
	  case 22050 :
	  case 44100 :
	    /* These values mean we are	not making an approximation. */
	    break;
	  default :
	    /* Any other value is an approximation. */
	    TURN_BESTFIT_ON;
	    /* is user's rate <= ( 8000	+ 11025	) / 2 ?	*/
	    if ( iinfo.srate <=	9512 )
	      iinfo.srate = 8000;
	    /* is user's rate <= ( 11025 + 22050 ) / 2 ? */
	    else if ( iinfo.srate <= 16537 )
	      iinfo.srate = 11025;
	    /* is user's rate <= ( 22050 + 44100 ) / 2 ? */
	    else if ( iinfo.srate <= 33075 )
	      iinfo.srate = 22050;
	    else
	      iinfo.srate = 44100;
	}
      else
      {
	iinfo.srate = 11025;
	TURN_BESTFIT_ON;
      }

      /* Now examine the bits per sample field.	*/
      switch( iinfo.bits_per_sample )
      {
	case 8 :
	  break;
	default	:
	  iinfo.bits_per_sample	= 8;
	  TURN_BESTFIT_ON;
      }

      /* Now examine the channels field. */
      switch( iinfo.channels )
      {
	case 1 :
	  break;
	case 2 :
	  if ( iinfo.srate == 44100 )
	  {
	    /* Mu-law and A-law	don't support 44 kHz stereo. */
	    iinfo.channels = 1;
	    TURN_BESTFIT_ON;
	  }
	  break;
	default	:
	  iinfo.channels = 1;
	  TURN_BESTFIT_ON;
      }
      break;
    case PCM :
      if ( iinfo.flags & ~( FIXED | TWOS_COMPLEMENT | SIGNED | BIG_ENDIAN ) )
      {
	/* Remove any flags that are not supported. */
	iinfo.flags &= ( FIXED | TWOS_COMPLEMENT | SIGNED | BIG_ENDIAN );
	TURN_BESTFIT_ON;
      }
      local_flags = iinfo.flags;
      if ( !( local_flags | FIXED ) )
      {
	local_flags = iinfo.flags | FIXED;    /* Ensure	that FIXED is on. */
	TURN_BESTFIT_ON;
      }
      /* Some of these flags are mutually exclusive; ensure that. */
      if ( ( local_flags & TWOS_COMPLEMENT ) &&	( local_flags &	SIGNED ) )
      {
	/* Turn	off twos complement. */
	local_flags &= ~TWOS_COMPLEMENT;
	TURN_BESTFIT_ON;
      }

      /* Now examine the sampling rate field. */
      if ( iinfo.srate != AUDIO_IGNORE )
	/* Determine whether we	are making an approximation. */
	switch ( iinfo.srate )
	{
	  case 8000 :
	  case 11025 :
	  case 22050 :
	  case 44100 :
	    /* These values mean we are	not making an approximation. */
	    break;
	  default :
	    /* Any other value is an approximation. */
	    TURN_BESTFIT_ON;
	    /* is user's rate <= ( 8000	+ 11025	) / 2 ?	*/
	    if ( iinfo.srate <=	9512 )
	      iinfo.srate = 8000;
	    /* is user's rate <= ( 11025 + 22050 ) / 2 ? */
	    else if ( iinfo.srate <= 16537 )
	      iinfo.srate = 11025;
	    /* is user's rate <= ( 22050 + 44100 ) / 2 ? */
	    else if ( iinfo.srate <= 33075 )
	      iinfo.srate = 22050;
	    else
	      iinfo.srate = 44100;
	}
      else
      {
	iinfo.srate = 11025;
	TURN_BESTFIT_ON;
      }

      /* Now examine the bits per sample field.	*/
      switch( iinfo.bits_per_sample )
      {
	case 8 :
	case 16	:
	  break;
	default	:
	  iinfo.bits_per_sample	= 8;
	  TURN_BESTFIT_ON;
      }

      /* Now examine the channels field. */
      switch( iinfo.channels )
      {
	case 1 :
	case 2 :
	  break;
	default	:
	  iinfo.channels = 1;
	  TURN_BESTFIT_ON;
      }
      break;
    default :
      TURN_BESTFIT_ON;		/* Because no mode was specified. */
      /* There is no need to examine the flags field in	this case. */
      local_flags |= FIXED;	/* Only	FIXED is supported right now. */

      /* Now examine the sampling rate field. */
      if ( iinfo.srate != AUDIO_IGNORE )
	/* Determine whether we	are making an approximation. */
	switch ( iinfo.srate )
	{
	  case 5500 :
	    iinfo.mode = ADPCM;
	    break;
	  case 8000 :
	  case 11025 :
	  case 22050 :
	  case 44100 :
	    iinfo.mode = PCM;
	    break;
	  default :
	    /* Any other value is an approximation. */
	    TURN_BESTFIT_ON;
	    /* is user's rate <= ( 5500	+ 8000 ) / 2 ? */
	    if ( iinfo.srate <=	6750 )
	    {
	      iinfo.srate = 5500;
	      iinfo.mode = ADPCM;
	    }
	    /* is user's rate <= ( 8000	+ 11025	) / 2 ?	*/
	    if ( iinfo.srate <=	9512 )
	    {
	      iinfo.srate = 8000;
	      iinfo.mode = PCM;
	    }
	    /* is user's rate <= ( 11025 + 22050 ) / 2 ? */
	    else if ( iinfo.srate <= 16537 )
	    {
	      iinfo.srate = 11025;
	      iinfo.mode = PCM;
	    }
	    /* is user's rate <= ( 22050 + 44100 ) / 2 ? */
	    else if ( iinfo.srate <= 33075 )
	    {
	      iinfo.srate = 22050;
	      iinfo.mode = PCM;
	    }
	    else
	    {
	      iinfo.srate = 44100;
	      iinfo.mode = PCM;
	    }
	}
      else
      {
	/* Use the default setup -- 5.5	kHz. */
	iinfo.srate = 5500;
	iinfo.mode = ADPCM;
      }

      /* Now examine the bits per sample field.	*/
      switch( iinfo.bits_per_sample )
      {
	case 8 :
	  if ( iinfo.mode == ADPCM )
	  {
	    iinfo.bits_per_sample = 16;
	    TURN_BESTFIT_ON;
	  }
	  break;
	case 16	:
	  break;
	default	:
	  iinfo.bits_per_sample	= 16;
	  TURN_BESTFIT_ON;
      }

      /* Now examine the channels field. */
      switch( iinfo.channels )
      {
	case 1 :
	  break;
	case 2 :
	  if ( iinfo.srate == 5500 || 
               (iinfo.srate == 22050 && iinfo.mode == ADPCM))
	  {
	    iinfo.channels = 1;
	    TURN_BESTFIT_ON;
	  }
	  break;
	default	:
	  iinfo.channels = 1;
	  TURN_BESTFIT_ON;
      }
      break;
  }

  /* If user wants ADPCM at 22050 kHz on track 2 return unsuccessfully */
  /* since only track 1 is supported at that rate.                     */
  if ((iinfo.mode == ADPCM) && (iinfo.srate == 22050) && (mpxchannel == 1))
    return(EINVAL);

  /* Now tell the application which capabilities are supported.	*/
  if ( iinfo.mode != SOURCE_MIX )
    local_flags |= INPUT | OUTPUT | MONITOR | VOLUME | VOLUME_DELAY |
		   BALANCE | BALANCE_DELAY;
  else
    local_flags |= INPUT | OUTPUT | VOLUME | VOLUME_DELAY | BALANCE | 
                   BALANCE_DELAY;

  iinfo.flags =	local_flags;	/* Return this info to the application.	*/

  /* Now determine what	the appropriate	microcode module is. */
  switch( iinfo.srate )
  {
    case 5500 :
      iinfo.bsize = 576;
      tptr->modulo_factor = tptr->preload_max =	4;
      switch( aptr->doing_what )
      {
	case Recording :
	  code_to_load = &record_ucode;
	  break;
	case Playing :
	  code_to_load = &playback_ucode;
	  break;
	default	:
	  /* WHAT OTHER	STATES CAN THERE BE? */
	  break;
      }
      break;
    case 11025 :
      if ( iinfo.mode == ADPCM )
      {
	if ( iinfo.channels == 2 )
	{
	  iinfo.bsize =	2256;
	  tptr->modulo_factor =	tptr->preload_max = 4;
	}
	else
	{
	  iinfo.bsize =	1128;
	  tptr->modulo_factor =	tptr->preload_max = 4;
	}
	switch(	aptr->doing_what )
	{
	  case Recording :
	    code_to_load = &record_ucode;
	    break;
	  case Playing :
	    code_to_load = &playback_ucode;
	    break;
	  default :
	    /* WHAT OTHER STATES CAN THERE BE? */
	    break;
	}
      }
      else		/* this	is PCM */
      {
	iinfo.bsize = 560;
	switch(	aptr->doing_what )
	{
	  case Recording :
	    tptr->modulo_factor	= tptr->preload_max = 16;
	    code_to_load = &record_pcm_ucode;
	    break;
	  case Playing :
	    if ( iinfo.channels	== 2 )
	      tptr->modulo_factor = tptr->preload_max =	16;
	    else
	      tptr->modulo_factor = tptr->preload_max =	8;
	    code_to_load = &playback_pcm_ucode;
	    break;
	  default :
	    /* WHAT OTHER STATES CAN THERE BE? */
	    break;
	}
      }
      break;
    case 22050 :
      if ( iinfo.mode == ADPCM )
      {
	iinfo.bsize = 1128;
	tptr->modulo_factor = tptr->preload_max = 8;

	switch(	aptr->doing_what )
	{
	  case Recording :
	    code_to_load = &record_22_ucode;
	    break;
	  case Playing :
	    code_to_load = &playback_22_ucode;
	    break;
	  default :
	    /* WHAT OTHER STATES CAN THERE BE? */
	    break;
	}
      }
      else
      {
        iinfo.bsize = 560;
        switch( aptr->doing_what )
        {
	  case Recording :
	    tptr->modulo_factor = tptr->preload_max = 16;
	    code_to_load = &record_pcm_ucode;
	    break;
	  case Playing :
	    if ( iinfo.channels == 2 )
	      tptr->modulo_factor = tptr->preload_max = 16;
	    else
	      tptr->modulo_factor = tptr->preload_max = 8;
	    code_to_load = &playback_pcm_ucode;
	    break;
	  default :
	    /* WHAT OTHER STATES CAN THERE BE? */
	    break;
        }
      }
      break;
    case 8000 :
    case 44100 :
      iinfo.bsize = 560;
      switch( aptr->doing_what )
      {
	case Recording :
	  tptr->modulo_factor =	tptr->preload_max = 16;
	  code_to_load = &record_pcm_ucode;
	  break;
	case Playing :
	  if ( iinfo.channels == 2 )
	    tptr->modulo_factor	= tptr->preload_max = 16;
	  else
	    tptr->modulo_factor	= tptr->preload_max = 8;
	  code_to_load = &playback_pcm_ucode;
	  break;
	default	:
	  /* WHAT OTHER	STATES CAN THERE BE? */
	  break;
      }
      break;
    default :
      break;
  }
  tptr->preload_max -= 1;       /* to prevent counter wrapping, make the */
				/* preload maximum one less than the */
				/* number required to wrap around. */

  /* Determine what the	other track is.	There are only 2 possible tracks. */
  if ( mpxchannel == 1 )
    otherchan =	0;
  else
    otherchan =	1;

  /* Two tracks	can play simultaneously	if neither is stereo and both */
  /* are of the	same type. */
  if ( ( aptr->doing_what == Playing ) &&
       ( aptr->track[otherchan].in_use == TRUE ) )
  {
     /*	Both tracks have been opened for playing; check	specification. */
     /* If either track is stereo, reject the request. */
     if	( iinfo.channels == 2 )
       return( EINVAL );
     if ( aptr->track[otherchan].rinfo.channels == 2 )
       return( EINVAL );
     else
     {
       /* The 2 modes must be the same to be valid unless one is */
       /* source mix.  Then the other must be PCM. */
       if ( aptr->track[otherchan].rinfo.mode != iinfo.mode )
       {
	 /* If source mix is used on this channel, it can't be used on */
	 /* the other channel. */
	 if ( iinfo.mode == SOURCE_MIX )
	 {
	   if ( aptr->track[otherchan].rinfo.mode != PCM )
	     return( EINVAL );
	 }
	 else
	   if ( aptr->track[otherchan].rinfo.mode == SOURCE_MIX )
	   {
	     if ( iinfo.mode != PCM )
	       return( EINVAL );
	   }
           else
             return( EINVAL );
       }
       else
	 if ( iinfo.mode == SOURCE_MIX )
	   return( EINVAL );

       /* Reject those modes that don't support 2 tracks simultaneously. */
       switch( iinfo.mode )
       {
	 case MU_LAW :
	 case A_LAW :
	   return( EINVAL );
	   break;
	 case ADPCM :
	   if ( iinfo.srate == 22050 )
	     return( EINVAL );
	   break;
	 default :
	   break;
       }
     }
     aptr->secondtime =	TRUE;	/* This	is a second simultaneous open */
  }
  else
  {
    /* If the adapter is in Stopped state, issue the stop command to the */
    /* microcode, giving it enough time	to stop	its activity. */
    if ( tptr->current_state ==	Stopped	)
    {
      tptr->current_state = Opened;
      if ( mpxchannel == 0 )
	acpaput( DSP_T1_MAILBOX_BASE + DSP_TX_CMD_FROM_HOST,
		 (unsigned short) 0, min );
      else
	acpaput( DSP_T2_MAILBOX_BASE + DSP_TX_CMD_FROM_HOST,
		 (unsigned short) 0, min );
      acpa_bus_delay( 2000 );
      (void) acpastop( aptr );
      tptr->host_buf_ptr = 0;
    }
    /* Now load	the microcode onto the adapter,	but only if it is not */
    /* already loaded.	Two simultaneous opens are permitted only if they */
    /* are for playback	and use	the same microcode. */
    rc = 0;
    if ( code_to_load != NULL )
    {
     /*	The requested functionality is supported. */
      if ( ( code_to_load->len > 0 ) &&	( code_to_load->data !=	NULL ) )
      {
	/* Now stop the	adapter	to ensure that any previously loaded */
	/* microcode is	not going to overwrite code or data before the */
	/* new microcode is activated. */
	acpaoutb(acpa[min].cmd_stat_reg, (unsigned char) 0);

	/* The requested microcode was loaded into memory.  Now	load it	*/
	/* onto	the adapter. */
	rc = acpaload( code_to_load, min ); /* Load the	DSP code */
	if ( rc	)
	  rc = ENXIO;	  /* No	code to	load, can't open the file */
	else
	  /* Remember that the code is now loaded */
	  aptr->ucode_loaded = code_to_load;

	/* Now ensure that the microcode has time to initialize	itself */
	/* before doing	anything else. */
	/* Write out any value to 0xe1d. */
	acpaput( DSP_T1_MAILBOX_BASE + DSP_TX_TMS_STATUS,
		 (unsigned short) 0x0F0F, min );
	/* Reset the adapter. */
	rc = TMS_RES | TMS_INT_CMD;
	acpaoutb(aptr->cmd_stat_reg, (unsigned char) rc);
	/* Now wait for	the register to	get cleared. */
	for ( i=0; i<0xffffffff; i++ )
	{
	  rc = (int) acpaget( DSP_T1_MAILBOX_BASE + DSP_TX_TMS_STATUS,
			      min );
	  if ( !rc )
	    break;
	}
	/* Wait	one more millisecond, and then everything is OK. */
        acpa_bus_delay( 2000 );
      }
      else
	return(	EFAULT );
    }
    else
      rc = EBUSY;	  /* The requested functionality is not	supported */
    if ( rc )
      return( rc );
#ifdef ACPA_MEMORY_TRACE
#endif
  }

  if ( iinfo.mode == ADPCM )
    /* ADPCM resolution	is stored as milliseconds. */
  {
    if (iinfo.srate == 22050)
      iinfo.position_resolution = 50;
    else
      iinfo.position_resolution = 100;
  }
  else
  {
    /* PCM resolution is in bytes, so must be converted	to milliseconds. */
    switch( iinfo.srate	)
    {
      case 8000	:
	if ( iinfo.channels == 2 )
	  if ( iinfo.bits_per_sample ==	8 )
	    /* 8000*2/560 = 28.571 intrs/sec, about 35 millisecs/intr */
	    iinfo.position_resolution =	35;
	  else
	    /* 8000*2*2/560 = 57.143 intrs/sec,	or about 17 millisecs/intr */
	    iinfo.position_resolution =	17;
	else
	  if ( iinfo.bits_per_sample ==	8 )
	    /* 8000/560	= 14.286 intrs/sec, or about 70	millisecs/intr */
	    iinfo.position_resolution =	70;
	  else
	    /* 8000*2/560 = 28.571 intrs/sec, or about 35 millisecs/intr */
	    iinfo.position_resolution =	35;
	break;
      case 11025 :
	if ( iinfo.channels == 2 )
	  if ( iinfo.bits_per_sample ==	8 )
	    /* 11025*2/560 = 39.375 intrs/sec, about 25	millisecs/intr */
	    iinfo.position_resolution =	25;
	  else
	    /* 11025*2*2/560 = 78.75 intrs/sec,	or about 13 millisecs/intr */
	    iinfo.position_resolution =	13;
	else
	  if ( iinfo.bits_per_sample ==	8 )
	    /* 11025/560 = 19.6875 intrs/sec, or about 51 millisecs/intr */
	    iinfo.position_resolution =	51;
	  else
	    /* 11025*2/560 = 39.375 intrs/sec, or about	25 millisecs/intr */
	    iinfo.position_resolution =	25;
	break;
      case 22050 :
	if ( iinfo.channels == 2 )
	  if ( iinfo.bits_per_sample ==	8 )
	    /* 22050*2/560 = 78.75 intrs/sec, or about 13 millisecs/intr */
	    iinfo.position_resolution =	13;
	  else
	    /* 22050*2*2/560 = 157.5 intrs/sec,	or about 6 millisecs/intr */
	    iinfo.position_resolution =	6;
	else
	  if ( iinfo.bits_per_sample ==	8 )
	    /* 22050/560 = 39.375 intrs/sec, or	about 25 millisecs/intr	*/
	    iinfo.position_resolution =	25;
	  else
	    /* 22050*2/560 = 78.75 intrs/sec, or about 13 millisecs/intr */
	    iinfo.position_resolution =	13;
	break;
      case 44100 :
	if ( iinfo.channels == 2 )
	  if ( iinfo.bits_per_sample ==	8 )
	    /* 44100*2/560 = 157.5 intrs/sec, or about 6 millisecs/intr	*/
	    iinfo.position_resolution =	6;
	  else
	    /* 44100*2*2/560 = 315 intrs/sec, or 3 millisecs/intr */
	    iinfo.position_resolution =	3;
	else
	  if ( iinfo.bits_per_sample ==	8 )
	    /* 44100/560 = 78.75 intrs/sec, or about 13	millisecs/intr */
	    iinfo.position_resolution =	13;
	  else
	    /* 44100*2/560 = 157.5 intrs/sec, or about 6 millisecs/intr	*/
	    iinfo.position_resolution =	6;
	break;
    }
  }

  /* NEED TO DISTINGUISH BETWEEN THE NEW AND OLD ADAPTERS */
  iinfo.device_id = MACPA;
  iinfo.slot_number = aptr->dds.slot_number;
  /* Remember the requested values for later use. */
  tptr->rinfo.srate = iinfo.srate;
  tptr->rinfo.bits_per_sample =	iinfo.bits_per_sample;
  tptr->rinfo.bsize = iinfo.bsize;
  tptr->rinfo.position_resolution = iinfo.position_resolution;
  tptr->rinfo.mode = iinfo.mode;
  tptr->rinfo.channels = iinfo.channels;
  tptr->rinfo.flags = iinfo.flags;
  tptr->rinfo.loadpath[0] = 0;
  tptr->rinfo.operation	= iinfo.operation;
  tptr->rinfo.slot_number = aptr->dds.slot_number;
  tptr->rinfo.device_id	= MACPA;

  /* Fill in the channel definitions with various predefined */
  /* values. Determine the size	and location of	the buffers in DSP memory */
  tptr->ring_event = EVENT_NULL; /* Nothing on the kernel */
					/* ring	yet */
  tptr->stop_event = EVENT_NULL; /* Not	stopping yet */
  tptr->cur_buf	= 0; /*	Current	buffer ptr */
  tptr->host_head = 0; /* Current host buffer ptr */
  tptr->host_tail = 0; /* Current tail */
  tptr->host_n_bufs = 0; /* N. buffers filled in kernel queue */
  tptr->host_buf_ptr = 0;
  tptr->host_buf_count = 0;
  tptr->waiting	= FALSE; /* No one waiting */
  tptr->host_cur_buf = 0;
  tptr->buf_size = tptr->rinfo.bsize >>	1;

  /* Determine the size	and location of	the buffers in DSP memory. */
  /* This buffer is adapter shared memory, and points to tracks	1 */
  /* and 2 as documented in the	DSP spec on p. 10. */
  switch( iinfo.srate )
  {
    case 5500 :
      for (i=0; i<tptr->modulo_factor; i++) {
	tptr->buf_loc[i] = acpa_voicebufbases[mpxchannel] +
	  i * tptr->buf_size;
      }
      break;
    case 11025 :
      if ( iinfo.mode == ADPCM )
	if ( iinfo.channels == 2 )
	  for (i=0; i<tptr->modulo_factor; i++) {
	    tptr->buf_loc[i] = acpa_stereobufbases[mpxchannel] +
	      i	* tptr->buf_size;
	  }
	else
	  for (i=0; i<tptr->modulo_factor; i++) {
	    tptr->buf_loc[i] = acpa_musicbufbases[mpxchannel] +
	      i	* tptr->buf_size;
	  }
      else	/* this	is PCM */
	if ( aptr->doing_what == Recording )
	  /* Recording always uses 16 buffers */
	  for (i=0; i<tptr->modulo_factor; i++) {
	    tptr->buf_loc[i] = acpa_16_pcmbases[mpxchannel] +
	      i	* tptr->buf_size;
	  }
	else	/* Playback */
	  /* Mono playback uses	8 buffers; stereo playback uses	16. */
	  if ( iinfo.channels == 1 )
	    for (i=0; i<tptr->modulo_factor; i++) {
	      tptr->buf_loc[i] = acpa_8_pcmbases[mpxchannel] +
		i * tptr->buf_size;
	    }
	  else
	    for (i=0; i<tptr->modulo_factor; i++) {
	      tptr->buf_loc[i] = acpa_16_pcmbases[mpxchannel] +
		i * tptr->buf_size;
	    }
      break;
    case 22050 :
      if ( iinfo.mode == ADPCM )
        for (i=0; i<tptr->modulo_factor; i++) {
	  tptr->buf_loc[i] = acpa_musicbufbases[mpxchannel] +
	    i * tptr->buf_size;
        }
      else	/* this	is PCM */
      {
        if ( aptr->doing_what == Recording )
	  /* Recording always uses 16 buffers */
	  for (i=0; i<tptr->modulo_factor; i++) {
	    tptr->buf_loc[i] = acpa_16_pcmbases[mpxchannel] +
	      i * tptr->buf_size;
          }
        else	/* Playback */
	  /* Mono playback uses 8 buffers; stereo playback uses 16. */
	  if ( iinfo.channels == 1 )
	    for (i=0; i<tptr->modulo_factor; i++) {
	      tptr->buf_loc[i] = acpa_8_pcmbases[mpxchannel] +
	        i * tptr->buf_size;
	    }
	  else
	    for (i=0; i<tptr->modulo_factor; i++) {
	      tptr->buf_loc[i] = acpa_16_pcmbases[mpxchannel] +
	        i * tptr->buf_size;
	    }
      }
      break;
    case 8000 :
    case 44100 :
      if ( aptr->doing_what == Recording )
	/* Recording always uses 16 buffers */
	for (i=0; i<tptr->modulo_factor; i++) {
	  tptr->buf_loc[i] = acpa_16_pcmbases[mpxchannel] +
	    i *	tptr->buf_size;
	}
      else	/* Playback */
	/* Mono	playback uses 8	buffers; stereo	playback uses 16. */
	if ( iinfo.channels == 1 )
	  for (i=0; i<tptr->modulo_factor; i++) {
	    tptr->buf_loc[i] = acpa_8_pcmbases[mpxchannel] +
	      i	* tptr->buf_size;
	  }
	else
	  for (i=0; i<tptr->modulo_factor; i++) {
	    tptr->buf_loc[i] = acpa_16_pcmbases[mpxchannel] +
	      i	* tptr->buf_size;
	  }
      break;
  }

  /* Determine how many	buffers	are needed.  This varies depending on */
  /* the speed and mode; we want to have enough	buffers	for the	number */
  /* of	seconds	specified in the ODM attributes.  There	are 10 interrupts */
  /* per second	for ADPCM; the number of interrrupts per second	for PCM	*/
  /* depends on	the sample width and number of channels. */
  oldrnkb = tptr->rnkb;		/* Save	the previous number of buffers.	*/
  oldwnkb = tptr->wnkb;		/* Save	the previous number of buffers.	*/
  switch( iinfo.srate )
  {
    case 5500  :
      tptr->rnkb = aptr->dds.MACPA_5r_secs * 10;
      tptr->wnkb = aptr->dds.MACPA_5w_secs * 10;
      break;
    case 8000  :
      if ( iinfo.channels == 2 )
	if ( iinfo.bits_per_sample == 16 )
	{
	  tptr->rnkb = aptr->dds.MACPA_8r_secs * 57;
	  tptr->wnkb = aptr->dds.MACPA_8w_secs * 57;
	}
	else
	{
	  tptr->rnkb = aptr->dds.MACPA_8r_secs * 29;
	  tptr->wnkb = aptr->dds.MACPA_8w_secs * 29;
	}
      else
	if ( iinfo.bits_per_sample == 16 )
	{
	  tptr->rnkb = aptr->dds.MACPA_8r_secs * 29;
	  tptr->wnkb = aptr->dds.MACPA_8w_secs * 29;
	}
	else
	{
	  tptr->rnkb = aptr->dds.MACPA_8r_secs * 14;
	  tptr->wnkb = aptr->dds.MACPA_8w_secs * 14;
	}
      break;
    case 11025 :
      if ( iinfo.mode == ADPCM )
      {
	tptr->rnkb = aptr->dds.MACPA_11r_secs *	10;
	tptr->wnkb = aptr->dds.MACPA_11w_secs *	10;
      }
      else
      {
	if ( iinfo.channels == 2 )
	  if ( iinfo.bits_per_sample ==	16 )
	  {
	    tptr->rnkb = aptr->dds.MACPA_11r_secs * 79;
	    tptr->wnkb = aptr->dds.MACPA_11w_secs * 79;
	  }
	  else
	  {
	    tptr->rnkb = aptr->dds.MACPA_11r_secs * 39;
	    tptr->wnkb = aptr->dds.MACPA_11w_secs * 39;
	  }
	else
	  if ( iinfo.bits_per_sample ==	16 )
	  {
	    tptr->rnkb = aptr->dds.MACPA_11r_secs * 39;
	    tptr->wnkb = aptr->dds.MACPA_11w_secs * 39;
	  }
	  else
	  {
	    tptr->rnkb = aptr->dds.MACPA_11r_secs * 20;
	    tptr->wnkb = aptr->dds.MACPA_11w_secs * 20;
	  }
      }
      break;
    case 22050 :
      if ( iinfo.mode == ADPCM )
      {
	tptr->rnkb = aptr->dds.MACPA_22r_secs *	20;
	tptr->wnkb = aptr->dds.MACPA_22w_secs *	20;
      }
      else
      {
        if ( iinfo.channels == 2 )
	  if ( iinfo.bits_per_sample == 16 )
	  {
	    tptr->rnkb = aptr->dds.MACPA_22r_secs * 158;
	    tptr->wnkb = aptr->dds.MACPA_22w_secs * 158;
	  }
	  else
	  {
	    tptr->rnkb = aptr->dds.MACPA_22r_secs * 79;
	    tptr->wnkb = aptr->dds.MACPA_22w_secs * 79;
	  }
        else
	  if ( iinfo.bits_per_sample == 16 )
	  {
	    tptr->rnkb = aptr->dds.MACPA_22r_secs * 79;
	    tptr->wnkb = aptr->dds.MACPA_22w_secs * 79;
	  }
	  else
	  {
	    tptr->rnkb = aptr->dds.MACPA_22r_secs * 39;
	    tptr->wnkb = aptr->dds.MACPA_22w_secs * 39;
	  }
      }
      break;
    case 44100 :
      if ( iinfo.channels == 2 )
	if ( iinfo.bits_per_sample == 16 )
	{
	  tptr->rnkb = aptr->dds.MACPA_44r_secs	* 315;
	  tptr->wnkb = aptr->dds.MACPA_44w_secs	* 315;
	}
	else
	{
	  tptr->rnkb = aptr->dds.MACPA_44r_secs	* 158;
	  tptr->wnkb = aptr->dds.MACPA_44w_secs	* 158;
	}
      else
	if ( iinfo.bits_per_sample == 16 )
	{
	  tptr->rnkb = aptr->dds.MACPA_44r_secs	* 158;
	  tptr->wnkb = aptr->dds.MACPA_44w_secs	* 158;
	}
	else
	{
	  tptr->rnkb = aptr->dds.MACPA_44r_secs	* 79;
	  tptr->wnkb = aptr->dds.MACPA_44w_secs	* 79;
	}
      break;
    default :
      /* Any other values are not supported. */
      iinfo.rc = INVALID_REQUEST;
      /* Need to return	the error code to the user. */
      if ( ( rc	= copyout( (char *) &(iinfo.rc), (char *) &(init->rc),
	     sizeof(init->rc) )	) != 0 )
	return(	EFAULT );
      else
	return(	EINVAL );
      break;
  }

  /* Copy the properly initialized information to the user's buffer. */
  if ( ( rc = copyout( (char *)	&iinfo,	(char *) init,
	 sizeof(audio_init) ) )	!= 0 )
  {
    TRACE("Failed copying info to user's init buffer\n");
    return( EFAULT );
  }

  /* Delete any	preallocated read queue	buffers. */
  for (	i=0; i<oldrnkb;	i++ )
  {
    /* These checks for	NULL should be redundant, but are kept here */
    /* for any unusual states (app killed, etc.). */
    if ( tptr->rbuf[i] != NULL )
    {
      (void) FREE_MEMORY( tptr->rbuf[i]	);
      tptr->rbuf[i] = NULL;
    }
  }
  /* Delete any	preallocated write queue buffers. */
  for (	i=0; i<oldwnkb;	i++ )
  {
    /* These checks for	NULL should be redundant, but are kept here */
    /* for any unusual states (app killed, etc.). */
    if ( tptr->wbuf[i] != NULL )
    {
      (void) FREE_MEMORY( tptr->wbuf[i]	);
      tptr->wbuf[i] = NULL;
    }
  }

  /* Now reallocate the	read buffers according to the current size. */
  for (	i=0; i<tptr->rnkb; i++ )
  {
    tptr->rbuf[i] = GET_MEMORY(	iinfo.bsize );
    if ( tptr->rbuf[i] == NULL )
    {
      /* Memory	allocation failed, so relinquish all memory and	return.	*/
      for ( j=0; j<MAXNKB; j++ )
      {
	if ( tptr->rbuf[j] != NULL )
	{
	  (void) FREE_MEMORY( tptr->rbuf[j] );
	  tptr->rbuf[j]	= NULL;
	}
      }
      return( ENOMEM );
    }
  }
  /* Now reallocate the	write buffers according	to the current size. */
  for (	i=0; i<tptr->wnkb; i++ )
  {
    tptr->wbuf[i] = GET_MEMORY(	iinfo.bsize );
    if ( tptr->wbuf[i] == NULL )
    {
      /* Memory	allocation failed, so relinquish all memory and	return.	*/
      for ( j=0; j<MAXNKB; j++ )
      {
	if ( tptr->wbuf[j] != NULL )
	{
	  (void) FREE_MEMORY( tptr->wbuf[j] );
	  tptr->wbuf[j]	= NULL;
	}
      }
      return( ENOMEM );
    }
  }

  if ( aptr->secondtime	== FALSE )
    /* Reset the interrupt counter to 0	(if this is the	first open). */
    aptr->int_count = 0;

  /* initialize	audio_buffer fields */
  init_audio_buffer( tptr, (unsigned long) 0, min );

  /* initialize	the request buffer fields */
  tptr->request_list = NULL;
  tptr->num_requests = 0;		/* there are no	requests */

  EXIT_ACPA_PTRACE( INIT, ACPA_MAIN, min );
#ifdef ACPA_MEMORY_TRACE
#endif

  return( 0 );

}

/************************************************************************/
/*									*/
/*  FUNCTION NAME: acpa_change_request					*/
/*									*/
/*  DESCRIPTION:							*/
/*									*/
/*	This function implements the AUDIO_CHANGE request for the	*/
/*	AUDIO_CONTROL ioctl.						*/
/*									*/
/*  INVOCATION:								*/
/*									*/
/*	rc = acpa_change_request( audio_change *uchange, int min,	*/
/*			int mpxchan, int type, int request_pid )	*/
/*									*/
/*  INPUT PARAMETERS:							*/
/*									*/
/*	uchange	is a pointer to	the AUDIO_CHANGE request specifics.	*/
/*	min is the minor number	for the	adapter	being used.		*/
/*	mpxchan	is the channel id for the adapter being	used.		*/
/*	type specifies whether ther request was	enqueued or is an	*/
/*		immediate request.					*/
/*	request_pid is the process id of the process that enqueued	*/
/*		a request for later execution.				*/
/*									*/
/*  OUTPUT PARAMETERS:							*/
/*									*/
/*	N/A								*/
/*									*/
/*  RETURN VALUES:							*/
/*									*/
/*	0 if successful							*/
/*	-1 if unsuccessful.  In	this case errno	will also be set.	*/
/*									*/
/************************************************************************/

/* This	routine	satisfies the AUDIO_CHANGE request */
int acpa_change_request( audio_change *uchange,	int min, int mpxchan,
  int type, int	request_pid )
{
  register int mpxchannel;	/* the transformed mpx channel number */
  int ctlflag;			/* flags whether this comes from ctl file */
  int rc;
  audio_change change;		/* user-specified change information */
  track_info tinfo;		/* track information */
  int pid;			/* process id of caller	*/
  register struct acpa_status *aptr; /*	ptr to current adapter's status	*/
  register Track *tptr;		/* pointer to current track info */
  int trkctl;			/* Base	addr of	track control block */
  int trkbox;			/* Track mailbox control block */
  audio_change *cptr;		/* pointer to the change data */
  track_info *trackptr;		/* pointer to dev dependent track info */
  struct audio_request *rptr;	/* pointer to enqueued request info */
  int pcmbase;
  int i, j;
  int oldspl;                   /* Used to hold interrupt state */
  extern void acpa_init_registers(); /*	sets all track registers to 0 */
  extern void acpa_init_pcm_regs();

  ENTER_ACPA_PTRACE( CHANGE, ACPA_MAIN,	min );

  mpxchannel = ( mpxchan & 3 ) - 1;	/* this	is used	as an index */
  ctlflag = mpxchan & ACPA_CTL;		/* is this a ctl request? */
  aptr = (struct acpa_status *)	&( acpa[min] );
  tptr = (Track	*) &( acpa[min].track[mpxchannel] );

  if ( type == NOT_ENQUEUED )
  {
    /* The data	needs to be copied from	user space into	kernel space. */
    if ( ( rc =	copyin(	(char *) uchange, (char	*) &change,
	   sizeof(audio_change)	) ) != 0 )
      return( EFAULT );
    cptr = &change;
  }
  else
    /* The data	has already been copied	into kernel space. */
    cptr = uchange;

  /* Request queues can	be used	only if	in playback mode. */
  if ( ( type == ENQUEUED ) && ( aptr->doing_what == Recording ) )
    return( EINVAL );

  pid =	getpid();
  if ( ( type == ENQUEUED ) && ( request_pid ==	tptr->pid ) )
  {
    /* The enqueued request has	the correct process id,	so continue. */
#ifdef ACPA_MEMORY_TRACE
#endif
  }
  else
    if ( pid !=	tptr->pid )
    {
      /* Other processes can change track specifications only, and they	*/
      /* must do it via	a ctl file. */
      if ( ctlflag )
      {
	/* Only	track characteristics can be changed. */
	if ( ( cptr->input != AUDIO_IGNORE ) ||	( cptr->output != AUDIO_IGNORE ) ||
	     ( cptr->monitor !=	AUDIO_IGNORE ) )
	  return( EINVAL );
	/* If the device is not	in use,	no changes can be made.	*/
	if ( tptr->in_use == FALSE )
	  return( ENOTREADY );
      }
      else
	return(	EINVAL );
    }

  /* Validate the input	specification (if any).	*/
  switch( cptr->input )
  {
    case LOW_GAIN_MIKE :
    case HIGH_GAIN_MIKE	:
      /* IS THIS RESTRICTION NECESSARY?	*/
      /* can't specify microphone if in	playback mode */
      if ( aptr->doing_what == Playing )
        if ( tptr->rinfo.mode != SOURCE_MIX )
	  return( EINVAL );
      break;
    case LINE_1	:
    case LINE_2	:
      /* On source mix, only high/low gain mikes and lines 1 and 2 can be */
      /* specified. */
      if ( tptr->rinfo.mode == SOURCE_MIX )
        return( EINVAL );
      break;
    case LINES_1AND2 :
      break;
    case ALL_LINES :
      /* The adapter doesn't understand	all lines; it means both lines.	*/
      cptr->input = LINES_1AND2;
      break;
    case AUDIO_IGNORE :
      break;
    default :
      return( EINVAL );
      break;
  }

  /* Validate the monitor specification	(if any). */
  if ( cptr->monitor !=	AUDIO_IGNORE )
  {
    if ( aptr->doing_what == Playing )
      return( EINVAL );
    if ( tptr->rinfo.mode == SOURCE_MIX )
      return( EINVAL );
    switch( cptr->monitor )
    {
      case MONITOR_OFF :
      case MONITOR_UNCOMPRESSED	:
	break;
      case MONITOR_COMPRESSED :
	if ( ( tptr->rinfo.mode	== PCM ) || ( tptr->rinfo.mode == MU_LAW ) ||
	     ( tptr->rinfo.mode	== A_LAW ) )
	  return( EINVAL );
	else
	  if ( ( tptr->rinfo.srate == 11025 ) &&
		  ( tptr->rinfo.channels == 2 )	)
	    return( EINVAL );
	break;
      default :
	return(	EINVAL );
    }
  }

  /* Verify the	output specification, if any.  Note that any output */
  /* specification is never applied, since the value is set in the  */
  /* config init code, and can't be changed.  Also note that if other */
  /* values are valid in the future, the code that checks for ctlflag ==*/
  /* TRUE must change, because it currently assumes that the current */
  /* mode is record. */
  if ( cptr->output != AUDIO_IGNORE )
    if ( aptr->doing_what == Recording )
      /* Can't change the output if recording is in progress. */
      return( EINVAL );
    else
      switch( cptr->output )
      {
	case OUTPUT_1 :
	  break;
	default	:
	  return( EINVAL );
      }

  ctlflag = FALSE;	/* Use flag to determine whether to stop card */
  /* Store any values that are valid. */
  if ( cptr->monitor !=	AUDIO_IGNORE )
  {
    ctlflag = TRUE;
    if ( tptr->current_state > Stopped )
      return( EINVAL );
    else
      tptr->ainfo.change.monitor = cptr->monitor;
  }
  if ( cptr->input != AUDIO_IGNORE )
  {
    ctlflag = TRUE;
    if ( tptr->rinfo.mode == SOURCE_MIX )
    {
      if ( tptr->current_state > Stopped )
        return( EINVAL );
    }
    else 
      if ( ( tptr->current_state > Stopped ) || ( aptr->doing_what ==
	   Playing ) )
        return( EINVAL );
    tptr->ainfo.change.input = cptr->input;
  }

  /* Now save any changes to volume and	balance. */
  if ( cptr->balance !=	AUDIO_IGNORE )
    tptr->ainfo.change.balance = cptr->balance;
  if ( cptr->balance_delay != AUDIO_IGNORE )
    tptr->ainfo.change.balance_delay = cptr->balance_delay;
  if ( cptr->volume != AUDIO_IGNORE )
    tptr->ainfo.change.volume =	cptr->volume;
  if ( cptr->volume_delay != AUDIO_IGNORE )
    tptr->ainfo.change.volume_delay = cptr->volume_delay;

  /* Deal with the track specification if there	is any.	*/
  if ( cptr->dev_info != NULL )
  {
    if ( type == NOT_ENQUEUED )
    {
      /* Copy the track	specification into kernel space. */
      if ( ( rc	= copyin( (char	*) cptr->dev_info, (char *) &tinfo,
	     sizeof(track_info)	) ) != 0 )
	return(	EFAULT );
      trackptr = &tinfo;
    }
    else
      /* The data has already been copied into kernel space. */
      trackptr = (track_info *)	cptr->dev_info;
    /* SHOULD THERE BE VALIDATION OF THE VALUES? */
    /* Store the specified values into the status information. */
    if ( trackptr->master_volume != (unsigned short) AUDIO_IGNORE )
      tptr->tinfo.master_volume	= trackptr->master_volume;
    if ( trackptr->dither_percent != (unsigned short) AUDIO_IGNORE )
      if ( trackptr->dither_percent > 100 )
	return( EINVAL );
      else
	tptr->tinfo.dither_percent = trackptr->dither_percent;
  }
  else
    trackptr = NULL;

  /* Find the track control block */
  if ( mpxchannel == 0 ) {
    trkctl = DSP_T1_CONTROL_BLOCK_BASE;
    trkbox = DSP_T1_MAILBOX_BASE;
    pcmbase = PCM_T1_CONTROL_PARMS;
  }
  else {
    trkctl = DSP_T2_CONTROL_BLOCK_BASE;
    trkbox = DSP_T2_MAILBOX_BASE;
    pcmbase = PCM_T2_CONTROL_PARMS;
  }

  if ( ( ctlflag == TRUE ) && (	tptr->current_state >= Stopped ) )
  {
    oldspl = DISABLE_INTERRUPTS();           /* Disable interrupts */
    /* Stop the adapter before changing input and monitor settings. */
    acpaput( trkbox + DSP_TX_CMD_FROM_HOST, (unsigned short) 0,	min );

    /* Now make sure that the microcode has stopped before resetting regs. */
    acpaput( trkbox + DSP_TX_TMS_STATUS,
	     (unsigned short) 0x0F0F, min );
    /* Now wait for the register to get cleared. */
    for ( i=0; i<0xffffffff; i++ )
    {
      rc = (int) acpaget( trkbox + DSP_TX_TMS_STATUS, min );
      if ( !rc )
        break;
    }
    (void) ENABLE_INTERRUPTS(oldspl);       /* Re-enable interrupts */
    /* Wait fifteen more milliseconds, and then everything is OK. */
    acpa_bus_delay( 15000 );

    /* Set the track registers to 0. */
    oldspl = DISABLE_INTERRUPTS();           /* Disable interrupts */
    acpa_init_registers( trkbox, min );
    tptr->host_buf_count = 0;
    tptr->preload_count = 0;
    tptr->host_buf_ptr = 0;

    /* Set audio type; this is necessary only for ADPCM sampling rates. */
    if ( tptr->rinfo.mode == ADPCM )
      switch( tptr->rinfo.srate )
      {
        case 5500 :
	  acpaput(trkctl + DSP_TX_AUDIO_TYPE,
	    (unsigned short) TYPE_VOICE, min );
          break;
        case 11025 :
	  if ( tptr->rinfo.channels == 2 )
	    acpaput(trkctl + DSP_TX_AUDIO_TYPE,
	      (unsigned short) TYPE_STEREO_MUSIC, min );
	  else
	    acpaput(trkctl + DSP_TX_AUDIO_TYPE,
	      (unsigned short) TYPE_MUSIC, min );
	  break;
        case 22050 :
	  acpaput(trkctl + DSP_TX_AUDIO_TYPE,
	    (unsigned short) TYPE_MUSIC, min );
          break;
        default :
	  return( EINVAL );
      }
    else
    {
      acpa_init_pcm_regs( pcmbase, min );
      /* Perform the initialization necessary for PCM sampling rates. */
      switch( tptr->rinfo.mode )
      {
	case SOURCE_MIX :
	case PCM :
	  acpaput( pcmbase + PCM_TYPE, (unsigned short) 0, min );
	  break;
        case MU_LAW :
	  acpaput( pcmbase + PCM_TYPE, (unsigned short) 2, min );
	  break;
        case A_LAW :
	  acpaput( pcmbase + PCM_TYPE, (unsigned short) 3, min );
	  break;
      }
      acpaput( pcmbase + PCM_SAMPLE_RATE,
	       (unsigned short) tptr->rinfo.srate, min );
      acpaput( pcmbase + PCM_DITHER_LEVEL,
	       (unsigned short) tptr->tinfo.dither_percent, min );
      acpaput( pcmbase + PCM_SAMPLE_WIDTH,
	       (unsigned short) tptr->rinfo.bits_per_sample, min );
      if ( tptr->rinfo.flags & BIG_ENDIAN	)
        acpaput( pcmbase + PCM_BYTE_SWAPPING, (unsigned short) 1, min );
      else
        acpaput( pcmbase + PCM_BYTE_SWAPPING, (unsigned short) 0, min );
      if ( tptr->rinfo.flags & TWOS_COMPLEMENT )
        acpaput( pcmbase + PCM_NUMBER_FORMAT, (unsigned short) 0, min );
      else if ( tptr->rinfo.flags & SIGNED )
        acpaput( pcmbase + PCM_NUMBER_FORMAT, (unsigned short) 1, min );
      else
        /* Default number format is unsigned. */
        acpaput( pcmbase + PCM_NUMBER_FORMAT, (unsigned short) 2, min );
      acpaput( pcmbase + PCM_NUMBER_CHANNELS,
	       (unsigned short) tptr->rinfo.channels, min );
      if ( tptr->rinfo.mode == SOURCE_MIX )
	acpaput( pcmbase + PCM_SOURCE_MIX, (unsigned short) 1, min );
      else
	acpaput( pcmbase + PCM_SOURCE_MIX, (unsigned short) 0, min );
    }

    /* Initialize the Input Select field */
    switch( tptr->ainfo.change.input)
    {
      case LINE_1 :
	acpaput(DSP_INPUT_SELECT, INPUT_FROM_LINE_LEFT,	min );
	break;
      case LINE_2 :
	acpaput(DSP_INPUT_SELECT, INPUT_FROM_LINE_RIGHT, min );
	break;
      case ALL_LINES :
      case LINES_1AND2 :
	acpaput(DSP_INPUT_SELECT, INPUT_FROM_LINE_BOTH,	min );
	break;
      case HIGH_GAIN_MIKE :
	acpaput(DSP_INPUT_SELECT, INPUT_FROM_HIGAIN_MIKE, min );
	break;
      case LOW_GAIN_MIKE :
	acpaput(DSP_INPUT_SELECT, INPUT_FROM_LOGAIN_MIKE, min );
	break;
      default:
	return(	EINVAL );
	break;
    }

    if ( tptr->rinfo.mode == ADPCM )
      acpaput(trkctl + DSP_TX_MASTER_VOLUME,
	(unsigned short) tptr->tinfo.master_volume, min );
    else
      /* Full volume is 0x7fff/100, which equals 327.	*/
      acpaput(trkctl + DSP_TX_MASTER_VOLUME,
	(unsigned short) ( tptr->tinfo.master_volume / 327 ),	min );

    acpaput(trkctl + DSP_TX_TRACK_VOLUME,
      (unsigned short) ( tptr->ainfo.change.volume >> 16 ), min );
    acpaput(trkctl + DSP_TX_TRACK_VOL_RATE,
      (unsigned short) ( tptr->ainfo.change.volume_delay >> 16 ), min );
    acpaput(trkctl + DSP_TX_TRACK_BALANCE,
      (unsigned short) ( tptr->ainfo.change.balance >> 16 ), min );
    acpaput(trkctl + DSP_TX_TRACK_BAL_RATE,
      (unsigned short) ( tptr->ainfo.change.balance_delay >> 16 ), min);
  
    if ( aptr->doing_what == Recording )
    {
      /* Set the proper monitor mode. */
      acpaput( DSP_MONITOR_SELECT, tptr->ainfo.change.monitor, min );
  
      /* This sequence of commands comes from the DSP spec, p. 4. */
      /* Zero out the track buffer counter */
      acpaput(trkbox + DSP_TX_BUFFER_CNT, (unsigned short) 0, min );
      /* Zero out the host track buffer pointer */
      acpaput(trkbox + DSP_TX_HOST_BUF_PTR, (unsigned short) 0, min );
      /* Start the adapter. */
      acpaput( trkbox + DSP_TX_CMD_FROM_HOST, (unsigned short) 1, min );
      (void) ENABLE_INTERRUPTS(oldspl);       /* Re-enable interrupts */
    }
    else /* The adapter is in playback mode. */
    {
      (void) ENABLE_INTERRUPTS(oldspl);       /* Re-enable interrupts */
      /* Zero out the host-to-TMS sample buffers. */
      for (i = 0; i < tptr->modulo_factor; i++)
      {
        oldspl = DISABLE_INTERRUPTS();           /* Disable interrupts */
  
        /* This loop preloads 0 into all of the adapter's buffers.  Source */
        /* mix takes advantage of this by using format SIGNED. */
        for ( j=tptr->buf_loc[i]; j<(tptr->buf_loc[i]+tptr->buf_size); j++ )
          /* Zero out the buffer */
          acpaput(j, (unsigned short) 0, min );
  
        (void) ENABLE_INTERRUPTS(oldspl);       /* Re-enable interrupts */
      }

      if (acpa[min].running != TRUE)
      {
	acpa[min].can_interrupt_host = FALSE;    /* Disable DSP interrupts */
	(void) acpastart( min );                              /* Start DSP */
	(void) DELAYTICKS(2);          /* Wait a moment for the DSP to start */

	/* Enable interrupts from the DSP */
	acpa[min].can_interrupt_host = TRUE;
	(void) acpastart( min );
      }

      /* Start the playback operation. */
      oldspl = DISABLE_INTERRUPTS();          /* Disable interrupts */
      acpaput(trkbox + DSP_TX_CMD_FROM_HOST, (unsigned short) 5, min );
      (void) ENABLE_INTERRUPTS(oldspl);       /* Re-enable interrupts */
    }

    EXIT_ACPA_PTRACE( CHANGE, ACPA_MAIN, min );

    /* Since this code reinitializes all registers, it can return now. */
    return( 0 );
  }

  /* Initialize	the Track "n" Control Block */
  oldspl = DISABLE_INTERRUPTS();           /* Disable interrupts */
  if ( trackptr	!= NULL	)
  {
    if ( trackptr->master_volume != (unsigned short) AUDIO_IGNORE )
    {
      if ( tptr->rinfo.mode == ADPCM )
	acpaput(trkctl + DSP_TX_MASTER_VOLUME,
	  (unsigned short) tptr->tinfo.master_volume, min );
      else
	/* Full	volume is 0x7fff/100, which equals 327.	*/
	acpaput(trkctl + DSP_TX_MASTER_VOLUME,
	  (unsigned short) ( tptr->tinfo.master_volume / 327 ),	min );
    }
    if ( trackptr->dither_percent != (unsigned short) AUDIO_IGNORE )
      acpaput( pcmbase + PCM_DITHER_LEVEL,
	       (unsigned short) tptr->tinfo.dither_percent, min );
  }
  if ( cptr->volume != AUDIO_IGNORE )
    acpaput(trkctl + DSP_TX_TRACK_VOLUME,
      (unsigned	short) ( tptr->ainfo.change.volume >> 16 ), min	);
  if ( cptr->volume_delay != AUDIO_IGNORE )
    acpaput(trkctl + DSP_TX_TRACK_VOL_RATE,
      (unsigned	short) ( tptr->ainfo.change.volume_delay >> 16 ), min );
  if ( cptr->balance !=	AUDIO_IGNORE )
    acpaput(trkctl + DSP_TX_TRACK_BALANCE,
      (unsigned	short) ( tptr->ainfo.change.balance >> 16 ), min );
  if ( cptr->balance_delay != AUDIO_IGNORE )
    acpaput(trkctl + DSP_TX_TRACK_BAL_RATE,
      (unsigned	short) ( tptr->ainfo.change.balance_delay >> 16	), min);
  (void) ENABLE_INTERRUPTS(oldspl);       /* Re-enable interrupts */

  EXIT_ACPA_PTRACE( CHANGE, ACPA_MAIN, min );

  return( 0 );
}

/************************************************************************/
/*									*/
/*  FUNCTION NAME: acpa_start						*/
/*									*/
/*  DESCRIPTION:							*/
/*									*/
/*	This function initializes the adapter and starts interrupts.	*/
/*	It implements the AUDIO_START request.				*/
/*									*/
/*  INVOCATION:								*/
/*									*/
/*	rc = acpa_start( int mpxchan, int min )				*/
/*									*/
/*  INPUT PARAMETERS:							*/
/*									*/
/*	mpxchan	is the channel id for this adapter.			*/
/*	min is the minor device	number for this	adapter.		*/
/*									*/
/*  OUTPUT PARAMETERS:							*/
/*									*/
/*	N/A								*/
/*									*/
/*  RETURN VALUES:							*/
/*									*/
/*	0 if successful							*/
/*	-1 if unsuccessful.  In	this case errno	will also be set.	*/
/*									*/
/************************************************************************/

int acpa_start(	int mpxchan, int min, unsigned long position )
{
  int mpxchannel;		/* the channel id for current adapter */
  register struct acpa_status *aptr; /*	ptr to current adapter's status	*/
  register Track *tptr;		/* pointer to current track info */
  int trkctl;			/* Base	addr of	track control block */
  int trkbox;			/* Track mailbox control block */
  int pcmbase;			/* PCM track mailbox control base */
  int i, j;
  int oldspl;                   /* Holds interrupt state */
  extern void init_audio_buffer(); /* initializes audio_buffer fields */
  extern void acpa_init_registers(); /*	sets all track registers to 0 */
  extern void acpa_init_pcm_regs();
#ifdef ACPA_MEMORY_TRACE
  extern void print_acpa_pcm_regs();
#endif

  ENTER_ACPA_PTRACE( DEVSTART, ACPA_MAIN, min );

  mpxchannel = ( mpxchan & 3 ) - 1;	/* this	is used	as an index */

  aptr = (struct acpa_status *)	&( acpa[min] );
  tptr = (Track	*) &( acpa[min].track[mpxchannel] );

  /* If	the adapter has	been started before, just change the flag this */
  /* time around. */
  if ( tptr->current_state == Stopped )
  {
    tptr->current_state	= Started;
    tptr->buffer.position = position;
    /* Restore the operation setting. */
    switch ( aptr->doing_what )
    {
      case Playing :
	tptr->rinfo.operation = PLAY;
	break;
      case Recording :
	tptr->rinfo.operation = RECORD;
	break;
      default :
	return( EINVAL );
    }
    oldspl = DISABLE_INTERRUPTS();           /* Disable interrupts */
    if ( mpxchannel == 0 )
      acpaput( DSP_T1_CONTROL_BLOCK_BASE + DSP_TX_TRACK_VOLUME,
	(unsigned short) ( tptr->ainfo.change.volume >>	16 ), min );
    else
      acpaput( DSP_T2_CONTROL_BLOCK_BASE + DSP_TX_TRACK_VOLUME,
	(unsigned short) ( tptr->ainfo.change.volume >>	16 ), min );
    (void) ENABLE_INTERRUPTS(oldspl);       /* Re-enable interrupts */
    return( 0 );
  }

  /* Don't allow a started track to be started again. */
  if ( tptr->current_state >= Almost_Started )
    return( EINVAL );

  /* If	we are opening a recording session, start the DSP.  It will */
  /* begin to capture data that	will be	read by	a later	read.  If */
  /* we're about to start playback, on the other hand, we shouldn't */
  /* start the DSP until at least one buffer is	filled in the DSP */
  /* memory.  For playback, just set a flag; the write routine will */
  /* start the DSP. */

  if ( aptr->secondtime	== FALSE )
  {
    aptr->closing = FALSE;		      /* Not closing the DSP yet! */

    /* If recording, start the DSP now (do not enable interrupts) */

    if (aptr->doing_what == Recording) {
      if (aptr->running	!= TRUE) {
	aptr->can_interrupt_host = FALSE;      /* Disable DSP interrupts */
	aptr->ps2_speaker_enabled = FALSE;     /* Speaker not in use */
	(void) acpastart( min );	       /* Start	the DSP	*/
      }
    }

    /* If playback, mark the device as opening;	the write routine will */
    /* start the DSP */

    else {
	if ( tptr->ainfo.change.output == INTERNAL_SPEAKER )
	  aptr->ps2_speaker_enabled = TRUE;
	else aptr->ps2_speaker_enabled = FALSE;
    }
  }

  /* Find the track control block */
  if ( mpxchannel == 0 ) {
    trkctl = DSP_T1_CONTROL_BLOCK_BASE;
    trkbox = DSP_T1_MAILBOX_BASE;
    pcmbase = PCM_T1_CONTROL_PARMS;
  }
  else {
    trkctl = DSP_T2_CONTROL_BLOCK_BASE;
    trkbox = DSP_T2_MAILBOX_BASE;
    pcmbase = PCM_T2_CONTROL_PARMS;
  }

  /* Set the track registers to	0. */
  oldspl = DISABLE_INTERRUPTS();           /* Disable interrupts */
#ifdef ACPA_MEMORY_TRACE
  /* *xxptr[min]++ = 0xfff10000 | mpxchannel;
  if ( mpxchannel == 0 )
    dbg_print_regs( 0x0e18, min, 0xaaac0000 );
  else
    dbg_print_regs( 0x0e20, min, 0xaaac0000 );  */
#endif
  acpa_init_registers( trkbox, min );
#ifdef ACPA_MEMORY_TRACE
  /* *xxptr[min]++ = 0xfff20000 | mpxchannel;
  if ( mpxchannel == 0 )
    dbg_print_regs( 0x0e18, min, 0xaaad0000 );
  else
    dbg_print_regs( 0x0e20, min, 0xaaad0000 );  */
#endif

  tptr->host_buf_count = 0;
  tptr->preload_count =	0;

  /* Set audio type; this is necessary only for	ADPCM sampling rates. */
  if ( tptr->rinfo.mode	== ADPCM )
    switch( tptr->rinfo.srate )
    {
      case 5500	:
	acpaput(trkctl + DSP_TX_AUDIO_TYPE,
	  (unsigned short) TYPE_VOICE, min );
	break;
      case 11025 :
	if ( tptr->rinfo.channels == 2 )
	  acpaput(trkctl + DSP_TX_AUDIO_TYPE,
	    (unsigned short) TYPE_STEREO_MUSIC,	min );
	else
	  acpaput(trkctl + DSP_TX_AUDIO_TYPE,
	    (unsigned short) TYPE_MUSIC, min );
	break;
      case 22050 :
	acpaput(trkctl + DSP_TX_AUDIO_TYPE,
	  (unsigned short) TYPE_MUSIC, min );
        break;
      default :
	return(	EINVAL );
    }
  else
  {
    acpa_init_pcm_regs(	pcmbase, min );
    /* Perform the initialization necessary for	PCM sampling rates. */
    switch( tptr->rinfo.mode )
    {
      case SOURCE_MIX :
      case PCM :
	acpaput( pcmbase + PCM_TYPE, (unsigned short) 0, min );
	break;
      case MU_LAW :
	acpaput( pcmbase + PCM_TYPE, (unsigned short) 2, min );
	break;
      case A_LAW :
	acpaput( pcmbase + PCM_TYPE, (unsigned short) 3, min );
	break;
    }
    acpaput( pcmbase + PCM_SAMPLE_RATE,
	     (unsigned short) tptr->rinfo.srate, min );
    acpaput( pcmbase + PCM_DITHER_LEVEL,
	     (unsigned short) tptr->tinfo.dither_percent, min );
    acpaput( pcmbase + PCM_SAMPLE_WIDTH,
	     (unsigned short) tptr->rinfo.bits_per_sample, min );
    if ( tptr->rinfo.flags & BIG_ENDIAN	)
      acpaput( pcmbase + PCM_BYTE_SWAPPING, (unsigned short) 1,	min );
    else
      acpaput( pcmbase + PCM_BYTE_SWAPPING, (unsigned short) 0,	min );
    if ( tptr->rinfo.flags & TWOS_COMPLEMENT )
      acpaput( pcmbase + PCM_NUMBER_FORMAT, (unsigned short) 0,	min );
    else if ( tptr->rinfo.flags	& SIGNED )
      acpaput( pcmbase + PCM_NUMBER_FORMAT, (unsigned short) 1,	min );
    else
      /* Default number	format is unsigned. */
      acpaput( pcmbase + PCM_NUMBER_FORMAT, (unsigned short) 2,	min );
    acpaput( pcmbase + PCM_NUMBER_CHANNELS,
	     (unsigned short) tptr->rinfo.channels, min	);
    if ( tptr->rinfo.mode == SOURCE_MIX )
      acpaput( pcmbase + PCM_SOURCE_MIX, (unsigned short) 1, min );
    else
      acpaput( pcmbase + PCM_SOURCE_MIX, (unsigned short) 0, min );
  }
#ifdef ACPA_MEMORY_TRACE
  /*print_acpa_pcm_regs( pcmbase, min );*/
#endif

  /* Initialize	the Track "n" Control Block */
  if ( tptr->rinfo.mode	== ADPCM )
    acpaput(trkctl + DSP_TX_MASTER_VOLUME,
      (unsigned	short) tptr->tinfo.master_volume, min );
  else
    /* For PCM,	100 is the full	volume.	 0x7fff/104 = 327, hence the */
    /* following calculation. */
    acpaput(trkctl + DSP_TX_MASTER_VOLUME,
      (unsigned	short) ( tptr->tinfo.master_volume / 327 ), min	);
  acpaput(trkctl + DSP_TX_TRACK_VOLUME,
    (unsigned short) ( tptr->ainfo.change.volume >> 16 ), min );
  acpaput(trkctl + DSP_TX_TRACK_VOL_RATE,
    (unsigned short) ( tptr->ainfo.change.volume_delay >> 16 ),	min );
  acpaput(trkctl + DSP_TX_TRACK_BALANCE,
    (unsigned short) ( tptr->ainfo.change.balance >> 16	), min );
  acpaput(trkctl + DSP_TX_TRACK_BAL_RATE,
    (unsigned short) ( tptr->ainfo.change.balance_delay	>> 16 ), min);
  (void) ENABLE_INTERRUPTS(oldspl);       /* Re-enable interrupts */

  /* Initialize	the audio_buffer fields	to starting values. */
  init_audio_buffer( tptr, position, min );

  /* Do	things which are different if we're playing or recording */

  switch(aptr->doing_what) {

  /* Initialize	the DSP	control	blocks for playback */

  case Playing:			/* Playback */

    /* Zero out the host-to-TMS sample buffers. */
    for (i = 0; i < tptr->modulo_factor; i++)
    {
      oldspl = DISABLE_INTERRUPTS();           /* Disable interrupts */

      /* This loop preloads 0 into all of the adapter's buffers.  Source */
      /* mix takes advantage of this by using format SIGNED. */
      for ( j=tptr->buf_loc[i]; j<(tptr->buf_loc[i]+tptr->buf_size); j++ )
        /* Zero out the buffer */
        acpaput(j, (unsigned short) 0, min );

      (void) ENABLE_INTERRUPTS(oldspl);       /* Re-enable interrupts */
    }

    if ( tptr->rinfo.mode != SOURCE_MIX )
      tptr->current_state = Almost_Started;
    else
    {
      oldspl = DISABLE_INTERRUPTS();           /* Disable interrupts */
      /* Source mix: Initialize the Input Select field */
      switch( tptr->ainfo.change.input)
      {
        case LINE_1 :
	  acpaput(DSP_INPUT_SELECT, INPUT_FROM_LINE_LEFT, min );
	  break;
        case LINE_2 :
	  acpaput(DSP_INPUT_SELECT, INPUT_FROM_LINE_RIGHT, min );
	  break;
        case ALL_LINES :
        case LINES_1AND2 :
	  acpaput(DSP_INPUT_SELECT, INPUT_FROM_LINE_BOTH, min );
	  break;
        case HIGH_GAIN_MIKE :
	  acpaput(DSP_INPUT_SELECT, INPUT_FROM_HIGAIN_MIKE, min );
	  break;
        case LOW_GAIN_MIKE :
	  acpaput(DSP_INPUT_SELECT, INPUT_FROM_LOGAIN_MIKE, min );
	  break;
        default:
	  return( EINVAL );
	  break;
      }
      (void) ENABLE_INTERRUPTS(oldspl);       /* Re-enable interrupts */

      /* The adapter is now ready to be started.  Since write() can't */
      /* be called in source mix mode, this code must start the adapter. */
      tptr->current_state = Started;
      if (acpa[min].running != TRUE)
      {
	acpa[min].can_interrupt_host = FALSE;    /* Disable DSP interrupts */
	(void) acpastart( min );                              /* Start DSP */
	(void) DELAYTICKS(2);          /* Wait a moment for the DSP to start */

	/* Enable interrupts from the DSP */
	acpa[min].can_interrupt_host = TRUE;
	(void) acpastart( min );
      }

      /* Start the operation. */
      oldspl = DISABLE_INTERRUPTS();          /* Disable interrupts */
      acpaput(trkbox + DSP_TX_CMD_FROM_HOST, (unsigned short) 5, min );
      (void) ENABLE_INTERRUPTS(oldspl);       /* Re-enable interrupts */
    }

    tptr->rinfo.operation = PLAY;

    break;

  /* Initialize	the DSP	control	blocks for recording */

  case Recording:		/* Recording */

    /* Enable interrupts from the DSP */

    aptr->can_interrupt_host = TRUE; /*	Enable DSP interrupts */
    (void) acpastart( min );	     /*	Enable DSP interrupts */
    tptr->current_state	= Started;

    /* Recording: Initialize the Input Select field */
    switch( tptr->ainfo.change.input)
    {
      case LINE_1 :
	acpaput(DSP_INPUT_SELECT, INPUT_FROM_LINE_LEFT,	min );
	break;
      case LINE_2 :
	acpaput(DSP_INPUT_SELECT, INPUT_FROM_LINE_RIGHT, min );
	break;
      case ALL_LINES :
      case LINES_1AND2 :
	acpaput(DSP_INPUT_SELECT, INPUT_FROM_LINE_BOTH,	min );
	break;
      case HIGH_GAIN_MIKE :
	acpaput(DSP_INPUT_SELECT, INPUT_FROM_HIGAIN_MIKE, min );
	break;
      case LOW_GAIN_MIKE :
	acpaput(DSP_INPUT_SELECT, INPUT_FROM_LOGAIN_MIKE, min );
	break;
      default:
	return(	EINVAL );
	break;
    }

    /* Recording:  Fill	in Monitor Mode. */
    acpaput( DSP_MONITOR_SELECT, tptr->ainfo.change.monitor, min );

    /* Here it comes ... setting the command from host to '1' starts */
    /* the DSP to recording.  We will start to get interrupts from now */
    /* on.  Initialize the buffer counters and let's go! */

    /* This sequence of	commands comes from the	DSP spec, p. 4.	*/
    /* Zero out	the track buffer counter */
    acpaput(trkbox + DSP_TX_BUFFER_CNT,	(unsigned short) 0, min	);
    /* Zero out	the host track buffer pointer */
    acpaput(trkbox + DSP_TX_HOST_BUF_PTR, (unsigned short) 0, min );
    /* Start the operation. */
    acpaput(trkbox + DSP_TX_CMD_FROM_HOST, (unsigned short) 1, min );

    tptr->rinfo.operation = RECORD;

    break;

  default:
    TRACE(("ACPA error 100\n"));
    break;

  }

  tptr->extra_data_left_over = FALSE;
  tptr->partial_block_left_over	= FALSE;
  tptr->extra_data_size	= 0;
  tptr->partial_block_size = 0;

#ifdef ACPA_MEMORY_TRACE
  /* *xxptr[min]++ = 0xcccc0000 | mpxchannel;
  dbg_print_regs( 0x0e10, min, 0xaaab0000 );
  if ( mpxchannel == 0 )
    dbg_print_regs( 0x0e18, min, 0xaaac0000 );
  else
    dbg_print_regs( 0x0e20, min, 0xaaac0000 );  */
#endif
  EXIT_ACPA_PTRACE( DEVSTART, ACPA_MAIN, min );

  return( 0 );
}

/************************************************************************/
/*									*/
/*  FUNCTION NAME: acpa_stop						*/
/*									*/
/*  DESCRIPTION:							*/
/*									*/
/*	This function resets the state of the track to stopped,	resets	*/
/*	the buffer pointers to a null state, and stops interrupts if	*/
/*	the other track	is not currently in use.			*/
/*	It implements the AUDIO_STOP request.				*/
/*									*/
/*  INVOCATION:								*/
/*									*/
/*	rc = acpa_stop(	int mpxchan, int min )				*/
/*									*/
/*  INPUT PARAMETERS:							*/
/*									*/
/*	mpxchan	is the channel id for this adapter.			*/
/*	min is the minor device	number for this	adapter.		*/
/*									*/
/*  OUTPUT PARAMETERS:							*/
/*									*/
/*	N/A								*/
/*									*/
/*  RETURN VALUES:							*/
/*									*/
/*	0 if successful							*/
/*	-1 if unsuccessful.  In	this case errno	will also be set.	*/
/*									*/
/************************************************************************/

int acpa_stop( int mpxchan, int	min )
{
  int mpxchannel;		/* the channel id for current adapter */
  register struct acpa_status *aptr; /*	ptr to current adapter's status	*/
  register Track *tptr;		/* pointer to current track info */
  int trkctl;			/* Base	addr of	track control block */
  int trkbox;			/* Track mailbox control block */
  int i;
  extern void acpa_remove_leftover_requests();

  ENTER_ACPA_PTRACE( DEVSTOP, ACPA_MAIN, min );

  i = DISABLE_INTERRUPTS( );

  mpxchannel = ( mpxchan & 3 ) - 1;	/* this	is used	as an index */

  aptr = (struct acpa_status *)	&( acpa[min] );
  tptr = (Track	*) &( acpa[min].track[mpxchannel] );

  if ( mpxchannel == 0 )
    acpaput( DSP_T1_CONTROL_BLOCK_BASE + DSP_TX_TRACK_VOLUME,
      (unsigned	short) ( 0 ), min );
  else
    acpaput( DSP_T2_CONTROL_BLOCK_BASE + DSP_TX_TRACK_VOLUME,
      (unsigned	short) ( 0 ), min );

  /* Reset the current state here so the interrupt handler picks it up as */
  /* quickly as	possible. */
  tptr->current_state =	Stopped;

  /* Reset the state for this track in the global data area. */
  /* There is nothing on the kernel ring yet. */
  tptr->ring_event = EVENT_NULL;
  /* There is nothing on the kernel ring yet. */
  tptr->stop_event = EVENT_NULL;
  /* Current queue buffer index. */
  tptr->cur_buf	= 0;
  /* NEED SOMETHING FOR	THE REQUEST QUEUE. */
  /* Current host buffer pointer. */
  tptr->host_head = 0;
  tptr->host_tail = 0; /* Current tail */
  /* The number	of used	buffers	in the kernel queues. */
  tptr->host_n_bufs = 0;
  tptr->waiting	= FALSE; /* No one is waiting */
  tptr->host_cur_buf = 0;

  /* Reset buffer size and time to 0 */
  if (tptr->rinfo.operation == PLAY)
  {
    tptr->buffer.write_buf_size = 0;
    tptr->buffer.write_buf_time = 0;
  }
  else if (tptr->rinfo.operation == RECORD)
  {
    tptr->buffer.read_buf_size = 0;
    tptr->buffer.read_buf_time = 0;
  }

  /* Reset the operation state also. */
  tptr->rinfo.operation	= STOPPED;

  acpa_remove_leftover_requests( min, mpxchannel );

  (void) ENABLE_INTERRUPTS( i );

  EXIT_ACPA_PTRACE( DEVSTOP, ACPA_MAIN, min );

  return( 0 );
}

/************************************************************************/
/*									*/
/*  FUNCTION NAME: acpa_pause						*/
/*									*/
/*  DESCRIPTION:							*/
/*									*/
/*	This function stops the	current	operation in progress on the	*/
/*	ACPA.  It does not clear the kernel buffers or anything	else.	*/
/*	The current function can be resumed via	an AUDIO_IOCTL		*/
/*	AUDIO_RESUME request.						*/
/*									*/
/*  INVOCATION:								*/
/*									*/
/*	rc = acpa_pause( int mpxchan, int min )				*/
/*									*/
/*  INPUT PARAMETERS:							*/
/*									*/
/*	mpxchan	is the channel id for this adapter.			*/
/*	min is the minor device	number for this	adapter.		*/
/*									*/
/*  OUTPUT PARAMETERS:							*/
/*									*/
/*	N/A								*/
/*									*/
/*  RETURN VALUES:							*/
/*									*/
/*	0 if successful							*/
/*	-1 if unsuccessful.						*/
/*									*/
/************************************************************************/

int acpa_pause(	int mpxchan, int min )
{
  int mpxchannel;		/* the channel id for current adapter */
  register struct acpa_status *aptr; /*	ptr to current adapter's status	*/
  register Track *tptr;		/* pointer to current track info */
  int otherchan;		/* The channel id of the other track */
  int i;

  ENTER_ACPA_PTRACE( PAUSE, ACPA_MAIN, min );

  mpxchannel = ( mpxchan & 3 ) - 1;	/* this	is used	as an index */

  aptr = (struct acpa_status *)	&( acpa[min] );
  tptr = (Track	*) &( acpa[min].track[mpxchannel] );

  /* Ensure that the adapter is	in the correct state. */
  if ( tptr->current_state != Started )
    return( -1 );

  /* Set the flag that the interrupt handler is	looking	for. */
  tptr->current_state =	Pausing;

  EXIT_ACPA_PTRACE( PAUSE, ACPA_MAIN, min );

  return( 0 );
}

/************************************************************************/
/*									*/
/*  FUNCTION NAME: acpa_resume						*/
/*									*/
/*  DESCRIPTION:							*/
/*									*/
/*	This function resumes the current operation that was suspended	*/
/*	via an AUDIO_PAUSE request on the ACPA.				*/
/*									*/
/*  INVOCATION:								*/
/*									*/
/*	rc = acpa_resume( int mpxchan, int min )			*/
/*									*/
/*  INPUT PARAMETERS:							*/
/*									*/
/*	mpxchan	is the channel id for this adapter.			*/
/*	min is the minor device	number for this	adapter.		*/
/*									*/
/*  OUTPUT PARAMETERS:							*/
/*									*/
/*	N/A								*/
/*									*/
/*  RETURN VALUES:							*/
/*									*/
/*	0 if successful							*/
/*	-1 if unsuccessful.						*/
/*									*/
/************************************************************************/

int acpa_resume( int mpxchan, int min )
{
  int mpxchannel;		/* the channel id for current adapter */
  register struct acpa_status *aptr; /*	ptr to current adapter's status	*/
  register Track *tptr;		/* pointer to current track info */
  int trkctl;			/* Base	addr of	track control block */
  int trkbox;			/* Track mailbox control block */
  int i, j;

  ENTER_ACPA_PTRACE( RESUME, ACPA_MAIN,	min );

  mpxchannel = ( mpxchan & 3 ) - 1;	/* this	is used	as an index */

  aptr = (struct acpa_status *)	&( acpa[min] );
  tptr = (Track	*) &( acpa[min].track[mpxchannel] );

  /* Don't allow a started track to be started again. */
  if ( tptr->current_state != Pausing )
    return( EINVAL );

  /* Set the flag that the interrupt handler is	looking	for. */
  tptr->current_state =	Started;

  EXIT_ACPA_PTRACE( RESUME, ACPA_MAIN, min );

  return( 0 );
}

/************************************************************************/
/*									*/
/*  FUNCTION NAME: acpa_status_request					*/
/*									*/
/*  DESCRIPTION:							*/
/*									*/
/*	This function implements the AUDIO_STATUS request for the	*/
/*	AUDIO_CONTROL ioctl.						*/
/*									*/
/*  INVOCATION:								*/
/*									*/
/*	rc = acpa_status_request( audio_status *ustatus, int min,	*/
/*				  int mpxchan )				*/
/*									*/
/*  INPUT PARAMETERS:							*/
/*									*/
/*	ustatus	is a pointer to	a AUDIO_STATUS structure.		*/
/*	min is the minor number	for the	adapter	being used.		*/
/*	mpxchan	is the channel id for the adapter being	used.		*/
/*									*/
/*  OUTPUT PARAMETERS:							*/
/*									*/
/*	N/A								*/
/*									*/
/*  RETURN VALUES:							*/
/*									*/
/*	0 if successful							*/
/*	-1 if unsuccessful.  In	this case errno	will also be set.	*/
/*									*/
/************************************************************************/

/* This	routine	satisfies the AUDIO_STATUS request */
int acpa_status_request( audio_status *ustatus,	int min, int mpxchan )
{
  register int mpxchannel;	/* the transformed mpx channel number */
  int rc;
  audio_status status;		/* user	status buffer */
  track_info tinfo;		/* track information */
  register struct acpa_status *aptr; /*	ptr to current adapter's state */
  register Track *tptr;		/* pointer to current track info */

  ENTER_ACPA_PTRACE( STATUS, ACPA_MAIN,	min );

  mpxchannel = ( mpxchan & 3 ) - 1;	/* this	is used	as an index */
  aptr = (struct acpa_status *)	&( acpa[min] );
  tptr = (Track	*) &( acpa[min].track[mpxchannel] );

  /* Copy the user's buffer into the kernel space. */
  if ( ustatus != NULL )
  {
    if ( ( rc =	copyin(	(char *) ustatus, (char	*) &status,
	   sizeof(audio_status)	) ) != 0 )
      return( EFAULT );
  }
  else
    return( EFAULT );

  /* Now copy the current status information into the local buffers.*/
  status.srate = tptr->rinfo.srate;
  status.bits_per_sample = tptr->rinfo.bits_per_sample;
  status.bsize = tptr->rinfo.bsize;
  status.mode =	tptr->rinfo.mode;
  status.channels = tptr->rinfo.channels;
  status.flags = tptr->rinfo.flags;
  status.operation = tptr->rinfo.operation;
  status.change.input =	tptr->ainfo.change.input;
  status.change.output = tptr->ainfo.change.output;
  status.change.monitor	= tptr->ainfo.change.monitor;
  tinfo.master_volume =	tptr->tinfo.master_volume;
  tinfo.dither_percent = tptr->tinfo.dither_percent;
  status.change.balance	= tptr->ainfo.change.balance;
  status.change.balance_delay =	tptr->ainfo.change.balance_delay;
  status.change.volume = tptr->ainfo.change.volume;
  status.change.volume_delay = tptr->ainfo.change.volume_delay;

  if ( status.change.dev_info != NULL )
    /* Return the track	information to the user. */
    if ( ( rc =	copyout( (char *) &tinfo, (char	*) status.change.dev_info,
			 sizeof( track_info ) )	) != 0 )
    {
      TRACE("Failed copying track information to user space in STATUS");
      return( EFAULT );
    }

  /* Return the	status information to the user.	*/
  if ( ( rc = copyout( (char *)	&status, (char *) ustatus,
		       sizeof( audio_status ) )	) != 0 )
  {
    TRACE("Failed copying status information to	user space in STATUS");
    return( EFAULT );
  }

  EXIT_ACPA_PTRACE( STATUS, ACPA_MAIN, min );

  return( 0 );
}


int acpa_wait_request( int min,	int mpxchan )
{
  register int mpxchannel;	/* the transformed mpx channel number */
  int i	= 0;
  unsigned int start_buf_count;	/* number of buffers in	the queue */
  register struct acpa_status *aptr; /*	ptr to current adapter's state */
  register Track *tptr;		/* pointer to current track info */
  int length;			/* number of interrupts	in kernel queue	*/
  int microsecs = 10000;        /* microseconds per interrupt at this rate */
  void acpa_bus_delay();


  ENTER_ACPA_PTRACE( WAIT, ACPA_MAIN, min );

  mpxchannel = ( mpxchan & 3 ) - 1;	/* this	is used	as an index */
  aptr = (struct acpa_status *)	&( acpa[min] );
  tptr = (Track	*) &( acpa[min].track[mpxchannel] );

  /* Just return if not	in playback mode; WAIT is used only on writes. */
  if ( aptr->doing_what	!= Playing )
    return( 0 );

  /* Calculate the number of microseconds per interrupt at this rate. */
  switch( tptr->rinfo.srate )
  {
    case 5500  :
      microsecs = 100000; /* 10 per second */
      break;
    case 8000  :
      if ( tptr->rinfo.channels == 2 )
	if ( tptr->rinfo.bits_per_sample == 16 )
           microsecs = 17544; /* 57 per second */
	else
           microsecs = 34483; /* 29 per second */
      else
	if ( tptr->rinfo.bits_per_sample == 16 )
           microsecs = 34483; /* 29 per second */
	else
           microsecs = 71429; /* 14 per second */
      break;
    case 11025 :
      if ( tptr->rinfo.mode == ADPCM )
         microsecs = 100000; /* 10 per second */
      else
	if ( tptr->rinfo.channels == 2 )
	  if ( tptr->rinfo.bits_per_sample ==	16 )
             microsecs = 12659; /* 79 per second */
	  else
             microsecs = 25642; /* 39 per second */
	else
	  if ( tptr->rinfo.bits_per_sample ==	16 )
             microsecs = 25642; /* 39 per second */
	  else
             microsecs = 50000; /* 20 per second */
      break;
    case 22050 :
      if ( tptr->rinfo.mode == ADPCM )
         microsecs = 50000; /* 20 per second */
      else
        if ( tptr->rinfo.channels == 2 )
	  if ( tptr->rinfo.bits_per_sample == 16 )
             microsecs = 6330; /* 158 per second */
	  else
             microsecs = 12659; /* 79 per second */
        else
	  if ( tptr->rinfo.bits_per_sample == 16 )
             microsecs = 12659; /* 79 per second */
	  else
             microsecs = 25642; /* 39 per second */
      break;
    case 44100 :
      if ( tptr->rinfo.channels == 2 )
	if ( tptr->rinfo.bits_per_sample == 16 )
           microsecs = 3175; /* 315 per second */
	else
           microsecs = 6330; /* 158 per second */
      else
	if ( tptr->rinfo.bits_per_sample == 16 )
           microsecs = 6330; /* 158 per second */
	else
           microsecs = 12659; /* 79 per second */
      break;
    default : microsecs = 100000; /* safest value (largest) */
      break;
  }

  if ( tptr->write_flag	== TRUE	)
  {
    /* The preceding write() may not yet have had enough time to get */
    /* data preloaded, so wait until the preload has occurred. */

    /* First get the number of interrupts that the kernel buffer holds.	*/
    length = tptr->wnkb;

    /* Now wait	until data is preloaded	*/
    while ( ( tptr->preload_count < 1 )	&& ( tptr->host_n_bufs > 0 ) )
    {
      acpa_bus_delay(microsecs);

      ++i;

      if (i >= length)   /* if waited for more than the size of the buffer */
        break;           /*   something is seriously wrong so break out    */
    }

    tptr->write_flag = FALSE;	/* turn	the flag back off */
  }

  i = 0;

  /* The buffer	is empty when preload_count = 0, so wait on that event.	*/
  start_buf_count = tptr->preload_count;  /* save the original count */

  while( tptr->preload_count > 0 )
  {
    acpa_bus_delay(microsecs);

    i++;			/* another sleep occurred */

    /* If there	has been no change in the buffer count,	see how	many */
    /* iterations have occurred.  If more than the size	of the queue, */
    /* then there is a problem,	so return. */
    if ( tptr->preload_count ==	start_buf_count	)
    {
      if ( i > length )
	return(	EBUSY );
    }
    else
    {
      /* A change occurred, so reset the initial state again. */
      /* (Because write	may keep adding	data to	the queue, preload_count */
      /* may increase as well as decrease, so we need to reset.) */
      start_buf_count =	tptr->preload_count;  /* save the new count */
      i	= 0;
    }
  }

  EXIT_ACPA_PTRACE( WAIT, ACPA_MAIN, min );

  return(0);

} /* acpa_wait_request() */



void init_audio_buffer(	Track *tptr, unsigned long position, int min )
{
  struct acpa_status *aptr;

  ENTER_ACPA_PTRACE( BUFFER, ACPA_SUBR_ONE, min	);

  aptr = (struct acpa_status *)	&( acpa[min] );

  /* Initialize	the data structure for AUDIO_BUFFER. */
  if ( tptr->rinfo.mode	== ADPCM )
    tptr->buffer.position = position;
  else
    /* On PCM, the position in milliseconds needs to be	converted into */
    /* bytes. */
    switch( tptr->rinfo.srate )
    {
      case 8000	:
	if ( tptr->rinfo.bits_per_sample == 8 )
	  if ( tptr->rinfo.channels == 1 )
	    tptr->buffer.position = ( position * 8000 )	/ 1000;
	  else
	    tptr->buffer.position = ( position * 8000 )	/ 500;
	else
	  if ( tptr->rinfo.channels == 1 )
	    tptr->buffer.position = ( position * 8000 )	/ 500;
	  else
	    tptr->buffer.position = ( position * 8000 )	/ 250;
	break;
      case 11025 :
	if ( tptr->rinfo.bits_per_sample == 8 )
	  if ( tptr->rinfo.channels == 1 )
	    tptr->buffer.position = ( position * 11025 ) / 1000;
	  else
	    tptr->buffer.position = ( position * 11025 ) / 500;
	else
	  if ( tptr->rinfo.channels == 1 )
	    tptr->buffer.position = ( position * 11025 ) / 500;
	  else
	    tptr->buffer.position = ( position * 11025 ) / 250;
	break;
      case 22050 :
	if ( tptr->rinfo.bits_per_sample == 8 )
	  if ( tptr->rinfo.channels == 1 )
	    tptr->buffer.position = ( position * 22050 ) / 1000;
	  else
	    tptr->buffer.position = ( position * 22050 ) / 500;
	else
	  if ( tptr->rinfo.channels == 1 )
	    tptr->buffer.position = ( position * 22050 ) / 500;
	  else
	    tptr->buffer.position = ( position * 22050 ) / 250;
	break;
      case 44100 :
	if ( tptr->rinfo.bits_per_sample == 8 )
	  if ( tptr->rinfo.channels == 1 )
	    tptr->buffer.position = ( position * 44100 ) / 1000;
	  else
	    tptr->buffer.position = ( position * 44100 ) / 500;
	else
	  if ( tptr->rinfo.channels == 1 )
	    tptr->buffer.position = ( position * 44100 ) / 500;
	  else
	    tptr->buffer.position = ( position * 44100 ) / 250;
	break;
    }

  tptr->buffer.flags = 0;
  tptr->buffer.read_buf_size = tptr->buffer.write_buf_size = 0;
  tptr->buffer.read_buf_time = tptr->buffer.write_buf_time = 0;
  tptr->buffer.read_buf_max = tptr->buffer.write_buf_max = 0;
  tptr->buffer.position_type = POS_MSECS;
  tptr->buffer.read_buf_cap = tptr->rnkb * tptr->rinfo.bsize;
  tptr->buffer.write_buf_cap = tptr->wnkb * tptr->rinfo.bsize;
  tptr->buffer.request_buf_cap = aptr->dds.MACPA_request_buf;

  EXIT_ACPA_PTRACE( BUFFER, ACPA_SUBR_ONE, min );

}

int acpa_buffer_request( audio_buffer *ubuffer,	int min, int mpxchan )
{
  register int mpxchannel;	/* the transformed mpx channel number */
  int rc;
  audio_buffer buffer;		/* local audio_buffer buffer */
  register Track *tptr;		/* pointer to current track info */
  extern int acpa_convert_position();

  ENTER_ACPA_PTRACE( BUFFER, ACPA_MAIN,	min );

  mpxchannel = ( mpxchan & 3 ) - 1;	/* this	is used	as an index */
  tptr = (Track	*) &( acpa[min].track[mpxchannel] );

  /* Copy the user's buffer into the kernel space. */
  if ( ubuffer != NULL )
  {
    if ( ( rc =	copyin(	(char *) ubuffer, (char	*) &buffer,
	   sizeof(audio_buffer)	) ) != 0 )
      return( EFAULT );
  }
  else
    return( EFAULT );

  /* Copy the information into the local buffer	*/
  buffer.flags = tptr->buffer.flags;
  buffer.read_buf_size = tptr->buffer.read_buf_size;
  buffer.write_buf_size	= tptr->buffer.write_buf_size;
  if ( tptr->rinfo.mode	!= ADPCM )
  {
    buffer.read_buf_time = acpa_convert_position( tptr,	ACPA_READ_TYPE,
			   tptr->buffer.read_buf_size );
    buffer.write_buf_time = acpa_convert_position( tptr, ACPA_WRITE_TYPE,
			    tptr->buffer.write_buf_size	);
  }
  else
  {
    if (tptr->rinfo.srate == 22050)
    {
      buffer.read_buf_time = ( tptr->buffer.read_buf_size /
			       tptr->rinfo.bsize ) * 50;
      buffer.write_buf_time = ( tptr->buffer.write_buf_size /
			       tptr->rinfo.bsize ) * 50;
    }
    else
    {
      buffer.read_buf_time = ( tptr->buffer.read_buf_size /
			       tptr->rinfo.bsize ) * 100;
      buffer.write_buf_time = ( tptr->buffer.write_buf_size /
			       tptr->rinfo.bsize ) * 100;
    }
  }

  buffer.read_buf_max =	tptr->buffer.read_buf_max * tptr->rinfo.bsize;
  buffer.write_buf_max = tptr->buffer.write_buf_max * tptr->rinfo.bsize;
  if ( tptr->rinfo.mode	== ADPCM )
    buffer.position = tptr->buffer.position;
  else
    if ( acpa[min].doing_what == Recording )
      buffer.position =	acpa_convert_position( tptr, ACPA_READ_TYPE,
			tptr->buffer.position );
    else
      buffer.position =	acpa_convert_position( tptr, ACPA_WRITE_TYPE,
			tptr->buffer.position );
  buffer.position_type = tptr->buffer.position_type;
  buffer.read_buf_cap =	tptr->buffer.read_buf_cap;
  buffer.write_buf_cap = tptr->buffer.write_buf_cap;
  buffer.request_buf_cap = tptr->buffer.request_buf_cap;

  /* Return the	buffer information to the user.	*/
  if ( ( rc = copyout( (char *)	&buffer, (char *) ubuffer,
		       sizeof( audio_buffer ) )	) != 0 )
  {
    TRACE("Failed copying buffer information to	user space in BUFFER");
    return( EFAULT );
  }

  EXIT_ACPA_PTRACE( BUFFER, ACPA_MAIN, min );

  return( 0 );
}

int acpa_add_request_to_q( audio_control *control, int min, int	mpxchan	)
{
  int mpxchannel;
  struct audio_request *rptr;
  audio_change *cptr;
  track_info *tptr;
  int rc;
  extern void acpa_insert_request();
  extern int acpa_convert_position();

  ENTER_ACPA_PTRACE( CHANGE, ACPA_SUBR_ONE, min	);

  mpxchannel = ( mpxchan & 3 ) -1;		/* get the track # */

  /* Only the number of	requests as specified in ODM may be enqueued. */
  if ( acpa[min].track[mpxchannel].num_requests	>
       acpa[min].dds.MACPA_request_buf )
    /* No more requests	are allowed. */
    return( E2BIG );

  /* Get local storage to store	the request and	save its contents. */
  if ( ( rptr =	(struct	audio_request *) GET_MEMORY( sizeof(
	 struct	audio_request )	) ) == NULL )
    return( ENOMEM );
  rptr->ioctl_request =	control->ioctl_request;
  rptr->position = control->position;
  rptr->min = min;
  rptr->mpxchan	= mpxchan;
  rptr->pid = getpid();

  if ( control->ioctl_request == AUDIO_CHANGE )
  {
    /* Get the CHANGE data. */
    if ( ( cptr	= (audio_change	*) GET_MEMORY( sizeof (	audio_change ) ) )
	   == NULL )
      return( ENOMEM );
    else
      rptr->request_info = (char *) cptr;
    /* Copy the	data from user space into kernel space.	*/
    if ( ( rc =	copyin(	(char *) control->request_info,	(char *) cptr,
	   sizeof( audio_change	) ) ) != 0 )
      return( EFAULT );
    if ( cptr->dev_info	!= NULL	)
    {
      /* Get the track data. */
      if ( ( tptr = (track_info	*) GET_MEMORY( sizeof (	track_info ) ) )
	     ==	NULL )
	return(	ENOMEM );
      if ( ( rc	= copyin( (char	*) cptr->dev_info, (char *) tptr,
	     sizeof( track_info	) ) ) != 0 )
	return(	EFAULT );
      else
	cptr->dev_info = tptr;
    }
  }
  else
    rptr->request_info = NULL;

  acpa_insert_request( rptr );

  EXIT_ACPA_PTRACE( CHANGE, ACPA_SUBR_ONE, min );

  return( 0 );
}

void acpa_insert_request( struct audio_request *new_rptr )
{
  register struct audio_request	*rptr1,	*rptr2;
  int done = FALSE;

  ENTER_ACPA_PTRACE( CHANGE, ACPA_SUBR_TWO, -1 );

  if ( acpa[new_rptr->min].track[(new_rptr->mpxchan&3)-1].request_list ==
       NULL )
  {
    /* The list	is empty, so make the new request the first one. */
    acpa[new_rptr->min].track[(new_rptr->mpxchan&3)-1].request_list =
      new_rptr;
    new_rptr->next_ptr = NULL;
  }
  else
  {
    rptr1 = acpa[new_rptr->min].track[(new_rptr->mpxchan&3)-1].request_list;
    if ( rptr1->position > new_rptr->position )
    {
      /* The new request should	be the first request in	the list. */
      new_rptr->next_ptr = rptr1;
      acpa[new_rptr->min].track[(new_rptr->mpxchan&3)-1].request_list =
	new_rptr;
    }
    else
    {
      /* Determine where the new request should	go and insert it there.	*/
      rptr2 = rptr1->next_ptr;
      while ( !	done )
	if ( rptr2 == NULL )
	{
	  /* The new request should be the last	request	in the list. */
	  rptr1->next_ptr = new_rptr;
	  new_rptr->next_ptr = NULL;
	  done = TRUE;
	}
	else
	  if ( rptr2->position > new_rptr->position )
	  {
	    /* The new request goes before rptr2. */
	    new_rptr->next_ptr = rptr2;
	    rptr1->next_ptr = new_rptr;
	    done = TRUE;
	  }
	  else
	  {
	    /* The proper position is not found	yet; keep searching. */
	    rptr1 = rptr2;
	    rptr2 = rptr2->next_ptr;
	  }
    }
  }

  EXIT_ACPA_PTRACE( CHANGE, ACPA_SUBR_TWO, -1 );

}

void acpa_remove_leftover_requests( int	min, int mpxchannel )
{
  struct audio_request *rptr, *rptr2;
  audio_change *cptr;
  track_info *tptr;

  ENTER_ACPA_PTRACE( CHANGE, ACPA_SUBR_FOUR, min );

  rptr = acpa[min].track[mpxchannel].request_list;

  while	( rptr != NULL )
  {
    if ( rptr->ioctl_request ==	AUDIO_CHANGE )
    {
      cptr = (audio_change *) rptr->request_info;
      if ( cptr->dev_info != NULL )
      {
	tptr = (track_info *) cptr->dev_info;
	(void) FREE_MEMORY( tptr );
      }
      (void) FREE_MEMORY( cptr );
    }
    rptr2 = rptr->next_ptr;
    (void) FREE_MEMORY(	rptr );
    rptr = rptr2;
  }

  acpa[min].track[mpxchannel].request_list = NULL;
  acpa[min].track[mpxchannel].num_requests = 0;

  EXIT_ACPA_PTRACE( CHANGE, ACPA_SUBR_FOUR, min	);

}

void acpa_handle_request( Track	*tptr, int min )
{
  struct audio_request *rptr, *rptr2;
  audio_change *cptr;
  track_info *tptr2;
  int rc;
  int done = FALSE;
  int do_it = FALSE;
  int new_position;
  extern int acpa_convert_position();

  ENTER_ACPA_PTRACE( CHANGE, ACPA_SUBR_THREE, min );

  while	( ! done )
  {
    rptr = tptr->request_list;
    /* Are there any requests in the queue? */
    if ( rptr != NULL )
    {
      /* Is the	request	ready to be executed? */
#ifdef ACPA_MEMORY_TRACE
#endif
      if ( tptr->rinfo.mode == ADPCM )
      {
	if ( rptr->position <= tptr->buffer.position )
	  do_it	= TRUE;
      }
      else
      {
	new_position = (unsigned long) acpa_convert_position
		       ( tptr, ACPA_WRITE_TYPE,	tptr->buffer.position );
#ifdef ACPA_MEMORY_TRACE
#endif
	if ( rptr->position <= new_position )
	  do_it	= TRUE;
      }
      if ( do_it == TRUE )
      {
#ifdef ACPA_MEMORY_TRACE
  *xxptr[min]++ = 0x1230;
  *xxptr[min]++ = rptr->position;
  *xxptr[min]++ = new_position;
#endif
	/* Yes,	so remove this request from the	list. */
	rptr2 =	rptr->next_ptr;
	tptr->request_list = rptr2;
#ifdef ACPA_MEMORY_TRACE
#endif
	/* Now execute the request. */
	switch ( rptr->ioctl_request )
	{
	  case AUDIO_CHANGE :
#ifdef ACPA_MEMORY_TRACE
  *xxptr[min]++ = 0x1231;
  *xxptr[min]++ = rptr->min;
  *xxptr[min]++ = rptr->mpxchan;
#endif
	    rc = acpa_change_request( rptr->request_info, rptr->min,
				      rptr->mpxchan, ENQUEUED, rptr->pid );
	    break;
	  case AUDIO_STOP :
	    rc = acpa_stop( rptr->mpxchan, rptr->min );
	    break;
	  case AUDIO_PAUSE :
	    rc = acpa_pause( rptr->mpxchan, rptr->min );
	    break;
	  default :
	    break;
	}

	/* Deallocate the request. */
	if ( rptr->ioctl_request == AUDIO_CHANGE )
	{
	  cptr = (audio_change *) rptr->request_info;
	  if ( cptr->dev_info != NULL )
	  {
	    tptr2 = (track_info	*) cptr->dev_info;
	    (void) FREE_MEMORY(	tptr2 );
	  }
	  (void) FREE_MEMORY( cptr );
	}
	(void) FREE_MEMORY( rptr );
	do_it =	FALSE;
      }
      else
	done = TRUE;
    }
    else
      done = TRUE;
  }

  EXIT_ACPA_PTRACE( CHANGE, ACPA_SUBR_THREE, min );

}

/************************************************************************/
/*									*/
/*  FUNCTION NAME: acpa_load						*/
/*									*/
/*  DESCRIPTION:							*/
/*									*/
/*	This function loads user-specified microcode.  The code	must	*/
/*	act exactly like supported microcode.				*/
/*									*/
/*  INVOCATION:								*/
/*									*/
/*	rc = acpa_load(	audio_load *uload, int mpxchan,	int min	)	*/
/*									*/
/*  INPUT PARAMETERS:							*/
/*									*/
/*	uload is a pointer to an audio_load structure.			*/
/*	mpxchan	is the channel id for this adapter.			*/
/*	min is the minor device	number for this	adapter.		*/
/*									*/
/*  OUTPUT PARAMETERS:							*/
/*									*/
/*	N/A								*/
/*									*/
/*  RETURN VALUES:							*/
/*									*/
/*	0 if successful							*/
/*	-1 if unsuccessful.						*/
/*									*/
/************************************************************************/

int acpa_load_request( audio_load *uload, int min, int mpxchan )
{
  register int mpxchannel;	/* the transformed mpx channel number */
  int rc;
  audio_load load;		/* local audio_load buffer */
  register Track *tptr;		/* pointer to current track info */
  char *tempptr;		/* pointer to temporary	buffer */
  Shorts app_ucode;		/* application's microcode to be loaded. */
  int i, j;

  ENTER_ACPA_PTRACE( INIT, ACPA_SUBR_ONE, min );

  mpxchannel = ( mpxchan & 3 ) - 1;	/* this	is used	as an index */
  tptr = (Track	*) &( acpa[min].track[mpxchannel] );

  /* Don't load	microcode if the device	isn't stopped. */
  if ( ( acpa[min].track[0].current_state > Stopped ) ||
       ( acpa[min].track[1].current_state > Stopped ) )
    return( EINVAL );

  /* Copy the user's buffer into the kernel space. */
  if ( uload !=	NULL )
  {
    if ( ( rc =	copyin(	(char *) uload,	(char *) &load,
	   sizeof( audio_load )	) ) != 0 )
      return( EFAULT );
  }
  else
    return( EFAULT );

  /* Verify that the proper flags have been set	by the caller. */
  if ( !( load.flags & LOAD_START ) || !( load.flags & LOAD_END	) )
    return( EINVAL );

  /* Reject the	request	if there is no buffer. */
  if ( load.buffer == NULL )
    return( EINVAL );

  /* Reject the	request	if the file size is 0. */
  if ( load.size == 0 )
    return( EINVAL );

  /* Now stop the adapter to ensure that any previously	loaded microcode */
  /* is	not going to overwrite code or data before the new microcode is	*/
  /* activated.	*/
  acpaoutb(acpa[min].cmd_stat_reg, (unsigned char) 0);

  /* Now get a temporary buffer	to transfer the	microcode from system */
  /* memory to kernel memory. */
  if ( ( tempptr = GET_MEMORY( load.size ) ) ==	NULL )
    return( ENOMEM );

  /* Copy the microcode	from user space	to kernel space. */
  if ( ( rc = copyin( load.buffer, tempptr, load.size )	) != 0 )
    return( EFAULT );

  /* Position the pointer past the copyright header. */
  tempptr += COPYRIGHT_LEN;
  app_ucode.data = (unsigned short *) tempptr;
  app_ucode.len	=  load.size - COPYRIGHT_LEN;

  tempptr -= COPYRIGHT_LEN;	/* Get back the	original address. */

  /* Now load the microcode onto the adapter. */
  if ( ( rc = acpaload(	&app_ucode, min	) ) != 0 )
  {
    FREE_MEMORY( tempptr );
    acpaerr(ERRID_ACPA_UCODE, "sysxacpa", min, __LINE__,"acpa_load_request","acpaload",rc,NULL,NULL);
    return( ENXIO );
  }

  /* Now ensure	that the microcode has time to initialize itself */
  /* before doing anything else. */
  /* Write out any value to 0xe1d. */
  /* NOTE THAT THIS NEEDS TO BE	VALID FOR EITHER TRACK */
  acpaput( DSP_T1_MAILBOX_BASE + DSP_TX_TMS_STATUS,
	   (unsigned short) 0x0F0F, min	);
  /* Reset the adapter.	*/
  acpaoutb(acpa[min].cmd_stat_reg, (unsigned char) TMS_RES | TMS_INT_CMD);
  /* Now wait for the register to get cleared. */
  for (	i=0; i<0xffffffff; i++ )
  {
    rc = (int) acpaget(	DSP_T1_MAILBOX_BASE + DSP_TX_TMS_STATUS,
			min );
    if ( !rc )
      break;
  }

  /* Wait fifteen more milliseconds, and then everything is OK. */
  acpa_bus_delay( 15000 );

  /* Now start the adapter. */
  /*acpaoutb( acpa[min].cmd_stat_reg, TMS_RES |	TMS_INT_CMD );	*/

  /* Note that,	unlike known microcode,	no permanent state for */
  /* application-supplied microcode is left around. */

  FREE_MEMORY( tempptr );

  EXIT_ACPA_PTRACE( INIT, ACPA_SUBR_ONE, min );

  return( 0 );

}

void acpa_init_registers( int trkbox, int min )
{
  acpaput( trkbox + DSP_TX_CMD_FROM_HOST, (unsigned short) 0, min );
  acpaput( trkbox + DSP_TX_DSP_BUF_PTR,	(unsigned short) 0, min	);
  acpaput( trkbox + DSP_TX_CMD_FROM_DSP, (unsigned short) 0, min );
  acpaput( trkbox + DSP_TX_HOST_BUF_PTR, (unsigned short) 0, min );
  acpaput( trkbox + DSP_TX_BUFFER_CNT, (unsigned short)	0, min );
  acpaput( trkbox + DSP_TX_TMS_STATUS, (unsigned short)	0, min );
  acpaput( trkbox + DSP_TX_LEVEL, (unsigned short) 0, min );
}

void acpa_print_regs( int trkbox, int min )
{
  unsigned short rc;

  rc = acpaget( trkbox + DSP_TX_CMD_FROM_HOST, min );
  printf("CMDFROMHOST %x ", rc );
  rc = acpaget( trkbox + DSP_TX_DSP_BUF_PTR, min );
  printf("DSPBUFPTR %x ", rc );
  rc = acpaget( trkbox + DSP_TX_CMD_FROM_DSP, min );
  printf("CMDFROMDSP %x ", rc );
  rc = acpaget( trkbox + DSP_TX_HOST_BUF_PTR, min );
  printf("HOSTBUFPTR %x ", rc );
  rc = acpaget( trkbox + DSP_TX_BUFFER_CNT, min );
  printf("BUFFERCNT %x ", rc );
  rc = acpaget( trkbox + DSP_TX_TMS_STATUS, min );
  printf("TMS_STATUS %x ", rc );
  rc = acpaget( trkbox + DSP_TX_LEVEL, min );
  printf("LEVEL %x\n", rc );
}

#ifdef ACPA_MEMORY_TRACE
void dbg_print_regs( int trkbox, int min, int number )
{
  unsigned short rc;

  *xxptr[min]++ = number | trkbox;
  *xxptr[min]++ = acpaget( trkbox + 0, min );
  *xxptr[min]++ = acpaget( trkbox + 1, min );
  *xxptr[min]++ = acpaget( trkbox + 2, min );
  *xxptr[min]++ = acpaget( trkbox + 3, min );
  *xxptr[min]++ = acpaget( trkbox + 4, min );
  *xxptr[min]++ = acpaget( trkbox + 5, min );
  *xxptr[min]++ = acpaget( trkbox + 6, min );
}
#endif

void acpa_init_pcm_regs( int pcmbase, int min )
{
  acpaput( pcmbase + PCM_INTERRUPT_COUNTER, (unsigned short) 0, min );
  acpaput( pcmbase + PCM_TYPE, (unsigned short)	0, min );
  acpaput( pcmbase + PCM_SAMPLE_RATE, (unsigned	short) 0, min );
  acpaput( pcmbase + PCM_DITHER_LEVEL, (unsigned short)	0, min );
  acpaput( pcmbase + PCM_SAMPLE_WIDTH, (unsigned short)	0, min );
  acpaput( pcmbase + PCM_BYTE_SWAPPING,	(unsigned short) 0, min	);
  acpaput( pcmbase + PCM_NUMBER_FORMAT,	(unsigned short) 0, min	);
  acpaput( pcmbase + PCM_NUMBER_CHANNELS, (unsigned short) 0, min );
  acpaput( pcmbase + PCM_SOURCE_MIX, (unsigned short) 0, min );
}

#ifdef ACPA_MEMORY_TRACE
void print_acpa_pcm_regs( int pcmbase, int min )
{
  int i, j;
  unsigned short rc[4];

  for( i=0; i<4; i++ )
  {
    for	( j=0; j<4; j++	)
      rc[j] = (unsigned	short) acpaget(	pcmbase	+ i*4 +	j, min );
    /* brkpoint( i, j, rc[0], rc[1], rc[2], rc[3] ); */
  }
}
#endif

/* This	routine	writes one block of data to the	adapter. */

void acpa_write_block( Track *tptr, struct acpa_status *aptr, int trkbox,
		       int min )
{
  int temp, temp2;
  extern void acpaout_mult();

  ENTER_ACPA_PTRACE( INTERRUPT,	ACPA_SUBR_ONE, min );

  /* Ask the adapter which buffer should be written next. */
  /*temp = (int) acpaget(       trkbox + DSP_TX_HOST_BUF_PTR, min ); */
  temp = tptr->host_buf_ptr;
#ifdef ACPA_MEMORY_TRACE
#endif

  /* Write the data. */

  /* Select slot in kernel queue to use	(store in temp2) */
  temp2	= tptr->host_tail + 1; /* Least	recently filled	kernel buffer */
  if (temp2 >= tptr->wnkb)
    temp2 -= tptr->wnkb;	       /* ...modulo wnkb */
  tptr->host_tail = temp2;

  /* Move data from kernel queue into the DSP queue */
  acpaout(aptr->addr_low_reg,
	(unsigned short) tptr->buf_loc[temp]);

  /* Output one	block at a time. */
  acpaout_mult(	aptr->data_low_reg, tptr->wbuf[temp2],
		tptr->buf_size );

  /* Update the	host buffer pointer for	the DSP	*/
  temp++;
  if ( temp >= tptr->modulo_factor )
    temp = 0;
  (void) acpaput(trkbox+DSP_TX_HOST_BUF_PTR,
		 (unsigned short) temp,	min );
  tptr->host_buf_ptr = temp;

  /* Free slot in the kernel queue */
  tptr->host_n_bufs--;	      /* One more empty	buffer in the kernel queue */

  /* If	anyone is waiting for a	slot in	the kernel queue, wake them up */
  if (tptr->waiting == TRUE)
  {
    /* If anyone was waiting for space,	*/
    WAKEUP2(&(tptr->ring_event), RING_EVENT );
			     /*	wake 'em up now	*/
  }

  /* Decrement the buffer's write buffer size by the number of */
  /* bytes in the block	of data. */
  tptr->buffer.write_buf_size -= tptr->rinfo.bsize;

  EXIT_ACPA_PTRACE( INTERRUPT, ACPA_SUBR_ONE, min );

}

/************************************************************************/
/*									*/
/*  FUNCTION NAME: acpaerr						*/
/*									*/
/*  DESCRIPTION:							*/
/*									*/
/*	This routine sets up the detail	data for errlogging and	calls	*/
/*	the errlogging routine 'errsave' to log	errors.			*/
/*									*/
/*  INVOCATION:								*/
/*									*/
/*	acpaerr( unsigned int errid, char *res_name, int min,		*/
/*		 char *dmodule,	char *fmodule, int return_code,		*/
/*		 int line_num, char *path, char	*image)			*/
/*									*/
/*  INPUT PARAMETERS:							*/
/*									*/
/*	errid is the id	number in errids.h for the message template.	*/
/*	*res_name is the component name	"sysxacpa".			*/
/*	min is the device number.					*/
/*	line_num is the	source code line number.			*/
/*	dmodule	is the module that detected the	error.			*/
/*	fmodule	is the module that returned the	error.			*/
/*	return_code is the code	returned by the	error condition.	*/
/*	path is	the where the microcode	resides.			*/
/*	image is the microcode image type.				*/
/*									*/
/*  OUTPUT PARAMETERS:							*/
/*									*/
/*	N/A								*/
/*									*/
/*  RETURN VALUES:							*/
/*									*/
/*	0 if successful							*/
/************************************************************************/

int
acpaerr(errid,res_name,min,line_num,dmodule,fmodule,return_code,path,image)
unsigned int  errid;
char   *res_name;
int    min;
int    line_num;
char   *dmodule;
char   *fmodule;
int    return_code;
char   *path;
char   *image;

{

ER.error_id = errid;			/* Template Id # */

sprintf(ER.resource_name,"%8s",res_name);

sprintf(ER.detail_data,"%5d%5d%25s%25s%5d%30s%30s",
	min,line_num,dmodule,fmodule,return_code,path,image);

/* Call	system error logging routine */
errsave(&ER, ERR_REC_SIZE + (strlen(ER.detail_data)+1));

return(0);
}

/* This	routine	converts position from bytes into milliseconds.	*/

int acpa_convert_position( Track *tptr,	int type, int position )
{
  int buf_time;

  /* The algorithm used	here is	(#_of_bytes*milliseconds_per_block) */
  /* divided by	#_of_bytes_per_block. */

  switch( tptr->rinfo.srate )
  {
    case 8000 :
      if ( tptr->rinfo.bits_per_sample == 8 )
	if ( tptr->rinfo.channels == 1 )
	{
	  if ( type == ACPA_READ_TYPE )
	    buf_time = ( ( position * 70 ) / tptr->rinfo.bsize );
	  else
	    buf_time = ( ( position * 70 ) / tptr->rinfo.bsize );
	}
	else
	{
	  if ( type == ACPA_READ_TYPE )
	    buf_time = ( ( position * 35 ) / tptr->rinfo.bsize );
	  else
	    buf_time = ( ( position * 35 ) / tptr->rinfo.bsize );
	}
      else	  /* 16	bits per sample	*/
	if ( tptr->rinfo.channels == 1 )
	{
	  if ( type == ACPA_READ_TYPE )
	    buf_time = ( ( position * 35 ) / tptr->rinfo.bsize );
	  else
	    buf_time = ( ( position * 35 ) / tptr->rinfo.bsize );
	}
	else
	{
	  if ( type == ACPA_READ_TYPE )
	    buf_time = ( ( position * 17 ) / tptr->rinfo.bsize );
	  else
	    buf_time = ( ( position * 17 ) / tptr->rinfo.bsize );
	}
      break;
    case 11025 :
      if ( tptr->rinfo.bits_per_sample == 8 )
	if ( tptr->rinfo.channels == 1 )
	{
	  if ( type == ACPA_READ_TYPE )
	    buf_time = ( ( position * 51 ) / tptr->rinfo.bsize );
	  else
	    buf_time = ( ( position * 51 ) / tptr->rinfo.bsize );
	}
	else
	{
	  if ( type == ACPA_READ_TYPE )
	    buf_time = ( ( position * 25 ) / tptr->rinfo.bsize );
	  else
	    buf_time = ( ( position * 25 ) / tptr->rinfo.bsize );
	}
      else	  /* 16	bits per sample	*/
	if ( tptr->rinfo.channels == 1 )
	{
	  if ( type == ACPA_READ_TYPE )
	    buf_time = ( ( position * 25 ) / tptr->rinfo.bsize );
	  else
	    buf_time = ( ( position * 25 ) / tptr->rinfo.bsize );
	}
	else
	{
	  if ( type == ACPA_READ_TYPE )
	    buf_time = ( ( position * 13 ) / tptr->rinfo.bsize );
	  else
	    buf_time = ( ( position * 13 ) / tptr->rinfo.bsize );
	}
      break;
    case 22050 :
      if ( tptr->rinfo.bits_per_sample == 8 )
	if ( tptr->rinfo.channels == 1 )
	{
	  if ( type == ACPA_READ_TYPE )
	    buf_time = ( ( position * 25 ) / tptr->rinfo.bsize );
	  else
	    buf_time = ( ( position * 25 ) / tptr->rinfo.bsize );
	}
	else
	{
	  if ( type == ACPA_READ_TYPE )
	    buf_time = ( ( position * 13 ) / tptr->rinfo.bsize );
	  else
	    buf_time = ( ( position * 13 ) / tptr->rinfo.bsize );
	}
      else	  /* 16	bits per sample	*/
	if ( tptr->rinfo.channels == 1 )
	{
	  if ( type == Recording )
	    buf_time = ( ( position * 13 ) / tptr->rinfo.bsize );
	  else
	    buf_time = ( ( position * 13 ) / tptr->rinfo.bsize );
	}
	else
	{
	  if ( type == ACPA_READ_TYPE )
	    buf_time = ( ( position * 6	) / tptr->rinfo.bsize );
	  else
	    buf_time = ( ( position * 6	) / tptr->rinfo.bsize );
	}
      break;
    case 44100 :
      if ( tptr->rinfo.bits_per_sample == 8 )
	if ( tptr->rinfo.channels == 1 )
	{
	  if ( type == ACPA_READ_TYPE )
	    buf_time = ( ( position * 13 ) / tptr->rinfo.bsize );
	  else
	    buf_time = ( ( position * 13 ) / tptr->rinfo.bsize );
	}
	else
	{
	  if ( type == ACPA_READ_TYPE )
	    buf_time = ( ( position * 6	) / tptr->rinfo.bsize );
	  else
	    buf_time = ( ( position * 6	) / tptr->rinfo.bsize );
	}
      else	  /* 16	bits per sample	*/
	if ( tptr->rinfo.channels == 1 )
	{
	  if ( type == ACPA_READ_TYPE )
	    buf_time = ( ( position * 6	) / tptr->rinfo.bsize );
	  else
	    buf_time = ( ( position * 6	) / tptr->rinfo.bsize );
	}
	else
	{
	  if ( type == ACPA_READ_TYPE )
	    buf_time = ( ( position * 3	) / tptr->rinfo.bsize );
	  else
	    buf_time = ( ( position * 3	) / tptr->rinfo.bsize );
	}
      break;
  }

  return( buf_time );
}
