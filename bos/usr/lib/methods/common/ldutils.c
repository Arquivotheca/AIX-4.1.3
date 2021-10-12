static char sccsid[] = "@(#)76	1.2  src/bos/usr/lib/methods/common/ldutils.c, cfgcommo, bos411, 9436C411a 9/7/94 09:30:46";
/*
 * COMPONENT_NAME: (CFGMETH) cfgampx
 *
 * FUNCTIONS: readmem(), writemem(), readio_reg(), writeio_reg()  
 *            issuecmd(), getbuffers(), intwait(), reset(),        
 *            getprimstat()                                       
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1992
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
³                                                                         ³
³                                                                         ³
³ PURPOSE: Utility routines used by the microcode loader in lducode.c     ³
³                                                                         ³
³ ALGORITHM DESCRIPTION: Described under each function                    ³
³                                                                         ³
³ EXTERNAL VARIABLES USED: None                                           ³
³                                                                         ³
³ PUBLIC VARIABLES DECLARED: readmem                                      ³
³                            writemem                                     ³
³                            readio_reg                                   ³
³                            writeio_reg                                  ³
³                            issuecmd                                     ³
³                            getbuffers                                   ³
³                            intwait                                      ³
³                            reset                                        ³
³                                                                         ³
³ EXTERNAL ROUTINES CALLED:  None                                         ³
³                                                                         ³
ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/mdio.h>
#include "lducode.h"

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
 ³ Error codes returned by these routines  ³
 ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
#define   E_BUSIO               -100
#define   E_BUSMEM              -101
#define   E_INVALID_IB          -102
#define   E_CMD_REJECTED        -103
#define   E_TIMEOUT             -104

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
 ³ Macro to return the minimum of 2 integers ³
 ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
#define   min(x,y) (x<=y)?x:y

int readmem(fd, pos, page, offset, num, buffer)
/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
 ³ Reads data from a specified address in the Shared Storage window to a    ³
 ³ user space buffer.                                                       ³
 ³                                                                          ³
 ³ INPUTS: file descriptor for machine device driver                        ³
 ³         POS register info of target adapter                              ³
 ³         number of page to read data from                                 ³
 ³         offset in page to read data from                                 ³
 ³         number of bytes to read                                          ³
 ³         user space buffer to read data into                              ³
 ³                                                                          ³
 ³ OUTPUTS: return code       0       no errors                             ³
 ³                            < 0     Error                                 ³
 ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
int fd;                /* Handle for /dev/bus0        */
POS_INFO *pos;         /* Adapters POS reg info       */
uchar page;            /* Page to read                */
int offset;            /* Offset in page              */
uint num;              /* Number of bytes to read     */
uchar *buffer;         /* Copy data read here         */
{
   MACH_DD_IO mddRecord; /* machine dd ioctl buffer               */
   uint absolute_addr;   /* Physical address of card's bus memory */
   uchar old_page;       /* Old CPU Page register value           */
   int count;            /* Number of bytes read on each loop     */
   int rc;               /* Return code from function calls       */

   /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
    ³ Save old CPU page register ³
    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
   if (rc = readio_reg(fd, pos->baseio + 5,&old_page))
      return(E_BUSIO);

   /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
    ³  Update CPU register if necessary   ³
    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

   if (old_page != page)
   {
      if (rc = writeio_reg(fd, pos->baseio + 5, page))
         return(E_BUSIO);
   }

   /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
    ³  Translate page/offset into Microchannel address³
    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
   absolute_addr = pos->win_base_addr + offset;

   /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
    ³ Copy data from card, adjusting the CPU page register ³
    ³ when necessary                                       ³
    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
   do
   {
      count = min((pos->win_size - offset), num);
      mddRecord.md_size = count;
      mddRecord.md_incr = MV_BYTE;
      mddRecord.md_data = buffer;
      mddRecord.md_addr = absolute_addr;

      if (ioctl(fd, MIOBUSGET, &mddRecord))
         return(E_BUSMEM);

      if (count < num)
      {
         num -= count;
         offset = 0;
         absolute_addr = pos->win_base_addr;
         buffer += count;
         if (rc = writeio_reg(fd, pos->baseio + 5, ++page))
            return(E_BUSIO);
      }
      else
         break;
   } while (1);

   /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
    ³ Restore old CPU page register if necessary ³
    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
   if (old_page != page)
   {
      if (rc = writeio_reg(fd, pos->baseio + 5, old_page))
         return(E_BUSIO);
   }

   /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
    ³ Return success   ³
    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
   return(0);
}


