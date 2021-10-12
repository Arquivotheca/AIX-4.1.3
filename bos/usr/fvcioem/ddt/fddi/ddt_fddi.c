static char sccsid[] = "@(#)10 1.1 src/bos/usr/fvcioem/ddt/fddi/ddt_fddi.c, fvcioem, bos411, 9428A410j 4/26/94 13:54:50";

/*--------------------------------------------------------------------------
*
*             DDT_FDDI.C
*
*  COMPONENT_NAME:  Communications Device Driver Tool FDDI code.
*
*  FUNCTIONS:
*
*  ORIGINS: 27
*
*  IBM CONFIDENTIAL -- (IBM Confidential Restricted when
*  combined with the aggregated modules for this product)
*                   SOURCE MATERIALS
*  (C) COPYRIGHT International Business Machines Corp. 1989
*  All Rights Reserved
*
*  US Government Users Restricted Rights - Use, duplication or
*  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
*
*
*--------------------------------------------------------------------------
*/
#include <fcntl.h>
#include <stdio.h>
#include <values.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/param.h>
#include <sys/devinfo.h>
#include <sys/poll.h>
#include <sys/comio.h>
#include <sys/ciouser.h>
#include <sys/watchdog.h>
#include <sys/lock_def.h>
#include <sys/intr.h>
#include <sys/device.h>
#include <sys/xmem.h>
#include <sys/err_rec.h>

#include <sys/time.h>

#include <sys/cdli_fddiuser.h>
#include <sys/fddiuser.h>
#include <fddibits.h>
#include <fdditypes.h>

#include <sys/sysconfig.h>
#include <fddikiduser.h>
#include "ddt_com.h"
#include "ddt_fddi.h"
#include "ddt_csp.h"

#include <sys/signal.h>
/*----------------------------------------------------------------------*/
/*          Global Variables           */
/*----------------------------------------------------------------------*/

/*  Shared from there:                    */

extern         errno;
extern int     block;         /* block mode flag */
extern int     kern;       /* kernel mode flag */
extern int     pattern;    /* type of test pattern */
extern int     started;    /* number of starts */
extern PROFILE    profile;    /* test param defaults */
extern char    str[30];    /* scratch string area */
extern char    path[30];
extern char    unit;       /* port or device number */
extern DRVR_OPEN  dopen;         /* open block for kid driver */
extern int     openstate;     /* open flag */
extern int     autoresponse;     /* autoresponse flag */
extern unsigned int  recv_frames;      /* total # recv frames */
extern unsigned int  xmit_frames;      /* total # xmit frames */
extern char    kidpath[];     /* kernel interface drvr path */
extern char    *convcode();      /* conv code to string */
extern char    *convstatus();    /* conv status code to string */

/*----------------------------------------------------------------------*/
/*  Shared from here:                     */

unsigned char     Server_Addr[ HLEN ]; /* hardware address of Server */
unsigned char     Client_Addr[ HLEN ]; /* hardware address of Client */
unsigned char     BroadCast[HLEN];  /* Broadcast address */
char        Cfile[] = CFPATH; /* saved client frames */
char        Sfile[] = SFPATH; /* saved server frames */
char        drvpath[50];      /* open driver pathname */
int         using_tty = TRUE; /* is stdout a tty ? */
FILE        *recordfp;     /* place to dump input chars */

/*----------------------------------------------------------------------*/
static char *ecm[] =
{
   "OUT",
   "IN",
   "TRACE",
   "LEAVE",
   "PATH_TEST",
   "INSERT",
   "CHECK",
   "DEINSERT",
   "INVALID"
};
static char *pcm[] =
{
   "OFF",
   "BREAK",
   "TRACE",
   "CONNECT",
   "NEXT",
   "SIGNAL",
   "JOIN",
   "VERIFY",
   "ACTIVE",
   "MAINT",
   "INVALID"
};
static char *cfm[] =
{
   "ISOLATED",
   "LOCAL",
   "SECONDARY",
   "PRIMARY",
   "CONCAT",
   "THRU",
   "INVALID"
};
static char *rmt[] =
{
   "ISOLATED",
   "NON_OP",
   "RING_OP",
   "DETECT",
   "NON_OP_DUP",
   "RING_OP_DUP",
   "DIRECTED",
   "TRACE",
   "INVALID"
};
static char *cf[] =
{
   "ISOLATED",
   "LOCAL_A",
   "LOCAL_B",
   "LOCAL_AB",
   "LOCAL_S",
   "WRAP_A",
   "WRAP_B",
   "WRAP_AB",
   "WRAP_S",
   "C_WRAP_A",
   "C_WRAP_B",
   "C_WRAP_S",
   "THRU",
   "INVALID"
};
static char *mcfm[] =
{
   "ISOLATED",
   "LOCAL",
   "SECONDARY",
   "PRIMARY",
   "INVALID"
};
/*----------------------------------------------------------------------*/
/*  Local:                       */

/* fastwrite's write ext to KID for client/server */
static WRITE_EXT  fw_ext;
static XMIT_FRAME writbuf;    /* write buffer */
static RECV_FRAME readbuf;    /* read buffer */

void
sig_alarm_handler()
{
   /*
    * we caught a signal :
    *    it broke us out of a read or write so just
    * reset the signal catcher and return
    */
   signal (SIGALRM, sig_alarm_handler);
   return ;
}

/*------------------------------  M A I N  -----------------------------*/
/*                         */
/*  NAME: main                      */
/*                         */
/*  FUNCTION:                       */
/* Main program loop for the device driver tool -- displays a  */
/* menu of choices and processes character commands from the   */
/* keyboard.                     */
/*                         */
/*  EXECUTION ENVIRONMENT:                */
/* Can only be executed at the user process level.       */
/*                         */
/*  DATA STRUCTURES:                   */
/* Modifies the openstate flag.              */
/*                            */
/*  RETURNS: Nothing                      */
/*                         */
/*----------------------------------------------------------------------*/

/*
 * open attributes specific to each open
 */
struct   openattr
{
   int   fildes;
   int   kern;
   int   block;
};

#include <ddtlog.h>

int
   main(
      int                    argc        ,
      char                 * argv[]
   )
{
   int         rc;
   char        test = 0;
   char        invalid = FALSE;
   void        sig_alarm_handler();
   int         fildes;
   int         stacktop;
   struct  openattr  open_stack[16];

   initLog(argc,argv);

   /* find out if stdin is a tty */
   if (!isatty(0))
   {
      /*
       * record that we do not have a tty for our stdin
       *    and turn off buffering for output
       */
      using_tty = FALSE;
      recordfp = stdout;
      setvbuf (stdout, NULL, _IONBF, 0);
   }
   else
   {
      recordfp = fopen("/tmp/fddiddt.input", "w");
      if (!recordfp)
      {
         recordfp = stdout;
      }
   }
   /*
    * Set up signal catcher for blocking mode reads and writes
    */
   signal (SIGALRM, sig_alarm_handler);

   started = 0;
   openstate = FALSE;
   /*
    * for nested opens: each open pushes the file descriptor onto the
    * open table 'open_tab' and each close pops the next fildes
    * into effect.
    */
   fildes = -1;
   stacktop = -1;
   dsopen();            /* do any initializations */
   while (TRUE)
   {
       printf("\n\tFDDI Device Driver Tool");
       if (openstate)
      if (kern)
          printf("\t%s (kernel mode)", path);
      else
          printf("\t%s (user mode)", path);
       else
           printf("\n");
       printf("\n\tControl Panel:");
       printf("\n\tO) %c Open Device    S) %c Start Device      ",
      (openstate) ? '*' : ' ', (started) ? '*' : ' ');
       printf("L)   List Statistics");
       printf("\n\tC)   Close Device   H)   Halt Device       ");
       printf("V)   Query VP Data");
       printf("\n\tE)   Edit Defaults  I)   Get FDDI info     ");
       printf("P)   Issue Rcv Command");
       printf("\n\tW)   Write          R)   Read              ");
       printf("D)   Download");
       printf("\n\tG)   Get Status     M)   Mem/DMA Access    ");
       printf("F)   Fastwrite ioctl");
       printf("\n\tU)   Query Address  A)   Set/Clear Address ");
       printf("X)   MaxOpens");
       printf("\n\tK)   Invalid Cmd    B)   HCR command       ");

#ifdef __DEAD_CODE__
       printf("T)   Read State");
#endif /* __DEAD_CODE__ */

       printf("\n\tY)   Clear Hw Addr  Z)   Illegal Ioctls    ");
       printf("J)   Tx Storm");
#ifdef __XTRA_CODE__
       printf("\n\t3)   Promisc ON     5)   Bad Frame ON      ");
       printf("7)   Beacon ON");
       printf("\n\t4)   Promisc OFF    6)   Bad Frame OFF     ");
       printf("8)   Beacon OFF");
       printf("\n\t9)   SMT ON         b)   NSA ON            ");
       printf("d)   Read Bad Pkt");
       printf("\n\tt)   SMT OFF        m)   NSA OFF           ");
       printf("\n");
#endif /* __XTRA_CODE__ */
       printf("\n\tClient/Server Tests:");
       printf("\n\t1)   Server Side    2)   Client Side       ");
       if (invalid)
       fprintf(STDERR, "\n\t\"%c\" is not a valid choice! \n", test);
       printf("\n\n\tChoose from the menu or enter Q to quit: ");
                  /* read char, throw out NL */
       if ((test = getinput()) != '\n') getinput();
       invalid = FALSE;    /* innocent til proven guilty */
       switch (test)
       {             /* select test routine */
         case 'o':         /* Open Device test */
         case 'O':
           rc = open_test(DEVPATH);
           if (rc >= 0)
           {
            /*
             * push the attributes of this open
             */
            fildes = open_stack[++stacktop].fildes = rc;
            open_stack[stacktop].kern = kern;
            open_stack[stacktop].block = block;
            openstate = TRUE;
           }
           break;
      case 'B':
         /*
          * hcr command test
          */
         hcr (fildes);
         break;
      case 'c':         /* Close Device test */
      case 'C': if (close_test(fildes) == TRUE)
           {
            /*
             * close successful
             */
            if (--stacktop < 0)
            {
               started = 0;
               openstate = FALSE;
               fildes = -1;
            }
            else
            {
               /*
                * pop open attributes
                */
               fildes = open_stack[stacktop].fildes;
               kern = open_stack[stacktop].kern;
               block = open_stack[stacktop].block;
            }
           }
           break;
         case 's':         /* Start Device test */
      case 'S': if (start_test(fildes)) started++;
           break;
         case 'h':         /* Halt Device test */
         case 'H': if (halt_test(fildes)) started--;
           break;
         case 'l':         /* Query Statistics Test */
         case 'L': statistics_test(fildes);
           break;
         case 'e':         /* Edit Defaults */
      case 'E': edit_defaults(fildes);
           break;
      case 'v':         /* Query Vital Product Data */
      case 'V': query_vpd_test(fildes);
           break;
      case 'i':         /* FDDI info test */
      case 'I': fddiinfo_test(fildes);
           break;
      case 'j':         /* Tx storm Test */
      case 'J': tx_storm(fildes);
           break;
      case 'w':         /* Write Test */
      case 'W': write_test(fildes);
           break;
      case 'g':         /* Read POS REG Test */
      case 'G': get_status_test(fildes);
           break;
                  /* download Test */
      case 'D': download_test (fildes);
           break;
                  /* Mem access Test */
      case 'M': fddi_mem_acc (fildes);
           break;
      case 'r':         /* Read Test */
      case 'R': read_test(fildes);
           break;
      case 'a':         /* set address Test */
      case 'A': set_address_test(fildes);
           break;
      case 'x':         /* see how many opens */
      case 'X': max_open_test(fildes);
           break;
      case 'u':         /* query address Test */
      case 'U': query_address_test(fildes);
           break;

      case 'f':         /* fastwrite from kernel Test */
      case 'F': fastwrite_test(fildes);
           break;

#ifdef __DEAD_CODE__
      case 'p':
      case 'P': rcv_cmd(fildes); /* issue receive command */
           break;
#endif /* __DEAD_CODE__ */

      case 'k':         /* getstatus form kernel Test */
      case 'K': kid_getstatus(fildes);
           break;

#ifdef __DEAD_CODE__
                  /* read adapter state */
      case 'T': read_state(fildes);
           break;
#endif /* __DEAD_CODE__ */

      case 'y':         /* clear hw address Test */
      case 'Y': clear_hw_addr(fildes);
           break;

      case 'z':         /* illegal ioctl calls */
      case 'Z': illegal_ioctl (fildes);
           break;

#ifdef __XTRA_CODE__
      case '3': fddi_promisc_on (fildes);/* Promiscuous mode on*/
           break;

      case '4': fddi_promisc_off (fildes);/* Promiscuous mode off */
           break;

      case '5': fddi_bad_frame_on (fildes);/* Bad Frame on */
           break;

      case '6': fddi_bad_frame_off (fildes);/* Bad Frame off */
           break;

      case '7': fddi_beacon_on (fildes);/* Beacon On*/
           break;

      case '8': fddi_beacon_off (fildes);/* Beacon off*/
           break;

      case '9': fddi_smt_on (fildes);/* SMT On */
           break;

      case 't': fddi_smt_off (fildes);/* SMT Off */
           break;

      case 'b': fddi_nsa_on (fildes);/* NSA On */
           break;

      case 'm': fddi_nsa_off (fildes);/* NSA Off */
           break;

      case 'd': fddi_bad_pkt_read (fildes);/* read bad pkt */
           break;
#endif /* __XTRA_CODE__ */

      case '1': Server(fildes);
           break;

      case '2': Client(fildes);
           break;
      case '\n':        /* Refresh screen */
           break;
         default:
          invalid = TRUE;     /* Invalid character */
          break;
       }
   }
}

/*------------------------  S T A R T _ T E S T  -----------------------*/
/*                         */
/*  NAME: start_test                   */
/*                         */
/*  FUNCTION:                       */
/* Issues an ioctl CIO_START to the driver under test (via the */
/* file descriptor "fildes") and decodes returned error codes, */
/* if any.  Optionally waits for the asynchronous start complete  */
/* block before returning.                */
/*                         */
/*  EXECUTION ENVIRONMENT:                */
/* Can only be executed at the user process level.       */
/*                         */
/*  DATA STRUCTURES:                   */
/* References the global profile structure for default netid   */
/* information.  If wait for status is selected, the Ethernet      */
/* Address returned in the status is stored in the profile  */
/* block (Ethernet only).                 */
/*                            */
/*  RETURNS:                              */
/* TRUE if the start succeeded.              */
/* FALSE if the start failed.             */
/*                         */
/*----------------------------------------------------------------------*/

start_test (fildes)
   int   fildes;
{
   struct session_blk   session;
   int   result, netid, rc = TRUE;

   printf("\n\t***********************  START  ************************");
   result = prompt("\n\tNetid (hex)?", "%x", profile.default_netid);
   if (result < 0) return (FALSE);
   session.netid = result;
   session.status = 0;
   result = IOCTL( fildes, CIO_START, &session );
   if (result < 0) {       /* if error */
       switch (errno) {
      case EIO:
             fprintf(STDERR, "\n\tERROR status returned -- ");
          show_session_blk( &session, STDERR );
          break;
      default:
             fprintf(STDERR, "\n\t");
          decode_error(errno);
          break;
       }
       rc = FALSE;
   }
   else
   {
       if (rc && y_or_n("\tWait for status", TRUE))
       {
      rc = wait_status(fildes, CIO_START_DONE, STATUS_TIMEOUT, TRUE);
       }
       if (rc)
       {
         printf("\n\t -- IOCTL Start succeeded --");
      show_session_blk( &session, stdout );
       }
   }
   printf("\n\t********************************************************");
   return(rc);
}

/*-------------------------  H A L T _ T E S T  ------------------------*/
/*                         */
/*  NAME: halt_test                    */
/*                         */
/*  FUNCTION:                       */
/* Issues an ioctl CIO_HALT to the driver under test (via the  */
/* file descriptor "fildes") and decodes returned error codes, */
/* if any.                                                        */
/*                         */
/*  EXECUTION ENVIRONMENT:                */
/* Can only be executed at the user process level.       */
/*                         */
/*  DATA STRUCTURES:                   */
/* Modifies no global data structures, references the global   */
/* profile block for netid information.            */
/*                            */
/*  RETURNS:                              */
/* TRUE if the halt succeeded.               */
/* FALSE if the halt failed.              */
/*                         */
/*----------------------------------------------------------------------*/

