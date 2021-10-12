static char sccsid[] = "@(#)52  1.60  src/bos/kernext/cat/cat_ioctl.c, sysxcat, bos412, 9446B 11/16/94 11:51:30";
/*
 * COMPONENT_NAME: (SYSXCAT) - Channel Attach device handler
 *
 * FUNCTIONS: catioctl(), do_dnld(), cio_start(), cio_halt(), cio_get_stat(),
 *      do_sram_cmd(), do_pos_cmd(), do_setadap(), reset_sub(),
 *      reset_all(), rw_sram(), load_cu(), do_cutable(), do_set_sub(),
 *      do_startsub(), reset_adapter(), free_stat_element(),
 *      cio_get_vpd(), catinfo(), do_diag(), adap_info(),
 *      do_stopsub(), startclawsub(), sess_start(), sess_disc(),
 *      cat_on_offline(), stop_all_subs(), cio_getfastwrt(), do_unst()
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

#define FNUM 5
#include <net/spl.h>
#include <sys/adspace.h>
#include <sys/errno.h>
#include <sys/ddtrace.h>
#include <sys/trchkid.h>
#include <sys/comio.h>
#include <sys/device.h>
#include <fcntl.h>
#include <sys/uio.h>
#include <sys/sleep.h>
#include <sys/malloc.h>
#include <sys/iocc.h>
#include <sys/except.h>
#include <sys/devinfo.h>
#include <sys/id.h>
#include <sys/ciouser.h>

#ifdef _POWER_PC
#include <sys/systemcfg.h>
#endif

#include "catdd.h"

/* External function prototypes *
void ack_stop(struct ca *ca, cmd_t *notify);

/* Local (static) Function Prototypes */
static int cat_on_offline(
        struct ca *ca,
        struct cat_on_offline *on_off,
        chan_t chan);

static uchar claw_correlator = 3; /*pick any number */

uchar claw_params[]= { 0x02,0x0,0x81,0x03,0x01,0x01, /* channel parameters */
                       0,0,0,0,0,0,0,0, /* adapter name */
                       0,0,0,0,0,0,0,0, /* hostname */
                     };

#define CLAW_PARAM_NAMES ((char *)(claw_params+6))


/*****************************************************************************
** NAME:     catioctl
**
** FUNCTION: ioctl entry point from kernel
**
** EXECUTION ENVIRONMENT: process only
**
** NOTES:
**
** RETURNS:
**              0 : SUCCESS
**              ENXIO -- bad mpx return code
**              EFAULT -- data copy failed or didn't specify
**              ENOSPC -- no subchannels available
**              EADDRINUSE -- duplicate subchannel id specified
**              EINVAL -- invalid subchannel id, speed, or opcode specified
**              ENOMEM -- no system memory available.
**
*****************************************************************************/
int
catioctl(
        dev_t devno,            /* major and minor number */
        int cmd,                        /* ioctl operation desired */
        int arg,                        /* argument for this command */
        ulong devflag,          /* flags including DKERNEL */
        chan_t chan,            /* channel number */
        int ext)                        /* optional additional argument */
{
        struct ca *ca;
        open_t *openp;
        int rc = 0;
        int spl;
        cmd_t local_cmd;

        DDHKWD5(HKWD_DD_CATDD,DD_ENTRY_IOCTL,0,devno,cmd,devflag,chan,ext);

        if ((chan < 0) || (chan > CAT_MAX_OPENS) ||
            ((ca = catget(minor(devno))) == NULL) ||
            (((openp = &ca->open_lst[chan])->op_flags&OP_OPENED) == 0)) {
                DDHKWD1(HKWD_DD_CATDD, DD_EXIT_IOCTL, ENXIO, devno);
                return ENXIO;
        }

        if (lockl(&ca->adap_lock, LOCK_SIGRET) != LOCK_SUCC) {
                DDHKWD1(HKWD_DD_CATDD, DD_EXIT_IOCTL, EINTR, devno);
                return EINTR;
        }

        /*
        ** handle standard ioctl's
        */
        switch (cmd) {
        case IOCINFO:
                /* Return device-specific info. */
                rc = catinfo(ca, arg, devflag);
                break;
        case CIO_QUERY:
                /* Return driver counter statistics */
                rc = cio_query(ca, openp, arg, devflag, chan);
                break;
        case CIO_GET_STAT:
                /* Return asynchronous status */
                rc = cio_get_stat(ca, openp, arg, devflag, chan);
                break;
        case CCC_GET_VPD:
        case CAT_GET_VPD:
                /* Return Vital Product Data information */
                rc = cio_get_vpd(ca, openp, arg, devflag, chan);
                break;
#ifdef DEBUG
        case DEBUG_SW:
                /* Set new debug level */
                cadebug = arg;
                break;
#endif /* DEBUG */
        case CAT_DEBUG:
              if(arg>=0x100) {   /* allow user to write cmd to ucode */
                /* wm3
                ** get parameter cmd_t structure from user, but
                ** do not copy in cmd_event or timer. Thats why
                ** I use sizeof(CTLFIFO) instead of cmd_t.
                ** If you user cmd_t size the cmd_event flag can
                ** get set and cause a DSI.
                */
                if (rc = COPYIN (devflag, (caddr_t)arg, &local_cmd, sizeof(CTLFIFO))) {
			unlockl(&ca->adap_lock);
                        return(EFAULT);
                }
                /* send cmd to ucode */
                if (cat_put_cmd(ca, &local_cmd))
                      rc = EIO;
                }
              else {
                  /* Set adapter trace level */
                  local_cmd.command = PSCACTRA;   /* PSCACDBA; */
                  local_cmd.cmdmod = 0;
                  if (arg >=0 && arg <=15)
                          local_cmd.data[0] = arg;
                  else
                          local_cmd.data[0] = 15;  /* Max debug level */
                  if (cat_put_cmd(ca, &local_cmd))
                            rc = EIO;
               }
                break;
        case CIO_START:
                /* Start a subchannel */
                rc = cio_start(ca, openp, arg, devflag, chan);
                break;
        case CIO_HALT:
                /* Stop a subchannel */
                rc = cio_halt(ca, openp, arg, devflag, chan);
                break;
        case CAT_DNLD:
                /* Download microcode to adapter */
                rc = do_dnld(ca, openp, arg, devflag);
                break;
        case CAT_SET_ADAP:
                /* Set Adapter Parameters */
                rc = do_setadap(ca, arg, devflag);
                break;
        case CAT_RESET_ADAP:
                /* Reset the adapter hardware */
                rc = reset_adapter(ca);
                break;
        case CAT_RESET_SUB:
                /* Reset a subchannel */
                rc = reset_sub(ca, openp, arg, devflag);
                break;
        case CAT_RESET_ALL:
                /* reset all subchannels on this adapter */
                rc = reset_all(ca);
                break;
        case CAT_CU_LOAD:
                /* Download a CU table */
                rc = load_cu(ca, arg, devflag);
                break;
        case CAT_POS_ACC:
                /* Access POS registers */
                rc = do_pos_cmd(ca, arg, devflag);
                break;
        case CAT_RW_SRAM:
                /* Read/Write Shared Data RAM */
                rc = rw_sram(ca, arg);
                break;
        case CAT_RUN_DIAG:
                /* Run a diagnostic test */
                rc = do_diag(ca, openp, arg, devflag);
                break;
        case CAT_ADAP_INFO:
                /* Read Adapter Settings */
                rc = adap_info(ca, arg);
                break;
        case CAT_ON_OFFLINE:
                /* Check or set online/offline status */
                rc = cat_on_offline(ca, arg, chan);
                break;
        case CIO_GET_FASTWRT:
                /* Return address of fast write function */
                rc = cat_getfastwrt(devno, arg, devflag);
                break;
        case CAT_SYNC_MODE:
                if (arg == CAT_SYNC_MODE_ON) {
                        openp->op_flags |= OP_SYNC_MODE;
                } else if (arg == CAT_SYNC_MODE_OFF) {
                        openp->op_flags &= ~OP_SYNC_MODE;
                } else {
                        rc = EINVAL;
                }
                break;
        case CAT_UNSOLICITED:
                rc = do_unst(ca, openp, arg);
                break;
        default:
                /* Invalid (unsupported) op */
                rc = EINVAL;
                break;
        } /* END SWITCH */

        DDHKWD1(HKWD_DD_CATDD, DD_EXIT_IOCTL, rc, devno);
        unlockl(&ca->adap_lock);
        return rc;
} /* catioctl() */


/*****************************************************************************
**
** NAME: reset_adapter
**
** FUNCTION:
**
** EXECUTION ENVIRONMENT:
**              process only
**
** NOTES:
**
** RETURNS:
**
*****************************************************************************/
int
reset_adapter(struct ca *ca)
{
        uchar reg;
        uchar posreg;
        ulong  iocc_a,mem;
	
        extern int pio_rc;
        ulong word;
		int  offset,io_iocc;	
        ulong old_level;


#ifdef _POWER_PC
		if ( __power_pc() ) {
				io_iocc = IO_IOCC_PPC;
				offset = OFFSET(iocc_ppc, c_reset_reg);
		}else {
#endif
				io_iocc = IO_IOCC;
				offset = OFFSET(iocc, c_reset_reg);
#ifdef _POWER_PC
		}
#endif

        DISABLE_INTERRUPTS(old_level);
        iocc_a = (ulong )IOCC_ATT(ca->caddp.bus_id, io_iocc);   /* access iocc */
#ifdef _POWER_MP
	if ( __power_mp() )
		word = 0xff000007; /* Add kludge to get around SMP problems */
	else
#endif /* _POWER_MP */
        	word = BUSIO_GETL(iocc_a + offset );

        word &= ~(1 << (31-SLOT));
        BUSIO_PUTL(iocc_a + offset , word);  /* Turn on MC reset fo */
		IOCC_DET(iocc_a);
        ENABLE_INTERRUPTS(old_level);
        delay(HZ/10);                   /* hold reset for a bit */
        DISABLE_INTERRUPTS(old_level);
        iocc_a = (ulong)IOCC_ATT(ca->caddp.bus_id, io_iocc);   /* access iocc */

#ifdef _POWER_MP
	if ( __power_mp() )
		word |= (1 << (31-SLOT));
	else
#endif /* _POWER_MP */
        	word = BUSIO_GETL(iocc_a + offset) | (1 << (31-SLOT));

        BUSIO_PUTL(iocc_a + offset, word);  /* Turn off microchanne l reset */
        IOCC_DET(iocc_a);                         /* release access to iocc */
        ENABLE_INTERRUPTS(old_level);


        cat_set_pos (ca);               /* this resets the shared RAM map */

        DISABLE_INTERRUPTS(old_level);
        iocc_a = CAT_IOCC_ATT;
        CAT_POS_READ(iocc_a, 4, &posreg); /* reset the on-board processor */
        CAT_POS_WRITE(iocc_a, 4, (posreg|POS_RESET));
		ENABLE_INTERRUPTS(old_level);
        delay(5);       /* hold reset for 50 millisec (arbitrary value) */
        DISABLE_INTERRUPTS(old_level);
        /* CAT_WRITE sets ca->piorc */
        CAT_POS_WRITE(iocc_a, 4, posreg);
        if (ca->piorc) {
                IOCC_DET(iocc_a);
                ENABLE_INTERRUPTS(old_level);
                cat_shutdown(ca);
                return EIO;
        }
        IOCC_DET(iocc_a);
        ENABLE_INTERRUPTS(old_level);
        return 0;
} /* reset_adapter() */


