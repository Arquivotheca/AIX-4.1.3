static char sccsid[]="@(#)00   1.1  src/bos/usr/fvcioem/ddt/ent/ddt_ent.c, fvcioem, bos411, 9428A410j 4/26/94 13:54:27";

/*--------------------------------------------------------------------------
*
*             DDT_ENT.C
*
*  COMPONENT_NAME:  Communications Device Debug & Test (ENT Ethernet Driver).
*
*  FUNCTIONS:  main, start_test, halt_test, query_vpd_test, write_test,
*     read_test, edit_defaults, decode_status, Get_Hw_Addr,
*     dsopen, dsclose, statistics test, access_pos_test,
*     pos_read_test, pos_write_test, access_io_test, io_read_test,
*     io_write_test, access_ram_test, ram_read_test,
*     ram_write_test, dma_test, multicast_test, iocinfo_test,
*     Client, Arp_Request, Wait_Ack, Send_Halt, Server,
*     Send_Ack, Transmit, Receive, Attach_Protocol,
*     Detach_Protocol
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
# include <sys/errno.h>
# include <sys/types.h>
# include <sys/ioctl.h>
# include <sys/devinfo.h>
# include <sys/stat.h>
# include <sys/param.h>
# include <sys/poll.h>
# include <sys/uio.h>
# include <sys/comio.h>
# include <sys/ciouser.h>
# include <sys/dump.h>
# include <sys/entuser.h>

#ifdef __XTRA_CODE__
# include <sys/generic_mibs.h>
# include <sys/ethernet_mibs.h>
#endif /* __XTRA_CODE__ */

# include <etkiduser.h>
# include "ddt_com.h"
# include "ddt_ent.h"
# include "ddt_csp.h"

/*----------------------------------------------------------------------*/
/*          Global Variables           */
/*----------------------------------------------------------------------*/

/*  Shared from there:                    */

extern         errno;
extern int     block;         /* block mode flag */
extern int     kern;       /* kernel mode flag */
extern int     pattern;    /* type of test pattern */
extern int     started;    /* number of starts */
extern int     writlen;    /* length of write data */
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

/*----------------------------------------------------------------------*/
/*  Shared from here:                     */

unsigned char     Server_Addr[ HLEN ]; /* hardware address of Server */
unsigned char     Client_Addr[ HLEN ]; /* hardware address of Client */
char        Cfile[] = CFPATH; /* saved client frames */
char        Sfile[] = SFPATH; /* saved server frames */

/*----------------------------------------------------------------------*/
/*  Local:                       */