int writemem(fd, pos, page, offset, num, buffer)
/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
 ³ Writes data to a specified address in the Shared Storage window from a   ³
 ³ user space buffer.                                                       ³
 ³                                                                          ³
 ³ INPUTS: file descriptor for machine device driver                        ³
 ³         POS register info of target adapter                              ³
 ³         number of page to write data to                                  ³
 ³         offset in page to write data to                                  ³
 ³         number of bytes to write                                         ³
 ³         user space buffer to copy data from                              ³
 ³                                                                          ³
 ³ OUTPUTS: return code       0       no errors                             ³
 ³                            < 0     Error                                 ³
 ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
int fd;                /* Handle for /dev/bus0        */
POS_INFO *pos;         /* Adapters POS reg info       */
uchar page;            /* Page to read                */
int offset;            /* Offset in page              */
uint num;              /* Number of bytes to write    */
uchar *buffer;         /* Write data copied from here */
{
   MACH_DD_IO mddRecord; /* machine dd ioctl buffer               */
   uint absolute_addr;   /* Physical address of card's bus memory */
   uchar old_page;       /* Old CPU Page register value           */
   int count;            /* Number of bytes read on each loop     */
   int rc;               /* Return code from function calls       */

   /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
    ³ Save old CPU page register ³
    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
   if (rc = readio_reg(fd, pos->baseio + 5,&old_page))
      return(E_BUSIO);

   /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
    ³  Update CPU register if necessary   ³
    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
   if (old_page != page)
   {
      if (rc = writeio_reg(fd, pos->baseio + 5, page))
         return(E_BUSIO);
   }

   /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
    ³  Translate page/offset into Microchannel address³
    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
   absolute_addr = pos->win_base_addr + offset;

   /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
    ³ Copy data to card, adjusting the CPU page register   ³
    ³ when necessary                                       ³
    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
   do
   {
      count = min((pos->win_size - offset), num);
      mddRecord.md_size = count;
      mddRecord.md_incr = MV_BYTE;
      mddRecord.md_data = buffer;
      mddRecord.md_addr = absolute_addr;

      if (ioctl(fd, MIOBUSPUT, &mddRecord))
         return(E_BUSMEM);

      if (count < num)
      {
         num -= count;
         offset = 0;
         absolute_addr = pos->win_base_addr;
         buffer += count;
         if (rc = writeio_reg(fd, pos->baseio + 5, ++page))
            return(E_BUSIO);
      }
      else
         break;
   } while (1);

   /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
    ³ Restore old CPU page register if necessary ³
    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
   if (old_page != page)
   {
      if (rc = writeio_reg(fd, pos->baseio + 5, old_page))
         return(E_BUSIO);
   }

   /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
    ³ Return success   ³
    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
   return(0);
}


int readio_reg(fd, ioaddr, data_ptr)
/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
 ³ Reads an 8 bit byte from a given I/O port and copies to user space       ³
 ³                                                                          ³
 ³ INPUTS: file descriptor for machine device driver                        ³
 ³         I/O address to read from                                         ³
 ³         user space buffer to copy data to                                ³
 ³                                                                          ³
 ³ OUTPUTS: return code       0       no errors                             ³
 ³                            < 0     Error                                 ³
 ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