halt_test (fildes)
   int   fildes;
{
   struct session_blk   session;
   int   result, rc = TRUE, netid;

   printf("\n\t**********************   HALT   ************************");
   result = prompt("\n\tUse what netid (hex)", "%x",
            profile.default_netid);
   if (result < 0) return(FALSE);
   session.netid = result;
   if (session.length < 0) return(FALSE);
   session.status = 0;
   result = IOCTL( fildes, CIO_HALT, &session );
   if (result < 0) {       /* if error */
       switch (errno) {
      case EIO:
             fprintf(STDERR, "\n\tERROR status returned -- ");
          show_session_blk( &session, STDERR );
          break;
      default:
             fprintf(STDERR, "\n\t");
          decode_error(errno);
          break;
       }
       rc = FALSE;
   } else {
       if (rc) {
           printf("\n\t-- IOCTL Halt succeeded --");
      show_session_blk( &session, stdout );
       }
   }
   printf("\n\t********************************************************");
   return(rc);
}


/*-------------------   Q U E R Y _ V P D _ T E S T  -------------------*/
/*                         */
/*  NAME: query_vpd_test                  */
/*                         */
/*  FUNCTION:                       */
/* Issues an ioctl GET_VPD to the driver under test (via the   */
/* file descriptor "fildes") and decodes returned error codes, */
/* if any.  If the query does not fail, the vital product data */
/* is decodes and formatted to stdout.                            */
/*                         */
/*  EXECUTION ENVIRONMENT:                */
/* Can only be executed at the user process level.       */
/*                         */
/*  DATA STRUCTURES:                   */
/* References no global data structures.           */
/*                            */
/*  RETURNS: nothing                      */
/*                         */
/*----------------------------------------------------------------------*/

query_vpd_test (
   int   fildes)
{
   int   result;
   VPDATA   vpd;
   struct   cfg_dd   cfg;        /* dd config structure */

   printf("\n\t******** SYSCONFIG QUERY VITAL PRODUCT DATA  ************");
   /*
    * Initialize the cfg structure for a call to sysconfig(), which
    * will in turn call the ddconfig() section of the appropriate
    * device driver.
    */

   cfg.devno = getdevno();    /* concatenated Major#, and Minor# */
   cfg.cmd = CFG_QVPD;     /* Command to read VPD */
   cfg.ddsptr = (char *) &vpd;   /* Storage space for VPD */
   cfg.ddslen = sizeof(vpd);  /* Size of VPD storage area */

/*   cfg.kmid = loadext ("fddidd", FALSE, TRUE); */
     cfg.kmid = NULL;
   if (sysconfig (SYS_CFGDD, &cfg, sizeof(cfg)) < 0)
   {
      fprintf(STDERR, "\n\t");
      fprintf(STDERR,
         "sysconfig() failed  devno=%x kmid=%x cmd=%d ddsptr=%x ddslen=%d\n",
               cfg.devno, cfg.kmid, cfg.cmd, cfg.ddsptr, cfg.ddslen);
      decode_error(errno);
   }
   else
   {
      printf("\n\tsysconfig Query VPD succeeded:\n");
      printf("\n\tPrimary VPD:\n");
      dump_vpd(vpd.status, vpd.l_vpd, vpd.vpd, 1);
      printf("\n\tSeconday VPD:\n");
      dump_vpd(vpd.xc_status, vpd.l_xcvpd, vpd.xcvpd, 0);
      printf("\n\t Press ENTER to continue "); getinput();
   }
   printf("\n\t********************************************************");
}
static int
dump_vpd (ulong stat, ulong len, uchar *p_vpd, int primary)
{

   switch (stat)
   {
      case FDDI_VPD_NOT_READ:
         fprintf(STDERR,
         "\tStatus code: VPD_NOT_READ returned.");
         break;
      case FDDI_VPD_INVALID:
         fprintf(STDERR,
         "\tStatus code: VPD_INVALID returned.");
         break;
      case FDDI_VPD_VALID:
         printf(
         "\tStatus code: VPD_VALID returned.");
         decode_vpd(p_vpd);
         break;
      default:
         fprintf(STDERR,
         "\tStatus code: Undefined return %d.", stat);
         break;
   }
   return ;
}

/*-----------------------   W R I T E _ T E S T   ----------------------*/
/*                         */
/*  NAME: write_test                   */
/*                         */
/*  FUNCTION:                       */
/* Writes a test frame to the open device associated with fildes. */
/* On network devices, the header of the frame is filled in with  */
/* the destination address, source address, and netid; the     */
/* remainder of the buffer is filled with the prompted for     */
/* length of data minus the header size.  On all other drivers,   */
/* the entire buffer is filled with test data.  Status can be  */
/* requested from the driver per every number of writes, this  */
/* is used to pace repeated writes to the size of the driver's */
/* input queues.                    */
/*                         */
/*  EXECUTION ENVIRONMENT:                */
/* Can only be executed at the user process level.       */
/*                         */
/*  DATA STRUCTURES:                   */
/* References the global profile block for defaults.     */
/*                            */
/*  RETURNS:                              */
/* TRUE  The write(s) completed successfully.         */
/* FALSE An error occurred during a write.         */
/*                         */
/*----------------------------------------------------------------------*/

write_test (fildes)
   int   fildes;
{
   WRITE_EXT   *p_ext, ext;
   int      result,
         writlen,
         count,
         rc,
         i,
         waitstat = FALSE,
         statusrate,
         done = FALSE;
   char     scratch[ 30 ];
   struct timestruc_t   ts, te;

   bzero(&ext, sizeof(ext));
   rc = TRUE;
   printf("\n\t**********************   WRITE   ***********************");
   if (kern)
   {
      printf("\n");
      (void) get_fastwrite_parms (&ext);
   }
   else
   {
      bzero (&fw_ext, sizeof (ext));
   }
   while (!done) {
       printf("\n\tDestination Address (6 bytes hex) [%s]: ",
         profile.default_dest);
       i = 0;
       while ((str[i++] = getinput()) != '\n');
       if (str[0] == '.') return(FALSE);
       str[i-1] = '\0';
       if (str[0] == '\0') {
      hexstring_to_array(profile.default_dest, writbuf.hdr.dest);
      break;
       }
       else
       {
           done = (strlen(str) == 12) &&
           hexstring_to_array(str, writbuf.hdr.dest);
      strncpy (profile.default_dest, str, 12);
       }
   }
   result = prompt("\tWhat is the destination netid (hex)",
            "%x", profile.default_netid);
   if ( result < 0 ) return(FALSE);
   sprintf( profile.default_netid, "%x", result );
   writbuf.hdr.data[0] = result;

   result = prompt("\tWhat is the TRF Flag (00,02,04,06)",
            "%x", profile.default_trf);
   if ((result != 00) && (result != 02) && (result!=04) && (result != 06))
      return(FALSE);
   sprintf( &profile.default_trf[0], "%x", result );
   writbuf.hdr.flag = result;

   Get_Hw_Addr(fildes, writbuf.hdr.src);
   /*
    * Set Frame Control
    * fc = Synchronous data, LLC type, 48 bit address
    */
   writbuf.hdr.fc = 0;
   if (result == FDDI_SMT_NETID)
   {
       writbuf.hdr.fc |= FDDI_SMT_TYPE;
   }
   else
   {
       writbuf.hdr.fc |= FDDI_LLC_TYPE;
   }
        writlen = prompt("\t Enter bytes of test data", "%d",
         profile.default_size);
   sprintf( profile.default_size, "%d", writlen );
   if (writlen == -1)
   {
      return(FALSE);
   }
   writlen += DATALINK_HDR;
   count =
       prompt("\tIssue how many writes", "%d", profile.default_writes);
   if (count < 0)
   {
      return(FALSE);
   }
   sprintf( profile.default_writes, "%d", count );
   statusrate = prompt("\tWait for transmit status per how many writes",
            "%d", profile.statusrate);
   sprintf( profile.statusrate, "%d", statusrate );
   if (statusrate < 0)
   {
      return(FALSE);
   }
   gettimer(TIMEOFDAY, &ts);
   for (i = 1; i <= count; i++)
   {
       if (profile.use_tx_ext)
       {
          ext.ciowe.flag = 0;
          if (!profile.free_mbuf)
          {
         ext.ciowe.flag |= CIO_NOFREE_MBUF;
          }
          ext.ciowe.status   = 0;
          ext.ciowe.write_id = i;
          ext.ciowe.netid = writbuf.hdr.data[0];
          if (ext.fastwrt)
          {
            ext.frames_req = statusrate;
          }
          if ((statusrate) && (waitstat = !(i % statusrate)))
          {
             /* time to wait for a status */
             ext.ciowe.flag |= CIO_ACK_TX_DONE;
          }
          p_ext = &ext;
       }
       else
       {
          p_ext = NULL;
       }
       while ((result = WRITE( fildes, &writbuf, writlen, p_ext )) < 0)
       {
      if ((errno == EAGAIN) ||
         ((errno == EIO) && (p_ext->ciowe.status == CIO_TX_FULL)))
      {
         continue;
      }
      break;
       }
       if (result < 0)
       {
      decode_error(errno);
         fprintf(STDERR, "\n\tWrite %d ", i);
      if (errno == EIO)
      {
         if (p_ext == NULL)
         {
            fprintf(STDERR, "ERROR EIO with no ext\n");
         }
         else
         {
            /*
             * look in the status field
             */
            fprintf(STDERR, "\t\tStatus code: %x, %s\n",
               p_ext->ciowe.status,
               convcode (p_ext->ciowe.status));
         }
      }
         rc = FALSE;
      break;
       }
       if (waitstat)
       {
      if (!wait_status(fildes, CIO_TX_DONE, TX_ACK_TIMEOUT, TRUE))
      {
             /* timed out or error */
             fprintf(STDERR, "\n\tWrite %d ", i);
          rc = FALSE;
          break;
      }
       }
   }
   if (rc)
   {
      double   t;

         printf("\n\t-- Write succeeded. --");
      if (p_ext)
         decode_write_ext (p_ext);
      gettimer (TIMEOFDAY, &te);
      t = (te.tv_sec + te.tv_nsec/1e9) -
         (ts.tv_sec + ts.tv_nsec/1e9);
      printf ("\n\t[(bytes=%d) PER (sec=%f)] == %f\n",
         count*writlen, t,
         (count*writlen)/(t));
   }
   else
   {
       fprintf(STDERR, "\n\tWrite Parameters:");
       if (kern)
      fprintf(STDERR, "\n\t   Device %d, KERNEL mode", unit);
       else
      fprintf(STDERR, "\n\t   Device %d, USER mode", unit);
       if (block)
      fprintf(STDERR, "\n\t   NDELAY off (blocking)");
       else
      fprintf(STDERR, "\n\t   NDELAY on (nonblocking)");
       if (profile.free_mbuf)
      fprintf(STDERR, "\n\t   Mbufs freed by driver");
       else
      fprintf(STDERR, "\n\t   Mbufs not freed by driver");
       if (waitstat)
      fprintf(STDERR, "\n\t   Wait for TX status");
       else
      fprintf(STDERR, "\n\t   Don't wait for TX status");
   }
   printf("\n\t********************************************************");
   return(rc);
}
int
tx_storm (int fildes)
{
   struct timestruc_t   ts, te;
   double         t;
   int         rc;
   TX_STORM    tx;

   if (!kern)
   {
      fprintf(STDERR,"\n\tNot in Kernel Mode.");
      return (1);
   }
   gettimer(TIMEOFDAY, &ts);
   tx.num = prompt("\tTx how many frames?", "%d", "1000");
   tx.siz = prompt("\tHow many frames/Fastwrite call?", "%d", "16");
   rc = IOCTL(fildes, KID_TX_STORM, &tx);
   if (rc)
   {
      decode_error(errno);
      return (rc);
   }
   gettimer (TIMEOFDAY, &te);
   t = (te.tv_sec + te.tv_nsec/1e9) - (ts.tv_sec + ts.tv_nsec/1e9);
   printf ("\n\tElapsed time == %f\n", t);
   return (0);
}
/*------------------------   R E A D _ T E S T   -----------------------*/
/*                         */
/*  NAME: read_test                    */
/*                         */
/*  FUNCTION:                       */
/* Reads frames from the driver under test and prints them to  */
/* stdout.                       */
/*                         */
/*  EXECUTION ENVIRONMENT:                */
/* Can only be executed at the user process level.       */
/*                         */
/*  DATA STRUCTURES:                   */
/* References the global profile block for defaults.     */
/*                            */
/*  RETURNS:                              */
/* TRUE  The read(s) completed successfully.       */
/* FALSE An error occurred during a read.       */
/*                         */
/*----------------------------------------------------------------------*/

read_test (fildes)
   int   fildes;
{
   int         verify, show;
   int         result, rc, count, i, j;
   int         length, errors;
   struct pollfd     pollblk;
   struct read_extension   ext;
   struct read_extension   *p_ext;


   printf("\n\t************************  READ  ************************");
   bzero(&readbuf, sizeof(readbuf));
   count = prompt("\n\tRead how many frames", "%d",
      profile.default_reads);
   if (count <= 0)
   {
      return(FALSE);
   }
   sprintf( profile.default_reads, "%d", count );
   sprintf(str, "%d", FDDI_MAX_LLC_PACKET);
   length = prompt("\tRead how many bytes per frame", "%d", str);
   if (length < 0)
   {
      return(FALSE);
   }
   show = y_or_n("\tDisplay each frame", FALSE);
   if (show < 0)
   {
      return(FALSE);
   }
   /*
    * setup read extension (even though nothing goes in it
    * the driver will take a different path depending
    * on whether it is present or not)
    */
   p_ext = (profile.use_rcv_ext == TRUE) ? p_ext = &ext : NULL;

   for (i = 1; i <= count; i++)
   {
       /*
        * if non blocking open and poll_reads on then poll
           */
       if (!block)
       {
         pollblk.fd = fildes;
         pollblk.reqevents = POLLIN;
         pollblk.rtnevents = 0;
         poll (&pollblk, 1, READ_TIMEOUT);
         result = READX( fildes, &readbuf, length, p_ext );
       }
       else
       {
         /*
          * READ_TIMEOUT is in milliseconds ... set the alarm
          * for number of seconds requested
          */
         alarm (READ_TIMEOUT/100);
         result = READX( fildes, &readbuf, length, p_ext );
         alarm (0);
       }
       if (result < 0)        /* if error */
       {             /* show it */
         fprintf(STDERR, "\n\tRead %d: ", i);
      decode_error(errno);
      if (p_ext)
      {
         fprintf(STDERR, "\n\tStatus: %d %s\n", p_ext->status,
            convcode (p_ext->status));
      }
         rc = FALSE;       /* return failed */
       }
       else
       {
      rc = TRUE;
      if ( !result )
      {
          fprintf( STDERR, "\n\tNo data on read %d.", i );
          break;
      }
      if ( show )
      {
         printf("\n\tFrame %d: ", i);
         print_buffer(&readbuf, result, 0, stdout);
      }
      else
      {
         if (i == 1)
            printf ("\n");
         printf ("\tReading frame [%d]\r", i);
      }
       }
       if (!rc)
      break;
   }  /* end for loop */
   if (rc)
   {
       printf("\n\tSuccessfully Read [%d] packets. --", i-1);
   }
   else
   {
       fprintf(STDERR, "\n\tRead Parameters:");
       if (kern)
      fprintf(STDERR, "\n\t   Device %d, KERNEL mode", unit );
       else
      fprintf(STDERR, "\n\t   Device %d, USER mode", unit );
       if (block)
      fprintf(STDERR, "\n\t   DNDELAY off (blocking)");
       else
      fprintf(STDERR, "\n\t   DNDELAY on (nonblocking)");
   }
   printf("\n\t********************************************************");
   return(rc);
}

/*-------------------  S T A T I S T I C S _ T E S T  ------------------*/
/*                         */
/*  NAME: statistics_test                 */
/*                         */
/*  FUNCTION:                       */
/* Issues the ioctl CIO_QUERY to the driver under test (via the   */
/* file descriptor "fildes") and decodes returned error codes, */
/* if any.  Optionally clears the statistics by reissuing the  */
/* ioctl with the clear flag turned on.                              */
/*                         */
/*  EXECUTION ENVIRONMENT:                */
/* Can only be executed at the user process level.       */
/*                         */
/*  DATA STRUCTURES:                   */
/* References no global data structures.           */
/*                            */
/*  RETURNS: nothing                      */
/*                         */
/*----------------------------------------------------------------------*/

