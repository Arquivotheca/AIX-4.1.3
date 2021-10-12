static char sccsid[] = "@(#)07 1.1 src/bos/usr/fvcioem/ddt/fddi/ddt_com.c, fvcioem, bos411, 9428A410j 4/26/94 13:54:43";

/*--------------------------------------------------------------------------
*
*             DDT_COM.C
*
*  COMPONENT_NAME:  Communications Device Driver Tool Common Code.
*
*  FUNCTIONS: open_test, close_test, status_test, getinput, prompt,
*        y_or_n, get_buf_length, fill_buffer, hexstring_to_array,
*             decode_error, print_binary, print_buffer, save_bad_frame,
*        show_session_blk, decode_vpd, wait_status, generate_pattern,
*        verify_pattern, bzero
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
# include <unistd.h>
# include <sys/stat.h>
# include <sys/errno.h>
# include <sys/err_rec.h>
# include <sys/types.h>
# include <sys/poll.h>
# include <sys/devinfo.h>
# include <sys/comio.h>
# include <sys/fddiuser.h>
# include <fddikiduser.h>
# include "ddt_com.h"
#include <fddibits.h>

/*----------------------------------------------------------------------*/
/*          Global Variables           */
/*----------------------------------------------------------------------*/

/*  Shared from there:                    */

extern         errno;
extern unsigned char Server_Addr[];    /* hardware address of Server */
extern unsigned char Client_Addr[];    /* hardware address of Client */
extern char    Cfile[];    /* saved client frames */
extern char    Sfile[];    /* saved server frames */
extern FILE    *recordfp;     /* saved input key strokes */
extern int     using_tty;     /* is stdout a tty ? */

/*----------------------------------------------------------------------*/
/*  Shared from here:                     */

int         block;         /* block mode flag */
int         kern;       /* kernel mode flag */
int         pattern;    /* type of test pattern */
int         started;    /* number of starts */
PROFILE        profile;    /* test param defaults */
char        str[30];    /* scratch string area */
char        path[30];
char        unit;       /* port or device number */
DRVR_OPEN      dopen;         /* open block for kid driver */
int         openstate;     /* open flag */
unsigned int      recv_frames;      /* total # recv frames */
unsigned int      xmit_frames;      /* total # xmit frames */
char        kidpath[] = KIDSPECIAL; /* kernel interface drvr path */
char        *convcode();


/*------------------------   O P E N _ T E S T   -----------------------*/
/*                         */
/*  NAME: open_test                    */
/*                         */
/*  FUNCTION:                       */
/* Issues an open to the driver specified by "devpath".  The   */
/* user is prompted for responses to configuration of several  */
/* parameters affecting the open, including read/write flags,  */
/* diagnostic open, and open thru the Kernel Interface Driver. */
/*                         */
/*  EXECUTION ENVIRONMENT:                */
/* Can only be executed at the user process level.       */
/*                         */
/*  DATA STRUCTURES:                   */
/* Modifies the global block mode flag and the dopen struct.   */
/* Sets path to the actual pathname opened.        */
/*                            */
/*  RETURNS:                              */
/* File descriptor of the opened driver if successful, or      */
/* -1 if an error occurs.                 */
/*                         */
/*----------------------------------------------------------------------*/

extern   char     drvpath[];

open_test (devpath)
   char  *devpath;
{
   int      i, fildes, error;
   struct stat fstat;
   char     mode = 0;

   unit = 0;
   strcpy(path, devpath);
   printf("\n\t **********************   OPEN   ************************");
   sprintf(str, "\n\t Open %s0, 1, 2, 3, 4, 5, 6, or 7? ", devpath);
   i = prompt(str, "%d", "0");
   if ( i < 0 ) return;
   unit = i;
   sprintf(drvpath, "%s%d", devpath, unit);
   if ( statx(drvpath, &fstat, sizeof(fstat), 0 ))
   {
       fprintf(STDERR, "\n\t There is no %s!", drvpath);
       fildes = -1;
       goto exit;
   }
   dopen.devno = fstat.st_rdev;
   kern = y_or_n("\t Open in kernel mode (through /dev/kid)", FALSE);
   if (kern)
   {
      if ( kern < 0 )
      {
         return(-1);
      }
      if ( statx(kidpath, &fstat, sizeof(fstat), 0 ))
      {
         fprintf(STDERR, "\n\t There is no %s!", kidpath);
         fildes = -1;
         goto exit;
      }
   }
   sprintf(str, "%d", unit);
   strcat(path, str);
   while (!mode) {
       i = prompt("\t Read/Write (R), or Diagnostic mode (D)",
               "%c", "R");
       if ( i < 0 ) return(-1);
       mode = i;
       switch (mode) {
      case 'r':
      case 'R':
          strcpy(dopen.channame, "");
          break;
      case 'd':
      case 'D':
          strcat(path, "/D");
          strcpy(dopen.channame, "D");
          break;
      case 'w':
      case 'W':
          strcat(path, "/W");
          strcpy(dopen.channame, "W");
          break;
      default: mode = 0;
          break;
       }
   }
   /*  Set flags to read/write.  If user desires no blocking mode,   */
   /*  turn on the NDELAY flag.              */

   dopen.flags = O_RDWR;
   block = y_or_n("\t Open in blocking mode (NDELAY flag off)", FALSE);
   if ( block < 0 ) return(-1);
   if ( !block )
       ( dopen.flags |= O_NDELAY );

   /*  If kernel mode testing, try to open the Kernel Interface   */
   /*  Driver.  If successful, try to issue an IOCTL to open the  */
   /*  driver under test; if no luck, close the file and return    */
   /*  an error.                    */

   if (kern) {
       fildes = open( kidpath, dopen.flags );
       if (fildes < 0)
       {
      error = errno;
      fprintf(STDERR, "\n\t I can't open %s!", kidpath);
       } else {
         if (IOCTL( fildes, KID_OPEN_DRVR, &dopen ) < 0)
      {
          error = errno;
          fprintf(STDERR, "\n\t I can't open %s in kernel mode!",
         path);
          close(fildes);
          fildes = -1;
           }
       }
   } else {
       fildes = open( path, dopen.flags );
       error = errno;
   }
   if (fildes < 0) {       /* if error, */
       fprintf(STDERR, "\n\t ");
       decode_error(error);
   } else {
       if (kern)           /* set kid mbuf threshold */
       {
      IOCTL( fildes, KID_MBUF_THRESH, atoi(profile.mbuf_thresh));
      /*
       * set fastwrite path between kid and fddi driver
       */
      IOCTL(fildes, KID_SET_FASTWRT, 0);
       }
       printf("\n\tStatus: Open succeeded");
   }
    exit:
   printf("\n\t ********************************************************");
   return(fildes);
}
/*
 * Max open test: see how many opens we can do before we get an error
 * Prompt for max opens to try.
 *
 * Default: use current defaults and user mode
 */