int fd;                /* Handle for /dev/bus0  */
int ioaddr;            /* I/O address          */
uchar *data_ptr;       /* Put reg contents here */
{
      MACH_DD_IO mddRecord; /* machine dd ioctl buffer               */

      mddRecord.md_size = 1;
      mddRecord.md_incr = MV_BYTE;
      mddRecord.md_data = data_ptr;
      mddRecord.md_addr = ioaddr;

      if (ioctl(fd, MIOBUSGET, &mddRecord))
         return(E_BUSIO);

      return(0);
}

int writeio_reg(fd, ioaddr, data)
/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
 ³ Writes an 8 bit byte to a given I/O port                                 ³
 ³                                                                          ³
 ³ INPUTS: file descriptor for machine device driver                        ³
 ³         I/O address to write to                                          ³
 ³         Byte to write to port                                            ³
 ³                                                                          ³
 ³ OUTPUTS: return code       0       no errors                             ³
 ³                            < 0     Error                                 ³
 ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
int fd;                /* Handle for /dev/bus0  */
int ioaddr;            /* Base I/O address      */
uchar data;            /* Data to write         */
{
      MACH_DD_IO mddRecord; /* machine dd ioctl buffer               */

      mddRecord.md_size = 1;
      mddRecord.md_incr = MV_BYTE;
      mddRecord.md_data = &data;
      mddRecord.md_addr = ioaddr;

      if (ioctl(fd, MIOBUSPUT, &mddRecord))
         return(E_BUSIO);

      return(0);
}

int issuecmd(fd, pos, tasknum, cmd, timeout, parms_length, parms)
/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
 ³ Issues a command to a task (or the ROS) on the card                      ³
 ³                                                                          ³
 ³ INPUTS: file descriptor for machine device driver                        ³
 ³         POS register info of target adapter                              ³
 ³         Task number to issue command to                                  ³
 ³         Command code to send to task                                     ³
 ³         Time in ms for command to be accepted                            ³
 ³         Length of command parameters                                     ³
 ³         Pointer to command parameters                                    ³
 ³                                                                          ³
 ³ OUTPUTS: return code       0       no errors                             ³
 ³                            < 0     Error                                 ³
 ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
int fd;                /* Handle for /dev/bus0                  */
POS_INFO *pos;         /* Adapters POS reg info                 */
uchar tasknum;         /* Task number to issue command to       */
uchar cmd;             /* Command                               */
uint timeout;          /* Time to wait for response             */
int parms_length;      /* Parms for command                     */
char *parms;           /* Ptr to parms                          */
{
   MACH_DD_IO mddRecord;
   uchar pc_select;
   uchar obinfo[3];
   uchar buf[2];
   ushort bcb_addr;
   uchar ob_page;
   uint ob_offset;
   int i;
   int rc;

    /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
     ³Verify valid PC Select Byte  ³
     ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
   if (rc = readmem(fd, pos, 0, 0x440, 1, &pc_select))
      return(rc);
   if (pc_select != 0xFF)
      return(E_INVALID_IB);

   /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
    ³ Get task's BCB offset  ³
    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
   if (rc = readmem(fd, pos, 0, 0x45A, 2, buf))
      return(rc);
   bcb_addr = (buf[1] << 8) | buf[0];
   bcb_addr += (tasknum << 4);

   /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
    ³ Write command code in BCB   ³
    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
   if (rc = writemem(fd, pos, 0, bcb_addr, 1, &cmd))
      return(rc);

    /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
     ³ Write task number in PC Select byte ³
     ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
   if (rc = writemem(fd, pos, 0, 0x440, 1, &tasknum))
      return(rc);

   /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
    ³  Write any parms to output buffer ³
    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
   if (parms_length)
   {
      /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
       ³ Read in task's output buffer address  ³
       ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
      if (rc = readmem(fd, pos, 0, bcb_addr + 0x0D, 3, obinfo))
         return(rc);

      ob_page = obinfo[2];
      ob_offset = (obinfo[1] << 8) | obinfo[0];

      /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
       ³ Write parms to output buffer   ³
       ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
      if (rc = writemem(fd, pos, ob_page, ob_offset, parms_length, parms))
         return(rc);
   }

   /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
    ³ Interrupt adapter  ³
    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
   if (rc = writeio_reg(fd, pos->baseio + 2, 0x09))
            return(E_BUSIO);

   /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
    ³ Wait for command acknowledgement  ³
    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
   for (i = 0; (i < timeout) || (i == 0); ++i)
   {
      if (rc = readmem(fd, pos, 0, 0x440, 1, &pc_select))
         return(rc);
      if (pc_select == 0xFF) break;
      if (timeout)
         usleep(1000);
   }

   /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
    ³ Did we timeout or was command rejected ? ³
    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
   if (pc_select != 0xFF)
   {
      if (pc_select == 0xFE)
         return(E_CMD_REJECTED);
      else
         return(E_TIMEOUT);
   }
   else
      return(0);
}