statistics_test (fildes)
   int   fildes;
{
   int   i, result;
   struct   query_parms parms;
   STATISTICS        stats;


   printf("\n\t******************* QUERY STATISTICS *******************");
   bzero( &parms, sizeof(parms));
   bzero( &stats, sizeof(stats));
   parms.bufptr = (caddr_t)&stats;
   parms.buflen = sizeof(stats);
   result = IOCTL( fildes, CIO_QUERY, &parms );
   if (result < 0)
   {
      fprintf(STDERR, "\n\t");
      decode_error(errno);
   }
   else
   {
      printf("\n\tIOCTL Query Statistics succeeded:\n");
      printf("\n\tTx Bytes   = %.1f", (double) (MAXLONG * stats.cc.tx_byte_mcnt + stats.cc.tx_byte_lcnt));
      printf("\t\tRx Bytes   = %.1f", (double) (MAXLONG * stats.cc.rx_byte_mcnt + stats.cc.rx_byte_lcnt));
      printf("\n\tTx Frames     = %d", stats.cc.tx_frame_lcnt);
      printf("\t\tRx Frames     = %d", stats.cc.rx_frame_lcnt);
      printf("\n\tTx ints       = %d", stats.ds.tx_intr_cnt);
      printf("\t\tRx ints       = %d", stats.ds.rcv_intr_cnt);
      printf("\n\tTx Errors     = %d", stats.cc.tx_err_cnt);
      printf("\t\tRx Errors     = %d", stats.cc.rx_err_cnt);
      printf("\n\tTx que ovflw     = %d", stats.ds.tx_que_ovflw);
      printf("\t\tMax Tx qued   = %d", stats.cc.xmt_que_high);
      printf("\n\tRx que ovflw     = %d", stats.ds.rcv_que_ovflw);
      printf("\t\tMax Rx qued   = %d", stats.cc.rec_que_high);
      printf("\n\tSt que ovflw     = %d", stats.ds.stat_que_ovflw);
      printf("\t\tMax St qued   = %d", stats.cc.sta_que_high);
      printf("\n\tRx cmd cmplt     = %d", stats.ds.rcv_cmd_cmplt);
      printf("\t\tPkts rejected    = %d", stats.ds.pkt_rej_cnt);
      printf("\n\tRx no buf     = %d", stats.ds.rcv_no_mbuf);
      printf("\t\tAdapter err   = %d", stats.ds.adap_err_cnt);
      /*
       * Link Statistics
       */
      printf ("\n\tSMT Err Low  = 0x%x", stats.ls.smt_error_lo);
      printf ("\t\tSMT Err Hi   = 0x%x", stats.ls.smt_error_hi);
      printf ("\n\tSMT Evt Low  = 0x%x", stats.ls.smt_event_lo);
      printf ("\t\tSMT Evt Hi   = 0x%x", stats.ls.smt_event_hi);
      printf ("\n\tConn Policy  = 0x%x", stats.ls.cpv);
      printf ("\t\tPort Event   = 0x%x", stats.ls.port_event);
      printf ("\n\tSetCount Lo  = 0x%x", stats.ls.setcount_lo);
      printf ("\t\tSetCount Hi  = 0x%x", stats.ls.setcount_hi);
      printf ("\n\tACI Code     = 0x%x", stats.ls.aci_code);
      printf ("\t\tPurge Frames = 0x%x", stats.ls.pframe_cnt);

      printf ("\n\tSBA Alloc Lo = 0x%x", stats.ls.sba_alloc_lo);
      printf ("\t\tSBA Alloc Hi = 0x%x", stats.ls.sba_alloc_hi);

      printf ("\n\tT_NEG Lo     = 0x%x", stats.ls.tneg_lo);
      printf ("\t\tT_NEG Hi     = 0x%x", stats.ls.tneg_hi);

      printf ("\n\tSBA Payload  Lo = 0x%x", stats.ls.payload_lo);
      printf ("\t\tSBA Payload  Hi = 0x%x", stats.ls.payload_hi);

      printf ("\n\tSBA Overhead Lo = 0x%x", stats.ls.overhead_lo);
      printf ("\t\tSBA Overhead Hi = 0x%x", stats.ls.overhead_hi);

      printf ("\n\tUcode Vers   = 0x%x", stats.ls.ucode_ver);

      i = (stats.ls.ecm_sm > 8) ? 8 : stats.ls.ecm_sm;
      printf("\n\n\tECM State Machine:       [%d] =   %s\n",
         stats.ls.ecm_sm,
         ecm[i]);

      i = (stats.ls.pcm_a_sm > 10) ? 10 : stats.ls.pcm_a_sm;
      printf("\tPCM State Machine (Port A):  [%d] =   %s\n",
         stats.ls.pcm_a_sm,
         pcm[i]);

      i = (stats.ls.pcm_b_sm > 10) ? 10 : stats.ls.pcm_b_sm;
      printf("\tPCM State Machine (Port B):  [%d] =   %s\n",
         stats.ls.pcm_b_sm,
         pcm[i]);

      i = (stats.ls.cfm_a_sm > 6) ? 6 : stats.ls.cfm_a_sm;
      printf("\tCFM State Machine (Port A):  [%d] =   %s\n",
         stats.ls.cfm_a_sm,
         cfm[i]);

      i = (stats.ls.cfm_b_sm > 6) ? 6 : stats.ls.cfm_b_sm;
      printf("\tCFM State Machine (Port B):  [%d] =   %s\n",
         stats.ls.cfm_b_sm,
         cfm[i]);

      i = (stats.ls.cf_sm > 6) ? 6 : stats.ls.cf_sm;
      printf("\tCF State Machine       [%d] =   %s\n",
         stats.ls.cf_sm,
         cf[i]);

      i = (stats.ls.mac_cfm_sm > 6) ? 6 : stats.ls.mac_cfm_sm;
      printf("\tMAC CFM State Machine        [%d] =   %s\n",
         stats.ls.mac_cfm_sm,
         mcfm[i]);

      i = (stats.ls.rmt_sm > 8) ? 8 : stats.ls.rmt_sm;
      printf("\tRMT State Machine:        [%d] =   %s\n",
         stats.ls.rmt_sm,
         rmt[i]);


   }
   printf("\n");
   if (!(result < 0) && y_or_n("\tClear statistics?", TRUE)) {
       parms.bufptr = (caddr_t)&stats;
       parms.buflen = sizeof(stats);
       parms.clearall = CIO_QUERY_CLEAR;
       result = IOCTL( fildes, CIO_QUERY, &parms );
       if (result < 0) {
           fprintf(STDERR, "\n\t");
           decode_error(errno);
       } else
      printf("\n\tStatistics cleared.");
   }
   printf("\n\t********************************************************");
}

/*--------------------   E D I T _ D E F A U L T S   -------------------*/
/*                         */
/*  NAME: edit_defaults                   */
/*                         */
/*  FUNCTION:                       */
/* Interactively allows the user to edit the contents of the   */
/* profile block -- this allows the user to change defaults */
/* that appear in certain tests without recompiling.     */
/*                         */
/*  EXECUTION ENVIRONMENT:                */
/* Can only be executed at the user process level.       */
/*                         */
/*  DATA STRUCTURES:                   */
/* Modifies the global profile block.           */
/*                            */
/*  RETURNS: nothing                      */
/*                         */
/*----------------------------------------------------------------------*/

edit_defaults (fildes)
   int   fildes;
{
   int   i, done = FALSE;

   printf("\n\t*******************  EDIT DEFAULTS  *******************\n");
   profile.use_rcv_ext = 1;
   i = y_or_n("\tUse receive extension", profile.use_rcv_ext);
   if (i < 0) return;
   profile.use_rcv_ext = i;

   profile.use_tx_ext = 1;
   i = y_or_n("\tUse transmit extension", profile.use_tx_ext);
   if (i < 0) return;
   profile.use_tx_ext = i;

   profile.ack_tx_done = 0;
   profile.free_mbuf = 0;
   if (profile.use_tx_ext)
   {
      i = y_or_n("\tAcknowledge transmit done", profile.ack_tx_done);
      if (i < 0) return;
      profile.ack_tx_done = i;

      i = y_or_n("\tFree mbufs in after write", profile.free_mbuf);
      if (i < 0) return;
      profile.free_mbuf = i;
   }
   sprintf(str, "%d", FDDI_MAX_PCHAIN);
   i = prompt("\tDefault fastwrt frames/request ", "%d", str);
   if (i < 0) return;
   profile.default_frames_req = i;

   i = prompt("\n\tMbuf size threshold", "%d", profile.mbuf_thresh);
   if (i < 0) return;
   sprintf(profile.mbuf_thresh, "%d", i);
   if (openstate && kern)
       IOCTL( fildes, KID_MBUF_THRESH, atoi(profile.mbuf_thresh));

#if 0
   /*
    * For FDDI just edit the above attributes. The following are
    * just constants.
    */
   profile.funky_mbuf = 0;
   i = y_or_n("\tHave mbuf data areas cross page boundary",
                  profile.funky_mbuf);
   if (i < 0) return;
   profile.funky_mbuf = i;
   if (openstate && kern)
       IOCTL( fildes, KID_FUNKY_MBUF,  profile.funky_mbuf );

   while (!done) {
       printf("\tDefault destination address [%s]: ",
         profile.default_dest);
       i = 0;
       while ((str[i++] = getinput()) != '\n');
       if ( str[0] == '.' ) return;
       str[i-1] = '\0';
       if (str[0] == '\0') {
      break;
       } else
      if (strlen(str) == 12) {
          strcpy( profile.default_dest, str );
          done = TRUE;
      }
   }
   done = FALSE;
   while (!done) {
       printf("\tDefault source address [%s]: ",
         profile.default_src);
       i = 0;
       while ((str[i++] = getinput()) != '\n');
       if ( str[0] == '.' ) return;
       str[i-1] = '\0';
       if (str[0] == '\0') {
      break;
       } else
      if (strlen(str) == 12) {
          strcpy( profile.default_src, str );
          done = TRUE;
      }
   }
   i = prompt("\tDefault netid (hex)", "%x", profile.default_netid);
   if (i < 0) return;
   sprintf(profile.default_netid, "%x", i);
   i = prompt("\tDefault number of writes", "%d", profile.default_writes);
   if (i < 0) return;
   sprintf(profile.default_writes, "%d", i);
   i = prompt("\tDefault number of reads", "%d", profile.default_reads);
   if (i < 0) return;
   sprintf(profile.default_reads, "%d", i);
#endif
   printf("\t********************************************************");
}

/*--------------------------   D S O P E N   ---------------------------*/
/*                         */
/*  NAME: dsopen                    */
/*                         */
/*  FUNCTION:                       */
/* Performs all device-specific preliminaries before the driver   */
/* under test is first opened.               */
/*                         */
/*  EXECUTION ENVIRONMENT:                */
/* Can only be executed at the user process level.       */
/*                         */
/*  DATA STRUCTURES:                   */
/* Modifies the global profile block by initializing it to the */
/* compile defaults for this driver.            */
/*                            */
/*  RETURNS:  nothing                     */
/*                         */
/*----------------------------------------------------------------------*/

dsopen ()
{
   profile.default_src[0] = '\0';
   profile.use_tx_ext = TRUE;
   profile.free_mbuf = TRUE;
   profile.ack_tx_done = FALSE;
   profile.poll_reads = TRUE;
   profile.funky_mbuf = FALSE;
   sprintf( profile.mbuf_thresh, "%d", CLBYTES );
   sprintf( profile.mbuf_wdto, "%d", 0 );
   strcpy ( profile.default_dest, DEFAULT_DEST );
   sprintf( profile.default_netid, "%x", DEFAULT_NETID );
   sprintf( &profile.default_trf[0], "%x", DEFAULT_TRF );
   sprintf( profile.default_size, "%d", DEFAULT_SIZE );
   sprintf( profile.default_writes, "%d", DEFAULT_WRITES );
   sprintf( profile.default_reads, "%d", DEFAULT_READS );
   sprintf( profile.statusrate, "%d", DEFAULT_STATUSRATE );
   profile.default_frames_req = FDDI_MAX_PCHAIN;
}

/*--------------------------  D S C L O S E  ---------------------------*/
/*                         */
/*  NAME: dsclose                   */
/*                         */
/*  FUNCTION:                       */
/* Currently does nothing -- can be used to cleanup globals    */
/* after close of driver under test.            */
/*                         */
/*  EXECUTION ENVIRONMENT:                */
/*                         */
/*  DATA STRUCTURES:                   */
/* References no global data structures.           */
/*                            */
/*  RETURNS:  nothing                     */
/*                         */
/*----------------------------------------------------------------------*/

dsclose ()
{
}

/*---------------------  D E C O D E _ S T A T U S  --------------------*/
/*                         */
/*  NAME: decode_status                   */
/*                         */
/*  FUNCTION:                       */
/* Formats the contents of the status block pointed to by      */
/* "status" to stdout.                 */
/*                         */
/*  EXECUTION ENVIRONMENT:                */
/* Can only be executed at the user process level.       */
/*                         */
/*  DATA STRUCTURES:                   */
/* References no global data structures.           */
/*                            */
/*  RETURNS: nothing                      */
/*                         */
/*----------------------------------------------------------------------*/

decode_status (status, fp)
   struct status_block  *status;
   int         fp;      /* stdout or STDERR */
{
   char  *p;

   switch (status->code)
   {
      case CIO_NULL_BLK:
         fprintf (fp,"\n\tStatus code: NULL BLOCK Status\n");
         break;
      case CIO_LOST_STATUS:
         fprintf (fp,"\n\tStatus code: LOST Status\n");
         break;
      case CIO_START_DONE:
         fprintf (fp,"\n\tStatus code: START Done Status\n");
         fprintf (fp,"\tOption[0] = %s\n",
                  convcode(status->option[0]));
         fprintf (fp,"\tOption[1] = %.8x\n",status->option[1]);
         fprintf (fp,"\tOption[2] = %.8x\n",status->option[2]);
         fprintf (fp,"\tOption[3] = %.8x\n",status->option[3]);
         break;
      case CIO_HALT_DONE:
         fprintf (fp,"\n\tStatus code: HALT Done Status\n");
         fprintf (fp,"\tOption[0] = %s\n",
                  convcode(status->option[0]));
         fprintf (fp,"\tOption[1] = %.8x\n",status->option[1]);
         break;
      case CIO_TX_DONE:
         fprintf (fp,"\n\tStatus code: TX Done Status\n");
         fprintf (fp,"\tOption[0] = %s\n",
                  convcode(status->option[0]));
         fprintf (fp,"\tOption[1] = %.8x\n",status->option[1]);
         fprintf (fp,"\tOption[2] = %.8x\n",status->option[2]);
         fprintf (fp,"\tOption[3] = %.8x\n",status->option[3]);
         break;
      case CIO_ASYNC_STATUS:
         fprintf (fp,"\n\tStatus code: ASYNCHRONOUS Status\n");
         fprintf (fp,"\tOption[0] = %s\n",
               convcode (status->option[0]));
         if (status->option[0] == CIO_NET_RCVRY_EXIT)
         {
            break;
         }
         fprintf (fp,"\tOption[1] = %s\n",
               convcode (status->option[1]));
         if (status->option[0] == CIO_NET_RCVRY_ENTER)
         {
            fprintf (fp,"\tOption[2] = 0x%x\n",
               status->option[2]);
            break;
         }
         fprintf (fp,"\tOption[2] = %s\n",
               convcode (status->option[2]));
         if ((status->option[0] == FDDI_RING_STATUS) ||
             (status->option[1] == CIO_HARD_FAIL))
         {
            fprintf (fp,"\tOption[3] = %s\n",
               convcode (status->option[3]));
         }
         break;
      default:
         fprintf (STDERR,"\n\tUnknown Status code: %x\n",
                     status->code);
         fprintf (fp,"\tOption[0] = 0x%x\n", status->option[0]);
         fprintf (fp,"\tOption[1] = 0x%x\n", status->option[1]);
         fprintf (fp,"\tOption[2] = 0x%x\n", status->option[2]);
         fprintf (fp,"\tOption[3] = 0x%x\n", status->option[3]);
         break;
   }
   return ;
}
clear_hw_addr (fildes)
   int fildes;
{
   fddi_set_addr_t      sa;
   int         rc;

   Get_Hw_Addr (fildes, sa.addr);
   sa.opcode = FDDI_DEL;
   set_addr (fildes, &sa);
   return (0);
}
/*---------------------  G E T _ H W _ A D D R  ------------------------*/
/*                         */
/*  NAME: Get_Hw_Addr                     */
/*                         */
/*  FUNCTION:                       */
/* Retrieves the Hardware Address of the adapter associated with  */
/* the driver under test.                 */
/*                         */
/*  EXECUTION ENVIRONMENT:                */
/* Can only be executed at the user process level.       */
/*                         */
/*  DATA STRUCTURES:                   */
/*      Adds the hardware address to default_src in the profile      */
/* block if it is not already defined.          */
/*                            */
/*  RETURNS:                              */
/* TRUE    The hardware address was written to "address".      */
/* FALSE   The hardware address could not be determined.    */
/*                         */
/*----------------------------------------------------------------------*/
Get_Hw_Addr (fildes, address)
   int      fildes;
   unsigned char  *address;
{
   int      i;
   int      result;
   struct devinfo info;

   bzero(&info, sizeof(info));
   result = IOCTL( fildes, IOCINFO, &info );
   if (result < 0)
   {
      fprintf(STDERR, "\n\t");
      decode_error(errno);
   }
   else
   {
      for (i = 0; i < HLEN; i++)
      {
         *(address + i) = info.un.fddi.netaddr[i];
      }
   }
   return(0);
}
get_addr (fildes, address)
   int   fildes;
   char  *address;
{
   char  tmp[32];
   int   i;

   /*
    * Ask user for address if different from hardware address
    */
   printf("\tLocal Address (12 hex chars): [hdwr default] %s",
      (using_tty) ? "\b\b\b\b\b\b\b\b\b\b\b\b\b\b" : "");
   i = 0;
   while ((str[i++] = getinput()) != '\n')
      ;
   str[i-1] = '\0';
   if ((str[0] != '\0') && (str[0] != '.'))
   {
      if ((strlen(str) == 12) && hexstring_to_array(str, tmp))
      {
         /*
          * set new address
          */
         fddi_set_addr_t      sa;

         sa.opcode = FDDI_ADD;
         bcopy (tmp, sa.addr, HLEN);
         if (set_addr(fildes, &sa) == TRUE)
         {
            bcopy (tmp, address, HLEN);
         }
      }
   }
   return (0);
}

