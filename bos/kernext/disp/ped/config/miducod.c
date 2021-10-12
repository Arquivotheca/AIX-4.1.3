static char sccsid[] = "@(#)57  1.9.1.7  src/bos/kernext/disp/ped/config/miducod.c, peddd, bos411, 9428A410j 3/21/94 13:05:44";
/*
 *   COMPONENT_NAME: PEDDD
 *
 *   FUNCTIONS: SWAP_LONG
 *		SWAP_SHORT
 *		down_mc
 *		download
 *		downsec
 *		input
 *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1992,1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#define Bool unsigned

#include <sys/types.h>
#include <sys/malloc.h>
#include <sys/ioacc.h>
#include <sys/conf.h>
#include <sys/device.h>
#include <sys/mdio.h>
#include <sys/devinfo.h>
#include <sys/file.h>
#include <sys/sysmacros.h>
#include <sys/uio.h>
#include <sys/iocc.h>
#include <sys/proc.h>
#include <fcntl.h>
#include <sys/syspest.h>
#include "mid.h"
#include "midhwa.h"
#include "hw_dd_model.h"
#include "mid_ras.h"
#include "hw_dsp.h"
#include "hw_macros.h"
#include "bidefs.h"
#include "hw_typdefs.h"
#include "hw_errno.h"
#include "hw_regs_k.h"
#include "hw_ind_mac.h"
#include "hw_addrs.h"
#include "hw_PCBrms.h"
#include "midddf.h"

BUGXDEF(dbg_middd);
BUGXDEF(dbg_miducod);

#define SEEK_SET        0

long    vttact(struct vtmstruc *);
long    vttdact(struct vtmstruc *);

/***************************************************************************/
/*                                                                         */
/* IDENTIFICATION: SWAP_SHORT and SWAP_LONG                                */
/*                                                                         */
/* DESCRIPTIVE NAME: Swaps short and long values from the microcode file   */
/*                                                                         */
/* FUNCTION:  These macros will convert values read from the microcode     */
/*            file into the correct format to be downloaded.  The          */
/*            SWAP_SHORT routine will swap the two bytes of a short        */
/*            value and the SWAP_LONG routine will reverse the order       */
/*            of the four bytes of a long value.                           */
/*                                                                         */
/* INPUTS:    A pointer to the value to be converted                       */
/*            (short for SWAP_SHORT and long for SWAP_LONG)                */
/*                                                                         */
/* OUTPUTS:  The input value with the bytes reversed.                      */
/*                                                                         */
/* CALLED BY: downsec(), swap_file(), swap_opt(), and swap_sec             */
/*                                                                         */
/* CALLS:     NONE.                                                        */
/*                                                                         */
/***************************************************************************/

/* Reverse order of bytes in short word */
#define SWAP_SHORT(buf)                         \
   ((* ((char *) (buf))) |                      \
   ((* (((char *) (buf)) + 1)) << 8))

/* Reverse order of bytes in long word */
#define SWAP_LONG(buf)                          \
   ((* ((char *) (buf)))               |        \
   ((* (((char *) (buf)) + 1)) << 8)   |        \
   ((* (((char *) (buf)) + 2)) << 16)  |        \
   ((* (((char *) (buf)) + 3)) << 24))

/****************************************************************************/
/*                                                                          */
/* IDENTIFICATION: download                                                 */
/*                                                                          */
/* DESCRIPTIVE NAME: Supervises the downloading of the microcode            */
/*                                                                          */
/* FUNCTION:  This routine supervises the microcode download.  It           */
/*            halts the adapter, calls down_mc() to perform the actual      */
/*            download, and then restarts the adapter.  It then waits       */
/*            for the microcode to test the CRC and respond with the        */
/*            results.  If no response is received in the allowed time,     */
/*            or if a CRC error is detected, the code will be dowloaded     */
/*            again until either a good CRC is computed (cc = 0) or until   */
/*            the download has been attempted "RETRYCNT" times (cc = -1).   */
/*                                                                          */
/* INPUTS:                                                                  */
/*   midddf  *ddf;                     Data structure pointer               */
/*                                                                          */
/* OUTPUTS:                                                                 */
/*   int     cc;                       cc = 0 if successful                 */
/*                                     cc = -1 for bad CRC                  */
/*                                     cc = -2 for time out                 */
/*                                                                          */
/* CALLED BY: down()                                                        */
/*                                                                          */
/* CALLS:     down_mc(), mid_delay()                                        */
/*                                                                          */
/****************************************************************************/

