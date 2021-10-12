static char sccsid[] = "@(#)91 1.1 src/bos/usr/fvcioem/ddt/tok/ddt_tok.c, fvcioem, bos411, 9428A410j 4/26/94 13:54:04";

/*--------------------------------------------------------------------------
*
*             DDT_TOK.C
*
*  COMPONENT_NAME:  Communications Device Driver Tool TOK code.
*
*  FUNCTIONS:  main, start_test, halt_test, query_vpd_test, write_test,
*     read_test, statistics_test, edit_defaults, dsopen,
*     dsclose, decode_status, Get_Hw_Addr, access_pos_test,
*     pos_read_test, pos_write_test, tokinfo_test,
*     set_init_op_test, set_open_op_test, group_address,
*     functional_address, ring_info, Client, Arp_Request,
*     Wait_Ack, Send_Halt, Server, Send_Ack, Transmit,
*     Receive, Attach_Protocol, Detach_Protocol, dump_test,
*     dump_write_test, dump_read_test
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
# include <fcntl.h>
# include <stdio.h>
# include <stddef.h>
# include <sys/errno.h>
# include <sys/types.h>
# include <sys/ioctl.h>
# include <sys/param.h>
# include <sys/devinfo.h>
# include <sys/poll.h>
# include <sys/comio.h>
# include <sys/uio.h>
# include <sys/ciouser.h>
# include <sys/dump.h>
# include <sys/device.h>
/* # include "tokuser.h" */
#include <sys/tokuser.h>

#ifdef __XTRA_CODE__
#include <sys/tokenring_mibs.h>
#endif /* __XTRA_CODE__ */

/*# include "kiduser.h"*/
#include <etkiduser.h>
# include "ddt_com.h"
# include "ddt_tok.h"
# include "ddt_csp.h"

#define tok

/*----------------------------------------------------------------------*/
/*          Global Variables           */
/*----------------------------------------------------------------------*/

/*  Shared from there:                    */

extern      errno;
extern int  block;         /* block mode flag */
extern int  kern;       /* kernel mode flag */
extern int  pattern;    /* type of test pattern */
extern int  started;    /* number of starts */
extern int  writlen;    /* length of write data */
extern PROFILE profile;    /* test param defaults */
extern char str[30];    /* scratch string area */
extern char path[30];
extern char unit;       /* port or device number */
extern DRVR_OPEN  dopen;      /* open block for kid driver */
extern int  openstate;     /* open flag */
extern int  autoresponse;     /* autoresponse flag */
extern unsigned int  recv_frames;   /* total # recv frames */
extern unsigned int  xmit_frames;   /* total # xmit frames */
extern char kidpath[];     /* kernel interface drvr path */
extern int  dumpinit;      /* dump init flag */
extern int  dumpstart;     /* dump start flag */

/*----------------------------------------------------------------------*/
/*  Shared from here:                     */

unsigned char  Server_Addr[ HLEN ]; /* hardware address of Server */
unsigned char  Client_Addr[ HLEN ]; /* hardware address of Client */


ROUTING_INFO                 csRoute;

char     Cfile[] = CFPATH; /* saved client frames */
char     Sfile[] = SFPATH; /* saved server frames */
DUMP_EXT dump_info;     /* Dump command structure */

/*----------------------------------------------------------------------*/
/*  Local:                       */

static MAC_FRAME  macbuf;     /* MAC buffer */
static XMIT_FRAME writbuf; /* write buffer */
static RECV_FRAME readbuf; /* read buffer */
static char    wrkbuf[ DATASIZE + 2 ]; /* work buffer */

#include "memdump.h"

static char                  pktlog[256] = "";

/*---------------------------------------------------------------------------*/
/*                  Routine to dump a packet to a log file                   */
/*---------------------------------------------------------------------------*/