/*----------------------  F D D I I N F O _ T E S T  -------------------*/
/*                         */
/*  NAME: fddiinfo_test                   */
/*                         */
/*  FUNCTION:                       */
/* Issues the IOCINFO ioctl to the driver under test, then     */
/* decodes and displays the returned information.        */
/*                         */
/*  EXECUTION ENVIRONMENT:                */
/* Can only be executed at the user process level.       */
/*                         */
/*  DATA STRUCTURES:                   */
/* References no global data structures.           */
/*                            */
/*  RETURNS: nothing                      */
/*                         */
/*----------------------------------------------------------------------*/

fddiinfo_test (fildes)
   int   fildes;
{
   int      result, i;
   struct devinfo info;

   printf("\n\t*********************  FDDIINFO  **********************");

   bzero(&info, sizeof(info));
   result = IOCTL( fildes, IOCINFO, &info );
   if (result < 0) {          /* if error */
       fprintf(STDERR, "\n\t");
       decode_error(errno);
   } else {
       printf("\n\tStatus code: IOCINFO succeeded");
       printf("\n\t   Device Type = ");
       if (info.devtype == DD_NET_DH)
       printf("DD_NET_DH");
       else printf("unknown - %d", info.devtype);

       printf("\n\t   Device Subtype = ");
       if (info.devsubtype == DD_FDDI)
       printf("DD_FDDI");
       else printf("unknown - %d", info.devsubtype);

       printf("\n\t   FDDI Attachment class of [%x] = %s",
      (info.un.fddi.attach_class == 1),
      (info.un.fddi.attach_class == 1) ? "DAS" : "SAS");
       printf("\n\t   FDDI Address in use  = 0x");
       for (i = 0; i < 6; i++)
       {
      printf("%.2x", info.un.fddi.netaddr[i]);
       }
       printf("\n\t   Adapter FDDI Station Address = 0x");
       for (i = 0; i < 6; i++)
       {
      printf("%.2x", info.un.fddi.haddr[i]);
       }
   }
   printf("\n\t********************************************************");
}
int
illegal_ioctl (fildes)
   int fildes;
{
   volatile int   i;
   int   rc;
   struct cmd_tag
   {
      int   cmd;
      char  *str;
   };
   static   struct  cmd_tag   cmds[] =
   {
      { IOCINFO,     "IOCINFO"      },
      { CIO_START,      "CIO_START"       },
      { CIO_HALT,    "CIO_HALT"     },
      { CIO_QUERY,      "CIO_QUERY"       },
      { CIO_GET_STAT,      "CIO_GET_STAT"       },
      { CIO_GET_FASTWRT,   "CIO_GET_FASTWRT"    },
      { FDDI_SET_LONG_ADDR,   "FDDI_SET_LONG_ADDR"    },
      { FDDI_QUERY_ADDR,   "FDDI_QUERY_ADDR"    },
      { FDDI_DWNLD_MCODE,  "FDDI_DWNLD_MCODE"   },
      { FDDI_MEM_ACC,      "FDDI_MEM_ACC"       },
      { FDDI_HCR_CMD,      "FDDI_HCR_CMD"       }
   };
   static int x;
   printf("\n\t*****************  Illegal Ioctls  ********************\n");
   /*
    * Independent of kernel or user mode make illegal ioctl calls.
    */

   rc = 0;
   i = 0;
   fprintf(STDERR, "\ti = %x\n", i);
   for (i = 0; i < sizeof(cmds)/sizeof(struct cmd_tag); i++)
   {
      x = i;
      fprintf(STDERR, "\ti = %x; x = %x; cmd = %x \n", i, x,
            cmds[i].cmd);
      if (kern && (cmds[i].cmd == CIO_GET_STAT))
      {
         /*
          * We tested this else where - here it only
          * tests the kid which we don't case about.
          */
         continue;
      }

      x = i;
      fprintf(STDERR, "\ti = %x; x = %x; cmd = %x \n", i, x,
            cmds[i].cmd);
      rc = IOCTL(fildes, cmds[i].cmd, 0);
      x = i;
      fprintf(STDERR, "\ti = %x; x = %x; cmd = %x \n", i, x,
            cmds[i].cmd);
      if (errno != EINVAL)
      {
         fprintf (STDERR, "\t%s failed with errno=%d\n",
               cmds[i].str);
         continue;
      }
      printf("\t%s correctly failed with EINVAL\n", cmds[i].str);
   }
   printf("\n\t*******************************************************");
   return 0;
}
int
kid_getstatus (fildes)
   int fildes;
{
   int result;
   struct devinfo info;

   /*
    * if we are in kernel mode this should fail - EACCES
    */
   if (kern)
      printf(
      "\n\t******************  GET STATUS  ***********************");
   else
      printf(
      "\n\t******************  ILLEGAL IOCTL  ********************");

   result = IOCTL (fildes, KID_GET_STAT, &info);
   if (result < 0)
   {
      /*
       * if kernel mode then this went through the kid driver
       * and it should return EACCES otherwise the FDDI driver
       * got it and it should be and invalid command.
       */
      if (kern && (errno == EACCES))
      {
         printf ("\n\tkernel get status correctly failed ");
         printf ("with EACCES.\n");
      }
      else if (!kern && (errno == EINVAL))
      {
         printf ("\n\tuser invalid command correctly failed ");
         printf ("with EINVAL.\n");
      }
      else
      {
         decode_error (errno);
      }
   }
   else
   {
      decode_error (errno);
   }
   printf("\n\t*******************************************************");
   return (0);
}
int
fastwrite_test (fildes)
   int fildes;
{
   int result;
   WRITE_EXT   arg;

   /*
    * if !kern this should fail - EACCES
    */
   if (kern)
      return (0);
   printf("\n\t******************  GET FASTWRITE  ********************");
   result = IOCTL (fildes, CIO_GET_FASTWRT, arg);
   if (result < 0)
   {
      if (!kern && (errno == EACCES))
      {
         printf ("\n\tuser get fastwrite correctly failed");
         printf (" with EACCES");
      }
      else
      {
         decode_error (errno);
      }
   }
   printf("\n\t*******************************************************");
   return (0);
}
xlate (char c)
{
   if (c <= 9) c +='0';
   else c += 'W';
   return c;
}
query_address_test (fildes)
   int   fildes;
{
   int         i, j;
   int         done;
   int         result;
   char        str[16];
   char        nib1, nib2;
   fddi_query_addr_t qa;

   printf("\n\t****************  Query Address  ******************");

   bzero (&qa, sizeof (qa));
   result = IOCTL (fildes, FDDI_QUERY_ADDR, &qa);
   if (result < 0)
   {
      decode_error (errno);
   }
   printf ("\n\tStatus code: %s", convcode (qa.status));
   if (qa.status == CIO_OK)
   {
      printf("\n\tAddresses:\n");
      for (i = 0; i < qa.addr_cnt; i++)
      {
         printf ("\t\taddress: %2d> ", i + 1);
         for (j = 0; j < FDDI_NADR_LENGTH; j++)
         {
            nib1 = qa.addrs[i][j] >> 4;
            nib2 = qa.addrs[i][j] & 0xF;
            putchar (xlate(nib1));
            putchar (xlate(nib2));
         }
         printf ("\n");
      }
   }
   printf("\n\t********************************************************");
}
set_address_test (fildes)
   int   fildes;
{
   int         i;
   int         done;
   int         result;
   char        str[16];
   fddi_set_addr_t      sa;

   printf("\n\t****************  Set/Clear Address  ******************");

   bzero (&sa, sizeof (sa));
   /*
    * Get opcode
    */
   result = prompt("\n\t(A) Add or (C) Clear address", "%c", "A");
   switch (result)
   {
      case 'c':
      case 'C':
         sa.opcode = FDDI_DEL;
         break;
      case 'a':
      case 'A':
         sa.opcode = FDDI_ADD;
         break;
      default:
         /*
          * allow for the testing of an invalid command
          */
         sa.opcode = result;
         break;
   }
   /*
    * get address
    */
   done = 0;
   while (!done)
   {
      printf("\n\tAddress to %s (12 hex chars): [000000000000] %s",
         (sa.opcode == FDDI_ADD) ? "SET" : "CLEAR",
         (using_tty) ? "\b\b\b\b\b\b\b\b\b\b\b\b\b\b" : "");
      i = 0;
      while ((str[i++] = getinput()) != '\n')
         ;
      if (str[0] == '.')
      {
         return(FALSE);
      }
      str[i-1] = '\0';
      if (str[0] != '\0')
      {
         done = (strlen(str) == 12) &&
            hexstring_to_array(str, sa.addr);
      }
   }
   (void) set_addr (fildes, &sa);

   printf("\n\t********************************************************");
}
set_addr (fildes, p_sa)
   int      fildes;
   fddi_set_addr_t   *p_sa;
{
   int   result;

   result = IOCTL (fildes, FDDI_SET_LONG_ADDR, p_sa);
   printf ("\tSet/Clear address: ");
   if (result < 0)
   {
      decode_error (errno);
      fprintf (STDERR, "\n\tStatus code: %s", convcode(p_sa->status));
   }
   else
   {
      fprintf (stdout, "\n\tStatus code: %s", convcode(p_sa->status));
      result = TRUE;
   }
   return (result);
}
/*======================================================================*/
/*                      CLIENT/SERVER TEST PROTOCOL                     */
/*======================================================================*/

/*---------------------------  C L I E N T  ----------------------------*/
/*                         */
/*  NAME: Client                    */
/*                         */
/*  FUNCTION:                       */
/* Main program loop for the Client -- prompts for test     */
/* parameters, configures the Server (via the connection), and */
/* generates test packets for the Server.          */
/*                         */
/*  EXECUTION ENVIRONMENT:                */
/* Can only be executed at the user process level.       */
/*                         */
/*  DATA STRUCTURES:                   */
/* Modifies most all of the global data structures in this file.  */
/*                            */
/*  RETURNS:  Nothing                     */
/*                         */
/*----------------------------------------------------------------------*/