download(ddf)
struct midddf *ddf;
{
  int     j;                                  /* General Loop Counter       */
  int     k;                                  /* General Loop Counter       */
  int     cc;                                 /* Condition Code             */
  int     rc;                                 /* Return Code                */
  int     status;                             /* Host Status                */
  int     dsp_code;                           /* DSP Commo Code             */

  HWPDDFSetup;                                /* Gain access to ped H/W     */
  BUGLPR(dbg_miducod, BUGNFO, ("Entering download\n"));

  /* Loop until the download is successful or until the retry count expires */
  for (j=0,cc = -2; j<RETRYCNT && cc < 0; j++)
  {

    /* The following sequence is required to fix a LEGA hardware problem    */
    /* It assures that the two adapter processor clocks are in sync         */
    MID_WR_DSP_CTL( MID_DSP_CTL_RST )         /* Halt adapter               */
    mid_delay(2);                             /* Nap for 2 microseconds     */
    MID_WR_DSP_CTL( MID_DSP_CTL_RUN )         /* Restart adapter            */
    mid_delay(2);                             /* Nap for 2 microseconds     */
    MID_WR_DSP_CTL( MID_DSP_CTL_RST )         /* Halt adapter               */
    rc = down_mc(ddf,ddf->mid_ucode_file_ptr);/* Download the microcode     */
    MID_WR_DSP_CTL( MID_DSP_CTL_RUN )         /* Restart adapter            */

    /* Loop until the CRC passes or the waitcnt expires */
    for (k=0,cc = -2; rc == 0 && cc == -2 && k<WAITCNT; k++)
    {
      MID_RD_HOST_STAT( status )              /* Get status of dsp commo reg*/
      if (status & MID_K_HOST_IO_WRITE_DSP_COMMO_REG)  /* If commo pending  */
      {
        MID_WR_HOST_STAT( MID_K_HOST_IO_WRITE_DSP_COMMO_REG )
        MID_RD_CARD_COMO( dsp_code )          /* Read dsp commo value       */
      }
      else
      {
        dsp_code = 0;                         /* Else commo is not pending  */
      }

      switch(dsp_code & 0xFFFF0000)           /* Mask off dsp commo opcode  */
      {
        case MICROCODE_CRC_FAILED:
        case BLAST_ERROR:                     /* No BLAST detected          */
        case BLAST_PROC_ERROR:                /* BLAST processor error      */
        case PIPE_PROC1_ERROR:                /* Pipe processor 1 error     */
        case PIPE_PROC2_ERROR:                /* Pipe processor 2 error     */
        case PIPE_PROC3_ERROR:                /* Pipe processor 3 error     */
        case PIPE_PROC4_ERROR:                /* Pipe processor 4 error     */
          cc = -1;                            /* Download failed            */
          BUGLPR(dbg_miducod, 0,
            ("download.case.FAIL.cc=%d status=%8.8X\n",cc,dsp_code));
          break;

        case RESET_COMPLETE:                  /* Reset complete-CRC's passed*/
        case MICROCODE_CRC_PASSED:            /* CRC's passed               */
          MID_WR_PCB_INIT( )                  /* Initialize PCB             */
          MID_HideShowActiveCursor(MID_HIDE_ACTIVE_CURSOR) /* Hide cursor   */
          cc = 0;                             /* Download passed            */
          BUGLPR(dbg_miducod, 0,
            ("download.case.PASS.cc=%d\n",cc));
          break;

        default:                              /* No download status yet     */
          BUGLPR(dbg_miducod, BUGNTA,
                ("download.case.default napping for 100 ms\n"));
          mid_delay(100000);                  /* Nap 0.1 seconds & retry    */
          break;
      }
    }
  }