void
   dumpPacket(
      char                 * title       ,
      char                 * fname       ,
      void                 * packet      ,
      int                    length
   )
{
   char                      temp[4096];
   char                    * p = temp;
   FILE                    * f;

   if (!(fname && *fname && p && length)) return;

   if ((f = fopen(fname,"a")) == NULL)
   {
      printf("Could not open packet dump file\n");
      return;
   }

   p += sprintf(p,"\n%s\nPacket Length=%d\n",title,length);

   memDump(p,0,16,dumpRel,packet,(256 < length ? 256 : length));

   fprintf(f,temp);

   fclose(f);
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

#include <ddtlog.h>

int
   main(
      int                    argc        ,
      char                 * argv[]
   )
{
   int   fildes, mcaddr = 0, gaddr = 0, faddr = 0;
   char  test = 0, invalid = FALSE, badtry = FALSE;
   char  suspend = 0, trace = 0;

   initLog(argc,argv);

   if (argc > 2 && argv[2][0])
   {
      strcpy(pktlog,argv[2]);
      truncate(pktlog,0);
   }


   autoresponse = 0;
   openstate = FALSE;
   started = 0;
   fildes = -1;
   dsopen();         /* do any initializations */
   writlen = DEFAULT_SIZE;
   while (TRUE) {
      printf("\n\n\t\t     Token Ring Device Driver Tool");
      if (openstate)
         if (kern)
            printf("\n\t\t  %s (kernel mode)", path);
         else
            printf("\n\t\t  %s (user mode)", path);
      else
         printf("\n");
      printf("\n   Control Panel:");
      if (!openstate && !started)
         printf("\n\t O)   Open Device    S)   Start Device");
      if (!openstate && started)
         printf("\n\t O)   Open Device    S) * Start Device");
      if (openstate && !started)
         printf("\n\t O) * Open Device    S)   Start Device");
      if (openstate && started)
         printf("\n\t O) * Open Device    S) * Start Device");
      printf("      L)   List Statistics");
      printf("\n\t C)   Close Device   H)   Halt Device       ");

      printf("V)   Query VP Data");
      printf("\n\t G)   Get Status     I)   Set Init Options  ");
      printf("B)   Ring Info" );
      printf("\n\t P)   Pos Registers  N)   Open Options      ");
      printf("E)   Edit Defaults");

      if (!gaddr && !faddr)
         printf("\n\t A)   Group Address  F)   Functional Addr");
      if (!gaddr && faddr)
         printf("\n\t A)   Group Address  F) * Functional Addr");
      if (gaddr && !faddr)
         printf("\n\t A) * Group Address  F)   Functional Addr");
      if (gaddr && faddr)
         printf("\n\t A) * Group Address  F) * Functional Addr");
      printf("   T)   Get TOKINFO");
      printf("\n\t W)   Write          R)   Read              ");
      printf("D)   Dump Commands");
      printf("\n\t Z) Read Freeze Dump M)   Write MAC frames  ");
      printf("X) Cause Freeze Dump");
#ifdef __XTRA_CODE__
      printf("\n\t J)   MIB Query      K)   MIB Get  ");
#endif /* __XTRA_CODE__ */

      printf("\n");
      printf("\n   Client/Server Tests:");
      printf("\n\t 1)   Server Side    2)   Client Side       ");
      printf("\n");
      if (invalid)
         printf("\n\t\t   \"%c\" is not a valid choice! \n", test);
      else if (badtry)
         printf("\n\t\t   You have not opened the driver! \n");
      printf("\n\t Choose from the menu or enter Q to quit: ");
                  /* read char, throw out NL */
      if ((test = getinput()) != '\n') getinput();
      invalid = badtry = FALSE;  /* innocent til proven guilty */
      switch (test) {         /* select test routine */
         case 'o':      /* Open Device test */
         case 'O':
            if (openstate) {
               printf("\n\t\t The device is already open!");
            } else {
               fildes = open_test(DEVPATH);
               if (fildes >= 0) openstate = TRUE;
            }
         break;
         case 'c':      /* Close Device test */
         case 'C':
            if (!openstate) badtry = TRUE;
            else if (close_test(fildes)) {
               started = 0;
               mcaddr  = 0;
               faddr   = 0;
               gaddr   = 0;
               suspend = 0;
               trace   = 0;
               autoresponse = 0;
               openstate = FALSE;
               fildes = -1;
            }
         break;
         case 'd':
         case 'D':
            if (!openstate) badtry = TRUE;
            else dump_test(fildes);
            break;
         case 's':      /* Start Device test */
         case 'S':
            if (!openstate) badtry = TRUE;
            else if (start_test(fildes)) started++;
            break;
         case 'h':      /* Halt Device test */
         case 'H':
            if (!openstate) badtry = TRUE;
            else if (halt_test(fildes)) started--;
            break;
         case 'g':      /* Get Status test */
         case 'G':
            if (!openstate) badtry = TRUE;
            else status_test(fildes);
            break;
         case 'l':      /* Query Statistics Test */
         case 'L':
            if (!openstate) badtry = TRUE;
            else statistics_test(fildes);
            break;
         case 'e':      /* Edit Defaults */
         case 'E':
            edit_defaults(fildes);
            break;
         case 'v':      /* Query Vital Product Data */
         case 'V':
            if (!openstate) badtry = TRUE;
            else query_vpd_test(fildes);
            break;
         case 'p':      /* Access POS test */
         case 'P':
            if (!openstate) badtry = TRUE;
            else access_pos_test(fildes);
            break;
         case 'i':      /* Set Init Options */
         case 'I':
            if (!openstate) badtry = TRUE;
            else set_init_op_test(fildes);
            break;
         case 'n':      /* Set Open Options */
         case 'N':
            if (!openstate) badtry = TRUE;
            else set_open_op_test(fildes);
            break;
         case 't':      /* Tokinfo test */
         case 'T':
            if (!openstate) badtry = TRUE;
            else tokinfo_test(fildes);
            break;
         case 'a':      /* Add/Delete Group Address */
         case 'A':
            if (!openstate) badtry = TRUE;
            else gaddr += group_address(fildes);
            break;
         case 'f':      /* Add/Delete Funct Address */
         case 'F':
            if (!openstate) badtry = TRUE;
            else faddr += functional_address(fildes);
            break;
         case 'b':               /* Query Ring Info */
         case 'B':
            if (!openstate) badtry = TRUE;
            else ring_info(fildes);
            break;
         case 'w':      /* Write Test */
         case 'W':
            if (!openstate) badtry = TRUE;
            else write_test(fildes);
            break;
         case 'r':      /* Read Test */
         case 'R':
            if (!openstate) badtry = TRUE;
            else read_test(fildes);
            break;
         case 'z':   /* Read an adapter dump */
         case 'Z':
            read_adap_dump(fildes);
            break;
         case 'x':   /* Cause an adapter dump */
         case 'X':
            cause_adap_dump(fildes);
            break;
         case 'm':      /* MAC Write Test */
         case 'M':
            if (!openstate) badtry = TRUE;
            else write_mac(fildes);
            break;
#ifdef __XTRA_CODE__
         case 'j':
         case 'J':
            if (!openstate) badtry = TRUE;
            else tok_mib_query(fildes);
            break;
         case 'k':
         case 'K':
            if (!openstate) badtry = TRUE;
            else tok_mib_get(fildes);
            break;
#endif /* __XTRA_CODE__ */
         case '1':
            if (!openstate) badtry = TRUE;
            Server(fildes);
            break;
         case '2':
            if (!openstate) badtry = TRUE;
            Client(fildes);
            break;
         case '\n':     /* Refresh screen */
            break;
         default:
            invalid = TRUE;   /* Invalid character */
            break;
      }
      if (started < 0) started = 0;
      if (mcaddr  < 0) mcaddr  = 0;
      if (gaddr   < 0) gaddr   = 0;
      if (faddr   < 0) faddr   = 0;
      if (suspend < 0) suspend = 0;
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

   printf("\n\t ***********************  START  ************************");
   result = prompt("\n\t Use what netid (hex)", "%x",
            profile.default_netid);
   if (result < 0) return (FALSE);
   session.netid = result;
   if (session.length < 0) return (FALSE);
   session.status = 0;
   result = IOCTL( fildes, CIO_START, &session );
   if (result < 0) {       /* if error */
       switch (errno) {
      case EIO:
             fprintf(stderr, "\n\t ERROR status returned -- ");
          show_session_blk( &session, stderr );
          break;
      default:
             fprintf(stderr, "\n\t ");
          decode_error(errno);
          break;
       }
       rc = FALSE;
   } else {
       if (rc)
      if (y_or_n("\t Wait for status", TRUE))
          rc = wait_status(fildes, CIO_START_DONE, STATUS_TIMEOUT);
       if (rc) {
           printf("\n\t -- IOCTL Start succeeded --");
      show_session_blk( &session, stdout );
       }
   }
   printf("\n\t ********************************************************");
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

   printf("\n\t **********************   HALT   ************************");
   result = prompt("\n\t Use what netid (hex)", "%x",
            profile.default_netid);
   if (result < 0) return(FALSE);
   session.netid = result;
   if (session.length < 0) return(FALSE);
   session.status = 0;
   result = IOCTL( fildes, CIO_HALT, &session );
   if (result < 0) {       /* if error */
      switch (errno) {
         case EIO:
            fprintf(stderr, "\n\t ERROR status returned -- ");
            show_session_blk( &session, stderr );
            break;
         default:
            fprintf(stderr, "\n\t ");
            decode_error(errno);
            break;
      }
      rc = FALSE;
   } else {
      if (rc) {
         printf("\n\t -- IOCTL Halt succeeded --");
         show_session_blk( &session, stdout );
      }
   }
   printf("\n\t ********************************************************");
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

query_vpd_test (fildes)
   int   fildes;
{
   int   result;
   VPDATA   vpd;

   printf("\n\t *************** QUERY VITAL PRODUCT DATA  **************");
   bzero(&vpd, sizeof(vpd));
   result = IOCTL( fildes, GET_VPD, &vpd );
   if (result < 0) {       /* if error */
      fprintf(stderr, "\n\t ");
      decode_error(errno);
   } else {
      printf("\n\t IOCTL Query VPD succeeded:");
      switch (vpd.status) {
         case NOT_READ_VPD:
            printf(" VPD_NOT_READ returned.");
            break;
         case INVALID_VPD:
            printf(" VPD_INVALID returned.");
            break;
         case VALID_VPD:
            printf(" VPD_VALID returned.");
            decode_vpd(vpd.vpd);
            break;
         default:
            printf(" Undefined return code %d.",vpd.status);
            break;
      }
   }
   printf("\n\t ********************************************************");
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
   WRITE_EXT   ext;
   int   result, rc, count, i, waitstat = FALSE;
   int   statusrate, done = FALSE;

   bzero(&ext, sizeof(ext));
   rc = TRUE;
   printf("\n\t **********************   WRITE   ***********************");
   printf("\n");
   result = prompt("\n\tFastwrite (F) or normal write (N)","%c","N");
   if ((result == 'F') || (result == 'f')) {
      if (!kern) {
         printf("\n\tDRIVER NOT IN KERNEL MODE\n");
         return;
      }
      ext.fastwrt_type = NORMAL_CHAIN;
      ext.fastwrt_opt.offlevel = FALSE;
   } else
      ext.fastwrt_type = 0;

   while (!done) {
      printf("\t Destination Address (6 bytes hex) [%s]: ",
            profile.default_dest);
      i = 0;
      while ((str[i++] = getinput()) != '\n');
      if (str[0] == '.') return(FALSE);
      str[i-1] = '\0';
      if (str[0] == '\0') {
         hexstring_to_array(profile.default_dest, writbuf.dest);
         break;
      } else
         done = (strlen(str) == 12) &&
         hexstring_to_array(str, writbuf.dest);
   }
   result = prompt("\t What is the destination netid (hex)",
               "%x", profile.default_netid);
   if ( result < 0 ) return(FALSE);
   sprintf( profile.default_netid, "%x", result );
   writbuf.netid = result;
   Get_Hw_Addr(fildes, writbuf.src);
   result = prompt("\t What is the access priority (hex)",
               "%x", profile.default_ac);
   if ( result < 0 ) return(FALSE);
   writbuf.ac = result;
   if (writbuf.netid == TOK_MAC_FRAME_NETID) /* this doesn't work */
      writbuf.fc = MAC_TYPE;
   else
      writbuf.fc = LLC_TYPE;
   writlen = MAX( 0, writlen - DATALINK_HDR );
   pattern = fill_buffer(writbuf.data, &writlen, BUFSIZE );
   if (pattern < 0) return(FALSE);
   writlen += DATALINK_HDR;
   count =
      prompt("\t Issue how many writes", "%d", profile.default_writes);
   if (count < 0) return(FALSE);
   sprintf( profile.default_writes, "%d", count );
   if (!ext.fastwrt_type)
      statusrate = prompt("\t Wait for transmit status per how many writes",
                     "%d", "1");
   else
      statusrate = 0;
   if (statusrate < 0) return(FALSE);
   for (i = 1; i <= count; i++) {
      ext.ciowe.flag = 0;        /* clear flags */
      if (!profile.free_mbuf)    /* don't free mbuf? */
         ext.ciowe.flag |= CIO_NOFREE_MBUF;
      ext.ciowe.status   = 0;    /* clear status */
      ext.ciowe.write_id = i;    /* index of transmit */
      if (statusrate)      /* time for status wait? */
         if (waitstat = !(i % statusrate))
            ext.ciowe.flag |= CIO_ACK_TX_DONE;
      result = WRITE( fildes, &writbuf, writlen, &ext );
      if (result < 0) {    /* if error */
         fprintf(stderr, "\n\t Write %d ", i);
         switch (errno) {
            case EIO:
               fprintf(stderr, "ERROR: See Status block for reason");
               break;
            default:
               decode_error(errno);
               break;
         }
         rc = FALSE;    /* failed */
         break;
      }
      if (waitstat) {
         if (!wait_status(fildes, CIO_TX_DONE, STATUS_TIMEOUT)) {
            fprintf(stderr, " (Write %d) ", i);
            rc = FALSE;    /* timed out or error */
            break;
         }
      }
   }
   if (rc)
      printf("\n\t -- Write succeeded. --");
   else {
      fprintf(stderr, "\n\t Write Parameters:");
      if (kern)
         fprintf(stderr, "\n\t    Device %d, KERNEL mode", unit);
      else
         fprintf(stderr, "\n\t    Device %d, USER mode", unit);
      if (block)
         fprintf(stderr, "\n\t    NDELAY off (blocking)");
      else
         fprintf(stderr, "\n\t    NDELAY on (nonblocking)");
      if (profile.free_mbuf)
         fprintf(stderr, "\n\t    Mbufs freed by driver");
      else
         fprintf(stderr, "\n\t    Mbufs not freed by driver");
      if (waitstat)
         fprintf(stderr, "\n\t    Wait for TX status");
      else
         fprintf(stderr, "\n\t    Don't wait for TX status");
   }
   printf("\n\t ********************************************************");
   return(rc);
}

/*-----------------------   W R I T E _ M A C  -------------------------*/
/*                         */
/*  NAME: write_mac                       */
/*                         */
/*  FUNCTION:                       */
/* Writes a test frame to the open device associated with fildes. */
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

write_mac (fildes)
   int   fildes;
{
   WRITE_EXT   ext;
   int   result, rc, count, i, waitstat = FALSE;
   int   statusrate, done = FALSE;

   bzero(&ext, sizeof(ext));
   rc = TRUE;
   printf("\n\t ******************** MAC WRITE   ***********************");
   printf("\n");
   ext.fastwrt_type = 0;

   while (!done) {
      printf("\t Destination Address (6 bytes hex) [%s]: ",
            profile.default_dest);
      i = 0;
      while ((str[i++] = getinput()) != '\n');
      if (str[0] == '.') return(FALSE);
      str[i-1] = '\0';
      if (str[0] == '\0') {
         hexstring_to_array(profile.default_dest, macbuf.dest);
         break;
      } else
         done = (strlen(str) == 12) &&
         hexstring_to_array(str, macbuf.dest);
   }
   Get_Hw_Addr(fildes, macbuf.src);
   macbuf.ac = 0;
   macbuf.fc = MAC_TYPE;
   writlen = MAX( 0, writlen - MAC_HDR );
   pattern = fill_buffer(macbuf.data, &writlen, BUFSIZE );
   if (pattern < 0) return(FALSE);
   writlen += MAC_HDR;
   count =
      prompt("\t Issue how many writes", "%d", profile.default_writes);
   if (count < 0) return(FALSE);
   sprintf( profile.default_writes, "%d", count );
   if (!ext.fastwrt_type)
      statusrate = prompt("\t Wait for transmit status per how many writes",
                     "%d", "1");
   else
      statusrate = 0;
   if (statusrate < 0) return(FALSE);
   for (i = 1; i <= count; i++) {
      ext.ciowe.flag = 0;        /* clear flags */
      if (!profile.free_mbuf)    /* don't free mbuf? */
         ext.ciowe.flag |= CIO_NOFREE_MBUF;
      ext.ciowe.status   = 0;    /* clear status */
      ext.ciowe.write_id = i;    /* index of transmit */
      if (statusrate)      /* time for status wait? */
         if (waitstat = !(i % statusrate))
            ext.ciowe.flag |= CIO_ACK_TX_DONE;
      result = WRITE( fildes, &macbuf, writlen, &ext );
      if (result < 0) {    /* if error */
         fprintf(stderr, "\n\t Write %d ", i);
         switch (errno) {
            case EIO:
               fprintf(stderr, "ERROR: See Status block for reason");
               break;
            default:
               decode_error(errno);
               break;
         }
         rc = FALSE;    /* failed */
         break;
      }
      if (waitstat) {
         if (!wait_status(fildes, CIO_TX_DONE, STATUS_TIMEOUT)) {
            fprintf(stderr, " (Write %d) ", i);
            rc = FALSE;    /* timed out or error */
            break;
         }
      }
   }
   if (rc)
      printf("\n\t -- Write succeeded. --");
   else {
      fprintf(stderr, "\n\t Write Parameters:");
      if (kern)
         fprintf(stderr, "\n\t    Device %d, KERNEL mode", unit);
      else
         fprintf(stderr, "\n\t    Device %d, USER mode", unit);
      if (block)
         fprintf(stderr, "\n\t    NDELAY off (blocking)");
      else
         fprintf(stderr, "\n\t    NDELAY on (nonblocking)");
      if (profile.free_mbuf)
         fprintf(stderr, "\n\t    Mbufs freed by driver");
      else
         fprintf(stderr, "\n\t    Mbufs not freed by driver");
      if (waitstat)
         fprintf(stderr, "\n\t    Wait for TX status");
      else
         fprintf(stderr, "\n\t    Don't wait for TX status");
   }
   printf("\n\t ********************************************************");
   return(rc);
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
   int      verify = TRUE, show;
   int      result, rc, count, i, j;
   int      length, errors;
   struct read_extension   ext;
   struct pollfd  pollblk;


   printf("\n\t ************************  READ  ************************");
   bzero(&readbuf, sizeof(readbuf));
   count = prompt("\n\t Read how many frames", "%d",
      profile.default_reads);
   if (count <= 0) return(FALSE);
   sprintf(str, "%d", atoi(profile.default_size) + RECV_PAD);
   length = prompt("\t Read how many bytes per frame", "%d", str);
   if (length < 0) return(FALSE);
   show = y_or_n("\t Display each frame", TRUE);
   if (show < 0) return(FALSE);
   for (i = 1; i <= count; i++) {
       /*                     */
       /*  Wait for receive data by polling for it:      */
          /*                        */
       if ( profile.poll_reads ) {
           pollblk.fd = fildes;
           pollblk.reqevents = POLLIN;
           pollblk.rtnevents = 0;
           if (poll( &pollblk, 1, READ_TIMEOUT ) <= 0)
           {
          rc = TRUE;
          printf("\n\t No data available on read %d.", i);
          break;
           }
       }
       result = READX( fildes, &readbuf, length, &ext );
       if (result < 0)        /* if error */
       {             /* show it */
         fprintf(stderr, "\n\t Read %d ", i);
      decode_error(errno);
         rc = FALSE;       /* return failed */
       } else {
      rc = TRUE;
      if ( !result )
      {
          fprintf( stderr, "\n\t No data on read %d.", i );
          break;
      }
      if ( verify )
      {
          errors = 0;
          for (j = 0; j < (result - DATALINK_HDR); j++)
          {
         if ( readbuf.data[j] != writbuf.data[j] )
         {
             rc = FALSE;
             fprintf(stderr, "\n\t Error: byte %d is 0x%.2x,",
            j, readbuf.data[j]);
             fprintf(stderr, " should be 0x%.2x!",
            writbuf.data[j]);
             if (++errors == ERROR_LIMIT)
            break;
         }
          }
      }
      if ( show )
      {
          printf("\n\t Frame %d: ", i);
          print_buffer(&readbuf, result, 0, stdout);
      }
       }
       if (!rc)
      break;
   }
   if (rc)
       printf("\n\t -- Read passed. --");
   else {
       fprintf(stderr, "\n\t Read Parameters:");
       if (kern)
      fprintf(stderr, "\n\t    Device %d, KERNEL mode", unit );
       else
      fprintf(stderr, "\n\t    Device %d, USER mode", unit );
       if (block)
      fprintf(stderr, "\n\t    NDELAY off (blocking)");
       else
      fprintf(stderr, "\n\t    NDELAY on (nonblocking)");
   }
   printf("\n\t ********************************************************");
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

   printf("\n\t ******************* QUERY STATISTICS *******************");
   bzero( &parms, sizeof(parms));
   bzero( &stats, sizeof(stats));
   parms.bufptr = (caddr_t)&stats;
   parms.buflen = sizeof(stats);
   result = IOCTL( fildes, CIO_QUERY, &parms );
   if (result < 0)   {        /* if error */
       fprintf(stderr, "\n\t ");
       decode_error(errno);
   } else {
       printf("\n\t IOCTL Query Statistics succeeded:");
       printf("\n\t   Transmit Byte mcnt   = %u", stats.cc.tx_byte_mcnt);
       printf("\n\t   Transmit Byte Count  = %u", stats.cc.tx_byte_lcnt);
       printf("\n\t   Receive Byte mcnt    = %u", stats.cc.rx_byte_mcnt);
       printf("\n\t   Receive Byte Count   = %u", stats.cc.rx_byte_lcnt);
       printf("\n\t   Transmit Frame mcnt  = %u", stats.cc.tx_frame_mcnt);
       printf("\n\t   Transmit Frame Count = %u", stats.cc.tx_frame_lcnt);
       printf("\n\t   Receive Frame mcnt   = %u", stats.cc.rx_frame_mcnt);
       printf("\n\t   Receive Frame Count  = %u", stats.cc.rx_frame_lcnt);
       printf("\n\t   Transmit Error Count = %d", stats.cc.tx_err_cnt);
       printf("\n\t   Receive Error Count  = %d", stats.cc.rx_err_cnt);
       printf("\n\t   Max Netid's in use   = %d", stats.cc.nid_tbl_high);
       printf("\n\t   Max Transmits queued = %d", stats.cc.xmt_que_high);
       printf("\n\t   Max Receives queued  = %d", stats.cc.rec_que_high);
       printf("\n\t   Max Stat Blks queued = %d", stats.cc.sta_que_high);
       printf("\n\t   Interrupts lost      = %d", stats.ds.intr_lost);
       printf("\n\t   WDT Interrupts lost  = %d", stats.ds.wdt_lost);
       printf("\n\t   Timeout Ints lost    = %d", stats.ds.timo_lost);
       printf("\n\t   Status lost          = %d",
            stats.ds.sta_que_overflow);
       printf("\n\t   Receive Packets Lost = %d",
            stats.ds.rec_que_overflow);
       printf("\n\t   No Mbufs Errors      = %d",
            stats.ds.rec_no_mbuf);
       printf("\n\t   No Mbuf Ext Errors   = %d",
            stats.ds.rec_no_mbuf_ext);
       printf("\n\t   Receive Int Count    = %d",
            stats.ds.recv_intr_cnt);
       printf("\n\t   Transmit Int Count   = %d",
            stats.ds.xmit_intr_cnt);
   }
   printf("\n");
   if (!(result < 0) && y_or_n("\t Clear statistics?", TRUE)) {
       parms.bufptr = (caddr_t)&stats;
       parms.buflen = sizeof(stats);
       parms.clearall = CIO_QUERY_CLEAR;
       result = IOCTL( fildes, CIO_QUERY, &parms );
       if (result < 0) {
           fprintf(stderr, "\n\t ");
           decode_error(errno);
       } else
      printf("\n\t Statistics cleared.");
   }
   printf("\n\t ********************************************************");
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

   printf("\n\t ********************  EDIT DEFAULTS  *******************");
   i = prompt("\n\t Mbuf size threshold", "%d", profile.mbuf_thresh);
   if (i < 0) return;
   sprintf(profile.mbuf_thresh, "%d", i);
   if (openstate && kern)
       IOCTL( fildes, KID_MBUF_THRESH, atoi(profile.mbuf_thresh));
   i = prompt("\t Mbuf write data transfer offset", "%d",
                  profile.mbuf_wdto);
   if (i < 0) return;
   sprintf(profile.mbuf_wdto, "%d", i);
   if (openstate && kern)
       IOCTL( fildes, KID_MBUF_WDTO,   atoi(profile.mbuf_wdto));
   i = y_or_n("\t Free mbufs in driver after write",
                  profile.free_mbuf);
   if (i < 0) return;
   profile.free_mbuf = i;
   i = y_or_n("\t Have mbuf data areas cross page boundary",
                  profile.funky_mbuf);
   if (i < 0) return;
   profile.funky_mbuf = i;
   if (openstate && kern)
       IOCTL( fildes, KID_FUNKY_MBUF,  profile.funky_mbuf );

   while (!done) {
       printf("\t Default destination address [%s]: ",
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
       printf("\t Default source address [%s]: ",
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
   i = prompt("\t Default netid (hex)", "%x", profile.default_netid);
   if (i < 0) return;
   sprintf(profile.default_netid, "%x", i);
   i = prompt("\t Default AC (hex)", "%x", profile.default_ac);
   if (i < 0) return;
   sprintf(profile.default_ac, "%x", i);
   i = prompt("\t Default data size", "%d", profile.default_size);
   if (i < 0) return;
   sprintf(profile.default_size, "%d", i);
   i = prompt("\t Default number of writes", "%d", profile.default_writes);
   if (i < 0) return;
   sprintf(profile.default_writes, "%d", i);
   i = prompt("\t Default number of reads", "%d", profile.default_reads);
   if (i < 0) return;
   sprintf(profile.default_reads, "%d", i);
   i = y_or_n("\t Poll for data before issuing read",
                  profile.poll_reads);
   if (i < 0) return;
   profile.poll_reads = i;
   printf("\t ********************************************************");
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
   profile.free_mbuf = TRUE;
   profile.poll_reads = TRUE;
   profile.funky_mbuf = FALSE;
   sprintf( profile.mbuf_thresh, "%d", CLBYTES );
   sprintf( profile.mbuf_wdto, "%d", 0 );
   strcpy ( profile.default_dest, DEFAULT_DEST );
   sprintf( profile.default_netid, "%x", DEFAULT_NETID );
   sprintf( profile.default_ac, "%x", DEFAULT_AC );
   sprintf( profile.default_size, "%d", DEFAULT_SIZE );
   sprintf( profile.default_writes, "%d", DEFAULT_WRITES );
   sprintf( profile.default_reads, "%d", DEFAULT_READS );
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

decode_status (status)
   struct status_block  *status;
{
      switch (status->code)
   {
       case CIO_LOST_STATUS:  fprintf(stderr, "Lost Status");
                  break;
          case CIO_NULL_BLK:          printf("Null Block");
               break;
          case CIO_START_DONE:   printf("Start Done");
               break;
       case CIO_HALT_DONE:         printf("Halt Done");
               break;
       case CIO_TX_DONE:           printf("Transmit Done");
               break;
       case CIO_ASYNC_STATUS: printf("Unsolicited Status");
               break;
       case TOK_ADAP_CHECK:        fprintf(stderr, "Adapter Check");
               break;
       case TOK_RING_STATUS:  fprintf(stderr, "Ring Status");
               break;
       default:              printf("Undefined status code %d",
                  status->code);
               break;
   }
   if (status->code != CIO_NULL_BLK)
   {
       if ( status->option[0] != CIO_OK )
       {
           fprintf(stderr, "\n\t Status Block:");
           fprintf(stderr, "\n\t   Option[0] field = ");
       } else {
           printf("\n\t Status Block:");
           printf("\n\t   Option[0] field = ");
       }
       switch (status->option[0])
       {
      case CIO_OK:      printf("CIO_OK");
                    break;
      case CIO_HARD_FAIL:  fprintf(stderr, "Hardware Failure");
               break;
      case CIO_BAD_MICROCODE: fprintf(stderr, "Bad Microcode");
               break;
      case TOK_LOBE_MEDIA_TST_FAIL:
               fprintf(stderr,
                   "Lobe Media Test Failed");
               break;
      case TOK_PHYS_INSERT:   fprintf(stderr, "Physical Insertion");
               break;
      case TOK_ADDR_VERIFY_FAIL:
               fprintf(stderr,
                     "Address Verify Failed");
               break;
      case TOK_RING_POLL:  fprintf(stderr, "Ring Poll");
               break;
      case TOK_REQ_PARMS:  fprintf(stderr, "Request Parameters");
               break;
      case TOK_ADAP_INIT_FAIL:
               fprintf(stderr,
                  "Adapter Initialization Failed");
               break;
      case TOK_LOBE_WIRE_FAULT:
               fprintf(stderr, "Lobe Wire Fault");
               break;
      case TOK_AUTO_REMOVE:   fprintf(stderr, "Auto Remove");
               break;
      case TOK_REMOVED_RECEIVED:
               fprintf(stderr, "Removed Received");
               break;
      default:    printf("Undefined: %x",
                  status->option[0]);
               break;
       }
       if ( status->option[0] == CIO_OK )
       {
           printf("\n\t   Option[1] field = %.8x", status->option[1]);
           printf("\n\t   Option[2] field = %.8x", status->option[2]);
           printf("\n\t   Option[3] field = %.8x", status->option[3]);
       } else {
           fprintf(stderr, "\n\t   Option[1] field = %.8x",
               status->option[1]);
           fprintf(stderr, "\n\t   Option[2] field = %.8x",
               status->option[2]);
           fprintf(stderr, "\n\t   Option[3] field = %.8x",
               status->option[3]);
       }
   }
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
   int   result, i, j, done = 0;
   char  str[80];
   VPDATA   vpd;

   bzero( address, 6 );
   bzero(&vpd, sizeof(vpd));

   result = IOCTL( fildes, GET_VPD, &vpd );
   if (result < 0) {          /* if error */
       return(FALSE);         /* return */
   } else {
       switch (vpd.status)
       {
      case NOT_READ_VPD:  return(FALSE);
      case INVALID_VPD:   return(FALSE);
      case VALID_VPD:            /* find network address */
          for (i = 7; i < 256; i++)
          {
         if (vpd.vpd[i] == '*')
         {
             if (vpd.vpd[ i + 1 ] == 'N')
             {
            vpd.vpd[ i + 4 + HLEN ] = '\0';
            for (j = 0; j < 6; j++)
            {
                address[j] = vpd.vpd[ i + 4 + j ];
                        sprintf(&profile.default_src[j << 1],
               "%.2x", address[j]);
            }
            return (TRUE);
             }
         }
          }
      default:    return(FALSE);
       }
   }
}

/*-------------------  A C C E S S _ P O S _ T E S T  ------------------*/
/*                         */
/*  NAME: access_pos_test                 */
/*                         */
/*  FUNCTION:                       */
/* Selects pos read or write tests based on user input.     */
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

access_pos_test (fildes)
   int   fildes;
{
   int   mode = 0;

   printf("\n\t **********************  POS ACCESS  ********************");
   while (!mode) {
       mode = prompt("\n\t Read (R) or Write (W)", "%c", "R");
       switch (mode) {
      case 'r':
      case 'R':
          pos_read_test(fildes);
          break;
      case 'w':
      case 'W':
          pos_write_test(fildes);
          break;
      case '.':
          return;
      default:
          mode = 0;
       }
   }
   printf("\n\t ********************************************************");
}

/*--------------------  P O S _ R E A D _ T E S T  ---------------------*/
/*                         */
/*  NAME: pos_read_test                   */
/*                         */
/*  FUNCTION:                       */
/* Issues the POS_ACC POS_READ ioctl to the driver under test  */
/* for each of the valid pos registers and displays the return    */
/* values.  If an error occurs, the error is decoded and       */
/* displayed.                    */
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

pos_read_test (fildes)
   int   fildes;
{
   int         i, result;
   POS         posdata;

   bzero(&posdata, sizeof(posdata));
   posdata.opcode = POS_READ;
   for (i = 0; i <= 7; i++) {    /* for each POS register */
       posdata.pos_reg = i;
       result = IOCTL( fildes, POS_ACC, &posdata );
       if (result < 0) {         /* if error */
         switch (errno) {
          case EIO:
         fprintf(stderr, "\n\t ERROR: ");
               switch (posdata.status) {
             case NOT_DIAG_MODE:
            fprintf(stderr, "NOT_DIAG_MODE");
               break;
             case BAD_RANGE:
            fprintf(stderr, "BAD_RANGE");
            break;
             case TOK_NO_POS:
            fprintf(stderr, "TOK_NO_POS");
            break;
             case INV_CMD:
            fprintf(stderr, "INV_CMD");
            break;
                  default:
            fprintf(stderr, "Undefined status value %d",
                  posdata.status);
                 break;
                 }
              break;
          default:
            fprintf(stderr, "\n\t ");
              decode_error(errno);
            break;
         }
       } else
         printf("\n\t POS %d read succeeded; value = 0x%x",
          i, posdata.pos_val);
   }              /* next pos register */
   printf("\n\t Press ENTER to continue "); getinput();
}

/*--------------------  P O S _ W R I T E _ T E S T  -------------------*/
/*                         */
/*  NAME: pos_write_test                  */
/*                         */
/*  FUNCTION:                       */
/* Issues the POS_ACC POS_WRITE ioctl to the driver under test    */
/* for the prompted for pos register.  If an error occurs, the */
/* error is decoded and displayed.              */
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

pos_write_test (fildes)
   int   fildes;
{
   int         i, result;
   POS         posdata;

   bzero(&posdata, sizeof(posdata));
   posdata.opcode = POS_WRITE;
   i = prompt("\t Write to which POS register", "%d", "2");
   if (i < 0) return;
   posdata.pos_reg = i;
   i = prompt("\t Value to write (hex)", "%x", "0");
   if (i < 0) return;
   posdata.pos_val = i;
        result = IOCTL( fildes, POS_ACC, &posdata );
        if (result < 0) {           /* if error */
       switch (errno) {
      case EIO:
               fprintf(stderr, "\n\t ERROR: ");
             switch (posdata.status) {
              case NOT_DIAG_MODE:
                fprintf(stderr, "NOT_DIAG_MODE");
             break;
         case BAD_RANGE:
             fprintf(stderr, "BAD_RANGE");
             break;
            case TOK_NO_POS:
             fprintf(stderr, "TOK_NO_POS");
             break;
              case INV_CMD:
             fprintf(stderr, "INV_CMD");
             break;
              default:
             fprintf(stderr, "Undefined status value %d",
                  posdata.status);
                    break;
             }
          break;
      default:
             fprintf(stderr, "\n\t ");
          decode_error(errno);
          break;
       }
   } else
       printf("\n\t -- POS write test succeeded --");
}

/*----------------------  T O K I N F O _ T E S T  ---------------------*/
/*                         */
/*  NAME: tokinfo_test                    */
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

tokinfo_test (fildes)
   int   fildes;
{
   int      result, i;
   struct devinfo info;

   printf("\n\t **********************  TOKINFO  ***********************");
   bzero(&info, sizeof(info));
   result = IOCTL( fildes, IOCINFO, &info );
   if (result < 0) {          /* if error */
       fprintf(stderr, "\n\t ");
       decode_error(errno);
   } else {
       printf("\n\t IOCTL IOCINFO succeeded:");
       printf("\n\t    Device Type = ");
       if (info.devtype == DD_NET_DH)
       printf("DD_NET_DH");
       else printf("unknown - %d", info.devtype);

       printf("\n\t    Device Subtype = ");
       if (info.devsubtype == DD_TR)
       printf("DD_TR");
       else printf("unknown - %d", info.devsubtype);

       printf("\n\t    Broadcast Packet Wrap Support = ");
       switch (info.un.token.broad_wrap) {
      case TRUE:  printf("Supported");
               break;
      case FALSE:    printf("Not Supported");
               break;
      default: printf("unknown: %d", info.un.token.broad_wrap);
            break;
       }
       printf("\n\t    Receive data xfer offset = %d", info.un.token.rdto);
       printf("\n\t    Ring Speed = ");
       switch (info.un.token.speed) {
      case TOK_4M:   printf("4 Megabits/second");
               break;
      case TOK_16M:  printf("16 Megabits/second");
               break;
      default: printf("unknown - %d", info.un.token.speed);
            break;
       }
       printf("\n\t    Token Ring Address in use  = 0x");
       for (i = 0; i < 6; i++)
      printf("%.2x", info.un.token.net_addr[i]);
       printf("\n\t    Adapter Token Ring Address = 0x");
       for (i = 0; i < 6; i++)
      printf("%.2x", info.un.token.haddr[i]);
   }
   printf("\n\t ********************************************************");
}

/*------------------  S E T _ I N I T _ O P _ T E S T  -----------------*/
/*                         */
/*  NAME: set_init_op_test                */
/*                         */
/*  FUNCTION:                       */
/* Prompts for initialization options and issues them to the   */
/* driver under test via the TOK_SET_ADAP_IPARMS ioctl.     */
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

set_init_op_test (fildes)
   int   fildes;
{
   int   result, i;
   tok_set_adap_i_parms_t  parms;

   bzero(&parms, sizeof(parms));
   printf("\n\t ********** Set Adapter Initializations Options *********");
   printf("\n\t Initialization Options field (hex): ");
   scanf("%x", &i); getinput();
   parms.init_options = i;
   printf("\t Receive Burst Size: ");
   scanf("%d", &i); getinput();
   parms.rcv_burst_size = i;
   printf("\t Transmit Burst Size: ");
   scanf("%d", &i); getinput();
   parms.xmit_burst_size = i;
   printf("\t DMA Abort Threshold: ");
   scanf("%d", &i); getinput();
   parms.dma_abort_thresh = i;
   result = IOCTL( fildes, TOK_SET_ADAP_IPARMS, &parms );
   if (result < 0) {       /* if error */
       switch (errno) {
      case EIO:
               fprintf(stderr, "\n\t ERROR: ");
             switch (parms.status) {
              case NOT_DIAG_MODE:
                fprintf(stderr, "NOT_DIAG_MODE");
             break;
         case BAD_RANGE:
             fprintf(stderr, "BAD_RANGE");
             break;
              case TOK_NO_PARMS:
             fprintf(stderr, "TOK_NO_PARMS");
             break;
              default:
             fprintf(stderr, "Undefined status value %d",
                  parms.status);
                    break;
             }
          break;
      default:
               fprintf(stderr, "\n\t ");
          decode_error(errno);
          break;
       }
   } else
       printf("\n\t -- Set Init Options succeeded --");
   printf("\n\t ********************************************************");
}

/*------------------  S E T _ O P E N _ O P _ T E S T  -----------------*/
/*                         */
/*  NAME: set_open_op_test                */
/*                         */
/*  FUNCTION:                       */
/* Prompts for adapter open options and issues them to the     */
/* driver under test via the TOK_SET_OPEN_PARMS ioctl.      */
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

set_open_op_test (fildes)
   int   fildes;
{
   int   result, i, done = 0;
   char  str[80];
   tok_set_open_opts_t  parms;

   printf("\n\t ************** Set Adapter Open Options ****************");
   bzero(&parms, sizeof(parms));
   printf("\n\t Open Options field (hex): ");
   scanf("%x", &i); getinput();
   parms.options = (short)i;
   printf("\t Buffer Size: ");
   scanf("%d", &i); getinput();
   parms.buf_size = (short)i;
   printf("\t Transmit Buffer Minimum Count: ");
   scanf("%d", &i); getinput();
   parms.xmit_buf_min_cnt = (char)i;
   printf("\t Transmit Buffer Maximum Count: ");
   scanf("%d", &i); getinput();
   parms.xmit_buf_max_cnt = (char)i;
   while (!done) {
       printf("\t Network Address (6 bytes hex): ");
       scanf("%s", str);
       done = (strlen(str) == 12) &&
           hexstring_to_array(str, &parms.i_addr1);
   }
   getinput();
   result = IOCTL( fildes, TOK_SET_OPEN_PARMS, &parms );
   if (result < 0) {       /* if error */
       switch (errno) {
      case EIO:
             fprintf(stderr, "\n\t ERROR: ");
             switch (parms.status) {
              case NOT_DIAG_MODE:
                fprintf(stderr, "NOT_DIAG_MODE");
             break;
         case BAD_RANGE:
             fprintf(stderr, "BAD_RANGE");
             break;
              case TOK_NO_PARMS:
             fprintf(stderr, "TOK_NO_PARMS");
             break;
              default:
             fprintf(stderr, "Undefined status value %d",
                  parms.status);
                    break;
             }
          break;
      default:
               fprintf(stderr, "\n\t ");
          decode_error(errno);
          break;
       }
   } else
       printf("\n\t -- Set Open Options succeeded --");
   printf("\n\t ********************************************************");
}

/*---------------------   G R O U P _ A D D R E S S   ------------------*/
/*                         */
/*  NAME: group_address                   */
/*                         */
/*  FUNCTION:                       */
/* Issues the TOK_GRP_ADDR ioctl to the driver under test (via */
/* the file descriptor "fildes"); with either TOK_ADD or TOK_DEL  */
/*    (add or delete) depending on user selection.  The group     */
/* address to be added or deleted is prompted for.       */
/*                         */
/*  EXECUTION ENVIRONMENT:                */
/* Can only be executed at the user process level.       */
/*                         */
/*  DATA STRUCTURES:                   */
/* References no global data structures.           */
/*                            */
/*  RETURNS:                        */
/* 1  when a group address is added.            */
/*     -1  when a group address is deleted.           */
/* 0  if an error occurs.                 */
/*                         */
/*----------------------------------------------------------------------*/

group_address (fildes)
   int   fildes;
{
   tok_group_addr_t  gaddr;
   char  c = 0;
   int   result, rc;

   printf("\n\t **************  ADD/DELETE GROUP ADDRESS  **************");
   printf("\n");
   bzero(&gaddr, sizeof(gaddr));
   while (!c) {
       printf("\t Add (A) or Delete (D) group address? ");
       c = getinput(); getinput();
       switch (c) {
      case 'a':
      case 'A':
          c = 'A';
          break;
      case 'd':
      case 'D':
          c = 'D';
          break;
      case '.':
          return;
      default:
          c = 0;
          break;
       }
   }
   printf("\t Enter Group Address (hex): ");
   scanf("%x", &gaddr.group_addr); getinput();
   if (c == 'A')
        gaddr.opcode = TOK_ADD;
   else gaddr.opcode = TOK_DEL;
   result = IOCTL( fildes, TOK_GRP_ADDR, &gaddr );
   if (result < 0) {       /* if error */
       switch (errno) {
      case ENETUNREACH:
          fprintf(stderr, "\n\t ERROR: Device not started.");
          break;
      case ENOMSG:
             fprintf(stderr, "\n\t ERROR: ");
             switch (gaddr.status) {
              case CIO_NOT_STARTED:
                fprintf(stderr, "CIO_NOT_STARTED");
             break;
         case INV_CMD:
             fprintf(stderr, "INV_CMD");
             break;
              case TOK_NO_GROUP:
             fprintf(stderr, "TOK_NO_GROUP");
             break;
         case CIO_NET_RCVRY_ENTER:
             fprintf(stderr, "CIO_NET_RCVRY_ENTER");
             break;
         case CIO_NET_RCVRY_EXIT:
             fprintf(stderr, "CIO_NET_RCVRY_EXIT");
             break;
         case CIO_NET_RCVRY_MODE:
             fprintf(stderr, "CIO_NET_RCVRY_MODE");
             break;
              default:
             fprintf(stderr, "Undefined status value %d",
                  gaddr.status);
                    break;
             }
          break;
      default:
             fprintf(stderr, "\n\t ");
          decode_error(errno);
          break;
       }
       rc = FALSE;
   } else {
       if (c == 'A') {
          printf("\n\t -- Add Group Address succeeded --");
          rc = 1;          /* added an address */
       } else {
          printf("\n\t -- Delete Group Address succeeded --");
          rc = -1;            /* deleted an address */
       }
   }
   printf("\n\t ********************************************************");
   return(rc);
}

/*------------------  F U N C T I O N A L _ A D D R E S S  -------------*/
/*                         */
/*  NAME: functional_address                 */
/*                         */
/*  FUNCTION:                       */
/* Issues the TOK_FUNC_ADDR ioctl to the driver under test (via   */
/* the file descriptor "fildes"); with either TOK_ADD or TOK_DEL  */
/*    (add or delete) depending on user selection.  The functional   */
/* address to be added or deleted is prompted for.       */
/*                         */
/*  EXECUTION ENVIRONMENT:                */
/* Can only be executed at the user process level.       */
/*                         */
/*  DATA STRUCTURES:                   */
/* References the global profile structure for default netid   */
/* information.                     */
/*                            */
/*  RETURNS:                        */
/* 1  when a functional address is added.          */
/*     -1  when a functional address is deleted.         */
/* 0  if an error occurs.                 */
/*                         */
/*----------------------------------------------------------------------*/

functional_address (fildes)
   int   fildes;
{
   tok_func_addr_t      faddr;
   char  c = 0;
   int   i, result, rc;

   printf("\n\t ***********  ADD/DELETE FUNCTIONAL ADDRESS  ************");
   printf("\n");
   bzero(&faddr, sizeof(faddr));
   while (!c) {
       printf("\t Add (A) or Delete (D) functional address? ");
       c = getinput(); getinput();
       switch (c) {
      case 'a':
      case 'A':
          c = 'A';
          break;
      case 'd':
      case 'D':
          c = 'D';
          break;
      case '.':
          return;
      default:
          c = 0;
          break;
       }
   }
   printf("\t Enter Functional Address (hex): ");
   scanf("%x", &faddr.func_addr); getinput();
   if (c == 'A')
        faddr.opcode = TOK_ADD;
   else faddr.opcode = TOK_DEL;
   faddr.netid = prompt("\t What netid (hex)", "%x",
               profile.default_netid);
   result = IOCTL( fildes, TOK_FUNC_ADDR, &faddr );
   if (result < 0) {       /* if error */
       switch (errno) {
      case ENETUNREACH:
          fprintf(stderr, "\n\t ERROR: Device not started.");
          break;
      case ENOMSG:
             fprintf(stderr, "\n\t ERROR: ");
             switch (faddr.status) {
              case CIO_NOT_STARTED:
                fprintf(stderr, "CIO_NOT_STARTED");
             break;
         case INV_CMD:
             fprintf(stderr, "INV_CMD");
             break;
         case CIO_NET_RCVRY_ENTER:
             fprintf(stderr, "CIO_NET_RCVRY_ENTER");
             break;
         case CIO_NET_RCVRY_EXIT:
             fprintf(stderr, "CIO_NET_RCVRY_EXIT");
             break;
         case CIO_NET_RCVRY_MODE:
             fprintf(stderr, "CIO_NET_RCVRY_MODE");
             break;
              default:
             fprintf(stderr, "Undefined status value %d",
                  faddr.status);
                    break;
             }
          break;
      default:
             fprintf(stderr, "\n\t ");
          decode_error(errno);
          break;
       }
       rc = FALSE;
   } else {
       if (c == 'A') {
          printf("\n\t -- Add Functional Address succeeded --");
          rc = 1;          /* added an address */
       } else {
          printf("\n\t -- Delete Functional Address succeeded --");
          rc = -1;            /* deleted an address */
       }
   }
   printf("\n\t ********************************************************");
   return(rc);
}

/*------------------------   R I N G _ I N F O   -----------------------*/
/*                         */
/*  NAME: ring_info                    */
/*                         */
/*  FUNCTION:                       */
/* Issues the TOK_RING_INFO ioctl to the driver under test (via   */
/* the file descriptor "fildes"); then decodes and displays the   */
/* returned ring information.             */
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

ring_info (fildes)
        int     fildes;
{
        int             result, i;
        tok_q_ring_info_t      info;
        tok_ring_info_t        data;

        printf("\n\t **************  TOK_RING_INFO  ****************");
        bzero(&info, sizeof(info));
        bzero(&data, sizeof(data));
        info.p_info = (caddr_t)&data;
        info.l_buf = sizeof(data);

        result = IOCTL( fildes, TOK_RING_INFO, &info );
        if (result < 0) {                       /* if error */
            fprintf(stderr, "\n\t ");
            decode_error(errno);
        } else {
            printf("\n\t IOCTL TOK_RING INFO succeeded:");
            printf("\n\t Adapter Physical address: 0x%.4x%.4x",
                  data.adap_phys_addr[0], data.adap_phys_addr[1]);
            printf("\n\t Upstream Node address: 0x");
            for (i = 0; i < 3; i++)
                printf("%.4x", data.upstream_node_addr[i]);
            printf("\n\t Upstream Physical address: 0x%.4x%.4x",
                        data.upstream_phys_addr[0], data.upstream_phys_addr[1]);
            printf("\n\t Last Poll Address: 0x");
            for (i = 0; i < 3; i++)
                printf("%.4x", data.last_poll_addr[i]);
            printf("\n\t Authorized Environment: 0x%.4x",
                                          data.author_env);
            printf("\n\t Transmit Access Priority: 0x%.4x",
                                          data.tx_access_prior);
            printf("\n\t Source Class Authority: 0x%.4x",
                                          data.src_class_author);
            printf("\n\t Last Attention Code: 0x%.4x",
                                          data.last_atten_code);
            printf("\n\t Last Source Address: 0x");
            for (i = 0; i < 3; i++)
                printf("%.4x", data.last_src_addr[i]);
            printf("\n\t Last Beacon Type: 0x%.4x",
                                          data.last_bcon_type);
            printf("\n\t Last Major Vector: 0x%.4x",
                                          data.last_maj_vector);
            printf("\n\t Ring Status: 0x%.4x",
                                          data.ring_status);
            printf("\n\t Software Error Timer Value: 0x%.4x",
                                          data.sft_err_time_val);
            printf("\n\t Front End Timer Value: 0x%.4x",
                                          data.front_end_time_val);
            printf("\n\t Monitor Error Code: 0x%.4x",
                                          data.monitor_err_code);
            printf("\n\t Beacon Transmit Type: 0x%.4x",
                                          data.bcon_tx_type);
            printf("\n\t Beacon Receive Type: 0x%.4x",
                                          data.bcon_rcv_type);
            printf("\n\t Frame Correlator Save: 0x%.4x",
                                          data.frame_corr_save);
            printf("\n\t Beaconing Station's NAUN: 0x");
            for (i = 0; i < 3; i++)
                printf("%.4x", data.bcon_station_naun[i]);
            printf("\n\t Beaconing Stations's Physical Addr: 0x%.4x%.4x",
               data.bcon_station_phys_addr[0], data.bcon_station_phys_addr[1]);
            printf("\n\t Press ENTER to continue "); getinput();
        }
        printf("\n\t ********************************************************");
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
   int      bad_frame, timeout = CLIENT_TIMEOUT;
   int      result, i, j, count, ack_spacing, length;
   int      burst_size, load, lower_limit, upper_limit;
   int      xmit, pattern_type, random = FALSE;
   int      ack_no = 0, recv_timeout = FALSE;
   int      srecv_frames = -1, sxmit_frames = -1, max_errors = 0;
   int      lost_frames = -1, bad_frames = 0, lost_acks = 0;
   int      sbad_frames = -1, errors = 0;
   int      halt_sent = FALSE, startstop = FALSE;
   int      netid;
   char     c;
   uchar    fastwrt; /*TRUE for fastwrite */

   ttp   = (TP        *)xmitbuf.data;
   rtp   = (TP        *)recvbuf.data;
   tarp  = (TP_ARP    *)ttp->data;
   tparm = (TP_PARMS  *)ttp->data;
   rarp  = (TP_ARP    *)rtp->data;
   rparm = (TP_PARMS  *)rtp->data;
   rres  = (TP_RESULT *)rtp->data;

   xmit_frames = recv_frames = 0;

   printf("\n\t ***********************  CLIENT  ***********************");

   netid = prompt("\n\t Use what netid (hex)", "%x",profile.default_netid);
   if (result < 0) return (FALSE);

   count = prompt("\n\t Send how many test frames", "%d", "1000000");
   if (count < 0) return;
   fastwrt = FALSE;
   if (kern)
   fastwrt = y_or_n("\tSend through fastwrite",FALSE);

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

   ack_spacing = prompt("\t Ack per how many frames", "%d", "1");
   if (ack_spacing < 0) return;
   load = prompt("\t Issue how many additional writes before 1st ack",
         "%d", "0");
   if (load < 0) return;

   /*  The test data length determines the size of the test    */
   /*  frames sent by the Client.  The data portion of the frame  */
   /*  also contains Client/Server header overhead, so the size   */
   /*  of this header must be subtracted to determine the actual  */
   /*  test data length.                  */

   length = 0;
   while (!length) {
       length = prompt("\t How many data bytes per frame (0 = random)",
      "%d", "0");
       if (length < 0) return;
       if (length == 0) {
           printf("\t    Random test data size --");
      random = FALSE;
           while (!random) {
          lower_limit =
         prompt("\n\t       Lower Limit", "%d", MIN_FRAME);
          if (lower_limit < 0) return;
          upper_limit =
         prompt("\t       Upper Limit", "%d",
                  profile.default_size);
          if (upper_limit < 0) return;
          if (upper_limit <= lower_limit)
          {
              fprintf( stderr, "\n\t Say what?" );
            continue;
          }
          if (upper_limit > BUFSIZE)
          {
              fprintf( stderr, "\n\t The upper limit is too big!" );
         continue;
          }
          if (lower_limit <= TEST_HEADER)
          {
              fprintf( stderr, "\n\t The lower limit is too small!" );
         continue;
          }
          random = TRUE;
           }            /* adjust for header overhead */
      lower_limit -= TEST_HEADER;
      upper_limit -= TEST_HEADER;
           length = upper_limit;
      break;
       } else {            /* fixed frame size: */
           if (length <= TEST_HEADER)  /* no room for header? */
      {
          fprintf(stderr,"\t That's too small!\n");
          length = 0;
          continue;
           }
      if (length > BUFSIZE)      /* bigger than buffer? */
      {
          fprintf(stderr,"\t That's too big!\n");
          length = 0;
          continue;
      }
      length -= TEST_HEADER;  /* adjust for header overhead */
       }
   }
   pattern_type = fill_buffer( ttp, &length, BUFSIZE );
   if (pattern_type < 0) return;
   max_errors = prompt("\t Halt after how many errors", "%d", "5");
   if (max_errors < 0) return;
   startstop = y_or_n("\t Issue device halts/starts during test", FALSE);
   if (startstop < 0) return;
   printf("\t Press ENTER to start test . . . "); c = getinput();
   if ( c == '.' )
   {
       printf("\t Test cancelled\n");
       return;
   }
   /*  The receive queues must be cleared to eliminate any  */
   /*  residual frames from previous tests.        */

   printf("\t Clearing receive queues . . .");
   while ((result = Receive(fildes, &recvbuf, sizeof(recvbuf), 100)) > 0);

   /*  A "well-known" netid is registered with the driver; this   */
   /*  same netid is registered by the Server.        */

   printf("\n\t Attaching Test protocol . . .");
   if (!Attach_Protocol(fildes, netid))
       return;

   /*  For LAN client/server tests, an ARP handshake must first   */
   /*  take place to resolve the hardware addresses.     */
   /*  The hardware address of the local adapter is read (via  */
   /*  vital product data) to uniquely identify this client to the   */
   /*  server.                   */

   printf("\n\t Getting hardware address . . . ");
   Get_Hw_Addr(fildes, Client_Addr);
   for (i = 0; i < HLEN; i++)
       printf("%.2x", Client_Addr[i]);

   /*  A broadcast ARP request is issued to the Server in an   */
   /*  attempt to get the Server's hardware address (this is   */
   /*  similar to a TCP/IP ping with the exception of the ICMP */
   /*  echo).  The broadcast is retried several times in case  */
   /*  the Server is started after the Client.        */

   i = 0;
   printf("\n\t Issuing ARP request to the Server . . .\n");
   while (!Arp_Request(fildes,netid))
   {
       if (i++ >= ARP_RETRY)
       {
      fprintf(stderr, "\n\t Could not contact the Server!");
      return;
       }
       printf("\n\t Retrying ARP request . . .");
   }
   for (i = 0; i < HLEN; i++)
   {
       xmitbuf.dest[i] = Server_Addr[i];  /* dest = server */
       xmitbuf.src[i]  = Client_Addr[i];  /* source = my address */
   }

   xmitbuf.netid = netid;              /* netid */

   /*  The ARP handshake completed -- now a test parameters frame */
   /*  is constructed and sent to configure the Server.     */

   xmitbuf.ac = 0;
   xmitbuf.fc = LLC_TYPE;

   strcpy(ttp->src,  CLIENT_PA);    /* My protocol address */
   strcpy(ttp->dest, SERVER_PA);    /* his protocol address */
   ttp->seq_number       = 0;    /* sequence number */
   ttp->operation      = TEST_PARMS;   /* sending test parameters */
   ttp->length         = sizeof(TP_PARMS);   /* length of data */
   tparm->ack_spacing  = ack_spacing;  /* acks per # transmits */
   tparm->pattern_type = pattern_type; /* type of test pattern */
   tparm->max_errors   = max_errors;   /* max error count */

   printf("\n\t Sending Test Parameters to the Server . . .");

   length = route(&xmitbuf,sizeof(TP_PARMS),&csRoute);

   while ( errors < max_errors )
   {
      dumpPacket("--->Test Parameters",pktlog,&xmitbuf,length);

      if (!Transmit(fildes, &xmitbuf, length, FALSE)) /* send packet */
           return;            /* quit if error */

       /*  The test parameters packet is sent.  The Server reponds  */
       /*  by returning an ack packet with the starting ack number. */

       if ((ack_no = Wait_Ack(fildes,netid)) < 0) /* get next ack (timeout?) */
       {
           lost_acks++; errors++;
      if ( errors >= max_errors )
      {
               fprintf(stderr, "\n\t Lost contact with the Server!");
               goto exit;
      }
       } else {
           printf("\n\t The Server acknowledged the test parameters.");
      lost_acks = errors = 0;    /* forgive and forget */
      break;
       }
   }
   /*  Finally, the send/ack loop is started -- the first burst is   */
   /*  augmented by the load size:              */

   printf("\n\n\t --  Test in Progress  --");
   j = 0;
   burst_size = load + ack_spacing; /* first burst */
   bad_frame = FALSE;
   while (TRUE) {          /* TRANSMIT/RECEIVE LOOP: */

       /*  The transmit subloop sends a burst of test frames; the */
       /*  data pattern is regenerated for each frame so that the */
       /*  Client spends as much time creating each frame as the  */
       /*  Server spends verifying each frame.        */

       if ( !halt_sent )
       {
           int               rlength;

           xmit = FALSE;

           if ( startstop )         /* restart device? */
           {
               Detach_Protocol(fildes, netid);
               Attach_Protocol(fildes, netid);
           }

           for (i = j; i < (j + burst_size); i++)
           {
               if (random)
                  length = lower_limit
                  + RANDNUM(upper_limit - lower_limit);

               xmitbuf.netid = netid;           /* netid */
               strcpy(ttp->src,  CLIENT_PA);    /* My protocol address */
               strcpy(ttp->dest, SERVER_PA);    /* his protocol address */
               ttp->operation = TEST_PATTERN;
               ttp->length = length;
               ttp->seq_number = i;

               generate_pattern(ttp->data, length, pattern_type);

               rlength = route(&xmitbuf,length,&csRoute);

               dumpPacket("--->Data",pktlog,&xmitbuf,rlength);

               xmit = Transmit(fildes, &xmitbuf, rlength, fastwrt);
               if (!xmit) return;
           }

           /*  The test frame sequence # is updated by the burst  */
           /*  size, the burst size is then updated to either the */
           /*  remaining # of test frames or the ack spacing,  */
           /*  whichever is smaller.  If the last test frame was  */
           /*  sent, a halt frame is sent to stop the Server.  */

           j += burst_size;

           burst_size = MIN( ack_spacing, count - j );

           if ( !xmit && !halt_sent )
           {
               Send_Halt(fildes, netid, Server_Addr);
              halt_sent = TRUE;
           }
       }

       /*  Await the next frame from the Server.  If a receive */
       /*  timeout occurs, the next burst is sent as a retry;  */
       /*  if the next receive times out, then the Server is   */
       /*  assumed to be dead and the test is aborted.      */

       bzero(&recvbuf, sizeof(recvbuf));
       result = Receive(fildes, &recvbuf, sizeof(recvbuf), CLIENT_TIMEOUT);
       if (result < 0)        /* timeout on read */
       {
           dumpPacket("***Receive Timeout",pktlog,NULL,0);
           ++lost_acks;
           if ((++errors >= max_errors) || recv_timeout)
           {           /* second recv timeout? */
              fprintf(stderr, "\n\t Lost contact with Server!");
              break;
           }
           else
           {       /* else, first timeout */
              recv_timeout = TRUE;   /* so retry */
              continue;        /* next transmit */
           }
       }
       else
           recv_timeout = FALSE;
       /*  For LAN Client/Server tests, acknowledgements are   */
       /*  returned as packets containing a sequence number.   */

       if (!strcmp(rtp->dest, CLIENT_PA))
       {
      /*  If the received frame is an ack, verify that it */
      /*  is in sequence -- if not, update the ack count  */
      /*  to the latest sequence number and increment the   */
      /*  lost ack count by the number lost acks.  If the   */
      /*  number of lost acks brings us to the max number */
      /*  errors, send a halt and abort the test.     */

      dumpPacket("<---Data ACK",pktlog,&recvbuf,result);

      if (rtp->operation == TEST_ACK)
      {              /* ACK Frame: */
          if (rtp->seq_number > ++ack_no)
          {
              /* If last frame was bad and we are only   */
              /* missing one ack, treat as corrupted  */
              /* frame, else treat as missing:     */

         if (((rtp->seq_number - ack_no) == 1) && bad_frame)
             ack_no = rtp->seq_number;
         else {
                     fprintf(stderr, "\n\t ACK out of sequence:");
                  fprintf(stderr, " Expect ack %d, got ack %d",
                   ack_no, rtp->seq_number);
             lost_acks =
                 lost_acks + (rtp->seq_number - ack_no);
             ack_no = rtp->seq_number; /* attempt recovery */
             if (++errors >= max_errors)
             {
                 Send_Halt(fildes, netid, Server_Addr);
            halt_sent = TRUE;
             }
         }
          }
          bad_frame = FALSE;
      }
      /*  If a test results frame has arrived (either    */
      /*  because we requested a halt or because the     */
      /*  Server has reached the maximum error count),   */
      /*  get the statistics from the frame and exit the */
      /*  test.  Include the test frame in the Server's  */
      /*  transmit frame count (add one).       */

      if (rtp->operation == TEST_RESULTS)
      {
          printf("\n\t Test results received.");
          lost_frames  = rres->lost_frames;
          sbad_frames  = rres->bad_frames;
          sxmit_frames = rres->xmit_frames + 1;
          srecv_frames = rres->recv_frames;
          break;        /* quit loop */
      }
       } else {

         fprintf(stderr, "\n\t Corrupted Frame received!");
      save_bad_frame(&recvbuf, result,
         RANDOM_PATTERN, ++bad_frames, TP_HDR_SIZE, Cfile);
      bad_frame = TRUE;
           if (++errors >= max_errors)
      {
               Send_Halt(fildes, netid, Server_Addr);
          halt_sent = TRUE;
      }
       }
   }              /* xmit/recv loop */
exit:
   /*  If any errors were recorded, display them.  If the   */
   /*  Server failed to return a results packet, then */
   /*  we don't know how many test frames were lost or   */
   /*  corrupted, so list these statistics as "unknown." */
   /*                   */
   if (lost_acks || (lost_frames > 0) || bad_frames || (sbad_frames > 0))
   {
       fprintf(stderr, "\n\t Tests completed with errors:");
       fprintf(stderr, "\n\t     Client, device %s:", &path[0]);
       fprintf(stderr, "\n\t     Frames from Server lost:        %d",
            lost_acks);
       fprintf(stderr, "\n\t     Data Frames Lost:               ");
       if (lost_frames < 0)
      fprintf(stderr, "Unknown");
       else
      fprintf(stderr, "%d", lost_frames);
       fprintf(stderr, "\n\t     Bad frames received by Client:  %d",
      bad_frames);
       fprintf(stderr, "\n\t     Bad frames received by Server:  ");
       if (sbad_frames < 0)
      fprintf(stderr, "Unknown");
       else
      fprintf(stderr, "%d", sbad_frames);
   } else
       printf("\n\t Tests completed successfully.");

   printf("\n\t     Frames transmitted by Client:   %d",
       xmit_frames);
   printf("\n\t     Frames received by Server:      ");
   if (srecv_frames < 0)
       printf("Unknown");
   else
       printf("%d", srecv_frames);
   printf("\n\t     Frames transmitted by Server:   ");
   if (sxmit_frames < 0)
       printf("Unknown");
   else
       printf("%d", sxmit_frames);
   printf("\n\t     Frames received by Client:      %d",
       recv_frames);
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

Arp_Request (fildes,netid)
   int      fildes;
   int      netid;
{
   RECV_FRAME  recvbuf;
   XMIT_FRAME  xmitbuf;

   TP                      * ttp    , * rtp;
   TP_PARMS                * tparm  , * rparm;
   TP_ARP                  * tarp   , * rarp;
   int                       result , i, length;

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

   xmitbuf.ac    = 0;
   xmitbuf.fc    = LLC_TYPE;
   xmitbuf.netid = netid;             /* Datalink header: */
   xmitbuf.pad   = 0x00;

                  /* Fill in packet: */

   strcpy(ttp->src,  CLIENT_PA);    /* My protocol address */
   strcpy(ttp->dest, SERVER_PA);    /* his protocol address */

   ttp->operation  = TEST_ARP;      /* type of test packet */
   ttp->length     = sizeof(TP_ARP);   /* size after header */
   ttp->seq_number = 0;       /* sequence number */
   tarp->type      = ARP_REQUEST;      /* request hw address */

   for (i = 0; i < HLEN; i++)       /* fill in hardware addr's */
   {
       tarp->sender[i] = Client_Addr[i];  /* mine */
       xmitbuf.src[i]  = Client_Addr[i];
       tarp->target[i] = 0;           /* his (unknown) */
       xmitbuf.dest[i] = 0xFF;      /* broadcast */
   }              /* send it via write */

   csRoute.rctl    = 0xC260;

   length = route(&xmitbuf,sizeof(TP_ARP),&csRoute);

   dumpPacket("--->ARP Request",pktlog,&xmitbuf,length);

   if (!Transmit( fildes, &xmitbuf, length, FALSE ))
       return(FALSE);         /* quit if error */

   while (TRUE)
   {
       result = Receive(fildes, &recvbuf, sizeof(recvbuf), CLIENT_TIMEOUT);

       if (result < 0) return(FALSE);          /* timeout */

       if (rarp->type == ARP_REPLY)       /* and a reply? */
       {
           dumpPacket("<---ARP Reply",pktlog,&recvbuf,result);

           for (i = 0; i < HLEN; i++)     /* then get */
              Server_Addr[i] = rarp->sender[i];   /* get address. */

           printf("\n\t The Server's address is ");

           for (i = 0; i < HLEN; i++)
              printf("%.2x", Server_Addr[i]);

           memcpy(&csRoute,&recvbuf.route,sizeof(csRoute));
           csRoute.rctl &= 0x1F7F;

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

Wait_Ack (fildes,netid)
   int   fildes;
   int   netid;
{
   RECV_FRAME  recvbuf;
   int      length;
   TP    *tp;

   tp = (TP *)recvbuf.data;

   while (TRUE)
   {
       bzero( &recvbuf, sizeof(recvbuf));
       length = Receive(fildes, &recvbuf, sizeof(recvbuf), CLIENT_TIMEOUT);
       if (length < 0) return (-1);    /* error */

       /*  For LAN client/server tests, the ack will return a  */
       /*  sequence number:               */

       dumpPacket("<---ACK",pktlog,&recvbuf,length);

       if (tp->operation == TEST_ACK)     /* an ack?  */
       {
      if (!strcmp(tp->dest, CLIENT_PA))   /* for me? */
          return (tp->seq_number);     /* return seq number */
       }
   }
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

Send_Halt (fildes, netid, net_addr)
   int      fildes;
   int      netid;
   unsigned char  *net_addr;
{
   XMIT_FRAME  xmitbuf;
   TP    *tp;
   int      i, length;

   tp   = (TP *)xmitbuf.data;
                  /* Fill in packet: */
   strcpy(tp->src,  CLIENT_PA);     /* My protocol address */
   strcpy(tp->dest, SERVER_PA);     /* his protocol address */
   tp->operation  = TEST_HALT;      /* type of test packet */
   tp->length     = 0;        /* size after header */
   tp->seq_number = 0;        /* null sequence number */
   for (i = 0; i < HLEN; i++)       /* fill in hardware addr's */
   {
       xmitbuf.src[i]  = Client_Addr[i];
       xmitbuf.dest[i] = net_addr[i];
   }
   xmitbuf.netid = netid;
   xmitbuf.ac = 0;
   xmitbuf.fc = LLC_TYPE;

   length = route(&xmitbuf,offsetof(TP,data),&csRoute);

   return (Transmit( fildes, &xmitbuf, length, FALSE ));
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
   int      max_errors, timeout = ARP_TIMEOUT;
   int      bad_frame, ignore_arps = FALSE;
   int      bad_frames = 0, lost_frames = 0;
   int      ack_no = -1, tp_no = -1;
   int      deadlock_ack = FALSE, errors = 0;
   int      netid;

   ttp   = (TP        *)xmitbuf.data;
   rtp   = (TP        *)recvbuf.data;
   tarp  = (TP_ARP    *)ttp->data;
   tparm = (TP_PARMS  *)ttp->data;
   tres  = (TP_RESULT *)ttp->data;
   rarp  = (TP_ARP    *)rtp->data;
   rparm = (TP_PARMS  *)rtp->data;
   xmit_frames = recv_frames = 0;

   printf("\n\t ***********************  SERVER  ***********************");

   netid = prompt("\n\t Use what netid (hex)", "%x",profile.default_netid);
   if (result < 0) return (FALSE);

   /*  The receive queues must be cleared to eliminate any  */
   /*  residual test frames from previous tests.         */

   printf("\n\t Clearing receive queues . . .");
   while ((result = Receive(fildes, &recvbuf, sizeof(recvbuf), 100)) > 0);

   /*  The hardware address of the local adapter is read (via  */
   /*  vital product data) to uniquely identify this server to the   */
   /*  client.                   */

   printf("\n\t Getting hardware address . . . ");
   Get_Hw_Addr(fildes, Server_Addr);
   for (i = 0; i < HLEN; i++)
       printf("%.2x", Server_Addr[i]);

   /*  A "well-known" netid is registered with the driver; this   */
   /*  same netid is registered by the Client.        */

   printf("\n\t Attaching Test protocol . . .");
   if (!Attach_Protocol(fildes, netid))
       return;

   /*  Receive processing loop; if a receive timeout occurs,   */
   /*  a retry ack is issued to break any deadlocks.  If the next */
   /*  receive times out, the Client is assumed dead and the test */
   /*  is aborted.                     */

   printf("\n\t Waiting for a request from the Client . . .");

   while (TRUE)
   {
       bzero(&recvbuf, sizeof(recvbuf));

       length = Receive(fildes, &recvbuf, sizeof(recvbuf), timeout);

       if (length < 0)     /* receive timeout? */
       {
          dumpPacket("***Read Timeout",pktlog,NULL,0);

          if (deadlock_ack)
          {  /* did we issue a deadlock ack? */
             fprintf(stderr, "\n\t Lost contact with Client!");
             lost_frames++; errors++;
             goto exit;    /* then give up */
          }
          else
          {    /* else, issue a deadlock ack */
             if (!Send_Ack("--->Deadlock ACK",fildes, netid,Client_Addr, ++ack_no))
                return;
             deadlock_ack = TRUE;
             continue;     /* and wait once more */
          }
       }
       else
          deadlock_ack = FALSE;         /* we got something */

       if (!strcmp(rtp->dest, SERVER_PA))
       {  /* for me? */
          switch (rtp->operation) {     /* then handle it: */

          /*  ARP request from Client -- send an ARP reply  */
          /*  back to the Client and lock out all future */
          /*  ARP requests (from other Client/Server sessions  */
          /*  starting later).  This is used only by LAN */
          /*  Client/Server tests.           */

      case TEST_ARP:         /* test addr resolve */
         if (ignore_arps)     /* we are taken */
             break;
         printf("\n\t ARP Request received:");

         printf("\n\t The Client's address is ");

         for (i = 0; i < HLEN; i++)
         {           /* get address */
             Client_Addr[i] = rarp->sender[i];
             printf("%.2x", Client_Addr[i]);
         }

         dumpPacket("<---ARP Request",pktlog,&recvbuf,length);

         memcpy(&csRoute,&recvbuf.route,sizeof(csRoute));

         csRoute.rctl &= 0x1FFF;
         csRoute.rctl |= 0x0080;

         printf("\n\t Issuing ARP reply to the Client . . .");
                     /* Fill in packet: */
         strcpy(ttp->src,  SERVER_PA); /* My prot address */
         strcpy(ttp->dest, CLIENT_PA); /* his prot address */
         ttp->operation = TEST_ARP; /* type of test pkt */
         ttp->length    = sizeof(TP_ARP);/* size after header */
         ttp->seq_number = 0;    /* null seq number */
         tarp->type     = ARP_REPLY;   /* return hw address */
         xmitbuf.ac = 0;
         xmitbuf.fc = LLC_TYPE;
         xmitbuf.netid = netid;
         for (i = 0; i < HLEN; i++)    /* fill in hw addr's */
         {
             tarp->sender[i] = Server_Addr[i];  /* mine */
             tarp->target[i] = Client_Addr[i];  /* his */
             xmitbuf.src[i]  = Server_Addr[i];
             xmitbuf.dest[i] = Client_Addr[i];
         }           /* send it via write */
         ignore_arps = TRUE;     /* lock out ARPS */

         length = route(&xmitbuf,sizeof(TP_ARP),&csRoute);

         dumpPacket("--->ARP Reply",pktlog,&xmitbuf,length);

         if (!Transmit( fildes, &xmitbuf, length, FALSE ))
             return;       /* quit if error */
         break;

         /*  Test parameters from the Client -- set our    */
         /*  local variables accordingly and return an ack.   */

         case TEST_PARMS:       /* test parameters */
         printf("\n\t Test Parameters Packet received");

         dumpPacket("<---Test Parameters",pktlog,&recvbuf,length);

         ack_spacing  = rparm->ack_spacing;
         pattern_type = rparm->pattern_type;
         max_errors   = rparm->max_errors;
         timeout = SERVER_TIMEOUT;  /* shorter timeout */
         j = 0;            /* reset counter */
         printf("\n\t Sending ACK to the Client");
         if (!Send_Ack("--->Test Parameter ACK", fildes, netid,Client_Addr, ++ack_no))
             return;       /* send ack */
         bad_frame = FALSE;
         printf("\n\n\t --  Test in Progress  --");
         break;

          /*  Test data packet from the Client:    */

          case TEST_PATTERN:

         dumpPacket("<---Data",pktlog,&recvbuf,length);

         /*  If the frames sequence number is greater    */
         /*  than the expected sequence number, then one */
         /*  or more frames were dropped -- increment */
         /*  the lost frame count by the missing frames. */
         /*  Update the receive count and receive seq */
         /*  number to the values of the last frame.  */

         if (++tp_no < rtp->seq_number)   /* out of sequence? */
         {
             /* If last frame was bad and we are only    */
             /* missing one frame, treat as corrupted */
             /* frame, else treat as missing:      */

             if (((rtp->seq_number - tp_no) == 1) && bad_frame)
            tp_no = rtp->seq_number;
             else {
                    fprintf(stderr,
               "\n\t Test Packet out of sequence:");
                 fprintf(stderr, " Expect pkt %d, got pkt %d.",
                tp_no, rtp->seq_number);
                 lost_frames =
                lost_frames + (rtp->seq_number - tp_no);
                 j += (rtp->seq_number - tp_no);
                 tp_no = rtp->seq_number;
            ++errors;
             }
         }
         /*  If this frame has data corruption, save it  */
         /*  in a file and increment the bad frame    */
         /*  count.              */

         if (!verify_pattern(rtp->data, rtp->length, 0,
               pattern_type, stdout))
         {
             save_bad_frame (&recvbuf,
            rtp->length + TP_HDR_SIZE, pattern_type,
            ++bad_frames, TP_HDR_SIZE, Sfile);
             ++errors;
         }
         bad_frame = FALSE;
         if (errors >= max_errors)
             goto exit;

         /*  If it is time to send an ack to the Client, */
         /*  increment the ack number and send the ack.  */

         if (ack_spacing != 0)      /* ack if required */
             if (!(++j % ack_spacing))
                 if (!Send_Ack("---> Data ACK",fildes, netid,Client_Addr, ++ack_no))
                return;
         break;

          /*  Halt request from the Client; exit the test.  */

          case TEST_HALT:
         printf("\n\t Halt frame received. ");
         goto exit;

          default:
         dumpPacket("<---????",pktlog,&recvbuf,length);
         fprintf(stderr,
            "\n\t Unknown Test Operation received: %d",
            rtp->operation);
         break;
      }
       }
       else
       {               /* corrupted frame */
          fprintf(stderr, "\n\t Corrupted Frame received:\n");

          dumpPacket("<---????",pktlog,&recvbuf,length);

          save_bad_frame(&recvbuf, length,
                         RANDOM_PATTERN,
                         ++bad_frames,
                         TP_HDR_SIZE,
                         Sfile);

          bad_frame = TRUE;
          if (++errors >= max_errors)
              break;
       }
   }
    exit:
   xmitbuf.ac = 0;
   xmitbuf.fc = LLC_TYPE;

   /*  End the test by sending a test results frame to the Client.   */
   /*  Display the results on this screen in case the results do  */
   /*  not make it to the Client.               */

   strcpy(ttp->src,  SERVER_PA);       /* my address */
   strcpy(ttp->dest, CLIENT_PA);       /* his address */
   xmitbuf.netid  = netid;
   ttp->operation = TEST_RESULTS;         /* sending results */
   ttp->length = sizeof(TP_RESULT);    /* length of data */
   tres->lost_frames = lost_frames;
   tres->bad_frames  = bad_frames;
   tres->xmit_frames = xmit_frames;
   tres->recv_frames = recv_frames;

   printf("\n\t Sending test results to Client . . .");

   length = route(&xmitbuf,sizeof(TP_RESULT),&csRoute);

   if (!Transmit(fildes, &xmitbuf, length,FALSE))
       return;             /* quit if error */
   if (!lost_frames && !bad_frames)
   {
       printf("\n\t Tests completed successfully.");
       printf("\n\t     Number of frames received:     %d",
           recv_frames);
       printf("\n\t     Number of frames transmitted:  %d",
           xmit_frames);
   } else {
       fprintf(stderr, "\n\t Tests completed with errors:");
       fprintf(stderr, "\n\t     Server, device %s:", &path[0]);
       fprintf(stderr, "\n\t     Frames from Client lost:       %d",
       lost_frames);
       fprintf(stderr, "\n\t     Frames with corrupted data:    %d",
       bad_frames);
       fprintf(stderr, "\n\t     Number of frames received:     %d",
           recv_frames);
       fprintf(stderr, "\n\t     Number of frames transmitted:  %d",
           xmit_frames);
   }
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

Send_Ack (title, fildes, netid,net_addr, ack_number)
   char    *title;
   int      fildes;
   int      netid;
   unsigned char  *net_addr;
   int      ack_number;
{
   XMIT_FRAME  xmitbuf;
   TP    *tp;
   int      i, length;

   bzero( &xmitbuf, sizeof(xmitbuf));

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
       xmitbuf.src[i]  = Server_Addr[i];
       xmitbuf.dest[i] = net_addr[i];
   }

   xmitbuf.netid = netid;
   xmitbuf.ac = 0;
   xmitbuf.fc = LLC_TYPE;

   length = route(&xmitbuf,offsetof(TP,data),&csRoute);

   dumpPacket(title,pktlog,&xmitbuf,length);

   return (Transmit( fildes, &xmitbuf, length,FALSE));
}

/*======================================================================*/
/*               CLIENT/SERVER TEST PROTOCOL PRIMITIVES                 */
/*======================================================================*/


int
   route(
      char                 * packet      ,
      int                    length      ,
      ROUTING_INFO         * ri
   )
{
   int                       ril = ((ri->rctl & 0x1F00) >> 8);

   ((MAC_FRAME *)packet)->src[0] |= 0x80; /* turn on source routing */

   memcpy(packet+MAC_HDR,ri,ril);

   if (ril < sizeof(csRoute))
   {
      char *src = packet + MAC_HDR + sizeof(csRoute);
      char *dst = packet + MAC_HDR + ril;

      int   l   = 2+TEST_HEADER+length;

      while(l--) *dst++ = *src++;
   }

   return MAC_HDR+ril+2+TEST_HEADER+length;
}

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

Transmit (fildes, packet, length, fastwrt_flag)
   int      fildes;
   char     *packet;
   unsigned int   length;
   uchar    fastwrt_flag;
{
   WRITE_EXT   ext;
   int   retry = 0, result;

   bzero(&ext, sizeof(ext));

   /*  If in kernel mode and mbufs are not freed by the driver */
   /*  after writes, then we must get status on every write so */
   /*  that we (specifically KID) will know when to free mbufs.   */

   if (!profile.free_mbuf)
       ext.ciowe.flag |= CIO_NOFREE_MBUF;

   if (fastwrt_flag)
   {
      ext.fastwrt_type = NORMAL_CHAIN;
      ext.fastwrt_opt.offlevel = FALSE;
   }
   else
      ext.fastwrt_type = 0;

   length = MAX(MIN_PACKET,length);

   while (TRUE)
   {
      result = WRITE( fildes, packet, length, &ext );

      if (result < 0)
      {
         printf("Write Error\n");
         dumpPacket("***Error on Write",pktlog,packet,length);

         switch (errno)
         {
            case EIO:
               fprintf(stderr, "\n\t ERROR during write: see status");
               return(FALSE);

            case EAGAIN:
               printf("EAGAIN ");
               dumpPacket("***EAGAIN",pktlog,packet,length);

               if (retry++ < WRITE_RETRIES)
                    break;
            default:
               fprintf(stderr, "\n\t Write ");
               decode_error(errno);
               return(FALSE);
         }
      }
      else
      {
         if (ext.ciowe.flag & CIO_ACK_TX_DONE)
            if (!wait_status(fildes, CIO_TX_DONE, STATUS_TIMEOUT))
            {
               printf("\nTimeout on Transmit-Done Status\n");
               return(FALSE);    /* timed out or error */
            }
         return(++xmit_frames);
      }
   }
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
   int      rc;
   struct pollfd  pollblk;
   struct read_extension   ext;

   bzero( &ext, sizeof(ext));

   /*  Wait for receive data by polling for it:    */

   pollblk.fd = fildes;
   pollblk.reqevents = POLLIN;
   pollblk.rtnevents = 0;
   if ((rc =poll( &pollblk, 1, timeout )) == 0)
   {
      printf("poll timeout\n");
      result = -1;
   }
   else if (rc < 0)
   {
      perror("poll failed");
      result = -1;
   }
   else {
       result = READX( fildes, packet, length, &ext );
       if (result < 0) {
           fprintf(stderr, "\n\t Read ");
           decode_error(errno);
       } else {
      if ( result )
               ++recv_frames;
       }
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
   short protocol;
{
   struct session_blk   session;
   int   result;

   session.netid = protocol;
   session.status = 0;
   result = IOCTL( fildes, CIO_START, &session );
   if (result < 0) {       /* if error */
       if (errno == EADDRINUSE)     /* if already issued */
      return(TRUE);        /* then no problem */
       switch (errno) {       /* else, no can do */
      case EIO:
             fprintf(stderr, "\n\t ERROR: ");
             switch (session.status) {
         case CIO_HARD_FAIL:
            fprintf(stderr, "CIO_HARD_FAIL");
            break;
         default:
            fprintf(stderr, "Undefined status %d",
                      session.status);
            break;
             }
          break;
      default:
             fprintf(stderr, "\n\t ");
          decode_error(errno);
          break;
       }
       return(FALSE);
   } else
       if (result = wait_status(fildes, CIO_START_DONE, STATUS_TIMEOUT))
      return(++started);
       else
      return(result);
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
   if (result < 0) {       /* if error */
       switch (errno) {       /* else, no can do */
      case EIO:
             fprintf(stderr, "\n\t ERROR: ");
             switch (session.status) {
         case CIO_HARD_FAIL:
            fprintf(stderr, "CIO_HARD_FAIL");
            break;
         default:
            fprintf(stderr, "Undefined status %d",
                      session.status);
            break;
             }
          break;
      default:
             fprintf(stderr, "\n\t ");
          decode_error(errno);
          break;
       }
       return(FALSE);
   } else {
       if (result = wait_status(fildes, CIO_HALT_DONE, STATUS_TIMEOUT))
      return(--started);
       else
      return(result);
   }
}

dump_test(fildes)
   int         fildes;
{
   char           test = 0;

   /*
    *  Check to make sure kid driver is opened.
    */
   if (!kern) {
      printf("\n\t\t   The driver must be opened in kernel mode! \n");
      return;
   }

   while (TRUE) {
      /*
       *  Display dump sub-menu.
       */
      printf("\n\n");
      printf("\n\n\t\t       Token Ring Device Driver Tool");
      printf("\n\t\t           %s (kernel mode)", path);
       printf("\n   Dump Commands Panel:");
      if (!dumpinit && !dumpstart)
         printf("\n\t I)   Dump Init      S)   Dump Start        ");
      if (dumpinit && !dumpstart)
         printf("\n\t I) * Dump Init      S)   Dump Start        ");
      if (dumpinit && dumpstart)
         printf("\n\t I) * Dump Init      S) * Dump Start        ");
      printf("U)   Dump Query");
      printf("\n\t W)   Dump Write     R)   Dump Read         ");
      printf("E)   Dump End");
      printf("\n\t T)   Dump Term      X)   Main Menu ");
      printf("\n");
      printf("\n\t Choose from the menu or enter Q to quit: ");

      /*
       *  Set up dump_info structure with common information.
       */
      dump_info.devno = dopen.devno;

      /*
       *  Process command.
       */
      if ((test = getinput()) != '\n') getinput();
      switch (test) {         /* select test routine */
         case 'i':
         case 'I':
            dump_info.dump_cmd = DUMPINIT;
            if (IOCTL( fildes, KID_DUMP_CMD, &dump_info ) < 0)
               printf("\nThe Dump Init command failed.\n");
            else
               if (!dumpinit)
                  dumpinit++;
            break;
         case 's':
         case 'S':
            dump_info.dump_cmd = DUMPSTART;
            if (IOCTL( fildes, KID_DUMP_CMD, &dump_info ) < 0)
               printf("\nThe Dump Start command failed.\n");
            else
               if (!dumpstart)
                  dumpstart++;
            break;
         case 'u':
         case 'U':
            dump_info.dump_cmd = DUMPQUERY;
            if (IOCTL( fildes, KID_DUMP_CMD, &dump_info ) < 0)
               printf("\nThe Dump Start command failed.\n");
            else {
               printf("\n");
               printf("\t*********** Dump Query Succeeded ***********\n");
               printf("\n");
               printf("Max Packet Size    :  %d\n",
                     dump_info.dmp_query.max_tsize);
               printf("Minimum Packet Size:  %d\n",
                     dump_info.dmp_query.min_tsize);
               printf("\n");
            }
            break;
         case 'w':
         case 'W':
            dump_info.dump_cmd = DUMPWRITE;
            if (dump_write_test( fildes ) < 0)
               printf("\n\tThe Dump Write command failed.\n");
            break;
         case 'r':
         case 'R':
            dump_info.dump_cmd = DUMPREAD;
            if (dump_read_test( fildes ) < 0)
               printf("\n\tThe Dump Read command failed.\n");
            break;
            break;
         case 'e':
         case 'E':
            dump_info.dump_cmd = DUMPEND;
            if (dumpstart)
               if (IOCTL( fildes, KID_DUMP_CMD, &dump_info ) < 0)
                  printf("\nThe Dump End command failed.\n");
               else
                  dumpstart--;
            break;
         case 't':
         case 'T':
            dump_info.dump_cmd = DUMPTERM;
            if (IOCTL( fildes, KID_DUMP_CMD, &dump_info ) < 0)
               printf("\nThe Dump Term command failed.\n");
            else
               if (dumpinit)
                  dumpinit--;
            break;
         case 'x':
         case 'X':
            return;
      }
   }
}

/*------------------   D U M P _ W R I T E _ T E S T   -----------------*/
/*                         */
/*  NAME: dump_write_test                 */
/*                         */
/*  FUNCTION:                       */
/* Writes a dump test frame to the open device associated with fildes.*/
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

dump_write_test (fildes)
   int   fildes;
{
   int   result, rc, i = FALSE;
   int   done = FALSE;
   return done;

   rc = TRUE;
   printf("\n\t ********************   DUMP WRITE   *********************");
   printf("\n");
   if (!dumpstart) {
      printf("\n");
      printf("\tYou must issue a DUMP START before issuing a DUMP WRITE.\n");
      return(-1);
   }
   while (!done) {
      printf("\t Destination Address (6 bytes hex) [%s]: ",
            profile.default_dest);
      i = 0;
      while ((str[i++] = getinput()) != '\n');
      if (str[0] == '.') return(-1);
      str[i-1] = '\0';
      if (str[0] == '\0') {
         hexstring_to_array(profile.default_dest, writbuf.dest);
         break;
      } else
         done = (strlen(str) == 12) &&
         hexstring_to_array(str, writbuf.dest);
   }
   result = prompt("\t What is the destination netid (hex)",
               "%x", profile.default_netid);
   if ( result < 0 ) return(-1);
   sprintf( profile.default_netid, "%x", result );
   writbuf.netid = result;
   Get_Hw_Addr(fildes, writbuf.src);
   writbuf.ac = 0;
   if (writbuf.netid == TOK_MAC_FRAME_NETID)
      writbuf.fc = MAC_TYPE;
   else
      writbuf.fc = LLC_TYPE;
   writlen = MAX( 0, writlen - DATALINK_HDR );
   pattern = fill_buffer(writbuf.data, &writlen, BUFSIZE );
   if (pattern < 0) return(-1);
   writlen += DATALINK_HDR;
   sprintf( profile.default_writes, "%d", 1 );
   dump_info.writebuf = &writbuf;
   dump_info.writelen = writlen;
   result = IOCTL(fildes,KID_DUMP_WRITE,&dump_info);
   if (result < 0) {    /* if error */
      fprintf(stderr, "\n\t Write %d ", i);
      switch (errno) {
         case EIO:
            fprintf(stderr, "ERROR: See Status block for reason");
            break;
         default:
            decode_error(errno);
            break;
      }
      rc = -1;    /* failed */
   }
   if (rc<0)
      printf("\n\t -- Dump write failed. --");
   else
      printf("\n\t -- Dump write succeeded. --");
   printf("\n\t ********************************************************");
   return(rc);
}

/*-------------------   D U M P _ R E A D _ T E S T   ------------------*/
/*                         */
/*  NAME: dump_read_test                  */
/*                         */
/*  FUNCTION:                       */
/* Reads a dump test frame from the open device associated with fildes.*/
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

dump_read_test(fildes)
   int      fildes;
{

   int      result;

   if (!dumpstart) {
      printf("\n");
      printf("\tYou must issue a DUMP START before issuing a DUMP WRITE.\n");
      return(-1);
   }

   result = IOCTL(fildes,KID_DUMP_CMD,&dump_info);

    if (!result) {
      read_test(fildes);
   } else
      printf("\n\t -- Dump read failed. --");

} /* End of dump_read_test() */

/*
 *  Get Adapter dump data
 */
read_adap_dump(fildes)
   int fildes;
{
   unsigned short  dump_data[32768];
   unsigned short  *tempbuf;
        int             len, words, bytes, offset;
   FILE            *testfile;

   IOCTL(fildes,TEST_IOCTL,&dump_data);

   testfile = fopen("/tmp/dump_data", "w");

   tempbuf = &dump_data;

        len = 65536;
   bytes = words = offset = 0;
   /*
    * Print a new line so data will start in column 0.
    */
   fprintf(testfile,"\n");

   /*
    * Print first offset
    */
   fprintf(testfile,"%2x", offset);
   fprintf(testfile,"^");

        while (len > 0)
        {
      fprintf(testfile,"%4x", *tempbuf);

      /*
       *  Put a space in between words
       */
      if (bytes == 1) {
         bytes = 0;
         words++;
         fprintf(testfile,"^");
      } else
         bytes++;
      /*
       * Check to see if a new line is needed
       */
      if (words == 4) {
         words = 0;
         offset += 16;
         fprintf(testfile,"\n");
         fprintf(testfile,"%2x", offset);
         fprintf(testfile,"^");
      }

                len -= 2;
                ++tempbuf;
        }

   fclose(testfile);

} /* End of read_adap_dump() */

/*
 *  Cause Adapter dump data
 */
cause_adap_dump(fildes)
   int fildes;
{
   unsigned short  dump_data[32768];
   unsigned short  *tempbuf;
        int             len, words, bytes, offset;
   FILE            *testfile;

   IOCTL(fildes,TEST_DO_FREEZE_DUMP,0);

} /* End of cause_adap_dump() */

#ifdef __XTRA_CODE__
/*                         */
/*  NAME: tok_mib_query                   */
/*                         */
/*  FUNCTION:                       */
/* Issues the ioctl TOK_MIB_QUERY  to the driver under test    */
/* (via the file descriptor "fildes") and decodes returned  */
/* error codes, if any.  Optionally clears the statistics by   */
/* reissuing the ioctl with the clear flag turned on.    */
/*  EXECUTION ENVIRONMENT:                */
/* Can only be executed at the user process level.       */
/*                         */
/*  DATA STRUCTURES:                   */
/* References no global data structures.           */
/*                            */
/*  RETURNS: nothing                      */
/*                         */

tok_mib_query (fildes)
   int   fildes;
{
   int   i, result;
   token_ring_all_mib_t mibs;

   printf("\n\t ******************* MIB QUERY *******************");
   bzero(&mibs, sizeof(mibs));

   result = IOCTL( fildes, TOK_MIB_QUERY, &mibs );
   if (result < 0)   {        /* if error */
       fprintf(stderr, "\n\t ");
       decode_error(errno);
   } else {
       printf("\n\t ");
       printf("\n\t Generic MIBs");
       printf("\n");
       printf("\n\t Chip Set             = %4d",
            mibs.Generic_mib.ifExtnsEntry.chipset);
       printf("   Revision Ware            = %4d",
            mibs.Generic_mib.ifExtnsEntry.revware[0]);

       printf("\n\t Multicast TX OK      = %4d",
            mibs.Generic_mib.ifExtnsEntry.mcast_tx_ok);
       printf("   Broadcast TX OK          = %4d",
            mibs.Generic_mib.ifExtnsEntry.bcast_tx_ok);
       printf("\n\t Multicast RCV OK     = %4d",
            mibs.Generic_mib.ifExtnsEntry.mcast_rx_ok);
       printf("   Broadcast RCV OK         = %4d",
            mibs.Generic_mib.ifExtnsEntry.bcast_rx_ok);

       printf("\n\t Extension Test Type  = %4d",
            mibs.Generic_mib.ifExtnsTestEntry.type);
       printf("   Extension Test Result    = %4d",
            mibs.Generic_mib.ifExtnsTestEntry.result);
       printf("\n\t Extension Test Code  = %4d",
            mibs.Generic_mib.ifExtnsTestEntry.code);

       printf("\n\t RCV Address          = %4d",
         mibs.Generic_mib.ifExtnsRcvAddrEntry.address[0]);
       printf("   RCV Address Status       = %4d",
            mibs.Generic_mib.ifExtnsRcvAddrEntry.status);



       printf("\n");
       printf("\n\t Token-Ring MIBs");
       printf("\n");
       printf("\n\t Commands             = %4d",
         mibs.Token_ring_mib.Dot5Entry.commands);
       printf("   Ring Status              = %4d",
         mibs.Token_ring_mib.Dot5Entry.ring_status);
       printf("\n\t Ring State           = %4d",
         mibs.Token_ring_mib.Dot5Entry.ring_state);
       printf("   Ring Open Status         = %4d",
         mibs.Token_ring_mib.Dot5Entry.ring_status);
       printf("\n\t Ring Speed           = %4d",
         mibs.Token_ring_mib.Dot5Entry.ring_speed);
       printf("   Upstream                 = %4d",
            mibs.Token_ring_mib.Dot5Entry.upstream[0]);
       printf("\n\t A.M. Participate     = %4d",
         mibs.Token_ring_mib.Dot5Entry.participate);
       printf("   Functional               = %4d",
            mibs.Token_ring_mib.Dot5Entry.functional[0]);

      /* Dot5StatsEntry */
       printf("\n\t Line Errors          = %4d",
         mibs.Token_ring_mib.Dot5StatsEntry.line_errs);
       printf("   Burst Errors             = %4d",
         mibs.Token_ring_mib.Dot5StatsEntry.burst_errs);
       printf("\n\t AC Errors            = %4d",
         mibs.Token_ring_mib.Dot5StatsEntry.ac_errs);
       printf("   Abort Errors             = %4d",
         mibs.Token_ring_mib.Dot5StatsEntry.abort_errs);
       printf("\n\t Internal Errors      = %4d",
         mibs.Token_ring_mib.Dot5StatsEntry.int_errs);
       printf("   Lost Frames              = %4d",
         mibs.Token_ring_mib.Dot5StatsEntry.lostframes);
       printf("\n\t RCV Congestion       = %4d",
         mibs.Token_ring_mib.Dot5StatsEntry.rx_congestion);
       printf("   Frame Copies             = %4d",
         mibs.Token_ring_mib.Dot5StatsEntry.framecopies);
       printf("\n\t Token Errors         = %4d",
         mibs.Token_ring_mib.Dot5StatsEntry.token_errs);
       printf("   Soft Errors              = %4d",
         mibs.Token_ring_mib.Dot5StatsEntry.soft_errs);
       printf("\n\t Hard Errors          = %4d",
         mibs.Token_ring_mib.Dot5StatsEntry.hard_errs);
       printf("   Signal Loss              = %4d",
         mibs.Token_ring_mib.Dot5StatsEntry.signal_loss);
       printf("\n\t TX Beacons           = %4d",
         mibs.Token_ring_mib.Dot5StatsEntry.tx_beacons);
       printf("   Recoverys                = %4d",
         mibs.Token_ring_mib.Dot5StatsEntry.recoverys);
       printf("\n\t Lobe Wires           = %4d",
         mibs.Token_ring_mib.Dot5StatsEntry.lobewires);
       printf("   Removes                  = %4d",
         mibs.Token_ring_mib.Dot5StatsEntry.removes);
       printf("\n\t Singles              = %4d",
         mibs.Token_ring_mib.Dot5StatsEntry.singles);
       printf("   Frequency Errors         = %4d",
         mibs.Token_ring_mib.Dot5StatsEntry.freq_errs);

      /* Dot5TimerEntry */
       printf("\n\t Return Repeat        = %4d",
         mibs.Token_ring_mib.Dot5TimerEntry.return_repeat);
       printf("   Holding                  = %4d",
         mibs.Token_ring_mib.Dot5TimerEntry.holding);
       printf("\n\t Queue PDU            = %4d",
         mibs.Token_ring_mib.Dot5TimerEntry.queue_pdu);
       printf("   Valid Transmit           = %4d",
         mibs.Token_ring_mib.Dot5TimerEntry.valid_tx);
       printf("\n\t No Token             = %4d",
         mibs.Token_ring_mib.Dot5TimerEntry.no_token);
       printf("   Active Monitor           = %4d",
         mibs.Token_ring_mib.Dot5TimerEntry.active_mon);
       printf("\n\t Standby Monitor      = %4d",
         mibs.Token_ring_mib.Dot5TimerEntry.standby_mon);
       printf("   Error Report             = %4d",
         mibs.Token_ring_mib.Dot5TimerEntry.err_report);
       printf("\n\t Beacon Transmit      = %4d",
         mibs.Token_ring_mib.Dot5TimerEntry.beacon_tx);
       printf("   Beacon RCV               = %4d",
         mibs.Token_ring_mib.Dot5TimerEntry.beacon_tx);
   }
   printf("\n");
   printf("\n\t ********************************************************");
}
/*                         */
/*  NAME: tok_mib_get                     */
/*                         */
/*  FUNCTION:                       */
/* Issues the ioctl TOK_MIB_GET  to the driver under test      */
/* (via the file descriptor "fildes") and decodes returned  */
/* error codes, if any.  Optionally clears the statistics by   */
/* reissuing the ioctl with the clear flag turned on.    */
/*  EXECUTION ENVIRONMENT:                */
/* Can only be executed at the user process level.       */
/*                         */
/*  DATA STRUCTURES:                   */
/* References no global data structures.           */
/*                            */
/*  RETURNS: nothing                      */
/*                         */

tok_mib_get (fildes)
   int   fildes;
{
   int   i, result;
   token_ring_all_mib_t mibs;

   printf("\n\t ******************* MIB GET *******************");
   bzero(&mibs, sizeof(mibs));

   result = IOCTL( fildes, TOK_MIB_GET, &mibs );
   if (result < 0)   {        /* if error */
       fprintf(stderr, "\n\t ");
       decode_error(errno);
   } else {
       printf("\n\t ");
       printf("\n\t Generic MIBs");
       printf("\n");
       printf("\n\t Chip Set             = %8d",
            mibs.Generic_mib.ifExtnsEntry.chipset);

       printf("\n\t Revision Ware        = 0x");
       for (i = 0; i < 16; i++)
      printf("%.2x", mibs.Generic_mib.ifExtnsEntry.revware[i]);

       printf("\n\t Multicast TX OK      = %8d",
            mibs.Generic_mib.ifExtnsEntry.mcast_tx_ok);
       printf("   Broadcast TX OK          = %8d",
            mibs.Generic_mib.ifExtnsEntry.bcast_tx_ok);
       printf("\n\t Multicast RCV OK     = %8d",
            mibs.Generic_mib.ifExtnsEntry.mcast_rx_ok);
       printf("   Broadcast RCV OK         = %8d",
            mibs.Generic_mib.ifExtnsEntry.bcast_rx_ok);


       printf("\n\t Extension Test Type  = %8d",
            mibs.Generic_mib.ifExtnsTestEntry.type);
       printf("   Extension Test Result    = %8d",
            mibs.Generic_mib.ifExtnsTestEntry.result);
       printf("\n\t Extension Test Code  = %8d",
            mibs.Generic_mib.ifExtnsTestEntry.code);

       printf("   RCV Address Status       = %8d",
            mibs.Generic_mib.ifExtnsRcvAddrEntry.status);


       printf("\n\t RCV Address          = 0x");
       for (i = 0; i < 6; i++)
      printf("%.2x", mibs.Generic_mib.ifExtnsRcvAddrEntry.address[i]);

       printf("\n");
       printf("\n\t Token-Ring MIBs");
       printf("\n");
       printf("\n\t Commands             = %8d",
         mibs.Token_ring_mib.Dot5Entry.commands);
       printf("   Ring Status              = %8d",
         mibs.Token_ring_mib.Dot5Entry.ring_status);
       printf("\n\t Ring State           = %8d",
         mibs.Token_ring_mib.Dot5Entry.ring_state);
       printf("   Ring Open Status         = %8d",
         mibs.Token_ring_mib.Dot5Entry.ring_status);
       printf("\n\t Ring Speed           = %8d",
         mibs.Token_ring_mib.Dot5Entry.ring_speed);
       printf("   A.M. Participate         = %8d",
         mibs.Token_ring_mib.Dot5Entry.participate);

       printf("\n\t Upstream             = 0x");
       for (i = 0; i < 6; i++)
      printf("%.2x", mibs.Token_ring_mib.Dot5Entry.upstream[i]);

       printf("\n\t Functional           = 0x");
       for (i = 0; i < 6; i++)
      printf("%.2x", mibs.Token_ring_mib.Dot5Entry.functional[i]);

      /* Dot5StatsEntry */
       printf("\n\t Line Errors          = %8d",
         mibs.Token_ring_mib.Dot5StatsEntry.line_errs);
       printf("   Burst Errors             = %8d",
         mibs.Token_ring_mib.Dot5StatsEntry.burst_errs);
       printf("\n\t AC Errors            = %8d",
         mibs.Token_ring_mib.Dot5StatsEntry.ac_errs);
       printf("   Abort Errors             = %8d",
         mibs.Token_ring_mib.Dot5StatsEntry.abort_errs);
       printf("\n\t Internal Errors      = %8d",
         mibs.Token_ring_mib.Dot5StatsEntry.int_errs);
       printf("   Lost Frames              = %8d",
         mibs.Token_ring_mib.Dot5StatsEntry.lostframes);
       printf("\n\t RCV Congestion       = %8d",
         mibs.Token_ring_mib.Dot5StatsEntry.rx_congestion);
       printf("   Frame Copies             = %8d",
         mibs.Token_ring_mib.Dot5StatsEntry.framecopies);
       printf("\n\t Token Errors         = %8d",
         mibs.Token_ring_mib.Dot5StatsEntry.token_errs);
       printf("   Soft Errors              = %8d",
         mibs.Token_ring_mib.Dot5StatsEntry.soft_errs);
       printf("\n\t Hard Errors          = %8d",
         mibs.Token_ring_mib.Dot5StatsEntry.hard_errs);
       printf("   Signal Loss              = %8d",
         mibs.Token_ring_mib.Dot5StatsEntry.signal_loss);
       printf("\n\t TX Beacons           = %8d",
         mibs.Token_ring_mib.Dot5StatsEntry.tx_beacons);
       printf("   Recoverys                = %8d",
         mibs.Token_ring_mib.Dot5StatsEntry.recoverys);
       printf("\n\t Lobe Wires           = %8d",
         mibs.Token_ring_mib.Dot5StatsEntry.lobewires);
       printf("   Removes                  = %8d",
         mibs.Token_ring_mib.Dot5StatsEntry.removes);
       printf("\n\t Singles              = %8d",
         mibs.Token_ring_mib.Dot5StatsEntry.singles);
       printf("   Frequency Errors         = %8d",
         mibs.Token_ring_mib.Dot5StatsEntry.freq_errs);

      /* Dot5TimerEntry */
       printf("\n\t Return Repeat        = %8d",
         mibs.Token_ring_mib.Dot5TimerEntry.return_repeat);
       printf("   Holding                  = %8d",
         mibs.Token_ring_mib.Dot5TimerEntry.holding);
       printf("\n\t Queue PDU            = %8d",
         mibs.Token_ring_mib.Dot5TimerEntry.queue_pdu);
       printf("   Valid Transmit           = %8d",
         mibs.Token_ring_mib.Dot5TimerEntry.valid_tx);
       printf("\n\t No Token             = %8d",
         mibs.Token_ring_mib.Dot5TimerEntry.no_token);
       printf("   Active Monitor           = %8d",
         mibs.Token_ring_mib.Dot5TimerEntry.active_mon);
       printf("\n\t Standby Monitor      = %8d",
         mibs.Token_ring_mib.Dot5TimerEntry.standby_mon);
       printf("   Error Report             = %8d",
         mibs.Token_ring_mib.Dot5TimerEntry.err_report);
       printf("\n\t Beacon Transmit      = %8d",
         mibs.Token_ring_mib.Dot5TimerEntry.beacon_tx);
       printf("   Beacon RCV               = %8d",
         mibs.Token_ring_mib.Dot5TimerEntry.beacon_tx);
   }
   printf("\n");
   printf("\n\t ********************************************************");
}

#endif /* __XTRA_CODE__ */