Client (fildes)
   int      fildes;
{
   RECV_FRAME  recvbuf;
   XMIT_FRAME  xmitbuf;
   TP    *ttp, *rtp;
   TP_PARMS *tparm, *rparm;
   TP_ARP      *tarp, *rarp;
   TP_RESULT   *rres;
   int      result, i, j, count, ack_spacing, length;
   int      burst_size, load;
   int      pattern_type, seq;
   int      ack_no = 0;
   int      srecv_frames = -1, sxmit_frames = -1, max_errors = 0;
   int      lost_frames = -1, bad_frames = 0, lost_acks = 0;
   int      sbad_frames = -1, errors = 0;
   int      halt_sent = FALSE;
   int      verbose = FALSE;
   int      big, small;
   int      netid;
   char     c;
   struct timestruc_t   tstart;
   int      sav_frames_req;
   int      rc;
   int      clear_addr;

   ttp   = (TP        *)xmitbuf.data;
   rtp   = (TP        *)recvbuf.data;
   tarp  = (TP_ARP    *)ttp->data;
   tparm = (TP_PARMS  *)ttp->data;
   rarp  = (TP_ARP    *)rtp->data;
   rparm = (TP_PARMS  *)rtp->data;
   rres  = (TP_RESULT *)rtp->data;
   xmit_frames = recv_frames = 0;

   printf("\n\t***********************  CLIENT  ***********************");
   count = prompt("\n\tSend how many test frames", "%d", "1000000");
   if (count < 0) return;

   /*  The number of additional writes issued before the first ack */
   /*  is called the "load;" by making the first burst larger than */
   /*  the ack spacing (test burst size), the testcase can force  */
   /*  concurrent packet reception and transmission -- since the   */
   /*  first acks will arrive before the entire first burst is     */
   /*  transmitted, both sides are forced to receive and transmit  */
   /*  at the same time.  This condition exists throughout the     */
   /*  test since the Server is always "ahead" of the Client in    */
   /*  response acks, so the client is never waiting for an ack    */
   /*  to send the next test burst.                                */

   ack_spacing = prompt("\tAck per how many frames", "%d", "10");
   if (ack_spacing < 0) return;
   load = prompt("\tIssue how many additional writes before 1st ack",
         "%d", "5");
   if (load < 0) return;

   /*
    * Determine what kind of sequence the user wants to
    * employ...
    */
   seq = 0;
   while (!seq)
   {
      printf ("\tSending sequence:\n");
      printf ("\t[1] Big, Small, Small, Big ...\n");
      printf ("\t[2] Big, Small, Big ...\n");
      printf ("\t[3] All Small frames ...\n");
      printf ("\t[4] All Big frames ...\n");
      printf ("\t[5] Random frames ...\n");
      seq = prompt("\tWhich sequence?", "%d", "5");
      big = prompt
         ("\tWhat size for Big (or Biggest if Random)?",
         "%d", "4422");
      small = prompt
         ("\tWhat size for Small (or Smallest if Random)?",
         "%d", "10");
      if ((small < 0) || (small > 100))
         small = 10;
      if ((big < small) || (big > (BUFSIZE-TP_HDR_SIZE)))
         big = 4000;
   }
   max_errors = prompt("\tHalt after how many errors", "%d", "1");
   if (max_errors < 0) return;

   verbose = y_or_n("\tShow Transmitted acks?", FALSE);
   if (verbose < 0) return;

   if (kern)
   {
      (void) get_fastwrite_parms(&fw_ext);
      /*
       * if fastwrite is used then
       *    for protocol handshaking make sure the frames/request
       * is one. Save the frames_req to be reinstated after
       * initial handshaking is completed.
       * Also, just zero out load value to simplify the
       * calculation of the proper frames per request
       * and ack spacing values.
       */
      if (fw_ext.fastwrt == TRUE)
      {
         load = 0;
         sav_frames_req = fw_ext.frames_req;
         fw_ext.frames_req = 1;
      }
   }
   else
   {
      sav_frames_req = 0;
      bzero (&fw_ext, sizeof (fw_ext));
   }

   /*
    * prompt for netid
    */
   {
      netid = prompt ("\tEnter NETID:", "%d", "2");
   }

   bzero (Client_Addr, 12);
#if LOCAL_ADDR_OPTION
   /*
    * prompt for address
    */
   get_addr(fildes, Client_Addr);
   clear_addr = TRUE;
#endif

   /*
    * prompt for destination address
    * default is broadcast
    */
   {
   char  tmp[32];
   int   i;

      strcpy (tmp, "FFFFFFFFFFFF");
      printf("\tDestination Addr (12 hex chars): [FFFFFFFFFFFF] %s",
         (using_tty) ? "\b\b\b\b\b\b\b\b\b\b\b\b\b\b" : "");
      i = 0;
      while ((str[i++] = getinput()) != '\n')
         ;
      str[i-1] = '\0';
      if ((str[0] != '\0') && (str[0] != '.'))
      {
         if ((strlen(str) == 12))
         {
            strcpy (tmp, str);
         }
      }
      hexstring_to_array(tmp, BroadCast);
   }
   printf("\tPress ENTER to start test (enter '.' to cancel test) ...");
   c = getinput();
   if ( c == '.' )
   {
       printf("\tTest cancelled\n");
       goto exit;
   }

   /*
    * start an unique netid (ie one that some other open has not started)
    * Sleep for a second after the initial start to give the
    * adapter time to configure (FDDI specific).
    */
   printf("\n\tAttaching Test protocol ...");
   netid = Attach_Protocol(fildes, netid);
   if (netid < 0)
   {
      fprintf (STDERR, "\nFailed attaching test protocol\n");
      decode_error(errno);
      goto exit;
   }
   printf("\n\tNetid ... %.2x\n", netid);
   sleep (1);

   /*  For LAN client/server tests, an ARP handshake must first   */
   /*  take place to resolve the hardware addresses.     */
   /*
    *  The client address is obtained from the VPD. the Server
    * address is resolved by ARP request.
    */

   if (Client_Addr[0] == '\0')
   {
      Get_Hw_Addr(fildes, Client_Addr);
      clear_addr = FALSE;
   }
   printf("\tHardware address . . . ");
   for (i = 0; i < HLEN; i++)
       printf("%.2x", Client_Addr[i]);
   printf ("\n");

   /*  The receive queues must be cleared to eliminate any  */
   /*  residual frames from previous tests.        */

   printf("\tClearing receive queues ...");
   while ((result = Receive(fildes, &recvbuf, sizeof(recvbuf), 100)) > 0);

   /*  A broadcast ARP request is issued to the Server in an   */
   /*  attempt to get the Server's hardware address (this is   */
   /*  similar to a TCP/IP ping with the exception of the ICMP */
   /*  echo).  The broadcast is retried several times in case  */
   /*  the Server is started after the Client.        */

   i = 0;
   printf("\n\tIssuing ARP request to the Server . . .");
   while (!Arp_Request(fildes, netid))
   {
       if (i++ >= ARP_RETRY)
       {
      fprintf(STDERR, "\n\tCould not contact the Server!");
      goto exit;
       }
       printf("\n\tRetrying ARP request . . .");
   }
   for (i = 0; i < HLEN; i++)
   {
       xmitbuf.hdr.dest[i] = Server_Addr[i]; /* dest = server */
       xmitbuf.hdr.src[i]  = Client_Addr[i]; /* source = my addr */
   }
   xmitbuf.hdr.data[0] = netid;     /* netid */

   /*  The ARP handshake completed -- now a test parameters frame */
   /*  is constructed and sent to configure the Server.     */

   xmitbuf.hdr.fc = FDDI_LLC_TYPE;

   strcpy(ttp->src,  CLIENT_PA);    /* My protocol address */
   strcpy(ttp->dest, SERVER_PA);    /* his protocol address */
   ttp->seq_number       = 0;    /* sequence number */
   ttp->operation      = TEST_PARMS;   /* sending test parameters */
   ttp->length         = sizeof(TP_PARMS);   /* length of data */
   tparm->ack_spacing  = ack_spacing;  /* acks per # transmits */
   tparm->pattern_type = pattern_type; /* type of test pattern */
   tparm->max_errors   = max_errors;   /* max error count */

   printf("\n\tSending Test Parameters to the Server . . .");
   while ( errors < max_errors )
   {
       if (!Transmit(fildes, &xmitbuf, PARM_SIZE, errors))
       {
      /* quit if error */
           goto exit;
       }
       /*  The test parameters packet is sent.  The Server reponds  */
       /*  by returning an ack packet with the starting ack number. */

       if ((ack_no = Wait_Ack(fildes)) < 0) /* get next ack (timeout?) */
       {
           lost_acks++; errors++;
      if ( errors >= max_errors )
      {
               fprintf(STDERR,
         "\n\tLost contact with the Server! Waiting Ack");
               goto exit;
      }
      fprintf (STDERR, "\n\tRetry Sending Test Parameters");
       }
       else
       {
           printf("\n\tThe Server acknowledged the test parameters.");
      lost_acks = errors = 0;    /* forgive and forget */
      break;
       }
   }
   /*  Finally, the send/ack loop is started -- the first burst is   */
   /*  augmented by the load size:              */

   ttp->operation = TEST_PATTERN;      /* sending test parameters */
   j = 0;
   burst_size = load + ack_spacing; /* first burst */
   /*
    * We are through with protocol handshaking so we can
    * reset the frames per request back.
    * (BUT with adjustments to ensure client/server
    * handshaking is preserved).
    */
   if (fw_ext.fastwrt == TRUE)
   {
      /*
       * WARNING: MBUF_THRESH must be PAGESIZE
       */
      /*
       * First, ensure that the number of descriptors used
       * per request is less than the FDDI_MAX_TX_DESC
       * value. (PAGESIZE == one descriptor)
       */
      if ((big + TP_HDR_SIZE) > PAGESIZE)
      {
         /*
          * Potential for 2 descriptors per transmit so make
          * sure the frames_req will all fit on
          * descriptor queue.
          */
         sav_frames_req =
            MIN(sav_frames_req, FDDI_MAX_TX_DESC/2);
      }
      /*
       * Second, make sure the frames per request is an
       * even multiple of the ack_spacing -
       * or we will be waiting for an acknowledgement for
       * transmits that the kid driver hasn't even sent out
       * yet.
       */
      if (ack_spacing & 0x1)
      {
         /*
          * ack spacing is odd therefore the frames/request
          * can only be the same value as ack_spacing
          * or 1.
          */
         fw_ext.frames_req = (sav_frames_req < ack_spacing) ?
                  1 : ack_spacing;
      }
      else
      {
         /*
          * ack spacing is even therefore the frames/request
          * can be the same value as ack_spacing or
          * any even multiple of ack_spacing.
          */
         int   ack = ack_spacing;

         while (ack)
         {
            if (sav_frames_req >= ack)
            {
               sav_frames_req = ack;
               break;
            }
            ack -= 2;
         }
         fw_ext.frames_req = sav_frames_req;
      }
      printf ("\n\tFrames per request = %d\n", fw_ext.frames_req);
   }
   printf("\n\n\t--  Test in Progress  --\n");
   /*
    *    Take a time stamp
    */
   (void) gettimer (TIMEOFDAY, &tstart);

   /* TRANSMIT RECEIVE LOOP: */
   while (TRUE)
   {
      /* TRANSMIT LOOP: */
      for (i = j; i < (j + burst_size); i++)
      {
         length = getlength(seq, big, small);
         ttp->length = length;
         ttp->seq_number = i;
#if 0
         trace_client(TP_HDR_SIZE + length, i);
#endif
         result = Transmit(fildes, &xmitbuf, (TP_HDR_SIZE + length),i);
         if (!result)
         {
            printf ("\tno xmit\n");
            goto exit;
         }
      }
      if (verbose)
      {
         printf("\t-- Transmitted frames: [%d] --\r", xmit_frames);
      }
      /*  The test frame sequence # is updated by the burst    */
      /*  size, the burst size is then updated to either the   */
      /*  remaining # of test frames or the ack spacing,    */
      /*  whichever is smaller.  If the last test frame was    */
      /*  sent, a halt frame is sent to stop the Server. */

      j += burst_size;
      burst_size = MIN( ack_spacing, count - j );
      if ((burst_size == 0)  && (!halt_sent))
      {
         Send_Halt(fildes, Server_Addr, netid);
         halt_sent = TRUE;
      }

      /* RECEIVE: */
      result = Receive(fildes, &recvbuf, sizeof(recvbuf), SERVER_TIMEOUT);
      if (result < 0)
      {
         printf("\n\tTry another RECEIVE\n");
         /*
          * timeout on read
          */
         result = Receive(fildes,
               &recvbuf,
               sizeof(recvbuf),
               SERVER_TIMEOUT);
         if (result < 0)
         {
            fprintf(STDERR,"\n\tLost contact with Server! rcv timeout");
            break;
         }
      }
      /*  For LAN Client/Server tests, acknowledgements are*/
      /*  returned as packets containing a sequence number.*/

      if (strcmp(rtp->dest, CLIENT_PA))
      {
         fprintf(STDERR, "\n\tCorrupted Frame received!");
         save_bad_frame(&recvbuf,
               result,
               RANDOM_PATTERN,
               ++bad_frames,
               TP_HDR_SIZE,
               Cfile);
         if (++errors >= max_errors)
         {
            Send_Halt(fildes, Server_Addr, netid);
            halt_sent = TRUE;
         }
         /*
          * Receive another (TBD)
          */
      }
      else if (rtp->operation == TEST_ACK)
      {
         /*
          * ACK Frame:
          */
#ifdef DDT_TRACE
         trace_rack (rtp->seq_number);
#endif
         if (rtp->seq_number != ++ack_no)
         {
            int   ack_fail = TRUE;

            fprintf(STDERR, "\n\tWrong ACK: Expect ack %x, Rcv'd ack %x", ack_no, rtp->seq_number);
            if ((rtp->seq_number + 1) == ack_no)
            {
               /*
                * Server resent ack and we may have both of
                * them so this Receive will catch us up.
                * Wait half the time for this one so we
                * have time to respond to the ack we did
                * receive.
                */
               result = Receive(
                     fildes,
                     &recvbuf,
                     sizeof(recvbuf),
                     SERVER_TIMEOUT/2);
               if (result < 0)
               {
                  fprintf (STDERR, "\n\t2nd Receive ack failed\n");
               }
               else if (rtp->seq_number == ack_no)
               {
                     fprintf (STDERR, "\n\tReceived Ack: %x", ack_no);
                     fprintf (STDERR, "\n\tRecovered from Out of Sequence Ack\n");
                     ack_fail = FALSE;
               }
            }
            if (ack_fail == TRUE)
            {
               fprintf(STDERR, "\n\tRecover failed: rcv size = %d\n", result);
               lost_acks += (rtp->seq_number - ack_no);
               ack_no = rtp->seq_number;
               if (++errors >= max_errors)
               {
                  Send_Halt(fildes, Server_Addr, netid);
                  halt_sent = TRUE;
               }
            }
         }
         /*
          * Ack received in sequence
          */
      }
      else if (rtp->operation == TEST_RESULTS)
      {
         printf("\n\tTest results received.");
         lost_frames  = rres->lost_frames;
         sbad_frames  = rres->bad_frames;
         sxmit_frames = rres->xmit_frames + 1;
         srecv_frames = rres->recv_frames;
         break;
      }

   } /* End While TRUE loop */
exit:
   /*
    *    Clear status queue
    */
   {
      struct status_block  status;
      int         rc;

      while ((rc = IOCTL(fildes, CIO_GET_STAT, &status)) >= 0)
      {
         decode_status (&status, stdout);
         if (status.code == CIO_NULL_BLK)
         {
            break;
         }
      }
   }
   /*
    *    Take ending time stamp
    */
   {
      struct   query_parms parms;
      STATISTICS        stats;
      struct timestruc_t   tend;

      (void) gettimer (TIMEOFDAY, &tend);
      bzero( &parms, sizeof(parms));
      bzero( &stats, sizeof(stats));
      parms.bufptr = (caddr_t)&stats;
      parms.buflen = sizeof(stats);
      result = IOCTL( fildes, CIO_QUERY, &parms );
      if (result >= 0)
      {
         int   totaltime;
         double   bytecnt;

         totaltime = tend.tv_sec - tstart.tv_sec;
         bytecnt = MAXLONG * stats.cc.tx_byte_mcnt + stats.cc.tx_byte_lcnt;
            printf ("\n\t[(bytes=%.3f) PER (sec=%d)] == %.3f\n",
            bytecnt, totaltime,
            bytecnt/totaltime);
      }
#if 0
      /*
       * Clear statistics
       */
      parms.bufptr = (caddr_t)&stats;
      parms.buflen = sizeof(stats);
      parms.clearall = CIO_QUERY_CLEAR;
      result = IOCTL( fildes, CIO_QUERY, &parms );
#endif
   }
#if LOCAL_ADDR_OPTION
   if (clear_addr == TRUE)
   {
      /*
       * Clear address: If it is the hardware address then
       * the driver better not let this happen.
       */
      fddi_set_addr_t   sa;

      sa.opcode = FDDI_DEL;
      bcopy (Client_Addr, sa.addr, 12);
      set_addr (fildes, &sa);
   }
#endif
   Detach_Protocol (fildes, netid);
   if ((lost_acks > 1) || (lost_acks < 0) || (lost_frames > 0) ||
          (bad_frames) || (sbad_frames > 0))
   {
      fprintf(STDERR, "\n\tTests completed with errors:");
      fprintf(STDERR, "\n\t    Client, device %s:", &path[0]);
      fprintf(STDERR, "\n\t    Frames from Server lost:        %d", lost_acks);
      fprintf(STDERR, "\n\t    Data Frames Lost:               ");
      if (lost_frames < 0)
         fprintf(STDERR, "Unknown");
      else
         fprintf(STDERR, "%d", lost_frames);
      fprintf(STDERR, "\n\t    Bad frames received by Client:  %d", bad_frames);
      fprintf(STDERR, "\n\t    Bad frames received by Server:  ");
      if (sbad_frames < 0)
         fprintf(STDERR, "Unknown");
      else
         fprintf(STDERR, "%d", sbad_frames);
   }
   else
   {
      printf("\n\tTests completed successfully.");
   }
   printf("\n\t    Frames transmitted by Client:   %d", xmit_frames);
   printf("\n\t    Frames received by Server:      ");
   if (srecv_frames < 0)
      printf("Unknown");
   else
      printf("%d", srecv_frames);
   printf("\n\t    Frames transmitted by Server:   ");
   if (sxmit_frames < 0)
      printf("Unknown");
   else
      printf("%d", sxmit_frames);
   printf("\n\t    Frames received by Client:      %d", recv_frames);
   close_dumps();
   return;
}

/*-----------------------  A R P _ R E Q U E S T  ----------------------*/
/*                         */
/*  NAME: Arp_Request                     */
/*                         */
/*  FUNCTION:                       */
/* Issues an ARP request to the Server and awaits a reply; if  */
/* the reply times out, FALSE is returned, otherwise the global   */
/* variable Server_Addr is updated with the Server's hardware  */
/* address and TRUE is returned.             */
/*                         */
/*  EXECUTION ENVIRONMENT:                */
/* Can only be executed at the user process level.       */
/*                         */
/*  DATA STRUCTURES:                   */
/* Modifies the global Server_Addr variable.       */
/*                            */
/*  RETURNS:                              */
/* TRUE  The Arp request was sent successfully.       */
/* FALSE An error occurred.               */
/*                         */
/*----------------------------------------------------------------------*/

Arp_Request (fildes, netid)
   int      fildes;
   int      netid;
{
   RECV_FRAME  recvbuf;
   XMIT_FRAME  xmitbuf;
   TP    *ttp, *rtp;
   TP_PARMS *tparm, *rparm;
   TP_ARP      *tarp, *rarp;
   int      result, i, length;

   /*  First, we build an ARP packet and broadcast it to the Server's   */
   /*  protocol address, then we poll receive frames until we get an   */
   /*  ARP reply from the Server -- the Server's hardware address is    */
   /*  read from the reply packet and saved.                            */

   ttp   = (TP       *)xmitbuf.data;
   tarp  = (TP_ARP   *)ttp->data;
   tparm = (TP_PARMS *)ttp->data;
   rtp   = (TP       *)recvbuf.data;
   rarp  = (TP_ARP   *)rtp->data;
   rparm = (TP_PARMS *)rtp->data;
   xmitbuf.hdr.fc = FDDI_LLC_TYPE;
                  /* Fill in packet: */
   strcpy(ttp->src,  CLIENT_PA);    /* My protocol address */
   strcpy(ttp->dest, SERVER_PA);    /* his protocol address */
   ttp->operation  = TEST_ARP;      /* type of test packet */
   ttp->length     = sizeof(TP_ARP);   /* size after header */
   ttp->seq_number = 0;       /* sequence number */
   tarp->type      = ARP_REQUEST;      /* request hw address */
   xmitbuf.hdr.data[0]  = netid;    /* Datalink header: */
   for (i = 0; i < HLEN; i++)       /* fill in hardware addr's */
   {
       tarp->sender[i] = Client_Addr[i];  /* mine */
       xmitbuf.hdr.src[i]  = Client_Addr[i];
       tarp->target[i] = 0;           /* his (unknown) */
       xmitbuf.hdr.dest[i] = BroadCast[i];   /* broadcast */
   }              /* send it via write */
   if (!Transmit( fildes, &xmitbuf, ARP_SIZE, 0 ))
       return(FALSE);         /* quit if error */
   while (TRUE)
   {
       result = Receive(fildes, &recvbuf, sizeof(recvbuf), CLIENT_TIMEOUT);
       if (result < 0)
      return(FALSE);          /* timeout */
       if (rarp->type == ARP_REPLY)       /* and a reply? */
       {
           printf("ARP reply received");
           for (i = 0; i < HLEN; i++)     /* then get */
          Server_Addr[i] = rarp->sender[i];   /* get address. */
      printf("\n\tThe Server's address is ");
      for (i = 0; i < HLEN; i++)
          printf("%.2x", Server_Addr[i]);
      return(TRUE);
       }
   }
}

/*--------------------------  W A I T _ A C K  -------------------------*/
/*                         */
/*  NAME: Wait_Ack                     */
/*                         */
/*  FUNCTION:                       */
/* Reads frames from the connection (fildes) until an Ack frame   */
/* is received or a timeout occurs.          */
/*                         */
/*  EXECUTION ENVIRONMENT:                */
/* Can only be executed at the user process level.       */
/*                         */
/*  DATA STRUCTURES:                   */
/* Modifies no global data structures.          */
/*                            */
/*  RETURNS:                              */
/* -1  Read timed out.                 */
/*  N  Sequence number of receive ack frame.       */
/*                         */
/*----------------------------------------------------------------------*/

