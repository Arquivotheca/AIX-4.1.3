static char sccsid[] = "@(#)37  1.13.4.3  src/bos/diag/tu/async/exectu.c, tu_async, bos41J, 9519A_all 5/1/95 13:27:23";
/*
 *   COMPONENT_NAME: TU_ASYNC
 *
 *   FUNCTIONS: alarm_signal
 *              ctrl_wrap_tst
 *              data_wrap_tst
 *              exectu
 *              init_mask
 *              lcl_loop
 *              peg_data_wrap_tst
 *              ram_tst
 *              rd_byte
 *              rgstr_chk_tst
 *              register_chk
 *              salctrl_wrap_tst
 *              saldata_wrap_tst
 *              salsetloop
 *              salsetup
 *              set_baud
 *              set_chars
 *              set_parity
 *              set_sbits
 *              set_tty
 *              sync_wrap_tst
 *              vpd_chk_tst
 *              wr_byte
 *
 *
 *   ORIGINS: 27, 83
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *   LEVEL 1, 5 Years Bull Confidential Information
 */


#include "atu_tty.h"
#include <sys/mode.h>
#include <sys/mdio.h>

/* System error messages are used to provide supplemental information
 * for certain kinds of errors.  The following give access to that
 * information.
 */
extern  int     errno;
extern  char    *sys_errlist[];
static  char    error_buf[256];

extern mid_t loadext(char *, int, int);

/* Mask table for control wrap tests  */
struct  Mask_table
{
        int result[4];
}wrap[5];

/* Save initial register values for Salmon TU20 */
uchar   mcrdata;
uchar   icrdata[4];

/*
 * NAME: exectu
 *
 * FUNCTION: Oversee test unit execution.
 *
 * EXECUTION ENVIRONMENT:  User level task.
 *
 * NOTES:  The TU executive function provides a single interface to the test
 *      units using the TU control block and the associated file descriptor.
 *
 *      In the diagnostic environment an abbreviated form of the TU control
 *      block is used.  Information from this short control block is copied
 *      to a complete control block and the additional information required
 *      is filled in.  This allows this module to be used without change for
 *      both the HTX and Diagnostic environments.
 *
 *      Each subtest returns a pass (0) or fail (not 0) code.
 *
 * RETURNS:     0 indicates test completed successfully.
 *              not 0 indicates type of failure.
 *              (see tty adapter CIS for details)
 */
exectu (fd, tucb_ptr)
int     fd;
struct  tucb_data  *tucb_ptr;
{
        void    init_mask ();
        int     atu_loops, fru;
        int     rc = PASSED;            /* saved error code */
        char    ldname[40];             /* save line discipline name */
        int     mfdes = FAILED;         /* fd for machine device driver */
        int     tu_rc = PASSED;         /* Return Code from functions */
        struct  sigaction sigvector;    /* structure for signals */
        void    alarm_signal(int);      /* handles the SIGALRM signal  */

        /* Get the VPD so we will know which adapter needs to be tested */
        if (tucb_ptr->ttycb.adapter == UNKNOWN)
        {
                if ((tu_rc = vpd_chk_tst (fd, tucb_ptr, CONFIG)) != 0)
                {
                        if (tu_rc == FAILED) {
                            err(0x1,tu_rc,0);
                            return (tu_rc = tucb_ptr->header.tu);
                        }
                        else {
                            err(0x2,tu_rc,0);
                            return (tu_rc += tucb_ptr->header.tu);
                        }
                }
        }
        else {
                if (tucb_ptr->ttycb.adapter == P128RS232) {
                    if((tu_rc = vpd128_chk_tst (fd,tucb_ptr,TEST)) != 0) {
                        if((tu_rc = vpd128_chk_tst(fd,tucb_ptr,CONFIG)) != 0) {
                            rw_t rw;

                            rw.rw_req = RW_READ;
                            rw.rw_conc = 0;
                            rw.rw_board=(tucb_ptr->ttycb.brd_conc_id >>4)& 0x07;
                            rw.rw_size = 1;

                            if(rw.rw_conc)
                                rw.rw_addr = 0xfc000;
                            else
                                rw.rw_addr = 0;

                            if (ioctl(fd, CXMA_KME, &rw) == 0)
                                tu_rc = 0;
                            else {
                                if (tu_rc == FAILED) {
                                    err(0x3,tu_rc,0);
                                    return (tu_rc = tucb_ptr->header.tu);
                                }
                                else {
                                    err(0x4,tu_rc,0);
                                    return (tu_rc += tucb_ptr->header.tu);
                                }
                            }
                        }
                    }
                }
        }
        /* Initialize tables for control wrap tests  */
        init_mask (tucb_ptr);
        /* Set up alarm signal handler  */
        sigemptyset (&(sigvector.sa_mask));     /* do not block signals */
        sigvector.sa_flags = 0;                 /* do not restart sys calls */
        sigvector.sa_handler = alarm_signal;
        sigaction (SIGALRM, &sigvector, (struct sigaction *) NULL);

        if (tucb_ptr->ttycb.adapter == P128RS232 ||
            tucb_ptr->ttycb.adapter == P128RS232ISA)
	{
                SetPortDefaults(fd);
        }

        if(tucb_ptr->header.mfg != TRUE) {
                switch (tucb_ptr->header.tu)
                {
                case 10:
                case 20:
                case 110:
                        fru = LOCAL;
                        break;
                case 30:
                case 40:
                        fru = DRIVER;
                        break;
                case 50:
                        fru = FANOUT;
                        break;
                case 60:
                case 70:
                        fru = CABLE;
                        break;
                case 80:
                        fru = INTERP;
                        break;
                default:
                        tu_rc = FAILED;
                        tucb_ptr->header.tu &= 0x7e;
                        break;
                }
        }

        /* Main TU test loop  */
        atu_loops = 0;
        while (atu_loops < tucb_ptr->header.loop && tu_rc == PASSED)
        {
                switch (tucb_ptr->header.tu)
                {
                case 10:
                case 110:
                        if ((tu_rc = rgstr_chk_tst (fd, tucb_ptr)) != PASSED) {
                                err(0x5,tu_rc,0);
                                break;
                        }
                        if (tucb_ptr->ttycb.adapter != SIO5 &&
                            tucb_ptr->ttycb.adapter != SIO6 &&
		            tucb_ptr->ttycb.adapter != SIO8 &&
			    tucb_ptr->ttycb.adapter != SIO9 &&
                            tucb_ptr->ttycb.adapter != SIO10 &&
                            tucb_ptr->ttycb.adapter != SIO7 &&
			    tucb_ptr->ttycb.adapter != P8RS232ISA &&
                            tucb_ptr->ttycb.adapter != P128RS232ISA)
                                if(tucb_ptr->ttycb.adapter == P128RS232)
                                    tu_rc = vpd128_chk_tst (fd, tucb_ptr, TEST);
                                else
                                    tu_rc = vpd_chk_tst (fd, tucb_ptr, TEST);
                        break;
                case 20:
                        switch (tucb_ptr->ttycb.adapter)
                        {
                        case SIO3:
                        case SIO4:
                        case SIO5:
                        case SIO6:
                                /* Open machine dd - for TU 20 salmon */
                                if ((mfdes = open("/dev/bus0", O_RDWR)) < 0) {
                                        err(0x6,mfdes,errno);
                                        tu_rc = FAILED;
                                        break;
                                }
                                if ((tu_rc = salsetup (mfdes, SET,
                                    tucb_ptr->ttycb.sal_sio)) != PASSED) {
                                        err(0x7,tu_rc,0);
                                        rc = FAILED;
                                }
                                if (rc == PASSED)
                                        if ((tu_rc = salsetloop (mfdes, SET,
                                            tucb_ptr->ttycb.sal_sio)) != PASSED) {
                                                err(0x8,tu_rc,0);
                                                rc = FAILED;
                                        }
                                if (rc == PASSED)
                                        if ((tu_rc = saldata_wrap_tst (mfdes,
                                            tucb_ptr->ttycb.sal_sio)) != PASSED) {
                                                err(0x9,tu_rc,0);
                                                rc = FAILED;
                                        }
                                if (rc == PASSED)
                                        if ((tu_rc = salctrl_wrap_tst (mfdes,
                                            tucb_ptr->ttycb.sal_sio)) != PASSED) {
                                                err(0x10,tu_rc,0);
                                                rc = FAILED;
                                        }
                                if (rc == PASSED)
                                {
                                        tu_rc = salsetloop (mfdes, CLEAR,
                                            tucb_ptr->ttycb.sal_sio);
                                        tu_rc = salsetup (mfdes, CLEAR,
                                            tucb_ptr->ttycb.sal_sio);
                                }
                                else
                                {
                                        salsetloop (mfdes, CLEAR,
                                            tucb_ptr->ttycb.sal_sio);
                                        salsetup (mfdes, CLEAR,
                                            tucb_ptr->ttycb.sal_sio);
                                }
                                close (mfdes);
                                break;

			case SIO8:
			case SIO9:
                        case SIO7:
				/* Pegasus native lines special case */
				/* interrupts are disabled by the UART */
				/* when in local loop mode */

                                if ((tu_rc = peg_data_wrap_tst (fd, tucb_ptr))
                                    != PASSED) {
                                        err(0x250,tu_rc,0);
                                        break;
                                }
				/* a special ctrl_wrap_tst function will have */
				/* to be added and called from here when */
				/* defect 161626 will be fixed */

                                break;

                        case P64RS232:
                                tu_rc = ram_tst (fd);
                                break;
                        case P128RS232:
                        case P128RS232ISA:
                                if(tucb_ptr->header.mfg == TRUE)
                                        tu_rc = sync_wrap_tst(fd);
                                else
                                        tu_rc = ram_tst (fd);
                                break;
                        default:
                                if(tucb_ptr->ttycb.adapter == P8RS232ISA)
				{
                                        if ((tu_rc = ram_tst (fd)) != PASSED)
						break;
                                }
				else /* temporary until lcl_loop gets fixed */
				{    /* temporary until lcl_loop gets fixed */
                                if ((tu_rc = lcl_loop (fd, tucb_ptr, SET, rc))
                                    != PASSED) {
                                        err(0x11,tu_rc,0);
                                        rc = tu_rc;
                                        (void) lcl_loop (fd, tucb_ptr, CLEAR,
                                            rc);
                                        break;
                                }
                                if ((tu_rc = data_wrap_tst (fd, tucb_ptr))
                                    != PASSED) {
                                        err(0x12,tu_rc,0);
                                        (void) lcl_loop (fd, tucb_ptr, CLEAR,
                                            rc);
                                        break;
                                }
                                if ((tu_rc = ctrl_wrap_tst (fd, tucb_ptr,
                                    fru)) != PASSED) {
                                        err(0x13,tu_rc,0);
                                        (void) lcl_loop (fd, tucb_ptr, CLEAR,
                                            rc);
                                        break;
                                }
                                tu_rc = lcl_loop (fd, tucb_ptr, CLEAR, rc);
                                }    /* temporary until lcl_loop gets fixed */
                                break;
                        }  /* end switch */
                        break;
                case 30:
                        if(tucb_ptr->header.mfg == TRUE &&
                           tucb_ptr->ttycb.adapter == P128RS232) {
                                tu_rc = vpd128_chk_tst(fd, tucb_ptr, TEST);
                                break;
                        }
                case 40:
                case 50:
                case 60:
                case 70:
                case 80:
                        switch(tucb_ptr->ttycb.adapter) {
                        case P64RS232:
                                if (tucb_ptr->header.tu < 50)
                                {
                                    if ((tu_rc=lcl_loop(fd,tucb_ptr,SET,rc))
                                        != PASSED) {
                                        err(0x14,tu_rc,0);
                                        rc = tu_rc;
                                        (void) lcl_loop (fd, tucb_ptr, CLEAR,
                                            rc);
                                        tu_rc = rc;
                                        break;
                                    }
                                    if ((tu_rc = sync_wrap_tst (fd)) != PASSED){
                                        err(0x15,tu_rc,0);
                                        rc = tu_rc;
                                    }
                                    (void) lcl_loop (fd, tucb_ptr, CLEAR, rc);
                                    tu_rc = rc;
                                }
                                else
                                {
                                    if ((tu_rc = data_wrap_tst (fd, tucb_ptr))
                                        != PASSED) {
                                            err(0x16,tu_rc,0);
                                            break;
                                    }
                                    tu_rc = ctrl_wrap_tst (fd, tucb_ptr, fru);
                                }
                                break;
                        case P128RS232:
                        case P128RS232ISA:
                                if (tucb_ptr->header.tu < 50)
                                {
                                    if ((tu_rc=sync128_wrap_tst (fd))!=PASSED) {
                                        err(0x17,tu_rc,0);
                                        rc = tu_rc;
                                    }
                                    tu_rc = rc;
                                }
                                else
                                {
                                    if ((tu_rc = data_wrap_tst (fd, tucb_ptr))
                                        != PASSED) {
                                            err(0x18,tu_rc,0);
                                            break;
                                    }
                                    tu_rc = ctrl_wrap_tst (fd, tucb_ptr, fru);
                                }
                                break;
                        default:
                                if ((tu_rc = data_wrap_tst (fd, tucb_ptr))
                                    != PASSED) {
                                        err(0x19,tu_rc,0);
                                        break;
                                }
                                tu_rc = ctrl_wrap_tst (fd, tucb_ptr, fru);
                                break;
                        }
                        break;
                 default:
                        err(0x20,0,0);
                        tu_rc = FAILED;
                        tucb_ptr->header.tu &= 0x7e;
                        break;
                }  /* end switch */
                atu_loops++;
        }  /* end while */
        /* Close Machine Device Driver and return test status   */
        alarm (5);
        close(mfdes);                   /* close machine device driver */
        alarm (5);
	/* clean up device driver buffers, preventing hanging upon close */
	(void) ioctl (fd, TCFLSH, TCIOFLUSH);
        alarm (OFF);
        if (tu_rc != PASSED)
        {
                if (tu_rc == FAILED)
                        tu_rc = PASSED;
                tu_rc += tucb_ptr->header.tu;
        }

