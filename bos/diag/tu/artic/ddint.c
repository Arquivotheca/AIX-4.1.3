static char sccsid[] = "@(#)96  1.1  src/bos/diag/tu/artic/ddint.c, tu_artic, bos411, 9428A410j 8/19/93 17:27:13";
/*
 * COMPONENT_NAME:  
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * FUNCTIONS:       icareset(), icareadmem(), icawritemem(), icaintreg(),
 *                  icaintwait(), icaintdereg(), icaissuecmd(), icagetparms(),
 *                  icainbuf(), icaoutbuf(), icasecstatbuf(), icagetprimstat(),
 *                  icadmasetup(), icadmarel(), icareadpos(), icawritepos()
 *                  icagetbuffers(), icareadio(), icawriteio(), icasendconfig()
 *
 *
 * INPUTS: file descriptor for ARTIC device driver and parameters
 *         required by the various IOCTL functions of the device
 *         driver
 *
 * OUTPUTS: return code          0       no errors
 *
 *                          0xFF23       Bad file descriptor
 *
 *                    anything else      error code returned by device driver
 *
 *
 * ALGORITHM DESCRIPTION:
 *
 *     Call ioctl() with appropiate parameter data and command code
 *     If ioctl() return code == -1 then
 *        return 0xFF23
 *     endif
 *
 *     If parameter buffer return code field = NO_ERROR then
 *        copy returned data into caller's buffer (when required)
 *        return no error
 *     else
 *        return error code received from device driver
 *     Endif
 */


#include <ddint.h>

/**********************************************************************/

ushort icareset(fd)
/* Resets a Co-Processor adapter */

int fd;               /* file descriptor for artic device driver */
{
   ARTIC_IOCTL_PARMS parms; /* Parm buffer for IOCTL call */

   if (ioctl(fd,ICARESET,(void *) &parms) == -1)
      return(E_ICA_INVALID_FD);
   else
      return(parms.icareset.retcode);
}

/*********************************************************************/


ushort icareadmem(fd, length, segpage, offset, addr_format, buffptr)
/* Reads from memory on an adapter into a user application buffer */
int fd;               /* file descriptor for artic device driver */
ulong length;         /* Number of bytes to read                 */
ushort segpage;       /* Segment or page                         */
ushort offset;        /* Offset within segment or page           */
uchar addr_format;    /* Control field for address format        */
uchar *buffptr;       /* Ptr to applications buffer              */
{
   ARTIC_IOCTL_PARMS parms; /* Parm buffer for IOCTL call */

   parms.icareadmem.length = length;
   parms.icareadmem.segpage = segpage;
   parms.icareadmem.offset = offset;
   parms.icareadmem.addr_format = addr_format;
   parms.icareadmem.dest = buffptr;

   if (ioctl(fd,ICAREADMEM,(void *) &parms) == -1)
      return(E_ICA_INVALID_FD);
   else
      return(parms.icareadmem.retcode);
}

/*********************************************************************/

ushort icawritemem(fd, length, segpage, offset, addr_format, buffptr)
/* Writes from a user application buffer to memory on an adapter */

int fd;               /* file descriptor for artic device driver */
ulong length;         /* Number of bytes to write                */
ushort segpage;       /* Segment or page                         */
ushort offset;        /* Offset within segment or page           */
uchar addr_format;    /* Control field for address format        */
uchar *buffptr;       /* Ptr to applications buffer              */
{
   ARTIC_IOCTL_PARMS parms; /* Parm buffer for IOCTL call */

   parms.icawritemem.length = length;
   parms.icawritemem.segpage = segpage;
   parms.icawritemem.offset = offset;
   parms.icawritemem.addr_format = addr_format;
   parms.icawritemem.dest = buffptr;

   if (ioctl(fd,ICAWRITEMEM,(void *) &parms) == -1)
      return(E_ICA_INVALID_FD);
   else
      return(parms.icawritemem.retcode);
}


/*********************************************************************/

ushort icaintreg(fd)
/* Registers a process to be notified of a task interrupt */