  /* Enable DSP_COMMO interrupt  */
  ddf->host_intr_mask_shadow = MID_K_HOST_IO_WRITE_DSP_COMMO_REG;
  MID_WR_HOST_INTR(MID_K_HOST_IO_WRITE_DSP_COMMO_REG);

  /* cc = 0 if passed,-1 if failed, and -2 if timed out */
  BUGLPR(dbg_miducod, 0, ("Leaving download with rc =%d.\n",cc));
  return(cc);
}

/****************************************************************************/
/*                                                                          */
/* IDENTIFICATION: down_mc                                                  */
/*                                                                          */
/* DESCRIPTIVE NAME: Download the microcode                                 */
/*                                                                          */
/* FUNCTION:  This routine reads and interprets the contents of the         */
/*            microcode download file.  It determines the number of         */
/*            code sections in the file and uses downsec() to download      */
/*            those which need to be downloaded.  The format of the         */
/*            microcode is COFF (Common Object File Format).                */
/*                                                                          */
/* INPUTS:                                                                  */
/*   midddf  *ddf;                     Data structure pointer               */
/*   file    *mcfile;                  File pointer to microcode file       */
/*                                                                          */
/* OUTPUTS:                                                                 */
/*   int     rc;                       rc = 0 if successful                 */
/*                                     rc = -1 if failed                    */
/*                                                                          */
/* CALLED BY: download()                                                    */
/*                                                                          */
/* CALLS:     input(), SWAP_SHORT(), SWAP_LONG(), downsec(), miderr()       */
/*                                                                          */
/****************************************************************************/

down_mc(ddf,mcfile)
struct midddf *ddf;
struct file    *mcfile;
{
  int     i;                                 /* General purpose loop ctr    */
  int     rc;                                /* Return code                 */
  short   filetype;                          /* File type of microcode file */
  short   seccount;                          /* Count of section headers    */
  long    addr;                              /* DRAM address of section     */
  long    size;                              /* Size of section             */
  long    sechdloc;                          /* Section header disk loc     */
  long    secdaloc;                          /* Section data disk location  */
  char    filehdr[FILHDLEN];                 /* File header                 */
  char    sechdr[SECHDLEN];                  /* Section header              */

  HWPDDFSetup;                               /* Gain access to ped H/W      */

  BUGLPR(dbg_miducod, BUGGID, ("Entering down_mc. \n"));
  BUGLPR(dbg_miducod, BUGGID, ("File ptr 0x%x\n",mcfile));

  rc = input(ddf,mcfile,0,filehdr,FILHDLEN,1);/* Input file header            */
  filetype = SWAP_SHORT(&filehdr[FILETYPE]); /* Get file type                */
  seccount = SWAP_SHORT(&filehdr[SECCOUNT]); /* Get count of section headers */

  if (filetype != 0x93 && rc == 0)           /* Verify file type             */
  {
    BUGLPR(dbg_miducod, BUGGID,
      ("filetype=%x, (should = 0x93).\n", filetype));
    miderr(ddf,NULL,"mid-level","down_mc","SWAP_SHORT",filetype,
      MID_BAD_UCODE_FORMAT, RAS_UNIQUE_1);
    return(-1);
  }

  sechdloc = FILHDLEN +
    (long) SWAP_SHORT(&filehdr[OPTHEADR]);   /* Compute 1st section hdr loc */