/*****************************************************************************
**
** NAME: do_dnld
**
** FUNCTION: loads microcode to the PCA
**
** EXECUTION ENVIRONMENT:
**              process only
**
** NOTES:
**
**    Input:    pointer to dds structure
**                      pointer to open_structure
**                      pionter to download structure
**
**    Called From: catioctl
**
**    Calls To:
**              disable, enable, CAT_POS_READ, CAT_WRITE, BUSMEM_DET, CAT_POS_WRITE,
**              IOCC_DET, cat_wait_sram, cat_user_write, letni32
**
** This function downloads code to the board by performing these steps
**      1.      Reset the board
**      2.      Wait for OPERLVL to contain DL_WAITING
**      3.      Write ucode to SRAM
**      4.      Write DL_CMD to OPERLVL
**      5.      Wait for OPERLVL to contain DL_CRC or DL_CONFIRM
**              if DL_CRC then failure else success
**
**
** RETURN:      0 = Good Return
**              EIO -- download timed out or test failed.
**
**
*****************************************************************************/
#define HANDLE_BUS_ERROR() \
        { \
                BUSMEM_DET(mem); \
                ENABLE_INTERRUPTS(spl); \
                cat_shutdown(ca); \
                dnld_str->status = CIO_HARD_FAIL; \
                rc = ca->piorc; \
                goto do_dnld_exit; \
        }

int
do_dnld(
        struct ca *ca,
        open_t *openp,
        int arg,
        ulong devflag)
{
        uchar reg = 0;          /* current OPERLVL value */
        ulong mem;
        int waittime;
        int len;
        int spl;
        int rc = 0;
        struct cat_dnld dnld_cmd;
        struct cat_dnld *dnld_str = & dnld_cmd;

        if (!(ca->flags & CATDIAG)) {
                return EACCES;
        }
        if (COPYIN(devflag, (caddr_t)arg, &dnld_cmd, sizeof(dnld_cmd))) {
                return EFAULT;          /* bad addr */
        }

        ca->flags &= ~(CATFUNC | CATSETUP | CATABLE);

        /*
        ** Reset the adapter via Component Reset Register and POS regs.
        */
        reset_adapter(ca);

        /*
        ** Clear the adapter's OPERLVL and READY registers
        */
        DISABLE_INTERRUPTS(spl);
        mem = CAT_MEM_ATT;
        /* CAT_WRITE sets ca->piorc */
        CAT_WRITE1(mem, OPERLVL, &reg);
        if (ca->piorc) {
                HANDLE_BUS_ERROR();
        }
        /* CAT_WRITE sets ca->piorc */
        CAT_WRITE1(mem, READY, &reg);
        if (ca->piorc) {
                HANDLE_BUS_ERROR();
        }
        BUSMEM_DET(mem);
        ENABLE_INTERRUPTS(spl);

        /*
        ** wait for OPERLVL == DL_WAITING
        ** This indicates POST has completed.
        ** THIS WILL SLEEP
        */
        waittime = HZ * 20;             /* time for POST to complete */
        if ((rc=cat_wait_sram(ca, openp, OPERLVL, reg, waittime, &reg)) == -1) {
                dnld_str->status = CIO_TIMEOUT;
                cat_logerr(ca, ERRID_CAT_ERR2);
                rc = EIO;
                goto do_dnld_exit;
        } else if (rc || (reg != DL_WAITING)) {
                dnld_str->status = CIO_HARD_FAIL;
                cat_logerr(ca, ERRID_CAT_ERR2);
                goto do_dnld_exit;
        }

        /*
        ** Check ERRCODE to make sure POST didn't complete with an error.
        */
        DISABLE_INTERRUPTS(spl);
        mem = CAT_MEM_ATT;
        CAT_READ1(mem, ERRCODE, &reg);
        if (ca->piorc) {
                HANDLE_BUS_ERROR();
        }
        BUSMEM_DET(mem);
        ENABLE_INTERRUPTS(spl);
        if (reg) {
                dnld_str->status = CIO_HARD_FAIL;
                cat_logerr(ca, ERRID_CAT_ERR5);
                rc = EIO;
                goto do_dnld_exit;
        }

        /*
        ** We must use PIO for downloading code since there
        ** isn't any code to tell me when Terminal Count occurs.
        */
        if (rc = cat_user_write(ca, dnld_str->mcode_ptr, DATABUFS,
                                CAT_USE_PIO, dnld_str->mcode_len)) {
                dnld_str->status = CIO_HARD_FAIL;
                goto do_dnld_exit;
        }

        len = dnld_str->mcode_len;
        letni32(&len);
        reg = DL_CMD;                   /* signal that ucode is in SRAM */
        DISABLE_INTERRUPTS(spl);
        mem = CAT_MEM_ATT;
        /*
        ** put ucode length in SRAM
        */
        /* CAT_WRITE sets ca->piorc */
        CAT_WRITE(mem, 0, &len, sizeof(len));
        if (ca->piorc) {
                HANDLE_BUS_ERROR();
        }
        /*
        ** Tell board to copy sram->dram
        */
        /* CAT_WRITE sets ca->piorc */
        CAT_WRITE1(mem, OPERLVL, &reg);
        if (ca->piorc) {
                HANDLE_BUS_ERROR();
        }
        BUSMEM_DET(mem);                /* release access to MCI bus */
        ENABLE_INTERRUPTS(spl);

        /*
        ** Wait for indication of successful download.
        */
        waittime = HZ * 10;     /* time for board to copy and check ucode */
        reg = 0;
        if ((rc = cat_wait_sram(ca, openp, READY, reg, waittime, &reg)) == -1) {
                dnld_str->status = CIO_TIMEOUT;
                cat_logerr(ca, ERRID_CAT_ERR2);
                rc = EIO;
                goto do_dnld_exit;
        } else if (rc || (reg != DL_WAITING)) {
                dnld_str->status = CIO_HARD_FAIL;
                cat_logerr(ca, ERRID_CAT_ERR2);
                goto do_dnld_exit;
        }

        /*
        ** Check adapter status
        */
        if (rc = cat_check_status(ca)) {
                cat_logerr(ca, ERRID_CAT_ERR2);
                goto do_dnld_exit;
        }

        /*
        ** Indicate that MCI wants interrupts.
        */
        reg = 1;
        DISABLE_INTERRUPTS(spl);
        mem = CAT_MEM_ATT;
        /* CAT_WRITE sets ca->piorc */
        CAT_WRITE1(mem, INTMCI, &reg);
        if (ca->piorc) {
                HANDLE_BUS_ERROR();
        }

        /*
        ** Reset card generation of interrupts.
        */
        reg = 0;
        /* CAT_WRITE sets ca->piorc */
        CAT_WRITE1(mem, RSTUCIRQ, &reg);
        if (ca->piorc) {
                HANDLE_BUS_ERROR();
        }
        BUSMEM_DET(mem);                /* release access to MCI bus */
        ENABLE_INTERRUPTS(spl);

        dnld_str->status = DNLD_SUCC;
        ca->flags &= ~CATDEAD;
do_dnld_exit:
        COPYOUT(devflag,&dnld_cmd, (caddr_t)arg, sizeof(dnld_cmd));
        return rc;
} /* do_dnld() */


