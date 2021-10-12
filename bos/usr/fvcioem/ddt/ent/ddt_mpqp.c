static char sccsid[]="@(#)02   1.1  src/bos/usr/fvcioem/ddt/ent/ddt_mpqp.c, fvcioem, bos411, 9428A410j 4/26/94 13:54:32";

/*--------------------------------------------------------------------------
*
*             DDT_MPQP.C
*
*  COMPONENT_NAME:  Communications Device Driver Tool MPQP Code.
*
*  FUNCTIONS:  main, start_test, halt_test, statistics_test, write_test,
*     read_test, edit_defaults, decode_status, dsopen, dsclose,
*     test_pattern, modify_start_parms, auto_response, ar_start,
*     ar_halt, chg_parms, download_asw, adapter_command,
*     bus_memory, examine_data_structs, examine_pcb,
*     examine_cmd_blocks, decode_bsc_msg, show_readext_status,
*     autodial, Client, Arp_Request, Wait_Ack, Send_Halt,
*     Server, Send_Ack, Transmit, Receive, Attach_Protocol,
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
# include <sys/devinfo.h>
# include <sys/param.h>
# include <sys/poll.h>
# include <sys/uio.h>
# include <sys/comio.h>
# include <sys/dump.h>
# include <sys/mpqp.h>
# include <sys/mpqpdd.h>
# include <sys/mpqpdiag.h>
# include "etkiduser.h"
# include "ddt_com.h"
# include "ddt_mpqp.h"
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
static t_start_dev   startdev;
static char    wrkbuf[ DATASIZE + 2 ]; /* work buffer */
static unsigned char *p_xlat;

static unsigned char asc_tbl[] = { 0x01, 0x02, 0x1F, 0x17, 0x03, 0x10, 0x05,
               0x04, 0x15, 0x16, 0x3C, 0x30, 0x31, 0x3B };
static unsigned char ebc_tbl[] = { 0x01, 0x02, 0x1F, 0x26, 0x03, 0x10, 0x2D,
               0x37, 0x3D, 0x32, 0x7C, 0x70, 0x61, 0x6B };
static unsigned char phys_link_prev;
static unsigned char ascii_dial = FALSE;
static unsigned char asc_chr_tbl[] =
   {   0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
       0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
       0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
       0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
       0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
       0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
       0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
       0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
       0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
       0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
       0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
       0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
       0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
       0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
       0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
       0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f     };
static unsigned char ebc_chr_tbl[] =
   {   0x00, 0x01, 0x02, 0x03, 0x37, 0x2d, 0x2e, 0x2f,
       0x16, 0x05, 0x25, 0x0b, 0x0c, 0x15, 0x0e, 0x0f,
       0x10, 0x00, 0x12, 0x00, 0x3c, 0x3d, 0x32, 0x26,
       0x18, 0x19, 0x3f, 0x27, 0x1c, 0x1d, 0x1e, 0x1f,
       0x40, 0x5a, 0x7f, 0x7b, 0x5b, 0x6c, 0x50, 0x7d,
       0x4d, 0x5d, 0x5c, 0x4e, 0x6b, 0x60, 0x4b, 0x61,
       0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7,
       0xf8, 0xf9, 0x7a, 0x5e, 0x4c, 0x7e, 0x6e, 0x6f,
       0x7c, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7,
       0xc8, 0xc9, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6,
       0xd7, 0xd8, 0xd9, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6,
       0xe7, 0xe8, 0xe9, 0xad, 0xe0, 0x00, 0x5f, 0x6d,
       0x79, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
       0x88, 0x89, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96,
       0x97, 0x98, 0x99, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6,
       0xa7, 0xa8, 0xa9, 0xc0, 0x4f, 0xd0, 0xa1, 0xff     };


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