int fd;               /* file descriptor for artic device driver */
{
   ARTIC_IOCTL_PARMS parms; /* Parm buffer for IOCTL call */

   if (ioctl(fd,ICAINTREG,(void *) &parms) == -1)
      return(E_ICA_INVALID_FD);
   else
      return(parms.icaintreg.retcode);
}


/*********************************************************************/

ushort icaintwait(fd, timeout)
/* Waits for a task interrupt */

int fd;               /* file descriptor for artic device driver */
uint timeout;         /* The time in ms to wait for interrupt    */
{
   ARTIC_IOCTL_PARMS parms; /* Parm buffer for IOCTL call */

   parms.icaintwait.timeout = timeout;

   if (ioctl(fd,ICAINTWAIT,(void *) &parms) == -1)
      return(E_ICA_INVALID_FD);
   else
      return(parms.icaintwait.retcode);
}


/*********************************************************************/

ushort icaintdereg(fd)
/* Deregisters a process that was registered to be notified of a task interrupt */

int fd;               /* file descriptor for artic device driver */
{
   ARTIC_IOCTL_PARMS parms; /* Parm buffer for IOCTL call */

   if (ioctl(fd,ICAINTDEREG,(void *) &parms) == -1)
      return(E_ICA_INVALID_FD);
   else
      return(parms.icaintdereg.retcode);
}


/*********************************************************************/

ushort icaissuecmd(fd, cmdcode, length, timeout, prmptr)
/* Issues a command to a task */

int fd;               /* file descriptor for artic device driver */
ushort cmdcode;       /* Command code to be sent to task         */
ushort length;        /* Length of parameter block               */
ushort timeout;       /* Time in ms for command to be accepted   */
uchar *prmptr;        /* Pointer to parameter block              */
{
   ARTIC_IOCTL_PARMS parms; /* Parm buffer for IOCTL call */

   parms.icaissuecmd.cmdcode = cmdcode;
   parms.icaissuecmd.length = length;
   parms.icaissuecmd.timeout = timeout;
   parms.icaissuecmd.prms = prmptr;

   if (ioctl(fd,ICAISSUECMD,(void *) &parms) == -1)
      return(E_ICA_INVALID_FD);
   else
      return(parms.icaissuecmd.retcode);
}


/*********************************************************************/

ushort icagetparms(fd, prmptr)
/* Get configuration parameters for an adapter */

int fd;               /* file descriptor for artic device driver */
ICAPARMS *prmptr;     /* Pointer to parameter block              */
{
   ARTIC_IOCTL_PARMS parms; /* Parm buffer for IOCTL call */

   if (ioctl(fd,ICAGETPARMS,(void *) &parms) == -1)
      return(E_ICA_INVALID_FD);
   else {
      if (parms.icagetparms.retcode == NO_ERROR)
         memcpy((void *) prmptr,(void *) &parms.icagetparms.cfgparms,
                sizeof(ICAPARMS));
      return(parms.icagetparms.retcode);
   }
}


/*********************************************************************/

ushort icagetadaptype(fd, typeptr)
/* Get adapter type */

int fd;           /* file descriptor for artic device driver */
int *typeptr;     /* Pointer to returned                     */
{
   ARTIC_IOCTL_PARMS parms; /* Parm buffer for IOCTL call */

   if (ioctl(fd,ICAGETADAPTYPE,(void *) &parms) == -1)
      return(E_ICA_INVALID_FD);
   else {
      if (parms.icagetadaptype.retcode == NO_ERROR)
          *typeptr = parms.icagetadaptype.type;
      return(parms.icagetadaptype.retcode);
   }
}


/*********************************************************************/

ushort icagetprimstat(fd, primstat)
/* Gets a tasks primary status byte */

int fd;               /* file descriptor for artic device driver */
uchar *primstat;      /* Address to store primary status byte    */
{
   ARTIC_IOCTL_PARMS parms; /* Parm buffer for IOCTL call */

   if (ioctl(fd,ICAGETPRIMSTAT,(void *) &parms) == -1)
      return(E_ICA_INVALID_FD);
   else {
      if (parms.icagetprimstat.retcode == NO_ERROR)
         *primstat = parms.icagetprimstat.psb;
      return(parms.icagetprimstat.retcode);
   }
}

