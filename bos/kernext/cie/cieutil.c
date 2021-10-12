static char sccsid[]="@(#)21   1.10  src/bos/kernext/cie/cieutil.c, sysxcie, bos411, 9438B411a 9/20/94 14:17:28";

/* 
 * 
 * COMPONENT_NAME: (SYSXCIE) COMIO Emulator
 * 
 * FUNCTIONS:
 * 
 *   dbginit
 *   dbgterm
 *   dbgwrt
 *   getEnv
 *   dbgout
 *   sortIOCTLdefTable
 *   uioToMbuf
 *   mbufToUio
 *   searchIODTable
 *   hexInt
 *   hexStr
 *   memDump
 * 
 * DESCRIPTION:
 * 
 *    COMIO Emulator Utility Routines
 * 
 * ORIGINS: 27
 * 
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 * 
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 * 
 */

#include "ciedd.h"
#include "cieutil.h"

#include <stdarg.h>
#include <stdio.h>
#include <sys/syslog.h>
#include <sys/file.h>
#include <sys/fp_io.h>
#include <sys/uio.h>
#include <sys/mbuf.h>

#include <sys/mstsave.h>
#include "dmalloc.h"


static Simple_lock           debugLock;
static const char            dbgFileVar [] = "CIE_TRACE_FILE";
static const char            dbgDevVar  [] = "CIE_TRACE_DEV";
static const char            dbgFileDflt[] = "/tmp/ciedd.dbg";
static const char            dbgDevDflt [] = "/dev/tty0";
static       char            dbgFile[256] = "";
static       char            dbgDev [32]  = "";


/*---------------------------------------------------------------------------*/
/*                          Initialize Debug Output                          */
/*---------------------------------------------------------------------------*/

void
   dbginit(
      void
   )
{
   char                    * p;

   /*-----------------------------*/
   /*  Initialize the debug lock  */
   /*-----------------------------*/

   lock_alloc(&debugLock,
              LOCK_ALLOC_PIN,
              CIO_LOCK_CLASS,
              CIE_LOCK_DEBUG);

   simple_lock_init(&debugLock);

   /*-----------------------*/
   /*  Set up output files  */
   /*-----------------------*/

   strcpy(dbgFile,((p = getEnv(dbgFileVar)) && *p) ? p : dbgFileDflt);
   strcpy(dbgDev ,((p = getEnv(dbgDevVar))  && *p) ? p : dbgDevDflt );
}

/*---------------------------------------------------------------------------*/
/*                          Terminate Debug Output                           */
/*---------------------------------------------------------------------------*/

void
   dbgterm(
      void
   )
{
   lock_free(&debugLock);
}

/*---------------------------------------------------------------------------*/
/*          Write a string to a debug file using fp_xxx operations           */
/*---------------------------------------------------------------------------*/

void
   dbgwrt(
      register const char  * fname       ,// I -Output File Name
      register const char  * data        ,// I -Data to be written
      register int           bytes        // I -Number of bytes
   )
{
   static int                oflags = O_WRONLY|O_CREAT|O_APPEND;
   static int                fmode  = 0744;

   struct file             * f;
   int                       written;
   register int              rc;

   simple_lock(&debugLock);

   if ((rc = fp_open((char *)fname,oflags,fmode,0,SYS_ADSPACE,&f)) != 0)
   {
      simple_unlock(&debugLock);
      return;
   }

   if (rc)
   {
      bsdlog(LOG_ALERT,"Unable to open %s: rc=%d errno=%d %m\n",fname,rc,errno);
      delay(HZ*2);
      simple_unlock(&debugLock);
      return;
   }

   rc = fp_write(f,(char *)data,bytes,0,SYS_ADSPACE,&written);

   if (rc)
   {
      bsdlog(LOG_ALERT,"Unable to write to %s: rc=%d errno=%d %m\n",fname,rc,errno);
      delay(HZ*2);
      simple_unlock(&debugLock);
      return;
   }

   if (written != bytes)
   {
      bsdlog(LOG_ALERT,"Write incomplete to %s: rc=%d errno=%d %m\n",fname,rc,errno);
      delay(HZ*2);
      simple_unlock(&debugLock);
      return;
   }

   rc = fp_close(f);

   if (rc)
   {
      bsdlog(LOG_ALERT,"Close Error on %s: rc=%d errno=%d %m\n",fname,rc,errno);
      delay(HZ*2);
      simple_unlock(&debugLock);
      return;
   }

   delay(HZ/15);

   simple_unlock(&debugLock);

   return;
}