int intwait(fd, pos, tasknum, timeout)
/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
 ³ Waits for an adapter interrupt from a specific task                      ³
 ³                                                                          ³
 ³ INPUTS: file descriptor for machine device driver                        ³
 ³         POS register info of target adapter                              ³
 ³         Task number expected to interrupt                                ³
 ³         Time in ms to wit for interrupt                                  ³
 ³                                                                          ³
 ³ OUTPUTS: return code       0       no errors                             ³
 ³                            < 0     Error                                 ³
 ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
int fd;                /* Handle for /dev/bus0                  */
POS_INFO *pos;         /* Adapters POS reg info                 */
uchar tasknum;         /* Task number interrupting              */
int timeout;           /* Time to wait for interrupt            */
{

   int i;              /* Loop counter                     */
   int rc;             /* Return code from function calls  */
   uchar int_id;       /* Interface Block's Int ID byte    */
   uchar rest_int_id;  /* Used to restore original Int ID  */

   /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
    ³  Wait for interrupt from card   ³
    ³  By looking at Interrupt ID byte³
    ³  in  Interface Block            ³
    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
   for (i = 0; (i < timeout) || (i == 0); ++i)
   {
      if (rc = readmem(fd, pos, 0, 0x441, 1, &int_id))
         return(rc);
      if (int_id == tasknum) break;
      if (timeout)
         usleep(1000);
   }
   /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
    ³  Restore int id for next interrupt³
    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
   if (int_id != 0xFF)
   {
      rest_int_id = 0xFF;
      if (rc = writemem(fd, pos, 0, 0x441, 1, &rest_int_id))
      {
         return(rc);
      }
   }

   /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
    ³ Return E_TIMEOUT if int id has not changed ³
    ³ else return SUCCESS                        ³
    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
   if (int_id == tasknum)
      return(0);
   else
      return(E_TIMEOUT);

}

int getbuffers(fd, pos, tasknum, bufs)
/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
 ³ Gets a tasks BCB page/offset  addresses                                  ³
 ³                                                                          ³
 ³ INPUTS: file descriptor for machine device driver                        ³
 ³         POS register info of target adapter                              ³
 ³         Buffer to write addresses                                        ³
 ³                                                                          ³
 ³ OUTPUTS: return code       0       no errors                             ³
 ³                            < 0     Error                                 ³
 ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
int fd;                /* Handle for /dev/bus0                  */
POS_INFO *pos;         /* Adapters POS reg info                 */
uchar tasknum;         /* Task number interrupting              */
BUFFER_ADDRS *bufs;    /* Store addresses here                  */
{
   ushort bcb_addr;    /* Starting address of task's BCB  */
   uchar bcb[16];      /* Tasks BCB entry                 */
   int rc;             /* Return code from function calls */

   /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
    ³ Get task's BCB pointer and calculate address of BCB ³
    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
   if (rc = readmem(fd, pos, 0, 0x45A, 2, bcb))
      return(rc);
   bcb_addr = (bcb[1] << 8) | bcb[0];
   bcb_addr += (tasknum << 4);

   /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
    ³ Read in task's  buffer addresses³
    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
   if (rc = readmem(fd, pos, 0, bcb_addr, 15, bcb))
      return(rc);

   /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
    ³ Copy addresses to caller's buffer ³
    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
   bufs->sspage = bcb[5];
   bufs->ssoffset = (bcb[4] << 8)  | bcb[3];
   bufs->inpage = bcb[10];
   bufs->inoffset = (bcb[9] << 8)  | bcb[8];
   bufs->outpage = bcb[15];
   bufs->outoffset = (bcb[14] << 8)  | bcb[13];

   return(0);
}