        return (tu_rc);
} /* exectu end */

/*
 * NAME:  alarm_signal
 *
 * FUNCTION:  function to catch and discard alarm signals
 *
 * EXECUTION ENVIRONMENT:  User level task.
 *
 * NOTES:  This function is here to catch alarm signals which might be gene-
 *      rated when a system call times out.  Alarms are used to make sure the
 *      test units do not hang because a system call cannot complete.
 *
 * RETURNS:
 */
void    alarm_signal (int sig)
{
        return;                 /* exit signal handler  */
}  /* alarm_signal end */

/*
 * NAME:  rgstr_chk_tst
 *
 * FUNCTION:  Perform some basic adapter register testing.
 *
 * EXECUTION ENVIRONMENT:  User level task.
 *
 * NOTES:  64 port: Do presence check, i.e. read POS regs.  Device driver
 *               does the actual test.
 *         others: Set various speeds, char. sizes, stop bits, parities.
 *               Line discipline does the work and reports the results.
 *
 * RETURNS:  0  test successful
 *          -1  ioctl failed
 *           4  register test failed
 */
rgstr_chk_tst (fdes, tucb_ptr)
int     fdes;
struct  tucb_data  *tucb_ptr;
{
        int     rct_rc = PASSED;
        int     pct_rc = PASSED;

        switch(tucb_ptr->ttycb.adapter) {
        case P64RS232:
        case P128RS232:
                /* Adapter does presence check.  Driver does the work. */
                if ((rct_rc = ioctl (fdes, LI_PRES, &pct_rc)) == FAILED) {
                        err(0x21,rct_rc,errno);
                        return (rct_rc);
                }
                if (pct_rc != PASSED || rct_rc != PASSED) {
                        err(0x22,pct_rc,0);
                        rct_rc = REG_VER;
                }
                break;
        default:
                /* SIO, 8, and 16 port use the device driver to do
                   all register tests  */
                if ((pct_rc = register_chk(fdes)) != PASSED) {
                        err(0x23,pct_rc,0);
                        rct_rc = REG_VER;
                }
                break;
        }
        return (rct_rc);
}  /* rgstr_chk_tst end */

/*
 * NAME:  register_chk
 *
 * FUNCTION:  Test the registers to make sure that they can be set
 *
 * EXECUTION ENVIRONMENT:  User level task.
 *
 * NOTES:
 *
 * RETURNS:  0  test successful
 *          -1  ioctl failed
 *           1  register check test failed
 */
register_chk (fdes)
int     fdes;
{
        struct  termio  termin, termout;
        struct  termio  tty_attributes;
        int             b,c,s,p;


        /* Store current settings for later restore  */
        if ((ioctl (fdes, TCGETA, &tty_attributes)) != 0) {
                err(0x24,0,errno);
                return (FAILED);
        }

        /* Get current settings and initialize for set/read/comapre */
        if ((ioctl (fdes, TCGETA, &termout)) != 0) {
                err(0x25,0,errno);
                return (FAILED);
        }
        /* Turn off all the input and output processing  */
        termout.c_lflag = 0;
        termout.c_cc[VMIN] = 0;
        termout.c_cc[VTIME] = 0;
        termout.c_iflag = 0;
        termout.c_oflag = 0;
        /* Set values for hardware characteristics  */
        termout.c_cflag = CREAD | CLOCAL;


        /* Loop through baud rates */
        for(b=1;b<=15;b++) {
                switch (b)
                {
                case 1:
                        termout.c_cflag |= B50;
                        break;
                case 2:
                        termout.c_cflag |= B75;
                        break;
                case 3:
                        termout.c_cflag |= B110;
                        break;
                case 4:
                        termout.c_cflag |= B134;
                        break;
                case 5:
                        termout.c_cflag |= B150;
                        break;
                case 6:
                        termout.c_cflag |= B200;
                        break;
                case 7:
                        termout.c_cflag |= B300;
                        break;
                case 8:
                        termout.c_cflag |= B600;
                        break;
                case 9:
                        termout.c_cflag |= B1200;
                        break;
                case 10:
                        termout.c_cflag |= B1800;
                        break;
                case 11:
                        termout.c_cflag |= B2400;
                        break;
                case 12:
                        termout.c_cflag |= B4800;
                        break;
                case 13:
                        termout.c_cflag |= EXTA;   /* 19200 */
                        break;
                case 14:
                        termout.c_cflag |= EXTB;   /* 38400 */
                        break;
                case 15:
                default:
                        termout.c_cflag |= B9600;
                        break;
                }

                /* Loop through character sizes */
                for(c=5;c<=8;c++) {
                         switch (c)
                         {
                         case 5:
                                 termout.c_cflag |= CS5;
                                 break;
                         case 6:
                                 termout.c_cflag |= CS6;
                                 break;
                         case 7:
                                 termout.c_cflag |= CS7;
                                 break;
                         case 8:
                         default:
                                 termout.c_cflag |= CS8;
                                 break;
                        }

                        /* Loop through stop bits */
                        for(s=2;s>=1;s--) {
                                switch (s)
                                {
                                case 2:
                                        termout.c_cflag &= ~CSTOPB;
                                        break;
                                case 1:
                                default:
                                        termout.c_cflag |= CSTOPB;
                                        break;
                                }


                                /* Loop through parity */
                                for(p=1;p<=3;p++) {
                                        switch (p)
                                        {
                                        case 1:     /* Odd parity */
                                                termout.c_cflag |=
                                                            PARENB | PARODD;
                                                termout.c_iflag |= INPCK;
                                                break;
                                        case 2:     /* Even parity */
                                                termout.c_cflag |= PARENB;
                                                termout.c_iflag |= INPCK;
                                                break;
                                        case 3:     /* No parity */
                                        default:    /* No parity, default  */
                                                termout.c_cflag &=
                                                            ~(PARENB | PARODD);
                                                termout.c_iflag &= ~INPCK;
                                                break;
                                        }


                                        /* Call driver to set the new values  */
                                      if ((ioctl (fdes, TCSETA, &termout)) != 0) {
                                                err(0x26,0,errno);
                                                return (FAILED);
                                      }

                                        /* Get new values */
                                       if ((ioctl (fdes, TCGETA, &termin)) != 0) {
                                                err(0x27,0,errno);
                                                return (FAILED);
                                       }

                                        /* Compare the values */
                                        if (!(termout.c_cflag == termin.c_cflag)) {
                                                err(0x28,0,0);
                                                return (1);
                                        }

                                } /* parity loop */
                        } /* stop bits loop */
                } /* char size loop */
        } /* baud loop */


        /* Restore original settings */
        if ((ioctl (fdes, TCSETAW, &tty_attributes)) != 0) {
                err(0x29,0,errno);
                return (FAILED);
        }

        return (PASSED);
}  /* register_chk end */

/*
 * NAME:  vpd_chk_tst
 *
 * FUNCTION:  Call sysconfig to get VPD from driver.
 *
 * EXECUTION ENVIRONMENT:  User level task.
 *
 * NOTES:  This test is required to determine what kind of adapter is out
 *      there.  Nothing can be tested if this does't work.  CRC of VPD is
 *      also checked here.  The adapter type and interface type are set by
 *      this function based on the contents of the VPD.
 *
 * RETURNS:  0  test successful
 *          -1  ioctl failed
 *           2  unknown adapter
 *           3  controller/adapter VPD test failed
 *           5  concentrator VPD test failed
 */
vpd_chk_tst (fdes, tucb_ptr, action)
int     fdes;
struct  tucb_data  *tucb_ptr;
int     action;
{
        char    rel0[] = "040F9772";    /* Release 1 MIL188 8port w/o parity */
        char    rel1[] = "081F8111";    /* Release 1 MIL188 8port w/o parity */
        char    sio4[] = "7008";        /* Machine type */
        char    buf[256+8];             /* Buffer for POS regs and VPD  */
        int     bufsize = sizeof (buf); /* Buffer size for POS regs & VPD */
        struct  cfg_dd  cfg_dd;         /* Command struct to request VPD     */
        struct  stat    sbuf;                   /* fstat system call buffer  */
        int     crc;                            /* Calculated CRC of the VPD  */
        char    *vpd_data;                      /* Pointer to VPD data  */
        int     length = 0;                     /* Length of VPD data   */
        int     expected_crc;                   /* Expected CRC of the VPD  */
        int     vpd_rc = PASSED;                /* vpd test return code  */
        extern  unsigned short crc_gen();       /* Routine to generate CRC  */

        memset (buf, 0, sizeof (buf));
        if ((vpd_rc = fstat (fdes, &sbuf)) == FAILED) {
                err(0x30,vpd_rc,0);
                return (vpd_rc);
        }

        cfg_dd.kmid=loadext(tucb_ptr->ttycb.dev_drvr,FALSE,TRUE);

        cfg_dd.devno = sbuf.st_rdev;
        cfg_dd.cmd = CFG_QVPD;
        cfg_dd.ddsptr = buf;
        cfg_dd.ddslen = bufsize;
        if (sysconfig (SYS_CFGDD, &cfg_dd, sizeof (cfg_dd)) == CONF_FAIL) {
                err(0x31,0,0);
                return (vpd_rc = UNK_VPD);
        }
        switch (action)
        {
        case CONFIG:
                /* Determine what kind of adapter it is  */
                switch (buf[0])
                {
                case SIO:
                        tucb_ptr->ttycb.adapter = SIO;
                        break;
                case SIO2:
                        tucb_ptr->ttycb.adapter = SIO2;
                        break;
                case SIO3:
                        if ((strstr (buf+19, sio4)))
                                tucb_ptr->ttycb.adapter = SIO4;
                        else
                                tucb_ptr->ttycb.adapter = SIO3;
                        break;
                case P8RS232:
                        tucb_ptr->ttycb.adapter = P8RS232;
                        break;
                case P8RS422:
                        tucb_ptr->ttycb.adapter = P8RS422;
                        break;
                case P8RS188:
                        tucb_ptr->ttycb.adapter = P8RS188;
                        if ((strstr(buf+47, rel0)) || (strstr(buf+47, rel1)))
                                tucb_ptr->ttycb.mcparity = FALSE;
                        else
                                tucb_ptr->ttycb.mcparity = TRUE;
                        break;
                case P16RS232:
                        tucb_ptr->ttycb.adapter = P16RS232;
                        break;
                case P16RS422:
                        tucb_ptr->ttycb.adapter = P16RS422;
                        break;
                case P64RS232:
                        tucb_ptr->ttycb.adapter = P64RS232;
                        break;
                default:
                        tucb_ptr->ttycb.adapter = UNKNOWN;
                        vpd_rc = UNK_VPD;
                        break;
                }  /* end switch (buf[0]) */
                break;
        case TEST:
                /* Now check the CRC of the VPD.   Skip over POS regs less one
                   to account for missing byte 0 of VPD */
                expected_crc = (buf[6+7]<<8)+buf[7+7];
                length = ((buf[4+7]<<8)+buf[5+7])*2;
                vpd_data = &buf[8+7];
                /* Make sure length is reasonable  */
                if (length < 256)
                {
                        crc = crc_gen (vpd_data, (int)length);
                        if (crc != expected_crc) {
                                err(0x32,crc,0);
                                vpd_rc = ADP_VPD;
                        }
                }
                /*Length of VPD is ridiculous  */
                else {
                        err(0x33,0,0);
                        vpd_rc = ADP_VPD;
                }
                /* If 64 port check for concentrator too  */
                if (tucb_ptr->ttycb.adapter == P64RS232 && vpd_rc == PASSED)
                {
                        expected_crc = (buf[6+7+127]<<8)+buf[7+7+127];
                        length = ((buf[4+7+127]<<8)+buf[5+7+127])*2;
                        vpd_data = &buf[8+7+127];
                        /* If VPD is there and length makes sense check CRC */
                        if (expected_crc > 0 && length < 256)
                        {
                                /* Skip over POS regs and adapter VPD */
                                crc = crc_gen (vpd_data, (int)length);
                                if (crc != expected_crc) {
                                        err(0x34,crc,0);
                                        vpd_rc = CON_VPD;
                                }
                        }
                        else
                        {
                                /* Length is ridiculous */
                                if (expected_crc > 0 && length > 256) {
                                        err(0x35,0,0);
                                        vpd_rc = CON_VPD;
                                }
                        }
                }
                break;
        }  /* end switch (action) */
        return (vpd_rc);
}  /* vpd_chk_tst end */