Wait_Ack (fildes)
   int   fildes;
{
   RECV_FRAME  recvbuf;
   int      length;
   TP    *tp;

   tp = (TP *)recvbuf.data;
   while (TRUE)
   {
       length = Receive(fildes, &recvbuf, sizeof(recvbuf), CLIENT_TIMEOUT);
       if (length < 0)
       {
      decode_error (errno);
      break;      /* error */
       }

       /*  For LAN client/server tests, the ack will return a  */
       /*  sequence number:               */

       if (tp->operation == TEST_ACK)     /* an ack?  */
       {
      if (!strcmp(tp->dest, CLIENT_PA))   /* for me? */
      {
            return (tp->seq_number);
      }
       }
   }
   return (-1);
}

/*------------------------  S E N D _ H A L T  -------------------------*/
/*                         */
/*  NAME: Send_Halt                    */
/*                         */
/*  FUNCTION:                       */
/* Creates and sends a HALT frame to the network address    */
/* specified by "net_addr".               */
/*                         */
/*  EXECUTION ENVIRONMENT:                */
/* Can only be executed at the user process level.       */
/*                         */
/*  DATA STRUCTURES:                   */
/* Modifies no global data structures.          */
/*                            */
/*  RETURNS:                              */
/* TRUE  The Halt was sent successfully.           */
/* FALSE An error occurred.               */
/*                         */
/*----------------------------------------------------------------------*/

Send_Halt (fildes, net_addr, netid)
   int      fildes;
   unsigned char  *net_addr;
   int      netid;
{
   XMIT_FRAME  xmitbuf;
   TP    *tp;
   int      i;
   int      sav_frames_req;
   int      rc;

   tp   = (TP *)xmitbuf.data;
                  /* Fill in packet: */
   strcpy(tp->src,  CLIENT_PA);     /* My protocol address */
   strcpy(tp->dest, SERVER_PA);     /* his protocol address */
   tp->operation  = TEST_HALT;      /* type of test packet */
   tp->length     = 0;        /* size after header */
   tp->seq_number = 0;        /* null sequence number */
   for (i = 0; i < HLEN; i++)       /* fill in hardware addr's */
   {
       xmitbuf.hdr.src[i]  = Client_Addr[i];
       xmitbuf.hdr.dest[i] = net_addr[i];
   }
   xmitbuf.hdr.data[0] = netid;
   xmitbuf.hdr.fc = FDDI_LLC_TYPE;
   if (fw_ext.fastwrt == TRUE)
   {
      /*
       * for protocol handshaking make sure the frames/request
       * is one. Save the frames_req to be reinstated after
       * initial handshaking is completed.
       */
      sav_frames_req = fw_ext.frames_req;
      fw_ext.frames_req = 1;
   }
   rc = Transmit( fildes, &xmitbuf, TP_HDR_SIZE, 0 );
   fw_ext.frames_req = sav_frames_req;
   return (rc);
}

/*---------------------------  S E R V E R  ----------------------------*/
/*                         */
/*  NAME: Server                    */
/*                         */
/*  FUNCTION:                       */
/* Main program loop for the Server -- waits (with long timeout)  */
/* for the first frame from the Client, handshakes hardware */
/* address and test parameter information, then checks and     */
/* acknowledges test packets.  Results are sent back to the */
/* Client at the end of the test.               */
/*                         */
/*  EXECUTION ENVIRONMENT:                */
/* Can only be executed at the user process level.       */
/*                         */
/*  DATA STRUCTURES:                   */
/* Modifies most all of the global data structures in this file.  */
/*                            */
/*  RETURNS:  Nothing                     */
/*                         */
/*----------------------------------------------------------------------*/

Server (fildes)
   int      fildes;
{
   RECV_FRAME  recvbuf;
   XMIT_FRAME  xmitbuf;
   TP    *ttp, *rtp;
   TP_PARMS *tparm, *rparm;
   TP_ARP      *tarp, *rarp;
   TP_RESULT   *tres;
   int      result, ack_spacing, i, j = 0, length = 0;
   int      pattern_type, pass = TRUE;
   int      max_errors;
   int      bad_frame, ignore_arps = FALSE;
   int      bad_frames = 0, lost_frames = 0;
   int      ack_no = -1, tp_no = -1;
   int      deadlock_ack = FALSE, errors = 0;
   int      verbose = FALSE;
   int      netid;
   int      clear_addr;

   ttp   = (TP        *)xmitbuf.data;
   rtp   = (TP        *)recvbuf.data;
   tarp  = (TP_ARP    *)ttp->data;
   tparm = (TP_PARMS  *)ttp->data;
   tres  = (TP_RESULT *)ttp->data;
   rarp  = (TP_ARP    *)rtp->data;
   rparm = (TP_PARMS  *)rtp->data;
   xmit_frames = recv_frames = 0;

   printf("\n\t***********************  SERVER  ***********************");

   /*  The receive queues must be cleared to eliminate any  */
   /*  residual test frames from previous tests.         */

   printf("\n\tClearing receive queues . . .");
   while ((result = Receive(fildes, &recvbuf, sizeof(recvbuf), 100)) > 0);

   verbose = y_or_n("\n\tShow Received frames?", FALSE);
   if (verbose < 0)
   {
      return;
   }
   /*
    * prompt for the netid
    */
   {
      netid = prompt ("\tEnter NETID:", "%d", "2");
   }
   printf("\n\tAttaching Test protocol . . .");
   netid = Attach_Protocol(fildes, netid);
   if (netid < 0)
   {
      goto exit;
   }
   printf("\n\tNetid ... %.2x\n", netid);

   /*
    * after a start get the hardware address
    */
   bzero (Server_Addr, 12);
   get_addr(fildes, Server_Addr);
   clear_addr = TRUE;

   if (Server_Addr[0] == '\0')
   {
      Get_Hw_Addr(fildes, Server_Addr);
      clear_addr = FALSE;
   }
   printf("\tHardware address . . . ");
   for (i = 0; i < HLEN; i++)
       printf("%.2x", Server_Addr[i]);
   printf("\n");

   /*  Receive processing loop; if a receive timeout occurs,   */
   /*  a retry ack is issued to break any deadlocks.  If the next */
   /*  receive times out, the Client is assumed dead and the test */
   /*  is aborted.                     */

   printf("\tWaiting for a request from the Client . . .");
   while (TRUE)
   {
      length = Receive(fildes, &recvbuf, sizeof(recvbuf), SERVER_TIMEOUT);
      if (length < 0)
      {
         if (deadlock_ack)
         {
             fprintf(STDERR, "\n\tLost contact with Client!");
             lost_frames++; errors++;
             goto exit;    /* then give up */
         }
         else
         {
             /*
              * In the event the Client did not see our first ack
              *   send a second one.
              */
             if (!Send_Ack(fildes, Client_Addr, ack_no, netid))
             {
            goto exit;
             }
             deadlock_ack = TRUE;
             continue;     /* and wait once more */
         }
      }
      /*
       * we got something
       */
      deadlock_ack = FALSE;
      if (verbose)
      {
         printf("\t-- Received frames: [%d]\r", recv_frames);
      }
      if (strcmp(rtp->dest, SERVER_PA))
      {
         fprintf(STDERR, "\n\tCorrupted Frame received:\n");
         if (verbose)
         {
            print_buffer( &recvbuf, length, 0, STDERR );
         }
         save_bad_frame(&recvbuf,
               length,
               RANDOM_PATTERN,
               ++bad_frames,
               TP_HDR_SIZE,
               Sfile);
         bad_frame = TRUE;
         if (++errors >= max_errors)
         {
            break;
         }
      }
      switch (rtp->operation)
      {
         case TEST_ARP:
            if (ignore_arps)
            {
               break;
            }
            printf("\n\tARP Request received:");
            printf("\n\tThe Client's address is ");
            for (i = 0; i < HLEN; i++)
            {
               Client_Addr[i] = rarp->sender[i];
               printf("%.2x", Client_Addr[i]);
            }
            printf("\n\tIssuing ARP reply to the Client . . .");
            strcpy(ttp->src,  SERVER_PA);
            strcpy(ttp->dest, CLIENT_PA);
            ttp->operation = TEST_ARP;
            ttp->length    = sizeof(TP_ARP);
            ttp->seq_number = 0;
            tarp->type     = ARP_REPLY;
            xmitbuf.hdr.fc = FDDI_LLC_TYPE;
            xmitbuf.hdr.data[0] = netid;
            for (i = 0; i < HLEN; i++)
            {
               tarp->sender[i] = Server_Addr[i];
               tarp->target[i] = Client_Addr[i];
               xmitbuf.hdr.src[i]  = Server_Addr[i];
               xmitbuf.hdr.dest[i] = Client_Addr[i];
            }
            ignore_arps = TRUE;
            if (!Transmit( fildes, &xmitbuf, ARP_SIZE, 0 ))
            {
               goto exit;
            }
            break;

         case TEST_PARMS:
            printf("\n\tTest Parameters Packet received");
            ack_spacing  = rparm->ack_spacing;
            pattern_type = rparm->pattern_type;
            max_errors   = rparm->max_errors;
            j = 0;
            printf("\n\tSending ACK to the Client");
            if (!Send_Ack(fildes, Client_Addr, ++ack_no, netid))
            {
               goto exit;
            }
            bad_frame = FALSE;
            printf("\n\n\t--  Test in Progress  --\n");
            break;
         case TEST_PATTERN:
#if 0
            trace_server (rtp->length, rtp->seq_number);
#endif
            if (++tp_no != rtp->seq_number)
            {
               fprintf(STDERR, "\tERROR: Lost Rcv pkt %x %x\n", tp_no, rtp->seq_number);
#if 0
               if ((tp_no + 1) == rtp->seq_number)
               {
                  /*
                   * lost one rcv: see if we can recover
                   */
                  tp_no++;
               }
#endif
               tp_no = rtp->seq_number;
               if (++errors >= max_errors)
               {
                  goto exit;
               }
            }

            /*  If it is time to send an ack to the Client, */
            /*  increment the ack number and send the ack.  */

            if ((ack_spacing != 0) && (!(++j % ack_spacing)) &&
                (!Send_Ack(fildes, Client_Addr, ++ack_no, netid)))
            {
                   goto exit;
            }
            break;
         case TEST_HALT:
            printf("\n\tHalt frame received. ");
            goto exit;
         default:
            fprintf(STDERR, "\n\tUnknown Test Operation received: %d", rtp->operation);
            break;
      }
   }
exit:
   xmitbuf.hdr.fc = FDDI_LLC_TYPE;

   /*  End the test by sending a test results frame to the Client.   */
   /*  Display the results on this screen in case the results do  */
   /*  not make it to the Client.               */

   strcpy(ttp->src,  SERVER_PA);       /* my address */
   strcpy(ttp->dest, CLIENT_PA);       /* his address */
   ttp->operation = TEST_RESULTS;         /* sending results */
   ttp->length = sizeof(TP_RESULT);    /* length of data */
   tres->lost_frames = lost_frames;
   tres->bad_frames  = bad_frames;
   tres->xmit_frames = xmit_frames;
   tres->recv_frames = recv_frames;

   printf("\n\tSending test results to Client . . .");
   if (!Transmit(fildes, &xmitbuf, RESULT_SIZE), 0)
   {
      return;
   }
   if (!lost_frames && !bad_frames)
   {
      printf("\n\tTests completed successfully.");
      printf("\n\t    Number of frames received:     %d", recv_frames);
      printf("\n\t    Number of frames transmitted:  %d", xmit_frames);
   }
   else
   {
      fprintf(STDERR, "\n\tTests completed with errors:");
      fprintf(STDERR, "\n\t    Server, device %s:", &path[0]);
      fprintf(STDERR, "\n\t    Frames from Client lost:       %d", lost_frames);
      fprintf(STDERR, "\n\t    Frames with corrupted data:    %d", bad_frames);
      fprintf(STDERR, "\n\t    Number of frames received:     %d", recv_frames);
      fprintf(STDERR, "\n\t    Number of frames transmitted:  %d", xmit_frames);
   }
   if (clear_addr == TRUE)
   {
      /*
       * Clear address: If it is the hardware address then
       * the driver better not let this happen.
       */
      fddi_set_addr_t   sa;

      sa.opcode = FDDI_DEL;
      bcopy (Server_Addr, sa.addr, 12);
      set_addr(fildes, &sa);
   }
   Detach_Protocol (fildes, netid);
   return;
}

/*------------------------  S E N D _ A C K  ---------------------------*/
/*                         */
/*  NAME: Send_Ack                     */
/*                         */
/*  FUNCTION:                       */
/* Sends an ack frame to the Client at the address "net_addr"; */
/* the "ack_number" is sent in the ack frame.         */
/*                         */
/*  EXECUTION ENVIRONMENT:                */
/* Can only be executed at the user process level.       */
/*                         */
/*  DATA STRUCTURES:                   */
/* References the global Server_Addr variable.        */
/*                            */
/*  RETURNS:                              */
/* TRUE  The Ack was sent successfully.            */
/* FALSE An error occurred.               */
/*                         */
/*----------------------------------------------------------------------*/

Send_Ack (fildes, net_addr, ack_number, netid)
   int      fildes;
   unsigned char  *net_addr;
   int      ack_number;
   int      netid;
{
   XMIT_FRAME  xmitbuf;
   TP    *tp;
   int      i;


   /*  For LAN Client/Server, create an ack containing   */
   /*  the current ack number:            */

   tp = (TP *)xmitbuf.data;
                  /* Fill in packet: */
   strcpy(tp->src,  SERVER_PA);     /* My protocol address */
   strcpy(tp->dest, CLIENT_PA);     /* his protocol address */
   tp->operation = TEST_ACK;     /* type of test packet */
   tp->length    = 0;         /* size after header */
   tp->seq_number = ack_number;     /* ack number */
   for (i = 0; i < HLEN; i++)       /* fill in hardware addr's */
   {
       xmitbuf.hdr.src[i]  = Server_Addr[i];
       xmitbuf.hdr.dest[i] = net_addr[i];
   }
   xmitbuf.hdr.data[0] = netid;
   xmitbuf.hdr.fc = FDDI_LLC_TYPE;
#ifdef DDT_TRACE
   trace_sack (ack_number);
#endif
   return (Transmit( fildes, &xmitbuf, TP_HDR_SIZE, 0 ));
}

/*======================================================================*/
/*               CLIENT/SERVER TEST PROTOCOL PRIMITIVES                 */
/*======================================================================*/

/*------------------------  T R A N S M I T  ---------------------------*/
/*                         */
/*  NAME: Transmit                     */
/*                         */
/*  FUNCTION:                       */
/* Performs a write of the data in "packet" ("length" bytes)   */
/* to the connection specified by "fildes".  Performs several  */
/* retries if EAGAIN is returned.               */
/*                         */
/*  EXECUTION ENVIRONMENT:                */
/* Can only be executed at the user process level.       */
/*                         */
/*  DATA STRUCTURES:                   */
/* Modifies the global transmit counter.           */
/*                            */
/*  RETURNS:                              */
/* FALSE An error or timeout occurred.          */
/* N  The number of this transmit.           */
/*                         */
/*----------------------------------------------------------------------*/

Transmit (fildes, packet, length, write_id)
   int      fildes;
   char     *packet;
   unsigned int   length;
   int      write_id;
{
   WRITE_EXT   ext;
   WRITE_EXT   *p_ext;
   int      retry = 0;
   int      result;

   /*
    * Fill in the extension if we are in kernel mode and using fastwrite
    * or if the user specifically says to use the extension.
    */
   bzero(&ext, sizeof(ext));
   if ((kern && fw_ext.fastwrt) || profile.use_tx_ext)
   {
      p_ext = &ext;
      if (!profile.free_mbuf)
      {
          ext.ciowe.flag |= CIO_NOFREE_MBUF;
      }
      if (profile.ack_tx_done)
      {
          ext.ciowe.flag |= CIO_ACK_TX_DONE;
      }
      ext.fastwrt = fw_ext.fastwrt;
      ext.frames_req = fw_ext.frames_req;
      ext.ciowe.write_id = write_id;
   }
   else
   {
      p_ext = NULL;
   }
   /*
    * If blocking mode then set alarm
    */
   if (block)
   {
      /* set alarm */
      alarm (TX_ACK_TIMEOUT*2/100);
   }
   retry = WRITE_RETRIES;
   while ((result = WRITE( fildes, packet, length, p_ext)) < 0)
   {
      decode_error(errno);
           switch (errno)
      {
         case EAGAIN:
            if (--retry)
            {
               sleep (1);
               continue;
            }
            break;
         case EIO:
            if (p_ext == NULL)
            {
               fprintf(STDERR, "EIO with no ext\n");
            }
            else
            {
               /*
                * look in the status field
                */
               fprintf(STDERR, "\n\tERROR : Status code = %s", convcode (p_ext->ciowe.status));
               if (p_ext->ciowe.status == CIO_TX_FULL)
               {
                  if (--retry)
                  {
                     sleep (1);
                     printf("\nsleep a sec");
                     continue;
                  }
               }
            }
            break;
         default:
            break;
           }
      return(0);
   }
   if (block)
   {
         /* turn off alarm */
      alarm (0);
   }
   if (!(ext.ciowe.flag & CIO_ACK_TX_DONE) ||
            (kern && fw_ext.fastwrt))
   {
      /*
       * If no ack then return or if this is a fastwrt
       * then we are done: no need to wait for status.
       */
      return(++xmit_frames);
   }
   if (wait_status (fildes, CIO_TX_DONE, TX_ACK_TIMEOUT, 0))
   {
      return(++xmit_frames);
   }
   printf ("\tTransmit returning false\n");
   return (0);
}