/*****************************************************************************
** NAME: cio_start()
**
** FUNCTION:
** This function handles the CIO_START ioctl.  It sets up and starts an
** adapter subchannel or subchannel group.
**
** EXECUTION ENVIRONMENT:       process thread only
**
** INPUT:       pointer to the adapter structure
**                      pointer to the open structure
**                      address of the user process' argument structure
**                      the mode flags for this open
**                      the channel number or the minor device's minor number
**
** OUTPUT:      see notes for catioctl
**
** CALLERS:     catioctl
**
** CALLEES:     COPYIN(), COPYOUT(), xmalloc(), do_setsub()
**
** RETURNS:     0 ---------- successful completion
**              EINVAL ----- an invalid subchannel number or range was specified
**              EADDRINUSE - a subchannel was specified that is currently in use
**              ENOMEM ----- a subchannel structure could not be allocated
**              EAGAIN ----- the required PSCA resources could not be allocated
**              EFAULT ----- COPYIN() or COPYOUT() function failed
**              EIO -------- a PIO operation failed
*****************************************************************************/
int
cio_start(
        struct ca *ca,
        open_t *openp,
        int arg,
        ulong devflag,
        chan_t chan)
{
        struct subchannel *sc = NULL;
        struct subchannel *new_sc;
        struct cat_set_sub sess_blk;
        int i;
        int lastsc;
        int rc = 0;

        /* Microcode must be functional */
        if (!(ca->flags & CATFUNC)) {
                return EIO;
        }

        /* Has Customer Engineer allowed starts? */
        if (ca->flags & CAT_NOSTARTS) {
                return EBUSY;
        }

        /*
        ** get parameter block
        */
        if (rc = COPYIN (devflag, (caddr_t)arg, &sess_blk, sizeof(sess_blk))) {
                DDHKWD2(HKWD_DD_CATDD,DD_ENTRY_START,0,ca->dev,chan);
                DDHKWD1(HKWD_DD_CATDD,DD_EXIT_START,rc,ca->dev);
                return(EFAULT);
        }

/* ix29279 */
/* If the subset field is not set then to allow the subchannel counter
   variables to work properly set them to 2 for claw and 1 for non-claw.
   The subset field only maters in NON-CLAW mode since it allows for starting
   of multiple subchannels on a given start request.  In CLAW 2 subchannels
   are started always, multiple pairs of subchannels can not be started on 
   one start IOCTL.
*/
	   if ( sess_blk.subset == 0 ) {
		if (sess_blk.specmode & CAT_CLAW_MOD )
			 sess_blk.subset = 2;
	     else
			sess_blk.subset = 1;
	   }

        /*
        ** This is kind of awkward, but netid should be part of the ENTRY
        ** trace and we don't have it until after the COPYIN.
        */
        DDHKWD5(HKWD_DD_CATDD,DD_ENTRY_START,0,ca->dev,chan,sess_blk.sb.netid,
                        sess_blk.sb.netid,sess_blk.subset);

        /*
        ** We copy out the session block with a successful completion
        ** status here to avoid the overhead of halting the subchannels
        ** if the COPYOUT function fails.  This would have to be done
        ** if we waited until the subchannels had been started before
        ** finding out if the COPYOUT would fail.
        */
        sess_blk.sb.status = CIO_OK;
        if (COPYOUT(devflag, &sess_blk, (caddr_t)arg, sizeof(sess_blk)) != 0) {
                DDHKWD2(HKWD_DD_CATDD,DD_EXIT_START,EFAULT,ca->dev,chan);
                return(EFAULT);
        }

        /*
        ** Handle CLAW mode differently: allow multiple starts per SC (links)
        */
        if (sess_blk.specmode & CAT_CLAW_MOD) {
                int err_code;

                if (err_code = sess_start(ca, openp, &sess_blk, chan)) {
                        COPYOUT(devflag, &sess_blk, (caddr_t)arg, sizeof(sess_blk));
                        async_status(ca, openp, CIO_START_DONE, CIO_NOT_STARTED,
                                sess_blk.sb.netid, sess_blk.claw_blk.linkid, 0);
                }
                DDHKWD2(HKWD_DD_CATDD,DD_EXIT_START,err_code,ca->dev,chan);
                return err_code;
        }

        /*
        ** Validate subchannel range
        */
        if ((sess_blk.sb.netid + sess_blk.subset-1) > MAX_SUBCHAN) {
                DDHKWD1(HKWD_DD_CATDD, DD_EXIT_START, rc, ca->dev);
                return EINVAL;
        }

        /*
        ** Check for duplicate netids
        */
        lastsc = sess_blk.sb.netid + sess_blk.subset;
        for (i = sess_blk.sb.netid ; i < lastsc ; i++) {
                if ((sc = ca->sc[i]) != NULL) {
                    /*  ix31884
                     * new state flag indicates that the
                     * subchannel should be delted
                     * since it was stopped
                     * should be already deleted unless
                     * an error occured then we have to delete it
                     */
                    if ( sc->sc_state == SC_DELETE ) {
				   #ifdef DEBUG
                        cat_gen_trace("SCDL",0,0,0);
				   #endif
                        free_sc_links(ca,sc);
                    } else {
                        sess_blk.sb.status = CIO_NETID_DUP;
                        /*
                        ** netid was a duplicate
                        */
                        rc = COPYOUT(devflag, &sess_blk, (caddr_t)arg, sizeof(sess_blk));
                        if (rc) {
                                DDHKWD1(HKWD_DD_CATDD,DD_EXIT_START,rc,ca->dev);
                                return rc;
                        } else {
                                DDHKWD1(HKWD_DD_CATDD, DD_EXIT_START,
                                        EADDRINUSE, ca->dev);
                                return EADDRINUSE;
                        }
                    }  /* ix31884 SC_DELETE end  */
                }
        }

        /*
        ** Allocate the subchannel structures, if necessary.
        */
        for (i=sess_blk.sb.netid ; i<lastsc ; i++) {
                /*
                ** First, get a chunk of memory for this subchannel structure.
                ** If this subchannel has been used before, we should already
                ** have memory allocated for it.
                */
                if (ca->sc[i] == NULL) {
                        ca->sc[i] = KMALLOC(subchannel_t);
                        if (ca->sc[i] == NULL) {
                                DDHKWD1(HKWD_DD_CATDD,
                                        DD_EXIT_START, ENOMEM, ca->dev);
                                /* if any malloc fails free entire group */
/* d51658 */                    free_sc_links(ca, ca->sc[sess_blk.sb.netid]);
                                return ENOMEM;
                        }
                }
                bzero(ca->sc[i], sizeof(subchannel_t));
                /*
                ** Now, initialize the structure.
                */
                new_sc = ca->sc[i];
                new_sc->sc_id = i;
                new_sc->specmode = sess_blk.specmode;
                new_sc->shrtbusy = sess_blk.shrtbusy;
                new_sc->startde  = sess_blk.startde;
                new_sc->sc_group = sess_blk.sb.netid;
                new_sc->sc_subset= sess_blk.subset;
                new_sc->sc_state = SC_CLOSED;
		
#ifndef AIXV3
                new_sc->sc_stop_ack = EVENT_NULL ; /* d51376 changed from EVENT_
NULL; */
#else
                new_sc->sc_stop_ack = 0 ; /* d51376 changed from EVENT_NULL; */
#endif

			 sc = new_sc;   /* ix32805  set sc pointer in case of error  */
						 /* in do_setsub.  Won't DSI system           */  


                /* table of 1 link */
                if (((new_sc->links = KMALLOC(link_t *)) == NULL)
                        || ((new_sc->links[0]= KMALLOC(link_t)) == NULL)) {
                        DDHKWD1(HKWD_DD_CATDD,DD_EXIT_START,ENOMEM,ca->dev);
                        /* if any malloc fails free entire group */
/*d51658 */             free_sc_links(ca, ca->sc[sess_blk.sb.netid]);
                        return ENOMEM;
                }
                new_sc->num_links = 1;

                /* Set the quick access pointer */
                new_sc->link[0] = new_sc->links[0];

                /* for 3088 mode the link is always active */
                bzero(new_sc->links[0], sizeof(link_t));
                new_sc->links[0]->lk_open = openp;
                new_sc->links[0]->lk_sc_id = new_sc->sc_id;
                new_sc->links[0]->lk_actual_id = 0;
                new_sc->links[0]->lk_appl_id = 0;

                if (openp->op_default == NULL) {
                        openp->op_default = new_sc;
                }
        }

        /*
        ** Call do_setsub() to set the subchannels parameters
        ** and start the subchannel.  If the subchannel starts
        ** the list of subchannel members will be linked into
        ** the open structures list of subchannels and the number
        ** of subchannels started for this open and for the adapter
        ** will be incremented by the offlevel interrupt handler
        ** clean_queue().
        */
        if (rc = do_setsub(ca, &sess_blk)) {
                sess_blk.sb.status = rc;
                sc->sc_state = SC_CLOSED;
                SET_GROUP(sess_blk.sb.netid, SC_CLOSED);
                COPYOUT(devflag, &sess_blk, (caddr_t)arg, sizeof (sess_blk));
                DDHKWD1(HKWD_DD_CATDD,DD_EXIT_START,rc,ca->dev);
                /*
                ** do_setsub() or err_sets_strt_stop() will
                ** free_sc_links(ca, sc) on any errors in start process.
                ** free_sc_links(ca, sc) will reset openp->op_default.
                */
                return rc;
        }

        /*
        ** The session block was copied out before starting the
        ** subchannels, so all we do here is return.
        */
        DDHKWD1(HKWD_DD_CATDD, DD_EXIT_START, rc, ca->dev);
        return 0;
} /* cio_start() */


/*********************************************************************
** NAME: cio_halt()
**
** FUNCTION:
** This function handles the CIO_HALT ioctl. It will halt a previously
** started subchannel or subchannel group.
**
** EXECUTION ENVIRONMENT: process thread only
**
** INPUT:       pointer to the adapter structure
**                      pointer to the open structure
**                      address of the user process' argument structure
**                      the mode flags for this open
**                      the channel number or the minor device's minor number
**
** OUTPUT:      see notes for catioctl
**
** CALLED BY:   catioctl
**
** CALLS:       COPYIN(), COPYOUT(), do_stopsub(), DISABLE_INTERRUPTS(),
**                      ENABLE_INTERRUPTS()
**
** RETURNS:     0 ------ successful completion
**              EINVAL - invalid subchannel number
**              ENOENT - subchannel is in an invalid state (not open)
**              EPERM -- user is not superuser and does not own the subchannel
**              EFAULT - COPYIN() or COPYOUT() failed
**********************************************************************/
int
cio_halt(
        struct ca *ca,
        open_t *openp,
        int arg,
        ulong devflag,
        chan_t chan)
{
        struct subchannel *sc;
        struct cat_set_sub sess_blk;
        int i;
        int spl;
        int rc = 0;

        if (!(ca->flags & CATFUNC)) {
                return EIO;
        }

        if (COPYIN(devflag, (caddr_t)arg, &sess_blk, sizeof(sess_blk))) {
                DDHKWD3(HKWD_DD_CATDD,DD_ENTRY_HALT, 0, 0, 0, 0);
                DDHKWD1(HKWD_DD_CATDD, DD_EXIT_HALT, rc, ca->dev);
                return EFAULT;
        }

        /*
        ** This is kind of awkward, but netid should be part of the ENTRY
        ** trace and we don't have it until after the COPYIN.
        */
        DDHKWD3(HKWD_DD_CATDD,DD_ENTRY_HALT,0,ca->dev,chan,sess_blk.sb.netid);

        /*
        ** Validate the subchannel number
        */
        if (sess_blk.sb.netid > MAX_SUBCHAN) {
                sess_blk.sb.status = CIO_NETID_INV;
                COPYOUT (devflag, &sess_blk, (caddr_t)arg, sizeof(sess_blk));
                DDHKWD1(HKWD_DD_CATDD, DD_EXIT_HALT, EINVAL, ca->dev);
                return EINVAL;
        }

        /*
        ** Verify that the subchannel exists.
        */
        if ((sc = ca->sc[sess_blk.sb.netid]) == NULL) {
                sess_blk.sb.status = CIO_NOT_STARTED;
                COPYOUT (devflag, &sess_blk, (caddr_t)arg, sizeof(sess_blk));
                DDHKWD1(HKWD_DD_CATDD, DD_EXIT_HALT, ENOENT, ca->dev);
                return ENOENT;
        }

        /*
        ** Just return if the SC is already closed.
        */
        if (sc->sc_state == SC_CLOSED) {
                DDHKWD1(HKWD_DD_CATDD, DD_EXIT_HALT, 0, ca->dev);
                return 0;
        }

        /*
        ** Handle CLAW mode SC close.
        */
        if (sc->specmode & CAT_CLAW_MOD) {
                /*
                ** If no current links, just stop the SC.
                */
                if (sc->num_links == 0) {
                        if (rc = do_stopsub(ca, &sess_blk)) {
                                COPYOUT(devflag, &sess_blk, (caddr_t)arg, sizeof(sess_blk));
                                DDHKWD1(HKWD_DD_CATDD,DD_EXIT_HALT,rc,ca->dev);
                                async_status(ca, openp, CIO_HALT_DONE, CIO_NETID_INV,
                                        sess_blk.sb.netid, sess_blk.claw_blk.linkid, 0);
                                return rc;
                        }
                        async_status(ca, openp, CIO_HALT_DONE, CIO_OK,
                                sess_blk.sb.netid, sess_blk.claw_blk.linkid, 0);
                        COPYOUT (devflag, &sess_blk, (caddr_t)arg, sizeof(sess_blk));
                        DDHKWD1(HKWD_DD_CATDD,DD_EXIT_HALT,rc,ca->dev);
                        return rc;
                }
                /*
                ** Disconnect all links associated with this open.
                */
                if (rc = sess_disc(ca, openp, &sess_blk, chan)) {
                        async_status(ca, openp, CIO_HALT_DONE, CIO_NETID_INV,
                                sess_blk.sb.netid, sess_blk.claw_blk.linkid, 0);
                } else if (sc->num_links == 0) {
                        if (rc = do_stopsub(ca, &sess_blk)) {
                                COPYOUT(devflag, &sess_blk, (caddr_t)arg, sizeof(sess_blk));
                                DDHKWD1(HKWD_DD_CATDD,DD_EXIT_HALT,rc,ca->dev);
                                async_status(ca, openp, CIO_HALT_DONE, CIO_NETID_INV,
                                        sess_blk.sb.netid, 0, 0);
                                return rc;
                        }
                        async_status(ca, openp, CIO_HALT_DONE, CIO_OK,
                                sess_blk.sb.netid, sess_blk.claw_blk.linkid, 0);
                } else {
                        async_status(ca, openp, CIO_HALT_DONE, CIO_OK,
                                sess_blk.sb.netid, sess_blk.claw_blk.linkid, 0);
                }
                COPYOUT (devflag, &sess_blk, (caddr_t)arg, sizeof(sess_blk));
                DDHKWD1(HKWD_DD_CATDD,DD_EXIT_HALT,rc,ca->dev);
                return rc;
        }

        /*
        **  Verify the caller's access priviledge
        */
        if (openp != sc->links[0]->lk_open && getuidx(ID_EFFECTIVE)) {
                DDHKWD1(HKWD_DD_CATDD, DD_EXIT_HALT, EPERM, ca->dev);
                return EPERM;
        }

        /*
        ** We copy out the session block with a successful completion
        ** status here to avoid the overhead of restarting the subchannels
        ** if the COPYOUT function fails.  This would have to be done
        ** if we waited until the subchannels had been halted before
        ** finding out if the COPYOUT would fail.
        */
        sess_blk.sb.status = 0;
        if (COPYOUT(devflag, &sess_blk, (caddr_t)arg, sizeof(sess_blk)) != 0) {
                DDHKWD1(HKWD_DD_CATDD, DD_EXIT_HALT, EFAULT, ca->dev);
                return EFAULT;
        }

        if (rc = do_stopsub(ca, &sess_blk)) {
                COPYOUT(devflag, &sess_blk, (caddr_t)arg, sizeof(sess_blk));
                DDHKWD1(HKWD_DD_CATDD,DD_EXIT_HALT,rc,ca->dev);
                async_status(ca, openp, CIO_HALT_DONE, CIO_NETID_INV, sess_blk.sb.netid, 0, 0);
                return rc;
        }
        /*
        ** If this group includes this open's default subchannel
        ** then reestablish a default.
        */
        for (i = 0; i < MAX_SUBCHAN; i++) {
                if (openp->op_default == ca->sc[i]) {
                        /*
                        ** Reusing the index variable 'i' should be OK
                        ** here since we are going to break out of the
                        ** enclosing 'for' after this loop.
                        */
                        for (i=0; i<MAX_SUBCHAN; i++) {
                                if ((sc = ca->sc[i]) != NULL
                                        && (sc->links[0]->lk_open == openp)
                                        && (ca->sc[i]->sc_group
                                        != ca->sc[sess_blk.sb.netid]->sc_group)) {
                                        openp->op_default = sc;
                                }
                        }
                        break;
                }
        }

        async_status(ca, openp, CIO_HALT_DONE, CIO_OK, sess_blk.sb.netid, 0, 0);
        DDHKWD1(HKWD_DD_CATDD, DD_EXIT_HALT, 0, ca->dev);
        return 0;
} /* cio_halt() */