/*
 * NAME: ram_tst
 *
 * FUNCTION: Have device driver run its DRAM test  (64 & 128 port only)
 *
 * EXECUTION ENVIRONMENT:  User level task.
 *
 * NOTES:
 *
 * RETURNS:  0  test successful
 *          -1  ioctl failed
 *           4  memory test failed
 */
ram_tst (fdes)
int     fdes;
{
        int     rt_rc = PASSED;

        sleep (2);
        if (ioctl (fdes, LI_DRAM, &rt_rc) != 0) {
                err(0x36,0,errno);
                return (rt_rc = MEM_TST);
        }
        /* Return code of 4 is acceptable because the current version of the
           test cannot guarantee it won't be interfered with.  Later device
           driver may fix this. */
        if (rt_rc == 4)
                rt_rc = PASSED;
        if (rt_rc != PASSED) {
                err(0x37,rt_rc,0);
                rt_rc = MEM_TST;
        }
        return (rt_rc);
}  /* ram_tst end */

/*
 * NAME:  lcl_loop
 *
 * FUNCTION:  Enable/disable device to run a local loop test.
 *
 * EXECUTION ENVIRONMENT:  User level task.
 *
 * NOTES:  This function is used to prepare an adapter to run a loop test or
 *       to indicate that the loop test mode is no longer needed.  In the case
 *       of the 64 port adapter this means that the synchronous interface to
 *       the concentrator will be tested and the driver must be prepared for
 *       that test.  For other adapters it means that the UART must be set
 *       into local loopback mode.
 *
 * RETURNS:  0  test successful
 *          -1  ioctl failed
 *           1  unable to execute because dd detected a hw error
 */
lcl_loop (fdes, tucb_ptr, param, saved_err)
int     fdes;
struct  tucb_data *tucb_ptr;
int     param;
int     saved_err;
{
        int     ll_rc = PASSED;
        int     io_rc = PASSED;

        if (tucb_ptr->ttycb.adapter == P64RS232)
        {
                if (param == SET)
                        io_rc = ioctl (fdes, LI_SLP1, &ll_rc);
                else
                        io_rc = ioctl (fdes, LI_SLP0, &ll_rc);
        }
        else
        {
                io_rc = ioctl (fdes, TCLOOP, &param);
        }
        if ((saved_err == 0 && ll_rc == 0) && (io_rc == FAILED)) {
                err(0x38,io_rc,errno);
                return (io_rc);
        }
        else
        {
                if (saved_err == 0 && ll_rc != 0) {
                       err(0x39,ll_rc,0);
                       ll_rc = DD_HW_F;
                }
                return (ll_rc);
        }
}  /* lcl_loop end */

/*
 * NAME:  data_wrap_tst
 *
 * FUNCTION:  Send data out to port and make sure it gets back.
 *
 * EXECUTION ENVIRONMENT:  User level task.
 *
 * NOTES:
 *
 * RETURNS:  0  test successful
 *          -1  ioctl failed
 *           2  data wrap test failed
 */
data_wrap_tst (fdes, tucb_ptr)
int     fdes;
struct  tucb_data *tucb_ptr;
{
        int     i;
        int     bytes_w, bytes_r, bytes_to_read, count;
        int     b_index, c_index, s_index, p_index;
        int     dw_rc = PASSED;
        char    in_buf[4096];
        char    mask;                   /* Mask for comparing received chrs  */
        int     char_size;              /* Size of data chars sent/received  */
        struct  termio  tty_attributes; /* duplicate structure for set_tty */
        unsigned int    atime;          /* computed alarm timeout in seconds */
        double  fbaud, fnchars, fchsize;
        int EagainCount;

        /* Get current settings  */
        if ((ioctl (fdes, TCGETA, &tty_attributes)) != 0) {
                err(0x40,0,errno);
                return (FAILED);
        }
        memset (in_buf, 0, sizeof (in_buf));
        /* Nested loop to vary:
                parity (most often)
                stop bits
                character size
                speed (least often) */
        b_index = 0;
        do
        { /* Speed */
                c_index = 0;
                do
                { /* Character size */
                /* Set mask for comparing received chars which accounts for
                   character size xmit'd/recv'd  */
                        char_size = tucb_ptr->ttycb.chars[c_index];
                        mask = char_size == 5 ? 0x1F :
                            char_size == 6 ? 0x3F :
                            char_size == 7 ? 0x7F :
                            0xFF;
                        s_index = 0;
                        /* calculate required alarm time * fudge factor */
                        fnchars = (double) tucb_ptr->ttycb.pat_size;
                        fbaud = (double) tucb_ptr->ttycb.baud[b_index];
                        fchsize = (double) tucb_ptr->ttycb.chars[c_index];
                        atime = (unsigned int)(fnchars/(fbaud/fchsize) *
                        TIME_FACTOR );
                        atime = (atime < 5) ? 5: atime;
                        do
                        { /* Stop bits */
                                p_index = 0;
                                do
                                { /* Parity */
                                        if ((dw_rc =
                                            set_tty (fdes, tucb_ptr, b_index,
                                            c_index, s_index, p_index)) != 0) {
                                                   err(0x41,dw_rc,0);
                                                   return (dw_rc);
                                        }
                                        (void) alarm (atime);
                                        if ((ioctl (fdes, TCFLSH, 2)) != 0) {
                                                err(0x42,0,errno);
                                                (void) alarm (OFF);
                                                if (errno == EINTR)
                                                        dw_rc = DWC_TST;
                                                else
                                                        dw_rc = FAILED;
                                                return (dw_rc);
                                        }
                                        bytes_w = write (fdes,
                                            tucb_ptr->ttycb.pattern,
                                            tucb_ptr->ttycb.pat_size);
                                        if (bytes_w !=
                                            tucb_ptr->ttycb.pat_size) {
                                                err(0x43,bytes_w,0);
                                                (void) alarm (OFF);
                                                return (dw_rc = DWC_TST);
                                        }

                                        (void) alarm (OFF);
                                        for (i = 0;
                                            i < tucb_ptr->ttycb.pat_size; i++)
                                                in_buf[i] = 0;
                                        bytes_to_read = bytes_w;
                                        i = 0;
                                        count = 0;
                                        /* Collect returned data  */
                                        EagainCount = 0;
                                        while (bytes_to_read > 0)
                                        {

                                               (void) alarm (atime);
                                                bytes_r = read (fdes,
                                                    &in_buf[i], bytes_to_read);
                                                (void) alarm (OFF);
                                                if (bytes_r < 0) {
                                                    if (errno != EAGAIN) {
                                                        err(0x44,0,errno);
                                                        if (errno == EINTR)
                                                            dw_rc = DWC_TST;
                                                        else
                                                            dw_rc = FAILED;
                                                        return (dw_rc);
                                                    }
                                                    else {
                                                        EagainCount++;
                                                        if (EagainCount<20){
                                                            sleep (1);
                                                        }
                                                        else {
                                                            err(0x45,0,0);
                                                            dw_rc = DWC_TST;
                                                            return (dw_rc);
                                                        }
                                                    }
                                                }
                                                /* If no chars were received,
                                                   delay about one second.
                                                   Otherwise reset count of
                                                   consecutive no data reads.
                                                   If this happens too many
                                                   times break loop. */
                                                else if (bytes_r == 0)
                                                {
                                                        if (count == 20)
                                                                break;
                                                        count++;
                                                        sleep (1);
                                                }
                                                else {
                                                    count = 0;
                                                    bytes_to_read =
                                                      bytes_to_read - bytes_r;
                                                    i += bytes_r;
                                                }
                                        }
                                        /* Did it all come back? */
                                        if (i != tucb_ptr->ttycb.pat_size) {
                                                err(0x46,i,0);
                                                return (dw_rc = DWC_TST);
                                        }
                                        /* Did we actually receive what we
                                           sent ? */
                                        for (i = 0;
                                            i < tucb_ptr->ttycb.pat_size;)
                                        {
                                                if (in_buf[i] == (mask &
                                                tucb_ptr->ttycb.pattern[i]))
                                                        i++;
                                                else {
                                                        err(0x47,i,0);
                                                        return (dw_rc =
                                                            DWC_TST);
                                                }
                                        }
                                        /* Data wrapped OK, try next parity
                                           setting  */
                                        p_index++;
                                }
                                while (tucb_ptr->ttycb.parity[p_index] !=
                                    STOP && p_index < MAX_PARITY);
                                /* Try next stop bits setting  */
                                s_index++;
                        }
                        while (tucb_ptr->ttycb.sbits[s_index] != STOP &&
                            s_index < MAX_SBITS);
                        /* Try next character size setting  */
                        c_index++;
                }
                while (tucb_ptr->ttycb.chars[c_index] != STOP &&
                    c_index < MAX_CHARSZ);
                /* Try next speed setting  */
                b_index++;
        }
        while (tucb_ptr->ttycb.baud[b_index] != STOP && b_index < MAX_BAUD);
        if ((ioctl (fdes, TCSETAW, &tty_attributes)) != 0) {
                err(0x48,0,errno);
                return (FAILED);
        }
        return (PASSED);
}  /* data_wrap_tst end */

/*
 * NAME:  peg_data_wrap_tst
 *
 * FUNCTION:  Send data out to port and make sure it gets back.
 *
 * EXECUTION ENVIRONMENT:  User level task.
 *
 * NOTES: Specific function for Pegasus native lines in local loop mode.
 *        This is a modified copy of data_wrap_tst function.
 *        The UART of Pegasus native lines 1 and 2 disables interrupts
 *        when TCLOOP is set.
 *        TCLOOP is cleared after data are written to the line:
 *        this enables backs interrupts and makes reading back datas possible.
 *        Write length must not exceed the UART buffer size.
 *        This function is only mandatory for lines 1 and 2, but
 *        is also applied to line 3 (which uses a different UART).
 *
 * RETURNS:  0  test successful
 *          -1  ioctl failed
 *           2  data wrap test failed
 */
peg_data_wrap_tst (fdes, tucb_ptr)
int     fdes;
struct  tucb_data *tucb_ptr;
{
        int     i;
        int     bytes_w, bytes_r, bytes_to_read, count;
        int     b_index, c_index, s_index, p_index;
        int     dw_rc = PASSED;
        char    in_buf[4096];
        char    mask;                   /* Mask for comparing received chrs  */
        int     char_size;              /* Size of data chars sent/received  */
        struct  termio  tty_attributes; /* duplicate structure for set_tty */
        unsigned int    atime;          /* computed alarm timeout in seconds */
        double  fbaud, fnchars, fchsize;
        int EagainCount;
	int     tu_rc = PASSED;
	int param;
#define	SZ_PEG_PATTERN	16	/* UART buffer size */
	static char peg_pattern[SZ_PEG_PATTERN] = {
						0x01,0x02,0x04,0x08,
						0x10,0x20,0x40,0x80,
			 	 		0x55,0xAA,0x0f,0xf0,
						0x33,0xcc,0xc3,0x3c };

        /* Get current settings  */
        if ((ioctl (fdes, TCGETA, &tty_attributes)) != 0) {
                err(0x251,0,errno);
                return (FAILED);
        }
        memset (in_buf, 0, sizeof (in_buf));
        /* Nested loop to vary:
                parity (most often)
                stop bits
                character size
                speed (least often) */
        b_index = 0;