int
max_open_test ()
{
   char  path[64];
   int   *fd_tab;
   int   fd_size;
   int   i;
   int   fd;
   int   max;

   printf("\n\t *****************  MAX OPEN TEST  *******************");
   sprintf(str, "\n\t Open /dev/fddi0, 1, 2, 3, 4, 5, 6, or 7? ");
   i = prompt(str, "%d", "0");
   if (i < 0)
   {
      return;
   }
   sprintf(path, "/dev/fddi%d", i);
   max = prompt("\n\tHow many opens?", "%d", "100");
   /*
    * do opens
    */
   fd_size = max*4;

   fd_tab = (int *) malloc (fd_size);

   for (i = 0; i < max ; i++)
   {
      fd_tab[i] = open (path, O_RDWR | O_NDELAY);
      if (fd_tab[i] < 0)
      {
         printf("fd = %d (errno %d)\n",fd_tab[i],errno);
         break;
      }
   }
   printf ("\tSuccessfully Opened %s %d times\n", path, i);
   while (i--)
   {
      close (fd_tab[i]);
   }
   if (fd_tab)
   {
      free (fd_tab);
   }
   printf("\n\t *****************************************************");
   return (0);
}
/*-----------------------   C L O S E _ T E S T   ----------------------*/
/*                         */
/*  NAME: close_test                   */
/*                         */
/*  FUNCTION:                       */
/* Closes the driver under test specified by fildes.  If the   */
/* driver is opened thru KID, an ioctl is issued to KID to     */
/* close the driver under test, then KID is closed.      */
/*                         */
/*  EXECUTION ENVIRONMENT:                */
/* Can only be executed at the user process level.       */
/*                         */
/*  DATA STRUCTURES:                   */
/* Modifies the global block mode flag and the dopen struct.   */
/* Sets path to the actual pathname opened.        */
/*                            */
/*  RETURNS:                              */
/* TRUE  The driver was closed successfully.       */
/* FALSE An error occurred while trying to close the driver.   */
/*                         */
/*----------------------------------------------------------------------*/

close_test (fildes)
   int   fildes;
{
   int   result = 0, rc = TRUE;
   printf("\n\t ***********************  CLOSE  ************************");
   /*                      */
   /*  If in kernel mode test, close the driver under test first   */
   /*  before closing the Kernel Interface Driver; otherwise close */
   /*  the driver under test.             */
   /*                      */
   if (kern)
       result = IOCTL( fildes, KID_CLOSE_DRVR, &dopen );
   if (result == 0)
       result = close( fildes );
   if (result < 0) {
       fprintf(STDERR, "\n\t ");
       decode_error(errno);
       rc = FALSE;
   } else {
       dsclose();          /* do any ds close stuff */
       printf("\n\tStatus: Close succeeded");
   }
   printf("\n\t ********************************************************");
   return(rc);
}

/*-----------------------  S T A T U S _ T E S T  ----------------------*/
/*                         */
/*  NAME: status_test                     */
/*                         */
/*  FUNCTION:                       */
/* Issues a CIO_GET_STAT ioctl to the driver under test and */
/* displays the returned status block (even if it is a "null"  */
/* status block).  For Ethernet drivers, the Ethernet address  */
/* returned in the status block is saved in the profile block. */
/*                         */
/*  EXECUTION ENVIRONMENT:                */
/* Can only be executed at the user process level.       */
/*                         */
/*  DATA STRUCTURES:                   */
/* Modifies the global profile block (Ethernet).         */
/*                            */
/*  RETURNS: nothing                      */
/*                         */
/*----------------------------------------------------------------------*/