/*********************************************************************
** NAME: cio_query()
**
** FUNCTION:
** This function handles the CIO_QUERY ioctl.  The statistics data
** collected by this driver is returned to the user process.
**
** EXECUTION ENVIRONMENT: process thread only
**
** INPUT:       pointer to the adapter structure
**                      pointer to the open structure
**                      address of the user process' argument structure
**                      the mode flags for this open
**                      the channel number or the minor device's minor number
**
** OUTPUT:      see notes for catioctl
**
** CALLERS:     catioctl
**
** CALLEES:     COPYIN(), COPYOUT(), bzero(), ENABLE_INTERRUPTS(),
**                      DISABLE_INTERRUPTS()
**
** RETURNS: 0 ------ successful completion
**                      EFAULT - COPYIN() or COPYOUT failed
**********************************************************************/
int
cio_query(
        struct ca *ca,
        open_t *openp,
        int arg,
        ulong devflag,
        chan_t chan)
{
        cio_query_blk_t qparms;
        int spl;

        DDHKWD2(HKWD_DD_CATDD, DD_ENTRY_QUERY, 0, ca->dev, chan);

        /*
        ** get the caller's parameters
        */
        if (COPYIN (devflag, (caddr_t)arg, &qparms, sizeof(qparms))) {
                DDHKWD1(HKWD_DD_CATDD,DD_EXIT_QUERY,EFAULT,ca->dev);
                return EFAULT;
        }

        /*
        ** Check for data truncation.
        */
        if (qparms.buflen < sizeof(cat_query_t)) {
                qparms.status = CIO_BUF_OVFLW;
                DDHKWD1(HKWD_DD_CATDD,DD_EXIT_QUERY,EIO,ca->dev);
                COPYOUT (devflag, &qparms, (caddr_t)arg, sizeof(qparms));
                return EIO;
        }
        qparms.status = CIO_OK;

        /*
        ** Copy data to caller's buffer and clear the
        ** RAS area if indicated.
        */
        DISABLE_INTERRUPTS(spl);
        if (COPYOUT(devflag, &ca->stats, qparms.bufptr,
                MIN(qparms.buflen, sizeof(cat_query_t))))  {
                DDHKWD1(HKWD_DD_CATDD,DD_EXIT_QUERY,EFAULT,ca->dev);
                ENABLE_INTERRUPTS(spl);
                return EFAULT;
        }
        if (qparms.clearall == CIO_QUERY_CLEAR)
                bzero(&ca->stats, sizeof(cat_query_t));
        ENABLE_INTERRUPTS(spl);

        /*
        ** return parameter block to caller
        */
        if (COPYOUT(devflag, &qparms, (caddr_t)arg, sizeof(qparms))) {
                DDHKWD1(HKWD_DD_CATDD,DD_EXIT_QUERY,EFAULT,ca->dev);
                return EFAULT;
        }

        DDHKWD1(HKWD_DD_CATDD,DD_EXIT_QUERY,0,ca->dev);
        return 0;
} /* cio_query() */


/*************************************************************************
** NAME: cio_get_stat()
**
** FUNCTION:
**      This function handles the CIO_GET_STAT ioctl.
**
** EXECUTION ENVIRONMENT:
**
**      This routine executes on the process thread.
**
** NOTES:
**
**      Called from:  catioctl
**
**  Calls: copyout, enable, disable
**
** RETURNS:
**      0 - success
**      EFAULT - copyout() failed
**      EACCESS - called by a kernel process
**************************************************************************/
int
cio_get_stat(
        struct ca *ca,
        open_t *openp,
        int arg,
        ulong devflag,
        chan_t chan)
{
        cio_stat_blk_t stat_blk;
        stat_elem_t *statp;
        stat_elem_t *tp;
        int spl;
        int rc = 0;

        DDHKWD2(HKWD_DD_CATDD, DD_ENTRY_GET_STAT, 0, ca->dev, chan);

        /*
        ** Calling from a kernel process is not allowed
        */
        if (devflag & DKERNEL) {
                DDHKWD1(HKWD_DD_CATDD, DD_EXIT_GET_STAT, EACCES, ca->dev);
                return EACCES;
        }

        DISABLE_INTERRUPTS(spl);

        /*
        ** Check the "active" list
        */
        if ((statp = openp->stat_act) == NULL) {
                stat_blk.code = (ulong)CIO_NULL_BLK;
                if (COPYOUT(devflag, &stat_blk, (caddr_t)arg, sizeof(stat_blk))) {
                        rc = EFAULT;
                }
        } else {
                if (statp->stat_blk.code == CIO_LOST_STATUS) {
                        openp->op_flags &= ~(LOST_STATUS | SENT_LOST_STATUS);
                }
                stat_blk = statp->stat_blk;
                if (COPYOUT(devflag, &stat_blk, (caddr_t)arg, sizeof(stat_blk))) {
                        rc = EFAULT;
                }
                /*
                ** Take the element from the "active" list.
                */
                if (statp->stat_next != NULL) {
                        statp->stat_next->stat_last = statp->stat_last;
                }
                openp->stat_act = statp->stat_next;
                statp->stat_next = NULL;
                statp->stat_last = statp;

                /*
                ** Add this buffer to the "free" list.
                */
                if (openp->stat_free == NULL) {
                        openp->stat_free = statp;
                        if ((ca->flags & CAT_PAUSED)
                        && (openp->op_flags & OP_SYNC_MODE)) {
                                /*
                                ** If the adapter was on hold,
                                ** start it up again.
                                */
                                ca->flags &= ~CAT_PAUSED;
                                dma_request(ca, 0);
                                i_sched(&(ca->ofl));
                        }
                } else {
                        statp->stat_last = openp->stat_free->stat_last;
                        statp->stat_last->stat_next = statp;
                        openp->stat_free->stat_last = statp;
                }

                if ((openp->op_flags & LOST_STATUS)
                        && ((openp->op_flags & SENT_LOST_STATUS) == 0)) {
                        async_status(ca, openp, CIO_LOST_STATUS, 0, -1, -1, 0);
                        openp->op_flags |= SENT_LOST_STATUS;
                }
        }

        ENABLE_INTERRUPTS(spl);

        DDHKWD1(HKWD_DD_CATDD, DD_EXIT_GET_STAT, rc, ca->dev);
        return rc;
} /* cio_get_stat() */


/*****************************************************************************
**
** NAME: cio_get_vpd
**
** FUNCTION:
**
** EXECUTION ENVIRONMENT:
**              process only
**
** NOTES:
**
** RETURNS:
**
*****************************************************************************/
int
cio_get_vpd (
        struct ca *ca,
        open_t *openp,
        cat_vpd_t *arg,
        ulong devflag,
        chan_t chan)
{
        cat_vpd_t tmp_vpd;
        int rc = 0;

        if (tmp_vpd.length = ca->vpd_length) {
                tmp_vpd.status = CAT_VPD_VALID;
                bcopy(ca->vpd, tmp_vpd.vpd, ca->vpd_length);
        } else  {
                tmp_vpd.status = CAT_VPD_INVALID;
        }

        if (COPYOUT(devflag, &tmp_vpd, arg, sizeof(cat_vpd_t))) {
                return EFAULT;
        }

        return 0;
} /* cio_get_vpd() */


/*****************************************************************************
**
** NAME: catinfo
**
** FUNCTION:
**
** EXECUTION ENVIRONMENT:
**              process only
**
** NOTES:
**
** RETURNS:
**
*****************************************************************************/
int
catinfo(
        struct ca *ca,
        caddr_t arg,
        ulong devflag)
{
        struct devinfo info;
        int rc = 0;

        /*
        ** check pointer
        */
        if (arg == NULL)
                return(EFAULT);

        if (ca->flags & CATFUNC) {
                bcopy(ca->status.hardname,
                        &info.un.pca.hi_adap_name,
                        sizeof(info.un.pca.hi_adap_name));
                bcopy((ulong)ca->status.hardname
                        + sizeof(info.un.pca.hi_adap_name),
                        &info.un.pca.lo_adap_name,
                        sizeof(info.un.pca.lo_adap_name));
                bcopy(ca->status.softname,
                        &info.un.pca.hi_ucode_name,
                        sizeof(info.un.pca.hi_ucode_name));
                bcopy((ulong)ca->status.softname
                        + sizeof(info.un.pca.hi_ucode_name),
                        &info.un.pca.lo_ucode_name,
                        sizeof(info.un.pca.lo_ucode_name));
                info.un.pca.chnl_speed = ca->status.chanspd;
        }
        else {
                info.un.pca.hi_adap_name = 0;
                info.un.pca.lo_adap_name = 0;
                info.un.pca.hi_ucode_name = 0;
                info.un.pca.lo_ucode_name = 0;
                info.un.pca.chnl_speed = 0;
        }
        info.un.pca.rdto = ca->caddp.rdto;
        info.un.pca.broad_wrap = 0;
        info.devtype = DD_CAT;
        info.devsubtype = DS_CAT;

        /*
        **  The structure is then returned to the user by copying it to
        **  the proper user or kernel space.
        */
        rc = COPYOUT(devflag, &info, arg, sizeof(info));
        return rc;
} /* catinfo() */


/*****************************************************************************
**
** NAME: do_sram_cmd
**
** FUNCTION: perform a read or write to shared ram on the PCA.
**
** EXECUTION ENVIRONMENT:    process
**
** NOTES:
**
**    Input:  pointer to dds struct
**               pointer to io structure
**    Output:
**              see catioctl notes
**
**    Called From: catioctl
**
**    Calls To: cat_user_write, cat_user_read
**
** Note that cmd->data is a user address and must be handled as such.
**
** RETURN:  0 = Good Return
**
**
*****************************************************************************/
int
do_sram_cmd(
        register struct ca *ca,
        struct cat_rw_sram *cmd)
{
        int rc;