        do
        { /* Speed */
                c_index = 0;
                do
                { /* Character size */
                /* Set mask for comparing received chars which accounts for
                   character size xmit'd/recv'd  */
                        char_size = tucb_ptr->ttycb.chars[c_index];
                        mask = char_size == 5 ? 0x1F :
                            char_size == 6 ? 0x3F :
                            char_size == 7 ? 0x7F :
                            0xFF;
                        s_index = 0;
                        /* calculate required alarm time * fudge factor */
                        fnchars = (double) SZ_PEG_PATTERN;
                        fbaud = (double) tucb_ptr->ttycb.baud[b_index];
                        fchsize = (double) tucb_ptr->ttycb.chars[c_index];
                        atime = (unsigned int)(fnchars/(fbaud/fchsize) *
                        TIME_FACTOR );
                        atime = (atime < 5) ? 5: atime;
                        do
                        { /* Stop bits */
                                p_index = 0;
                                do
                                { /* Parity */

                                        /* Collect returned data  */

                                        if ((dw_rc =
                                            set_tty (fdes, tucb_ptr, b_index,
                                            c_index, s_index, p_index)) != 0) {
                                                   err(0x252,dw_rc,0);
                                                   return (dw_rc);
                                        }
                                        (void) alarm (atime);
                                        if ((ioctl (fdes, TCFLSH, 2)) != 0) {
                                                err(0x253,0,errno);
                                                (void) alarm (OFF);
                                                if (errno == EINTR)
                                                        dw_rc = DWC_TST;
                                                else
                                                        dw_rc = FAILED; 
                                                return (dw_rc);
                                        }
					/* Set TCLOOP mode */
					param = SET;
                			tu_rc = ioctl (fdes, TCLOOP, &param);
        				if (tu_rc != PASSED) {
                				err(0x254,tu_rc,errno);
                				dw_rc = FAILED;
						param = CLEAR;
                				(void)ioctl (fdes, TCLOOP,
								&param);
                                                return (dw_rc);
       					}

                                        bytes_w = write (fdes,
                                             peg_pattern, SZ_PEG_PATTERN);
                                        if (bytes_w != SZ_PEG_PATTERN) {
                                                err(0x255,bytes_w,0);
                                                (void) alarm (OFF);
						param = CLEAR;
                				(void)ioctl (fdes, TCLOOP,
								&param);
                                                return (dw_rc = DWC_TST);
                                        }

                                        (void) alarm (OFF);
                                        sleep(1); /* to ensure data are written
						     before clearing TCLOOP */

					/* Clear TCLOOP mode,
					   to enable interrupts back */
					param = CLEAR;
                			(void) ioctl (fdes, TCLOOP, &param);

                                        for (i = 0;
                                            i < SZ_PEG_PATTERN; i++)
                                                in_buf[i] = 0;
                                        bytes_to_read = bytes_w;
                                        i = 0;
                                        count = 0;

                                        /* Collect returned data  */
                                        EagainCount = 0;
                                        while (bytes_to_read > 0)
                                        {

                                               (void) alarm (atime);
                                                bytes_r = read (fdes,
                                                    &in_buf[i], bytes_to_read);
                                                (void) alarm (OFF);
                                                if (bytes_r < 0) {
                                                    if (errno != EAGAIN) {
                                                        err(0x256,0,errno);
                                                        if (errno == EINTR)
                                                            dw_rc = DWC_TST;
                                                        else
                                                            dw_rc = FAILED;
                                                        return (dw_rc);
                                                    }
                                                    else {
                                                        EagainCount++;
                                                        if (EagainCount<20){
                                                            sleep (1);
                                                        }
                                                        else {
                                                            err(0x257,0,0);
                                                            dw_rc = DWC_TST;
                                                            return (dw_rc);
                                                        }
                                                    }
                                                }
                                                /* If no chars were received,
                                                   delay about one second.
                                                   Otherwise reset count of
                                                   consecutive no data reads.
                                                   If this happens too many
                                                   times break loop. */
                                                else if (bytes_r == 0)
                                                {
                                                        if (count == 20)
                                                                break;
                                                        count++;
                                                        sleep (1);
                                                }
                                                else {
                                                    count = 0;
                                                    bytes_to_read =
                                                      bytes_to_read - bytes_r;
                                                    i += bytes_r;
                                                }
                                        }
                                        /* Did it all come back? */
                                        if (i != SZ_PEG_PATTERN) {
                                                err(0x258,i,0);
                                                return (dw_rc = DWC_TST);
                                        }
                                        /* Did we actually receive what we
                                           sent ? */
                                        for (i = 0;
                                            i < SZ_PEG_PATTERN;)
                                        {
                                                if (in_buf[i] == (mask &
                                                 peg_pattern[i]))
                                                        i++;
                                                else {
                                                        err(0x259,i,0);
                                                        return (dw_rc =
                                                            DWC_TST);
                                                }
                                        }
                                        /* Data wrapped OK, try next parity
                                           setting  */
                                        p_index++;
                                }
                                while (tucb_ptr->ttycb.parity[p_index] !=
                                    STOP && p_index < MAX_PARITY);
                                /* Try next stop bits setting  */
                                s_index++;
                        }
                        while (tucb_ptr->ttycb.sbits[s_index] != STOP &&
                            s_index < MAX_SBITS);
                        /* Try next character size setting  */
                        c_index++;
                }
                while (tucb_ptr->ttycb.chars[c_index] != STOP &&
                    c_index < MAX_CHARSZ);
                /* Try next speed setting  */
                b_index++;
 	}
	while (tucb_ptr->ttycb.baud[b_index] != STOP && b_index < MAX_BAUD);
	if ((ioctl (fdes, TCSETAW, &tty_attributes)) != 0) {
                return (FAILED);
        }
        return (PASSED);
}  /* peg_data_wrap_tst end */

/*
 * NAME: ctrl_wrap_tst
 *
 * FUNCTION: Test modem control through various wrap configurations.
 *
 * EXECUTION ENVIRONMENT:  User level task.
 *
 * NOTES:  DTR and RTS are toggled through all possible combinations and the
 *      resulting modem status lines are checked.
 *              gmask = 0  DTR off and RTS off
 *              gmask = 2  DTR on and RTS off
 *              gmask = 4  DTR off and RTS on
 *              gmask = 6  DTR on and RTS on
 *
 *      The expected status is set in the init_mask function.
 *
 * RETURNS:  0  test successful
 *          -1  ioctl failed
 *           3  control wrap test failed
 */
ctrl_wrap_tst (fdes, tucb_ptr, param)
int     fdes;
struct  tucb_data *tucb_ptr;
int     param;
{
        int             gmask, result[4], i, j;
        int             cwt_rc = PASSED;
        /* Mask off DSR, RI & CTS */
        int             masktscd = TIOCM_DSR | TIOCM_CTS | TIOCM_RI;
        struct strioctl str_ioctl;
        static const int ctrl_cmd[4]={  0,
                                        TIOCM_DTR,
                                        TIOCM_RTS,
                                        TIOCM_RTS|TIOCM_DTR };
        int save_tiocmget;

        str_ioctl.ic_timout = 0;
        str_ioctl.ic_dp = (char *) &gmask;
        str_ioctl.ic_len = sizeof(int);

        /* saving line controls */
        str_ioctl.ic_cmd = TIOCMGET;
        if ((ioctl(fdes, I_STR, &str_ioctl)) != 0) {
                err(0x236,0,errno);
                return (FAILED);
        }
        save_tiocmget = gmask;

        /* For 64-port, 128-port (MCA & ISA), and 8-port (ISA) allow 2 seconds
	   for registers to settle,  and only test one time */
        if (tucb_ptr->ttycb.adapter == P64RS232 ||
            tucb_ptr->ttycb.adapter == P128RS232 ||
            tucb_ptr->ttycb.adapter == P8RS232ISA ||
            tucb_ptr->ttycb.adapter == P128RS232ISA) {

                /* Test all possible combinations of DTR and RTS */
                for (i = 0; i < 4; i++)
                {
                        gmask = ctrl_cmd[i];
                        str_ioctl.ic_cmd = TIOCMSET;
                        if ((ioctl (fdes, I_STR, &str_ioctl)) != 0) {
                                err(0x49,0,errno);
                                cwt_rc = FAILED;
                                break;
                        }
                        sleep (2);
                        str_ioctl.ic_cmd = TIOCMGET;
                        if ((ioctl (fdes, I_STR, &str_ioctl)) != 0) {
                                err(0x50,0,errno);
                                cwt_rc = FAILED;
                                break;
                        }
                        result[i] = gmask & masktscd;
                }
                /* Verify status values for all combinations  */
                for (i = 0; cwt_rc == PASSED && i < 4; i++) {
                        if (result[i] != wrap[param].result[i]) {
                                if (tucb_ptr->ttycb.pinout_conv == 0) {
                                        err(0x51,i,result[i]);
                                        cwt_rc = MCL_TST;
                                }
                                else {
                                        err(0x52,i,result[i]);
                                        cwt_rc = PCC_TST; /* Pinout converter */
                                }
                        }
                }
        }  /* endif */

        /* For 8 and 16 port, loop 200 times with a 100 microsecond delay */
        else {

                /* Loop 200 times for intermittent or late-failing adapters */

                for(j = 0; (j < 200); j++)
                {

                        /* Test all possible combinations of DTR and RTS */
                        for (i = 0; i < 4; i++)
                        {
                                gmask = ctrl_cmd[i];
                                str_ioctl.ic_cmd = TIOCMSET;
                                if ((ioctl (fdes, I_STR, &str_ioctl)) != 0) {
                                        err(0x53,0,errno);
                                        cwt_rc = FAILED;
                                        break;
                                }
                                usleep (100);
                                str_ioctl.ic_cmd = TIOCMGET;
                                if ((ioctl (fdes, I_STR, &str_ioctl)) != 0) {
                                        err(0x54,0,errno);
                                        cwt_rc = FAILED;
                                        break;
                                }
                                result[i] = gmask & masktscd;
                        }
                        /* Verify status values for all combinations  */
                        for (i = 0; cwt_rc == PASSED && i < 4; i++) {
                                if (result[i] != wrap[param].result[i]) {
                                        err(0x55,i,result[i]);
                                        cwt_rc = MCL_TST;
                                }
                        }
                        if (cwt_rc != PASSED)
                                break;
                }
        }
        /* restoring line controls */
        str_ioctl.ic_cmd = TIOCMSET;
        gmask = save_tiocmget;
        if ((ioctl(fdes, I_STR, &str_ioctl)) != 0) {
                err(0x237,0,errno);
                return (FAILED);
        }
        return (cwt_rc);
}  /* ctrl_wrap_tst end */


/*
 * NAME:  sync_wrap_tst
 *
 * FUNCTION:  64 port sync interface check.
 *
 * EXECUTION ENVIRONMENT:  User level task.
 *
 * NOTES:  This function provides an interface to the 64 port driver's synch-
 *      ronous interface test.  Prior to calling this function the adapter must
 *      have loop test mode set. (see lcl_loop)
 *
 * RETURNS:  0  test successful
 *          -1  test failed
 */
sync_wrap_tst (fdes)
int     fdes;
{
        int     i;
        int     swt_rc = PASSED;
        struct  slp_data slp;

        /* Initialize the test pattern, counts, and result  */
        for (i = 0; i < 16; i++)
                slp.out_data[i] = 48 + i;
        slp.in_count = 0;
        slp.out_count = 16;
        slp.result = 0;
        /* Request test with ioctl.  Run timer to prevent possible hangs. */
        (void) alarm (MAXTIME);
        if (ioctl (fdes, LI_DSLP, &slp) != PASSED) {
                err(0x56,0,errno);
                (void) alarm (OFF);
                return (swt_rc = SYN_TST);
        }
        /* Test complete, stop timer and check results  */
        (void) alarm (OFF);
        if (slp.result != 0) {
                err(0x57,slp.result,0);
                return (swt_rc = SYN_TST);
        }
        /* Verify wrap data  */
        for (i = 0; i < 16; i++)
                if (slp.in_data[i] != slp.out_data[i]) {
                        err(0x58,i,0);
                        return (swt_rc = SYN_TST);
                }
        return (swt_rc);
}  /* sync_wrap_tst end */

/*
 * NAME:  set_tty
 *
 * FUNCTION:  Set termio attributes for port being tested.
 *
 * EXECUTION ENVIRONMENT:  User level task.
 *
 * NOTES:  This function sets up a termio structure with the desired param-
 *      eters for a test and issues the ioctl to put those parameters into
 *      effect.
 *
 * RETURNS:  0  successful
 *          -1  failed
 */
set_tty (fdes, tucb_ptr, b_index, c_index, s_index, p_index)
int     fdes;
struct  tucb_data *tucb_ptr;
int     b_index, c_index, s_index, p_index;
{
        struct termio   terms;
        int             rc;

        /* Get current settings  */
        if ((ioctl (fdes, TCGETA, &terms)) != 0) {
                err(0x59,0,errno);
                return (FAILED);
        }
        /* Turn off all the input and output processing  */
        terms.c_lflag = 0;
        terms.c_cc[VMIN] = 0;
        terms.c_cc[VTIME] = 0;
        terms.c_iflag = 0;
        terms.c_oflag = 0;
        /* Set values for hardware characteristics  */
        terms.c_cflag = CREAD | CLOCAL;
        set_baud (&terms, tucb_ptr->ttycb.baud[b_index]);
        set_chars (&terms, tucb_ptr->ttycb.chars[c_index]);
        set_sbits (&terms, tucb_ptr->ttycb.sbits[s_index]);
        set_parity (&terms, tucb_ptr->ttycb.parity[p_index]);
        /* Call device drivers to set the new values  */
        if ((ioctl (fdes, TCSETAW, &terms)) != 0) {
                err(0x60,0,errno);
                return (FAILED);
        }
        return (PASSED);
}  /* set_tty end */

/*
 * NAME:  set_baud
 *
 * FUNCTION:  Set line speed
 *
 * EXECUTION ENVIRONMENT:  User level task.
 *
 * NOTES:  Sets desired speed in the termio flags.
 *
 * RETURNS:  None.
 */