status_test (fildes)
   int   fildes;
{
   struct status_block  status;
   char  *byte;
   int   i, result;

   printf("\n\t **********************  GET STATUS  ********************");
   result = IOCTL( fildes, CIO_GET_STAT, &status );
   if (result < 0)   {     /* if error */
       fprintf(STDERR, "\n\t ");
       decode_error(errno);
   } else {
       printf("\n\tStatus: Getstatus succeeded: ");
       decode_status(&status, stdout);
   }
   printf("\n\t ********************************************************");
}

/*--------------------------  G E T I N P U T  -------------------------*/
/*                         */
/*  NAME: getinput                     */
/*                         */
/*  FUNCTION:                       */
/* Gets and returns a character from stdin, filtering out any  */
/* comment lines (starting with "#" and ending with a newline  */
/* character).  If a 'Q' or 'q' is entered (quit), exit is  */
/* called.                       */
/*                         */
/*  EXECUTION ENVIRONMENT:                */
/* Can only be executed at the user process level.       */
/*                         */
/*  DATA STRUCTURES:                   */
/* References no global data structures.           */
/*                            */
/*  RETURNS:                              */
/* Character received from standard in.            */
/*                         */
/*----------------------------------------------------------------------*/

char
getinput()
{
   register int   c;
   extern int  using_tty;

   while ((c = getchar()) == '#')      /* if comment, throw out */
       while ((c = getchar()) != '\n');   /* the whole line */
   if ((c == 'q') || (c == 'Q'))
   {
      fprintf (stdout,"\n");
      fprintf (STDERR,"\n");
         exit(0);       /* if quit, do so */
   }
   /*
    * record all the input key strokes
    */
   putc (c, recordfp);
   return(c);
}

/*----------------------------  P R O M P T  ---------------------------*/
/*                         */
/*  NAME: prompt                    */
/*                         */
/*  FUNCTION:                       */
/* Displays prompt "string" with default response "deflt"; "type"    */
/* is a string that describes the type of input (scanf style);     */
/* "%x" for hex input, "%d" for decimal, or "%c" for character.   */
/*                         */
/*  EXECUTION ENVIRONMENT:                */
/* Can only be executed at the user process level.       */
/*                         */
/*  DATA STRUCTURES:                   */
/* References no global data structures.           */
/*                            */
/*  RETURNS:                              */
/* Numeric or character value of input.            */
/* "." or -1 means abort.                 */
/*                         */
/*----------------------------------------------------------------------*/

prompt (
   char  *string,
   char  *intype,
   char  *deflt)
{
   int   rc, i = 0;
   char  *srcstr, str[80];

   printf("%s [%s]? ", string, deflt);
   while ((str[i++] = getinput()) != '\n');
   if ( str[0] == '.' ) return (-1);
   str[i-1] = '\0';
   srcstr = str;
   if (str[0] == '\0')  /* Default */
       srcstr = deflt;  /* Single Character: */
   if (!strcmp("%c", intype))
       rc = srcstr[0];
   else           /* Number: */
       sscanf(srcstr, intype, &rc);
   return(rc);
}

/*----------------------------  Y _ O R _ N  ---------------------------*/
/*                         */
/*  NAME: y_or_n                    */
/*                         */
/*  FUNCTION:                       */
/* Prompts STRING for Yes/No response; default value is "Y" if */
/* DEFLT is TRUE, "N" if DEFLT is FALSE.           */
/*                         */
/*  EXECUTION ENVIRONMENT:                */
/* Can only be executed at the user process level.       */
/*                         */
/*  DATA STRUCTURES:                   */
/* References no global data structures.           */
/*                            */
/*  RETURNS:                              */
/* TRUE  for "Yes"                  */
/* FALSE for "No"                */
/* -1 for abort                  */
/*                         */
/*----------------------------------------------------------------------*/

y_or_n (
   char  *string,
   int   deflt)
{
   char  response = FALSE;

   while (!response) {
       printf("%s (Y/N) [", string);
       if (deflt)
      printf("Y");
       else
      printf("N");
       printf("]? ");
       response = getinput();
       if (response == '\n')
      return(deflt ? TRUE : FALSE);
       else
      getinput();
       switch (response) {
      case 'y':
      case 'Y':
          return(TRUE);
      case 'n':
      case 'N':
          return(FALSE);
      case '.':
          return(-1);
      default:
          response = FALSE;
       }
   }
}

/*-----------------  H E X S T R I N G _ T O _ A R R A Y  --------------*/
/*                         */
/*  NAME: hexstring_to_array                 */
/*                         */
/*  FUNCTION:                       */
/* Converts a string of ASCII hexadecimal characters to an array  */
/* of bytes.  The size of the array must be at least half the  */
/* length of the source string.  The string is first checked to   */
/* ensure that it contains only hex characters (0-1, a-f, or A-F);   */
/* the string is then converted and written to the buffer.  This  */
/* routine is useful for converting strings that will not fit        */
/* into an int (or smaller), such as Ethernet and Token Ring   */
/* addresses.                    */
/*                         */
/*  EXECUTION ENVIRONMENT:                */
/* Can only be executed at the user process level.       */
/*                         */
/*  DATA STRUCTURES:                   */
/* References no global data structures.           */
/*                            */
/*  RETURNS:                              */
/* TRUE  The string is converted.               */
/* FALSE The string contained an invalid character.      */
/*                         */
/*----------------------------------------------------------------------*/