/*-------------------------  R E C E I V E  ----------------------------*/
/*                         */
/*  NAME: Receive                   */
/*                         */
/*  FUNCTION:                       */
/* Performs a read of "length" bytes to "packet" from the      */
/* connection specified by "fildes".            */
/*                         */
/*  EXECUTION ENVIRONMENT:                */
/* Can only be executed at the user process level.       */
/*                         */
/*  DATA STRUCTURES:                   */
/* Modifies the global receive counter.            */
/*                            */
/*  RETURNS:                              */
/* FALSE An error or timeout occurred.          */
/* N  The number of this receive.            */
/*                         */
/*----------------------------------------------------------------------*/

Receive (fildes, packet, length, timeout)
   int      fildes;
   char     *packet;
   unsigned int   length;
   int      timeout;
{
   int      result;
   struct pollfd  pollblk;
   struct read_extension   ext;

   bzero( &ext, sizeof(ext));


   /*
    * If opened with NO DELAY then do a poll else
    * just do the read ... it will delay if the queue is empty.
    */
   if (block)
   {
      /*
       * Timeout is in milliseconds ... set the alarm
       * for number of seconds requested
       */
      alarm (timeout/100);
      result = READX( fildes, packet, length, &ext );
      alarm (0);
   }
   else
   {
      pollblk.fd = fildes;
      pollblk.reqevents = POLLIN;
      pollblk.rtnevents = 0;
      if (poll( &pollblk, 1, timeout ) <= 0)
      {
         return (-1);
      }
      result = READX( fildes, packet, length, &ext );
   }
   if (result < 0)
   {
      if ((errno == EINTR) && block)
      {
         /* no error */
         fprintf(STDERR,"Interrupted read ...\n");
      }
      else
      {
         decode_error (errno);
      }
   }
   else
   {
      if (result == 0)
         fprintf (STDERR,"\tZero length read!\n");
      ++recv_frames;

   }
   return(result);
}

/*-------------------  A T T A C H _ P R O T O C O L  ------------------*/
/*                         */
/*  NAME: Attach_Protocol                 */
/*                         */
/*  FUNCTION:                       */
/* Issues a start to the driver under test with the netid      */
/* associated with the Client/Server protocol and waits for */
/* the return status block.               */
/*                         */
/*  EXECUTION ENVIRONMENT:                */
/* Can only be executed at the user process level.       */
/*                         */
/*  DATA STRUCTURES:                   */
/* Modifies the global profile block (Ethernet only).    */
/*                            */
/*  RETURNS:                              */
/* FALSE An error or timeout occurred.          */
/* TRUE  The attachment succeeded.           */
/*                         */
/*----------------------------------------------------------------------*/


Attach_Protocol (fildes, protocol)
   int   fildes;
{
   struct session_blk   session;
   int         rc;

   while (TRUE)
   {
      session.netid = protocol;
      session.status = 0;
      rc = IOCTL( fildes, CIO_START, &session );
      if (rc < 0)
      {
         if (session.status == CIO_NETID_DUP)
         {
            protocol += 2;
            continue;
         }
         protocol = rc;
      }
      break;
   }
   return (protocol);
}

Detach_Protocol (fildes, protocol)
   int   fildes;
   short protocol;
{
   struct session_blk   session;
   int   result;

   session.netid = protocol;
   session.status = 0;
   result = IOCTL( fildes, CIO_HALT, &session );
   return (result);
}
#include <sys/cfgodm.h>
#include <sys/mdio.h>
int
read_pos_test()
{
   int      i;
   int      fd;
   int      rc;
   MACH_DD_IO  mdio;
   unsigned char  posbuf[8];
   char     srchstr[256];  /* generic search buffer */
   struct   CuDv  cust_obj;   /* customized attribute struct */
   struct   Class *CDhdl;     /* Customized Devices OC handle */
   int      slot;

   printf("\n\t*****************  POS REGISTERS  ********************");

   /* Get customized object */
   sprintf(srchstr,"name = '%s'",drvpath);
   if((rc =(int)odm_get_obj(CDhdl,srchstr,&cust_obj,ODM_FIRST))==0)
   {
      fprintf (STDERR,"failed odm_get_obj\n");
      return FALSE;
   }
   if (rc == -1)
   {
      fprintf (STDERR,"failed odm_get_obj\n");
      return FALSE;
   }
   strcpy(srchstr,cust_obj.connwhere);
   slot = atoi(&srchstr[strcspn(srchstr,"0123456789")]);
   slot--;

   fd = open ("/dev/bus0", O_RDONLY);
   if (fd < 0)
   {
      fprintf (STDERR, "failed opening \"/dev/bus0\"\n");
      decode_error(errno);
      return (FALSE);
   }
   /*
    * setup parameters:
    *
    *    - get address for pos register 0
    *    - read 7 POS registers
    *    - a byte at a time
    *    - into local posbuf
    */
   mdio.md_addr = POSREG(0, slot);
   mdio.md_size = 7;
   mdio.md_incr = MV_BYTE;
   mdio.md_data = posbuf;
   bzero (posbuf, 7);

   rc = IOCTL (fd, MIOCCGET, &mdio);
   if (rc < 1)
   {
      fprintf(STDERR,"failed ioctl to machdd\n");
      decode_error(errno);
      return (FALSE);
   }
   printf ("POS Registers:\n");
   for (i = 0; i<7; i++)
   {
      printf ("\tpos%d = 0x%x\n", i, posbuf[i]);
   }
   printf("\n\t******************************************************");

   return (0);
}

int
download_test (fildes)
   int      fildes;
{
   int      result;
   fddi_dwnld_t   cmd;
   int      mcode_len,  /* length of microcode file */
         mcode_fd,   /* microcode file descriptor */
         rc;
   char     *mcode;     /* pointer to microcode image */
   static char *mcode_filename = "/etc/microcode/8ef4m.00.01";

   printf("\n\t*******************  DOWNLOAD  ************************");

   if ((mcode_fd = open(mcode_filename,O_RDONLY)) == -1)
   {
      decode_error(errno);
   }
   else if ((mcode_len = lseek(mcode_fd,0L,2)) == -1)
   {
      decode_error(errno);
      close(mcode_fd);
   }
   else if (lseek(mcode_fd,0L,0) == -1)
   {
      decode_error(errno);
      close(mcode_fd);
   }
   else if ((mcode = malloc(mcode_len)) == NULL)
   {
      decode_error(errno);
      close(mcode_fd);
   }
   else if (read(mcode_fd, mcode, mcode_len)== -1)
   {
      decode_error(errno);
      close(mcode_fd);
   }
   else
   {
      close(mcode_fd);

      /*
       * set values in microcode command structure
       */
      cmd.p_mcode = mcode;    /* addr of microcode */
      cmd.l_mcode = mcode_len;   /* microcode length */

      result = ioctl(fildes, FDDI_DWNLD_MCODE, &cmd);
      if (result < 0)
      {
         /* decode status field in the cmd */
         decode_error(errno);
         fprintf(STDERR,
            "\n\tStatus code: %s\n",convcode(cmd.status));
         rc = FALSE;
      }
      else
      {
         fprintf(stdout,
            "\n\tStatus code: %s\n",convcode(cmd.status));
      }
   }
   printf("\n\t********************************************************");
   return(rc);
}


fddi_mem_acc (fildes)
   int   fildes;
{
   int   result,  rc = TRUE;
   fddi_mem_acc_t mem;
   int   done, i, j;
   char  space, dir;
   ushort   opcode;
   uint  offset;
   uchar *p_buf[3];
   uchar *p_data;
   uint  l_buf[3]={0,0,0};
   FILE  *fpout;
   ushort   out_word;
   uchar out_word1, out_word2;
   int   pc;

   printf("\n\t************* Access Memory and DMA Components *********");
   done = FALSE;
   while ( !done )
   {
      printf("\n\tHardware component to Access.\n");
      printf("\n\t\t(1) MEM_FDDI_RAM ");
      printf("\n\t\t(2) MEM_NP_BUS_DATA ");
      printf("\n\t\t(3) MEM_NP_BUS_PROGRAM ");
      printf("\n\t\t(4) SHARED_RAM ");

      space = prompt("\n\tWhich Component?", "%d", "1");

      switch(space)
      {
         case 1:  /* MEM_FDDI_RAM */
         case 2:  /* MEM_NP_BUS_DATA */
         case 3:  /* MEM_NP_BUS_PROGRAM */
         case 4:  /* SHARED_RAM */
            done = TRUE;
            break;
         default:
            break;      /* invalid input */
      }
   }

   done = FALSE;
   /*
    * Detmine if read or write operation
    */
   while ( !done )
   {
      dir = prompt("\tRead (R) or Write (W)?", "%c", "R");

      switch(dir)
      {
         case 'R':
         case 'r':
            done = TRUE;
            break;
         case 'W':
         case 'w':
            done = TRUE;
            break;
         default:
            break;
      }
   } /* end while to determine read or write */

   done = FALSE;
   while ( !done )
   {
      offset = prompt("\tInput the RAM offset(hex)", "%x", "0x0");
      if (offset >= 0)
         done = TRUE;

   } /* end while to get the offset */


   /*
    * get the transfer locations and the sizes
    */
   done = FALSE;
   while ( !done )
   {
      mem.num_transfer = prompt("\tNumber of transfers (hex)",
                  "%x", "0x1");

      if ( (mem.num_transfer > 0) && (mem.num_transfer <= 3) )
      {
         for ( i=0; i < mem.num_transfer; ++i)
         {
            while ( !done )
            {
               printf("\tInput len for transfer %d ",
                  i);
               l_buf[i] = prompt(" ", "%x", "0x10");

               if ( l_buf[i] > 0 )
                  done = TRUE;
            }
            done = FALSE;
         } /* end for loop */
         done = TRUE;
      }
   } /* end while for xfer locations */

   /*
    * get the required memory to do the transfer
    */

   for ( i=0; i < mem.num_transfer; ++i )
   {
      p_buf[i] = malloc(l_buf[i]);
      if (p_buf[i] == NULL)
      {
         fprintf(STDERR, "\n\tMalloc Operation Failed.");

         /*
          * free the memory already malloced
          */
         for ( j=0; j < i; ++j )
            free(p_buf[j]);

         return(ENOMEM);
      }
   }

   /* !!! handle the case when it is a write operation.
    * what type of data is to be written...patern, incrementing
    * random?
    */

   /* build the command opcode */

   switch(space)
   {
      case 1:  /* MEM_FDDI_RAM */
         if ( (dir == 'W') || (dir == 'w') )
            opcode = FDDI_WR_MEM_FDDI_RAM;
         else
            opcode = FDDI_RD_MEM_FDDI_RAM;
         break;
      case 2:  /* MEM_NP_BUS_DATA */
         if ( (dir == 'W') || (dir == 'w') )
            opcode = FDDI_WR_MEM_NP_BUS_DATA;
         else
            opcode = FDDI_RD_MEM_NP_BUS_DATA;
         break;
      case 3:  /* MEM_NP_BUS_PROGRAM */
         if ( (dir == 'W') || (dir == 'w') )
            opcode = FDDI_WR_MEM_NP_BUS_PROGRAM;
         else
            opcode = FDDI_RD_MEM_NP_BUS_PROGRAM;
         break;
      default:    /* SHARED_RAM */
         if ( (dir == 'W') || (dir == 'w') )
            opcode = FDDI_WR_SHARED_RAM;
         else
            opcode = FDDI_RD_SHARED_RAM;
         break;
   } /* end switch space */

   /*
    * now we fill in the fddi_mem_acc_t structure
    */

   mem.status = 0;
   mem.opcode = opcode;
   mem.ram_offset = offset;
   mem.buffer_1 = p_buf[0];
   mem.buffer_2 = p_buf[1];
   mem.buffer_3 = p_buf[2];
   mem.buff_len1 = l_buf[0];
   mem.buff_len2 = l_buf[1];
   mem.buff_len3 = l_buf[2];

   result = IOCTL( fildes, FDDI_MEM_ACC, &mem );
   if (result < 0)
   {
      /* decode status field in the cmd */
      decode_error(errno);
      fprintf (STDERR,"\n\tStatus code: %s\n",convcode(mem.status));
      rc = FALSE;
   }
   else
   {
      fprintf (stdout,"\n\tStatus code: %s\n",convcode(mem.status));

      /*
       * ioctl was successful if operation was a read, write out the
       *    data that was read to /tmp
       */
      if ( (dir == 'R') || (dir == 'r') )
      {
         fpout = fopen("/tmp/fddimemacc1", "w");

         pc = (int) mem.ram_offset;
         p_data = p_buf[0];
         for (i = 0; i < l_buf[0]; i+=2, p_data+=2)
         {
            out_word = (unsigned short)
               (((unsigned short) *p_data)) |
               (((unsigned short) *(p_data + 1)) << 8);
            out_word1 = *p_data;
            out_word2 = *(p_data + 1);
            if (!(i % 8))
               fprintf(fpout,"\n%04x:  ",pc);
            fprintf(fpout,"%04x (", out_word);
            if ((out_word1 > 0x1f) && (out_word1 < 0x7f))
               fprintf(fpout,"%c", out_word1);
            else
               fprintf(fpout,".");
            if ((out_word2 > 0x1f) && (out_word2 < 0x7f))
               fprintf(fpout,"%c)  ", out_word2);
            else
               fprintf(fpout,".)  ");
            pc++;
         }
            fclose(fpout);

         fpout = fopen("/tmp/fddimemacc2", "w");
         p_data = p_buf[1];
         for ( i=0; i< l_buf[0]; i+=2, p_data += 2)
         {
            fprintf(fpout, "%c", *p_data);
         }

         fclose(fpout);


         fpout = fopen("/tmp/fddimemacc3", "w");
         p_data = p_buf[2];
         for ( i=0; i< l_buf[0]; i+=2, p_data += 2)
         {
            fprintf(fpout, "%c", *p_data);
         }

         fclose(fpout);

         printf("\n\tWrote data to files(s) in /tmp \n");

      } /* end if read operation */
   }
   printf("\n\t********************************************************");
   return(rc);

} /* end fddi_mem_acc test */

int
get_fastwrite_parms(
   WRITE_EXT   *p_ext)
{
   int   result;

   result = prompt("\tFastwrite (F) or normal write (N)","%c","N");
   if ((result == 'F') || (result == 'f'))
   {
      /*
       * always call from process thread
       * FDDI type
       * issue max of FDDI_MAX_PCHAIN requests
       */
      p_ext->fastwrt = TRUE;
      p_ext->frames_req = profile.default_frames_req;
   }
   else
   {
      /*
       * Don't employ fastwrite
       */
      p_ext->fastwrt = FALSE;
   }
   return(1);
}

#define MAX_FILE_SIZE   1000000

static   FILE  *cfp = NULL;
static  int cfp_cnt = 0;
int
trace_client (int len, int seq)
{
   if (cfp_cnt++ > MAX_FILE_SIZE)
   {
      /*
       * start back at the beginning
       */
      fseek (cfp, 0, 0);
   }
   if (cfp == NULL)
   {
      cfp = fopen ("/tmp/client", "w");
      if (cfp == NULL)
      {
         return (0);
      }
   }
   fprintf (cfp, "%6x %6x\n", len, seq);
   return (0);
}
static   FILE  *sfp = NULL;
static  int sfp_cnt = 0;
int
trace_server (int len, int seq)
{
   if (sfp_cnt++ > MAX_FILE_SIZE)
   {
      /*
       * start back at the beginning
       */
      fseek (sfp, 0, 0);
   }
   if (sfp == NULL)
   {
      sfp = fopen ("/tmp/server", "w");
      if (sfp == NULL)
      {
         return (0);
      }
   }
   fprintf (sfp, "%6x %6x\n", len, seq);
   return (0);
}
/*
 * Dump send acks
 */
static   FILE  *sackfp = NULL;
static  int sack_cnt = 0;
int
trace_sack (int seq)
{
   if (sack_cnt++ > MAX_FILE_SIZE)
   {
      /*
       * start back at the beginning
       */
      fseek (sackfp, 0, 0);
   }
   if (sackfp == NULL)
   {
      sackfp = fopen ("/tmp/sack", "w");
      if (sackfp == NULL)
      {
         return (0);
      }
   }
   fprintf (sackfp, "%6x\n", seq);
   return (0);
}
/*
 * Dump receive acks
 */