set_baud (termio_ptr, baud)
struct  termio  *termio_ptr;
int     baud;
{
        switch (baud)
        {
        case 50:
                (*termio_ptr).c_cflag |= B50;
                break;
        case 75:
                (*termio_ptr).c_cflag |= B75;
                break;
        case 110:
                (*termio_ptr).c_cflag |= B110;
                break;
        case 134:
                (*termio_ptr).c_cflag |= B134;
                break;
        case 150:
                (*termio_ptr).c_cflag |= B150;
                break;
        case 200:
                (*termio_ptr).c_cflag |= B200;
                break;
        case 300:
                (*termio_ptr).c_cflag |= B300;
                break;
        case 600:
                (*termio_ptr).c_cflag |= B600;
                break;
        case 1200:
                (*termio_ptr).c_cflag |= B1200;
                break;
        case 1800:
                (*termio_ptr).c_cflag |= B1800;
                break;
        case 2400:
                (*termio_ptr).c_cflag |= B2400;
                break;
        case 4800:
                (*termio_ptr).c_cflag |= B4800;
                break;
        case 19200:
                (*termio_ptr).c_cflag |= EXTA;
                break;
        case 38400:
                (*termio_ptr).c_cflag |= EXTB;
                break;
        case 9600:
        default:
                (*termio_ptr).c_cflag |= B9600;
                break;
        }
        return;
}  /* set_baud end */

/*
 * NAME:  set_chars
 *
 * FUNCTION:  Set character size.
 *
 * EXECUTION ENVIRONMENT:  User level task.
 *
 * NOTES:  Sets desired character size in the termio flags.
 *
 * RETURNS:  None.
 */
set_chars (termio_ptr, chars)
struct  termio  *termio_ptr;
int     chars;
{
        switch (chars)
        {
        case 5:
                (*termio_ptr).c_cflag |= CS5;
                break;
        case 6:
                (*termio_ptr).c_cflag |= CS6;
                break;
        case 7:
                (*termio_ptr).c_cflag |= CS7;
                break;
        case 8:
        default:
                (*termio_ptr).c_cflag |= CS8;
                break;
        }
        return;
}  /* set_chars end */

/*
 * NAME:  set_sbits
 *
 * FUNCTION:  Set stop bits
 *
 * EXECUTION ENVIRONMENT:  User level task.
 *
 * NOTES:  Sets desired number of stop bits in the termio flags.
 *
 * RETURNS:  None.
 */
set_sbits (termio_ptr, sbits)
struct  termio  *termio_ptr;
int     sbits;
{
        switch (sbits)
        {
        case 2:
                (*termio_ptr).c_cflag &= ~CSTOPB;
                break;
        case 1:
        default:
                (*termio_ptr).c_cflag |= CSTOPB;
                break;
        }
        return;
}  /* set_sbits end */

/*
 * NAME:  set_parity
 *
 * FUNCTION:  Set parity.
 *
 * EXECUTION ENVIRONMENT:  User level task.
 *
 * NOTES:  Sets desired parity in the termio flags.
 *
 * RETURNS:  None.
 */
set_parity (termio_ptr, parity)
struct  termio  *termio_ptr;
int     parity;
{
        switch (parity)
        {
        case 1:     /* Odd parity */
                (*termio_ptr).c_cflag |= PARENB | PARODD;
                (*termio_ptr).c_iflag |= INPCK;
                break;
        case 2:     /* Even parity */
                (*termio_ptr).c_cflag |= PARENB;
                (*termio_ptr).c_iflag |= INPCK;
                break;
        case 3:     /* No parity */
        default:    /* No parity, default  */
                (*termio_ptr).c_cflag &= ~(PARENB | PARODD);
                (*termio_ptr).c_iflag &= ~INPCK;
                break;
        }
        return;
}  /* set_parity end */

/*
 * NAME:  init_mask
 *
 * FUNCTION:  Initialize masks for control wrap tests.
 *
 * EXECUTION ENVIRONMENT:  User level task.
 *
 * NOTES:  This function merely initializes a table of expected results for
 *         the control wrap tests.  There are 4 results for each test which can
 *         be run.  They correspond to the following control lead settings:
 *              0 - DTR off and RTS off
 *              2 - DTR on  and RTS off
 *              4 - DTR off and RTS on
 *              6 - DTR on  and RTS on
 *
 *                     2  1
 *                     5  2 6 3 1
 *      - - - -  - - - 6  8 4 2 6  8 4 2 1
 *                     ³  ³ ³ ³ ³  ³ ³ ³ ³
 *	0 0 0 0  0 0 0 0  0 0 0 0  0 0 0 0
 *                     ³  ³ ³ ³ ³  ³ ³ ³ ÀÄÄ TIOCM_LE	0x0001
 *                     ³  ³ ³ ³ ³  ³ ³ ÀÄÄÄÄ TIOCM_DTR	0x0002
 *                     ³  ³ ³ ³ ³  ³ ÀÄÄÄÄÄÄ TIOCM_RTS	0x0004
 *                     ³  ³ ³ ³ ³  ÀÄÄÄÄÄÄÄÄ TIOCM_ST	0x0008
 *                     ³  ³ ³ ³ ÀÄÄÄÄÄÄÄÄÄÄÄ TIOCM_SR	0x0010
 *                     ³  ³ ³ ÀÄÄÄÄÄÄÄÄÄÄÄÄÄ TIOCM_CTS	0x0020
 *                     ³  ³ ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ TIOCM_CD	0x0040
 *                     ³  ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ TIOCM_RI	0x0080
 *                     ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ TIOCM_DSR	0x0100
 *
 *      LOCAL = 0  DRIVER = 1  CABLE = 2 FANOUT = 3  INTERP = 4
 *
 * RETURNS:     None
 */