hexstring_to_array (
   char     *string,
   unsigned char  *array)
{
   register int   i, k, nibble;

   for (i = 0; i < strlen(string); i = i + 2 )
   {
       for (k = 0; k < 2; k++)
       {
            switch (string[i + k])
      {
          case '0': nibble = 0;
           break;
          case '1': nibble = 1;
           break;
          case '2': nibble = 2;
           break;
          case '3': nibble = 3;
           break;
          case '4': nibble = 4;
           break;
          case '5': nibble = 5;
           break;
          case '6': nibble = 6;
           break;
          case '7': nibble = 7;
           break;
          case '8': nibble = 8;
           break;
          case '9': nibble = 9;
           break;
          case 'a':
          case 'A': nibble = 10;
           break;
          case 'b':
          case 'B': nibble = 11;
           break;
          case 'c':
          case 'C': nibble = 12;
           break;
          case 'd':
          case 'D': nibble = 13;
           break;
          case 'e':
          case 'E': nibble = 14;
           break;
          case 'f':
          case 'F': nibble = 15;
           break;
          default:
           return(FALSE);
       }
      if (k == 0)
          array[i >> 1] = nibble << 4;
      else
          array[i >> 1] = array[i >> 1] | nibble;
       }
   }
   return(TRUE);
}

/*-----------------------  D E C O D E _ E R R O R  --------------------*/
/*                         */
/*  NAME: decode_error                    */
/*                         */
/*  FUNCTION:                       */
/* Looks up "errcode" in the system error list table and prints   */
/* its meaning to the display.               */
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

extern int  errno;
extern int  sys_nerr;
extern char *sys_errlist[];

decode_error (
   int   errcode)
{
   fprintf (STDERR, "\n\tStatus: ERROR: %s: %d",
      (errcode > sys_nerr) ? "Unknown error" : sys_errlist[errcode],
      errcode);
}


/*----------------------  P R I N T _ B I N A R Y  ---------------------*/
/*                         */
/*  NAME: print_binary                    */
/*                         */
/*  FUNCTION:                       */
/* Prints LEN size NUMBER (bits) in binary.        */
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

print_binary (number, len)
   int   number;
   short len;
{
   int   i;

   if (len <= 0)
       return;
   for ( i = (len - 1); i >= 0; i-- )
   {
       if ( number & (1 << i))
           printf("1");
       else
           printf("0");
       if (!((len - i) % 8 ))
      printf(" ");
   }
   if ( len <= 4 )
       printf(" (%.1x hex)", number);
   else if (len <= 8 )
       printf(" (%.2x hex)", number);
   else if (len <= 16 )
       printf(" (%.4x hex)", number);
   else
       printf(" (%.8x hex)", number);
}

/*----------------------  P R I N T _ B U F F E R  ---------------------*/
/*                         */
/*  NAME: print_buffer                    */
/*                         */
/*  FUNCTION:                       */
/* Prints the contents of BUFFER to "to_where" a la the kernel */
/* debugger.  LENGTH is the number of bytes to show, and OFFSET   */
/* is the starting address to be printed on the left hand side.   */
/* The output goes to the display if "to_where" is set to stdout. */
/* Alternately, to_where can be a file pointer for output to a */
/* file.                      */
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

print_buffer (char   *buffer,
         int   length,
         int   offset,
         int   to_where)
{
   int   i, j, k, limit, line = 0;
   char  c;

   limit = (length + 15) & ~0xF;    /* next highest mult of 16 */
   for (i = 0; i <= limit; i++) {
       if (!( i % 16 )) {        /* mult of 16? */
      if (i) {       /* not zeroth byte? */
          fprintf(to_where, " ");   /* print ASCII of last 16 */
          for (j = i - 16; j < i; j++) {
         if (j >= length)  /* only up to the length */
             fprintf(to_where, " ");
         else        /* control chars = "." */
                       if (buffer[j] < 0x20 || buffer[j] > 0x7e)
                      fprintf(to_where, ".");
                  else
                      fprintf(to_where, "%c", buffer[j]);
          }
      }
      if (i != limit) {    /* no label at limit */
             if (line++ > 20) {     /* hold screen? */
              if (to_where == stdout) {
             line = 0;
             k = prompt("\n\t Forward or Backward",
            "%c", "F");
             if (k < 0) return(k);
             c = k;
             if ( c == 'b' || c == 'B' )
            i = MAX( i - 704, 0 );
         }
          }          /* start next line: */
          fprintf(to_where, "\n\t %.4x:", offset + i);
      }
       }
       if (!( i % 4 ))        /* add space every */
      fprintf(to_where, " ");    /* four bytes */
       if (i >= length)
      fprintf(to_where, "  ");   /* print hex only up to */
       else          /* length */
           fprintf(to_where, "%.2x", buffer[i]);
   }
   if (to_where == stdout) {
       printf("\n\t  Press Enter to continue ");
       c = getinput();
       if (c == '.') {
      getinput();
      return(-1);
       }
   }
}

/*---------------------  S A V E _ B A D _ F R A M E  ------------------*/
/*                         */
/*  NAME: save_bad_frame                  */
/*                         */
/*  FUNCTION:                       */
/* Formats the data in "buf" to the file "filepath".  The data */
/* is formatted with the print_buffer routine (above) and      */
/* appended to the end of the file if it exists; if it does not   */
/* exist, the file is created first.            */
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