static XMIT_FRAME writbuf;    /* write buffer */
static RECV_FRAME readbuf;    /* read buffer */
static char    wrkbuf[ DATASIZE + 2 ]; /* work buffer */
static ent_dma_buf_t dma;        /* dma parameter block */
static int     dmafd;         /* file desc to entdma */



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

   autoresponse = 0;
   openstate = FALSE;
   started = 0;
   fildes = -1;
   dsopen();            /* do any initializations */
   writlen = DEFAULT_SIZE;
   while (TRUE) {
       printf("\n\n\t\t          Ethernet Device Driver Tool");
       if (openstate)
      if (kern)
          printf("\n\t\t           %s (kernel mode)", path);
      else
          printf("\n\t\t            %s (user mode)", path);
       else
           printf("\n");
       printf("\n   Control Panel:");
       if (!openstate && !started)
           printf("\n\t O)   Open Device    S)   Start Device      ");
       if (!openstate && started)
           printf("\n\t O)   Open Device    S) * Start Device      ");
       if (openstate && !started)
           printf("\n\t O) * Open Device    S)   Start Device      ");
       if (openstate && started)
           printf("\n\t O) * Open Device    S) * Start Device      ");
       printf("L)   List Statistics");
       printf("\n\t C)   Close Device   H)   Halt Device       ");

       printf("V)   Query VP Data");
       printf("\n\t G)   Get Status     P)   POS Registers     ");
       printf("D)   DMA Transfers");
       printf("\n\t A)   Adapter RAM    I)   I/O Registers     ");
       if (mcaddr)
           printf("M) * Multicast Address");
       else
           printf("M)   Multicast Address");
       printf("\n\t E)   Edit Defaults  F)   IOCINFO           ");

#ifdef __XTRA_CODE__
       printf("X)   Read Bad Frame");
       printf("\n\t J)   Promisc ON     K)   Promisc OFF       ");
       printf("B)   Bad Frame ON");
       printf("\n\t U)   MIB Query      Y)   MIB Get           ");
       printf("N)   Bad Frame OFF");
#endif /* __XTRA_CODE__ */

       printf("\n\t W)   Write          R)   Read              ");
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
       invalid = badtry = FALSE;    /* innocent til proven guilty */
       switch (test) {        /* select test routine */
         case 'o':         /* Open Device test */
         case 'O':
           if (openstate) {
              printf("\n\t\t The device is already open!");
           } else {
              fildes = open_test(DEVPATH);
              if (fildes >= 0) openstate = TRUE;
           }
           break;
         case 'c':         /* Close Device test */
      case 'C': if (!openstate) badtry = TRUE;
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
         case 's':         /* Start Device test */
      case 'S': if (!openstate) badtry = TRUE;
           else if (start_test(fildes)) started++;
           break;
         case 'h':         /* Halt Device test */
         case 'H': if (!openstate) badtry = TRUE;
           else if (halt_test(fildes)) started--;
           break;
         case 'g':         /* Get Status test */
         case 'G': if (!openstate) badtry = TRUE;
           else status_test(fildes);
           break;
         case 'l':         /* Query Statistics Test */
         case 'L': if (!openstate) badtry = TRUE;
           else statistics_test(fildes);
           break;
         case 'e':         /* Edit Defaults */
      case 'E': edit_defaults(fildes);
           break;
      case 'v':         /* Query Vital Product Data */
      case 'V': if (!openstate) badtry = TRUE;
           else query_vpd_test(fildes);
           break;
         case 'p':         /* Access POS test */
         case 'P': if (!openstate) badtry = TRUE;
           else access_pos_test(fildes);
           break;
      case 'i':         /* Ethernet Access I/O test */
      case 'I': if (!openstate) badtry = TRUE;
           else access_io_test(fildes);
           break;
      case 'a':         /* Ethernet Access RAM test */
      case 'A': if (!openstate) badtry = TRUE;
           else access_ram_test(fildes);
           break;
      case 'd':         /* Ethernet DMA test */
      case 'D': if (!openstate) badtry = TRUE;
           else dma_test(fildes);
           break;
      case 'm':         /* Ethernet Multicast test */
      case 'M': if (!openstate) badtry = TRUE;
           else mcaddr += multicast_test(fildes);
           break;
      case 'f':         /* IOCINFO test */
      case 'F': if (!openstate) badtry = TRUE;
           else iocinfo_test(fildes);
           break;
      case 'w':         /* Write Test */
      case 'W': if (!openstate) badtry = TRUE;
           else write_test(fildes);
           break;
      case 'r':         /* Read Test */
      case 'R': if (!openstate) badtry = TRUE;
           else read_test(fildes);
           break;
#ifdef __XTRA_CODE__
      case 'j':
      case 'J': if (!openstate)badtry = TRUE;
           else ent_promisc_on(fildes);
           break;
      case 'k':
      case 'K': if (!openstate)badtry = TRUE;
           else ent_promisc_off(fildes);
           break;
      case 'b':
      case 'B': if (!openstate)badtry = TRUE;
           else ent_bad_frame_on(fildes);
           break;
      case 'n':
      case 'N': if (!openstate)badtry = TRUE;
           else ent_bad_frame_off(fildes);
           break;
      case 'x':
      case 'X': if (!openstate)badtry = TRUE;
           else ent_bad_pkt_read(fildes);
           break;
         case 'u':         /* MIB Query Test */
         case 'U': if (!openstate) badtry = TRUE;
           else ent_mib_query(fildes);
           break;
         case 'y':         /* MIB Get Test */
         case 'Y': if (!openstate) badtry = TRUE;
           else ent_mib_get(fildes);
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
      case '\n':        /* Refresh screen */
           break;
         default:
          invalid = TRUE;     /* Invalid character */
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
   session.length = prompt("\t Number of valid bytes in netid", "%d",
            profile.default_netid_size);
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
   session.length = prompt("\t Number of valid bytes in netid", "%d",
            profile.default_netid_size);
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
      case NOT_READ_VPD:  printf(" VPD_NOT_READ returned.");
                break;
      case NOT_AVAIL_VPD: printf(" VPD_NOT_AVAIL returned.");
                break;
      case INVALID_VPD:   printf(" VPD_INVALID returned.");
                break;
      case VALID_VPD:       printf(" VPD_VALID returned.");
                decode_vpd(vpd.vpd);
                break;
      default:     printf(" Undefined return code %d.",
               vpd.status);
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
   int      result, rc, count, i, waitstat = FALSE;
   int      statusrate, done = FALSE;

   bzero(&ext, sizeof(ext));
   rc = TRUE;
   printf("\n\t **********************   WRITE   ***********************");
   printf("\n");
   result = prompt("\n\tFastwrite (F) or normal write (N)", "%c","N");
   if ( (result == 'F') || (result == 'f') )
   {
      if (!kern)
      {
         printf("\n\tDRIVER NOT IN KERNEL MODE\n");
         return;
      }
      ext.fastwrt_type = NORMAL_CHAIN;
      ext.fastwrt_opt.offlevel = FALSE;

   }
   else
   {
      ext.fastwrt_type = 0;
   }
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
   writlen = MAX( 0, writlen - DATALINK_HDR );
   pattern = fill_buffer(writbuf.data, &writlen, BUFSIZE );
   if (pattern < 0) return(FALSE);
   writlen += DATALINK_HDR;
   count =
       prompt("\t Issue how many writes", "%d", profile.default_writes);
   if (count < 0) return(FALSE);
   sprintf( profile.default_writes, "%d", count );

   if (!ext.fastwrt_type)
   {
      statusrate =
          prompt("\t Wait for transmit status per how many writes",
            "%d", "1");
   }
   else
   {
      statusrate = 0;
   }
   if (statusrate < 0) return(FALSE);
   for (i = 1; i <= count; i++)
   {
       ext.ciowe.flag = 0;    /* clear flags */
       if (!profile.free_mbuf)   /* don't free mbuf? */
           ext.ciowe.flag |= CIO_NOFREE_MBUF;
       ext.ciowe.status   = 0;      /* clear status */
       ext.ciowe.write_id = i;      /* index of transmit */
       if (statusrate)     /* time for status wait? */
      if (waitstat = !(i % statusrate))
          ext.ciowe.flag |= CIO_ACK_TX_DONE;
       result = WRITE( fildes, &writbuf, writlen, &ext );
       if (result < 0) {      /* if error */

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
          rc = FALSE;      /* timed out or error */
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
   int      verify = FALSE, show;
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
   i = prompt("\t Default netid size", "%d", profile.default_netid_size);
   if (i < 0) return;
   sprintf(profile.default_netid_size, "%d", i);
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
          case CIO_START_DONE:   printf("Start Done ,");
               break;
       case CIO_HALT_DONE:         printf("Halt Done ,");
               break;
       case CIO_TX_DONE:           printf("Transmit Done ,");
               break;
       case CIO_ASYNC_STATUS: printf("Unsolicited Status ,");
               break;
       default:              printf("Undefined status code %d ,",
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
      case CIO_TIMEOUT: fprintf(stderr, "Operation timed out");
               break;
      case CIO_NET_RCVRY_ENTER:
            fprintf(stderr, "Enter Network Recovery Mode");
            break;
      case CIO_NET_RCVRY_EXIT:
            fprintf(stderr, "Exit Network Recovery Mode");
            break;
      case CIO_NET_RCVRY_MODE:
            fprintf(stderr, "Network Recovery Mode");
            break;
      case CIO_INV_CMD:
            fprintf(stderr, "Invalid Command");
            break;
      case CIO_BAD_RANGE:
            fprintf(stderr, "Bad Range");
            break;
      case CIO_NETID_INV:
            fprintf(stderr, "Netid Invalid");
            break;
      case CIO_NETID_DUP:
            fprintf(stderr, "Duplicate Netid");
            break;
      case CIO_NETID_FULL:
            fprintf(stderr, "Netid Table Full");
            break;
      case CIO_TX_FULL:
            fprintf(stderr, "Transmit Queue Full");
            break;
      default:
               printf("Undefined: %x",
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
/* compile defaults for this driver.  Null's the entdma file   */
/* descriptor.                   */
/*                            */
/*  RETURNS:  nothing                     */
/*                         */
/*----------------------------------------------------------------------*/

dsopen ()
{
   dmafd = -1;    /* set entdma file desc to "null" */
   profile.default_src[0] = '\0';
   bzero(&dma, sizeof(dma));
   dma.length = DEF_DMA_LENGTH;
   profile.free_mbuf = TRUE;
   profile.funky_mbuf = FALSE;
   profile.poll_reads = TRUE;
   sprintf( profile.mbuf_thresh, "%d", CLBYTES );
   sprintf( profile.mbuf_wdto, "%d", 0 );
   strcpy ( profile.default_dest, DEFAULT_DEST );
   sprintf( profile.default_netid, "%x", DEFAULT_NETID );
   sprintf( profile.default_netid_size, "%d", DEF_NETID_SIZE );
   sprintf( profile.default_size, "%d", DEFAULT_SIZE );
   sprintf( profile.default_writes, "%d", DEFAULT_WRITES );
   sprintf( profile.default_reads, "%d", DEFAULT_READS );
}

/*--------------------------  D S C L O S E  ---------------------------*/
/*                         */
/*  NAME: dsclose                   */
/*                         */
/*  FUNCTION:                       */
/* Closes any left open file descriptors, specifically that of */
/* the ENTDMA driver if used.             */
/*                         */
/*  EXECUTION ENVIRONMENT:                */
/* Can only be executed at the user process level.       */
/*                         */
/*  DATA STRUCTURES:                   */
/* Sets the entdma file descriptor to "null".         */
/*                            */
/*  RETURNS:  nothing                     */
/*                         */
/*----------------------------------------------------------------------*/

dsclose ()
{
   if (!(dmafd < 0)) {
       close(dmafd); /* close entdma if open */
       dmafd = -1;      /* set entdma file desc to "null" */
   }
   if (dma.p_user) { /* free any memory used for dma */
       free(dma.p_user);
       dma.p_user = NULL;
   }
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
       printf("\n\t ");
       printf("\n\t Transmit Byte Count  = %8d", stats.cc.tx_byte_lcnt);
       printf("   CRC Error Count   = %8d", stats.ds.crc_error);
       printf("\n\t Receive Byte Count   = %8d", stats.cc.rx_byte_lcnt);
       printf("   Align Error Count = %8d", stats.ds.align_error);
       printf("\n\t Transmit Frame Count = %8d", stats.cc.tx_frame_lcnt);
       printf("   Recv Overrun Count   = %8d", stats.ds.overrun);
       printf("\n\t Receive Frame Count  = %8d", stats.cc.rx_frame_lcnt);
       printf("   Packets Too Short = %8d", stats.ds.too_short);
       printf("\n\t Transmit Error Count = %8d", stats.cc.tx_err_cnt);
       printf("   Packets Too Long  = %8d", stats.ds.too_long);
       printf("\n\t Receive Error Count  = %8d", stats.cc.rx_err_cnt);
       printf("   No Resources Count   = %8d", stats.ds.no_resources);
       printf("\n\t Max Transmits queued = %8d", stats.cc.xmt_que_high);
       printf("   Recv Pkts Discarded  = %8d", stats.ds.pckts_discard);
       printf("\n\t Max Receives queued  = %8d", stats.cc.rec_que_high);
       printf("   Xmit Collisions   = %8d", stats.ds.max_collision);
       printf("\n\t Max Stat Blks queued = %8d", stats.cc.sta_que_high);
       printf("   Xmit Carrier Lost    = %8d", stats.ds.carrier_lost);
       printf("\n\t Interrupts lost      = %8d", stats.ds.intr_lost);
       printf("   Xmit Underrun Cnt = %8d", stats.ds.underrun);
       printf("\n\t Status lost          = %8d",
            stats.ds.sta_que_overflow);
       printf("   Xmit CTS Lost Cnt = %8d", stats.ds.cts_lost);
       printf("\n\t Receive Packets Lost = %8d",
            stats.ds.rec_que_overflow);
       printf("   Xmit Timeouts  = %8d", stats.ds.xmit_timeouts);
       printf("\n\t No Mbufs Errors      = %8d",
            stats.ds.rec_no_mbuf);
       printf("   Parity Errors  = %8d", stats.ds.par_err_cnt);
       printf("\n\t No Mbuf Ext Errors   = %8d",
            stats.ds.rec_no_mbuf_ext);
       printf("   Execute Q Ovflows = %8d",
            stats.ds.exec_over_flow);
       printf("\n\t Receive Int Count    = %8d",
            stats.ds.recv_intr_cnt);
       printf("   Execute Cmd Errors   = %8d",
            stats.ds.exec_cmd_errors);
       printf("\n\t Transmit Int Count   = %8d",
            stats.ds.xmit_intr_cnt);
       printf("   Host End Of List  = %8d",
            stats.ds.host_rec_eol);
       printf("\n\t Reserved Field 1     =     %.4x",
            stats.ds.reserved[0]);
       printf("   Adapter End Of List  = %8d",
            stats.ds.adpt_rec_eol);
       printf("\n\t Reserved Field 2     =     %.4x",
            stats.ds.reserved[1]);
       printf("   Adap Recv Packets    = %8d",
            stats.ds.adpt_rec_pack);
       printf("\n\t Reserved Field 3     =     %.4x",
            stats.ds.reserved[2]);
       printf("   Host Recv Packets    = %8d",
            stats.ds.host_rec_pack);
       printf("\n\t Reserved Field 4     =     %.4x",
            stats.ds.reserved[3]);
       printf("   Start Recep Cmds  = %8d",
            stats.ds.start_recp_cmd);
       printf("\n\t Reserved Field 5     =     %.4x",
            stats.ds.reserved[4]);
       printf("   Recv DMA Timeouts    = %8d",
            stats.ds.rec_dma_to);
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
      printf("\t  -- Statistics cleared --");
   }
   printf("\n\t ********************************************************");
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
       printf("\n\t -- POS write succeeded --");
}

/*-------------------  A C C E S S _ I O _ T E S T  --------------------*/
/*                         */
/*  NAME: access_io_test                  */
/*                         */
/*  FUNCTION:                       */
/* Selects io read or write tests based on user input.      */
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

access_io_test (fildes)
   int   fildes;
{
   int   mode = 0;

   printf("\n\t *****************  REGISTER I/O ACCESS  ****************");
   while (!mode) {
       mode = prompt("\n\t Read (R) or Write (W)", "%c", "R");
       switch (mode) {
      case 'r':
      case 'R':
          io_read_test(fildes);
          break;
      case 'w':
      case 'W':
          io_write_test(fildes);
          break;
      case '.':
          return;
      default:
          mode = 0;
       }
   }
   printf("\n\t ********************************************************");
}

/*----------------------   I O _ R E A D _ T E S T   -------------------*/
/*                         */
/*  NAME: io_read_test                    */
/*                         */
/*  FUNCTION:                       */
/* Issues the CCC_REG_ACC READ ioctl to the driver under test  */
/* for the prompted for IO register.  If an error occurs, the  */
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

io_read_test (fildes)
   int   fildes;
{
   ccc_reg_acc_t          iodata;
   int         i, queue, result;

   bzero(&iodata, sizeof(iodata));
   queue = y_or_n("\t Read Command/Status Register Queue", TRUE);
   i = prompt("\t Read which register (hex)", "%x", "0");
   if (i < 0) return;
   iodata.io_reg    = i;
   iodata.io_val    = 0;
   iodata.io_val_o  = 0;
   iodata.io_status = 0;
   if (queue)
       iodata.opcode = CCC_READ_Q_OP;
   else
       iodata.opcode = CCC_READ_OP;
   sleep(1);            /* delay between reads */
   result = IOCTL( fildes, CCC_REG_ACC, &iodata );
        if (result < 0) {        /* if error */
       switch (errno) {
      case EIO:
             fprintf(stderr, "\n\t ERROR: ");
             switch (iodata.status) {
              case CCC_NOT_DIAG_MODE:
                fprintf(stderr, "CCC_NOT_DIAG_MODE");
             break;
         case CCC_BAD_RANGE:
             fprintf(stderr, "CCC_BAD_RANGE");
             break;
              case CCC_INV_CMD:
             fprintf(stderr, "CCC_INV_CMD");
             break;
         case CCC_QUE_EMPTY:
             fprintf(stderr, "CCC_QUE_EMPTY");
             break;
              default:
             fprintf(stderr, "Undefined status value %d",
                  iodata.status);
                    break;
             }
          break;
      default:
          fprintf(stderr, "\n\t ");
          decode_error(errno);
          break;
       }
   } else {
       printf("\t -- I/O Register read %d succeeded --", iodata.io_reg);
       printf("\n\t   Read data value = 0x%x", iodata.io_val);
       printf("\n\t   Optional Read data value = 0x%x", iodata.io_val_o);
       printf("\n\t   Optional Read Status value = 0x%x",
      iodata.io_status);
   }
}

/*---------------------  I O _ W R I T E _ T E S T   -------------------*/
/*                         */
/*  NAME: io_write_test                   */
/*                         */
/*  FUNCTION:                       */
/* Issues the CCC_REG_ACC WRITE ioctl to the driver under test    */
/* for the prompted for IO register.  If an error occurs, the  */
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

io_write_test (fildes)
   int   fildes;
{
   ccc_reg_acc_t          iodata;
   int         i, result;

   bzero(&iodata, sizeof(iodata));
   i = prompt("\t Write which register (hex)", "%x", "2");
   if (i < 0) return;
   iodata.io_reg    = i;
   i = prompt("\t Value to write (hex)", "%x", "0");
   if (i < 0) return;
   iodata.io_val    = i;
   iodata.io_status = 0;
   iodata.opcode    = CCC_WRITE_OP;
   iodata.io_val_o  = 0;
   iodata.io_status = 0;
        result = IOCTL( fildes, CCC_REG_ACC, &iodata );
        if (result < 0) {        /* if error */
       switch (errno) {
      case EIO:
               fprintf(stderr, "\n\t ERROR: ");
             switch (iodata.status) {
              case CCC_NOT_DIAG_MODE:
                fprintf(stderr, "CCC_NOT_DIAG_MODE");
             break;
         case CCC_BAD_RANGE:
             fprintf(stderr, "CCC_BAD_RANGE");
             break;
              case CCC_INV_CMD:
             fprintf(stderr, "CCC_INV_CMD");
             break;
         case CCC_QUE_EMPTY:
             fprintf(stderr, "CCC_QUE_EMPTY");
             break;
              default:
             fprintf(stderr, "Undefined status value %d",
                  iodata.status);
                    break;
             }
          break;
      default:
          fprintf(stderr, "\n\t ");
          decode_error(errno);
          break;
       }
   } else
       printf("\t I/O Register write to %d succeeded", iodata.io_reg);
}

/*-------------------  A C C E S S _ R A M _ T E S T  ------------------*/
/*                         */
/*  NAME: access_ram_test                 */
/*                         */
/*  FUNCTION:                       */
/* Selects adapter ram read or write tests based on user input.   */
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

access_ram_test (fildes)
   int   fildes;
{
   int   mode = 0;

   printf("\n\t ******************  ADAPTER RAM ACCESS  ****************");
   while (!mode) {
       mode = prompt("\n\t Read (R) or Write (W)", "%c", "R");
       switch (mode) {
      case 'r':
      case 'R':
          ram_read_test(fildes);
          break;
      case 'w':
      case 'W':
          ram_write_test(fildes);
          break;
      case '.':
          return;
      default:
          mode = 0;
       }
   }
   printf("\n\t ********************************************************");
}

/*----------------------  R A M _ R E A D _ T E S T  -------------------*/
/*                         */
/*  NAME: ram_read_test                   */
/*                         */
/*  FUNCTION:                       */
/* Issues the CCC_MEM_ACC READ ioctl to the driver under test  */
/* for the prompted for ram address.  If an error occurs, the  */
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

ram_read_test (fildes)
   int   fildes;
{
   ccc_mem_acc_t          ramdata;
   unsigned int      length;
   int         i, result = 0;
   char        buffer[RAMSIZE];

   bzero(&ramdata, sizeof(ramdata));
   bzero(buffer, RAMSIZE);
   ramdata.opcode = CCC_READ_OP;
   i = prompt("\t RAM offset (hex)", "%x", "0");
   if (i < 0) return;
   ramdata.ram_offset = i;
   i = 0;
   while (!i) {
       i = prompt("\t Length (bytes) to read", "%d", "1");
       if (i < 0) return;
       if (i > RAMSIZE) {
      printf("\t That's too big!");
      i = 0;
       } else
      length = i;
   }
   ramdata.length = i;
   ramdata.buffer = (unsigned char *)buffer;
   result = IOCTL( fildes, CCC_MEM_ACC, &ramdata );
        if (result < 0) {        /* if error */
       switch (errno) {
      case EIO:
             fprintf(stderr, "\n\t ERROR: ");
             switch (ramdata.status) {
              case CCC_NOT_DIAG_MODE:
                fprintf(stderr, "CCC_NOT_DIAG_MODE");
             break;
         case CCC_BAD_RANGE:
             fprintf(stderr, "CCC_BAD_RANGE");
             break;
              case CCC_INV_CMD:
             fprintf(stderr, "CCC_INV_CMD");
             break;
              default:
             fprintf(stderr, "Undefined status value %d",
                  ramdata.status);
                    break;
             }
          break;
      default:
          fprintf(stderr, "\n\t ");
          decode_error(errno);
          break;
       }
   } else {
       printf("\t Adapter RAM Read Succeeded --");
       if (y_or_n(" Show buffer contents", TRUE))
           print_buffer(buffer, length, ramdata.ram_offset, stdout);
   }
}

/*--------------------  R A M _ W R I T E _ T E S T  -------------------*/
/*                         */
/*  NAME: ram_read_test                   */
/*                         */
/*  FUNCTION:                       */
/* Issues the CCC_MEM_ACC WRITE ioctl to the driver under test    */
/* for the prompted ram address.  If an error occurs, the      */
/* error is decoded and displayed.   The write area can be filled */
/* with a test pattern or filled manually from user input.     */
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

ram_write_test (fildes)
   int   fildes;
{
   ccc_mem_acc_t          ramdata;
   int         i, result = 0;
   char        buffer[RAMSIZE];

   bzero(&ramdata, sizeof(ramdata));
   bzero(buffer, RAMSIZE);
   ramdata.opcode = CCC_WRITE_OP;
   i = prompt("\t RAM offset (hex)", "%x", "0");
   if (i < 0) return;
   ramdata.ram_offset = i;
   i = 0;
   while (!i) {
       i = prompt("\t Length (bytes) to write", "%d", "1");
       if (i < 0) return;
       if (i > RAMSIZE) {
      printf("\t That's too big!");
      i = 0;
       } else
           ramdata.length = i;
   }
   ramdata.buffer = (unsigned char *)buffer;
   if (fill_buffer (buffer, &i, BUFSIZE) < 0) return;
   ramdata.length = i;
   result = IOCTL( fildes, CCC_MEM_ACC, &ramdata );
        if (result < 0) {        /* if error */
       switch (errno) {
      case EIO:
             fprintf(stderr, "\n\t ERROR: ");
             switch (ramdata.status) {
              case CCC_NOT_DIAG_MODE:
                fprintf(stderr, "CCC_NOT_DIAG_MODE");
             break;
         case CCC_BAD_RANGE:
             fprintf(stderr, "CCC_BAD_RANGE");
             break;
              case CCC_INV_CMD:
             fprintf(stderr, "CCC_INV_CMD");
             break;
              default:
             fprintf(stderr, "Undefined status value %d",
                  ramdata.status);
                    break;
             }
          break;
      default:
          fprintf(stderr, "\n\t ");
          decode_error(errno);
          break;
       }
   } else
       printf("\t -- Adapter RAM Write Succeeded --");
}

/*--------------------------  D M A _ T E S T  -------------------------*/
/*                         */
/*  NAME: dma_test                     */
/*                         */
/*  FUNCTION:                       */
/* Issues DMA ioctls to the driver under test (ENT_ALOC_BUF,   */
/* ENT_FREE_BUF, and ENT_COPY_BUF) depending on interactive */
/* input from the user.  Requires use of the entdma driver. */
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

dma_test (fildes)
   int   fildes;
{
   int   i, op = 0, result = 0;
   char  c;

   printf("\n\t ********************  DMA FACILITY  ********************");
   while (!op) {
       printf("\n\t Lock (L) or Unlock (U) DMA buffer? ");
#if 0
       printf("\n\t Lock (L), Unlock(U), Allocate (A), Deallocate (D),");
       printf("\n\t     or Copy (C) DMA buffer? ");
#endif
       c = getinput(); getinput();
       switch (c) {
      case 'l':
      case 'L':
          op = ENT_LOCK_DMA;
          break;
      case 'u':
      case 'U':
          op = ENT_UNLOCK_DMA;
          break;
#if 0
      case 'a':
      case 'A':
          op = ENT_ALOC_BUF;
          break;
      case 'd':
      case 'D':
          op = ENT_FREE_BUF;
          break;
      case 'c':
      case 'C':
          op = ENT_COPY_BUF;
          break;
#endif
      case '.':
          return;
      default:
          break;
       }
   }
   switch (op) {
       case ENT_LOCK_DMA:
      if (dma.p_user) {
          fprintf(stderr,
         "\t You haven't unlocked the previous buffer!");
          goto exit;
      }
      sprintf(str, "%d", dma.length);
      dma.length = prompt("\t Length of buffer to lock", "%d", str);
      dma.p_user = malloc(dma.length);
      dma.p_bus  = 0;
      printf("\t Address of the user buffer is 0x%.8x\n", dma.p_user);
      sprintf(str, "%d", (dma.length >> 1));
      i = prompt("\t Length of test data", "%d", str);
      if (fill_buffer(dma.p_user, &i, BUFSIZE) < 0) return;
      break;

       case ENT_UNLOCK_DMA:
      if (!dma.p_user) {
          fprintf(stderr,
         "\t There is no user buffer to unlock!");
          goto exit;
      }
      break;
   }
   result = IOCTL( fildes, op, &dma );
        if (result < 0) {        /* if error */
       fprintf(stderr, "\n\t ");
       decode_error(errno);      /* free any dma memory: */
       if (dma.p_user && (op == ENT_LOCK_DMA))
       {
           free(dma.p_user);
           dma.p_user = NULL;
       }
   } else {
       printf("\n\t -- DMA Command Succeeded --");
       switch (op) {
      case ENT_UNLOCK_DMA:
          if (y_or_n("\n\t Show user buffer", FALSE))
              print_buffer(dma.p_user, dma.length, 0, stdout);
          free(dma.p_user);
          dma.p_user = NULL;
          break;
#if 0
      case ENT_COPY_BUF:
          printf("\n\t   Length of transfer (bytes)   = %d",
         dma.length);
          if (c == 'T')
              if (y_or_n("\t Show user buffer", TRUE))
                  print_buffer(dma.p_user, dma.length, 0, stdout);
          free(dma.p_user);
          dma.p_user = NULL;
          break;
      case ENT_FREE_BUF:
          dma.p_user = NULL;
          break;
#endif
       }
   }
    exit:
   printf("\n\t ********************************************************");
}

/*--------------------  M U L T I C A S T _ T E S T  -------------------*/
/*                         */
/*  NAME: multicast_test                  */
/*                         */
/*  FUNCTION:                       */
/* Issues the ENT_SET_MULTI ioctl to the driver under test (via   */
/* the file descriptor "fildes"); with either ENT_ADD or ENT_DEL  */
/*    (add or delete) depending on user selection.  The multicast */
/* address to be added or deleted is prompted for.       */
/*                         */
/*  EXECUTION ENVIRONMENT:                */
/* Can only be executed at the user process level.       */
/*                         */
/*  DATA STRUCTURES:                   */
/* References no global data structures.           */
/*                            */
/*  RETURNS:                        */
/* 1  when a multicast address is added.           */
/*     -1  when a multicast address is deleted.          */
/* 0  if an error occurs.                 */
/*                         */
/*----------------------------------------------------------------------*/

multicast_test (fildes)
   int   fildes;
{
   ent_set_multi_t      set;
   char  c = 0;
   int   result, rc, done = 0;

   printf("\n\t ************  ADD/DELETE MULTICAST ADDRESS  ************");
   printf("\n");
   bzero(&set, sizeof(set));
   while (!c) {
       c = prompt("\t Add (A) or Delete (D) a multicast address", "%c",
         "A");
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
   while (!done) {
       printf("\t Enter Multicast Address (6 bytes hex): ");
       scanf("%s", str);
       done = (strlen(str) == 12) &&
           hexstring_to_array(str, set.multi_addr);
   }
   getinput();
   if (c == 'A')
        set.opcode = ENT_ADD;
   else set.opcode = ENT_DEL;
   result = IOCTL( fildes, ENT_SET_MULTI, &set );
   if (result < 0) {       /* if error */
       switch (errno) {
      case EINVAL:
          if (c == 'A')
            fprintf(stderr, "ERROR: Multicast list is full.");
          else
         fprintf(stderr, "ERROR: Multicast list is empty.");
          break;
      default:
             fprintf(stderr, "\n\t ");
          decode_error(errno);
          break;
       }
       rc = FALSE;
   } else {
       if (c == 'A') {
          fprintf(stderr, "\n\t -- Add Multicast Address succeeded --");
          rc = 1;          /* added an address */
       } else {
          fprintf(stderr, "\n\t -- Delete Multicast Address succeeded --");
          rc = -1;            /* deleted an address */
       }
   }
   printf("\n\t ********************************************************");
   return(rc);
}

/*----------------------  I O C I N F O _ T E S T  ---------------------*/
/*                         */
/*  NAME: iocinfo_test                    */
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

iocinfo_test (fildes)
   int   fildes;
{
   int      result, i;
   struct devinfo info;

   printf("\n\t **********************  IOCINFO  ***********************");
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
       if (info.devsubtype == DD_EN)
       printf("DD_EN");
       else printf("unknown - %d", info.devsubtype);

       printf("\n\t    Broadcast Packet Wrap Support: ");
       switch (info.un.ethernet.broad_wrap) {
      case TRUE:  printf("Supported");
               break;
      case FALSE:    printf("Not Supported");
               break;
      default: printf("unknown: %d",
               info.un.ethernet.broad_wrap);
            break;
       }
       printf("\n\t    Receive data xfer offset = %d",
               info.un.ethernet.rdto);
       printf("\n\t    Ethernet Address in use  = 0x");
       for (i = 0; i < 6; i++)
      printf("%.2x", info.un.ethernet.net_addr[i]);
       printf("\n\t    Adapter Ethernet Address = 0x");
       for (i = 0; i < 6; i++)
      printf("%.2x", info.un.ethernet.haddr[i]);
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
   char     c;
   uchar    fastwrt; /* True for fastwrite */

   ttp   = (TP        *)xmitbuf.data;
   rtp   = (TP        *)recvbuf.data;
   tarp  = (TP_ARP    *)ttp->data;
   tparm = (TP_PARMS  *)ttp->data;
   rarp  = (TP_ARP    *)rtp->data;
   rparm = (TP_PARMS  *)rtp->data;
   rres  = (TP_RESULT *)rtp->data;
   xmit_frames = recv_frames = 0;

   printf("\n\t ***********************  CLIENT  ***********************");
   count = prompt("\n\t Send how many test frames", "%d", "1000000");
   if (count < 0) return;

   fastwrt = FALSE;
   if (kern)
   {
      fastwrt = y_or_n("\tSend through fastwrite", FALSE);
   }
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
   if (!Attach_Protocol(fildes, TEST_PROTOCOL))
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
   printf("\n\t Issuing ARP request to the Server . . .");
   while (!Arp_Request(fildes))
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
   xmitbuf.netid = TEST_PROTOCOL;      /* netid */

   /*  The ARP handshake completed -- now a test parameters frame */
   /*  is constructed and sent to configure the Server.     */

   strcpy(ttp->src,  CLIENT_PA);    /* My protocol address */
   strcpy(ttp->dest, SERVER_PA);    /* his protocol address */
   ttp->seq_number       = 0;    /* sequence number */
   ttp->operation      = TEST_PARMS;   /* sending test parameters */
   ttp->length         = sizeof(TP_PARMS);   /* length of data */
   tparm->ack_spacing  = ack_spacing;  /* acks per # transmits */
   tparm->pattern_type = pattern_type; /* type of test pattern */
   tparm->max_errors   = max_errors;   /* max error count */

   printf("\n\t Sending Test Parameters to the Server . . .");
   while ( errors < max_errors ) {
       if (!Transmit(fildes, &xmitbuf, PARM_SIZE, FALSE)) /* send packet */
           return;            /* quit if error */

       /*  The test parameters packet is sent.  The Server reponds  */
       /*  by returning an ack packet with the starting ack number. */

       if ((ack_no = Wait_Ack(fildes)) < 0) /* get next ack (timeout?) */
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
   ttp->operation = TEST_PATTERN;      /* sending test parameters */
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
           xmit = FALSE;
           if ( startstop )         /* restart device? */
           {
               Detach_Protocol(fildes, TEST_PROTOCOL);
               Attach_Protocol(fildes, TEST_PROTOCOL);
           }
           for (i = j; i < (j + burst_size); i++)
           {
               if (random)
              length = lower_limit
             + RANDNUM(upper_limit - lower_limit);
               generate_pattern(ttp->data, length, pattern_type);
               ttp->length = length;
               ttp->seq_number = i;
          xmit = Transmit(fildes, &xmitbuf, (TP_HDR_SIZE + length));
          if (!xmit)
              return;
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
               Send_Halt(fildes, Server_Addr);
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
      ++lost_acks;
           if ((++errors >= max_errors) || recv_timeout)
      {           /* second recv timeout? */
          fprintf(stderr, "\n\t Lost contact with Server!");
          break;
      } else {       /* else, first timeout */
          recv_timeout = TRUE;   /* so retry */
          continue;        /* next transmit */
      }
       } else
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
                 Send_Halt(fildes, Server_Addr);
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
               Send_Halt(fildes, Server_Addr);
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
       if (((lost_acks + bad_frames)    < max_errors) &&
      ((lost_frames + sbad_frames) < max_errors))
           printf("\n\t Tests completed successfully with errors:");
       else
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

Arp_Request (fildes)
   int      fildes;
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
                  /* Fill in packet: */
   strcpy(ttp->src,  CLIENT_PA);    /* My protocol address */
   strcpy(ttp->dest, SERVER_PA);    /* his protocol address */
   ttp->operation  = TEST_ARP;      /* type of test packet */
   ttp->length     = sizeof(TP_ARP);   /* size after header */
   ttp->seq_number = 0;       /* sequence number */
   tarp->type      = ARP_REQUEST;      /* request hw address */
   xmitbuf.netid  = TEST_PROTOCOL;     /* Datalink header: */
   for (i = 0; i < HLEN; i++)       /* fill in hardware addr's */
   {
       tarp->sender[i] = Client_Addr[i];  /* mine */
       xmitbuf.src[i]  = Client_Addr[i];
       tarp->target[i] = 0;           /* his (unknown) */
       xmitbuf.dest[i] = 0xFF;      /* broadcast */
   }              /* send it via write */
   if (!Transmit( fildes, &xmitbuf, ARP_SIZE ))
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
      printf("\n\t The Server's address is ");
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
   while (TRUE) {
       bzero( &recvbuf, sizeof(recvbuf));
       length = Receive(fildes, &recvbuf, sizeof(recvbuf), CLIENT_TIMEOUT);
       if (length < 0) return (-1);    /* error */

       /*  For LAN client/server tests, the ack will return a  */
       /*  sequence number:               */

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

Send_Halt (fildes, net_addr)
   int      fildes;
   unsigned char  *net_addr;
{
   XMIT_FRAME  xmitbuf;
   TP    *tp;
   int      i;

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
   xmitbuf.netid = TEST_PROTOCOL;
   return (Transmit( fildes, &xmitbuf, TP_HDR_SIZE ));
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

   ttp   = (TP        *)xmitbuf.data;
   rtp   = (TP        *)recvbuf.data;
   tarp  = (TP_ARP    *)ttp->data;
   tparm = (TP_PARMS  *)ttp->data;
   tres  = (TP_RESULT *)ttp->data;
   rarp  = (TP_ARP    *)rtp->data;
   rparm = (TP_PARMS  *)rtp->data;
   xmit_frames = recv_frames = 0;

   printf("\n\t ***********************  SERVER  ***********************");

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
   if (!Attach_Protocol(fildes, TEST_PROTOCOL))
       return;

   /*  Receive processing loop; if a receive timeout occurs,   */
   /*  a retry ack is issued to break any deadlocks.  If the next */
   /*  receive times out, the Client is assumed dead and the test */
   /*  is aborted.                     */

   printf("\n\t Waiting for a request from the Client . . .");
   while (TRUE) {
       bzero(&recvbuf, sizeof(recvbuf));
       length = Receive(fildes, &recvbuf, sizeof(recvbuf), timeout);
       if (length < 0)     /* receive timeout? */
       {          /* if so, */
      if (deadlock_ack) {  /* did we issue a deadlock ack? */
          fprintf(stderr, "\n\t Lost contact with Client!");
          lost_frames++; errors++;
          goto exit;    /* then give up */
      } else {    /* else, issue a deadlock ack */
          if (!Send_Ack(fildes, Client_Addr, ++ack_no))
              return;
          deadlock_ack = TRUE;
          continue;     /* and wait once more */
      }
       } else
      deadlock_ack = FALSE;         /* we got something */
       if (!strcmp(rtp->dest, SERVER_PA)) {  /* for me? */
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
         printf("\n\t Issuing ARP reply to the Client . . .");
                     /* Fill in packet: */
         strcpy(ttp->src,  SERVER_PA); /* My prot address */
         strcpy(ttp->dest, CLIENT_PA); /* his prot address */
         ttp->operation = TEST_ARP; /* type of test pkt */
         ttp->length    = sizeof(TP_ARP);/* size after header */
         ttp->seq_number = 0;    /* null seq number */
         tarp->type     = ARP_REPLY;   /* return hw address */
         xmitbuf.netid = TEST_PROTOCOL;
         for (i = 0; i < HLEN; i++)    /* fill in hw addr's */
         {
             tarp->sender[i] = Server_Addr[i];  /* mine */
             tarp->target[i] = Client_Addr[i];  /* his */
             xmitbuf.src[i]  = Server_Addr[i];
             xmitbuf.dest[i] = Client_Addr[i];
         }           /* send it via write */
         ignore_arps = TRUE;     /* lock out ARPS */
         if (!Transmit( fildes, &xmitbuf, ARP_SIZE ))
             return;       /* quit if error */
         break;

          /*  Test parameters from the Client -- set our    */
          /*  local variables accordingly and return an ack.   */

          case TEST_PARMS:       /* test parameters */
         printf("\n\t Test Parameters Packet received");
         ack_spacing  = rparm->ack_spacing;
         pattern_type = rparm->pattern_type;
         max_errors   = rparm->max_errors;
         timeout = SERVER_TIMEOUT;  /* shorter timeout */
         j = 0;            /* reset counter */
         printf("\n\t Sending ACK to the Client");
         if (!Send_Ack(fildes, Client_Addr, ++ack_no))
             return;       /* send ack */
         bad_frame = FALSE;
         printf("\n\n\t --  Test in Progress  --");
         break;

          /*  Test data packet from the Client:    */

          case TEST_PATTERN:

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
                 if (!Send_Ack(fildes, Client_Addr, ++ack_no))
                return;
         break;

          /*  Halt request from the Client; exit the test.  */

          case TEST_HALT:
         printf("\n\t Halt frame received. ");
         goto exit;

          default:
         fprintf(stderr,
            "\n\t Unknown Test Operation received: %d",
            rtp->operation);
         break;
      }
       } else {               /* corrupted frame */
      fprintf(stderr, "\n\t Corrupted Frame received:\n");
      print_buffer( &recvbuf, length, 0, stderr );
      save_bad_frame(&recvbuf, length,
         RANDOM_PATTERN, ++bad_frames, TP_HDR_SIZE, Sfile);
      bad_frame = TRUE;
      if (++errors >= max_errors)
          break;
       }
   }
    exit:
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

   printf("\n\t Sending test results to Client . . .");
   if (!Transmit(fildes, &xmitbuf, RESULT_SIZE))
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

Send_Ack (fildes, net_addr, ack_number)
   int      fildes;
   unsigned char  *net_addr;
   int      ack_number;
{
   XMIT_FRAME  xmitbuf;
   TP    *tp;
   int      i;

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
   xmitbuf.netid = TEST_PROTOCOL;
   return (Transmit( fildes, &xmitbuf, TP_HDR_SIZE ));
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

Transmit (fildes, packet, length, fastwrt_flag)
   int      fildes;
   char     *packet;
   unsigned int   length;
   uchar    fastwrt_flag;
{
   WRITE_EXT   ext;
   int      retry = 0, result;

   length = MAX( MIN_PACKET, length ); /* normalize length */
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
   {
      ext.fastwrt_type = 0;
   }
   while (TRUE) {
       result = WRITE( fildes, packet, length, &ext );
       if (result < 0) {
           switch (errno) {
               case EIO:
                 fprintf(stderr, "\n\t ERROR during write: see status");
         return(FALSE);
          case EAGAIN:
              if (retry++ < WRITE_RETRIES)
             break;
               default:
              fprintf(stderr, "\n\t Write ");
                   decode_error(errno);
         return(FALSE);
           }
       } else {
           if (ext.ciowe.flag & CIO_ACK_TX_DONE)
          if (!wait_status(fildes, CIO_TX_DONE, STATUS_TIMEOUT))
         return(FALSE);    /* timed out or error */
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
   struct pollfd  pollblk;
   struct read_extension   ext;

   bzero( &ext, sizeof(ext));

   /*  Wait for receive data by polling for it:    */

   pollblk.fd = fildes;
   pollblk.reqevents = POLLIN;
   pollblk.rtnevents = 0;
   if (poll( &pollblk, 1, timeout ) <= 0)
       result = -1;
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
   session.length = 2;
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
   session.length = 2;
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

#ifdef __XTRA_CODE__
/*                         */
/*  NAME: ent_promisc_on                  */
/*                         */
/*  FUNCTION:                       */
/* Issues an ioctl ENT_PROMISCUOUS_ON to the driver under test    */
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

ent_promisc_on (fildes)
   int   fildes;
{
   int   result, rc = TRUE;

   printf("\n\t ******************  PROMISCUSOUS ON  *******************");
   result = IOCTL( fildes, ENT_PROMISCUOUS_ON, NULL);
   if (result < 0) {       /* if error */
       fprintf(stderr, "\n\t ");
       decode_error(errno);
       rc = FALSE;
   } else {
       printf("\n\t -- IOCTL ENT_PROMISCUOUS_ON succeeded --");
   }
   printf("\n\t ********************************************************");
   return(rc);
}

/*                         */
/*  NAME: ent_promisc_off                 */
/*                         */
/*  FUNCTION:                       */
/* Issues an ioctl ENT_PROMISCUOUS_OFF to the driver under test   */
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

ent_promisc_off (fildes)
   int   fildes;
{
   int   result, rc = TRUE;

   printf("\n\t ******************  PROMISCUSOUS OFF  ******************");
   result = IOCTL( fildes, ENT_PROMISCUOUS_OFF, NULL);
   if (result < 0) {       /* if error */
       fprintf(stderr, "\n\t ");
       decode_error(errno);
       rc = FALSE;
   } else {
       printf("\n\t -- IOCTL ENT_PROMISCUOUS_OFF succeeded --");
   }
   printf("\n\t ********************************************************");
   return(rc);
}

/*                         */
/*  NAME: ent_bad_frame_on                */
/*                         */
/*  FUNCTION:                       */
/* Issues an ioctl ENT_BAD_FRAME_ON to the driver under test   */
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

ent_bad_frame_on (fildes)
   int   fildes;
{
   int   result, rc = TRUE;

   printf("\n\t ******************  BAD FRAME ON ***********************");
   result = IOCTL( fildes, ENT_BADFRAME_ON, NULL);
   if (result < 0) {       /* if error */
       fprintf(stderr, "\n\t ");
       decode_error(errno);
       rc = FALSE;
   } else {
       printf("\n\t -- IOCTL ENT_BADFRAME_ON succeeded --");
   }
   printf("\n\t ********************************************************");
   return(rc);
}

/*                         */
/*  NAME: ent_bad_frame_off                  */
/*                         */
/*  FUNCTION:                       */
/* Issues an ioctl ENT_BADFRAME_OFF to the driver under test   */
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

ent_bad_frame_off (fildes)
   int   fildes;
{
   int   result, rc = TRUE;

   printf("\n\t ******************  BAD FRAME OFF **********************");
   result = IOCTL( fildes, ENT_BADFRAME_OFF, NULL);
   if (result < 0) {       /* if error */
       fprintf(stderr, "\n\t ");
       decode_error(errno);
       rc = FALSE;
   } else {
       printf("\n\t -- IOCTL ENT_BADFRAME_OFF succeeded --");
   }
   printf("\n\t ********************************************************");
   return(rc);
}

/*                         */
/*  NAME: ent_bad_pkt_read                */
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

ent_bad_pkt_read (fildes)
   int   fildes;
{
   etkid_bad_read_t  badkif;
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
/*  NAME: ent_mib_query                   */
/*                         */
/*  FUNCTION:                       */
/* Issues the ioctl ENT_MIB_QUERY  to the driver under test    */
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

ent_mib_query (fildes)
   int   fildes;
{
   int   i, result;
   ethernet_all_mib_t   mibs;

   printf("\n\t ******************* MIB QUERY *******************");
   bzero(&mibs, sizeof(mibs));

   result = IOCTL( fildes, ENT_MIB_QUERY, &mibs );
   if (result < 0)   {        /* if error */
       fprintf(stderr, "\n\t ");
       decode_error(errno);
   } else {
       printf("\n\tIOCTL ENT_MIB_QUERY succeeded:");
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
       printf("\n\t Ethernet MIBs");
       printf("\n");
       printf("\n\t Align Errors         = %4d",
         mibs.Ethernet_mib.Dot3StatsEntry.align_errs);
       printf("   FCS Errors               = %4d",
         mibs.Ethernet_mib.Dot3StatsEntry.fcs_errs);
       printf("\n\t Single Col. Frames   = %4d",
         mibs.Ethernet_mib.Dot3StatsEntry.s_coll_frames);
       printf("   Multiple Col. Frames     = %4d",
         mibs.Ethernet_mib.Dot3StatsEntry.m_coll_frames);
       printf("\n\t SQE Test Errors      = %4d",
         mibs.Ethernet_mib.Dot3StatsEntry.sqetest_errs);
       printf("   Deferred Transmission    = %4d",
         mibs.Ethernet_mib.Dot3StatsEntry.defer_tx);
       printf("\n\t Late Collisions      = %4d",
         mibs.Ethernet_mib.Dot3StatsEntry.late_collisions);
       printf("   Excessive Collisions     = %4d",
         mibs.Ethernet_mib.Dot3StatsEntry.excess_collisions);
       printf("\n\t MAC TX Errors        = %4d",
         mibs.Ethernet_mib.Dot3StatsEntry.mac_tx_errs);
       printf("   Carrier Sense Errors     = %4d",
         mibs.Ethernet_mib.Dot3StatsEntry.carriers_sense);
       printf("\n\t Frames Too Long      = %4d",
         mibs.Ethernet_mib.Dot3StatsEntry.long_frames);
       printf("   MAC RCV Errors           = %4d",
         mibs.Ethernet_mib.Dot3StatsEntry.mac_rx_errs);
       printf("\n\t Collision Count      = %4d",
         mibs.Ethernet_mib.Dot3CollEntry.count[0]);
       printf("   Collision Frequency      = %4d",
         mibs.Ethernet_mib.Dot3CollEntry.freq[0]);
   }
   printf("\n");
   printf("\n\t ********************************************************");
}
/*                         */
/*  NAME: ent_mib_get                     */
/*                         */
/*  FUNCTION:                       */
/* Issues the ioctl ENT_MIB_GET  to the driver under test      */
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

ent_mib_get (fildes)
   int   fildes;
{
   int   i, result;
   ethernet_all_mib_t   mibs;

   printf("\n\t ******************* MIB GET *******************");
   bzero(&mibs, sizeof(mibs));

   result = IOCTL( fildes, ENT_MIB_GET, &mibs );
   if (result < 0)   {        /* if error */
       fprintf(stderr, "\n\t ");
       decode_error(errno);
   } else {
       printf("\n\tIOCTL ENT_MIB_GET succeeded:");
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
       printf("\n\t Ethernet MIBs");
       printf("\n");
       printf("\n\t Align Errors         = %8d",
         mibs.Ethernet_mib.Dot3StatsEntry.align_errs);
       printf("   FCS Errors           = %8d",
         mibs.Ethernet_mib.Dot3StatsEntry.fcs_errs);
       printf("\n\t Single Col. Frames   = %8d",
         mibs.Ethernet_mib.Dot3StatsEntry.s_coll_frames);
       printf("   Multiple Col. Frames = %8d",
         mibs.Ethernet_mib.Dot3StatsEntry.m_coll_frames);
       printf("\n\t SQE Test Errors      = %8d",
         mibs.Ethernet_mib.Dot3StatsEntry.sqetest_errs);
       printf("   Deferred Transmission= %8d",
         mibs.Ethernet_mib.Dot3StatsEntry.defer_tx);
       printf("\n\t Late Collisions      = %8d",
         mibs.Ethernet_mib.Dot3StatsEntry.late_collisions);
       printf("   Excessive Collisions = %8d",
         mibs.Ethernet_mib.Dot3StatsEntry.excess_collisions);
       printf("\n\t MAC TX Errors        = %8d",
         mibs.Ethernet_mib.Dot3StatsEntry.mac_tx_errs);
       printf("   Carrier Sense Errors = %8d",
         mibs.Ethernet_mib.Dot3StatsEntry.carriers_sense);
       printf("\n\t Frames Too Long      = %8d",
         mibs.Ethernet_mib.Dot3StatsEntry.long_frames);
       printf("   MAC RCV Errors       = %8d",
         mibs.Ethernet_mib.Dot3StatsEntry.mac_rx_errs);

       printf("\n\n\t                     1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16");

       printf("\n\t Collision Count     ");
       for (i = 0; i < 16; i++)
      printf("%.2x ", mibs.Ethernet_mib.Dot3CollEntry.count[i]);

       printf("\n\t Collision Frequency ");
       for (i = 0; i < 16; i++)
      printf("%.2x ", mibs.Ethernet_mib.Dot3CollEntry.freq[i]);
   }
   printf("\n");
   printf("\n\t ********************************************************");
}
#endif /* __XTRA_CODE__ */