void    init_mask (tucb_ptr)
struct  tucb_data *tucb_ptr;
{
        switch (tucb_ptr->ttycb.adapter)
        {
        case SIO:
        case SIO2:
                wrap[LOCAL].result[0]  = 0;
                wrap[LOCAL].result[1]  = TIOCM_DSR;
                wrap[LOCAL].result[2]  = TIOCM_CTS;
                wrap[LOCAL].result[3]  = TIOCM_DSR | TIOCM_CTS;
                wrap[DRIVER].result[0] = 0;
                wrap[DRIVER].result[1] = TIOCM_DSR;
                wrap[DRIVER].result[2] = TIOCM_RI | TIOCM_CTS;
                wrap[DRIVER].result[3] = TIOCM_DSR | TIOCM_CTS;
                wrap[CABLE].result[0]  = 0;
                wrap[CABLE].result[1]  = TIOCM_DSR;
                wrap[CABLE].result[2]  = TIOCM_RI | TIOCM_CTS;
                wrap[CABLE].result[3]  = TIOCM_DSR | TIOCM_CTS;
                wrap[FANOUT].result[0] = 0;
                wrap[FANOUT].result[1] = TIOCM_DSR;
                wrap[FANOUT].result[2] = TIOCM_RI | TIOCM_CTS;
                wrap[FANOUT].result[3] = TIOCM_DSR | TIOCM_CTS;
                wrap[INTERP].result[0] = 0;
                wrap[INTERP].result[1] = TIOCM_DSR;
                wrap[INTERP].result[2] = TIOCM_CTS;
                wrap[INTERP].result[3] = TIOCM_DSR | TIOCM_CTS;
                break;
        case SIO3:
        case SIO4:
        case SIO5:
        case SIO6:
        case SIO7:
	case SIO8:
	case SIO9:
	case SIO10:
                wrap[LOCAL].result[0]  = 0;
                wrap[LOCAL].result[1]  = TIOCM_DSR;
                wrap[LOCAL].result[2]  = TIOCM_CTS;
                wrap[LOCAL].result[3]  = TIOCM_DSR | TIOCM_CTS;
                wrap[DRIVER].result[0] = 0;
                wrap[DRIVER].result[1] = TIOCM_DSR;
                wrap[DRIVER].result[2] = TIOCM_RI | TIOCM_CTS;
                wrap[DRIVER].result[3] = TIOCM_DSR | TIOCM_CTS | TIOCM_RI;
                wrap[CABLE].result[0]  = 0;
                wrap[CABLE].result[1]  = TIOCM_DSR;
                wrap[CABLE].result[2]  = TIOCM_RI | TIOCM_CTS;
                wrap[CABLE].result[3] = TIOCM_DSR | TIOCM_CTS | TIOCM_RI;
                wrap[FANOUT].result[0] = 0;
                wrap[FANOUT].result[1] = TIOCM_DSR;
                wrap[FANOUT].result[2] = TIOCM_RI | TIOCM_CTS;
                wrap[FANOUT].result[3] = TIOCM_DSR | TIOCM_CTS;
                wrap[INTERP].result[0] = 0;
                wrap[INTERP].result[1] = TIOCM_DSR;
                wrap[INTERP].result[2] = TIOCM_CTS;
                wrap[INTERP].result[3] = TIOCM_DSR | TIOCM_CTS;
                break;
        case P8RS232:
        case P8RS232ISA:
                wrap[LOCAL].result[0]  = 0;
                wrap[LOCAL].result[1]  = TIOCM_DSR;
                wrap[LOCAL].result[2]  = TIOCM_CTS;
                wrap[LOCAL].result[3]  = TIOCM_DSR | TIOCM_CTS;
                wrap[DRIVER].result[0] = 0;
                wrap[DRIVER].result[1] = TIOCM_RI | TIOCM_DSR;
                wrap[DRIVER].result[2] = TIOCM_CTS;
                wrap[DRIVER].result[3] = TIOCM_RI | TIOCM_DSR | TIOCM_CTS;
                wrap[CABLE].result[0]  = 0;
                wrap[CABLE].result[1]  = TIOCM_DSR;
                wrap[CABLE].result[2]  = TIOCM_RI | TIOCM_CTS;
                wrap[CABLE].result[3]  = TIOCM_RI | TIOCM_DSR | TIOCM_CTS;
                wrap[FANOUT].result[0] = 0;
                wrap[FANOUT].result[1] = TIOCM_DSR;
                wrap[FANOUT].result[2] = TIOCM_RI | TIOCM_CTS;
                wrap[FANOUT].result[3] = TIOCM_RI | TIOCM_DSR | TIOCM_CTS;
                wrap[INTERP].result[0] = 0;
                wrap[INTERP].result[1] = TIOCM_DSR;
                wrap[INTERP].result[2] = TIOCM_CTS;
                wrap[INTERP].result[3] = TIOCM_DSR | TIOCM_CTS;
                break;
        case P8RS422:
                wrap[LOCAL].result[0]  = 0;
                wrap[LOCAL].result[1]  = TIOCM_DSR;
                wrap[LOCAL].result[2]  = TIOCM_CTS;
                wrap[LOCAL].result[3]  = TIOCM_DSR | TIOCM_CTS;
                wrap[DRIVER].result[0] = TIOCM_DSR | TIOCM_CTS;
                wrap[DRIVER].result[1] = TIOCM_DSR | TIOCM_CTS;
                wrap[DRIVER].result[2] = TIOCM_DSR | TIOCM_CTS;
                wrap[DRIVER].result[3] = TIOCM_DSR | TIOCM_CTS;
                wrap[CABLE].result[0]  = TIOCM_DSR | TIOCM_CTS;
                wrap[CABLE].result[1]  = TIOCM_DSR | TIOCM_CTS;
                wrap[CABLE].result[2]  = TIOCM_DSR | TIOCM_CTS;
                wrap[CABLE].result[3]  = TIOCM_DSR | TIOCM_CTS;
                wrap[FANOUT].result[0] = TIOCM_DSR | TIOCM_CTS;
                wrap[FANOUT].result[1] = TIOCM_DSR | TIOCM_CTS;
                wrap[FANOUT].result[2] = TIOCM_DSR | TIOCM_CTS;
                wrap[FANOUT].result[3] = TIOCM_DSR | TIOCM_CTS;
                wrap[INTERP].result[0] = TIOCM_DSR | TIOCM_CTS;
                wrap[INTERP].result[1] = TIOCM_DSR | TIOCM_CTS;
                wrap[INTERP].result[2] = TIOCM_DSR | TIOCM_CTS;
                wrap[INTERP].result[3] = TIOCM_DSR | TIOCM_CTS;
                break;
        case P8RS188:
                wrap[LOCAL].result[0]  = 0;
                wrap[LOCAL].result[1]  = TIOCM_DSR;
                wrap[LOCAL].result[2]  = TIOCM_CTS;
                wrap[LOCAL].result[3]  = TIOCM_DSR | TIOCM_CTS;
                wrap[DRIVER].result[0] = 0;
                wrap[DRIVER].result[1] = TIOCM_RI | TIOCM_DSR;
                wrap[DRIVER].result[2] = TIOCM_CTS;
                wrap[DRIVER].result[3] = TIOCM_RI | TIOCM_DSR | TIOCM_CTS;
                wrap[CABLE].result[0]  = 0;
                wrap[CABLE].result[1]  = TIOCM_DSR;
                wrap[CABLE].result[2]  = TIOCM_RI | TIOCM_CTS;
                wrap[CABLE].result[3]  = TIOCM_RI | TIOCM_DSR | TIOCM_CTS;
                wrap[FANOUT].result[0] = 0;
                wrap[FANOUT].result[1] = TIOCM_DSR;
                wrap[FANOUT].result[2] = TIOCM_RI | TIOCM_CTS;
                wrap[FANOUT].result[3] = TIOCM_RI | TIOCM_DSR | TIOCM_CTS;
                if (tucb_ptr->ttycb.mcparity == TRUE)
                {
                        wrap[INTERP].result[0] = 0;
                        wrap[INTERP].result[1] = TIOCM_DSR;
                        wrap[INTERP].result[2] = TIOCM_CTS;
                        wrap[INTERP].result[3] = TIOCM_DSR | TIOCM_CTS;
                }
                else
                {
                        wrap[INTERP].result[0] = TIOCM_RI;
                        wrap[INTERP].result[1] = TIOCM_RI | TIOCM_DSR;
                        wrap[INTERP].result[2] = TIOCM_RI | TIOCM_CTS;
                        wrap[INTERP].result[3] = TIOCM_RI | TIOCM_DSR | TIOCM_CTS;
                }
                if (tucb_ptr->ttycb.prtr_att == TRUE)
                {
                        wrap[DRIVER].result[0] = 0;
                        wrap[DRIVER].result[1] = TIOCM_RI | TIOCM_DSR;
                        wrap[CABLE].result[0]  = 0;
                        wrap[CABLE].result[2]  = TIOCM_RI | TIOCM_CTS;
                        wrap[FANOUT].result[0] = 0;
                        wrap[FANOUT].result[2] = TIOCM_RI | TIOCM_CTS;
                        if (tucb_ptr->ttycb.mcparity == TRUE)
                        {
                                wrap[INTERP].result[0] = 0;
                                wrap[INTERP].result[2] = TIOCM_CTS;
                        }
                        else
                        {
                                wrap[INTERP].result[0] = TIOCM_RI;
                                wrap[INTERP].result[2] = TIOCM_RI | TIOCM_CTS;
                        }
                }
                break;
        case P16RS232:
                wrap[LOCAL].result[0]  = 0;
                wrap[LOCAL].result[1]  = TIOCM_DSR;
                wrap[LOCAL].result[2]  = TIOCM_CTS;
                wrap[LOCAL].result[3]  = TIOCM_DSR | TIOCM_CTS;
                wrap[DRIVER].result[0] = TIOCM_DSR | TIOCM_CTS;
                wrap[DRIVER].result[1] = TIOCM_DSR | TIOCM_CTS;
                wrap[DRIVER].result[2] = TIOCM_DSR | TIOCM_CTS;
                wrap[DRIVER].result[3] = TIOCM_DSR | TIOCM_CTS;
                wrap[CABLE].result[0]  = TIOCM_DSR | TIOCM_CTS;
                wrap[CABLE].result[1]  = TIOCM_DSR | TIOCM_CTS;
                wrap[CABLE].result[2]  = TIOCM_DSR | TIOCM_CTS;
                wrap[CABLE].result[3]  = TIOCM_DSR | TIOCM_CTS;
                wrap[FANOUT].result[0] = TIOCM_DSR | TIOCM_CTS;
                wrap[FANOUT].result[1] = TIOCM_DSR | TIOCM_CTS;
                wrap[FANOUT].result[2] = TIOCM_DSR | TIOCM_CTS;
                wrap[FANOUT].result[3] = TIOCM_DSR | TIOCM_CTS;
                wrap[INTERP].result[0] = TIOCM_DSR | TIOCM_CTS;
                wrap[INTERP].result[1] = TIOCM_DSR | TIOCM_CTS;
                wrap[INTERP].result[2] = TIOCM_DSR | TIOCM_CTS;
                wrap[INTERP].result[3] = TIOCM_DSR | TIOCM_CTS;
                if (tucb_ptr->ttycb.prtr_att == TRUE)
                {
                        wrap[LOCAL].result[0]  = 0;
                        wrap[LOCAL].result[1]  = TIOCM_DSR;
                        wrap[LOCAL].result[2]  = TIOCM_CTS;
                        wrap[LOCAL].result[3]  = TIOCM_DSR | TIOCM_CTS;
                        wrap[DRIVER].result[0] = TIOCM_DSR | TIOCM_CTS;
                        wrap[DRIVER].result[2] = TIOCM_DSR | TIOCM_CTS;
                        wrap[CABLE].result[0]  = TIOCM_DSR | TIOCM_CTS;
                        wrap[CABLE].result[2]  = TIOCM_DSR | TIOCM_CTS;
                        wrap[FANOUT].result[0] = TIOCM_DSR | TIOCM_CTS;
                        wrap[FANOUT].result[2] = TIOCM_DSR | TIOCM_CTS;
                        wrap[INTERP].result[0] = TIOCM_DSR | TIOCM_CTS;
                        wrap[INTERP].result[2] = TIOCM_DSR | TIOCM_CTS;
                }
                break;
        case P16RS422:
                wrap[LOCAL].result[0]  = 0;
                wrap[LOCAL].result[1]  = TIOCM_DSR;
                wrap[LOCAL].result[2]  = TIOCM_CTS;
                wrap[LOCAL].result[3]  = TIOCM_DSR | TIOCM_CTS;
                wrap[DRIVER].result[0] = TIOCM_DSR | TIOCM_CTS;
                wrap[DRIVER].result[1] = TIOCM_DSR | TIOCM_CTS;
                wrap[DRIVER].result[2] = TIOCM_DSR | TIOCM_CTS;
                wrap[DRIVER].result[3] = TIOCM_DSR | TIOCM_CTS;
                wrap[CABLE].result[0]  = TIOCM_DSR | TIOCM_CTS;
                wrap[CABLE].result[1]  = TIOCM_DSR | TIOCM_CTS;
                wrap[CABLE].result[2]  = TIOCM_DSR | TIOCM_CTS;
                wrap[CABLE].result[3]  = TIOCM_DSR | TIOCM_CTS;
                wrap[FANOUT].result[0] = TIOCM_DSR | TIOCM_CTS;
                wrap[FANOUT].result[1] = TIOCM_DSR | TIOCM_CTS;
                wrap[FANOUT].result[2] = TIOCM_DSR | TIOCM_CTS;
                wrap[FANOUT].result[3] = TIOCM_DSR | TIOCM_CTS;
                wrap[INTERP].result[0] = TIOCM_DSR | TIOCM_CTS;
                wrap[INTERP].result[1] = TIOCM_DSR | TIOCM_CTS;
                wrap[INTERP].result[2] = TIOCM_DSR | TIOCM_CTS;
                wrap[INTERP].result[3] = TIOCM_DSR | TIOCM_CTS;
                if (tucb_ptr->ttycb.prtr_att == TRUE)
                {
                        wrap[LOCAL].result[0] = 0;
                        wrap[LOCAL].result[1] = TIOCM_DSR;
                        wrap[LOCAL].result[2] = TIOCM_CTS;
                        wrap[LOCAL].result[3] = TIOCM_DSR | TIOCM_CTS;
                }
                break;
        case P64RS232:
                wrap[LOCAL].result[0]  = 0xff;
                wrap[LOCAL].result[1]  = 0xff;
                wrap[LOCAL].result[2]  = 0xff;
                wrap[LOCAL].result[3]  = 0xff;
                wrap[DRIVER].result[0] = 0xff;
                wrap[DRIVER].result[1] = 0xff;
                wrap[DRIVER].result[2] = 0xff;
                wrap[DRIVER].result[3] = 0xff;
                wrap[CABLE].result[0]  = 0;
                wrap[CABLE].result[1]  = TIOCM_DSR;
                wrap[CABLE].result[2]  = TIOCM_CTS;
                wrap[CABLE].result[3]  = TIOCM_DSR | TIOCM_CTS;
                wrap[FANOUT].result[0] = 0;
                wrap[FANOUT].result[1] = TIOCM_DSR;
                wrap[FANOUT].result[2] = TIOCM_CTS;
                wrap[FANOUT].result[3] = TIOCM_DSR | TIOCM_CTS;
                wrap[INTERP].result[0] = 0;
                wrap[INTERP].result[1] = TIOCM_DSR;
                wrap[INTERP].result[2] = TIOCM_CTS;
                wrap[INTERP].result[3] = TIOCM_DSR | TIOCM_CTS;
                break;
        case P128RS232:
        case P128RS232ISA:
                /* If testing the pinout converter on 128-port */
                if (tucb_ptr->ttycb.pinout_conv == 1) {
                        wrap[LOCAL].result[0]  = 0xff;
                        wrap[LOCAL].result[1]  = 0xff;
                        wrap[LOCAL].result[2]  = 0xff;
                        wrap[LOCAL].result[3]  = 0xff;
                        wrap[DRIVER].result[0] = 0xff;
                        wrap[DRIVER].result[1] = 0xff;
                        wrap[DRIVER].result[2] = 0xff;
                        wrap[DRIVER].result[3] = 0xff;
                        wrap[CABLE].result[0]  = 0;
                        wrap[CABLE].result[1]  = 0;
                        wrap[CABLE].result[2]  = TIOCM_CTS;
                        wrap[CABLE].result[3]  = TIOCM_CTS;
                        wrap[FANOUT].result[0] = 0;
                        wrap[FANOUT].result[1] = 0;
                        wrap[FANOUT].result[2] = TIOCM_CTS;
                        wrap[FANOUT].result[3] = TIOCM_CTS;
                        wrap[INTERP].result[0] = 0;
                        wrap[INTERP].result[1] = 0;
                        wrap[INTERP].result[2] = TIOCM_CTS;
                        wrap[INTERP].result[3] = TIOCM_CTS;
                }
                else {                  /* Regular 128-port testing */
                        wrap[LOCAL].result[0]  = 0xff;
                        wrap[LOCAL].result[1]  = 0xff;
                        wrap[LOCAL].result[2]  = 0xff;
                        wrap[LOCAL].result[3]  = 0xff;
                        wrap[DRIVER].result[0] = 0;
                        wrap[DRIVER].result[1] = TIOCM_DSR;
                        wrap[DRIVER].result[2] = TIOCM_CTS|TIOCM_RI;
                        wrap[DRIVER].result[3] = TIOCM_DSR|TIOCM_CTS|TIOCM_RI;
                        wrap[CABLE].result[0]  = 0;
                        wrap[CABLE].result[1]  = TIOCM_DSR;
                        wrap[CABLE].result[2]  = TIOCM_CTS|TIOCM_RI;
                        wrap[CABLE].result[3]  = TIOCM_DSR|TIOCM_CTS|TIOCM_RI;
                        wrap[FANOUT].result[0] = 0;
                        wrap[FANOUT].result[1] = TIOCM_DSR;
                        wrap[FANOUT].result[2] = TIOCM_CTS|TIOCM_RI;
                        wrap[FANOUT].result[3] = TIOCM_DSR|TIOCM_CTS|TIOCM_RI;
                        wrap[INTERP].result[0] = 0;
                        wrap[INTERP].result[1] = TIOCM_DSR;
                        wrap[INTERP].result[2] = TIOCM_CTS;
                        wrap[INTERP].result[3] = TIOCM_DSR|TIOCM_CTS;
                }
                break;
        }  /* end switch (tucb_ptr->ttycb.adapter) */
}  /* init_mask end */

/*
 * NAME:  salsetup
 *
 * FUNCTION:  Set tty attributes and interrupts.
 *
 * EXECUTION ENVIRONMENT:  User level task.
 *
 * NOTES:  This function sets the baud rate, character size, number of stop
 *      bits, parity and interrupts.  The registers are read and saved prior
 *      to setting the test parameters and are restored when the loopmode is
 *      cleared.
 *
 * RETURNS:  0  test successful
 *          -1  ioctl failed
 *           1  unable to execute because dd detected a hw error
 */
salsetup (fd, set_regs, port)
int     fd, set_regs, port;
{

        uchar   data;
        int     rc = 0;
        int     i;

        if (set_regs == SET)
        {
                if ((rc = rd_byte (fd, &icrdata[2], 0x31+port)) != 0) {
                        err(0x61,rc,0);
                        return (rc);
                }
                if ((rc = rd_byte (fd, &icrdata[3], 0x33+port)) != 0) {
                        err(0x62,rc,0);
                        return (rc);
                }
                usleep (2 * 1000);
                data = 0x80;    /* Set dlab latch to set baud rate */
                if ((rc = wr_byte (fd, &data, 0x33+port)) != 0) {
                        err(0x63,rc,0);
                        return (rc);
                }
                for (i=0; i<2; i++)
                {
                        if ((rc = rd_byte (fd, &icrdata[i], 0x30+port+i)) != 0) {
                                err(0x64,rc,0);
                                return (rc);
                        }
                }
                usleep (2 * 1000);
                data = 0x34;    /* set lower byte for 9600 */
                if ((rc = wr_byte (fd, &data, 0x30+port)) != 0) {
                        err(0x65,rc,0);
                        return (rc);
                }
                data = 0x00;    /* set upper byte for 9600 */
                if ((rc = wr_byte (fd, &data, 0x31+port)) != 0) {
                        err(0x66,rc,0);
                        return (rc);
                }
                data = 0x03;    /* dlab 0, 8 data bits, 1 stop, no parity */
                if ((rc = wr_byte (fd, &data, 0x33+port)) != 0) {
                        err(0x67,rc,0);
                        return (rc);
                }
                data = 0x00;    /* no interrupts, we will poll */
                rc = wr_byte (fd, &data, 0x31+port);
        }
        else
        {
                data = 0x80|icrdata[3]; /* Set dlab latch to set baud rate */
                if ((rc = wr_byte (fd, &data, 0x33+port)) != 0) {
                        err(0x68,rc,0);
                        return (rc);
                }
                for (i=0; i<2; i++)
                {
                        if ((rc = wr_byte (fd, &icrdata[i], 0x30+port+i)) != 0){
                                err(0x69,rc,0);
                                return (rc);
                        }
                }
                data = icrdata[3];      /* restore lc register */
                if ((rc = wr_byte (fd, &data, 0x33+port)) != 0) {
                        err(0x70,rc,0);
                        return (rc);
                }
                data = icrdata[2];      /* restore ie register */
                if ((rc = wr_byte (fd, &data, 0x31+port)) != 0) {
                        err(0x238,rc,0);
                }
        }
        return (rc);
}