/*---------------------------------------------------------------------------*/
/*                      Get Environment Variable Value                       */
/*---------------------------------------------------------------------------*/

const char *
   getEnv(
      const char           * v
   )
{
   extern char      ** environ;
   const char             ** e;

   if ((e = environ) != NULL)
   {
      while(*e != NULL)
      {
         const char        * p = v;
         const char        * q = *e;

         while(*p && *q && *p==*q && *q != '=') p++, q++;

         if (*p == 0 && *q == '=') return q+1;

         e++;
      }
   }

   return NULL;
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*       Format and write a debug output line - limited printf support       */
/*                                                                           */
/*         THIS ROUTINE MUST NOT BE CALLED WITH INTERRUPTS DISABLED          */
/*                                                                           */
/*                      csa->intpri must equal INTBASE                       */
/*                                                                           */
/*---------------------------------------------------------------------------*/

void
   dbgout(
      const char  * fmt                  ,// I -Format String
      ...                                 // I -Arguments to be formatted
   )
{
   static const char         fmtChars   [] = "diuosx";
   static char               fmtBuf     [] = "%x";

   char                    * f;
   char                      workarea[8192];
   int                       pidLen  = 0;
   short                     newLine = 0;

   register char           * i = fmt;
   register char           * o = workarea;

   va_list                   arglist;

   /*------------------------------------------------*/
   /*  No output if called with interrupts disabled  */
   /*------------------------------------------------*/

   /* if (csa->intpri != INTBASE) return; */

   while(*i && *i == '\n') *o++ = *i++;

   if (*i == 0) return;

   o += (pidLen = sprintf(o,"\rpid=%d: ",getpid())-1) + 1;

   va_start(arglist,fmt);

   while(*i)
   {
      switch(*i)
      {
         case '%':
         {
            char *p = fmtChars;

            while(*p && *p != i[1]) p++;

            if (*p)
            {
               fmtBuf[1] = *p;

               if (*p == 's')
                  o += sprintf(o,fmtBuf,va_arg(arglist,char *));
               else
                  o += sprintf(o,fmtBuf,va_arg(arglist,int));

               i++;
            }
            else
               *(o++) = '%';

            newLine = 0;

            break;
         }

         case '\n':
         {
            int              b = pidLen;
            *(o++) = '\n';
            *(o++) = '\r';
            while(b--) *(o++) = ' ';
            newLine = 1;

            break;
         }

         default:
         {
            *(o++)  = *i;
            newLine = 0;

            break;
         }
      }

      i++;
   }

   /*-----------------------------------------------------------*/
   /*  If the last input char was \n, adjust back; else add \n  */
   /*-----------------------------------------------------------*/

   if (newLine)
      o -= pidLen;
   else
      *(o++) = '\n';

   *(o++) = '\r';

   va_end(arglist);

   if (dbgFile) dbgwrt(dbgFile,workarea,o-workarea);

   if (dbgDev)  dbgwrt(dbgDev ,workarea,o-workarea);

   return;
}

/*---------------------------------------------------------------------------*/
/*             Sort IOCTL Definition Table by IOCTL Command Code             */
/*---------------------------------------------------------------------------*/

int
   sortIOCTLdefTable(
      register IODT_ENTRY  * table       ,// IO-Ptr to array of entries
      register int           size         // I -Number of entries
   )
{
   FUNC_NAME(sortIOCTLdefTable);

   register int              end = size-1;
   register int              i;

   /*-----------------------------------------------------------------------*/
   /*  This is a simple bubble-sort -- Gets executed once per devType only  */
   /*-----------------------------------------------------------------------*/

   while(end > 0)
   {
      register int           lswap = 0;

      for (i=0; i<end; i++)
      {
         if (table[i].cmd > table[i+1].cmd)
         {
            IODT_ENTRY       temp;

            temp       = table[i];
            table[i]   = table[i+1];
            table[i+1] = temp;
            lswap      = i;
         }
      }

      end = lswap;
   }

   /*---------------------------------------*/
   /*  Check sort order and check for dups  */
   /*---------------------------------------*/
   {
      int                    prev = -1;

      for (i=0; i<size; i++)
      {
         if ((table[i].cmd <= prev)) return 1;
         prev = table[i].cmd;
      }
   }

   return 0;
}

/*---------------------------------------------------------------------------*/
/*                Copy the contents of a uio to an mbuf chain                */
/*---------------------------------------------------------------------------*/

int
   uioToMbuf(
      register mbuf_t     ** mp          ,//  O-Return area for chain head ptr
      register uio_t       * uiop         // IO-uio pointer
   )
{
   FUNC_NAME(uioToMbuf);

   mbuf_t                  * chain      = NULL;   // Ptr to first mbuf
   register mbuf_t        ** chptr      = &chain; // Ptr to fwd link ptr

   /*------------------------------------------------*/
   /*  Initialize output mbuf chain pointer to NULL  */
   /*------------------------------------------------*/

   *mp = NULL;

   /*---------------------------------------------*/
   /*  Loop as long as the uio has residual data  */
   /*---------------------------------------------*/

   while(uiop->uio_resid > 0)
   {
      register mbuf_t      * m;          // The current mbuf in the chain
      register int           mSize;      // Mbuf data capacity
      register int           segLen;     // The amt of data to move this time
      int                    rc;         // Return Code

      /*--------------------------------------------*/
      /*  Allocate a new mbuf of the required type  */
      /*--------------------------------------------*/

      if (chain == NULL)
      {
         /*----------------------------------------*/
         /*  First time, get a Packet Header mbuf  */
         /*----------------------------------------*/

         if ((m = m_gethdr(M_WAIT,MT_HEADER)) == NULL) return ENOMEM;

         m->m_pkthdr.len = uiop->uio_resid;
         mSize           = MHLEN;
      }
      else
      {
         /*---------------------------------------------*/
         /*  Additional mbuf, get a 'normal' data mbuf  */
         /*---------------------------------------------*/

         if ((m = m_get(M_WAIT,MT_DATA)) == NULL)
         {
            m_freem(chain);
            return ENOMEM;
         }

         mSize           = MLEN;
      }

      /*-------------------------------------------------------*/
      /*  Will the remaining data fit into the mbuf directly?  */
      /*-------------------------------------------------------*/

      if (uiop->uio_resid <= mSize)
      {
         /*---------------------------*/
         /*  Move all remaining data  */
         /*---------------------------*/

         segLen = uiop->uio_resid;
      }
      else
      {
         /*-----------------------------------------------------------*/
         /*  If not, allocate a cluster and calculate Segment Length  */
         /*-----------------------------------------------------------*/

         if (m_clgetm(m,M_WAIT,MCLBYTES) == 0)
         {
            m_free(m);
            if (chain) m_freem(chain);
            return ENOMEM;
         }

         segLen = min(uiop->uio_resid,MCLBYTES);
      }

      /*--------------------------------*/
      /*  Move the data, using uiomove  */
      /*--------------------------------*/

      rc = uiomove(MTOD(m,caddr_t),       // Ptr to mbuf data area
                   segLen,                // Number of bytes
                   UIO_WRITE,             // Move from user to kernel
                   uiop);                 // uio containing data

      if (rc)
      {
         m_free(m);
         if (chain) m_freem(chain);
         return rc;
      }

      /*-------------------------------------------*/
      /*  Set the actual data length in this mbuf  */
      /*-------------------------------------------*/

      m->m_len = segLen;

      /*-------------------------------------*/
      /*  Add to the mbuf chain being built  */
      /*-------------------------------------*/

      *chptr = m;
      chptr  = &m->m_next;
   }

   /*-----------------------------------------------------*/
   /*  Place the chain head pointer into the return area  */
   /*-----------------------------------------------------*/

   *mp = chain;

   return 0;
}

/*---------------------------------------------------------------------------*/
/*                Copy the contents of an mbuf chain to a uio                */
/*---------------------------------------------------------------------------*/

int
   mbufToUio(
      CIE_DEV_TYPE           devType     ,// I -Device Type Code
      register uio_t       * uiop        ,// IO-uio pointer
      register mbuf_t     ** chain        // IO-Ptr to head of mbuf chain
   )
{
   FUNC_NAME(mbufToUio);

   register mbuf_t         * m      = *chain;
   register int              rc;
   int                       riLen;
   int                       riPad;

   /*-----------------------------------*/
   /*  Special handling by device-type  */
   /*-----------------------------------*/

   switch(devType)
   {
      case CIE_DEV_TOK:
      {
         /*------------------------------------------------------------*/
         /*  Pad the routing info, if present, out to a full 18 bytes  */
         /*------------------------------------------------------------*/

         TOK_FRAME_HDR     * f = mtod(m,TOK_FRAME_HDR *);

         riLen = (TOK_FRAME_HAS_ROUTING_INFO(f))
                    ? TOK_ROUTING_INFO_LENGTH(f)
                    : 0;
         riPad = sizeof(TOK_ROUTING_INFO) - riLen;

         break;
      }

      /*--------------------------------------------------*/
      /*  Ethernet and FDDI have no special requirements  */
      /*--------------------------------------------------*/

      case CIE_DEV_ENT:
      case CIE_DEV_FDDI:
         riLen = 0;
         riPad = 0;
         break;
   }

   /*-----------------------------------------------------------------*/
   /*  Determine whether or not the frame will fit in the output buf  */
   /*-----------------------------------------------------------------*/

   rc = (m->m_pkthdr.len+riPad > uiop->uio_resid) ? EMSGSIZE : 0;

   /*-----------------------------*/
   /*  Perform padding if needed  */
   /*-----------------------------*/

   if (riPad != 0)
   {
      static char            nulls[sizeof(TOK_ROUTING_INFO)];

      int                    len_1 = offsetof(TOK_FRAME_HDR,d) + riLen;
      int                    len_2 = m->m_len - len_1;

      memset(nulls,0x00,sizeof(nulls));

      uiomove(mtod(m,caddr_t)       ,len_1 ,UIO_READ ,uiop);
      uiomove(nulls                 ,riPad ,UIO_READ ,uiop);
      uiomove(mtod(m,caddr_t)+len_1 ,len_2 ,UIO_READ ,uiop);

      m = m_free(m);
   }

   /*----------------------------------------------------*/
   /*  Move the data to the uio, freeing mbufs as we go  */
   /*----------------------------------------------------*/

   while(m)
   {
      if (uiop->uio_resid > 0)
         uiomove(mtod(m,caddr_t),m->m_len,UIO_READ,uiop);
      m = m_free(m);
   }

   *chain = NULL;

   return rc;
}

/*---------------------------------------------------------------------------*/
/*         Search the IOCTL Dispatch Table for a given command code          */
/*---------------------------------------------------------------------------*/

IODT_ENTRY *
   searchIODTable(
      register int                cmd         ,// I -Command Code
      register const IODT_ENTRY * iodTable    ,// I -IOCTL Dispatch Table
      register                    iodtSize     // I -Number of entries in table
   )
{
   register int              lo = -1;
   register int              hi = iodtSize;

   while((hi-lo)>1)
   {
      register int           i = (lo+hi)/2;

      if (iodTable[i].cmd < cmd)
         lo = i;
      else if (iodTable[i].cmd > cmd)
         hi = i;
      else
         return &(iodTable[i]);
   }

   return NULL;
}

/*---------------------------------------------------------------------------*/
/*                      Format an int into a hex string                      */
/*---------------------------------------------------------------------------*/

static
int
   hexInt(
      unsigned int           in          ,// I -Data to be formatted
      char                 * out         ,//  O-Output Buffer
      int                    width        // I -Output width
   )
{
   static char               hexChar[16]  = "0123456789ABCDEF";
   char                    * p;

   p = out + (width < 1 ? 1 : (width > 8 ? 8 : width));

   *p = 0;
   while(p > out)
   {
      *(p--) = hexChar[in & 0x0000000F];
      in >>= 4;
   }

   return(in != 0);
}

/*---------------------------------------------------------------------------*/
/*                Format a string into its hex representation                */
/*---------------------------------------------------------------------------*/

char *
   hexStr(
      char                 * out         ,//  O-Output Buffer
      int                    olen        ,// I -Output width
      const char           * in          ,// I -Input buffer
      int                    ilen         // I -Input length
   )
{
   static char               hexChar[16]  = "0123456789ABCDEF";
   register char           * p = out;
   register char           * i = in;
   register char           * e = in + min(ilen,olen/2);

   while(i < e)
   {
      *(p++) = hexChar[(((unsigned int)(*i)) >> 4) & 0x0000000F];
      *(p++) = hexChar[( (unsigned int)(*i))       & 0x0000000F];

      i++;
   }

   *p = 0x00;

   return out;
}

/*---------------------------------------------------------------------------*/
/*                              Hex Memory Dump                              */
/*---------------------------------------------------------------------------*/

int
   memDump(
      char                 * out         ,// IO-Output Buffer
      int                    indent      ,// I -Output Line Indent
      int                    width       ,// I -Output Line Width (src bytes)
      DUMPOPT                opt         ,// I -Dump Options
      void                 * data        ,// I -Source Data
      int                    len          // I -Length of Source
   )
{
   int                       totLen = 0; // Total formatted output length
   char                    * s;          // Ptr to data
   char                    * e;          // Ptr to end of data
   char                    * lo;         // s rounded down to width
   char                    * hi;         // e rounded up to width

   char                    * p;          // Scan pointer
   char                      buffer[160];// Work buffer

   static char               hexChar[16]  = "0123456789ABCDEF";

   static char               charSet[256] =
   {
      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
   };

   width  = ((width > 16) ? 32 : ((width > 8) ? 16 : 8));
   indent = min(indent,30);

   s  = data;
   e  = s + len;

   if (opt == dumpMem)
   {
      lo = (char *)(((unsigned long)s)         & ~((unsigned long)width-1));
      hi = (char *)(((unsigned long)(e+width)) & ~((unsigned long)width-1));
   }
   else
   {
      lo = s;
      hi = lo + ((unsigned long)(len+width-1) & ~((unsigned long)width-1));
   }

   p = lo;

   while(p < hi)
   {
      int                    i;
      char                 * dx = buffer+indent;
      char                 * dc;

      memset(buffer,' ',sizeof(buffer));

      if (opt == dumpMem)
      {
         hexInt((unsigned int)p,dx,8);
         dx += 10;
      }
      else
      {
         const int           width  = (len & 0xFFFF0000) ? 8 : 4;
         unsigned int        curOff = p-lo;

         hexInt(curOff,dx,width);
         dx += width+2;
      }

      dc = dx + 2*width + width/4 + 2;

      for(i=0; i<width; i++)
      {
         if (p < s || p >= e)
         {
            *dx++ = ' ';
            *dx++ = ' ';
            *dc++ = ' ';
         }
         else
         {
            *dx++ = hexChar[*p>>4 & 0x0F];
            *dx++ = hexChar[*p    & 0x0F];
            *dc++ = (char)(charSet[*p] ? *p : '.');
         }

         p++;

         dx += (i%4 == 3);
      }

      *++dx = '<';
      *dc++ = '>';
      *dc++ = '\n';
      *dc   = 0;

      strcpy(out,buffer);

      out    += (dc-buffer);
      totLen += (dc-buffer);
   }

   return totLen;
}