save_bad_frame (
   char     *buffer,
   int      length,
   int      type,
   int      id,
   int      hdrsize,
   char     *filepath)
{
   FILE  *fp;

   /*  Open the file in append mode (create if not there), and */
   /*  write some identifying header information.        */

   if ((fp = fopen(filepath, "a")) == NULL)
   {
       fprintf(STDERR, "\n\t %s file open failed", filepath);
       exit(1);
   }
   if (id == 1)            /* first entry? */
       fprintf(fp, "\n\n Bad frames for this test:");
   fprintf(fp, "\n\n Frame %d, size %d (%xh) data bytes:",
       id, length, length);      /* show errors: */
   /*                      */
   /*  Write the discrepencies to the file, followed by the data  */
   /*  in the buffer.                  */
   /*                      */
   verify_pattern( buffer + hdrsize,
       length - hdrsize, hdrsize, type, fp );
   print_buffer( buffer, length, 0, fp ); /* save buffer */
   fprintf(fp, "\n");         /* end line */
   fflush(fp);          /* empty stream */
   fclose(fp);          /* close file */
   return;
}

/*------------------  S H O W _ S E S S I O N _ B L K  -----------------*/
/*                         */
/*  NAME: show_session_blk                */
/*                         */
/*  FUNCTION:                       */
/* Displays the contents of the session block pointed                */
/* to by "spb" to the output specified by "to_where".    */
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

show_session_blk (
   struct session_blk   *sbp,
   int          to_where)
{
   fprintf(to_where, "\n\tSession Block:");
      fprintf(to_where, "\n\t   Status code: %s", convcode (sbp->status));
      fprintf(to_where, "\n\t   Netid:        %.4x", sbp->netid);
}

/*------------------------  D E C O D E _ V P D  -----------------------*/
/*                         */
/*  NAME: decode_vpd                   */
/*                         */
/*  FUNCTION:                       */
/* Decodes and formats the Vital Product Data encoded in the   */
/* string "vpd" to stdout.                */
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

decode_vpd (vpd)
   unsigned char  *vpd;
{
   int      i, j, hex, skip, len, entrylen;
   char     c;

   printf("\n\t Vital Product Data Structure:");
   printf("\n\t    Label = ");
   for (i = 0; i < 3; i++)       /* should print 'VPD' */
   {
       if (vpd[i] < 0x20 || vpd[i] > 0x73)
       printf("0x%.2x", vpd[i]);
       else printf ("%c", vpd[i]);
   }
   len = (((vpd[3] << 8) | vpd[4]) << 1); /* read length field * 2 */
   printf("\n\t    Length in bytes = %d", len);
   len = len + 7;          /* add 'VPD', length, and CRC */
   printf("\n\t    CRC field = 0x%.2x%.2x", vpd[5], vpd[6]);
   if (len > 256) len = 256;     /* don't go over edge */
   for (i = 7; i < len; i++)
   {
       hex = skip = FALSE;
       if (vpd[i] == '*')        /* found an entry */
       {
         switch (vpd[ i + 1 ])
      {
               case 'N':
            printf("\n\t    Network Address: ");
            hex = TRUE;
            break;
          case 'D':
              switch (vpd[ i + 2 ]) {
             case 'S':
                  printf("\n\t    Description of Product: ");
hex=TRUE;
               break;
             case 'D':
               printf("\n\t    Device Driver Level: ");
               break;
             case 'G':
               printf("\n\t    Diagnostic Level: ");
               break;
             default:
               skip = TRUE;
               break;
            }
            break;
          case 'E':
            printf("\n\t    Engineering Change Level: ");
            break;
          case 'F':
            printf("\n\t    IBM FRU Number: ");
            break;
          case 'M':
            printf("\n\t    Manufacturer Location: ");
            break;
          case 'P':
            printf("\n\t    Part Number: ");
            break;
          case 'S':
            printf("\n\t    Serial Number: ");
            break;
          case 'R':
            printf("\n\t    ROS Level: ");
            break;
          case 'L':
            printf("\n\t    Loadable Microcode Level: ");
            break;
          case 'C':
            printf("\n\t    Card ID: ");
            break;
          case 'A':
            printf("\n\t    Address Field: ");
            break;
          default:
              skip = TRUE;
              break;
           }
      if (!skip)
      {
          entrylen = (vpd[ i + 3 ] << 1) - 4;
          for (j = 0; j < entrylen; j++)
          {
            if (hex) {
             printf("%.2x", vpd[ i + 4 + j ]);
            } else {
             c = vpd[ i + 4 + j ];
             if (c < 0x20 || c > 0x7e) c = '.';
             printf("%c", c);
            }
          }
      }
       }
   }
}

int
hexdump (data, len)
char *data;
long len;
{
   int   i,j,k;
   char  str[18];

   fprintf(stderr,"hexdump(): length=%ld\n",len);
   i=j=k=0;
   while(i<len)
   {
      j=(int) data[i++];
      if(j>=32 && j<=126)
         str[k++]=(char) j;
      else
         str[k++]='.';
      fprintf(stderr,"%02x ",j);
      if(!(i%8))
      {
         fprintf(stderr,"  ");
         str[k++]=' ';
      }
      if(!(i%16))
      {
         str[k]='\0';
         fprintf(stderr,"     %s\n",str);
         k=0;
      }
   }
   while(i%16)
   {
      if(!(i%8))
         fprintf(stderr,"  ");
      fprintf(stderr,"   ");
      i++;
   }
   str[k]='\0';
   fprintf(stderr,"       %s\n",str);
   fflush(stderr);
   return ;
}

