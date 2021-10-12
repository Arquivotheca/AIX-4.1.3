#ifndef lint
static char sccsid[] = "@(#)87 1.9 src/bos/kernext/pty/spty_db.c, sysxpty, bos41J, 9515B_all 4/5/95 04:14:35";
#endif

/*
 * COMPONENT_NAME: SYSXPTY - sptydd streams module
 *
 * FUNCTIONS: pty_print
 *
 *
 * ORIGINS: 83
 *
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */


/************************************************************************/
/*                                                                      */
/*      INCLUDE FILE                                                    */
/*                                                                      */
/************************************************************************/

#include<sys/types.h>
#include<sys/ioctl.h>
#include<sys/param.h>
#include<sys/user.h>            /* uio.h needs this and
                                   remote mode looks at u space */
#include<sys/uio.h>             /* uio struct */
#include<sys/errno.h>           /* ENOMEM */
#include<sys/lock_def.h>        /* lock */
#include<sys/lock_alloc.h>
#include<sys/lockname.h>
#include <sys/lockl.h>          /* LOCK_AVAIL */
#include <sys/i_machine.h>
#include<sys/malloc.h>          /* maloc */
#include<sys/sleep.h>           /* EVENT_NULL */
#include<sys/device.h>          /* CFG_INIT CFG_TERM */
#include<sys/strconf.h>
#include<sys/stream.h>
#include<sys/stropts.h>
#include<sys/strstat.h>
#include<sys/strlog.h>
#include<sys/termio.h>
#include<sys/poll.h>            /* POLLIN,etc */
#include<sys/ttydefaults.h>
#include<sys/trchkid.h>         /* trace */
#include<sys/str_tty.h>         /* streams TTY */
#include<sys/eucioctl.h>


#include "./spty.h"

#if !defined(_KERNEL) && defined(_KDBUSR)

#include "kdb.h"

#define DB_READ_MEM     1
#define DB_READ_WORD    2
#define DB_READ_BYTE    3

#define db_printf                       brkpoint
#define db_read_mem(vad, lad, siz)      brkpoint(DB_READ_MEM, vad, lad, siz)
#define db_read_word(vad)               brkpoint(DB_READ_WORD, vad)
#define db_read_byte(vad)               brkpoint(DB_READ_BYTE, vad)

#define kgetstruct	                db_read_mem
#define knget		                db_read_word

#define spty_db_printf                  db_printf
#define memcpy(lad, vad, siz)           db_read_mem(vad, lad, siz)

#else /* !_KERNEL && _KDBUSR */

#if defined(_KERNEL) && defined(IN_TTYDBG)
#define spty_db_printf   tty_db_printf
#else
#define spty_db_printf   printf
#endif  /* _KERNEL && IN_TTYDBG */

#endif /* !_KERNEL && _KDBUSR */

/************************************************************************/
/*                                                                      */
/* LIST OF FUNCTIONS                                                    */
/*                                                                      */
/************************************************************************/


/* -------------------------------------------------------------------- */
int	pty_print();
/* -------------------------------------------------------------------- */