/*
 * NAME:  salsetloop
 *
 * FUNCTION:  Enable/disable device to run a local loop test.
 *
 * EXECUTION ENVIRONMENT:  User level task.
 *
 * NOTES:  This function is used to set or reset the local loopback mode of the
 *      VLSI 16C552 DUART.  The register is read and saved prior to setting
 *      loop mode and restored when the loopmode is cleared.
 *
 * RETURNS:  0  test successful
 *          -1  ioctl failed
 *           1  unable to execute because dd detected a hw error
 */
salsetloop (fd, lp_mode, port)
int     fd, lp_mode, port;
{
        uchar   data;
        int     rc = 0;

        /* Set loop mode */
        if (lp_mode == SET)
        {
                if ((rc = rd_byte (fd, &mcrdata, 0x34+port)) != 0) {
                        err(0x71,rc,0);
                        return (rc);
                }
                usleep (2 * 1000);
                data = 0x10;
        }
        else
        {
                data = mcrdata&0x0f;
        }
        if ((rc = wr_byte (fd, &data, 0x34+port)) != 0) {
                err(0x239,rc,0);
        }
        usleep (10 * 1000);
        return (rc);
}

/*
 * NAME:  saldata_wrap_tst
 *
 * FUNCTION:  Send data out to port and make sure it gets back.
 *
 * EXECUTION ENVIRONMENT:  User level task.
 *
 * NOTES:
 *
 * RETURNS:  0  test successful
 *          -1  ioctl failed
 *           2  data wrap test failed
 */
saldata_wrap_tst (fd, port)
int     fd, port;
{
        uchar   data;
        int     i, rc = 0;
        static uchar    patterns[3] = {0x55, 0xaa, 0xff};

        for (i = 0; i < 3; i++)
        {
                data = patterns[i];             /* Send a data char */
                wr_byte (fd, &data, 0x30+port);
                usleep (2 * 1000);
                rd_byte (fd, &data, 0x30+port);
                if (data != patterns[i]) {       /* error occurred */
                        err(0x72,i,data);
                        return (rc = 2);
                }
        }
        return (rc);
}

/*
 * NAME: salctrl_wrap_tst
 *
 * FUNCTION: Test modem control through various wrap configurations.
 *
 * EXECUTION ENVIRONMENT:  User level task.
 *
 * NOTES:  DTR and RTS are toggled through all possible combinations and the
 *      resulting modem status lines are checked.
 *              gmask = 0  DTR off and RTS off
 *              gmask = 1  DTR on and RTS off
 *              gmask = 2  DTR off and RTS on
 *              gmask = 3  DTR on and RTS on
 *
 *      The expected status is set in the init_mask function.
 *
 * RETURNS:  0  test successful
 *          -1  ioctl failed
 *           3  control wrap test failed
 */
salctrl_wrap_tst (fd, port)
int     fd, port;
{
        uchar   data;
        int     rc = 0;

        /* Clear modem stat reg by reading it */
        rd_byte (fd, &data, 0x36+port);

        /* Set loop mode and turn on DTR */
        data = 0x11;
        wr_byte (fd, &data, 0x34+port);
        usleep (2000);          /* wait for it to loop to DSR and delta DSR */
        rd_byte (fd, &data, 0x36+port);
        if (data != 0x22) {       /* error occurred */
                err(0x73,data,0);
                return (rc = 3);
        }

        /* Set loop mode, turn on RTS and DTR */
        data = 0x13;
        wr_byte (fd, &data, 0x34+port);
        usleep (2000);          /* It should loop to DSR, CTS and delta CTS */
        rd_byte(fd, &data, 0x36+port);
        if (data != 0x31) {       /* Error occurred */
                err(0x74,data,0);
                return (rc = 3);
        }

        /* Set loop mode, turn on OUT1, RTS, DTR */
        data = 0x17;
        wr_byte (fd, &data, 0x34+port);
        usleep (2000);          /* It should loop OUT1 to RI */
        rd_byte (fd, &data, 0x36+port);
        if (data != 0x70) {
                err(0x75,data,0);
                return (rc = 3);
        }

        /* Set loop mode, turn off OUT1, Catch trailing edge of RI indicator */
        data = 0x13;
        wr_byte (fd, &data, 0x34+port);
        usleep (2000);          /* It should loop OUT1 to RI */
        rd_byte (fd, &data, 0x36+port);
        if (data != 0x34) {      /* Trailing edge detector for RI failed */
                err(0x76,data,0);
                return (rc = 3);
        }

        /* Set loop mode, turn on OUT2, OUT1, RTS, DTR */
        data = 0x1f;
        wr_byte (fd, &data, 0x34+port);
        usleep (2000);          /* It should loop */
        rd_byte (fd, &data, 0x36+port);
        if (data != 0xf8) {      /* Error occurred */
                err(0x77,data,0);
                rc = 3;
        }
        return (rc);
}

/*
 * NAME:  rd_byte
 *
 * FUNCTION:  read one byte of data at a time.
 *
 * EXECUTION ENVIRONMENT:  User level task.
 *
 * NOTES:
 *
 * RETURNS:  0  test successful
 *          -1  ioctl failed
 */
int     rd_byte (fd, pdata, addr)
int     fd;
uchar   *pdata;
uint    addr;
{
        MACH_DD_IO iob;
        int     rc = 0;

        iob.md_data = pdata;
        iob.md_incr = MV_BYTE;
        iob.md_size = sizeof (*pdata);
        iob.md_addr = addr;
        rc = ioctl (fd, MIOBUSGET, &iob);
        return (rc);
}

/*
 * NAME:  wr_byte
 *
 * FUNCTION:  write one byte of data at a time.
 *
 * EXECUTION ENVIRONMENT:  User level task.
 *
 * NOTES:
 *
 * RETURNS:  0  test successful
 *          -1  ioctl failed
 */
int     wr_byte (fd, pdata, addr)
int     fd;
uchar   *pdata;
uint    addr;
{
        MACH_DD_IO iob;
        int     rc = 0;

        iob.md_data = pdata;
        iob.md_incr = MV_BYTE;
        iob.md_size = sizeof (*pdata);
        iob.md_addr = addr;
        rc = ioctl (fd, MIOBUSPUT, &iob);
        return (rc);
}

/*
 * NAME:  vpd128_chk_tst
 *
 * FUNCTION:  Get 128-port VPD from driver.
 *
 * EXECUTION ENVIRONMENT:  User level task.
 *
 * NOTES:  This test is required to determine what kind of adapter is out
 *      there.  Nothing can be tested if this does't work.  CRC of VPD is
 *      also checked here.  The adapter type and interface type are set by
 *      this function based on the contents of the VPD.
 *
 * RETURNS:  0  test successful
 *          -1  ioctl failed
 *           2  unknown adapter
 *           3  controller/adapter VPD test failed
 *           5  concentrator VPD test failed
 */