get_status_test (fildes)
   int   fildes;
{
   struct status_block  status;
   struct pollfd     pollblk;
   int         i, rc, result;
   unsigned char     *opt;
   char        *byte;

   printf("\n\t ******************   GET Status    ********************");
   result = IOCTL( fildes, CIO_GET_STAT, &status );
   if (result < 0)
   {
       fprintf(STDERR, "\n\tGet status failed: ");
       decode_error(errno);
       rc = FALSE;
   }
   else
   {
       rc = TRUE;       /* succeeded */
       decode_status(&status, stdout);
   }
   printf("\n\t ********************************************************");
   return(rc);
}
/*-----------------------  W A I T _ S T A T U S  ----------------------*/
/*                         */
/*  NAME: wait_status                     */
/*                         */
/*  FUNCTION:                       */
/* Polls the status queue of the driver under test until a     */
/* status block with code equal to "code" is found or until */
/* the wait for a status block times out.  All status blocks   */
/* received before the expected one are thrown out.      */
/* If any status block is to be accepted, code = CIO_STATUS_MAX.  */
/*                         */
/*  EXECUTION ENVIRONMENT:                */
/* Can only be executed at the user process level.       */
/*                         */
/*  DATA STRUCTURES:                   */
/*                            */
/*  RETURNS:                              */
/* TRUE    The expected status block was received.       */
/* FALSE The wait for the expected status timed out or error.  */
/*                         */
/*----------------------------------------------------------------------*/

wait_status (fildes, code, timeout_val, verbose)
   int   fildes;
   int   code;
   int   timeout_val;
   int   verbose;
{
   struct status_block  status;
   struct pollfd     pollblk;
   int         i, rc, result;
   unsigned char     *opt;
   char        *byte;
   int         doublecheck = 0;

   while (TRUE)
   {
       /*  Wait for a status block by polling for it:    */

       pollblk.fd = fildes;
       pollblk.reqevents = POLLPRI; /* wait for exception */
       pollblk.rtnevents = 0;
       result = poll( &pollblk, 1, timeout_val );
       if (result < 0)
       {
           fprintf(STDERR, "\n\tGet status: POLL failed ");
      decode_error(errno);
      return(FALSE);
       }
       if (result == 0)
       {
      if (!kern)
      {
         fprintf(STDERR, "\n\tWait for status timed out ... ");
         return (FALSE);
      }
      /*
       * in kernel mode we must deal with a bug in the kid driver...
       * it should have exclusive access to the status queue
       * from the kid_select routine but it doesn't and we
       * may have just missed an event that did indeed happen.
       * Just go check it again.
       */
      if (++doublecheck > 3)
      {
         return (FALSE);
      }
      continue;
       }
       else
       {
      /*                   */
      /*  Status block is available -- issue ioctl to get it.  */
      /*                   */
           result = IOCTL( fildes, CIO_GET_STAT, &status );
           if (result < 0)
      {
          decode_error(errno);
          decode_status(&status, STDERR);
          rc = FALSE;
          break;
           }
      /*  Is this the code we are waiting for?  If not, loop   */
      /*  to next wait.             */

      if ((( code == CIO_STATUS_MAX ) &&
           ( status.code != CIO_NULL_BLK )) ||
          ( code == status.code ))
      {
          if ( status.option[0] == CIO_OK )
          {
              rc = TRUE;         /* succeeded */
              if (verbose) decode_status(&status, stdout);
         break;
          } else {
              rc = FALSE;        /* failed */
              decode_status(&status, STDERR);
          }
          break;
      } else
          continue;           /* next block */
       }
   }
   return(rc);
}


/*-----------------  G E N E R A T E _ P A T T E R N  ------------------*/
/*                         */
/*  NAME: generate_pattern                */
/*                         */
/*  FUNCTION:                       */
/* Generates a test pattern into "buffer" depending on the type:  */
/*     ONES_PATTERN  Sets all bytes in the buffer to 0xFF.  */
/*     ZEROES_PATTERN   Sets all bytes in the buffer to 0x00.  */
/*     MODULO_256_PATTERN  Sets each byte to the value of its  */
/*              index modulo 256.         */
/*     MODULO_128_PATTERN  Sets each byte to the value of its  */
/*              index modulo 128.         */
/*     RANDOM_PATTERN   Sets all bytes to a random value */
/*              between 0x00 and 0xFF.    */
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

generate_pattern (
   char     *buffer,
   int      length,
   int      type)
{
   int   i;

   switch (type)
   {
       case ONES_PATTERN:     /* all ones (F's) */
      for (i = 0; i < length; i++)
          buffer[i] = 0xFF;
      break;
       case MODULO_128_PATTERN:  /* incrementing pattern */
      for (i = 0; i < length; i++)
          buffer[i] = (char)(i % 128);
      break;
       case MODULO_256_PATTERN:  /* incrementing pattern */
      for (i = 0; i < length; i++)
          buffer[i] = (char)i;
      break;
       case RANDOM_PATTERN:   /* random numbers */
      for (i = 0; i < length; i++)
          buffer[i] = (char)(random() & 0xFF);
      break;
       case ZEROES_PATTERN:   /* all zeroes */
      bzero(buffer, length);
      break;
       default:
      break;
   }
}