        switch (cmd->rd_wrt) {
        case CAT_CHNL_WR:
                rc = cat_user_write(ca, cmd->str_ptr,
                        cmd->sram_offset, CAT_USE_PIO, cmd->num_bytes);
                break;
        case CAT_CHNL_RD:
                rc = cat_user_read(ca, cmd->str_ptr, cmd->sram_offset,
                        CAT_USE_PIO, cmd->num_bytes);
                break;
        case CAT_CHNL_RD_NO_FAIL:
                rc = cat_user_read(ca, cmd->str_ptr, cmd->sram_offset,
                        CAT_USE_PIO | NO_FAIL, cmd->num_bytes);
                break;
        case CAT_CHNL_WR_NO_FAIL:
                rc = cat_user_write(ca, cmd->str_ptr,
                        cmd->sram_offset, CAT_USE_PIO|NO_FAIL, cmd->num_bytes);
                break;
        default:                                /* invalid opcode */
                rc = EINVAL;
        }
        cmd->status = rc;       /* # of pio retries (not well planned) */
        return rc;
} /* do_sram_cmd() */



/*****************************************************************************
**
** NAME: do_pos_cmd
**
** FUNCTION: reads or writes the pos registers on the PCA
**
** EXECUTION ENVIRONMENT:  process
**
** NOTES:
**
**    Input:    pointer to dds structure
**              pointer to pos access structure
**
**    Output:  see catioctl notes
**
**    Called From: catioctl
**
**    Calls To: IOCC_ATT,CAT_POS_WRITE, CAT_POS_READ, IOCC_DET
**
**
*****************************************************************************/
int
do_pos_cmd(
        struct ca *ca,
        struct cat_pos_acc *cmd)
{
        int rc = 0;
        ulong bus;

        extern ulong cat_addr_map[];
#define ADDR_BITS(x)    (((x) & 0xf0) >> 4)

        if (!(ca->flags & CATDIAG))
                return EACCES;

        if (cmd->pos_reg > 7) {         /* pos_reg is unsigned therefore >= 0 */
                cmd->status = CIO_BAD_RANGE;
                return(EINVAL);
        }
        if (!(ca->flags & CATDIAG))
                return EINVAL;

        cmd->status = READ_DONE;
        bus = CAT_IOCC_ATT;
        switch (cmd->opcode) {
        case CAT_CHNL_WR:
                CAT_POS_WRITE(bus, cmd->pos_reg, cmd->pos_val);
                /*
                ** if pos register 2 was changed, the base address for
                ** shared ram might have changed, so check the value
                */
                if (cmd->pos_reg == 2 && cmd->status == CIO_OK)
                        ca->base_addr = cat_addr_map[ADDR_BITS(cmd->pos_val)];
                cmd->status = WRITE_DONE;
                break;

        case CAT_CHNL_RD:
                CAT_POS_READ(bus, cmd->pos_reg, &cmd->pos_val);
                cmd->status = READ_DONE;
                break;

        default:
                rc = EINVAL;
        }

        IOCC_DET(bus);                  /* release access to MCI bus */
        return rc;
} /* do_pos_cmd() */


/*****************************************************************************
**
** NAME: do_setadap
**
** FUNCTION: set the adapter parameters on the PCA
**
** EXECUTION ENVIRONMENT:  process
**
** NOTES:
**
**    Input:    pointer to dds structure
**              pointer to setadap cmd structure
**
**    Output:  set catioctl notes
**
**    Called From: catioctl
**
**    Calls To: xmalloc, xmfree, letni16, MEM_ATT, CAT_WRITE, BUSMEM_DET,
**                      cat_put_cmd, cat_wait_ack
**
**
*****************************************************************************/
int
do_setadap(
        struct ca *ca,
        struct cat_set_adap *args,
        int devflag)
{
        struct cat_set_adap params;
        struct cat_adapcfg *cfg;
        ulong bus;
        int i;
        int rc = 0;
        int spl;
        cmd_t cmd;

        if (!(ca->flags & CATDIAG)) {
                return EACCES;
        }

        /*
        ** Copy in the cat_set_adap structure.
        */
        if (COPYIN(devflag, args, &params, sizeof(struct cat_set_adap))) {
                params.status = CIO_LOST_DATA;
                COPYOUT(devflag, &params, args, sizeof(struct cat_set_adap));
                return EFAULT;
        }

        /*
        ** Setup command structure for SetAdapter command.
        */
        cmd.command     = PSCASETA;
        cmd.cmdmod      = MODACK;
        cmd.correl      = 0;
        cmd.subchan     = 0;   /* d51658 */
        cmd.length      = sizeof(cat_adapcfg_t);
        cmd.buffer      = DATABUFS;

        cfg = (struct cat_adapcfg *) &ca->caddp.config_params;

        /*
        ** Copy in the cat_adapcfg structure.
        */
        if (COPYIN(devflag, params.adap_param, cfg, cmd.length)) {
                params.status = CIO_LOST_DATA;
                COPYOUT(devflag, &params, args, sizeof(struct cat_set_adap));
                return(EFAULT);
        }

        /*
        ** Byte-swap the multi-byte structure members.
        */
        letni16(&cfg->xmitsz);
        letni16(&cfg->recvsz);
        letni16(&cfg->xmitno);
        letni16(&cfg->recvno);
        letni16(&cfg->nosubs);

        /*
        ** Move the SET_ADAP command to shared RAM
        */
        DISABLE_INTERRUPTS(spl);
        bus = CAT_MEM_ATT;
        CAT_WRITE(bus, DATABUFS, cfg, cmd.length);
        BUSMEM_DET(bus);        /* release access to MCI bus */
        ENABLE_INTERRUPTS(spl);

        /*
        ** Restore the multi-byte structure members.
        */
        letni16(&cfg->xmitsz);
        letni16(&cfg->recvsz);
        letni16(&cfg->xmitno);
        letni16(&cfg->recvno);
        letni16(&cfg->nosubs);

        /*
        ** Check the return code, then execute the PSCASETA
        ** command and wait for it to be acknowledged.
        */
        if (ca->piorc) {
                /*
                ** The adapter is dead
                */
                params.status = CIO_HARD_FAIL;
                cat_shutdown(ca);
        }
        else if (cat_put_cmd(ca, &cmd)) {
                params.status = CIO_HARD_FAIL;
                rc = EIO;
        }
        else if (cat_wait_ack(ca, &cmd, 15)) {
                params.status = CIO_TIMEOUT;
                cat_logerr(ca, ERRID_CAT_ERR5);
                rc = EIO;
        }
        else if (ca->ca_cmd.origcmd != PSCASETA) {
                params.status = CIO_HARD_FAIL;
                cat_logerr(ca, ERRID_CAT_ERR5);
                rc = EIO;
        }
        else if (ca->ca_cmd.retcode != RETGOOD) {
                params.status = ca->ca_cmd.retcode;
                cat_logerr(ca, ERRID_CAT_ERR5);
                rc = EIO;
        }
        else {
                ca->flags |= CATPARMS_SET;
                params.status = CIO_OK;
        }
        COPYOUT(devflag, &params, args, sizeof(struct cat_set_adap));
        if (rc == 0)
                rc = cat_check_status(ca);
        return rc;
} /* do_setadap() */


/*****************************************************************************
**
** NAME: reset_sub
**
** FUNCTION: reset the specified subchannel
**
** EXECUTION ENVIRONMENT:  process only
**
** NOTES:
**
**    Input:  pointer to dds structure
**                       pointer to session block
**
**    Output: see catioctl notes
**
**    Called From: catioctl
**
**    Calls To: clean_sc_queues
**
**      RETURNS:
**              EFAULT - copyin() or copyout() failed
**              EINVAL - invalid Subchannel ID
**              EIO - microcode not functional or cat_put_cmd() failed
**              EPERM - caller didn't start this SC or isn't superuser
**              ENXIO -
*****************************************************************************/
int
reset_sub(
        struct ca *ca,
        open_t *openp,
        struct session_blk *arg,
        int devflag)
{
        open_t *tp;
        subchannel_t *sc;
        int spl;
        cmd_t cmd;
        struct session_blk sess_blk;

        if (!(ca->flags & CATFUNC))
                return EIO;

        if (COPYIN(devflag, arg, &sess_blk, sizeof(struct session_blk))) {
                return EFAULT;
        }

        if (sess_blk.netid > MAX_SUBCHAN) {
                sess_blk.status = CIO_NETID_INV;
                COPYOUT(devflag, &sess_blk, arg, sizeof(struct session_blk));
                return EINVAL;
        }

        if ((sc = ca->sc[arg->netid]) == NULL ) {
                sess_blk.status = CIO_NOT_STARTED;
                COPYOUT(devflag, &sess_blk, arg, sizeof(struct session_blk));
                return EINVAL;
        }

        tp = sc->links[0]->lk_open;
        if (tp == NULL) {
                sess_blk.status = CIO_NETID_INV;
                COPYOUT(devflag, &sess_blk, arg, sizeof(struct session_blk));
                return ENXIO;
        }

        if (tp != openp && getuidx(ID_EFFECTIVE)) {
                sess_blk.status = CIO_NETID_INV;
                COPYOUT(devflag, &sess_blk, arg, sizeof(struct session_blk));
                return EPERM;
        }

        clean_sc_queues(ca, sc->sc_id, -1);

        cmd.command = PSCARSTS;
        cmd.cmdmod = 0;
        cmd.correl = 0;
        cmd.subchan = arg->netid;
        cmd.data[0] = 0;
        cmd.data[1] = 0;
        cmd.length = 0;
        cmd.buffer = 0;

        if (cat_put_cmd(ca, &cmd)) {
                sess_blk.status = CIO_HARD_FAIL;
                COPYOUT(devflag, &sess_blk, arg, sizeof(struct session_blk));
                return EIO;
        }

        sess_blk.status = CIO_OK;
        COPYOUT(devflag, &sess_blk, arg, sizeof(struct session_blk));
        return 0;
} /* reset_sub() */

/*****************************************************************************
**
** NAME: reset_all
**
** FUNCTION: reset all of the subchannels
**
** EXECUTION ENVIRONMENT:  process only
**
** NOTES:
**
**    Input:  pointer to dds structure
**                       pointer to session block
**
**    Output: see catioctl notes
**
**    Called From: catioctl
**
**    Calls To: clean_sc_queues
**
**
*****************************************************************************/
int
reset_all(struct ca *ca)
{
        if (!(ca->flags & CATFUNC)) {
                return EIO;
        }
        if (getuidx(ID_EFFECTIVE)) {
                return EPERM;
        }

        clean_sc_queues(ca, -1, 0);

        ca->ca_cmd.command = PSCARSTA;
        ca->ca_cmd.cmdmod = 0;
        ca->ca_cmd.correl = 0;
        ca->ca_cmd.subchan = 0;
        ca->ca_cmd.data[0] = 0;
        ca->ca_cmd.data[1] = 0;
        ca->ca_cmd.length = 0;
        ca->ca_cmd.buffer = 0;

        if (cat_put_cmd(ca, &ca->ca_cmd)) {
                return EIO;
        }

        return 0;
} /* reset_all() */


/*****************************************************************************
**
** NAME: rw_sram
**
** FUNCTION: read or write shared ram
**
** EXECUTION ENVIRONMENT:  process only
**
** NOTES:
**
**    Input:  pointer to dds structure
**            pointer to r/w structure
**    Output:
**                      see catioctl notes
**
**    Called From: catioctl
**
**    Calls To: copyin, copyout, do_sram_cmd
**
**
*****************************************************************************/
int
rw_sram (
        struct ca *ca,
        struct cat_rw_sram *arg)
{
        struct cat_rw_sram rw_cmd;
        int rc;

        if (copyin(arg, &rw_cmd, sizeof(rw_cmd)))
                return EFAULT;
        if (rw_cmd.rd_wrt == CAT_CHNL_WR) {
                if (!(ca->flags & CATDIAG)) {
                        return EACCES;
                }
/*              ca->flags &= ~(CATFUNC | CATSETUP | CATABLE);
*/
        }
        rc = do_sram_cmd(ca, &rw_cmd);