/*********************************************************************/

ushort icagetbuffers(fd, ib, ob, ssb)
/* Gets the length and address of a tasks input buffer, */
/* output buffer and secondary status buffer            */
int fd;               /* file descriptor for artic device driver */
ICABUFFER *ib;        /* Pointer to parameter block              */
ICABUFFER *ob;        /* Pointer to parameter block              */
ICABUFFER *ssb;       /* Pointer to parameter block              */
{
   ARTIC_IOCTL_PARMS parms; /* Parm buffer for IOCTL call */

   if (ioctl(fd,ICAGETBUFADDRS,(void *) &parms) == -1)
      return(E_ICA_INVALID_FD);
   else {
      if (parms.icagetbufaddrs.retcode == NO_ERROR) {
         memcpy((void *) ib,(void *) &parms.icagetbufaddrs.ib,
                sizeof(ICABUFFER));
         memcpy((void *) ob,(void *) &parms.icagetbufaddrs.ob,
                sizeof(ICABUFFER));
         memcpy((void *) ssb,(void *) &parms.icagetbufaddrs.ssb,
                sizeof(ICABUFFER));
      }

      return(parms.icagetbufaddrs.retcode);
   }
}


/*********************************************************************/

ushort icainbuf(fd, ib)
/* Gets the length and address of a tasks input buffer */

int fd;               /* file descriptor for artic device driver */
ICABUFFER *ib;        /* Pointer to parameter block              */
{
   ARTIC_IOCTL_PARMS parms; /* Parm buffer for IOCTL call */

   if (ioctl(fd,ICAGETBUFADDRS,(void *) &parms) == -1)
      return(E_ICA_INVALID_FD);
   else {
      if (parms.icagetbufaddrs.retcode == NO_ERROR)
         memcpy((void *) ib,(void *) &parms.icagetbufaddrs.ib,
                sizeof(ICABUFFER));
      return(parms.icagetbufaddrs.retcode);
   }
}

/*********************************************************************/

ushort icaoutbuf(fd, ob)
/* Gets the length and address of a tasks output buffer */

int fd;               /* file descriptor for artic device driver */
ICABUFFER *ob;        /* Pointer to parameter block              */
{
   ARTIC_IOCTL_PARMS parms; /* Parm buffer for IOCTL call */

   if (ioctl(fd,ICAGETBUFADDRS,(void *) &parms) == -1)
      return(E_ICA_INVALID_FD);
   else {
      if (parms.icagetbufaddrs.retcode == NO_ERROR)
         memcpy((void *) ob,(void *) &parms.icagetbufaddrs.ob,
                sizeof(ICABUFFER));
      return(parms.icagetbufaddrs.retcode);
   }
}

/*********************************************************************/

ushort icasecstatbuf(fd, ssb)
/* Gets the length and address of a tasks secondary status buffer */

int fd;               /* file descriptor for artic device driver */
ICABUFFER *ssb;       /* Pointer to parameter block              */
{
   ARTIC_IOCTL_PARMS parms; /* Parm buffer for IOCTL call */

   if (ioctl(fd,ICAGETBUFADDRS,(void *) &parms) == -1)
      return(E_ICA_INVALID_FD);
   else {
      if (parms.icagetbufaddrs.retcode == NO_ERROR)
         memcpy((void *) ssb,(void *) &parms.icagetbufaddrs.ssb,
                sizeof(ICABUFFER));
      return(parms.icagetbufaddrs.retcode);
   }
}

/*********************************************************************/

ushort icasendconfig(fd, parmbuf)
/* Updates the Interface Block with MAXTASK, MAXQUEUE etc        */

int fd;               /* file descriptor for artic device driver */
ICAPARMS *parmbuf;    /* Config data                             */
{
   ARTIC_IOCTL_PARMS parms; /* Parm buffer for IOCTL call */

   memcpy(&parms.icasendconfig.cfgparms,parmbuf,sizeof(ICAPARMS));

   if (ioctl(fd,ICASENDCFG,(void *) &parms) == -1)
      return(E_ICA_INVALID_FD);
   else
      return(parms.icasendconfig.retcode);

}