vpd128_chk_tst (fdes, tucb_ptr, action)
int     fdes;
struct  tucb_data  *tucb_ptr;
int     action;
{
        char    buf[256+8];             /* Buffer for POS regs and VPD  */
        int     bufsize = sizeof (buf); /* Buffer size for POS regs & VPD */
        struct  cfg_dd  cfg_dd;         /* Command struct to request VPD */
        struct  stat    sbuf;           /* fstat system call buffer */
        int     crc;                    /* Calculated CRC of the VPD */
        char    *vpd_data;              /* Pointer to VPD data */
        int     length = 0;             /* Length of VPD data */
        int     expected_crc;           /* Expected CRC of the VPD */
        int     vpd_rc = PASSED;        /* vpd test return code */
        int     busfd;                  /* Bus file descriptor */
        struct mdio md;
        int i,j;
        int id;                         /* board id */
        int slot;                       /* machine adapter slots */
        int pos0;
        int bus;
        short x;
        short size;
        char p[2];
        char addr;                      /* for addressing POS info */
        int k;
        int num;
        char *c,*d,busst[16];
        char *bptr;
        int line;
        int conc0;
        u_short line0;
        u_short line1;
        u_short status0;
        u_short status1;
        u_short conc;

        rw_t rw;                        /* for KME - see digi.h */
        u_short num_reads;
        u_short vpd_size;               /* valid VPD size */
        u_short vpd_crc, cvpd_crc;      /* saved CRC from VPD header */
        u_short startaddr;              /* start of VPD data */
        int brdnum = 0;
        extern  unsigned short crc_gen(); /* Routine to generate CRC  */

        memset (buf, 0, sizeof (buf));
        if ((vpd_rc = fstat (fdes, &sbuf)) == FAILED) {
            err(0x78,vpd_rc,0);
            return (vpd_rc);
        }
        vpd_rc = PASSED;

        cfg_dd.kmid=loadext(tucb_ptr->ttycb.dev_drvr,FALSE,TRUE);
        cfg_dd.devno = sbuf.st_rdev;
        cfg_dd.cmd = CFG_QVPD;
        cfg_dd.ddsptr = buf;
        cfg_dd.ddslen = bufsize;
        if (sysconfig(SYS_CFGDD,&cfg_dd,sizeof(cfg_dd)) == CONF_FAIL) {
                  err(0x79,0,0);
                  return (vpd_rc = UNK_VPD);
        }


        slot = ((sbuf.st_rdev >> 8) & 0x0f);

        switch (action) {
        case CONFIG:    /* Determine what kind of adapter it is  */
            switch (buf[0]) {
            case P128RS232:
                tucb_ptr->ttycb.adapter = P128RS232;
                break;
            default:
                tucb_ptr->ttycb.adapter = UNKNOWN;
                vpd_rc = UNK_VPD;
                break;
            }
            break;
        case TEST:
            if(tucb_ptr->header.mfg == FALSE) {
                switch(tucb_ptr->header.tu) {
                case 10:
                case 110:
                    bus = (sbuf.st_rdev >> 12) & 0x01;
                    switch(bus){
                    case 0:
                        strcpy(busst,"/dev/bus0");
                        break;
                    case 1:
                        strcpy(busst,"/dev/bus1");
                        break;
                    }  /* end switch */

                    /* do host */
                    if ((busfd = open(busst,O_RDWR)) == -1) {
                        err(0x80,busfd,errno);
                        return (ADP_VPD);
                    }
                    brdnum=0;
                    for (i = 0; i < 2; i++) {
                        md.md_addr = POSREG(i, slot);
                        md.md_size = 1;
                        md.md_incr = MV_BYTE;
                        md.md_data = &p[i];
                        if (ioctl(busfd, MIOCCGET, &md) == -1) {
                            err(0x81,0,errno);
                            return (ADP_VPD);
                        }
                    }

                    id = (p[1] << 8) + p[0];

                    /* 128port adapter pos id */
                    if (id == 0xffe1) {
                        pos0 = POSREG(0, slot);

                        /* get header */
                        /* CAVEAT: the adapter has an Intel mind-set */
                        for (j = 1; j < 8; j++) {
                            addr = j;
                            md.md_data = &addr;
                            md.md_addr = POSREG(pos0+6, slot);
                            md.md_size = 1;
                            md.md_incr = MV_BYTE;
                            if (ioctl(busfd, MIOCCPUT, &md) < 0) {
                                err(0x82,0,errno);
                                return (ADP_VPD);
                            }
                            addr = 0;
                            md.md_data = &addr;
                            md.md_addr = POSREG(pos0+7, slot);
                            md.md_size = 1;
                            md.md_incr = MV_BYTE;
                            if (ioctl(busfd, MIOCCPUT, &md) < 0) {
                                err(0x83,0,errno);
                                return (ADP_VPD);
                            }
                            md.md_addr = POSREG(pos0+3, slot);
                            md.md_size = 1;
                            md.md_incr = MV_BYTE;
                            md.md_data = (char *) &addr;
                            if (ioctl(busfd, MIOCCGET, &md) < 0) {
                                err(0x84,0,errno);
                                return (ADP_VPD);
                            }
                            buf[j] = addr;
                        } /* End of Header loop */

                        /* get data */
                        vpd_size = (u_short) ((buf[4] << 8) | buf[5]);
                        vpd_crc = (u_short) ((buf[6] << 8) | buf[7]);
                        num_reads = vpd_size * 2;
                        vpd_size *= 2;

                        if (vpd_size+8 > bufsize) {
                            err(0x85,0,0);
                            return(ADP_VPD);
                        }

                        for (j = 0; j < num_reads; j++) {
                            addr = (j+8) & 0xff;
                            md.md_data = &addr;
                            md.md_addr = POSREG(pos0+6, slot);
                            md.md_size = 1;
                            md.md_incr = MV_BYTE;
                            if (ioctl(busfd, MIOCCPUT, &md) < 0) {
                                err(0x86,0,errno);
                                return (ADP_VPD);
                            }
                            addr = (j >> 8) & 0xff;
                            md.md_data = &addr;
                            md.md_addr = POSREG(pos0+7, slot);
                            md.md_size = 1;
                            md.md_incr = MV_BYTE;
                            if (ioctl(busfd, MIOCCPUT, &md) < 0) {
                                err(0x87,0,errno);
                                return (ADP_VPD);
                            }
                            md.md_addr = POSREG(pos0+3, slot);
                            md.md_size = 1;
                            md.md_incr = MV_BYTE;
                            md.md_data = (char *) &addr;
                            if (ioctl(busfd, MIOCCGET, &md) < 0) {
                                err(0x88,0,errno);
                                return (ADP_VPD);
                            }
                            buf[j+8] = addr;
                        } /* End of Data loop */

                        cvpd_crc = crc_gen(&buf[8], num_reads);

                        if (vpd_crc != cvpd_crc) {
                            err(0x89,vpd_crc,cvpd_crc);
                            return(ADP_VPD);
                        }

                    } /* end if (id = 0xffe1) */
                    break;
                case 30:
                    conc = (sbuf.st_rdev >> 4) & 0x07;
                    conc++;

                    /*
                     * get the pointers to the fep's per line private data
                     * structures line1 at 0x0D28, line2 at 0x0D2A.
                    */
                    rw.rw_board = bus<<4|(sbuf.st_rdev >> 8) & 0x0f;
                    rw.rw_conc = 0;
                    rw.rw_req = RW_READ;
                    rw.rw_size = 128;
                    rw.rw_addr = 0x0d28;
                    if (ioctl(fdes, CXMA_KME, &rw) != 0) {
                        err(0x90,0,errno);
                        return(ADP_ERR);
                    }
                    line0 = (rw.rw_data[1] << 8) | rw.rw_data[0];
                    line1 = (rw.rw_data[3] << 8) | rw.rw_data[2];

                    /*
                     * the data starting at 0x0CD0 on the board contains the
                     * config string which informed the fep what locations
                     * on each line would have concentrators and the sync
                     * line baud rate.
                    */
                    rw.rw_addr = 0x0cd0;
                    if (ioctl(fdes, CXMA_KME, &rw) != 0) {
                        err(0x91,0,errno);
                        return(ADP_ERR);
                    }

                    conc0 = line = i = 0;
                    for (c = &rw.rw_data[1]; *c != 0xff; c++) {
                       if (i == conc)
                           break;
                       if (*c == 0x0) {
                           conc0 = i;
                           line = 1;
                       }
                       if (*c == 0x10)
                           i++;
                    }
                    if (i != conc) {
                        err(0x92,i,conc);
                        return(ADP_ERR);
                    }

                    /*
                     * line0 and line1 are the addresses of the status
                     * for each sync line.
                    */
                    rw.rw_addr = line0;
                    if (ioctl(fdes, CXMA_KME, &rw) != 0) {
                        err(0x93,0,errno);
                        return(ADP_ERR);
                    }
                    status0 = (rw.rw_data[1] << 8) | rw.rw_data[0];

                    rw.rw_addr = line1;
                    if (ioctl(fdes, CXMA_KME, &rw) != 0) {
                        err(0x94,0,errno);
                        return(ADP_ERR);
                    }
                    status1 = (rw.rw_data[1] << 8) | rw.rw_data[0];

                    if (line == 1)
                        status0 = status1;
                    /*
                     * If the lsb of this byte is set, it indicates an
                     * open line
                    */
                    if (status0 & 0x01) {
                        err(0x95,0,0);
                        return(LINE_ERR);
                    }

                    /*
                     * this location contains the per concentrator
                     * information kept by the host fep to indicate if the
                     * concentrator is responding normally.  Possible
                     * values are 1, 3, & 7.
                    */
                    rw.rw_addr = 0x0e10;
                    if (ioctl(fdes, CXMA_KME, &rw) != 0) {
                        err(0x96,0,errno);
                        return(ADP_ERR);
                    }
                    if (rw.rw_data[(line*8)+conc-conc0-1] != 0x07) {
                        err(0x97,0,0);
                        return(CON_ERR);
                    }

                    /* do concentrator */
                    rw.rw_board = (bus<<4)|((sbuf.st_rdev >> 8) & 0x0f);
                    rw.rw_conc = (sbuf.st_rdev >> 4) & 0x07;
                    rw.rw_conc += 1;
                    rw.rw_req = RW_READ;
                    rw.rw_addr = 0xfc000;
                    rw.rw_size = 128;
                    for (k=0; k<128; k++) {
                        rw.rw_data[k] = '\0';
                    }
                    if (ioctl(fdes, CXMA_KME, &rw) != 0) {
                        err(0x98,0,errno);
                        return(CON_VPD);
                    }
                    c = rw.rw_data;

                    /*
                     * VPD is in eprom, only the low rom contains the
                     * information.
                    */
                    vpd_size = (*(c + 8) << 8) | *(c + 10);
                    num_reads = vpd_size * 2;
                    vpd_size *= 2;
                    vpd_crc = (*(c + 12) << 8) | *(c + 14);

                    if (vpd_size+8 > bufsize) {
                        err(0x99,0,0);
                        return(CON_VPD);
                    }

                    if (num_reads == 0) {
                        err(0x100,0,0);
                        return(CON_VPD);
                    }
                    startaddr = 16;
                    bptr = &buf[8];
                    while (num_reads) {
                        for (k=0; k<128; k++) {
                            rw.rw_data[k] = '\0';
                        }
                        rw.rw_req = RW_READ;
                        rw.rw_addr = 0xfc000 + startaddr;
                        rw.rw_size = (num_reads < 64) ? num_reads*2 : 128;
                        startaddr += ((num_reads < 64) ? num_reads*2 : 128);
                        if (ioctl(fdes, CXMA_KME, &rw) != 0) {
                            err(0x101,0,errno);
                            return(CON_VPD);
                        }
                        c = rw.rw_data;
                        k = 0;
                        num = (num_reads < 64) ? num_reads*2 : 128;
                        for (k = 0; k < num;) {
                            *bptr = *c;
                            k += 2;
                            c += 2;
                            bptr++;
                        }
                        num_reads = (num_reads < 64) ? 0 : num_reads - 64;
                    }  /* end while */
                    cvpd_crc = crc_gen(&buf[8], vpd_size);
                    if (vpd_crc != cvpd_crc) {
                        err(0x102,vpd_crc,cvpd_crc);
                        return(ADP_VPD);
                    }
                    break;
                }  /* end switch */

                return (vpd_rc);
        } /* end if */
        break;
    } /* end switch */
}  /* vpd128_chk_tst end */

/*
 * NAME:  sync128_wrap_tst
 *
 * FUNCTION:  128 port sync interface check.
 *
 * EXECUTION ENVIRONMENT:  User level task.
 *
 * NOTES:  This function provides an interface to the 128 port driver's
 *      synchronous interface test.
 *
 * RETURNS:  0  test successful
 *          -1  test failed
 */
sync128_wrap_tst (fdes)
int     fdes;
{
        int i;
        int swt_rc = PASSED;
        int line;
        int conc0;
        int bus;
        char *c;
        u_short line0;
        u_short line1;
        u_short status0;
        u_short status1;
        u_short conc;
        rw_t rw;
        struct  stat sbuf;

        if ((swt_rc = fstat (fdes, &sbuf)) == FAILED) {
            err(0x103,swt_rc,0);
            return (-1);
        }

        /*
         * get the pointers to the fep's per line private data structures
         * line1 at 0x0D28, line2 at 0x0D2A.
         */
        conc = (sbuf.st_rdev >> 4) & 0x07;
        conc++;
        rw.rw_board = (bus<<4)|((sbuf.st_rdev >> 8) & 0x0f);
        rw.rw_conc = 0;
        rw.rw_req = RW_READ;
        rw.rw_size = 128;
        rw.rw_addr = 0x0d28;
        if (ioctl(fdes, CXMA_KME, &rw) != 0) {
            err(0x104,0,errno);
            return(-1);
        }
        line0 = (rw.rw_data[1] << 8) | rw.rw_data[0];
        line1 = (rw.rw_data[3] << 8) | rw.rw_data[2];

        /*
         * the data starting at 0x0CD0 on the board contains the config
         * string which informed the fep what locations on each line
         * would have concentrators and the sync line baud rate.
         */
        rw.rw_addr = 0x0cd0;
        if (ioctl(fdes, CXMA_KME, &rw) != 0) {
            err(0x105,0,errno);
            return(-1);
        }

        for (conc0 = line = i = 0, c = &rw.rw_data[1]; *c != 0xff; c++) {
            if (i == conc)
                break;
            if (*c == 0x0) {
                conc0 = i;
                line = 1;
            }
            if (*c == 0x10) i++;
        }
        if (i != conc) {
            err(0x106,i,conc);
            return(-1);
        }

        /*
         * line0 and line1 are the addresses of the status for each
         * sync line.
         */
        rw.rw_addr = line0;
        if (ioctl(fdes, CXMA_KME, &rw) != 0) {
            err(0x107,0,errno);
            return(-1);
        }
        status0 = (rw.rw_data[1] << 8) | rw.rw_data[0];

        rw.rw_addr = line1;
        if (ioctl(fdes, CXMA_KME, &rw) != 0) {
            err(0x108,0,errno);
            return(-1);
        }
        status1 = (rw.rw_data[1] << 8) | rw.rw_data[0];

        /* no reason to check both lines in this test */
        /* if (line == 1) status0 = status1; */
        /* If the lsb of this byte is set, it indicates an open line */
        if (status0 & 0x01) {
            line = 0;
            err(0x109,0,0);
            return(SYN_TST);
        }
        if (status1 & 0x01) {
            line = 1;
            err(0x110,0,0);
            return(SYN_TST);
        }

        return (swt_rc);
}  /* sync128_wrap_tst end */

/*
 * NAME:  SetPortDefaults
 *
 * FUNCTION:  128 port attribute setting
 *
 * EXECUTION ENVIRONMENT:  User level task.
 *
 * NOTES:  This function provides an interface to the 128 port driver's
 *      synchronous interface test.
 *
 * RETURNS:  0  test successful
 *           1  test failed
 */
SetPortDefaults(int async_des)
{
   int rc;
   struct termios terms;
   errno = 0;
   rc = tcgetattr(async_des, &terms);
   if (rc == -1) {
      err(0x111,0,0);
      return(1);
   }

   terms.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL | ICANON | IEXTEN | NOFLSH |
          TOSTOP | ISIG | XCASE | FLUSHO | PENDIN | ECHOPRT | ECHOKE | ECHOCTL);
   /*
    * Set input/output processing flags
   */
   terms.c_iflag |=  (ICRNL | INPCK | PARMRK | IXOFF | IXON | BRKINT);
   terms.c_iflag &= ~(IGNBRK | IGNCR | IGNPAR | INLCR | ISTRIP | IUCLC | IXANY |
           IMAXBEL);
   terms.c_oflag &= ~(OPOST);
   terms.c_cflag |=  (HUPCL | CREAD);
   terms.c_cflag &= ~(PAREXT);

   rc = tcsetattr(async_des, TCSANOW, &terms);
   if(rc == -1) {
       err(0x112,0,0);
       return(1);
   }

   return(0);
}

/*
 * NAME:  err
 *
 * FUNCTION:  performs special-case error logging
 *
 * EXECUTION ENVIRONMENT:  User level task.
 *
 * NOTES:  This function provides a high-granularity debug to assist in
 *      detecting and correcting errors in the async code.
 *
 * RETURNS:  0
 *
 */
err(int id, int rcode, int errnum)
{
   int   fd;
   long  longerr;
   struct stat stat_buffer;

   if(AsyncDebug == 0) {
       if(stat("/tmp/DASYNC_DEBUG", &stat_buffer) == 0)
           AsyncDebug=1;
       else {
           if(getenv("DASYNC_DEBUG") != (char *) NULL)
               AsyncDebug=1;
           else
               AsyncDebug=-1;
       }
   }

   if(AsyncDebug == 1) {
       sleep(3);
       longerr=id;
       fd=open("/dev/nvram", O_RDWR);
       ioctl(fd,MIONVLED,longerr);
       sleep(1);
       if (rcode != 0) {
          longerr=0xfa1;  /* LED = ' c1' */
          sleep(2);
          ioctl(fd,MIONVLED,longerr);
          longerr=rcode;
          sleep(2);
          ioctl(fd,MIONVLED,longerr);
          sleep(1);
       } /* endif */
       if (errnum != 0) {
          longerr=0xfa2;  /* LED = ' c2' */
          sleep(2);
          ioctl(fd,MIONVLED,longerr);
          longerr=errnum;
          sleep(2);
          ioctl(fd,MIONVLED,longerr);
          sleep(1);
       } /* endif */
       close(fd);
   }

   return(0);

}