        rw_cmd.status = ( rc ) ? rc : CIO_OK;
        if (copyout(&rw_cmd, arg, sizeof(rw_cmd)))
                return EFAULT;

        /*
        ** return bad error code only if it is a normal
        ** read/write -- return 0 if they specified "NO_FAIL"
        */
        if (rc && ((rw_cmd.rd_wrt == CAT_CHNL_WR)
                || (rw_cmd.rd_wrt == CAT_CHNL_RD)))
                return EFAULT;
        else
                return 0;
} /* rw_sram() */



/*****************************************************************************
**
** NAME: do_diag
**
** FUNCTION: run an on-board diagnostic test a given number of times
**
** EXECUTION ENVIRONMENT:   process only
**
**
** NOTES:
**
**    Input:  pointer to dds structure
**                       pointer to diag cmd structure
**
**    Output:  see catioctl notes
**
**    Called From: catioctl
**
**    Calls To:  cat_put_cmd, cat_wait_sram
**
**
******************************************************************************/
int
do_diag(
        struct ca *ca,
        open_t *openp,
        struct cat_run_diag *rd,
        dev_t devflag)
{
        int rc;
        int reg;
        int waittime;
        struct cat_run_diag lrd;
        int mem;
        int spl;

        if (!(ca->flags & CATDIAG))
                return EACCES;

        if (COPYIN(devflag, rd, &lrd, sizeof(struct cat_run_diag)))
                return EFAULT;

        DISABLE_INTERRUPTS(spl);
        mem = CAT_MEM_ATT;
        CAT_READ1(mem, 0x66, &reg);
        BUSMEM_DET(mem);
        ENABLE_INTERRUPTS(spl);
        waittime = CAT_TIMEOUT;
        /*
        ** Setup command structure to return the unused xmit buf
        ** use a local command structure for commands that don't
        ** use a correlator
        */
        ca->ca_cmd.command = PSCADIAG;
        ca->ca_cmd.cmdmod = 0;
        ca->ca_cmd.correl = 0;
        ca->ca_cmd.length = 0;
        ca->ca_cmd.buffer = 0;
        ca->ca_cmd.data[0] = lrd.test_num;
        ca->ca_cmd.data[1] = lrd.times;

        if (cat_put_cmd(ca, &ca->ca_cmd)) {
                lrd.status = CAT_NOPCA_BUF;
                COPYOUT(devflag, &lrd, rd, sizeof(struct cat_run_diag));
                return EAGAIN;
        }

        /*
        ** now wait for the appropriate response in the
        ** status area of shared RAM
        */
        /*
        ** wait for diags to finish
        */
        rc = cat_wait_sram(ca, openp, 0x66, reg, waittime, &reg);
        if (reg != 0 && rc < 0)
                lrd.status = POST_TIME_OUT;
        else if (reg != 0 && rc > 0) {
                lrd.status = POST_FAIL;
                COPYOUT(devflag, &lrd, rd, sizeof(struct cat_run_diag));
                return EIO;
        }
        lrd.status = CIO_OK;
        COPYOUT(devflag, &lrd, rd, sizeof(struct cat_run_diag));
        return 0;
} /* do_diag() */


/*****************************************************************************
**
** NAME: adap_info
**
** FUNCTION:    queries adapter information
**
** EXECUTION ENVIRONMENT: process
**
** NOTES:
**
**    Input:  pointer to dds structure
**                       pointer to adapter_info structure
**                       devflag
**
**    Output: see catioctl notes
**
**    Called From: catioctl
**
**    Calls To: bcopy, copyout
**
**      RETURNS:
**              0 - success
**              EIO - the microcode isn't functional
**              EFAULT - copyout() failed
**
******************************************************************************/
int
adap_info(
        struct ca *ca,
        struct cat_adap_info *ci,
        dev_t devflag)
{
        struct cat_adap_info tci;

        if (cat_check_status(ca)) {
                return EIO;
        }

        tci.status = CIO_OK;
        tci.chanspd = ca->status.chanspd;
        tci.sbchact = ca->status.sbchact;
        tci.xbuflen = ca->status.xbuflen;
        tci.rbuflen = ca->status.rbuflen;
        tci.xbufno = ca->status.xbufno;
        tci.rbufno = ca->status.rbufno;
        bcopy(ca->status.status, tci.sub_status, sizeof (tci.sub_status));
        bcopy(ca->status.opmode, tci.opmode, sizeof (tci.opmode));
        bcopy(ca->status.cutype, tci.cutype, sizeof (tci.cutype));

        if (COPYOUT(devflag, &tci, ci, sizeof(tci))) {
                return EFAULT;
        }

        return 0;
} /* adap_info() */


/*****************************************************************************
**
** NAME:        load_cu
**
** FUNCTION:    loads a control unit table definition to the PCA
**
** EXECUTION ENVIRONMENT:  process only
**
** NOTES:
**
**    Input: pointer to dds structure
**              pointer to control unit structure
**              devflag
**
**    Output: see catioctl
**
**    Called From: catioctl
**
**    Calls To: cat_check_status, copyin, do_cutable
**
** RETURNS:
**
*****************************************************************************/
int
load_cu(
        struct ca *ca,
        struct cat_cu_load *arg,
        dev_t dev_flag )
{
        struct cat_cu_load dnld_tbl_cmd;
        int rc = 0;

        /* Microcode must be functional and driver has to be in diag mode */
        if ((ca->flags & CATFUNC) == 0 || (ca->flags & CATDIAG) == 0)
                return EACCES;

        ca->flags &= ~CATABLE;

        if (COPYIN(dev_flag, arg, &dnld_tbl_cmd, sizeof(dnld_tbl_cmd)))
                return EFAULT;
        /*
        ** Set the adapter's parameters.  This is just like the CAT_SET_ADAP
        ** ioctl(), but is done automatically so the device methods don't
        ** have to do it.
        */
        if (!(ca->flags & CATPARMS_SET)) {
                struct cat_set_adap set_adap;

                set_adap.num_sub = ca->caddp.config_params.nosubs;
                set_adap.adap_param = &ca->caddp.config_params;
                if (rc = do_setadap(ca, &set_adap, DKERNEL)) {
                        ca->flags &= ~CATSETUP;
                        /*
                        ** dnld_cmd.status = ???
                        ** COPYOUT(devflag, &dnld_cmd, arg, sizeof(dnld_cmd));
                        */
                        return rc;
                }
                rc = cat_check_status(ca);
        }
        if ((rc = do_cutable(ca, dnld_tbl_cmd.cu_addr, dnld_tbl_cmd.length,
                dnld_tbl_cmd.cu_type, dnld_tbl_cmd.overwrite)) == 0) {
                dnld_tbl_cmd.status = rc;
        } else {
                ca->flags |= CATABLE;
        }
        if( COPYOUT(dev_flag, &dnld_tbl_cmd, arg, sizeof(dnld_tbl_cmd)) )
                return EFAULT;
        if (rc == 0)
                rc = cat_check_status(ca);
        return rc;
} /* load_cu() */



/*****************************************************************************
**
** NAME: do_cutable
**
** FUNCTION: loads cutable into PCA
**
** EXECUTION ENVIRONMENT: process
**
** NOTES:
**
** RETURNS:
**              0 : SUCCESS
**              CAT_CU_NOTLOADED
**              EIO
**
******************************************************************************/
int
do_cutable(
        struct ca *ca,
        uchar *uaddr,
        ulong len,
        ulong cutype,
        ulong overwrite)
{
        int bus;
        struct cat_cu_load local_arg;
        uchar *sense_id;
		int  flags=0;
        uchar normal_sense_id[7] = { 0xFF, 0x30, 0x88, 0x60, 0x00, 0x00, 0x00 };
        uchar claw_sense_id[7]   = { 0xFF, 0x30, 0x88, 0x61, 0x00, 0x00, 0x00 };

        if (cat_user_write(ca, uaddr, DATABUFS, CAT_USE_PIO, len)) {
                local_arg.status = CAT_CU_NOTLOADED;
                return EIO;
        }
        if (cutype == 3 /* CLAW */)
                sense_id = claw_sense_id;
        else
                sense_id = normal_sense_id;
		if (overwrite)  flags |= 0x04;

        bus = CAT_MEM_ATT;
        /* CAT_WRITE sets ca->piorc */
        CAT_WRITE(bus, DATABUFS+len, sense_id, 7);
        BUSMEM_DET(bus);                /* release access to MCI bus */
        if (ca->piorc) {
                local_arg.status = CAT_CU_NOTLOADED;
                return EIO;
        }

        /*
        ** Setup command structure for SetAdapter command
        */
        ca->ca_cmd.command = PSCALDCU;
        ca->ca_cmd.cmdmod = MODACK;
        ca->ca_cmd.correl = 0;
        ca->ca_cmd.subchan = 0;            /* d51658 */
        ca->ca_cmd.data[0] = cutype;
        /* ca->ca_cmd.data[1] = overwrite | 4; /* overwrite old table, sense info*/
        ca->ca_cmd.data[1] = flags ; /* overwrite old table, sense info*/
        ca->ca_cmd.length = len+7;
        ca->ca_cmd.buffer = DATABUFS;
        if (cat_put_cmd(ca, &ca->ca_cmd)
                || cat_wait_ack(ca, &ca->ca_cmd, 4)
                || ca->ca_cmd.origcmd != PSCALDCU
                || ca->ca_cmd.retcode != RETGOOD) {
                local_arg.status = CAT_CU_NOTLOADED;
                return CAT_CU_NOTLOADED;
        }
        return 0;
} /* do_cutable() */


/*****************************************************************************
**
** NAME: do_setsub
**
** FUNCTION: sets the subchannel parameters
**
** EXECUTION ENVIRONMENT:  process only
**
** NOTES:
**
**    Input: pointer to dds structure
**                      pointer to set subchannel structure
**
**    Output: see catioctl notes
**
**    Called From: catioctl
**
**    Calls To: cat_get_cfb, MEM_ATT, CAT_WRITE, BUSMEM_DET, cat_put_cmd,
**
**
******************************************************************************/
int
do_setsub(
        struct ca *ca,
        struct cat_set_sub *setsub_cmd)
{
        SUBPARMS subparms;
        link_t *link;                                     /*d51658*/
        subchannel_t *sc;                                 /*d51658*/

        /* get sc and link pointer */                     /*d51658*/
        sc = ca->sc[setsub_cmd->sb.netid];                /*d51658*/
        link = sc->links[0];                              /*d51658*/

        /*
        ** Setup command structure for SetSubchannel command
        */
        ca->ca_cmd.command = PSCASETS;
        ca->ca_cmd.cmdmod = MODACK;
        ca->ca_cmd.correl = link->lk_correl;    /* d51658 */
        ca->ca_cmd.subchan = setsub_cmd->sb.netid;
        ca->ca_cmd.length = sizeof(SUBPARMS);

        /*
        ** Setup data for Set Subchannel Parameters command
        */

        if( setsub_cmd->set_default ) {
                subparms.specmode = CAT_FLUSHX_MOD;
                ca->sc[setsub_cmd->sb.netid]->specmode = CAT_FLUSHX_MOD;
                subparms.shrtbusy = 0x03;
                subparms.startde = 0x01;
        } else {
                subparms.specmode = setsub_cmd->specmode;
                subparms.shrtbusy = setsub_cmd->shrtbusy;
                subparms.startde = setsub_cmd->startde;
        }
        subparms.subset = setsub_cmd->subset;
        subparms.flags = NEW_SC_DEF;
        letni16(&subparms.subset);