/*********************************************************************/

ushort icareadio(fd, reg, value)
/* Reads a given I/O port from an adapter  */

int fd;               /* file descriptor for artic device driver */
uchar reg;            /* I/O address to read                     */
uchar *value;         /* Return value read here                  */
{
   ushort rc;
   ARTIC_IOCTL_PARMS parms; /* Parm buffer for IOCTL call */

   parms.icaioread.portnum = reg;

   if (ioctl(fd,ICAIOREAD,(void *) &parms) == -1)
      return(E_ICA_INVALID_FD);
   else
   {
      if (parms.icaioread.retcode == NO_ERROR)
         *value = parms.icaioread.value;

      return(parms.icaioread.retcode);
   }
}

/*********************************************************************/

ushort icawriteio(fd, reg, value)
/* Writes to a given I/O port on an adapter  */

int fd;               /* file descriptor for artic device driver */
uchar reg;            /* I/O address to read                     */
uchar value;         /* Return value read here                  */
{
   ARTIC_IOCTL_PARMS parms; /* Parm buffer for IOCTL call */

   parms.icaiowrite.portnum = reg;
   parms.icaiowrite.value = value;

   if (ioctl(fd,ICAIOWRITE,(void *) &parms) == -1)
      return(E_ICA_INVALID_FD);
   else
      return(parms.icaiowrite.retcode);
}

/*********************************************************************/

ushort icareadpos(fd, regnum, value)
/* Reads a POS register on an adapter  */

int fd;               /* file descriptor for artic device driver */
uchar regnum;         /* POS register to read                    */
uchar *value;         /* Return value read here                  */
{
   ushort rc;
   ARTIC_IOCTL_PARMS parms; /* Parm buffer for IOCTL call */

   parms.icaposread.regnum = regnum;

   if (ioctl(fd,ICAPOSREAD,(void *) &parms) == -1)
      return(E_ICA_INVALID_FD);
   else
   {
      if (parms.icaposread.retcode == NO_ERROR)
         *value = parms.icaposread.value;

      return(parms.icaposread.retcode);
   }
}

/*********************************************************************/

ushort icawritepos(fd, regnum, value)
/* Writes to a given POS register on an adapter  */

int fd;               /* file descriptor for artic device driver */
uchar regnum;         /* POS register to write                   */
uchar value;          /* Value to write                          */
{
   ARTIC_IOCTL_PARMS parms; /* Parm buffer for IOCTL call */

   parms.icaposwrite.regnum = regnum;
   parms.icaposwrite.value = value;

   if (ioctl(fd,ICAPOSWRITE,(void *) &parms) == -1)
      return(E_ICA_INVALID_FD);
   else
      return(parms.icaposwrite.retcode);
}

/*********************************************************************/

ushort icadmasetup(fd, type, buffer, count, physaddr)

int fd;               /* file descriptor for artic device driver */
int type;
uchar *buffer;
uint count;
ulong *physaddr;
{
   ARTIC_IOCTL_PARMS parms; /* Parm buffer for IOCTL call */

   parms.icadmasetup.uaddr = buffer;
   parms.icadmasetup.count = count;
   parms.icadmasetup.type = type;

   if (ioctl(fd,ICADMASETUP,(void *) &parms) == -1)
      return(E_ICA_INVALID_FD);
   else
   {
      *physaddr = parms.icadmasetup.physaddr;
      return(parms.icadmasetup.retcode);
   }
}

/*********************************************************************/

ushort icadmarel(fd)

int fd;               /* file descriptor for artic device driver */
{
   ARTIC_IOCTL_PARMS parms; /* Parm buffer for IOCTL call */

   if (ioctl(fd,ICADMAREL,(void *) &parms) == -1)
      return(E_ICA_INVALID_FD);
   else
      return(parms.icadmarel.retcode);
}