static   FILE  *rackfp = NULL;
static  int rack_cnt = 0;
int
trace_rack (int seq)
{
   if (rack_cnt++ > MAX_FILE_SIZE)
   {
      /*
       * start back at the beginning
       */
      fseek (rackfp, 0, 0);
   }
   if (rackfp == NULL)
   {
      rackfp = fopen ("/tmp/rack", "w");
      if (rackfp == NULL)
      {
         return (0);
      }
   }
   fprintf (rackfp, "%6x\n", seq);
   return (0);
}

close_dumps()
{
   rack_cnt = 0;
   sack_cnt = 0;
   sfp_cnt = 0;
   cfp_cnt = 0;
   fclose (cfp);
   fclose (sfp);
   fclose (rackfp);
   fclose (sackfp);
}

/*
 * Sequences set and determined here:
 * The data sizes are given below. Add to this the TP_HDR_SIZE and
 * this is the size of frame that will be transmitted.
 */

static int previous = 0;
static int current =0;

int
getlength(
   int   seq,
   int   big,
   int   small)
{
   int   length;

   switch (seq)
   {
      case 1:  /* big, small, small, big */
      {
         if ((previous == small) && (current == small))
         {
            length = big;
         }
         else
         {
            length = small;
         }
         break;
      }
      case 2:  /* big, small, big ... */
      {
         length =  (current == big) ? small : big;
         break;
      }
      case 3:  /* all small */
      {
         length = small;
         break;
      }
      case 4:  /* all big */
      {
         length = big;
         break;
      }
      case 5:  /* random */
      default:
      {
         length = random();
         length &= 0x7FFFFFFF;
         length %= (big);
         if (length < small)
            length = small;
         break;
      }
   }
   previous = current;
   current = length;
   return (length);
}
#ifdef __DEAD_CODE__
read_state (fildes)
   int   fildes;
{
   int   i;
   int   result;
   ushort   states[5];

   printf("\n\t******************** READ STATE CMD *******************\n");
   result = IOCTL (fildes, FDDI_READ_STATE, states);
   if (result < 0)
   {
      fprintf(STDERR, "Status: READ STATE command failed\n");
   }
   else
   {
      i = (states[0] > 8) ? 8 : states[0];
      printf("\tECM State Machine:     [%d] =   %s\n",
         states[0],
         ecm[i]);

      i = (states[1] > 10) ? 10 : states[1];
      printf("\tPCM State Machine (Port A):  [%d] =   %s\n",
         states[1],
         pcm[i]);

      i = (states[2] > 10) ? 10 : states[2];
      printf("\tPCM State Machine (Port B):     [%d] =   %s\n",
         states[2],
         pcm[i]);

      i = (states[3] > 6) ? 6 : states[3];
      printf("\tCFM State Machine:     [%d] =   %s\n",
         states[3],
         cfm[i]);

      i = (states[4] > 8) ? 8 : states[4];
      printf("\tRMT State Machine:        [%d] =   %s\n",
         states[4],
         rmt[i]);
   }
   printf("\t******************** ************** *******************\n");
   return (0);
}
rcv_cmd(int fildes)
{
int   result;

   result = IOCTL(fildes, FDDI_ISSUE_RCV_CMD, 1);
   if (result < 0)
   {
      decode_error(errno);
   }
   else
   {
      printf("\n\tReceive command successfully issued\n");
   }
   return;
}
#endif /* __DEAD_CODE__ */


#ifdef __XTRA_CODE__
/*                         */
/*  NAME: fddi_promisc_on                 */
/*                         */
/*  FUNCTION:                       */
/* Issues an ioctl FDDI_PROMISCUOUS_ON to the driver under test   */
/* (via the file descriptor "fildes") and decodes returned error  */
/* codes,   if any.                    */
/*                         */
/*  EXECUTION ENVIRONMENT:                */
/* Can only be executed at the user process level.       */
/*                         */
/*  DATA STRUCTURES:                   */
/*                            */
/*  RETURNS:                              */
/* TRUE if the start succeeded.              */
/* FALSE if the start failed.             */
/*                         */

fddi_promisc_on (fildes)
   int   fildes;
{
   int   result, rc = TRUE;

   printf("\n\t ******************  PROMISCUSOUS ON  *******************");
   result = IOCTL( fildes, FDDI_PROMISCUOUS_ON, NULL);
   if (result < 0) {       /* if error */
       fprintf(stderr, "\n\t ");
       decode_error(errno);
       rc = FALSE;
   } else {
       printf("\n\t -- IOCTL FDDI_PROMISCUOUS_ON succeeded --");
   }
   printf("\n\t ********************************************************");
   return(rc);
}

/*                         */
/*  NAME: fddi_promisc_off                */
/*                         */
/*  FUNCTION:                       */
/* Issues an ioctl FDDI_PROMISCUOUS_OFF to the driver under test  */
/* (via the file descriptor "fildes") and decodes returned error  */
/* codes,   if any.                    */
/*                         */
/*  EXECUTION ENVIRONMENT:                */
/* Can only be executed at the user process level.       */
/*                         */
/*  DATA STRUCTURES:                   */
/*                            */
/*  RETURNS:                              */
/* TRUE if the start succeeded.              */
/* FALSE if the start failed.             */
/*                         */

fddi_promisc_off (fildes)
   int   fildes;
{
   int   result, rc = TRUE;

   printf("\n\t ******************  PROMISCUSOUS OFF  ******************");
   result = IOCTL( fildes, FDDI_PROMISCUOUS_OFF, NULL);
   if (result < 0) {       /* if error */
       fprintf(stderr, "\n\t ");
       decode_error(errno);
       rc = FALSE;
   } else {
       printf("\n\t -- IOCTL FDDI_PROMISCUOUS_OFF succeeded --");
   }
   printf("\n\t ********************************************************");
   return(rc);
}

/*                         */
/*  NAME: fddi_bad_frame_on                  */
/*                         */
/*  FUNCTION:                       */
/* Issues an ioctl FDDI_BAD_FRAME_ON to the driver under test  */
/* (via the file descriptor "fildes") and decodes returned error  */
/* codes,   if any.                    */
/*                         */
/*  EXECUTION ENVIRONMENT:                */
/* Can only be executed at the user process level.       */
/*                         */
/*  DATA STRUCTURES:                   */
/*                            */
/*  RETURNS:                              */
/* TRUE if the start succeeded.              */
/* FALSE if the start failed.             */
/*                         */

fddi_bad_frame_on (fildes)
   int   fildes;
{
   int   result, rc = TRUE;

   printf("\n\t ******************  BAD FRAME ON ***********************");
   result = IOCTL( fildes, FDDI_BAD_FRAME_ON, NULL);
   if (result < 0) {       /* if error */
       fprintf(stderr, "\n\t ");
       decode_error(errno);
       rc = FALSE;
   } else {
       printf("\n\t -- IOCTL FDDI_BAD_FRAME_ON succeeded --");
   }
   printf("\n\t ********************************************************");
   return(rc);
}

/*                         */
/*  NAME: fddi_bad_frame_off                 */
/*                         */
/*  FUNCTION:                       */
/* Issues an ioctl FDDI_BAD_FRAME_OFF to the driver under test    */
/* (via the file descriptor "fildes") and decodes returned error  */
/* codes,   if any.                    */
/*                         */
/*  EXECUTION ENVIRONMENT:                */
/* Can only be executed at the user process level.       */
/*                         */
/*  DATA STRUCTURES:                   */
/*                            */
/*  RETURNS:                              */
/* TRUE if the start succeeded.              */
/* FALSE if the start failed.             */
/*                         */

fddi_bad_frame_off (fildes)
   int   fildes;
{
   int   result, rc = TRUE;

   printf("\n\t ******************  BAD FRAME OFF **********************");
   result = IOCTL( fildes, FDDI_BAD_FRAME_OFF, NULL);
   if (result < 0) {       /* if error */
       fprintf(stderr, "\n\t ");
       decode_error(errno);
       rc = FALSE;
   } else {
       printf("\n\t -- IOCTL FDDI_BAD_FRAME_OFF succeeded --");
   }
   printf("\n\t ********************************************************");
   return(rc);
}

/*                         */
/*  NAME: fddi_bad_pkt_read                  */
/*                         */
/*  FUNCTION:                       */
/* Issues an ioctl KID_BAD_PKT_READ ioctl to the kid  */
/* (via the file descriptor "fildes") and decodes returned error  */
/* codes,   if any.                    */
/*                         */
/*  EXECUTION ENVIRONMENT:                */
/* Can only be executed at the user process level.       */
/*                         */
/*  DATA STRUCTURES:                   */
/*                            */
/*  RETURNS:                              */
/* TRUE if the succeeded.                 */
/* FALSE if the failed.                */
/*                         */

fddi_bad_pkt_read (fildes)
   int   fildes;
{
   fddikid_bad_read_t   badkif;
   int         show;
   int         result, rc, count, i;
   int      length, errors;

   result = 0;
   rc = 0;
   count = 0;


   badkif.outlen = 0;
   badkif.pktlen = 0;
   badkif.data = &readbuf;
   badkif.len = sizeof(readbuf);

   printf("\n\t ******************  READ BAD PKT ***********************");

   bzero(&readbuf, sizeof(readbuf));
   count = prompt("\n\t Read how many frames", "%d",
      profile.default_reads);
   if (count <= 0) return(FALSE);

   sprintf(str, "%d", atoi(profile.default_size) + RECV_PAD);
   length = prompt("\t Read how many bytes per frame", "%d", str);
   if (length < 0) return(FALSE);

   badkif.len = length;

   show = y_or_n("\t Display each frame", TRUE);
   if (show < 0) return(FALSE);

   for (i = 1; i <= count; i++)
   {
      result = IOCTL( fildes, KID_BAD_PKT_READ, &badkif);
      if (result < 0)
      {        /* if error */
          fprintf(stderr, "\n\t ");
          decode_error(errno);
          rc = FALSE;
      }
      else
      {
         rc = TRUE;
         if ( badkif.outlen == 0 )
         {
             fprintf( stderr, "\n\t No data on read %d.", i );
                  break;
         }

      }
      if ( show )
      {
          printf("\n\t Frame %d: ", i);
          print_buffer(&readbuf, badkif.outlen, 0, stdout);
      }
       if (!rc)
      break;
   }
   printf("\n\t ********************************************************");
   return(rc);
}

/*                         */
/*  NAME: fddi_beacon_on                  */
/*                         */
/*  FUNCTION:                       */
/* Issues an ioctl FDDI_BEACON_ON to the driver under test  */
/* (via the file descriptor "fildes") and decodes returned error  */
/* codes,   if any.                    */
/*                         */
/*  EXECUTION ENVIRONMENT:                */
/* Can only be executed at the user process level.       */
/*                         */
/*  DATA STRUCTURES:                   */
/*                            */
/*  RETURNS:                              */
/* TRUE if the start succeeded.              */
/* FALSE if the start failed.             */
/*                         */

fddi_beacon_on (fildes)
   int   fildes;
{
   int   result, rc = TRUE;

   printf("\n\t ******************  BEACON ON ***********************");
   result = IOCTL( fildes, FDDI_BEACON_ON, NULL);
   if (result < 0) {       /* if error */
       fprintf(stderr, "\n\t ");
       decode_error(errno);
       rc = FALSE;
   } else {
       printf("\n\t -- IOCTL FDDI_BEACON_ON succeeded --");
   }
   printf("\n\t ********************************************************");
   return(rc);
}

/*                         */
/*  NAME: fddi_beacon_off                 */
/*                         */
/*  FUNCTION:                       */
/* Issues an ioctl FDDI_BEACON_OFF to the driver under test    */
/* (via the file descriptor "fildes") and decodes returned error  */
/* codes,   if any.                    */
/*                         */
/*  EXECUTION ENVIRONMENT:                */
/* Can only be executed at the user process level.       */
/*                         */
/*  DATA STRUCTURES:                   */
/*                            */
/*  RETURNS:                              */
/* TRUE if the start succeeded.              */
/* FALSE if the start failed.             */
/*                         */

fddi_beacon_off (fildes)
   int   fildes;
{
   int   result, rc = TRUE;

   printf("\n\t ******************  BEACON OFF **********************");
   result = IOCTL( fildes, FDDI_BEACON_OFF, NULL);
   if (result < 0) {       /* if error */
       fprintf(stderr, "\n\t ");
       decode_error(errno);
       rc = FALSE;
   } else {
       printf("\n\t -- IOCTL FDDI_BEACON_OFF succeeded --");
   }
   printf("\n\t ********************************************************");
   return(rc);
}

/*                         */
/*  NAME: fddi_smt_on                     */
/*                         */
/*  FUNCTION:                       */
/* Issues an ioctl FDDI_SMT_ON to the driver under test     */
/* (via the file descriptor "fildes") and decodes returned error  */
/* codes,   if any.                    */
/*                         */
/*  EXECUTION ENVIRONMENT:                */
/* Can only be executed at the user process level.       */
/*                         */
/*  DATA STRUCTURES:                   */
/*                            */
/*  RETURNS:                              */
/* TRUE if the start succeeded.              */
/* FALSE if the start failed.             */
/*                         */

fddi_smt_on (fildes)
   int   fildes;
{
   int   result, rc = TRUE;

   printf("\n\t ******************  SMT ON ***********************");
   result = IOCTL( fildes, FDDI_SMT_ON, NULL);
   if (result < 0) {       /* if error */
       fprintf(stderr, "\n\t ");
       decode_error(errno);
       rc = FALSE;
   } else {
       printf("\n\t -- IOCTL FDDI_SMT_ON succeeded --");
   }
   printf("\n\t ********************************************************");
   return(rc);
}

/*                         */
/*  NAME: fddi_smt_off                    */
/*                         */
/*  FUNCTION:                       */
/* Issues an ioctl FDDI_SMT_OFF to the driver under test       */
/* (via the file descriptor "fildes") and decodes returned error  */
/* codes,   if any.                    */
/*                         */
/*  EXECUTION ENVIRONMENT:                */
/* Can only be executed at the user process level.       */
/*                         */
/*  DATA STRUCTURES:                   */
/*                            */
/*  RETURNS:                              */
/* TRUE if the start succeeded.              */
/* FALSE if the start failed.             */
/*                         */

fddi_smt_off (fildes)
   int   fildes;
{
   int   result, rc = TRUE;

   printf("\n\t ******************  SMT OFF **********************");
   result = IOCTL( fildes, FDDI_SMT_OFF, NULL);
   if (result < 0) {       /* if error */
       fprintf(stderr, "\n\t ");
       decode_error(errno);
       rc = FALSE;
   } else {
       printf("\n\t -- IOCTL FDDI_SMT_OFF succeeded --");
   }
   printf("\n\t ********************************************************");
   return(rc);
}

/*                         */
/*  NAME: fddi_nsa_on                     */
/*                         */
/*  FUNCTION:                       */
/* Issues an ioctl FDDI_NSA_ON to the driver under test     */
/* (via the file descriptor "fildes") and decodes returned error  */
/* codes,   if any.                    */
/*                         */
/*  EXECUTION ENVIRONMENT:                */
/* Can only be executed at the user process level.       */
/*                         */
/*  DATA STRUCTURES:                   */
/*                            */
/*  RETURNS:                              */
/* TRUE if the start succeeded.              */
/* FALSE if the start failed.             */
/*                         */

fddi_nsa_on (fildes)
   int   fildes;
{
   int   result, rc = TRUE;

   printf("\n\t ******************  NSA ON ***********************");
   result = IOCTL( fildes, FDDI_NSA_ON, NULL);
   if (result < 0) {       /* if error */
       fprintf(stderr, "\n\t ");
       decode_error(errno);
       rc = FALSE;
   } else {
       printf("\n\t -- IOCTL FDDI_NSA_ON succeeded --");
   }
   printf("\n\t ********************************************************");
   return(rc);
}

/*                         */
/*  NAME: fddi_nsa_on                     */
/*                         */
/*  FUNCTION:                       */
/* Issues an ioctl FDDI_NSA_OFF to the driver under test       */
/* (via the file descriptor "fildes") and decodes returned error  */
/* codes,   if any.                    */
/*                         */
/*  EXECUTION ENVIRONMENT:                */
/* Can only be executed at the user process level.       */
/*                         */
/*  DATA STRUCTURES:                   */
/*                            */
/*  RETURNS:                              */
/* TRUE if the start succeeded.              */
/* FALSE if the start failed.             */
/*                         */

fddi_nsa_off (fildes)
   int   fildes;
{
   int   result, rc = TRUE;

   printf("\n\t ******************  NSA OFF **********************");
   result = IOCTL( fildes, FDDI_NSA_OFF, NULL);
   if (result < 0) {       /* if error */
       fprintf(stderr, "\n\t ");
       decode_error(errno);
       rc = FALSE;
   } else {
       printf("\n\t -- IOCTL FDDI_NSA_OFF succeeded --");
   }
   printf("\n\t ********************************************************");
   return(rc);
}

#endif /* __XTRA_CODE__ */