        if (cat_get_cfb(ca, &ca->ca_cmd.buffer)) {
                setsub_cmd->sb.status = CAT_NOPCA_BUF;
                free_sc_links(ca, sc);           /* d51658 */
                return EAGAIN;
        }
        cat_write_buf(ca, ca->ca_cmd.buffer, &subparms, sizeof(subparms));

        if (cat_put_cmd(ca, &ca->ca_cmd)) {
                cat_ret_buf(ca, ca->ca_cmd.buffer, CFB_TYPE);
                setsub_cmd->sb.status = CAT_NOPCA_BUF;
                free_sc_links(ca, sc);       /* d51658 */
                return EAGAIN;
        }

        /*
        ** at this point the command was issued without any problems,
        ** so set the return status to 0
        */
        setsub_cmd->sb.status = CIO_OK;
        return 0;
} /* do_setsub() */



/*****************************************************************************
**
** NAME:  do_stopsub
**
** FUNCTION: halts a specified subchannel
**
** EXECUTION ENVIRONMENT:  process only
**
** NOTES:
**
**    Input:  pointer to dds structure
**                       pointer to session block
**
**    Output: see catioctl notes
**
**    Called From: catioctl
**
**    Calls To: cat_put_cmd
**
** RETURN:  0 = Good Return
**      EIO - failed
**
*****************************************************************************/
int
do_stopsub(
        struct ca *ca,
        struct session_blk *sess_blk)
{
        int spl;
        int i;
        cmd_t fake_notify;

        sess_blk->status = 0;

        /*
        ** If already closed, just return.
        */
        if (ca->sc[sess_blk->netid]->sc_state == SC_CLOSED
        || ca->sc[sess_blk->netid]->sc_state == SC_CLOSING) {
                return 0;
        }

        /*
        ** Update the SC state(s).
        */
        DISABLE_INTERRUPTS(spl);
        SET_GROUP(ca->sc[sess_blk->netid]->sc_group, SC_CLOSING);
        ENABLE_INTERRUPTS(spl);

        /*
        ** Setup command structure for StopSubchannel command
        */
        bzero(&ca->ca_cmd,sizeof(CTLFIFO)); /* d51376 */
        ca->ca_cmd.command = PSCASTOP;
        ca->ca_cmd.cmdmod = MODACK;
        ca->ca_cmd.correl = 0;
        ca->ca_cmd.subchan = sess_blk->netid;

        /*
        ** Send the STOP command.
        */
        if (cat_put_cmd(ca, &ca->ca_cmd)) {
                sess_blk->status = CIO_HARD_FAIL;
                DISABLE_INTERRUPTS(spl);
                SET_GROUP(ca->sc[sess_blk->netid]->sc_group, SC_OPEN);
                ENABLE_INTERRUPTS(spl);
                return EIO;
        }

        /*
        ** Get the process ID so the e_post() knows who to post.
        */
#ifdef AIXV3
        ca->sc[sess_blk->netid]->sc_stop_ack = getpid();
#endif


        /*
        ** If the command was sent, wait for acknowledgement.
        */
#ifndef AIXV3
        if (e_sleep(&ca->sc[sess_blk->netid]->sc_stop_ack ,EVENT_SIGRET) == EVENT_SIG){
#else

        if (e_wait(CAT_STOP_EVENT, CAT_STOP_EVENT, EVENT_SIGRET) != CAT_STOP_EVENT){
#endif
                /*
                ** If the user terminated the wait, clear sc_stop_ack so
                ** a wakeup isn't done, and call ack_stop() to free the
                ** associated resources.
                */
                DISABLE_INTERRUPTS(spl);
#ifndef AIXV3
                ca->sc[sess_blk->netid]->sc_stop_ack = EVENT_NULL;
#else
                ca->sc[sess_blk->netid]->sc_stop_ack = 0;
#endif

                bzero(&fake_notify, sizeof(CTLFIFO)); /* d51376 */
                fake_notify.subchan = sess_blk->netid;
                ack_stop(ca, &fake_notify);
                ENABLE_INTERRUPTS(spl);
                /* ix31884  ack_stop no nolonger will free the link structures
                   since it is most often executed from the offlevel interrupt
                   and memory can only be allocated and freed on the PROCESS
                   environment
                 */
                free_sc_links(ca,ca->sc[sess_blk->netid]);
                if (ca->sc[sess_blk->netid]->specmode & CAT_CLAW_MOD){
					/* Do nothing for CLAW mode..... */
                } else {
                    /* clean the queues for the session block*/
                    clean_sc_queues(ca,sess_blk->netid,-1);
                }


                return EINTR;
        } else {  /* got the stop event just free the links  */
                free_sc_links(ca,ca->sc[sess_blk->netid]);
			 if (ca->sc[sess_blk->netid]->specmode & CAT_CLAW_MOD){
					/* Do nothing for CLAW mode..... */
			 } else {
				/* clean the queues for the session block*/ 
				clean_sc_queues(ca,sess_blk->netid,-1);
			 }
 
        }


        return 0;
} /* do_stopsub() */


/***********************************************************************
** NAME: sess_start()
**
** FUNCTION:
** This function handles the SESSION_REQ ioctl.  It sets up and starts an
** adapter subchannel or subchannel group.
**
** EXECUTION ENVIRONMENT:       process thread only
**
** INPUT:       pointer to the adapter structure
**                      pointer to the open structure
**                      address of the user process' argument structure
**                      the mode flags for this open
**                      the channel number or the minor device's minor number
**
** OUTPUT:      see notes for catioctl
**
** CALLERS:     catioctl
**
** CALLEES:     COPYIN(), COPYOUT(), xmalloc(), do_setsub()
**
** RETURNS:     0 ---------- successful completion
**              EINVAL ----- an invalid subchannel number or range was specified
**              EADDRINUSE - a subchannel was specified that is currently in use
**              ENOMEM ----- a subchannel structure could not be allocated
**              EAGAIN ----- the required PSCA resources could not be allocated
**              EFAULT ----- COPYIN() or COPYOUT() function failed
**              EIO -------- a PIO operation failed
*****************************************************************************/
int
sess_start(
        struct ca *ca,
        open_t *openp,
        struct cat_set_sub *sess_blk,
        chan_t chan)
{
        subchannel_t *sc = NULL;
        int rc = 0;
        int i;
        cmd_t cmd;
        char buf[3 * LK_NAME_LEN];
        int spl;
        link_t *link;
        int sess_scid = sess_blk->sb.netid;
        ulong *status = &(sess_blk->sb.status);
        static link_correl;

        /*
        ** check to see if the subchannel exists. if it doesn't and it is
        ** in the claw tables, then create it, else check to see if already
        ** existing sc is in claw mode
        */
        DISABLE_INTERRUPTS(spl);

        if ((sc = ca->sc[sess_scid]) == NULL) {
                if (ca->param_table[sess_scid]) {
                        /* read subchannel */
			ENABLE_INTERRUPTS(spl);
			LOCK(ca->lcklvl);
                        ca->sc[sess_scid] = sc = KMALLOC(subchannel_t);
			UNLOCK(ca->lcklvl);
			DISABLE_INTERRUPTS(spl);
/*d51658*/               if (sc == 0) {
/*d51658*/                       ENABLE_INTERRUPTS(spl);
/*d51658*/                       return ENOMEM;
/*d51658*/               }
                        bzero(sc, sizeof(subchannel_t));
                        sc->specmode = claw_params[2];
                        sc->sc_state = SC_CLOSED;
                        sc->sc_group = sess_scid;
                        sc->sc_subset = sess_blk->subset;
                        sc->sc_id = sess_scid;
#ifndef AIXV3
                        sc->sc_stop_ack = EVENT_NULL;
#else
                        sc->sc_stop_ack = 0;
#endif

                        sc->num_links = 0;

				   ENABLE_INTERRUPTS(spl);
				   LOCK(ca->lcklvl);
                        sc->links = (link_t **)
                                xmalloc(MAX_LINKS * sizeof(link_t *),
                                        2, pinned_heap);
				   UNLOCK(ca->lcklvl);
				   DISABLE_INTERRUPTS(spl);
                        if (sc->links == 0) {
                                ENABLE_INTERRUPTS(spl);
						  LOCK(ca->lcklvl);
/*d51658*/                       KFREE(sc);
						  UNLOCK(ca->lcklvl);
                                return ENOMEM;
                        }
                        bzero(sc->links, MAX_LINKS * sizeof(link_t *));

                        /* write subchannel points to the read subchannel */
                        ca->sc[sess_scid + 1] = sc;
                } else {
                        *status = CIO_NETID_INV;
                        ENABLE_INTERRUPTS(spl);
                        return EADDRINUSE;
                }
        } else if ((sc->num_links + 1) >= MAX_LINKS) {
                *status = EMLINK;
                ENABLE_INTERRUPTS(spl);
                return EMLINK;
        }
/*
 ** Search the links list to determine if there is already
 ** a link for this open.
 ** Since multiple links per open on a specific subchannel
 **  are not supported return EPERM
 ** Defect 110852.  Enforce Permanent restriction of one link
 ** per open per subchannel       
 */
        for (i=0;i<MAX_LINKS;i++) {
            if ( (sc->links[i] != (link_t *)0)) {
                 if ((sc->links[i]->lk_open == openp)){
                    ENABLE_INTERRUPTS(spl);
                    return EPERM;
                 }
            }
        }

        /*
        ** Find an empty slot.  There has to be one,
        ** since we already validated the number of links.
        */
        for (i=0; i<MAX_LINKS; i++) {
                if (sc->links[i] == (link_t *)0) {
                        break;
                }
        }

        /*
        ** Allocate the link structure and initialize it.
        */
	   ENABLE_INTERRUPTS(spl);
	   LOCK(ca->lcklvl);
        if ((link = sc->links[i] = KMALLOC(link_t)) == (link_t *)0) {
			 UNLOCK(ca->lcklvl);
			 if (i == 0 ) free_sc_links(ca,sc);
                return ENOMEM;
        }
	   UNLOCK(ca->lcklvl);
	   DISABLE_INTERRUPTS(spl);

	openp->op_sc_opened = 1;
        bzero(link, sizeof(link_t));
        link->lk_open = openp;
        link->lk_sc_id = sess_scid;
        /* link->lk_actual_id = 0;              Don't know it yet */
        /* link->lk_appl_id = 0;                Don't know it yet */
        link->lk_state = LK_NEW;
        link->lk_correl = i; /* d51658 */
        link->lk_recv_elem = (recv_elem_t *)NULL;
        bcopy(sess_blk->claw_blk.WS_appl, link->lk_WS_appl, LK_NAME_LEN);
        bcopy(sess_blk->claw_blk.H_appl, link->lk_H_appl, LK_NAME_LEN);
        link->overrun = 0;   /* d50453  initialize the overrun counter */

        /*
        ** If new SC, initiate the set subchannel params cmd.
        */
        if (sc->sc_state == SC_CLOSED) {
                bzero(&cmd, sizeof(CTLFIFO)); /* d51376 */
                bcopy(ca->caddp.adapter_name, CLAW_PARAM_NAMES, LK_NAME_LEN);
                bcopy(ca->caddp.host_name, CLAW_PARAM_NAMES+8, LK_NAME_LEN);
                /*
                ** Substitute blanks for NULLs---the host TCPIP is too
                ** stupid to quit comparing when it sees a NULL...
                */
                for (i=0; i<LK_NAME_LEN*2; i++) {
                        if (*(CLAW_PARAM_NAMES+i) == 0) {
                                *(CLAW_PARAM_NAMES+i) = ' ';
                        }
                }
                /* send the set parameters */
                cmd.command = PSCASETS;
                cmd.cmdmod = MODACK;
                cmd.subchan = sess_scid;
/*d51658*/       cmd.correl  = link->lk_correl;
                cmd.length = sizeof(claw_params);

                if (cat_get_cfb(ca, &cmd.buffer)) {
                        /* d51658 comment
                        ** free sc and links
                        */
                        ENABLE_INTERRUPTS(spl);
/*d51658*/               free_sc_links(ca, sc);
                        return EAGAIN;
                }
                cat_write_buf(ca, cmd.buffer, claw_params, sizeof(claw_params));
                if (cat_put_cmd(ca, &cmd)) {
                        ENABLE_INTERRUPTS(spl);
/*d51658*/               free_sc_links(ca, sc);
                        return EAGAIN;
                }
        } else if (sc->sc_state == SC_OPEN) {
            /*
            ** SC already started or starting, just request new link id.
            ** Write the host application and workstation
            ** application names to the send buffer.
            */
            bzero(&cmd, sizeof(CTLFIFO)); /* d51376*/
            cat_get_cfb(ca, &(cmd.buffer));
            bcopy(sess_blk->claw_blk.WS_appl, buf, 3 * LK_NAME_LEN);
            cat_write_buf(ca, cmd.buffer, buf, 3 * LK_NAME_LEN);

            /*
            ** Send a connect request to get the link id.
            */
            link->lk_state = LK_REQ;
            cmd.command = PSCACLREQ;
            cmd.subchan = sess_scid;
            cmd.correl = link->lk_correl;
            cmd.length = 3 * LK_NAME_LEN;
            if (cat_put_cmd(ca, &cmd)) {
                    ENABLE_INTERRUPTS(spl);
                    /* d51658 comment
                    ** free up only the new link here last one in links
                    */
/*d51658*/           drop_link(sc, link);
                    return EAGAIN;
            }
        } else {
                /*
                ** Must have a start in progress already.  Let it finish.
                ** Don't worry about not making the PSCACLREQ request,
                ** when the PSCASYSVAL comes in we'll run through the
                ** list of link structures and send requests for all
                ** those in LK_DISC or LK_NEW state.
 d51658          ** In this case we leave link and sc structs and pointers
 d51658          ** in tact.
                */
                ENABLE_INTERRUPTS(spl);
            return EAGAIN;
        }

        ENABLE_INTERRUPTS(spl);
        return 0;
} /* sess_start() */

/*************************************************************************
** NAME: sess_disc()
**
** FUNCTION:
** This function handles the SESSION_REQ ioctl.  It sets up and starts an
** adapter subchannel or subchannel group.
**
** EXECUTION ENVIRONMENT:       process thread only
**
** INPUT:       pointer to the adapter structure
**                      pointer to the open structure
**                      address of the user process' argument structure
**                      the mode flags for this open
**                      the channel number or the minor device's minor number
**
** OUTPUT:      see notes for catioctl
**
** CALLED BY:   catioctl
**
** CALLS:       cat_put_cmd()
**
** RETURNS:
**      0 - successful completion
**      EINVAL - an invalid subchannel number or range was specified
**      ENOENT - link not started
**      EIO - couldn't send HALT command to the adapter
*****************************************************************************/
int
sess_disc(
        struct ca *ca,
        open_t *openp,
        struct cat_set_sub *sess_blk,
        chan_t chan)
{
        subchannel_t *sc = NULL;
        int rc = 0;
        cmd_t cmd;
        int spl;
        ulong *status = &sess_blk->sb.status;
        int sess_scid = sess_blk->sb.netid;
        link_t *link;
        int i;


        DISABLE_INTERRUPTS(spl);

        sc = ca->sc[sess_scid];

        if (sess_blk->claw_blk.linkid < 1
        || sess_blk->claw_blk.linkid >= MAX_LINKS) {
                *status = CIO_NETID_INV;
                ENABLE_INTERRUPTS(spl);
                return EFAULT;
        } else if ((link = sc->link[sess_blk->claw_blk.linkid]) == NULL
                || link->lk_state != LK_FIRM)  {
                *status = CIO_NOT_STARTED;
                ENABLE_INTERRUPTS(spl);
                return ENOENT;
        }

        /*
        ** Find the link slot in the sc->links[] "array"
        */
        for (i=0; i<MAX_LINKS; i++) {
                if (link ==  sc->links[i]) {
                        break;
                }
        }
        if (i == MAX_LINKS) {
                *status = CIO_NETID_INV;
                ENABLE_INTERRUPTS(spl);
                return EFAULT;
        }

        /* send a connection disconnect */
        bzero(&cmd, sizeof(CTLFIFO)); /* d 51376 */
        cmd.command = PSCACLDISC;
        cmd.subchan = sess_scid;
        cmd.ccw = sess_blk->claw_blk.linkid;
        if (cat_put_cmd(ca, &cmd)) {
                *status = CIO_HARD_FAIL;
                ENABLE_INTERRUPTS(spl);
                return EIO;
        }

        /*
        ** sc->links[] is an unordered (by link_id) array of
        ** link_t structures, while sc->link[] is an ordered
        ** array (indexed by link_id) that gets filled in once
        ** we receive a valid link_id
        */
        sc->links[i] = (link_t *)0;
        sc->link[sess_blk->claw_blk.linkid] = (link_t *)0;
        sc->num_links--;
        ENABLE_INTERRUPTS(spl);
		LOCK(ca->lcklvl);
        KFREE(link);
		UNLOCK(ca->lcklvl);



        *status = CIO_OK;
        return 0;
} /* sess_disc() */


/***********************************************************************
** NAME: cat_on_offline
**
** FUNCTION:
**
** EXECUTION ENVIRONMENT:       process thread only
**
** INPUT:       pointer to the adapter structure
**              pointer to  the user process' argument structure
**
** OUTPUT:
**
** CALLS:
**
** CALLED BY:
**
** RETURNS:     0 : SUCCESS
**              EINVAL : invalid command
*****************************************************************************/
static int cat_on_offline(
        struct ca *ca,
        struct cat_on_offline *on_off,
        chan_t chan)
{
        int i;
        int rc = 0;

        /* Must have opened in '/C' mode */
        if ((ca->open_lst[chan].op_flags & OP_CE_OPEN) == 0)
                return EACCES;

        switch (on_off->cmd) {
                case CAT_SET_OFFLINE:
                        ca->flags |= CAT_NOSTARTS;
                        rc = stop_all_subs(ca);
                        break;
                case CAT_SET_ONLINE:
                        ca->flags &= ~CAT_NOSTARTS;
                        break;
                case CAT_CHECK_ON_OFF:
                        rc = cat_check_status(ca);
                        if (ca->status.sbchact)
                                on_off->state = CAT_ONLINE;
                        else
                                on_off->state = CAT_OFFLINE;
                        break;
                default:
                        rc = EINVAL;
                        break;
        }

        return rc;
} /* cat_on_offline() */


/***********************************************************************
** NAME: stop_all_subs
**
** FUNCTION:
**
** EXECUTION ENVIRONMENT:       process thread only
**
** INPUT:       pointer to the adapter structure
**              pointer to  the user process' argument structure
**
** OUTPUT:
**
** CALLS:
**
** CALLED BY:
**
** RETURNS:     0 : SUCCESS
**              EINVAL : invalid command
*****************************************************************************/
int stop_all_subs(
        struct ca *ca)
{
        int i;
        int j;
        int spl;
        struct subchannel *sc;
        struct session_blk sess_blk;
        cmd_t cmd;