  for (i=0; i<seccount && rc == 0; i++)      /* Loop for each section       */
  {
    BUGLPR(dbg_miducod, 5,
      ("down_mc.for i=%d seccount=%d\n", i,seccount));
    rc = input(ddf,mcfile,sechdloc,sechdr,SECHDLEN,1);/* Input section header */
    sechdloc += SECHDLEN;                    /* Point to next section hdr   */
    secdaloc = SWAP_LONG(&sechdr[SECRAWDA]); /* Get section data location   */

    if (secdaloc && rc == 0)                 /* Download if nonzero         */
    {
      addr = SWAP_LONG(&sechdr[SECPADDR]);   /* Get section DRAM address    */
      size = SWAP_LONG(&sechdr[SECSIZE]);    /* Get section size            */
      rc = downsec(ddf,mcfile,secdaloc,addr,size);  /* Download section     */
      BUGLPR(dbg_miducod, 5, ("down_mc.for.if. addr=%x size=%d\n", addr,size));
    }
  }

  BUGLPR(dbg_miducod, 5, ("Leaving down_mc. \n"));
  return(rc);
}

/****************************************************************************/
/*                                                                          */
/* IDENTIFICATION: downsec                                                  */
/*                                                                          */
/* DESCRIPTIVE NAME: Download a section of the microcode                    */
/*                                                                          */
/* FUNCTION:  This routine will download a section of the microcode.        */
/*            It sets up the BIM and writes the microcode into the DSP      */
/*            DRAM a word at a time via the indirect address port.          */
/*                                                                          */
/* INPUTS:                                                                  */
/*   midddf  *ddf;                          Data structure pointer          */
/*   file    *secfile;                      File pointer to microcode file  */
/*   long    seekloc;                       Seek location for section       */
/*   long    secaddr;                       Address for loading section     */
/*   long    secsize;                       Section size in words           */
/*                                                                          */
/* OUTPUTS:                                                                 */
/*   int     rc;                            rc = 0 if successful            */
/*                                          rc = -1 if failed               */
/*                                                                          */
/* CALLED BY: down_mc()                                                     */
/*                                                                          */
/* CALLS:     fp_lseek(), miderr(), cd_write(), fp_read(), SWAP_LONG()      */
/*                                                                          */
/****************************************************************************/

downsec(ddf,secfile,seekloc,secaddr,secsize)
struct midddf *ddf;
struct file     *secfile;                 /* File pointer to microcode file */
long    seekloc;                          /* Seek location for section      */
long    secaddr;                          /* Address for loading section    */
long    secsize;                          /* Section size in words          */
{
  int     i,j;                            /* General purpose loop counter   */
  int     indaddr;                        /* BIM indirect address           */
  int     databuf[256];                   /* Buffer for inputting data      */
  int     rc;                             /* General purpose return code    */
  int     numbytes;                       /* Byte read count                */
  int     numwords;                       /* Word read count                */
  int     bytes_to_read;                  /* Bytes to read from file        */

  HWPDDFSetup;                            /* Gain access to ped H/W         */

  BUGLPR(dbg_miducod, BUGGID, ("Entering downsec. \n"));

  if ((rc = fp_lseek(secfile,seekloc,SEEK_SET)) != 0) /* Seek section loc   */
  {
    BUGLPR(dbg_miducod, BUGGID,
      ("Bad return code on fp_lseek.  rc=%d.\n",rc));
    miderr(ddf,NULL,"mid-level","downsec","fp_lseek",rc,NULL,RAS_UNIQUE_1);
  }

  MID_MAP_IND_ADR( indaddr, secaddr )     /* Convert DRAM to BIM adr        */
  MID_LOAD_IND( indaddr, MID_IND_WR_AI )  /* Point to DRAM location         */

  for (i=0; i<secsize; i+= numwords)      /* Loop until secion is downloaded*/
  {                                       /* Numwords is computed in loop   */
    bytes_to_read = (secsize - i) << 2;   /* Compute bytes left in section  */
    if (bytes_to_read > sizeof(databuf))  /* If bytes won't fit in buffer   */
      bytes_to_read = sizeof(databuf);    /* Limit bytes to buffer size     */

    rc = fp_read(secfile, databuf,        /* Read <= 256 words from file    */
                 bytes_to_read, 0,
                 UIO_SYSSPACE, &numbytes);/* numbytes = number of bytes read*/

    if (rc < 0)                           /* If read failed                 */
    {
      BUGLPR(dbg_miducod, 0, ("Bad return code on fp_read.  rc=%d.\n",rc));
      miderr(ddf,NULL,"mid-level","downsec","fp_read",rc, MID_BAD_UCODE_INPUT,RAS_UNIQUE_1);
      return(-1);
    }

    numwords = numbytes >> 2;             /* Compute number of words        */

    for (j=0; j<numwords; j++)
      databuf[j] = SWAP_LONG(&databuf[j]);/* Swap bytes in each data word   */

    MID_WR_IND( databuf, numwords )       /* Write data to adapter          */
  }

  BUGLPR(dbg_miducod, BUGGID, ("Leaving downsec. \n"));
  return(0);
}