/*######################### TRACE FUNCTION ############################*/
/*                                                                     */
/*        FUNCTION : pty_print()                                       */
/*                                                                     */
/*        PARAMETER :  ptp             : pty_s structure               */
/*		       arg	       : void for PTY		       */
/*                                                                     */
/*        RETURN : 0  on sucess,                                       */
/*                 -1 or return code from tty_db_nomore() otherwise    */
/*                                                                     */
/*        PURPOSE :                                                    */
/*                                                                     */
/*        this procedure print pty_s structure                         */
/*                                                                     */
/***********************************************************************/
int
pty_print(
              struct pty_s *ptp,
              int   arg
             )
{
   struct  pty_s buffer;

#ifdef TTYDBG
   if (tty_read_mem(ptp, &buffer, sizeof(struct pty_s)) != 0)
	return(-1);
#else /* TTYDBG */
   memcpy(&buffer,ptp,sizeof(struct pty_s));
#endif /* TTYDBG */

   if(!buffer.pt_mwq) tty_db_nomore(spty_db_printf("pt_mwq...............: NULL(Master Write Queue)\n"));
   else tty_db_nomore(spty_db_printf("pt_mwq...............: 0x%x(Master Write Queue)\n",buffer.pt_mwq));

   if(!buffer.pt_mrq) tty_db_nomore(spty_db_printf("pt_mrq...............: NULL(Master Read Queue)\n"));
   else tty_db_nomore(spty_db_printf("pt_mrq...............: 0x%x(Master Read Queue)\n",buffer.pt_mrq));

   if(!buffer.pt_swq) tty_db_nomore(spty_db_printf("pt_swq...............: NULL(Slave Write Queue)\n"));
   else tty_db_nomore(spty_db_printf("pt_swq...............: 0x%x(Slave Write Queue)\n",buffer.pt_swq));

   if(!buffer.pt_srq) tty_db_nomore(spty_db_printf("pt_srq...............: NULL(Slave Read Queue)\n"));
   else tty_db_nomore(spty_db_printf("pt_srq...............: 0x%x(Slave Read Queue)\n",buffer.pt_srq));
        
   tty_db_nomore(spty_db_printf("pt_flags.............:"));
   if(buffer.pt_flags & PF_MOPEN)tty_db_nomore(spty_db_printf(" PF_MOPEN"));
   if(buffer.pt_flags & PF_SOPEN)tty_db_nomore(spty_db_printf(" PF_SOPEN"));
   if(buffer.pt_flags & PF_XCLUDE)tty_db_nomore(spty_db_printf(" PF_XCLUDE"));
   if(buffer.pt_flags & PF_TTSTOP)tty_db_nomore(spty_db_printf(" PF_TTSTOP"));
   if(buffer.pt_flags & PF_TTINSTOP)tty_db_nomore(spty_db_printf(" PF_TTINSTOP"));
   if(buffer.pt_flags & PF_MFLOW)tty_db_nomore(spty_db_printf(" PF_MFLOW"));
   if(buffer.pt_flags & PF_SFLOW)tty_db_nomore(spty_db_printf(" PF_SFLOW"));
   if(buffer.pt_flags & PF_SWOPEN)tty_db_nomore(spty_db_printf(" PF_SWOPEN"));
   if(buffer.pt_flags & PF_REMOTE)tty_db_nomore(spty_db_printf(" PF_REMOTE"));
   if(buffer.pt_flags & PF_SWDRAIN)tty_db_nomore(spty_db_printf(" PF_SWDRAIN"));
   if(buffer.pt_flags & PF_MWDRAIN)tty_db_nomore(spty_db_printf(" PF_MWDRAIN"));
   if(buffer.pt_flags & PF_PKT)tty_db_nomore(spty_db_printf(" PF_PKT"));
   if(buffer.pt_flags & PF_NOSTOP)tty_db_nomore(spty_db_printf(" PF_NOSTOP"));
   if(buffer.pt_flags & PF_UCNTL)tty_db_nomore(spty_db_printf(" PF_UCNTL"));
   if(buffer.pt_flags & PF_SAK)tty_db_nomore(spty_db_printf(" PF_SAK"));
   if(buffer.pt_flags & PF_CREAD)tty_db_nomore(spty_db_printf(" PF_CREAD"));
   if(buffer.pt_flags & PF_IXON)tty_db_nomore(spty_db_printf(" PF_IXON"));
   if(buffer.pt_flags & PF_IXOFF)tty_db_nomore(spty_db_printf(" PF_IXOFF"));
   if(buffer.pt_flags & PF_IXANY)tty_db_nomore(spty_db_printf(" PF_IXANY"));
   if(buffer.pt_flags & PF_PTM_XFER)tty_db_nomore(spty_db_printf(" PF_PTM_XFER"));
   if(buffer.pt_flags & PF_PTS_XFER)tty_db_nomore(spty_db_printf(" PF_PTS_XFER"));
   if(buffer.pt_flags & PF_MWXFER)tty_db_nomore(spty_db_printf(" PF_MWXFER"));
   if(buffer.pt_flags & PF_SWXFER)tty_db_nomore(spty_db_printf(" PF_SWXFER"));
   if(buffer.pt_flags & PF_M_RSE)tty_db_nomore(spty_db_printf(" PF_M_RSE"));
   if(buffer.pt_flags & PF_RSE_OK)tty_db_nomore(spty_db_printf(" PF_RSE_OK"));
   tty_db_nomore(spty_db_printf("\n"));

   /* termios structure */

   tty_db_nomore(spty_db_printf("pt_tio.c_iflag.......:"));
   if(buffer.pt_tio.c_iflag & IGNBRK)tty_db_nomore(spty_db_printf(" IGNBRK"));
   if(buffer.pt_tio.c_iflag & BRKINT)tty_db_nomore(spty_db_printf(" BRKINT"));
   if(buffer.pt_tio.c_iflag & IGNPAR)tty_db_nomore(spty_db_printf(" IGNPAR"));
   if(buffer.pt_tio.c_iflag & PARMRK)tty_db_nomore(spty_db_printf(" PARMRK"));
   if(buffer.pt_tio.c_iflag & INPCK)tty_db_nomore(spty_db_printf(" INPCK"));
   if(buffer.pt_tio.c_iflag & ISTRIP)tty_db_nomore(spty_db_printf(" ISTRIP"));
   if(buffer.pt_tio.c_iflag & INLCR)tty_db_nomore(spty_db_printf(" INLCR"));
   if(buffer.pt_tio.c_iflag & IGNCR)tty_db_nomore(spty_db_printf(" IGNCR"));
   if(buffer.pt_tio.c_iflag & ICRNL)tty_db_nomore(spty_db_printf(" ICRNL"));
   if(buffer.pt_tio.c_iflag & IXON)tty_db_nomore(spty_db_printf(" IXON"));
   if(buffer.pt_tio.c_iflag & IXOFF)tty_db_nomore(spty_db_printf(" IXOFF"));
   if(buffer.pt_tio.c_iflag & IUCLC)tty_db_nomore(spty_db_printf(" IUCLC"));
   if(buffer.pt_tio.c_iflag & IXANY)tty_db_nomore(spty_db_printf(" IXANY"));
   if(buffer.pt_tio.c_iflag & IMAXBEL)tty_db_nomore(spty_db_printf(" IMAXBEL"));
   tty_db_nomore(spty_db_printf("\n"));

   tty_db_nomore(spty_db_printf("pt_tio.c_oflag.......:"));
   if(buffer.pt_tio.c_oflag & OLCUC)tty_db_nomore(spty_db_printf(" OLCUC"));
   if(buffer.pt_tio.c_oflag & ONLCR)tty_db_nomore(spty_db_printf(" ONLCR"));
   if(buffer.pt_tio.c_oflag & OCRNL)tty_db_nomore(spty_db_printf(" OCRNL"));
   if(buffer.pt_tio.c_oflag & ONOCR)tty_db_nomore(spty_db_printf(" ONOCR"));
   if(buffer.pt_tio.c_oflag & ONLRET)tty_db_nomore(spty_db_printf(" ONLRET"));
   if(buffer.pt_tio.c_oflag & OFILL)tty_db_nomore(spty_db_printf(" OFILL"));
   if(buffer.pt_tio.c_oflag & OFDEL)tty_db_nomore(spty_db_printf(" OFDEL"));
   if(buffer.pt_tio.c_oflag & CRDLY)tty_db_nomore(spty_db_printf(" CRDLY"));
   if(buffer.pt_tio.c_oflag & CR0)tty_db_nomore(spty_db_printf(" CR0"));
   if(buffer.pt_tio.c_oflag & CR1)tty_db_nomore(spty_db_printf(" CR1"));
   if(buffer.pt_tio.c_oflag & CR2)tty_db_nomore(spty_db_printf(" CR2"));
   if(buffer.pt_tio.c_oflag & CR3)tty_db_nomore(spty_db_printf(" CR3"));
   if(buffer.pt_tio.c_oflag & TABDLY)tty_db_nomore(spty_db_printf(" TABDLY"));
   if(buffer.pt_tio.c_oflag & TAB0)tty_db_nomore(spty_db_printf(" TAB0"));
   if(buffer.pt_tio.c_oflag & TAB1)tty_db_nomore(spty_db_printf(" TAB1"));
   if(buffer.pt_tio.c_oflag & TAB2)tty_db_nomore(spty_db_printf(" TAB2"));
   if(buffer.pt_tio.c_oflag & TAB3)tty_db_nomore(spty_db_printf(" TAB3"));
   if(buffer.pt_tio.c_oflag & BSDLY)tty_db_nomore(spty_db_printf(" BSDLY"));
   if(buffer.pt_tio.c_oflag & BS0)tty_db_nomore(spty_db_printf(" BS0"));
   if(buffer.pt_tio.c_oflag & BS1)tty_db_nomore(spty_db_printf(" BS1"));
   if(buffer.pt_tio.c_oflag & FFDLY)tty_db_nomore(spty_db_printf(" FFDLY"));
   if(buffer.pt_tio.c_oflag & FF0)tty_db_nomore(spty_db_printf(" FF0"));
   if(buffer.pt_tio.c_oflag & FF1)tty_db_nomore(spty_db_printf(" FF1"));
   if(buffer.pt_tio.c_oflag & NLDLY)tty_db_nomore(spty_db_printf(" NLDLY"));
   if(buffer.pt_tio.c_oflag & NL0)tty_db_nomore(spty_db_printf(" NL0"));
   if(buffer.pt_tio.c_oflag & NL1)tty_db_nomore(spty_db_printf(" NL1"));
   if(buffer.pt_tio.c_oflag & VTDLY)tty_db_nomore(spty_db_printf(" VTDLY"));
   if(buffer.pt_tio.c_oflag & VT0)tty_db_nomore(spty_db_printf(" VT0"));
   if(buffer.pt_tio.c_oflag & VT1)tty_db_nomore(spty_db_printf(" VT1"));
   tty_db_nomore(spty_db_printf("\n"));

   tty_db_nomore(spty_db_printf("pt_tio.c_cflag.......:"));
   if(buffer.pt_tio.c_cflag & CBAUD)tty_db_nomore(spty_db_printf(" CBAUD"));
   if(buffer.pt_tio.c_cflag & CSIZE)tty_db_nomore(spty_db_printf(" CSIZE"));
   if(buffer.pt_tio.c_cflag & CS5)tty_db_nomore(spty_db_printf(" CS5"));
   if(buffer.pt_tio.c_cflag & CS6)tty_db_nomore(spty_db_printf(" CS6"));
   if(buffer.pt_tio.c_cflag & CS7)tty_db_nomore(spty_db_printf(" CS7"));
   if(buffer.pt_tio.c_cflag & CS8)tty_db_nomore(spty_db_printf(" CS8"));
   if(buffer.pt_tio.c_cflag & CSTOPB)tty_db_nomore(spty_db_printf(" CSTOPB"));
   if(buffer.pt_tio.c_cflag & CREAD)tty_db_nomore(spty_db_printf(" CREAD"));
   if(buffer.pt_tio.c_cflag & PARENB)tty_db_nomore(spty_db_printf(" PARENB"));
   if(buffer.pt_tio.c_cflag & PARODD)tty_db_nomore(spty_db_printf(" PARODD"));
   if(buffer.pt_tio.c_cflag & HUPCL)tty_db_nomore(spty_db_printf(" HUPCL"));
   if(buffer.pt_tio.c_cflag & CLOCAL)tty_db_nomore(spty_db_printf(" CLOCAL"));
   if(buffer.pt_tio.c_cflag & CIBAUD)tty_db_nomore(spty_db_printf(" CIBAUD"));
   if(buffer.pt_tio.c_cflag & IBSHIFT)tty_db_nomore(spty_db_printf(" IBSHIFT"));
   if(buffer.pt_tio.c_cflag & PAREXT)tty_db_nomore(spty_db_printf(" PAREXT"));
   tty_db_nomore(spty_db_printf("\n"));

   tty_db_nomore(spty_db_printf("pt_tio.c_lflag.......:"));
   if(buffer.pt_tio.c_lflag & ISIG)tty_db_nomore(spty_db_printf(" ISIG"));
   if(buffer.pt_tio.c_lflag & ICANON)tty_db_nomore(spty_db_printf(" ICANON"));
   if(buffer.pt_tio.c_lflag & XCASE)tty_db_nomore(spty_db_printf(" XCASE"));
   if(buffer.pt_tio.c_lflag & ECHO)tty_db_nomore(spty_db_printf(" ECHO"));
   if(buffer.pt_tio.c_lflag & ECHOE)tty_db_nomore(spty_db_printf(" ECHOE"));
   if(buffer.pt_tio.c_lflag & ECHOK)tty_db_nomore(spty_db_printf(" ECHOK"));
   if(buffer.pt_tio.c_lflag & ECHONL)tty_db_nomore(spty_db_printf(" ECHONL"));
   if(buffer.pt_tio.c_lflag & NOFLSH)tty_db_nomore(spty_db_printf(" NOFLSH"));
   if(buffer.pt_tio.c_lflag & TOSTOP)tty_db_nomore(spty_db_printf(" TOSTOP"));
   if(buffer.pt_tio.c_lflag & ECHOCTL)tty_db_nomore(spty_db_printf(" ECHOCTL"));
   if(buffer.pt_tio.c_lflag & ECHOPRT)tty_db_nomore(spty_db_printf(" ECHOPRT"));
   if(buffer.pt_tio.c_lflag & ECHOKE)tty_db_nomore(spty_db_printf(" ECHOKE"));
   if(buffer.pt_tio.c_lflag & FLUSHO)tty_db_nomore(spty_db_printf(" FLUSHO"));
   if(buffer.pt_tio.c_lflag & PENDIN)tty_db_nomore(spty_db_printf(" PENDIN"));
   if(buffer.pt_tio.c_lflag & IEXTEN)tty_db_nomore(spty_db_printf(" IEXTEN"));
   tty_db_nomore(spty_db_printf("\n"));

   if(VINTR<NCCS) 
      tty_db_nomore(spty_db_printf("pt_tio.c_cc[VINTR]...: 0x%x\n",buffer.pt_tio.c_cc[VINTR]));
   if(VQUIT<NCCS) 
      tty_db_nomore(spty_db_printf("pt_tio.c_cc[VQUIT]...: 0x%x\n",buffer.pt_tio.c_cc[VQUIT]));
   if(VERASE<NCCS) 
      tty_db_nomore(spty_db_printf("pt_tio.c_cc[VERASE]..: 0x%x\n",buffer.pt_tio.c_cc[VERASE]));
   if(VKILL<NCCS) 
      tty_db_nomore(spty_db_printf("pt_tio.c_cc[VKILL]...: 0x%x\n",buffer.pt_tio.c_cc[VKILL]));
   if(VEOF<NCCS) 
      tty_db_nomore(spty_db_printf("pt_tio.c_cc[VEOF]....: 0x%x\n",buffer.pt_tio.c_cc[VEOF]));
   if(VEOL<NCCS) 
      tty_db_nomore(spty_db_printf("pt_tio.c_cc[VEOL]....: 0x%x\n",buffer.pt_tio.c_cc[VEOL]));
   if(VEOL2<NCCS) 
      tty_db_nomore(spty_db_printf("pt_tio.c_cc[VEOL2]...: 0x%x\n",buffer.pt_tio.c_cc[VEOL2]));
   if(VSTART<NCCS) 
      tty_db_nomore(spty_db_printf("pt_tio.c_cc[VSTART]..: 0x%x\n",buffer.pt_tio.c_cc[VSTART]));
   if(VSTOP<NCCS) 
      tty_db_nomore(spty_db_printf("pt_tio.c_cc[VSTOP]...: 0x%x\n",buffer.pt_tio.c_cc[VSTOP]));
   if(VSUSP<NCCS) 
      tty_db_nomore(spty_db_printf("pt_tio.c_cc[VSUSP]...: 0x%x\n",buffer.pt_tio.c_cc[VSUSP]));
   if(VDSUSP<NCCS) 
      tty_db_nomore(spty_db_printf("pt_tio.c_cc[VDSUSP]..: 0x%x\n",buffer.pt_tio.c_cc[VDSUSP]));
   if(VREPRINT<NCCS) 
      tty_db_nomore(spty_db_printf("pt_tio.c_cc[VREPRINT]: 0x%x\n",buffer.pt_tio.c_cc[VREPRINT]));
   if(VDISCRD<NCCS) 
      tty_db_nomore(spty_db_printf("pt_tio.c_cc[VDISCRD].: 0x%x\n",buffer.pt_tio.c_cc[VDISCRD]));
   if(VWERSE<NCCS) 
      tty_db_nomore(spty_db_printf("pt_tio.c_cc[VWERSE]..: 0x%x\n",buffer.pt_tio.c_cc[VWERSE]));
   if(VLNEXT<NCCS) 
      tty_db_nomore(spty_db_printf("pt_tio.c_cc[VLNEXT]..: 0x%x\n",buffer.pt_tio.c_cc[VLNEXT]));

   /* winsize structure */

   tty_db_nomore(spty_db_printf("pt_ws.ws_row.........: %d(window size row)\n",buffer.pt_ws.ws_row));
   tty_db_nomore(spty_db_printf("pt_ws.ws_col.........: %d(window size column)\n",buffer.pt_ws.ws_col));
   tty_db_nomore(spty_db_printf("pt_ws.ws_xpixel......: %d(window size x pixels)\n",buffer.pt_ws.ws_xpixel));
   tty_db_nomore(spty_db_printf("pt_ws.ws_ypixel......: %d(window size y pixels)\n",buffer.pt_ws.ws_ypixel));

   if(!buffer.pt_send) tty_db_nomore(spty_db_printf("pt_send..............: NULL"));
   else 
   tty_db_nomore(spty_db_printf("pt_send...............:"));
   if(buffer.pt_send & TIOCPKT_FLUSHREAD)tty_db_nomore(spty_db_printf(" TIOCPKT_FLUSHREAD"));
   if(buffer.pt_send & TIOCPKT_STOP)tty_db_nomore(spty_db_printf(" TIOCPKT_STOP"));
   if(buffer.pt_send & TIOCPKT_START)tty_db_nomore(spty_db_printf(" TIOCPKT_START"));
   if(buffer.pt_send & TIOCPKT_FLUSHWRITE)tty_db_nomore(spty_db_printf(" TIOCPKT_FLUSHWRITE"));
   if(buffer.pt_send & TIOCPKT_NOSTOP)tty_db_nomore(spty_db_printf(" TIOCPKT_NOSTOP"));
   if(buffer.pt_send & TIOCPKT_DOSTOP)tty_db_nomore(spty_db_printf(" TIOCPKT_DOSTOP"));
   tty_db_nomore(spty_db_printf("\n"));

   tty_db_nomore(spty_db_printf("pt_ucntl.............: 0x%x(Used in User Control Mode)\n",buffer.pt_ucntl));

   tty_db_nomore(spty_db_printf("pt_compatflags.......: 0x%x(Used in BSD compatibility mode)\n",
          buffer.pt_compatflags));
  
   tty_db_nomore(spty_db_printf("pt_minor.............: 0x%x\n",buffer.pt_minor));

   tty_db_nomore(spty_db_printf("pt_name..............: %s\n",buffer.pt_name));

   tty_db_nomore(spty_db_printf("pt_dev_slave.........: 0x%x\n",buffer.pt_dev_slave));

   if(buffer.pt_mode == ATT) {
      tty_db_nomore(spty_db_printf("pt_mode..............: ATT\n"));
   }
   else tty_db_nomore(spty_db_printf("pt_mode..............: BSD\n"));

   tty_db_nomore(spty_db_printf("pt_open_count........: 0x%x\n",buffer.pt_open_count));
   tty_db_nomore(spty_db_printf("pt_wait_master.......: 0x%x\n",buffer.pt_wait_master));
   tty_db_nomore(spty_db_printf("pt_mwait_drain.......: 0x%x\n",buffer.pt_mwait_drain));
   tty_db_nomore(spty_db_printf("pt_swait_drain.......: 0x%x\n",buffer.pt_swait_drain));
   tty_db_nomore(spty_db_printf("pt_mwait_xfer........: 0x%x\n",buffer.pt_mwait_xfer));
   tty_db_nomore(spty_db_printf("pt_swait_xfer........: 0x%x\n",buffer.pt_swait_xfer));

   return(0);

}
/*        --- End of pty_print() function ---		*/