/*-------------------  V E R I F Y _ P A T T E R N  --------------------*/
/*                         */
/*  NAME: verify_pattern                  */
/*                         */
/*  FUNCTION:                       */
/* Recreates the pattern generated by generate_pattern and  */
/* verifies that every byte in "buffer" follows this pattern.  */
/* If a discrepancy is found, an error message is printed to   */
/* "to_where" (stdout, STDERR, or file) describing the location   */
/* and nature of the discrepancy.               */
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

verify_pattern (
   char  *buffer,
   int   length,
   int   hs,
   int   type,
   int   to_where )
{
   register int   i, passed = TRUE, errors = 0;

   switch (type)
   {
       case ONES_PATTERN:     /* all ones (F's) */
      for (i = 0; i < length; i++)
             if (buffer[i] != 0xFF)
          {
         passed = FALSE;
         fprintf(to_where, "\n\t Error: byte %d is 0x%.2x,",
             i + hs, buffer[i]);
         fprintf(to_where, " should be 0xFF!");
         if (++errors == ERROR_LIMIT)
             break;
          }
      break;

       case MODULO_256_PATTERN:  /* incrementing pattern */
      for (i = 0; i < length; i++)
             if (buffer[i] != (char)i)
          {
         passed = FALSE;
         fprintf(to_where, "\n\t Error: byte %d is 0x%.2x,",
             i + hs, buffer[i]);
         fprintf(to_where, " should be 0x%.2x!", (char)i);
         if (++errors == ERROR_LIMIT)
             break;
          }
      break;

       case MODULO_128_PATTERN:  /* incrementing pattern */
      for (i = 0; i < length; i++)
             if (buffer[i] != (char)(i % 128))
          {
         passed = FALSE;
         fprintf(to_where, "\n\t Error: byte %d is 0x%.2x,",
             i + hs, buffer[i]);
         fprintf(to_where, " should be 0x%.2x!", (char)i);
         if (++errors == ERROR_LIMIT)
             break;
          }
      break;

       case ZEROES_PATTERN:   /* all zeroes */
      for (i = 0; i < length; i++)
             if (buffer[i] != 0x00)
          {
         passed = FALSE;
         fprintf(to_where, "\n\t Error: byte %d is 0x%.2x,",
             i + hs, buffer[i]);
         fprintf(to_where, " should be 0x00!");
         if (++errors == ERROR_LIMIT)
             break;
          }
      break;

       default:         /* all others non-reconstructable */
      break;
   }
   return (passed);     /* any errors? */
}

/*  This should be available, but isn't - sigh . . .  */

bzero (addr, size)
   register char  *addr;
   register int   size;
{
   while (size--) *addr++ = 0;
}
int
getdevno ()
{
   return (dopen.devno);
}
int
decode_write_ext (
   cio_write_ext_t      *p_ext)
{
   printf ("\n\tWrite Extension:\n");
   printf ("\tstatus:   %s\n", convcode(p_ext->status));
   printf ("\twrite id:    %d\n", p_ext->write_id);

   return (1);
}