/****************************************************************************/
/*                                                                          */
/* IDENTIFICATION: input                                                    */
/*                                                                          */
/* DESCRIPTIVE NAME: Inputs data from a file                                */
/*                                                                          */
/* FUNCTION:  This routine will seek to the specified section of a file     */
/*            and read the specified count of values of the specified       */
/*            size into the specified address.                              */
/*                                                                          */
/* INPUTS:                                                                  */
/*   file    *file;                         Input file pointer              */
/*   long    seekloc;                       Seek location of data in file   */
/*   char    *addr;                         Buffer address for data         */
/*   long    size;                          Size of values to read          */
/*   long    count;                         Count of values to read         */
/*                                                                          */
/* OUTPUTS:                                                                 */
/*   int     rc;                            rc = 0 if data read successfully*/
/*                                          rc = -1 if failed seek or read  */
/*                                                                          */
/* CALLED BY: down_mc()                                                     */
/*                                                                          */
/* CALLS:     fp_lseek(), fp_read(), miderr()                               */
/*                                                                          */
/****************************************************************************/

input(ddf,file,seekloc,addr,size,count)
struct midddf 	*ddf;
struct file     *file;                      /* Input file pointer           */
long    seekloc;                            /* Seek location of data in file*/
char    *addr;                              /* Buffer address for data      */
long    size;                               /* Size of values to read       */
long    count;                              /* Count of values to read      */
{
  int     rc;                               /* Return code                  */
  int     numbytes;                         /* Number of bytes read         */

  BUGLPR(dbg_miducod, BUGGID, ("Entering input. \n"));

  /* Seek to specified location in file */
  if ((rc = fp_lseek(file,seekloc,SEEK_SET)) != 0)
  {
    BUGLPR(dbg_miducod, BUGGID,
      ("Bad return code on fp_lseek.  rc=%d.\n",rc));
    miderr(ddf,NULL,"mid-level","input","fp_lseek",rc,NULL,RAS_UNIQUE_1);
    return(-1);
  }

  BUGLPR(dbg_miducod, BUGGID,
    ("input.fp_read.fileptr=0x%x addr=0x%x size=%d count=%d\n",
    file,addr,size,count));

  /* Read specified amount of data into specified address */
  if ((rc = fp_read(file,addr,size*count,0,UIO_SYSSPACE,&numbytes)) < 0)
  {
    BUGLPR(dbg_miducod, BUGGID,
      ("Bad return code on fp_lread.  rc=%d.  \n",rc));
    BUGLPR(dbg_miducod, BUGGID,
      ("Intended to read: %d Actually read: %d\n",
      size*count, numbytes));
    miderr(ddf,NULL,"mid-level","input","fp_read",rc,
      MID_BAD_UCODE_INPUT,RAS_UNIQUE_1);
    return(-1);
  }

  BUGLPR(dbg_miducod, BUGGID, ("Leaving input. \n"));
  return(0);
}