main( )
{
   int   fildes, mcaddr = 0, gaddr = 0, faddr = 0;
   char  test = 0, invalid = FALSE, badtry = FALSE;
   char  suspend = 0, trace = 0;

   autoresponse = 0;
   openstate = FALSE;
   started = 0;
   fildes = -1;
   dsopen();            /* do any initializations */
   writlen = DEFAULT_SIZE;
   while (TRUE) {
       printf("\n\n\t\t  Multiprotocol Quad Port Device Driver Tool");
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

       printf("G)   Get Status");
       if (autoresponse)
           printf("\n\t A) * Auto Response  P)   Change Parms      ");
       else
           printf("\n\t A)   Auto Response  P)   Change Parms      ");
       printf("N)   Adapter Commands");
       printf("\n\t D)   Download ASW   M)   Modify Start Blk  ");
       printf("E)   Edit Defaults");
       printf("\n\t B)   Bus Memory     X)   Examine Adapter   ");
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
      case 'a':
      case 'A': if (!openstate) badtry = TRUE;
           else autoresponse = auto_response(fildes);
           break;
      case 'm':
      case 'M': modify_start_parms(fildes);
           break;
      case 'd':
      case 'D': if (!openstate) badtry = TRUE;
           else download_asw(fildes);
           break;
      case 'p':
      case 'P': if (!openstate) badtry = TRUE;
           else chg_parms(fildes);
           break;
      case 'b':
      case 'B': if (!openstate) badtry = TRUE;
           else bus_memory(fildes);
           break;
      case 'n':
      case 'N': if (!openstate) badtry = TRUE;
           else adapter_command(fildes);
           break;
      case 'x':
      case 'X': if (!openstate) badtry = TRUE;
           else examine_data_structs(fildes);
           break;
      case 'w':         /* Write Test */
      case 'W': if (!openstate) badtry = TRUE;
           else write_test(fildes);
           break;
      case 'r':         /* Read Test */
      case 'R': if (!openstate) badtry = TRUE;
           else read_test(fildes);
           break;
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
/* information.                     */
/*                            */
/*  RETURNS:                              */
/* TRUE if the start succeeded.              */
/* FALSE if the start failed.             */
/*                         */
/*----------------------------------------------------------------------*/

start_test (fildes)
   int   fildes;
{
   int   result, netid, rc = TRUE;
   int   time_amt;

   printf("\n\t ********************  START PORT  **********************");
   result = prompt("\n\t Use what netid (hex)", "%x",
                  profile.default_netid);
   if (result < 0) return(FALSE);
   startdev.mpqp_session.netid = result;
   result = IOCTL( fildes, CIO_START, &startdev );
   if (result < 0) {       /* if error */
       switch (errno) {
      case EIO:
             fprintf(stderr, "\n\t ERROR status returned -- ");
          show_session_blk( &startdev.mpqp_session, stderr );
          break;
      default:
             fprintf(stderr, "\n\t ");
          decode_error(errno);
          break;
       }
       rc = FALSE;
   } else {
       if (rc) {
      time_amt = prompt("\t Wait how many seconds for status",
         "%d", "30");
      /* time_amt should be in milliseconds */
      time_amt *= 1000;
      if (!time_amt)
          rc = TRUE;
      else
          rc = wait_status( fildes, CIO_START_DONE, time_amt );
       }
       if (rc) {
           printf("\n\t -- IOCTL Start succeeded --");
      show_session_blk( &startdev.mpqp_session, stdout );
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
   STATISTICS     parms;
   t_cio_stats    stats;

   printf("\n\t ******************* QUERY STATISTICS *******************");
   bzero( &parms, sizeof(parms));
   bzero( &stats, sizeof(stats));
   parms.bufptr = (t_cio_stats *)&stats;
   parms.buflen = sizeof(stats);
   result = IOCTL( fildes, CIO_QUERY, &parms );
   if (result < 0)   {        /* if error */
       fprintf(stderr, "\n\t ");
       decode_error(errno);
   } else {
       printf("\n\t IOCTL Query Statistics succeeded:");
       printf("\n\t   Transmit Byte Count  = %d", stats.tx_byte_lcnt);
       printf("\n\t   Receive Byte Count   = %d", stats.rx_byte_lcnt);
       printf("\n\t   Transmit Frame Count = %d", stats.tx_frame_lcnt);
       printf("\n\t   Receive Frame Count  = %d", stats.rx_frame_lcnt);
       printf("\n\t   Transmit Error Count = %d", stats.tx_err_cnt);
       printf("\n\t   Receive Error Count  = %d", stats.rx_err_cnt);
   }
   printf("\n\t ********************************************************");
}

/*-----------------------   W R I T E _ T E S T   ----------------------*/
/*                         */
/*  NAME: write_test                   */
/*                         */
/*  FUNCTION:                       */
/* Writes a test frame to the open device associated with fildes. */
/* The entire buffer is filled with test data.  Status can be  */
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
   struct   mp_write_extension ext;
   int   result, rc, count, i, waitstat = FALSE;
   int   statusrate, done = FALSE;

   bzero(&ext, sizeof(ext));
   rc = TRUE;
   printf("\n\t **********************   WRITE   ***********************");
   writlen = test_pattern( &writbuf, writlen );
   if (writlen < 0) return(FALSE);
   pattern = USER_INPUT;
   i = y_or_n("\t Transparent mode", TRUE);
   if (i < 0) return(FALSE);
   ext.transparent = i;
   count =
       prompt("\t Issue how many writes", "%d", profile.default_writes);
   if (count < 0) return(FALSE);
   sprintf( profile.default_writes, "%d", count );
   statusrate =
       prompt("\t Wait for transmit status per how many writes",
            "%d", "1");
   if (statusrate < 0) return(FALSE);
   for (i = 1; i <= count; i++)
   {
       ext.cio_write.flag = 0;   /* clear flags */
       if (!profile.free_mbuf)   /* don't free mbuf? */
           ext.cio_write.flag |= CIO_NOFREE_MBUF;
       ext.cio_write.status   = 0;  /* clear status */
       ext.cio_write.write_id = i;  /* index of transmit */
       if (statusrate)     /* time for status wait? */
      if (waitstat = !(i % statusrate))
          ext.cio_write.flag |= CIO_ACK_TX_DONE;
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
   verify = y_or_n("\t Compare data with last write", FALSE);
   if (verify < 0) return(FALSE);
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

      /*  MPQP destroys (by design) the original data of */
      /*  some Bisync frames and returns, instead, a message   */
      /*  type in the read extension.  Decode_bsc_msg    */
      /*  attempts to restore the original frame from the   */
      /*  message type.             */

      if ( startdev.data_proto == DATA_PROTO_BSC )
          result = decode_bsc_msg( ext.status, &readbuf, result );

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

   i = prompt("\t Default netid (hex)", "%x", profile.default_netid);
   if (i < 0) return;
   sprintf(profile.default_netid, "%x", i);
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
          case CIO_START_DONE:   printf("Start Done");
               break;
       case CIO_HALT_DONE:         printf("Halt Done");
               break;
       case CIO_TX_DONE:           printf("Transmit Done");
               break;
       case CIO_ASYNC_STATUS: printf("Unsolicited Status");
               break;
       case MP_RDY_FOR_MAN_DIAL: printf("Ready for Manual Dial");
               break;
       case MP_ERR_THRESHLD_EXC: fprintf(stderr,
                  "Error Threshold Exceeded");
               break;
       case MP_END_OF_AUTO_RESP: printf("End of Auto Response");
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

      case MP_ADAP_NOT_FUNC:  fprintf(stderr,
                     "Adapter Not Functioning");
               break;
      case MP_TX_FAILSAFE_TIMEOUT:
               fprintf(stderr,
                     "Transmit Failsafe Timeout");
               break;
      case MP_DSR_ON_TIMEOUT: fprintf(stderr,
                  "Data-Set-Ready ON Timeout");
               break;
      case MP_DSR_ALRDY_ON:   fprintf(stderr,
                  "Data-Set-Ready Already ON");
               break;
      case MP_X21_RETRIES_EXC:
               fprintf(stderr,
                   "X.21: Retries Exceeded");
               break;
      case MP_X21_TIMEOUT: fprintf(stderr, "X.21: Timeout");
               break;
      case MP_X21_CLEAR:   fprintf(stderr, "X.21: Clear");
               break;
      case MP_RCV_TIMEOUT: fprintf(stderr, "Receive Timeout");
               break;
      case MP_DSR_DROPPED: fprintf(stderr,
                  "Data-Set-Ready Dropped");
               break;
      case MP_TX_UNDERRUN: fprintf(stderr, "Transmit Underrun");
               break;
      case MP_CTS_TIMEOUT: fprintf(stderr,
                  "Clear-to-Send Timeout");
               break;
      case MP_TX_FS_TIMEOUT:  fprintf(stderr,
                     "Transmit Failsafe Timeout");
               break;
      case MP_RX_OVERRUN:  fprintf(stderr, "Receiver Overrun");
               break;
      case MP_RX_ABORT: fprintf(stderr, "Receiver Abort");
               break;
      case MP_RX_FRAME_ERR:   fprintf(stderr, "Receive Frame Error");
               break;
      case MP_RX_PARITY_ERR:  fprintf(stderr, "Receive Parity Error");
               break;
      case MP_RX_BAD_SYNC: fprintf(stderr, "Bad Sync on Receive");
               break;
      case MP_RX_DMA_BFR_ERR: fprintf(stderr,
                  "Receive DMA Buffer Error");
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
/* compile defaults for this driver. Initializes the start     */
/* device block.                                   */
/*                            */
/*  RETURNS:  nothing                     */
/*                         */
/*----------------------------------------------------------------------*/

dsopen ()
{
   /*                      */
   /*  Set start device parameters to default values:    */
   /*                      */
   bzero( &startdev, sizeof(startdev));
   startdev.phys_link = PL_232D;
   startdev.dial_proto = 0;
   startdev.dial_flags = 0;
   startdev.data_proto = DATA_PROTO_BSC;
   startdev.data_flags = 0;
   startdev.modem_flags = 0xC0;
   startdev.poll_addr = 0;
   startdev.select_addr = 0;
   startdev.modem_intr_mask = 0;
   startdev.baud_rate = 0;
   startdev.rcv_timeout = 0;
   startdev.rcv_data_offset = 0;
   startdev.p_err_threshold = NULL;

   profile.free_mbuf = TRUE;
   profile.funky_mbuf = FALSE;
   profile.poll_reads = TRUE;
   profile.default_src[0] = '\0';
   sprintf( profile.mbuf_thresh, "%d", CLBYTES );
   sprintf( profile.mbuf_wdto, "%d", 0 );
   sprintf( profile.default_netid, "%x", DEFAULT_NETID );
   sprintf( profile.default_size, "%d", DEFAULT_SIZE );
   sprintf( profile.default_writes, "%d", DEFAULT_WRITES );
   sprintf( profile.default_reads, "%d", DEFAULT_READS );
}

/*--------------------------  D S C L O S E  ---------------------------*/
/*                         */
/*  NAME: dsclose                   */
/*                         */
/*  FUNCTION:                       */
/* Currently does nothing -- can be used to cleanup after a */
/* close.                        */
/*                         */
/*  EXECUTION ENVIRONMENT:                */
/* Can only be executed at the user process level.       */
/*                         */
/*  DATA STRUCTURES:                   */
/*                            */
/*  RETURNS:  nothing                     */
/*                         */
/*----------------------------------------------------------------------*/

dsclose ()
{
}

/*---------------------  T E S T _ P A T T E R N  ----------------------*/
/*                         */
/*  NAME: test_pattern                    */
/*                         */
/*  FUNCTION:                       */
/* Generates test data patterns for write buffers.       */
/*                         */
/*  EXECUTION ENVIRONMENT:                */
/* Can only be executed at the user process level.       */
/*                         */
/*  DATA STRUCTURES:                   */
/* References the startdev block.               */
/*                            */
/*  RETURNS:  nothing                     */
/*                         */
/*----------------------------------------------------------------------*/

test_pattern (
   caddr_t     buffer,
   int      length )
{
   char     response;

   switch (startdev.data_proto) {
       case DATA_PROTO_BSC:
      if (startdev.data_flags & DATA_FLG_BSC_ASC)
          p_xlat = asc_tbl;
      else
          p_xlat = ebc_tbl;
           printf("\n\t A)  DLE ACK0");
           printf("\n\t B)  DLE ACK1");
      printf("\n\t   C)  DLE WACK");
      printf("\n\t   D)  DLE RVI");
           printf("\n\t E)  EOT");
           printf("\n\t F)  NAK");
           printf("\n\t G)  ENQ");
           printf("\n\t H)  SOH D STX D D D ETX");
           printf("\n\t I)  SOH D STX D D D ITB");
           printf("\n\t J)  SOH D STX D D D ETB");
           printf("\n\t K)  SOH D STX D D D ENQ");
      printf("\n\t   L)  STX D D D ETX");
           printf("\n\t M)  STX D D D ITB");
           printf("\n\t N)  SOH D D D ITB");
           printf("\n\t O)  D D D ETB");
      printf("\n\t   P)  D D D ITB");
      printf("\n\t   R)  D D D ETX");
           printf("\n\t S)  D D D ENQ");
      printf("\n\t   T)  Key in test pattern");
      printf("\n\t   U)  Use last write data");
           response =
          prompt("\n\n\t Send which test pattern", "%c", "L");

      switch (response) {
             case 'a':
          case 'A':
         length = decode_bsc_msg( MP_ACK0, buffer, 0 );
         break;
          case 'b':
          case 'B':
         length = decode_bsc_msg( MP_ACK1, buffer, 0 );
         break;
          case 'c':
          case 'C':
         length = decode_bsc_msg( MP_WACK, buffer, 0 );
         break;
          case 'd':
          case 'D':
         length = decode_bsc_msg( MP_RVI, buffer, 0 );
         break;
          case 'e':
          case 'E':
         length = decode_bsc_msg( MP_EOT, buffer, 0 );
         break;
          case 'f':
          case 'F':
         length = decode_bsc_msg( MP_NAK, buffer, 0 );
         break;
          case 'g':
          case 'G':
         length = decode_bsc_msg( MP_ENQ, buffer, 0 );
         break;
          case 'h':
          case 'H':
         buffer[0] = SOH;
         buffer[1] = 0x42;
         buffer[2] = 0x43;
         buffer[3] = STX;
         if (fill_buffer( &buffer[4], &length, BUFSIZE ) < 0)
             return(-1);
         buffer[length+4] = ETX;
         length += 5;
         break;
          case 'i':
          case 'I':
         buffer[0] = SOH;
         buffer[1] = 0x42;
         buffer[2] = 0x43;
         buffer[3] = STX;
         if (fill_buffer( &buffer[4], &length, BUFSIZE ) < 0)
             return(-1);
         buffer[length+4] = ITB;
         length += 5;
         break;
          case 'j':
          case 'J':
         buffer[0] = SOH;
         buffer[1] = 0x42;
         buffer[2] = 0x43;
         buffer[3] = STX;
         if (fill_buffer( &buffer[4], &length, BUFSIZE ) < 0)
             return(-1);
         buffer[length+4] = ETB;
         length += 5;
         break;
          case 'k':
          case 'K':
         buffer[0] = SOH;
         buffer[1] = 0x42;
         buffer[2] = 0x43;
         buffer[3] = STX;
         if (fill_buffer( &buffer[4], &length, BUFSIZE ) < 0)
             return(-1);
         buffer[length+4] = ENQ;
         length += 5;
         break;
          case 'l':
          case 'L':
         buffer[0] = STX;
         if (fill_buffer( &buffer[1], &length, BUFSIZE ) < 0)
             return(-1);
         buffer[length+1] = ETX;
         length += 2;
         break;
          case 'm':
          case 'M':
         buffer[0] = STX;
         if (fill_buffer( &buffer[1], &length, BUFSIZE ) < 0)
             return(-1);
         buffer[length+1] = ITB;
         length += 2;
         break;
          case 'n':
          case 'N':
         buffer[0] = SOH;
         if (fill_buffer( &buffer[1], &length, BUFSIZE ) < 0)
             return(-1);
         buffer[length+1] = ITB;
         length += 2;
         break;
          case 'o':
          case 'O':
         if (fill_buffer( &buffer[0], &length, BUFSIZE ) < 0)
             return(-1);
         buffer[length] = ETB;
         length++;
         break;
          case 'p':
          case 'P':
         if (fill_buffer( &buffer[0], &length, BUFSIZE ) < 0)
             return(-1);
         buffer[length] = ITB;
         length++;
         break;
          case 'r':
          case 'R':
         if (fill_buffer( &buffer[0], &length, BUFSIZE ) < 0)
             return(-1);
         buffer[length] = ETX;
         length++;
         break;
          case 's':
          case 'S':
         if (fill_buffer( &buffer[0], &length, BUFSIZE ) < 0)
             return(-1);
         buffer[length] = ENQ;
         length++;
         break;
          case 't':
          case 'T':
         if (fill_buffer( buffer, &length, BUFSIZE ) < 0)
             return(-1);
         break;
          case 'u':
          case 'U':
         break;
          default:
         length = -1;
         break;
      }
         break;

       default:
      if (fill_buffer( buffer, &length, BUFSIZE ) < 0)
          return(-1);
      break;
   }
   return( length );
}



/*-----------------  M O D I F Y _ S T A R T _ P A R M S  --------------*/
/*                         */
/*  NAME: modify_start_parms                 */
/*                         */
/*  FUNCTION:                       */
/* Interactively allows the user to edit the contents of the   */
/* startdev block.                                       */
/*                         */
/*  EXECUTION ENVIRONMENT:                */
/* Can only be executed at the user process level.       */
/*                         */
/*  DATA STRUCTURES:                   */
/* Modifies the global startdev block.          */
/*                            */
/*  RETURNS: nothing                      */
/*                         */
/*----------------------------------------------------------------------*/

modify_start_parms (fildes)
   int   fildes;
{
   char  *cptr;
   char  nextin, mode = 0;
 static int use_nrzi=FALSE;
   int   i, inp, flags, stopbits, charbits, setdflgs=0;

        phys_link_prev = startdev.phys_link;
   while (!mode) {
       printf("\n\t Physical Link: RS-232D (D), RS-422A (A), V.35 (V),");
       printf("\n\t      X.21 (X), Hayes Dial (H), or V.25bis (B) [");
       switch (startdev.phys_link) {
      case PL_232D: printf("D");
               break;
      case PL_422A: printf("A");
               break;
      case PL_V35:  printf("V");
               break;
      case PL_X21:  printf("X");
               break;
      case PL_SMART_MODEM: printf("H");
               break;
      case PL_V25:  printf("B");
               break;
      default:      printf("?");
               break;
       }
       printf("]? ");
       mode = getinput();
       if (mode == '\n')
      break;
       else
      getinput();
       switch (mode) {
      case 'd':
      case 'D':
          startdev.phys_link = PL_232D;
          break;
      case 'a':
      case 'A':
          startdev.phys_link = PL_422A;
          break;
      case 'v':
      case 'V':
          startdev.phys_link = PL_V35;
          break;
      case 'x':
      case 'X':
          startdev.phys_link = PL_X21;
          break;
      case 'h':
      case 'H':
          startdev.phys_link = PL_SMART_MODEM;
          startdev.dial_proto = DIAL_PRO_ASYNC;
          startdev.dial_flags |= DIAL_FLG_PAR_EN;
          startdev.dial_flags |= DIAL_FLG_PAR_ODD;
          startdev.dial_flags |= DIAL_FLG_STOP_1;
          startdev.dial_flags |= DIAL_FLG_CHAR_7;
          startdev.modem_flags &= ~MF_LEASED;
          startdev.modem_flags &= ~MF_MANUAL;
          startdev.baud_rate = 2400;
          break;
      case 'b':
      case 'B':
          startdev.phys_link = PL_V25;
          startdev.dial_proto = DIAL_PRO_ASYNC;
          startdev.dial_flags |= DIAL_FLG_PAR_EN;
          startdev.dial_flags |= DIAL_FLG_PAR_ODD;
          startdev.dial_flags |= DIAL_FLG_STOP_1;
          startdev.dial_flags |= DIAL_FLG_CHAR_7;
          startdev.modem_flags &= ~MF_LEASED;
          startdev.modem_flags &= ~MF_MANUAL;
          startdev.baud_rate = 2400;
          break;
      case '.':
          return;
      default:
          mode = 0;
       }
   }
   sprintf(str, "%d", startdev.baud_rate);
   inp = prompt("\t Baud Rate (enter zero for external clock)",
            "%d", str);
   if (inp < 0) return;
   startdev.baud_rate = inp;
   if ((startdev.phys_link == PL_SMART_MODEM) ||
       (startdev.phys_link == PL_V25)) {
       mode = 0;
       while (!mode) {
           printf("\t Dial Protocol: Bisync (B), SDLC (S),");
           printf(" or Async (A) [");
      switch (startdev.dial_proto) {
          case DIAL_PRO_BSC:  printf("Bisync");
                     break;
          case DIAL_PRO_SDLC: printf("SDLC");
               break;
          case DIAL_PRO_ASYNC:printf("Async");
               break;
          default:      printf("None");
               break;
      }
      printf("]? ");
           mode = getinput();
           if (mode == '\n')
          break;
           else
          getinput();
           switch (mode) {
          case 'b':
          case 'B':
              startdev.dial_proto = DIAL_PRO_BSC;
              break;
          case 's':
          case 'S':
              startdev.dial_proto = DIAL_PRO_SDLC;
              break;
          case 'a':
          case 'A':
              startdev.dial_proto = DIAL_PRO_ASYNC;
              break;
          case '.':
         return;
          default:
              mode = 0;
           }
       }
       switch (startdev.dial_proto)
       {
       case DIAL_PRO_BSC:
           i = y_or_n("\t Enable Parity",
          startdev.dial_flags & DIAL_FLG_PAR_EN);
      if (i < 0) return;
      if (i) {
          startdev.dial_flags |= DIAL_FLG_PAR_EN;
          mode = 0;
          while (!mode) {
         printf("\t Even (E) or Odd (O) parity [");
         if (startdev.dial_flags & DIAL_FLG_PAR_ODD)
             printf("O");
         else
             printf("E");
         printf("]? ");
            mode = getinput();
            if (mode == '\n')
                break;
            else
                getinput();
         switch (mode) {
             case 'e':
             case 'E':
            startdev.dial_flags &= ~DIAL_FLG_PAR_ODD;
            break;
             case 'o':
             case 'O':
            startdev.dial_flags |= DIAL_FLG_PAR_ODD;
            break;
             case '.':
            return;
             default:
            mode = 0;
            break;
         }
          }
      } else
          startdev.dial_flags &= ~DIAL_FLG_PAR_EN;
      i = y_or_n("\t ASCII Bisync", ascii_dial);
      if (i < 0) return;
      if (i)
      {
          startdev.dial_flags |= DIAL_FLG_BSC_ASC;
          ascii_dial=TRUE;
      }
      else
      {
          startdev.dial_flags &= ~DIAL_FLG_BSC_ASC;
          ascii_dial=FALSE;
      }
      break;

       case DIAL_PRO_SDLC:
      i = y_or_n("\t Use NRZI",
         startdev.dial_flags & DATA_FLG_NRZI);
      if (i < 0) return;
      if (i)
          startdev.dial_flags |= DATA_FLG_NRZI;
      else
          startdev.dial_flags &= ~DATA_FLG_NRZI;
      i = y_or_n("\t ASCII Dial Data", ascii_dial);
      if (i < 0) return;
      if (i)
          ascii_dial = TRUE;
      else
          ascii_dial = FALSE;
      break;

       case DIAL_PRO_ASYNC:
      setdflgs=y_or_n("\t Set Dial Flags", FALSE);
      if (setdflgs < 0) return;
      if (setdflgs)
      {
          startdev.dial_flags = 0;
          startdev.dial_flags |= DIAL_FLG_PAR_EN;
          startdev.dial_flags |= DIAL_FLG_PAR_ODD;
          startdev.dial_flags |= DIAL_FLG_STOP_1;
          startdev.dial_flags |= DIAL_FLG_CHAR_7;
          i = y_or_n("\t Enable Parity",
         startdev.dial_flags & DIAL_FLG_PAR_EN);
          if (i < 0) return;
          if (i) {
              startdev.dial_flags |= DIAL_FLG_PAR_EN;
              mode = 0;
              while (!mode) {
                  printf("\t Even (E) or Odd (O) parity [");
                  if (startdev.dial_flags & DIAL_FLG_PAR_ODD)
            printf("O");
             else
            printf("E");
             printf("]? ");
             mode = getinput();
             if (mode == '\n')
            break;
             else
            getinput();
             switch (mode) {
            case 'e':
            case 'E':
                startdev.dial_flags &= ~DIAL_FLG_PAR_ODD;
                break;
            case 'o':
            case 'O':
                startdev.dial_flags |= DIAL_FLG_PAR_ODD;
                break;
                 case '.':
                return;
                 default:
                mode = 0;
                break;
             }
         }
          } else
         startdev.dial_flags &= ~DIAL_FLG_PAR_EN;
          mode = 0;
          nextin = 0;
          while (!mode) {
              printf("\t Enter number of stop bits [");
         if (startdev.dial_flags & DIAL_FLG_STOP_1)
             printf("1");
         else if (startdev.dial_flags & DIAL_FLG_STOP_15)
             printf("1.5");
         else if (startdev.dial_flags & DIAL_FLG_STOP_2)
             printf("2");
         else
             printf("0");
         printf("]: ");
                    mode = getinput();
         if (mode == '.') return;
                    if (mode == '\n') {
             stopbits = DIAL_FLG_STOP_1;
                break;
                    } else
                if (getinput() == '.')
                 if ((nextin = getinput()) != '/n')
                getinput();
         switch (mode) {
             case '0':
            stopbits = DIAL_FLG_STOP_0;
            break;
             case '1':
            if (nextin == '5')
                stopbits = DIAL_FLG_STOP_15;
            else
                if (nextin) {
               mode = 0;
               break;
                } else
               stopbits = DIAL_FLG_STOP_1;
            break;
             case '2':
            stopbits = DIAL_FLG_STOP_2;
            break;
             default:
            mode = 0;
            break;
         }
          }
          startdev.dial_flags |= stopbits;
          mode = 0;
          while (!mode) {
              printf("\t Enter number of bits/char [");
         if (startdev.dial_flags & DIAL_FLG_CHAR_6)
             printf("6");
         else if (startdev.dial_flags & DIAL_FLG_CHAR_7)
             printf("7");
         else if (startdev.dial_flags & DIAL_FLG_CHAR_8)
             printf("8");
         else
             printf("5");
         printf("]: ");
                    mode = getinput();
                    if (mode == '\n') {
             charbits = 0;
                break;
                    } else
             getinput();
         switch (mode) {
             case '5':
            charbits = DIAL_FLG_CHAR_5;
            break;
             case '6':
            charbits = DIAL_FLG_CHAR_6;
            break;
             case '7':
            charbits = DIAL_FLG_CHAR_7;
            break;
             case '8':
            charbits = DIAL_FLG_CHAR_8;
            break;
             case '.':
            return;
             default:
            mode = 0;
            break;
         }
          }
          startdev.dial_flags |= charbits;
          break;
      } /* if (setdflgs) */
      else
      {
          startdev.dial_flags = 0;
      }
       } /* end switch */
       if (setdflgs)
       {
           i = y_or_n("\t Continuous Carrier",
          startdev.dial_flags & DIAL_FLG_C_CARR_ON);
           if (i < 0) return;
           if (i)
          startdev.dial_flags |= DIAL_FLG_C_CARR_ON;
           else
          startdev.dial_flags &= ~DIAL_FLG_C_CARR_ON;
           i = y_or_n("\t Wait for CTS to transmit",
          startdev.dial_flags & DIAL_FLG_TX_CTS);
           if (i < 0) return;
           if (i)
          startdev.dial_flags |= DIAL_FLG_TX_CTS;
           else
          startdev.dial_flags &= ~DIAL_FLG_TX_CTS;
       }
   }
   mode = 0;
   while (!mode) {
       printf("\t Data Protocol: Bisync (B) or SDLC Full Duplex (F),");
       printf("\n\t      or SDLC Half Duplex (H) [");
       switch (startdev.data_proto) {
      case DATA_PROTO_BSC: printf("B");
                 break;
      case DATA_PROTO_SDLC_FDX: printf("F");
                 break;
      case DATA_PROTO_SDLC_HDX: printf("H");
                 break;
      default:      printf("None");
                 break;
       }
       printf("]? ");
       mode = getinput();
       if (mode == '\n')
      break;
       else
      getinput();
       switch (mode) {
      case 'b':
      case 'B':
          startdev.data_proto = DATA_PROTO_BSC;
          break;
      case 'f':
      case 'F':
          startdev.data_proto = DATA_PROTO_SDLC_FDX;
          break;
      case 'h':
      case 'H':
          startdev.data_proto = DATA_PROTO_SDLC_HDX;
          break;
      case '.':
          return;
      default:
          mode = 0;
       }
   }
   switch (startdev.data_proto) {
       case DATA_PROTO_BSC:
      i = y_or_n("\t ASCII Bisync",
         startdev.data_flags & DATA_FLG_BSC_ASC);
      if (i < 0) return;
      if (i)
          startdev.data_flags |= DATA_FLG_BSC_ASC;
      else
          startdev.data_flags &= ~DATA_FLG_BSC_ASC;
      break;
       case DATA_PROTO_SDLC_FDX:
       case DATA_PROTO_SDLC_HDX:
      startdev.data_flags = 0;
      use_nrzi = y_or_n("\t Use NRZI", use_nrzi);
      if (use_nrzi < 0) return;
      if (use_nrzi)
          startdev.data_flags |= DATA_FLG_NRZI;
      else
          startdev.data_flags &= ~DATA_FLG_NRZI;
      break;
   }
   mode = 0;
   while (!mode) {
       printf("\t Leased (L) or Switched (S) telephone circuit [");
       if (startdev.modem_flags & MF_LEASED)
      printf("L");
       else
      printf("S");
       printf("]? ");
             mode = getinput();
             if (mode == '\n')
         break;
             else
         getinput();
       switch (mode) {
      case 'l':
      case 'L':
          startdev.modem_flags |= MF_LEASED;
          break;
      case 's':
      case 'S':
          startdev.modem_flags &= ~MF_LEASED;
          break;
      case '.':
          return;
      default:
          mode = 0;
       }
   }
   mode = 0;
   while (!mode) {
       printf("\t Incoming (I) or Outgoing (O) call [");
       if (startdev.modem_flags & MF_CALL)
      printf("O");
       else
      printf("I");
       printf("]? ");
             mode = getinput();
             if (mode == '\n')
         break;
             else
         getinput();
       switch (mode) {
      case 'i':
      case 'I':
          startdev.modem_flags &= ~MF_CALL;
          break;
      case 'o':
      case 'O':
          startdev.modem_flags |= MF_CALL;
          break;
      case '.':
          return;
      default:
          mode = 0;
       }
   }
   mode = 0;
   while (!mode) {
       printf("\t Auto (A) or Manual (M) dial/answer [");
       if (startdev.modem_flags & MF_MANUAL)
      printf("M");
       else
      printf("A");
       printf("]? ");
             mode = getinput();
             if (mode == '\n')
         break;
             else
         getinput();
       switch (mode) {
      case 'a':
      case 'A':
          startdev.modem_flags &= ~MF_MANUAL;
          break;
      case 'm':
      case 'M':
          startdev.modem_flags |= MF_MANUAL;
          break;
      case '.':
          return;
      default:
          mode = 0;
       }
   }
   switch (startdev.modem_flags & MF_MANUAL) {
       case TRUE:
           break;
       case FALSE:
      i=autodial();
      if (i < 0) return;
      break;
        default:
      break;
   }
   i = y_or_n("\t Reset Receive Timer",
         startdev.data_flags & DATA_FLG_RST_TMR );
   if (i < 0) return;
   if (i)
       startdev.data_flags |= DATA_FLG_RST_TMR;
   else
       startdev.data_flags &= ~DATA_FLG_RST_TMR;
   i = y_or_n("\t Address Compare Select",
         startdev.data_flags & DATA_FLG_ADDR_CHK);
   if (i < 0) return;
   if (i)
       startdev.data_flags |= DATA_FLG_ADDR_CHK;
   else
       startdev.data_flags &= ~DATA_FLG_ADDR_CHK;
        i = y_or_n("\t Continuous Carrier",
         startdev.data_flags & DIAL_FLG_C_CARR_ON);
   if (i < 0) return;
   if (i)
       startdev.data_flags |= DIAL_FLG_C_CARR_ON;
   else
       startdev.data_flags &= ~DIAL_FLG_C_CARR_ON;
   i = y_or_n("\t Enable DTR after Ring Indicate",
      startdev.modem_flags & MF_CDSTL_ON);
   if (i < 0) return;
   if (i)
       startdev.modem_flags |= MF_CDSTL_ON;
   else
       startdev.modem_flags &= ~MF_CDSTL_ON;
   i = y_or_n("\t Enable Data Rate Select",
      startdev.modem_flags & MF_DRS_ON);
   if (i < 0) return;
   if (i)
       startdev.modem_flags |= MF_DRS_ON;
   else
       startdev.modem_flags &= ~MF_DRS_ON;
   sprintf(str, "%x", startdev.poll_addr);
   inp = prompt("\t Poll address (hex)", "%x", str);
   if (inp < 0) return;
   startdev.poll_addr = inp;
   if (startdev.data_proto == DATA_PROTO_BSC)
   {
       sprintf(str, "%x", startdev.select_addr);
       inp = prompt("\t Select address (hex)", "%x", str);
       if (inp < 0) return;
       startdev.select_addr = inp;
   }
   startdev.modem_intr_mask = 0;    /* unused */
   sprintf(str, "%d", startdev.rcv_timeout);
   inp = prompt("\t Recv timeout in 100's of milliseconds", "%d", str);
   if (inp < 0) return;
   startdev.rcv_timeout = inp;
   sprintf(str, "%d", startdev.rcv_data_offset);
   inp = prompt("\t Receive Data Offset", "%d", str);
   if (inp < 0) return;
   startdev.rcv_data_offset = inp;
   startdev.p_err_threshold = NULL;
   printf("\n\t - These changes will be loaded on the next start -");
   return;
}

/*--------------------  A U T O _ R E S P O N S E  ---------------------*/
/*                         */
/*  NAME: auto_response                   */
/*                         */
/*  FUNCTION:                       */
/* Selects start or halt auto response based on user input. */
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

auto_response (fildes)
   int   fildes;
{
   int   rc, mode = 0;

   printf("\n\t *********************  AUTO RESPONSE  ******************");
   while (!mode) {
       mode = prompt("\n\t Start (S) or Halt (H) Auto Response",
         "%c", "S");
       if (mode < 0) return;
       switch (mode) {
      case 's':
      case 'S':
          rc = ar_start(fildes);
          break;
      case 'h':
      case 'H':
          rc = !ar_halt(fildes);
          break;
      case '.':
          return;
      default:
          mode = 0;
       }
   }
   printf("\n\t ********************************************************");
   return(rc);
}

t_start_ar  aresp;

/*---------------------------  A R _ S T A R T  ------------------------*/
/*                         */
/*  NAME: ar_start                     */
/*                         */
/*  FUNCTION:                       */
/* Prompts for adapter response parameters and        */
/* issues the MP_START_AR ioctl to the driver under test.      */
/*                         */
/*  EXECUTION ENVIRONMENT:                */
/* Can only be executed at the user process level.       */
/*                         */
/*  DATA STRUCTURES:                   */
/* References no global data structures.           */
/*                            */
/*  RETURNS:                              */
/* TRUE  if the halt succeeded.              */
/* FALSE if the halt failed.              */
/*                         */
/*----------------------------------------------------------------------*/

ar_start (fildes)
   int   fildes;
{
   int   inp, result, rc = TRUE;

   inp = prompt("\t Use what receive timeout (in 100 ms units)",
         "%d", "48");
   if (inp < 0) return;
   aresp.rcv_timer = inp;
   inp = prompt("\t What is the compare address (hex)",
         "%x", "01");
   if (inp < 0) return;
   aresp.tx_rx_addr = inp;
   inp = prompt("\t Use what control byte on transmit frames (hex)",
         "%x", "11");
   if (inp < 0) return;
   aresp.tx_cntl = inp;
   inp = prompt("\t Use what control byte on receive frames (hex)",
         "%x", "11");
   if (inp < 0) return;
   aresp.rx_cntl = inp;

   result = IOCTL( fildes, MP_START_AR, &aresp );
   if (result < 0) {
       fprintf(stderr, "\n\t ");
       decode_error(errno);
       rc = FALSE;
   } else
       printf("\n\t Start Auto-Response succeeded.");
   return(rc);
}

/*--------------------------  A R _ H A L T  ---------------------------*/
/*                         */
/*  NAME: ar_halt                   */
/*                         */
/*  FUNCTION:                       */
/* Issues the MP_STOP_AR ioctl to the driver under test.    */
/*                         */
/*  EXECUTION ENVIRONMENT:                */
/* Can only be executed at the user process level.       */
/*                         */
/*  DATA STRUCTURES:                   */
/* References no global data structures.           */
/*                            */
/*  RETURNS:                              */
/* TRUE  if the halt succeeded.              */
/* FALSE if the halt failed.              */
/*                         */
/*----------------------------------------------------------------------*/

ar_halt (fildes)
   int   fildes;
{
   int   result, rc = TRUE;

   result = IOCTL( fildes, MP_STOP_AR, &aresp );
   if (result < 0) {
       fprintf(stderr, "\n\t ");
       decode_error(errno);
       rc = FALSE;
   } else
       printf("\n\t Halt Auto-Response succeeded.");
   return(rc);
}


/*--------------------------  C H G _ P A R M S  -----------------------*/
/*                         */
/*  NAME: chg_parms                    */
/*                         */
/*  FUNCTION:                       */
/* Issues the MP_CHG_PARMS ioctl to the driver under test.  The   */
/* new values of the receive timer, poll address, and select   */
/* address are input and compared to the values in the startdev   */
/* block -- if the new value is different, the change parameters  */
/* mask is set for that value change and the startdev block is */
/* updated to the new value.                 */
/*                         */
/*  EXECUTION ENVIRONMENT:                */
/* Can only be executed at the user process level.       */
/*                         */
/*  DATA STRUCTURES:                   */
/* Alters the rcv_timeout, poll_addr, and select_addr fields of   */
/* the startdev block.                 */
/*                            */
/*  RETURNS:                        */
/* TRUE  If the ioctl succeeds.              */
/* FALSE If the ioctl fails.              */
/*                         */
/*----------------------------------------------------------------------*/

chg_parms (fildes)
   int   fildes;
{
   t_chg_parms cparm;
   int   inp, result, rc = TRUE;

   printf("\n\t ******************  CHANGE PARAMETERS  *****************");

   bzero( &cparm, sizeof(cparm));

   sprintf(str, "%d", startdev.rcv_timeout);
   inp = prompt("\n\t Recv timeout in 100's of milliseconds", "%d", str);
   if (inp < 0) return;
   cparm.rcv_timer = inp;
   if (cparm.rcv_timer != startdev.rcv_timeout)
   {
       cparm.chg_mask |= FS_RCV_TMR;
       startdev.rcv_timeout = cparm.rcv_timer;
   }
   sprintf(str, "%x", startdev.poll_addr);
   inp = prompt("\t Poll address (hex)", "%x", str);
   if (inp < 0) return;
   cparm.poll_addr = inp;
   if (cparm.poll_addr != startdev.poll_addr)
   {
       cparm.chg_mask |= FS_POLL_ADDR;
       startdev.poll_addr = cparm.poll_addr;
   }
   sprintf(str, "%x", startdev.select_addr);
   inp = prompt("\t Select address (hex)", "%x", str);
   if (inp < 0) return;
   cparm.select_addr = inp;
   if (cparm.select_addr != startdev.select_addr)
   {
       cparm.chg_mask |= FS_SEL_ADDR;
       startdev.select_addr = cparm.select_addr;
   }
   result = IOCTL( fildes, MP_CHG_PARMS, &cparm );
   if (result < 0) {
       fprintf(stderr, "\n\t ");
       decode_error(errno);
       rc = FALSE;
   } else
       printf("\n\t -- Change Parms succeeded --");

   printf("\n\t ********************************************************");
   return(rc);
}

/*----------------------  D O W N L O A D _ A S W  ---------------------*/
/*                         */
/*  NAME: download_asw                    */
/*                         */
/*  FUNCTION:                       */
/*                         */
/*  EXECUTION ENVIRONMENT:                */
/* Can only be executed at the user process level.       */
/*                         */
/*  DATA STRUCTURES:                   */
/* References no global data structures.           */
/*                            */
/*  RETURNS:                        */
/*                         */
/*----------------------------------------------------------------------*/

download_asw (fildes)
   int   fildes;
{
   int    rc;
        int        asw_fd;       /* file descriptor for adapter software  */
      int       asw_len;      /* length of adapter software file       */
      char      *asw;         /* adapter software area..to be malloc'd */
   char     *mpqpasw_path = "/etc/asw/mpqpasw";
   t_rw_cmd *cmd_blk;

   printf("\n\t **************  DOWNLOAD ADAPTER SOFTWARE  *************");
   printf("\n\t Opening %s . . .", mpqpasw_path);
   asw_fd = open( mpqpasw_path, O_RDONLY );
   if (asw_fd < 0)
   {
       fprintf(stderr, "\n\t Open on %s failed --\n\t ", mpqpasw_path);
       decode_error(errno);
       rc = FALSE;
       goto exit;
   }
   if ((asw_len = lseek( asw_fd, 0L, 2 )) < 0)
   {
       fprintf(stderr, "\n\t Lseek to EOF on code file failed --\n\t");
       decode_error(errno);
       rc = FALSE;
       close(asw_fd);
       goto exit;
   }
   if ((rc = lseek( asw_fd, 0L, 0 )) < 0)
   {
       fprintf(stderr, "\n\t Lseek to start of code file failed --\n\t");
       decode_error(errno);
       rc = FALSE;
       close(asw_fd);
       goto exit;
   }
   asw = malloc( asw_len );
   printf("\n\t Reading %s . . .", mpqpasw_path);
   if ((rc = read( asw_fd, asw, asw_len )) < 0)
   {
       fprintf(stderr, "\n\t Read of adapter software failed --\n\t");
       decode_error(errno);
       rc = FALSE;
       close(asw_fd);
       goto exit;
   }
   cmd_blk = (t_rw_cmd *)malloc(sizeof(t_rw_cmd));
   cmd_blk->length  = asw_len;
   cmd_blk->mem_off = ASW_OFFSET;
   cmd_blk->usr_buf = asw;
   printf("\n\t Downloading to adapter . . .");
   rc = IOCTL( fildes, MP_RASW, cmd_blk );
   if (rc < 0) {
       fprintf(stderr, "\n\t ");
       decode_error(errno);
       rc = FALSE;
   } else {
       printf("\n\t -- Download Adapter Software succeeded --");
       started = 0;
       if (kern)
           rc = IOCTL( fildes, KID_CLOSE_DRVR, &dopen );
       if (rc == 0)
           rc = close( fildes );
       dsclose();          /* do any ds close stuff */
       openstate = FALSE;
       started = FALSE;
       autoresponse = FALSE;
       rc = TRUE;
   }
   close(asw_fd);
   free(asw);
   free(cmd_blk);
    exit:
   printf("\n\t ********************************************************");
   return(rc);
}


/*--------------------  A D A P T E R _ C O M M A N D  -----------------*/
/*                         */
/*  NAME: adapter_command                 */
/*                         */
/*  FUNCTION:                       */
/*                         */
/*  EXECUTION ENVIRONMENT:                */
/* Can only be executed at the user process level.       */
/*                         */
/*  DATA STRUCTURES:                   */
/* References no global data structures.           */
/*                            */
/*  RETURNS:                        */
/*                         */
/*----------------------------------------------------------------------*/

adapter_command (fildes)
   int   fildes;
{
   char     test;
   unsigned short val, addr;
   int      i, rc;
   t_usr_cmd_ovl  adapcmd;
   t_get_stat  getstat;
   struct pollfd  pollblk;

   printf("\n\t ******************  ADAPTER COMMANDS  ******************");
   printf("\n");
   printf("\n   Adapter Diagnostic Commands:");
   printf("\n\t A)   RAM Test       B)   ROM Test          ");
   printf("C)   CPU Test     ");
   printf("\n\t D)   CIO Test       E)   SCC Test          ");
   printf("F)   Bus Master DMA ");
   printf("\n");
   printf("\n   Adapter Setup and Status Commands:");
   printf("\n\t G)   Memory Size    H)   Get Interface ID  ");
   printf("I)   Get EIB ID   ");
   printf("\n\t J)   Cfg CIO Port   K)   Cfg SCC Channel   ");
   printf("L)   Cfg DMA Channel");
   printf("\n\t M)   Cfg HW Timer   N)   Init WD Timer     ");
   printf("O)   Priority Switch");
   printf("\n\t P)   Get ROS Vers.  R)   I/O Read          ");
   printf("S)   I/O Write      ");
   printf("\n");
   printf("\n   Port Specific Diagnostic Commands:");
   printf("\n\t T)   CIO Registers  U)   DUSCC Registers   ");
   printf("V)   DMA Registers");
   printf("\n\t W)   CPU Timers     Y)   CPU DMA Registers ");
   printf("\n\n\t Choose a test from above: ");
   bzero(&adapcmd, sizeof(adapcmd));
   bzero(&getstat, sizeof(getstat));
   adapcmd.p_edrr = malloc(64);     /* get memory areas */
   getstat.p_edrr = malloc(64);
   bzero(adapcmd.p_edrr, 64);
   bzero(getstat.p_edrr, 64);
                  /* read char, throw out NL */
   if ((test = getinput()) != '\n') getinput();
   switch (test) {            /* select test routine */
       case 'a':           /* RAM TEST */
       case 'A':
      adapcmd.command = RAM_EXT_TEST;
      i = prompt("\t Test Start Address (hex)", "%x", "20000");
      if (i < 0) return;
      adapcmd.u_data_area.c_ovl.tst_addr = i;
      i = prompt("\t Test Length in 16 byte blocks", "%d", "100");
      if (i < 0) return;
      adapcmd.u_data_area.c_ovl.tst_length = i;
      break;

       case 'b':           /* ROM TEST */
       case 'B':
      adapcmd.command = MEMORY_CKSUM;
      i = prompt("\t Test Start Address (hex)", "%x", "F8000");
      if (i < 0) return;
      adapcmd.u_data_area.c_ovl.tst_addr = i;
      i = prompt("\t Test Length in bytes", "%d", "8000");
      if (i < 0) return;
      adapcmd.u_data_area.c_ovl.tst_length = i;
      break;

       case 'c':           /* CPU TESTS */
       case 'C':
      adapcmd.command = ROS_CPU_AU_TEST;
      break;

       case 'd':           /* CIO TESTS */
       case 'D':
      i = prompt("\t Which CIO component, 0 or 1", "%d", "0");
      if (i < 0) return;
      adapcmd.u_data_area.d_ovl.data[0] = i;
      adapcmd.command = CIO_ROS_AU_TEST;
      break;

       case 'e':           /* SCC TESTS */
       case 'E':
      i = prompt("\t Which SCC component, 0 or 1", "%d", "0");
      if (i < 0) return;
      adapcmd.u_data_area.d_ovl.data[0] = i;
      adapcmd.command = SCC_ROS_AU_TEST;
      break;

       case 'f':           /* BUS MASTER DMA TESTS */
       case 'F':
      adapcmd.command = BUS_MSTR_DMA;
      break;

       case 'g':           /* GET MEMORY SIZE */
       case 'G':
      adapcmd.command = RTN_MEM_SIZE;
      break;

       case 'h':           /* GET INTERFACE ID */
       case 'H':
      adapcmd.command = GET_INTF_ID;
      break;

       case 'i':           /* GET EIB ID */
       case 'I':
      adapcmd.command = GET_EIB_ID;
      break;

       case 'j':           /* CIO CONFIG */
       case 'J':
      i = prompt("\t Which CIO Port, 0, 1, 2, or 3", "%d", "0");
      if (i < 0) return;
      adapcmd.u_data_area.d_ovl.data[0] = i;
      adapcmd.command = CFG_CIO_PORT;
      break;

       case 'k':           /* SCC CONFIG */
       case 'K':
      i = prompt("\t Which SCC Channel, 0, 1, 2, or 3", "%d", "0");
      if (i < 0) return;
      adapcmd.u_data_area.d_ovl.data[0] = i;
      adapcmd.command = CFG_SCC_CHNL;
      break;

       case 'l':           /* DMA CONFIG */
       case 'L':
      i = prompt("\t Which DMA Channel, 0, 1, 2, or 3", "%d", "0");
      if (i < 0) return;
      adapcmd.u_data_area.d_ovl.data[0] = i;
      adapcmd.command = CFG_DMA_CHNL;
      break;

       case 'm':           /* CONFIG HARDWARE TIMER */
       case 'M':
      adapcmd.command = CFG_HDW_TMR;
      break;

       case 'n':           /* INIT WATCHDOG TIMER */
       case 'N':
      adapcmd.command = INIT_WD_TMR;
      break;

       case 'o':           /* PRIORITY SWITCH */
       case 'O':
      adapcmd.command = PRIORITY_SWTCH;
      break;

       case 'p':           /* GET ROS VERSION */
       case 'P':
      adapcmd.command = GET_ROS_VRSN;
      break;

       case 'r':           /* I/O READ */
       case 'R':
      adapcmd.command = IO_READ;
      i = y_or_n("\t Use word size (enter N for bytes)", TRUE);
      if (i < 0) return;
      adapcmd.u_data_area.d_ovl.data[0] = i;
      addr = prompt("\t Read from what I/O address (hex)",
         "%x", "1800");
      if (addr < 0) return;
      adapcmd.u_data_area.d_ovl.data[2] = LO_BYTE(addr);
      adapcmd.u_data_area.d_ovl.data[3] = HI_BYTE(addr);
      break;

       case 's':           /* I/O WRITE */
       case 'S':
      adapcmd.command = IO_WRITE;
      i = y_or_n("\t Use word size (enter N for bytes)", TRUE);
      if (i < 0) return;
      adapcmd.u_data_area.d_ovl.data[0] = i;
      addr = prompt("\t Write to what I/O address (hex)",
         "%x", "1800");
      if (addr < 0) return;
      adapcmd.u_data_area.d_ovl.data[2] = LO_BYTE(addr);
      adapcmd.u_data_area.d_ovl.data[3] = HI_BYTE(addr);
      val = prompt("\t Enter value to write (hex)", "%x", "0");
      if (val < 0) return;
      adapcmd.u_data_area.d_ovl.data[4] = LO_BYTE(val);
      adapcmd.u_data_area.d_ovl.data[5] = HI_BYTE(val);
      break;

       case 't':           /* CIO REGISTER QUERY */
       case 'T':
      adapcmd.command = CIO_REG_QRY;
      i = prompt("\t Which Port, 0, 1, 2, or 3", "%d", "0");
      if (i < 0) return;
      adapcmd.u_data_area.d_ovl.data[0] = i;
      break;

       case 'u':           /* DUSCC REGISTER QUERY */
       case 'U':
      adapcmd.command = DUSCC_REG_QRY;
      i = prompt("\t Which Port, 0, 1, 2, or 3", "%d", "0");
      if (i < 0) return;
      adapcmd.u_data_area.d_ovl.data[0] = i;
      break;

       case 'v':           /* DMA REGISTER QUERY */
       case 'V':
      adapcmd.command = DMA_REG_QRY;
      i = prompt("\t Port DMA Channel Number", "%d", "0");
      if (i < 0) return;
      adapcmd.u_data_area.d_ovl.data[0] = i;
      break;

       case 'w':           /* CPU TIMER QUERY */
       case 'W':
      adapcmd.command = 0xC8;
      i = prompt("\t Which Timer, 0, 1, or 2", "%d", "0");
      if (i < 0) return;
      adapcmd.u_data_area.d_ovl.data[0] = i;
      break;

       case 'y':           /* CPU DMA CHANNEL QUERY */
       case 'Y':
      adapcmd.command = 0xC9;
      i = prompt("\t Which Channel, 0 or 1", "%d", "0");
      if (i < 0) return;
      adapcmd.u_data_area.d_ovl.data[0] = i;
      break;

       default:            /* EXIT */
      test = 0;
      break;
   }
   if (test) {
       rc = IOCTL( fildes, MP_QPCMD, &adapcmd );
       if (rc < 0)
       {
           fprintf(stderr, "\n\t ");
           decode_error(errno);
       } else {
           /*                       */
           /*  Wait for a status block by polling for it:      */
           /*                    */
           pollblk.fd = fildes;
           pollblk.reqevents = POLLPRI;   /* wait for exception */
           pollblk.rtnevents = 0;
           rc = poll( &pollblk, 1, STATUS_TIMEOUT );
           if (rc < 0)
      {
               fprintf(stderr, "\n\t POLL ");
          decode_error(errno);
          return(FALSE);
           }
           if (rc == 0) {
               fprintf(stderr, "\n\t Wait for status timed out!");
           } else {
          /*                     */
          /*  Status is available -- issue ioctl to get it. */
          /*                     */
               rc = IOCTL( fildes, MP_GETSTAT, &getstat );
          if ( getstat.type == TEST_PASS )
          {
              i = y_or_n("\t Adapter command succeeded; view EDRR ",
            FALSE);
              if (i < 0)
         {
                       free(adapcmd.p_edrr);
                       free(getstat.p_edrr);
             return;
         }
              if (i) {
             switch (test) {

                 case 't': /* CIO REGISTER QUERY */
                 case 'T':
                printf("\n");
                printf("\n\t MICR  = ");
                print_binary( getstat.p_edrr[1], 8 );
                printf("\n\t MCCR  = ");
                print_binary( getstat.p_edrr[2], 8 );
                printf("\n\t PIV   = ");
                print_binary( getstat.p_edrr[3], 8 );
                printf("\n\t PCSR  = ");
                print_binary( getstat.p_edrr[4], 8 );
                printf("\n\t CTCSR = ");
                print_binary( getstat.p_edrr[5], 8 );
                printf("\n\t PDR   = ");
                print_binary( getstat.p_edrr[6], 8 );
                printf("\n\t CTCCM = ");
                print_binary( getstat.p_edrr[7], 8 );
                printf("\n\t CTCCL = ");
                print_binary( getstat.p_edrr[8], 8 );
                printf("\n\t CTTCM = ");
                print_binary( getstat.p_edrr[9], 8 );
                printf("\n\t CTTCL = ");
                print_binary( getstat.p_edrr[10], 8 );
                printf("\n\t CTMS  = ");
                print_binary( getstat.p_edrr[11], 8 );
                printf("\n\t PMSR  = ");
                print_binary( getstat.p_edrr[12], 8 );
                printf("\n\t PDPP  = ");
                print_binary( getstat.p_edrr[13], 8 );
                printf("\n\t PDD   = ");
                print_binary( getstat.p_edrr[14], 8 );
                printf("\n\t PSIOC = ");
                print_binary( getstat.p_edrr[15], 8 );
                printf("\n\t PPP   = ");
                print_binary( getstat.p_edrr[16], 8 );
                printf("\n\t PPT   = ");
                print_binary( getstat.p_edrr[17], 8 );
                printf("\n\t PPM   = ");
                print_binary( getstat.p_edrr[18], 8 );
                printf("\n\n\t Press Enter to continue ");
                getinput();
                break;

                    case 'u': /* DUSCC REGISTER QUERY */
                    case 'U':
                printf("\n");
                printf("\n\t CMR1  = ");
                print_binary(getstat.p_edrr[1], 8);
                printf("\t CTH   = ");
                print_binary(getstat.p_edrr[13], 8);
                printf("\n\t CMR2  = ");
                print_binary(getstat.p_edrr[2], 8);
                printf("\t CTL   = ");
                print_binary(getstat.p_edrr[14], 8);
                printf("\n\t S1R   = ");
                print_binary(getstat.p_edrr[3], 8);
                printf("\t PCR   = ");
                print_binary(getstat.p_edrr[15], 8);
                printf("\n\t S2R   = ");
                print_binary(getstat.p_edrr[4], 8);
                printf("\t CCR   = ");
                print_binary(getstat.p_edrr[16], 8);
                printf("\n\t TPR   = ");
                print_binary(getstat.p_edrr[5], 8);
                printf("\t RSR   = ");
                print_binary(getstat.p_edrr[25], 8);
                printf("\n\t TTR   = ");
                print_binary(getstat.p_edrr[6], 8);
                printf("\t TRSR  = ");
                print_binary(getstat.p_edrr[26], 8);
                printf("\n\t RPR   = ");
                print_binary(getstat.p_edrr[7], 8);
                printf("\t ICTSR = ");
                print_binary(getstat.p_edrr[27], 8);
                   printf("\n\t RTR   = ");
                print_binary(getstat.p_edrr[8], 8);
                printf("\t GSR   = ");
                print_binary(getstat.p_edrr[28], 8);
                printf("\n\t CTPRH = ");
                print_binary(getstat.p_edrr[9], 8);
                printf("\t IER   = ");
                print_binary(getstat.p_edrr[29], 8);
                printf("\n\t CTPRL = ");
                print_binary(getstat.p_edrr[10], 8);
                printf("\t IVR   = ");
                print_binary(getstat.p_edrr[31], 8);
                printf("\n\t CTCR  = ");
                print_binary(getstat.p_edrr[11], 8);
                printf("\t ICR   = ");
                print_binary(getstat.p_edrr[32], 8);
                printf("\n\t OMR   = ");
                print_binary(getstat.p_edrr[12], 8);
                printf("\n\n\t Press Enter to continue ");
                getinput();
                break;

                    case 'v': /* DMA REGISTER QUERY */
                    case 'V':
                printf("\n\t CCW   = %04x",
                    getstat.p_edrr[0] |
                  getstat.p_edrr[1]<< 8);
                printf("\n\t TAR   = %04x",
                    getstat.p_edrr[2] |
                  getstat.p_edrr[3]<< 8);
                printf("\n\t CMR   = %04x",
                    getstat.p_edrr[4] |
                  getstat.p_edrr[5]<< 8);
                printf("\n\t IOAR  = %04x",
                    getstat.p_edrr[6] |
                  getstat.p_edrr[7]<< 8);
                printf("\n\t TCR   = %04x",
                    getstat.p_edrr[8] |
                  getstat.p_edrr[9]<< 8);
                printf("\n\t TARE  = %04x",
                    getstat.p_edrr[10]|
                  getstat.p_edrr[11]<< 8);
                printf("\n\t LARE  = %04x",
                    getstat.p_edrr[12]|
                  getstat.p_edrr[13]<< 8);
                printf("\n\t LAR   = %04x",
                    getstat.p_edrr[14]|
                  getstat.p_edrr[15]<< 8);
                printf("\n\n\t Press Enter to continue ");
                getinput();
                break;

                 default:
                         if (print_buffer
               (getstat.p_edrr, 48, 0, stdout) < 0)
                return;
                break;
             }
         }
          } else {
              fprintf(stderr, "\n\t Adapter command failed: ");
         fprintf(stderr, "\n\t   Type code   = %d",
            getstat.type );
         fprintf(stderr, "\n\t   Status code = %d",
            getstat.status );
          }
           }
       }
   }
   free(adapcmd.p_edrr);
   free(getstat.p_edrr);
   printf("\n\t ********************************************************");
   return;
}


/*------------------------  B U S _ M E M O R Y  -----------------------*/
/*                         */
/*  NAME: bus_memory                   */
/*                         */
/*  FUNCTION:                       */
/*                         */
/*  EXECUTION ENVIRONMENT:                */
/* Can only be executed at the user process level.       */
/*                         */
/*  DATA STRUCTURES:                   */
/* References no global data structures.           */
/*                            */
/*  RETURNS:                        */
/*                         */
/*----------------------------------------------------------------------*/

bus_memory (fildes)
   int   fildes;
{
   t_rw_cmd cmd_blk;
   int   inp, mode = 0, rc = 0;

   printf("\n\t ******************  BUS MEMORY ACCESS  *****************");
   while (!mode) {
       mode = prompt("\n\t Read (R) or Write (W)", "%c", "R");
       switch (mode) {
      case 'r':
      case 'R':
          mode = MP_RMEM;
          break;
      case 'w':
      case 'W':
          mode = MP_WMEM;
          break;
      case '.':
          return;
      default:
          mode = 0;
       }
   }
   if (mode == MP_RMEM) {
       while (TRUE) {
           inp = prompt("\t Length (bytes) to read", "%d", "1800");
           if (inp < 0) return;
      if (inp > PAGESIZE)
          fprintf(stderr, "\t That's too big!\n");
      else
          break;
       }
       cmd_blk.length = inp;
       inp = prompt("\t Offset (hex) from beginning of adapter memory",
         "%x", "1f800");
       if (inp < 0) return;
       cmd_blk.mem_off = inp;
       cmd_blk.usr_buf = malloc(cmd_blk.length);
       while (TRUE) {
           rc = IOCTL( fildes, mode, &cmd_blk );
           if (rc >= 0) {
          if (print_buffer(cmd_blk.usr_buf, cmd_blk.length,
             cmd_blk.mem_off, stdout) < 0)
         break;
           } else
          break;
           inp = y_or_n("\t   Go to next page", TRUE);
      if (inp < 0) break;
      if (inp)
          cmd_blk.mem_off += PAGESIZE;
      else
          break;
       }
       free(cmd_blk.usr_buf);
   } else {
       inp = prompt("\t Offset (hex) from beginning of adapter memory",
         "%x", "0");
       if (inp < 0) return;
       cmd_blk.mem_off = inp;
       cmd_blk.usr_buf = malloc(cmd_blk.length);
       if (fill_buffer(cmd_blk.usr_buf, &cmd_blk.length, BUFSIZE) < 0)
      return;
       rc = IOCTL( fildes, mode, &cmd_blk );
       if (rc >= 0)
           printf("\t -- Write Succeeded --");
   }
   if (rc < 0) {
       fprintf(stderr, "\n\t ");
       decode_error(errno);
       rc = FALSE;
   }
   printf("\n\t ********************************************************");
   return(rc);
}

/*--------------  E X A M I N E _ D A T A _ S T R U C T S  -------------*/
/*                         */
/*  NAME: examine_data_structs                  */
/*                         */
/*  FUNCTION:                       */
/* Dispatches on one of several Adapter data structure format  */
/* routines, based on user input.               */
/*                         */
/*  EXECUTION ENVIRONMENT:                */
/* Can only be executed at the user process level.       */
/*                         */
/*  DATA STRUCTURES:                   */
/* None                       */
/*                            */
/*  RETURNS:                        */
/* Nothing                       */
/*                         */
/*----------------------------------------------------------------------*/

examine_data_structs (fildes)
   int   fildes;
{
   int   r;
   char  c = 0;

   printf("\n\t ***********  Examine Adapter Data Structures  **********");
   while ( !c ) {
       r = prompt("\n\t Examine Port Ctrl (P), Cmd blocks (C), or Port Cmd Que (R)", "%c", "P");
       if ( r < 0 ) break;
       c = (char)r;
       switch( c ) {
           case 'p':
           case 'P':
          examine_pcb( fildes );
          break;
           case 'c':
           case 'C':
      case 'r':
      case 'R':
          examine_cmd_blocks( fildes, c );
          break;
           default:
          c = 0;
          break;
       }
        }
   printf("\n\t ********************************************************");
}

/*-----------------------  E X A M I N E _ P C B  ----------------------*/
/*                         */
/*  NAME: examine_pcb                     */
/*                         */
/*  FUNCTION:                       */
/* Reads the Port Control Block from Adapter Software Memory   */
/* and formats it to the screen, taking care of short and      */
/* long byte swaps for Little/Big Endian differences.    */
/*                         */
/*  EXECUTION ENVIRONMENT:                */
/* Can only be executed at the user process level.       */
/*                         */
/*  DATA STRUCTURES:                   */
/* References the port number (unit).           */
/*                            */
/*  RETURNS:                        */
/* Nothing                       */
/*                         */
/*----------------------------------------------------------------------*/


/*----------------------------------------------------------------------*/
/*                                                                      */
/*  This is a "customized" version of the Adapter Software PCB       */
/*  definition -- this struct must be maintained to reflect the      */
/*  actual contents and lengths of the Adapter Software definition   */
/*  with the following modifications:              */
/*                         */
/* ASW Declaration (C/2)      Here (RIOS Compiler)    */
/* ---------------------      --------------------    */
/*     int        ->    short       */
/*     unsigned int  ->    unsigned short    */
/*     (char *)      ->    unsigned short    */
/*     (int *)    ->    unsigned short    */
/*     (short *)     ->    unsigned short    */
/*     (struct *)    ->    unsigned short    */
/*     *(function)() ->    unsigned short    */
/*                                                                      */
/*----------------------------------------------------------------------*/

typedef struct
{
   unsigned char  port_no;
   unsigned char  start_parm;

   unsigned short mask_on; /* Scheduler mask, On (OR) */
   unsigned short mask_off;   /* Scheduler mask, Off (~AND) */

   unsigned char  rx_next; /* Next allocated Rx Buffer # */
   unsigned char  rx_curr; /* Current Rx Buffer # */
   unsigned char  rx_last; /* Just completed Rx Buffer # */
   unsigned char  rxchan;     /* Even/Odd channel indicator */
   unsigned short rx_ccw;     /* Channel default Rx CCW */
   unsigned short rx_cmb;     /* Channel default Rx Character
                  Match bytes */
   unsigned short rx_off;     /* Data Offset in each frame */
   unsigned short rx_flgt; /* Frame length in bytes */
   unsigned char  txbuf0;     /* Even TX Buffer Number */
   unsigned char  txbuf1;     /* Odd TX Buffer Number */
   unsigned char  txchan;     /* Even/Odd channel indicator */
   unsigned char  tx_pad;     /* Pad byte, Internal clocking */

   unsigned short cmd_q;      /* Port Command Queue Address */
   unsigned short rsp_q;      /* Port Response Queue Address */

   unsigned short rx_dma_0;   /* Port RX DMA channel 0 */
   unsigned short rx_dma_1;   /* Port RX DMA channel 1 */
   unsigned short tx_dma_0;   /* Port TX DMA channel 0 */
   unsigned short tx_dma_1;   /* Port TX DMA channel 1 */

   unsigned short scc_base;   /* Port DUSCC base address */
   unsigned short scc_data;   /* Port DUSCC data reg. I/O addr */
   unsigned short cio_base;   /* Port CIO base I/O address */
   unsigned short cio_data;   /* Port CIO data register I/O addr */
   unsigned short alt_cio; /* Alternate CIO base */
   unsigned char  cio_port;   /* Which port, A = 0, B = 1 */
   unsigned char  cio_last_data; /* Last CIO data value read */

   unsigned short f_txserve;  /* Tx data preprocessor  */
   unsigned short f_txframe;  /* Tx frame routine */
   unsigned short f_rxserve;  /* Rx data postprocessor */

   unsigned int   sleep_rqe;  /* RQE removed which re-  */
               /* scheduled a sleeper.   */
   unsigned int   error_rqe;  /* Storage during err/sts */
               /* offlevel processing.   */

   unsigned int   start_rqe;  /* Storage during start   */
               /* processing.         */
   unsigned short baud_rate;
   unsigned char  modem_int;
   unsigned char  modem_state;
   unsigned char  port_state;
   unsigned char  port_status;
   unsigned char  x21_state;
   unsigned char  proto;
   unsigned char  flags;
   unsigned char  field_select;
   unsigned char  modem_mask;
   unsigned char  phys_link;
   unsigned char  poll_addr;
   unsigned char  select_addr;
   unsigned char  dial_proto;
   unsigned char  dial_flags;
   unsigned char  data_proto;
   unsigned char  data_flags;

   unsigned char  timer_type;
   unsigned char  fs_timer_type;
   unsigned short rx_timeout;

   unsigned short rx_state;   /* rx state machine ctl */
   unsigned short tx_state;   /* tx state machine ctl */

   unsigned short recv_type;  /* receive message type */
   unsigned short bsc_prev_rx;   /* Previous bisync frame, Rx */
   unsigned short bsc_prev_tx;   /* Previous bisync frame, Tx */

   unsigned int   dial_data;

   unsigned long  tx_bytes;
   unsigned long  rx_bytes;

   unsigned long  tx_frames;
   unsigned long  rx_frames;

   unsigned int   destptr; /* ptr to dest buffer */
   unsigned int   destlimit;  /* end of dest buffer */
   unsigned short destlen; /* length of dest buff */
   unsigned short write_chars;   /* # of chars written */

   unsigned int   srcptr;     /* ptr to src buffer */
   unsigned int   srclimit;   /* end of src buffer */
   unsigned short srclen;     /* length of src buff */
   unsigned short read_chars; /* # of chars read */

   unsigned short retry_in_prog; /* x21 retry flag */
               /* ignore the rest */
} PCB;

examine_pcb (fildes)
   int   fildes;
{
   t_rw_cmd cmd_blk;
   int    rc = 0, port;
   char  str[4];
   extern   char unit;
   PCB   *p_pcb;

   sprintf(str, "%d", unit);
   port = prompt("\n\t Examine which port", "%d", str);
   if (port < 0) return;
   cmd_blk.length  = sizeof( PCB );
   cmd_blk.mem_off = ((port % 4) * PCB_SIZE) + PCB_BASE;
   cmd_blk.usr_buf = malloc( sizeof( PCB ));
   p_pcb = (PCB *)cmd_blk.usr_buf;
   rc = IOCTL( fildes, MP_RMEM, &cmd_blk );
   if (rc < 0) {
       fprintf(stderr, "\n\t ");
       decode_error(errno);
       rc = FALSE;
   } else {
       printf("\n   Port Number = %.1d    ", p_pcb->port_no);
       printf("     CIO Base    = %.4xh    ", SWAPSHORT(p_pcb->cio_base));
       printf("   Dial Proto  = %.2xh      ", p_pcb->dial_proto);
       printf("\n   Mask On     = %.4xh", SWAPSHORT(p_pcb->mask_on));
       printf("     CIO Data    = %.4xh    ", SWAPSHORT(p_pcb->cio_data));
       printf("   Dial Flags  = %.2xh      ", p_pcb->dial_flags);
       printf("\n   Mask Off    = %.4xh", SWAPSHORT(p_pcb->mask_off));
       printf("     CIO Port    = %.2xh      ", p_pcb->cio_port);
       printf("   Data Proto  = %.2xh      ", p_pcb->data_proto);
       printf("\n   RX Next     = %.3d  ", p_pcb->rx_next);
       printf("     CIO Last    = %.2xh      ", p_pcb->cio_last_data);
       printf("   Data Flags  = %.2xh      ", p_pcb->data_flags);
       printf("\n   RX Current  = %.3d  ", p_pcb->rx_curr);
       printf("     TX Serve F  = %.4xh    ", SWAPSHORT(p_pcb->f_txserve));
       printf("   TX Bytes    = %.8d ", SWAPLONG(p_pcb->tx_bytes));
       printf("\n   RX Last     = %.3d  ", p_pcb->rx_last);
       printf("     TX Frame F  = %.4xh    ", SWAPSHORT(p_pcb->f_txframe));
       printf("   TX Frames   = %.8d ", SWAPLONG(p_pcb->tx_frames));
       printf("\n   RX Channel  = ");
       if ( ODD( p_pcb->rxchan ))
      printf("ODD  ");
       else
      printf("EVEN ");
       printf("     RX Serve F  = %.4xh    ", SWAPSHORT(p_pcb->f_rxserve));
       printf("   TX State    = %.2xh      ", p_pcb->tx_state);
       printf("\n   RX CCW      = %.4xh", SWAPSHORT(p_pcb->rx_ccw));
       printf("     Sleep RQE   = %.8xh", SWAPLONG(p_pcb->sleep_rqe));
       printf("   RX Bytes    = %.8d ", SWAPLONG(p_pcb->rx_bytes));
       printf("\n   RX Match    = %.4xh", SWAPSHORT(p_pcb->rx_cmb));
       printf("     Error RQE   = %.8xh", SWAPLONG(p_pcb->error_rqe));
       printf("   RX Frames   = %.8d ", SWAPLONG(p_pcb->rx_frames));
       printf("\n   RX Offset   = %.5d", SWAPSHORT(p_pcb->rx_off));
       printf("     Start RQE   = %.8xh", SWAPLONG(p_pcb->start_rqe));
       printf("   RX State    = %.2xh      ", p_pcb->rx_state);
       printf("\n   RX Framelen = %.5d", SWAPSHORT(p_pcb->rx_flgt));
       printf("     Modem Int   = %.2xh      ", p_pcb->modem_int);
       printf("   Timer Type  = %.2xh      ", p_pcb->timer_type);
       printf("\n   TX Even Buf = %.3d  ", p_pcb->txbuf0);
       printf("     Modem State = %.2xh      ", p_pcb->modem_state);
       printf("   Recv Type   = %.4xh    ", SWAPSHORT(p_pcb->recv_type));
       printf("\n   TX Odd Buf  = %.3d  ", p_pcb->txbuf1);
       printf("     Port State  = %.2xh      ", p_pcb->port_state );
       printf("   BSC Prev RX = %.4xh    ", SWAPSHORT(p_pcb->bsc_prev_rx));
       printf("\n   TX Channel  = ");
       if ( ODD( p_pcb->txchan ))
      printf("ODD  ");
       else
      printf("EVEN ");
       printf("     Port Status = %.2xh      ", p_pcb->port_status );
       printf("   BSC Prev TX = %.4xh    ", SWAPSHORT(p_pcb->bsc_prev_tx));
       printf("\n   TX Pad byte = %.2xh  ", p_pcb->tx_pad);
       printf("     X21 State   = %.2xh      ", p_pcb->x21_state );
       printf("   Dest Ptr    = %.8xh", SWAPLONG(p_pcb->destptr));
       printf("\n   Port Cmd Q  = %.4xh", SWAPSHORT(p_pcb->cmd_q));
       printf("     Protocol    = %.2xh      ", p_pcb->proto );
       printf("   Dest Limit  = %.8xh", SWAPLONG(p_pcb->destlimit));
       printf("\n   Port Resp Q = %.4xh", SWAPSHORT(p_pcb->rsp_q));
       printf("     Flags       = %.2xh      ", p_pcb->flags);
       printf("   Dest Length = %.5d    ", SWAPSHORT(p_pcb->destlen));
       printf("\n   RX DMA 0    = %.4xh", SWAPSHORT(p_pcb->rx_dma_0));
       printf("     Field Sel.  = %.2xh      ", p_pcb->field_select);
       printf("   Write Chars = %.5d    ", SWAPSHORT(p_pcb->write_chars));
       printf("\n   RX DMA 1    = %.4xh", SWAPSHORT(p_pcb->rx_dma_1));
       printf("     Modem Mask  = %.2xh      ", p_pcb->modem_mask);
       printf("   Source Ptr  = %.8xh", SWAPLONG(p_pcb->srcptr));
       printf("\n   TX DMA 0    = %.4xh", SWAPSHORT(p_pcb->tx_dma_0));
       printf("     Phys Link   = %.2xh      ", p_pcb->phys_link);
       printf("   Src Limit   = %.8xh", SWAPLONG(p_pcb->srclimit));
       printf("\n   TX DMA 1    = %.4xh", SWAPSHORT(p_pcb->tx_dma_1));
       printf("     Poll Addr   = %.2xh      ", p_pcb->poll_addr);
       printf("   Src Length  = %.5d    ", SWAPSHORT(p_pcb->srclen));
       printf("\n   SCC Base    = %.4xh", SWAPSHORT(p_pcb->scc_base));
       printf("     Select Addr = %.2xh      ", p_pcb->select_addr);
       printf("   Read Chars  = %.5d    ", SWAPSHORT(p_pcb->read_chars));
       printf("\n   SCC Data    = %.4xh", SWAPSHORT(p_pcb->scc_data));
       printf("     Baud Rate   = %.5d    ", p_pcb->baud_rate);
       printf("   RX Timeout  = %.6d   ",
         SWAPSHORT(p_pcb->rx_timeout));
       printf("\n   Start Parm  = %.2xh  ", p_pcb->start_parm);
       printf("     X21 Retry   = %.2xh      ", p_pcb->retry_in_prog);
       printf("\n\t Press Enter to Continue "); getinput();
       rc = TRUE;
   }
   free( cmd_blk.usr_buf );
   return (rc);
}

/*---------------  E X A M I N E _ C M D _ B L O C K S  ----------------*/
/*                         */
/*  NAME: examine_cmd_blocks                 */
/*                         */
/*  FUNCTION:                       */
/* Reads the command block list from Adapter memory and formats   */
/* each command block to the screen, taking care of short and  */
/* long byte swaps for Little/Big Endian differences.    */
/*                         */
/*  EXECUTION ENVIRONMENT:                */
/* Can only be executed at the user process level.       */
/*                         */
/*  DATA STRUCTURES:                   */
/* References the port number (unit).           */
/*                            */
/*  RETURNS:                        */
/*                         */
/*----------------------------------------------------------------------*/


examine_cmd_blocks (fildes, blk_type)
   int   fildes;
   char  blk_type;
{
   t_rw_cmd    cmd_blk;
   int      k, out, in, rc = 0, i = 0, actual_offset = 0;
   char     c, str[4];
   adap_cmd_t  *p_cmd;
   unsigned char  queue[ NUM_CMD_BLOCKS + 4 ];
   int      port;
   extern   char  unit;

   /*  Get the out and in pointers of the adapter command   */
   /*  queue:                 */

   cmd_blk.length  = sizeof( queue );
   cmd_blk.usr_buf = queue;
   if ((blk_type == 'C') || (blk_type == 'c'))  /* adapter cmd queue */
   {
       cmd_blk.mem_off = ACQ_OFFSET;
   }
   else                 /* port cmd queue */
   {
       sprintf(str, "%d", unit);
       port = prompt("\n\t Examine which port", "%d", str);
       if (port < 0) return;
       cmd_blk.mem_off = ((port % 4) * PCQ_NXT_OFFSET) + PCQ_OFFSET;
       printf("\n   Port Number = %.1d    ", port);
   }
   rc = IOCTL( fildes, MP_RMEM, &cmd_blk );
   if (rc < 0) {
       fprintf(stderr, "\n\t ");
       decode_error(errno);
       return;
   } else {
       out = (int)queue[2];
       in  = (int)queue[3];
   }
   printf("\n\t out: %.2xh in: %.2xh",out,in);
   if (( out >= NUM_CMD_BLOCKS ) || ( in >= NUM_CMD_BLOCKS ))
   {
       fprintf(stderr, "\n\t Bad adapter command queue!");
       return;
   }
   i = in;
   while (TRUE)
   {
       cmd_blk.length  = sizeof( adap_cmd_t );
       actual_offset = (int)queue[i+4]; /* add in header length */
       cmd_blk.mem_off =
      CMD_BLK_OFFSET + (actual_offset * sizeof(adap_cmd_t));
       cmd_blk.usr_buf = malloc(sizeof( adap_cmd_t ));
       p_cmd = (adap_cmd_t *)cmd_blk.usr_buf;
       rc = IOCTL( fildes, MP_RMEM, &cmd_blk );
       if (rc < 0) {
           fprintf(stderr, "\n\t ");
           decode_error(errno);
           rc = FALSE;
           free( cmd_blk.usr_buf );
      break;
       } else {
           rc = TRUE;
      printf("\n\t Command Blk %.2xh at %.5xh (Q offset %.2xh):",
          actual_offset, cmd_blk.mem_off, i);
      if ( i == in )
          printf("  *** IN ***");
      if ( i == out )
          printf("  *** OUT ***");
      printf("\n\t   Port   = %d", p_cmd->port);
      printf("      Type  = ");
      switch ( p_cmd->type ) {
          case XMIT_SHORT:     printf("XMIT_SHORT   ");
                     break;
          case XMIT_LONG:      printf("XMIT_LONG    ");
                     break;
          case XMIT_GATHER:    printf("XMIT_GATHER  ");
                     break;
          case RCV_BUF_INDC:   printf("RCV_BUF_INDC ");
                     break;
          case SET_PARAM:      printf("SET_PARAM    ");
                     break;
          case START_PORT:     printf("START_PORT   ");
                     break;
          case STOP_PORT:      printf("STOP_PORT    ");
                     break;
          case START_RECV:     printf("START_RECV   ");
                     break;
          case HALT_RECV:      printf("HALT_RECV    ");
                     break;
          case FLUSH_PORT:     printf("FLUSH_PORT   ");
                     break;
          case QURY_MDM_INT:   printf("QURY_MDM_INT ");
                     break;
          case STRT_AUTO_RSP:  printf("STRT_AUTO_RSP");
                     break;
          case STOP_AUTO_RSP:  printf("STOP_AUTO_RSP");
                     break;
          case CHG_PARAM:      printf("CHG_PARAM    ");
                     break;
          default:       printf("Invalid: %.2xh ",
                  p_cmd->type);
                break;
      }
      printf("  Sequence = %.4xh",
            SWAPSHORT((short)p_cmd->sequence));
      printf("\n\t   Length = %.4d ",
            SWAPSHORT((short)p_cmd->length));
      printf("  Flags = ");
      print_binary( p_cmd->flags, 16 );
      printf("\n\t   Host Address = %.8x",
            SWAPLONG((long)p_cmd->host_addr ));
      printf("\n\t   Card Address = %.8x",
            SWAPLONG((long)p_cmd->card_addr ));
      switch (p_cmd->type) {
          case XMIT_SHORT:
         printf("\n\t   Data:");
         print_buffer( p_cmd->cs.data, 48, 0, stdout );
         break;
          case SET_PARAM:

         /* Show parameters */

         break;
      }
           free( cmd_blk.usr_buf );
      printf("\n");
      k = prompt("\n\t Previous (P), Next (N), or exit (.)",
          "%c", "P");
      if (k < 0) break;
      c = (char)k;
      if ( c == 'p' || c == 'P' )
      {
          if ( i )
         i = i - 1;
          else
         i = NUM_CMD_BLOCKS - 1;
      } else {
          if ( i == ( NUM_CMD_BLOCKS - 1 ))
         i = 0;
          else
         i = i + 1;
      }
       }
   }
   return (rc);
}


/*-------------------  D E C O D E _ B S C _ M S G  --------------------*/
/*                         */
/*  NAME: decode_bsc_msg                  */
/*                         */
/*  FUNCTION:                       */
/* Converts the message type CODE to a string of bisync     */
/* characters (into BUFFER) that that code represents.  The */
/* number of characters written is returned.  LENGTH is the */
/*      original length of the buffer.             */
/*                         */
/*  EXECUTION ENVIRONMENT:                */
/* Can executed at any level.             */
/*                         */
/*  DATA STRUCTURES:                   */
/* Reference no global data structures.            */
/*                            */
/*  RETURNS:                        */
/* Number of characters written to buffer, zero if no match.   */
/*                         */
/*----------------------------------------------------------------------*/

decode_bsc_msg (
   int      code,
   unsigned char  *buffer,
   int      length )
{
   int      result;

   if ( startdev.data_flags & DATA_FLG_BSC_ASC )
       p_xlat = asc_tbl;
   else
       p_xlat = ebc_tbl;

   switch ( code ) {
       case MP_ACK0:
           buffer[0] = DLE;
           buffer[1] = ACK0;
      result = 2;
           break;
       case MP_ACK1:
           buffer[0] = DLE;
           buffer[1] = ACK1;
         result = 2;
           break;
       case MP_DISC:
           buffer[0] = DLE;
           buffer[1] = EOT;
         result = 2;
           break;
       case MP_RVI:
           buffer[0] = DLE;
           buffer[1] = RVI;
         result = 2;
      break;
       case MP_WACK:
           buffer[0] = DLE;
           buffer[1] = WACK;
         result = 2;
           break;
       case MP_EOT:
           buffer[0] = EOT;
         result = 1;
           break;
       case MP_ENQ:
           buffer[0] = ENQ;
         result = 1;
           break;
       case MP_NAK:
           buffer[0] = NAK;
         result = 1;
           break;
       case MP_DATA_ACK0:
      buffer[ length ] = ACK0;
      result = length + 1;
      break;
       case MP_DATA_ACK1:
      buffer[ length ] = ACK1;
      result = length + 1;
      break;
       case MP_DATA_NAK:
      buffer[ length ] = NAK;
      result = length + 1;
      break;
       case MP_DATA_ENQ:
      buffer[ length ] = ENQ;
      result = length + 1;
      break;
       case MP_STX_ITB:
      bcopy( buffer, wrkbuf, length );
      buffer[0] = STX;
      bcopy( wrkbuf, &buffer[1], length );
      buffer[ length + 1 ] = ITB;
      result = length + 2;
      break;
       case MP_STX_ETB:
      bcopy( buffer, wrkbuf, length );
      buffer[0] = STX;
      bcopy( wrkbuf, &buffer[1], length );
      buffer[ length + 1 ] = ETB;
      result = length + 2;
      break;
       case MP_STX_ETX:
      bcopy( buffer, wrkbuf, length );
      buffer[0] = STX;
      bcopy( wrkbuf, &buffer[1], length );
      buffer[ length + 1 ] = ETX;
      result = length + 2;
      break;
       case MP_STX_ENQ:
      bcopy( buffer, wrkbuf, length );
      buffer[0] = STX;
      bcopy( wrkbuf, &buffer[1], length );
      buffer[ length + 1 ] = ENQ;
      result = length + 2;
      break;
       case MP_SOH_ITB:
      bcopy( buffer, wrkbuf, length );
      buffer[0] = SOH;
      bcopy( wrkbuf, &buffer[1], length );
      buffer[ length + 1 ] = ITB;
      result = length + 2;
      break;
       case MP_SOH_ETB:
      bcopy( buffer, wrkbuf, length );
      buffer[0] = SOH;
      bcopy( wrkbuf, &buffer[1], length );
      buffer[ length + 1 ] = ETB;
      result = length + 2;
      break;
       case MP_SOH_ETX:
      bcopy( buffer, wrkbuf, length );
      buffer[0] = SOH;
      bcopy( wrkbuf, &buffer[1], length );
      buffer[ length + 1 ] = ETX;
      result = length + 2;
      break;
       case MP_SOH_ENQ:
      bcopy( buffer, wrkbuf, length );
      buffer[0] = SOH;
      bcopy( wrkbuf, &buffer[1], length );
      buffer[ length + 1 ] = ENQ;
      result = length + 2;
      break;
       default:
         result = length;
           break;
   }
   return( result );
}


/*----------------  S H O W _ R E A D E X T _ S T A T U S  -------------*/
/*                         */
/*  NAME: show_readext_status                */
/*                         */
/*  FUNCTION:                       */
/* Decodes the message type or error code in TYPE and prints   */
/* its meaning to the screen.             */
/*                         */
/*  EXECUTION ENVIRONMENT:                */
/* Can only be executed at the user process level.       */
/*                         */
/*  DATA STRUCTURES:                   */
/* References no global data structures.           */
/*                            */
/*  RETURNS:                        */
/*                         */
/*----------------------------------------------------------------------*/

show_readext_status (
   int   type,
   int   to_where )
{
   switch ( type ) {
       case MP_SOH_ETX: fprintf( to_where, "MP_SOH_ETX");
            break;
       case MP_SOH_ITB: fprintf( to_where, "MP_SOH_ITB");
            break;
       case MP_SOH_ETB: fprintf( to_where, "MP_SOH_ETB");
            break;
       case MP_SOH_ENQ: fprintf( to_where, "MP_SOH_ENQ");
            break;
       case MP_STX_ETX: fprintf( to_where, "MP_STX_ETX");
            break;
       case MP_STX_ITB: fprintf( to_where, "MP_STX_ITB");
            break;
       case MP_STX_ETB: fprintf( to_where, "MP_STX_ETB");
            break;
       case MP_STX_ENQ: fprintf( to_where, "MP_STX_ENQ");
            break;
       case MP_ACK0: fprintf( to_where, "MP_ACK0");
            break;
       case MP_DATA_ACK0:  fprintf( to_where, "MP_DATA_ACK0");
            break;
       case MP_ACK1: fprintf( to_where, "MP_ACK1");
            break;
       case MP_DATA_ACK1:  fprintf( to_where, "MP_DATA_ACK1");
            break;
       case MP_DISC: fprintf( to_where, "MP_DISC");
            break;
       case MP_RVI:  fprintf( to_where, "MP_RVI");
            break;
       case MP_WACK: fprintf( to_where, "MP_WACK");
            break;
       case MP_EOT:  fprintf( to_where, "MP_EOT");
            break;
       case MP_ENQ:  fprintf( to_where, "MP_ENQ");
            break;
       case MP_DATA_ENQ:   fprintf( to_where, "MP_DATA_ENQ");
            break;
       case MP_NAK:  fprintf( to_where, "MP_NAK");
            break;
       case MP_DATA_NAK:   fprintf( to_where, "MP_DATA_NAK");
            break;
       case CIO_OK:  fprintf( to_where, "CIO_OK");
            break;
       case MP_BUF_OVERFLOW:  fprintf( to_where, "MP_BUF_OVERFLOW");
               break;
       case MP_X21_CPS: fprintf( to_where, "MP_X21_CPS");
            break;
       case MP_X21_DPI: fprintf( to_where, "MP_X21_DPI");
            break;
       case MP_MODEM_DATA: fprintf( to_where, "MP_MODEM_DATA");
            break;
       case MP_AR_DATA_RCVD:  fprintf( to_where, "MP_AR_DATA_RCVD");
               break;
       case MP_RCV_TIMEOUT:   fprintf( to_where, "MP_RCV_TIMEOUT");
               break;
       case MP_RX_BSC_FRAME_ERR:
            fprintf( to_where, "MP_RX_BSC_FRAME_ERR");
            break;
       case MP_RX_FRAME_ERR:  fprintf( to_where, "MP_RX_FRAME_ERR");
               break;
       default:      fprintf( to_where, "Undefined msg type: %x",
               type);
            break;
   }
}

/*-------------------------  A U T O D I A L  --------------------------*/
/*                         */
/*  NAME: autodial                        */
/*                         */
/*  FUNCTION:                       */
/* Interactively allows the user to edit the autodial/V25 portion */
/* of the startdev block.                                      */
/*                         */
/*  EXECUTION ENVIRONMENT:                */
/* Can only be executed at the user process level.       */
/*                         */
/*  DATA STRUCTURES:                   */
/* Modifies the autodial/V25 portion of the global startdev block.   */
/*                            */
/*  RETURNS: TRUE if completed successfully           */
/*    FALSE if error                */
/*                         */
/*----------------------------------------------------------------------*/

char
conv_to_ebc(inchar)
   char inchar;
{
   return( ebc_chr_tbl [inchar] );
}

autodial()
{
   char     numstr[80];
   static char dflt[SELECT_SIG_LEN] = "\0";
   static char numdflt[SELECT_SIG_LEN] = "\0";
   char     final_str[SELECT_SIG_LEN] = "\0";
   static char    modem_type=0;
   int      time_amt;
   int      i;
   char     mode = 0;
   static unsigned char use_slash = FALSE;

    if (startdev.phys_link == PL_V25)  /* v.25 bis */
    {
        if (startdev.modem_flags & MF_CALL)
            strcpy(dflt, "CRN ");
        else
            strcpy(dflt, "CIC ");
   time_amt = prompt("\t Wait how many seconds after connect", "%d",
       "5");
   if ((time_amt == FALSE) || (time_amt == '.'))
       return(FALSE);
   startdev.t_dial_data.auto_data.v25b_tx_timer = time_amt;
    }
    else /* smart modem */
    {
        if (modem_type == 0)
            modem_type = 'H';
        mode = 0;
        while (!mode)
   {
            printf("\t Hayes (H) or Ventel (V) modem [");
            if (modem_type == 'H') printf("H");
            else printf("V");
            printf("]? ");
            mode = getinput();
            if (mode == '\n')
                break;
            else
                getinput();
            switch (mode) {
                case 'h':
                case 'H':
                    modem_type = 'H';
                    break;
                case 'v':
                case 'V':
                    modem_type = 'V';
                    break;
                case '.':
                    return(FALSE);
                default:
                    mode = 0;
            }
        }
   if ((strlen(dflt) == 0) || (startdev.phys_link != phys_link_prev))
        {
            if (startdev.modem_flags & MF_CALL) {
                switch (modem_type) {
                    case 'H': /* Hayes */
                        strcpy(dflt,
             "AT L0 B0 &C1 &D2 &R0 &S1 &M1 &X0 S25=0 DT");
                        break;
                    case 'V': /* Ventel modem */
                        strcpy(dflt,
                            "AT L0 B0 %B0 &C1 &D2 &R0 &S2 &M1 &X0 S25=0 DT");
                        break;
                }
            } else {
                switch (modem_type) {
                    case 'H': /* Hayes */
                        strcpy(dflt,
                            "AT L0 B0 &C1 &D2 &R0 &S1 &M1 &X0 S25=0 S0=1");
         break;
                    case 'V': /* Ventel modem */
                        strcpy(dflt,
                            "AT L0 B0 %B0 &C1 &D2 &R0 &S2 &M1 &X0 S25=0 S0=1");
         break;
                        /* Ventel specific: */
                        /* L0-low volume        B0-CCITT   */
                        /* %B0-9600bps V.32  &C1-CD follows carrier*/
                        /* &D-mdm on_hk,dsbl AA &R0-CTS trks RTS      */
                        /* &S2-DSR follows CD   &M1-asnc dial,snc data*/
                        /* &X0-internal clock   S0=2-2 rings          */
                        /* S25=0-0secs dly to DTR                     */
      }
            }
        }    /* if (strlen (dflt == 0)) */
    }
    printf("\t Dial String-zero for no dial string [%s]? ", dflt);
    i = 0;
    while ((str[i++] = getinput()) != '\n');
    str[i-1] = '\0';
    if (str[0] != '\0')
        strcpy(dflt,str);
    if (dflt[0] == '0')
        dflt[0]=NULL;
    else if (startdev.modem_flags & MF_CALL)
    {
        if (strlen(numdflt) == 0)
            strcpy(numdflt,"34866");
        printf("\t Phone Number [%s]? ",numdflt);
        i = 0;
        while ((numstr[i++] = getinput()) != '\n');
        numstr[i-1] = '\0';
        if (numstr[0] != '\0')
            strcpy(numdflt,numstr);
   time_amt = prompt("\t Wait how many seconds to connect", "%d", "30");
   if ((time_amt == FALSE) || (time_amt == '.'))
       return(FALSE);
   /* connect_timer is in tenths of seconds */
   startdev.t_dial_data.auto_data.connect_timer = time_amt * 10;
    }
    strcpy(startdev.t_dial_data.auto_data.sig, dflt);
    /* Append phone number if calling side */
    if (startdev.modem_flags & MF_CALL)
        strcat(startdev.t_dial_data.auto_data.sig, numdflt);

    if ( (!ascii_dial) && (startdev.phys_link == PL_V25) )
    {
        for (i=0;i<strlen(startdev.t_dial_data.auto_data.sig);i++)
       startdev.t_dial_data.auto_data.sig[i] =
      conv_to_ebc(startdev.t_dial_data.auto_data.sig[i]);
    }

    if (startdev.phys_link == PL_V25)
    {
        i = y_or_n("\t Use slash to delimit control characters", use_slash);
        if (i < 0) return;
        if (i)
            use_slash = TRUE;
        else
            use_slash = FALSE;
        if (!(use_slash))
        {
            if (startdev.dial_proto == DIAL_PRO_BSC)
            {
           final_str[0] = STX; final_str[1] = NULL;   /* add STX */
                strcat(final_str,startdev.t_dial_data.auto_data.sig);
                i=strlen(startdev.t_dial_data.auto_data.sig);
                final_str[++i]=ETX;
           final_str[++i]=NULL;
                strcpy(startdev.t_dial_data.auto_data.sig, final_str);
            }
            else if (startdev.dial_proto == DIAL_PRO_SDLC)
            {
           final_str[0] = SDLC_MDM_ADDR;
           final_str[1] = SDLC_FNL_CTRL;  /* add SDLC addr, ctrl fields */
           final_str[2] = NULL;
                strcat(final_str,startdev.t_dial_data.auto_data.sig);
                strcpy(startdev.t_dial_data.auto_data.sig, final_str);
            }
        }
        else
        {
            if (startdev.dial_proto == DIAL_PRO_BSC)
            {
           strcpy(final_str, "\\02");  /* add STX */
                strcat(final_str,startdev.t_dial_data.auto_data.sig);
                strcat(final_str,"\\03");    /* add ETX */
                strcpy(startdev.t_dial_data.auto_data.sig, final_str);
            }
            else if (startdev.dial_proto == DIAL_PRO_SDLC)
            {
           strcpy(final_str, "\\ff\\13");  /* SDLC addr & control field*/
                strcat(final_str,startdev.t_dial_data.auto_data.sig);
                strcpy(startdev.t_dial_data.auto_data.sig, final_str);
            }
        }
    }
    if (startdev.dial_proto == DIAL_PRO_ASYNC)
   strcat(startdev.t_dial_data.auto_data.sig, "\r");

    if ((startdev.t_dial_data.auto_data.len = (unsigned short)
        strlen(startdev.t_dial_data.auto_data.sig))>SELECT_SIG_LEN)
        printf("The dial string is too long: data may be corrupted\n");

    printf("Dial string: %s, dial length: %d\n",
   startdev.t_dial_data.auto_data.sig,startdev.t_dial_data.auto_data.len);
    for (i=0; i<startdev.t_dial_data.auto_data.len; i++)
   printf("%02x ",startdev.t_dial_data.auto_data.sig[i]);
    printf("\n");
    return(TRUE);
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

   ttp   = (TP        *)xmitbuf.data;
   rtp   = (TP        *)recvbuf.data;

   /*  For MPQP, setup the character translation for ASCII.  If   */
   /*  Bisync is used, leave room for an STX; if EBCDIC Bisync,   */
   /*  use EBCDIC character translation table.        */

   if (!(startdev.data_flags & DIAL_FLG_BSC_ASC))
       p_xlat = ebc_tbl;
   else
       p_xlat = asc_tbl;
   if ( startdev.data_proto == DATA_PROTO_BSC )
   {
       ttp  = (TP *)(xmitbuf.data + 1);
       rtp  = (TP *)(recvbuf.data + 1);
   }
   tarp  = (TP_ARP    *)ttp->data;
   tparm = (TP_PARMS  *)ttp->data;
   rarp  = (TP_ARP    *)rtp->data;
   rparm = (TP_PARMS  *)rtp->data;
   rres  = (TP_RESULT *)rtp->data;
   xmit_frames = recv_frames = 0;

   printf("\n\t ***********************  CLIENT  ***********************");
   count = prompt("\n\t Send how many test frames", "%d", "1000000");
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
       if (!Transmit(fildes, &xmitbuf, PARM_SIZE)) /* send packet */
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

       /*  For MPQP Client/Server tests, acknowledgements are  */
       /*  sent as alternating DLE ACK0/ACK1 sequences.  If    */
       /*  it is not an ack, then it must be a test results    */
       /*  frame; otherwise it is to be treated as an error.   */

       if (( recvbuf.data[0] == DLE ) &&
           ((recvbuf.data[1] == ACK0 ) ||
       (recvbuf.data[1] == ACK1)))
       {
      ++ack_no;
      if ((( recvbuf.data[1] == ACK0 ) &&  ( ack_no & 1 )) ||
           ( recvbuf.data[1] == ACK1 ) && !( ack_no & 1 ))
      {
             fprintf(stderr, "\n\t ACK out of sequence:");
          if ( ack_no & 1 )
              fprintf(stderr, " Expect ACK1, got ACK0");
          else
              fprintf(stderr, " Expect ACK0, got ACK1");
          ++lost_acks;
          if (++errors >= max_errors)
          {
              Send_Halt( fildes, Server_Addr );
         halt_sent = TRUE;
          } else
              ++ack_no;    /* attempt to recover */
      }
       } else {
           /*  If a test results frame has arrived (either  */
      /*  because we requested a halt or because the  */
      /*  Server has reached the maximum error count),   */
      /*  get the statistics from the frame and exit the */
      /*  test.  Include the test frame in the Server's  */
      /*  transmit frame count (add one).       */

      if ((!strcmp(rtp->dest, CLIENT_PA)) &&
          (rtp->operation == TEST_RESULTS))
      {
          printf("\n\t Test results received.");
          lost_frames  = rres->lost_frames;
          sbad_frames  = rres->bad_frames;
          sxmit_frames = rres->xmit_frames + 1;
          srecv_frames = rres->recv_frames;
          break;     /* exit loop */
      } else {

      /*  Neither ACK nor a test results frame, so it */
      /*  must be a corrupted frame:         */

          fprintf(stderr, "\n\t Corrupted Frame received!");
          save_bad_frame(&recvbuf, result,
         RANDOM_PATTERN, ++bad_frames, TP_HDR_SIZE, Cfile);
          ++ack_no;     /* attempt to recover */
          ++bad_frames;
               if (++errors >= max_errors)
          {
                   Send_Halt(fildes, Server_Addr);
         halt_sent = TRUE;
          }
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
       bzero( &recvbuf, sizeof(recvbuf));
       length = Receive(fildes, &recvbuf, sizeof(recvbuf), CLIENT_TIMEOUT);
       if (length < 0) return (-1);    /* error */

       /*  For MPQP, the acks will toggle between DLE ACK0  */
       /*  and DLE ACK1:                  */

       if ( recvbuf.data[0] == DLE )
           return( recvbuf.data[1] == ACK1 );
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

   /*  If MPQP Bisync, leave room for an STX:         */

   if ( startdev.data_proto == DATA_PROTO_BSC )
       tp  = (TP *)(xmitbuf.data + 1);
   else
       tp   = (TP *)xmitbuf.data;
                  /* Fill in packet: */
   strcpy(tp->src,  CLIENT_PA);     /* My protocol address */
   strcpy(tp->dest, SERVER_PA);     /* his protocol address */
   tp->operation  = TEST_HALT;      /* type of test packet */
   tp->length     = 0;        /* size after header */
   tp->seq_number = 0;        /* null sequence number */
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

   /*  For MPQP, setup the character translation for ASCII.  If   */
   /*  Bisync is used, leave room for an STX; if EBCDIC Bisync,   */
   /*  use EBCDIC character translation table.        */

   if (!(startdev.data_flags & DIAL_FLG_BSC_ASC))
       p_xlat = ebc_tbl;
   else
       p_xlat = asc_tbl;
   if ( startdev.data_proto == DATA_PROTO_BSC )
   {
       ttp  = (TP *)(xmitbuf.data + 1);
       rtp  = (TP *)(recvbuf.data + 1);
   }
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
   xmitbuf.data[0] = DLE;
   xmitbuf.data[1] = (ack_number & 1) ? ACK1 : ACK0;
   return (Transmit( fildes, &xmitbuf, 2 ));
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

Transmit (fildes, packet, length)
   int      fildes;
   char     *packet;
   unsigned int   length;
{
   struct  mp_write_extension ext;
   int   retry = 0, result;

   length = MAX( MIN_PACKET, length ); /* normalize length */
   bzero(&ext, sizeof(ext));

   /*  If in kernel mode and mbufs are not freed by the driver */
   /*  after writes, then we must get status on every write so */
   /*  that we (specifically KID) will know when to free mbufs.   */

   ext.cio_write.flag |= CIO_ACK_TX_DONE;
   if (!profile.free_mbuf)
       ext.cio_write.flag |= CIO_NOFREE_MBUF;
   if (( startdev.data_proto == DATA_PROTO_BSC ) &&
       ( length > 2 ))        /* if Bisync text block, */
   {
       ext.transparent = TRUE;      /* use transparent mode */
       packet[0] = STX; length++;      /* add start of text */
       packet[length++] = ETX;      /* and end of text delimiters */
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
           if (ext.cio_write.flag & CIO_ACK_TX_DONE)
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

      /*  For MPQP Bisync, control messages are returned as */
      /*  message types in the read extension rather than as   */
      /*  data in the receive buffer.  To restore symmetry, */
      /*  map the message type back to the data that was */
      /*  discarded:                */

           if ( startdev.data_proto == DATA_PROTO_BSC )
          result = decode_bsc_msg( ext.status, packet, result );
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
   int   result;

   if ( started )          /* if the port is already */
       return(TRUE);       /* started, just return */
   startdev.mpqp_session.netid = protocol;
   startdev.mpqp_session.status = 0;
   result = IOCTL( fildes, CIO_START, &startdev );
   if (result < 0) {       /* if error */
       if (errno == EADDRINUSE)     /* if already issued */
      return(TRUE);        /* then no problem */
       switch (errno) {       /* else, no can do */
      case EIO:
             fprintf(stderr, "\n\t ERROR: ");
             switch (startdev.mpqp_session.status) {
         case CIO_HARD_FAIL:
            fprintf(stderr, "CIO_HARD_FAIL");
            break;
         default:
            fprintf(stderr, "Undefined status %d",
                   startdev.mpqp_session.status);
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
       if (result = wait_status(fildes, CIO_START_DONE, STATUS_TIMEOUT))
      return(++started);
       else
      return(result);
   }
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