char *
convcode (code)
   int   code;
{
   char     *p;
   static char tmp[64];

      switch (code)
   {
      case CIO_NETID_DUP:
         p = "CIO_NETID_DUP";
         break;
      case CIO_NETID_FULL:
         p = "CIO_NETID_FULL";
         break;
      case FDDI_ADAP_CHECK:
         p = "FDDI_ADAP_CHECK";
         break;
      case FDDI_MC_ERROR:
         p = "FDDI_MC_ERROR";
         break;
      case FDDI_CMD_FAIL:
         p = "FDDI_CMD_FAIL";
         break;
      case FDDI_PORT_EVENT:
         p = "FDDI_PORT_EVENT";
         break;
      case FDDI_REMOTE_DISCONNECT:
         p = "FDDI_REMOTE_DISCONNECT";
         break;
      case FDDI_LLC_DISABLE:
         p = "FDDI_LLC_DISABLE";
         break;
      case FDDI_LLC_ENABLE:
         p = "FDDI_LLC_ENABLE";
         break;
      case FDDI_REMOTE_SELF_TEST:
         p = "FDDI_REMOTE_SELF_TEST";
         break;
      case FDDI_REMOTE_T_REQ:
         p = "FDDI_REMOTE_T_REQ";
         break;
      case FDDI_PIO_FAIL:
         p = "FDDI_PIO_FAIL";
         break;
      case FDDI_RCV_ERROR:
         p = "FDDI_RCV_ERROR";
         break;
      case CIO_TIMEOUT:
         p = "CIO_TIMEOUT";
         break;
      case CIO_NET_RCVRY_ENTER:
         p = "CIO_NET_RCVRY_ENTER";
         break;
      case CIO_NET_RCVRY_EXIT:
         p = "CIO_NET_RCVRY_EXIT";
         break;
      case FDDI_RING_STATUS:
         p = "FDDI RING STATUS";
         break;
      case FDDI_SMT_ERROR:
         p = "FDDI SMT ERROR";
         break;
      case FDDI_SMT_EVENT:
         p = "FDDI SMT EVENT";
         break;
      case FDDI_PHYS_INSERT:
         p = "FDDI_PHYS_INSERT";
         break;
      case FDDI_TX_ERROR:
         p = "FDDI_TX_ERROR";
         break;
      case FDDI_NO_ADDR:
         p = "FDDI_NO_ADDR";
         break;
      case FDDI_ADAP_INIT_FAIL:
         p = "FDDI_ADAP_INIT_FAIL";
         break;
      case FDDI_NOT_DIAG_MODE :
         p = "FDDI_NOT_DIAG_MODE";
         break;
      case FDDI_ADDR_ADDED:
         p = "FDDI_ADDR_ADDED";
         break;
      case FDDI_RTT:
         p = "FDDI_RTT";
         break;
      case FDDI_PATH_TEST:
         p = "FDDI_PATH_TEST";
         break;
      case FDDI_MAC_DISCONNECT:
         p = "FDDI_MAC_DISCONNECT";
         break;
      case FDDI_DIRECT_BCON:
         p = "FDDI_DIRECT_BCON";
         break;
      case FDDI_VER_MISMATCH:
         p = "FDDI_VER_MISMATCH";
         break;
      case FDDI_ADDR_RMVD:
         p = "FDDI_ADDR_RMVD";
         break;
      case FDDI_MAC_FRAME_ERR:
         p = "FDDI_MAC_FRAME_ERR";
         break;
      case FDDI_PORT_STUCK:
         p = "FDDI_PORT_STUCK";
         break;
      case FDDI_PORT_DISABLED:
         p = "FDDI_PORT_DISABLED";
         break;
      case FDDI_PORT_STOP:
         p = "FDDI_PORT_STOP";
         break;
      case FDDI_BYPASS_STUCK:
         p = "FDDI_BYPASS_STUCK";
         break;
      case FDDI_TRACE_MAX:
         p = "FDDI_TRACE_MAX";
         break;
      case FDDI_SELF_TEST:
         p = "FDDI_SELF_TEST";
         break;
      case FDDI_STARTED:
         p = "FDDI_STARTED";
         break;
      case CIO_OK:
         p = "CIO_OK";
         break;
      case CIO_TX_FULL:
         p = "CIO_TX_FULL";
         break;
      case CIO_NOT_STARTED:
         p = "CIO_NOT_STARTED";
         break;
      case CIO_BAD_RANGE:
         p = "CIO_BAD_RANGE";
         break;
      case CIO_BAD_MICROCODE:
         p = "CIO_BAD_MICROCODE";
         break;
      case CIO_BUF_OVFLW:
         p = "CIO_BUF_OVFLW";
         break;
      case CIO_HARD_FAIL:
         p = "CIO_HARD_FAIL";
         break;
      case CIO_LOST_DATA:
         p = "CIO_LOST_DATA";
         break;
      case CIO_NOMBUF:
         p = "CIO_NOMBUF";
         break;
      case CIO_NET_RCVRY_MODE:
         p = "CIO_NET_RCVRY_MODE";
         break;
      case CIO_INV_CMD:
         p = "CIO_INV_CMD";
         break;
      case CIO_NETID_INV:
         p = "CIO_NETID_INV";
         break;
      case FDDI_ROP:
         p = "FDDI_ROP";
         break;
      case FDDI_NO_ROP:
         p = "FDDI_NO_ROP";
         break;
      default:
         sprintf (tmp,"Unknown status: 0x%.8x", code);
         p = tmp;
         break;
   }
   return (p);
}
hcr (fildes)
int fildes;
{
   int      i;
   int      pidx;
   int      result;
   unsigned short param;
   char     tmp[64];
   fddi_hcr_cmd_t cmd;

   printf("\t **********************  HCR CMD  ***********************\n");

   bzero (&cmd, sizeof (cmd));
   cmd.hcr_val = prompt("\tEnter HCR command value.", "%x", "0x100");
   pidx = 0;
   if (y_or_n("\tDo you want to enter parameters?", TRUE))
   {
      for (pidx = 0; pidx < FDDI_CPB_SIZE; pidx++)
      {
         printf("\tParameter %d >: [0000] %s", pidx,
            (using_tty) ? "\b\b\b\b\b\b" : "");
         i = 0;
         while ((str[i++] = getinput()) != '\n')
            ;
         str[i-1] = '\0';
         if ((str[0] == '\0') || (str[0] == '.'))
         {
            break;
         }
         sscanf (str, "%d", &param);
         cmd.cpb[pidx] = param;
      }
   }
   /*
    * in bytes please
    */
   cmd.l_cpb = pidx << 1;

   result = IOCTL (fildes, FDDI_HCR_CMD, &cmd);
   if (result < 0)
   {
      decode_error(errno);
      fprintf(STDERR, "\tStatus code: %s\n", convcode (cmd.status));
   }
   else
   {
      fprintf(stdout, "\tStatus code: %s\n", convcode (cmd.status));
      printf ("\t\thsr value: %.4x\n", cmd.hsr_val);
      printf ("\t\tParameters: \n");
      for (pidx = 0; pidx < FDDI_CPB_SIZE; )
      {
         printf("\t\t0x%.4x 0x%.4x 0x%.4x 0x%.4x\n",
            cmd.cpb[pidx],
            cmd.cpb[pidx+1],
            cmd.cpb[pidx+2],
            cmd.cpb[pidx+3]);
         pidx += 4;
      }
      printf("\n\t Press ENTER to continue "); getinput();
   }

   printf("\n\t ********************************************************");
   return(0);
}