        for (i=0; i<CAT_MAX_SC; i++) {
                if ((sc = ca->sc[i])
                && (sc->sc_state != SC_CLOSING || sc->sc_state != SC_CLOSED)) {
                        if (sc->specmode & CAT_CLAW_MOD) {
                                for (j=0; j<sc->num_links; j++) {
                                        if (sc->link[j] == 0) {
                                                continue;
                                        }
                                    /* send a connection disconnect */
                                    bzero(&cmd, sizeof(CTLFIFO)); /* d51376 */
                                    cmd.command = PSCACLDISC;
                                    cmd.subchan = i;
                                    cmd.ccw = j;
                                    if (cat_put_cmd(ca, &cmd)) {
                                            return EIO;
                                    }

                                    /*
                                    ** sc->links[] is an unordered (by link_id) array of
                                    ** link_t structures, while sc->link[] is an ordered
                                    ** array (indexed by link_id) that gets filled in once
                                    ** we receive a valid link_id
                                    */
                                    sc->link[j] = (link_t *)0;
                                    sc->num_links--;
                                }
                        }

                        sess_blk.netid = i;
                        sess_blk.length = 1;
                        do_stopsub(ca, &sess_blk);
                        i += (sc->sc_subset - 1);
                }
        } /* for */
} /* stop_all_subs() */


/*****************************************************************************/
/*
 * NAME:     cat_getfastwrt
 *
 * FUNCTION: returns the fastwrite address to a kernel user along with
 *           the parameters required to access the entry point.
 *
 * EXECUTION ENVIRONMENT: called from the process environment only
 *
 * NOTES:
 *
 * RETURNS:
 *      0 - success
 *      EPERM - kernel-mode callers only...
 */
/**********************************************************************/
static int cat_getfastwrt(
        dev_t devno,
        cio_get_fastwrt_t *p_arg,
        ulong devflag)
{
        cio_get_fastwrt_t fastwrt;      /* holds cat_fastwrt */

        if (!(devflag & DKERNEL))
                return EPERM;

        fastwrt.status = CIO_OK;
        fastwrt.fastwrt_fn = cat_fastwrt;
        fastwrt.chan = 0;
        fastwrt.devno = devno;

        bcopy(&fastwrt, p_arg, sizeof(cio_get_fastwrt_t));

        return 0;
} /* cat_getfastwrt() */



/*****************************************************************************/
/*
 * NAME:     do_unst
 *
 * FUNCTION:    Send unsolicited status to the 370 channel.
 *
 * EXECUTION ENVIRONMENT: called from the process environment only
 *
 * NOTES:
 *
 * RETURNS:
 *      0 - success
 *  EPERM - invalid SC or linkid specified
 */
/**********************************************************************/
int do_unst(
        struct ca *ca,
        open_t *openp,
        cat_unst_t *unst_blk)
{
        cmd_t cmd;
        subchannel_t *sc;
        link_t *link;
        int rc;

        /*
        ** Verify that this process opened this SC.
        */
        if ((sc = ca->sc[unst_blk->sc]) == NULL
        || (link = sc->link[unst_blk->linkid]) == NULL
        || link->lk_open != openp) {
                return EPERM;
        }

        /*
        ** Build the command.
        */
        bzero(&cmd, sizeof(CTLFIFO)); /* d51376 */
        cmd.command = PSCAUNST; /* Send unsolicited status */
        cmd.cmdmod = MODACK;
        cmd.subchan = unst_blk->sc;
        cmd.data[0] = unst_blk->status_byte;
        cmd.length = unst_blk->sense_length;

        /*
        ** Get and fill a control buffer if there is extra sense data.
        */
        if (unst_blk->sense_length) {
                if (rc = cat_get_cfb(ca, &cmd.buffer)) {
                        return EIO;
                } else if (rc = cat_write_buf(ca, cmd.buffer,
                                unst_blk->sense_addr, unst_blk->sense_length)) {
                        return EIO;
                }
        }

        /*
        ** Initiate the command.
        */
        if (rc = cat_put_cmd(ca, &cmd)) {
                return EIO;
        }

        return 0;
} /* do_unst() */