#if !defined(_KERNEL) && defined(_KDBUSR)

/*######################### TRACE FUNCTION ############################*/
/*                                                                     */
/*        FUNCTION : pty_kdb()                                         */
/*                                                                     */
/*        PARAMETER :                                                  */
/*                                                                     */
/*        RETURN : Nothing                                             */
/*                                                                     */
/*        PURPOSE :                                                    */
/*                                                                     */
/*        this procedure print pty_o_s structure when in KDB (kernel   */
/*        debugger.)                                                   */
/*                                                                     */
/***********************************************************************/

uchar kdb_pty_sym_done                   = 0;
struct pty_o_s **kdb_pty_s_head_ATT_addr = 0;
struct pty_o_s **kdb_pty_s_head_BSD_addr = 0;

kdb_pty_sym_init()
{
	ulong_t addr;
	int sym_done = 1;

	if (kdb_pty_sym_done)
		return 1;
	if (kdb_symboladdr(kdb_pty_s_head_ATT_addr, "pty_s_head_ATT") == FALSE) {
		spty_db_printf("Unable to find <pty_s_head_ATT>\n");
		spty_db_printf("Enter the pty_s_head_ATT address (in hex): ");
		addr = (ulong_t)kdb_get_hex();
		if (addr == -1)
			return 0;
		kdb_pty_s_head_ATT_addr = (struct pty_o_s **)addr;
		sym_done = 0;
	}
	if (kdb_symboladdr(kdb_pty_s_head_BSD_addr, "pty_s_head_BSD") == FALSE) {
		spty_db_printf("Unable to find <pty_s_head_BSD>\n");
		spty_db_printf("Enter the pty_s_head_BSD address (in hex): ");
		addr = (ulong_t)kdb_get_hex();
		if (addr == -1)
			return 0;
		kdb_pty_s_head_BSD_addr = (struct pty_o_s **)addr;
		sym_done = 0;
	}
	kdb_pty_sym_done = sym_done;
	return 1;
}