int getprimstat(fd, pos, tasknum, primstat)
/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
 ³ Gets a tasks primary status byte                                         ³
 ³                                                                          ³
 ³ INPUTS: file descriptor for machine device driver                        ³
 ³         POS register info of target adapter                              ³
 ³         Buffer to write PSB                                              ³
 ³                                                                          ³
 ³ OUTPUTS: return code       0       no errors                             ³
 ³                            < 0     Error                                 ³
 ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
int fd;                /* Handle for /dev/bus0                  */
POS_INFO *pos;         /* Adapters POS reg info                 */
uchar tasknum;         /* Task number                           */
uchar *primstat;       /* Store PSB here                        */
{
   ushort bcb_addr;    /* Starting address of task's BCB  */
   uchar bcb[16];      /* Tasks BCB entry                 */
   int rc;             /* Return code from function calls */

   /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
    ³ Read task's PSB from Interface Block                ³
    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
   return(readmem(fd, pos, 0, 0x47C + tasknum, 1, primstat));
}

int reset(fd, pos)
/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
 ³ Resets an adapter                                                        ³
 ³                                                                          ³
 ³ INPUTS: file descriptor for machine device driver                        ³
 ³         POS register info of target adapter                              ³
 ³                                                                          ³
 ³ OUTPUTS: return code       0       no errors                             ³
 ³                            < 0     Error                                 ³
 ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
int fd;                /* Handle for /dev/bus0                  */
POS_INFO *pos;         /* Adapters POS reg info                 */
{
   uchar comreg;       /* Command register value          */
   uchar initreg1;     /* INITREG1 register value         */
   int rc;             /* Return code from function calls */
   int i;              /* Loop counter                    */

   /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
    ³ Clear the PROMREADY bit in INITREG1 so we can check that the ³
    ³ reset completed successfully                                 ³
    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
   if (rc = writeio_reg(fd, pos->baseio + 2, 0x10))
      return(rc);

   if (rc = readio_reg(fd, pos->baseio + 3, &initreg1))
      return(rc);

   initreg1 &= 0xBF;

   if (rc = writeio_reg(fd, pos->baseio + 2, 0x10))
      return(rc);

   if (rc = writeio_reg(fd, pos->baseio + 3, initreg1))
      return(rc);

   /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
    ³ Do the hardware reset by setting COMREG to 0x11 (reset command)³
    ³ then to 0 to clear it and finally to 0x10 to enable interrupts ³
    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
   if (rc = writeio_reg(fd, pos->baseio + 6, 0x11))
      return(rc);

   if (rc = writeio_reg(fd, pos->baseio + 6, 0x00))
      return(rc);

   if (rc = writeio_reg(fd, pos->baseio + 6, 0x10))
      return(rc);

   /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
    ³ Wait up to 1 minute for adapter to complete the    ³
    ³ reset process. Check PROMREADY bit in INITREG1 to  ³
    ³ see if reset has completed successfully            ³
    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
   for (i = 0; i < 60; ++i)
   {

      if (rc = writeio_reg(fd, pos->baseio + 2, 0x10))
         return(rc);

      if (rc = readio_reg(fd, pos->baseio + 3, &initreg1))
         return(rc);

      if (initreg1 & 0x40) break;

      sleep(1);
   }

   if (initreg1 & 0x40)
      return(0);
   else
      return(E_TIMEOUT);
}