int
pty_kdb() 
{
	struct pty_o_s *pty_o, pty_o_s;
	char   device_name[10];
  
	spty_db_printf("*** Welcome to KDB PTY ***\n\n");

	/* print AT&T open device */
	if (kdb_pty_sym_init() == 0)
		return;
	pty_o = (struct pty_o_s *)knget(kdb_pty_s_head_ATT_addr);

	while(pty_o) {

		kgetstruct(pty_o, &pty_o_s, sizeof(struct pty_o_s));
		pty_o = &pty_o_s;

		if (pty_o->pt_o_flag) {
			sprintf(device_name,"pts/%d\0",pty_o->pt_o_minor);
			spty_db_printf("------------------------------------");
			spty_db_printf("--------------------------\n");
			spty_db_printf("DEVICE NAME..........: /dev/%s\n", device_name);
			if (!pty_o->pt_o_flag)
				spty_db_printf("pty_o->pt_o_flag.....: NULL ");
			else 
				spty_db_printf("pty_o->pt_o_flag.....:");
			if (pty_o->pt_o_flag & PF_MOPEN)
				spty_db_printf(" PF_MOPEN");
			if (pty_o->pt_o_flag & PF_SOPEN)
				spty_db_printf(" PF_SOPEN");
			spty_db_printf("\n");

			if (!pty_o->pt_o_ptp)
				spty_db_printf("pty_o->pt_o_ptp.....: NULL POINTER\n");
			else {
				spty_db_printf("pty_o->pt_o_ptp......: 0x%x\n", pty_o->pt_o_ptp);
				pty_print(pty_o->pt_o_ptp, NULL);
			}
		}

		pty_o = pty_o->pt_o_next;
	}

	/* print BSD open device */
	pty_o = (struct pty_o_s *)knget(kdb_pty_s_head_BSD_addr);

	while(pty_o) {

		kgetstruct(pty_o, &pty_o_s, sizeof(struct pty_o_s));
		pty_o = &pty_o_s;

		if (pty_o->pt_o_flag) {
			device_name[0] = 't';
			device_name[1] = 't';
			device_name[2] = 'y';
			device_name[3] = "pqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
				[pty_o->pt_o_minor >> 4];
			device_name[4] = "0123456789abcdef"[pty_o->pt_o_minor&0x0f];
			device_name[5] = 0;

			spty_db_printf("------------------------------------");
			spty_db_printf("--------------------------\n");
			spty_db_printf("DEVICE NAME..........: /dev/%s\n",device_name);
			if (!pty_o->pt_o_flag)
				spty_db_printf("pty_o->pt_o_flag.....: NULL ");
			else
				spty_db_printf("pty_o->pt_o_flag.....:");
			if (pty_o->pt_o_flag & PF_MOPEN)
				spty_db_printf(" PF_MOPEN");
			if (pty_o->pt_o_flag & PF_SOPEN)
				spty_db_printf(" PF_SOPEN");
			spty_db_printf("\n");
			if (!pty_o->pt_o_ptp)
				spty_db_printf("pty_o->pt_o_ptp.....: NULL POINTER\n");
			else {
				spty_db_printf("pty_o->pt_o_ptp......: 0x%x\n", pty_o->pt_o_ptp);
				pty_print(pty_o->pt_o_ptp, NULL);
			}
		}

		pty_o = pty_o->pt_o_next;
	}
	spty_db_printf("\n*** KDB PTY END OF LIST ***\n");
}

#endif /* !_KERNEL && _KDBUSR */
