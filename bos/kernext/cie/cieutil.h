/* @(#)42   1.7  src/bos/kernext/cie/cieutil.h, sysxcie, bos411, 9428A410j 4/18/94 16:21:08 */

/* 
 * 
 * COMPONENT_NAME: (SYSXCIE) COMIO Emulator
 * 
 * FUNCTIONS:
 * 
 *   TOK_FRAME_HAS_ROUTING_INFO
 *   TOK_ROUTING_INFO_LENGTH
 *   dbginit
 *   dbgterm
 *   dbgwrt
 *   getEnv
 *   dbgout
 *   sortIOCTLdefTable
 *   uioToMbuf
 *   mbufToUio
 *   searchIODTable
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

#if !defined(CIEUTIL_H)
#define  CIEUTIL_H


/*---------------------------------------------------------------------------*/
/*                             Token Ring Frame                              */
/*---------------------------------------------------------------------------*/

typedef struct TOK_FRAME_HDR      TOK_FRAME_HDR;
typedef struct TOK_ROUTING_INFO   TOK_ROUTING_INFO;

struct TOK_ROUTING_INFO
{
   unsigned short control;
   unsigned short segment[8];
};

#define  TOK_ROUTING_INFO_PRESENT        0x80
#define  TOK_ROUTING_INFO_LENGTH_MASK    0x1F00
#define  TOK_FRAME_HAS_ROUTING_INFO(f)   (((f)->srcAddr[0] & TOK_ROUTING_INFO_PRESENT) != 0)
#define  TOK_ROUTING_INFO_LENGTH(f)      (((f)->d.r.info.control & TOK_ROUTING_INFO_LENGTH_MASK) >> 8)

struct TOK_FRAME_HDR
{
   unsigned char             accessControl;
   unsigned char             frameControl;
   unsigned char             destAddr[6];
   unsigned char             srcAddr[6];
   union
   {
      struct
      {
         TOK_ROUTING_INFO    info;
         unsigned char       data[1];
      }                      r;
      unsigned char          data[1];
   } d;
};

/*---------------------------------------------------------------------------*/
/*                               Dump Options                                */
/*---------------------------------------------------------------------------*/

typedef enum DUMPOPT         DUMPOPT;

enum DUMPOPT
{
   dumpMem,                  // Format addresses as memory (seg:off) refs
   dumpRel                   // Format addresses as relative offsets
};

/*---------------------------------------------------------------------------*/
/*                          Initialize Debug Output                          */
/*---------------------------------------------------------------------------*/

void
   dbginit(
      void
   );

/*---------------------------------------------------------------------------*/
/*                          Terminate Debug Output                           */
/*---------------------------------------------------------------------------*/

void
   dbgterm(
      void
   );

/*---------------------------------------------------------------------------*/
/*          Write a string to a debug file using fp_xxx operations           */
/*---------------------------------------------------------------------------*/

void
   dbgwrt(
      register const char  * fname       ,// I -Output File Name
      register const char  * data        ,// I -Data to be written
      register int           bytes        // I -Number of bytes
   );

/*---------------------------------------------------------------------------*/
/*                      Get Environment Variable Value                       */
/*---------------------------------------------------------------------------*/

const char *
   getEnv(
      const char           * v
   );

/*---------------------------------------------------------------------------*/
/*       Format and write a debug output line - limited printf support       */
/*---------------------------------------------------------------------------*/

void
   dbgout(
      const char  * fmt                  ,// I -Format String
      ...                                 // I -Arguments to be formatted
   );

/*---------------------------------------------------------------------------*/
/*             Sort IOCTL Definition Table by IOCTL Command Code             */
/*---------------------------------------------------------------------------*/

int
   sortIOCTLdefTable(
      register IODT_ENTRY  * table       ,// IO-Ptr to array of entries
      register int           size         // I -Number of entries
   );

/*---------------------------------------------------------------------------*/
/*                Copy the contents of a uio to an mbuf chain                */
/*---------------------------------------------------------------------------*/

int
   uioToMbuf(
      register mbuf_t     ** mp          ,//  O-Return area for chain head ptr
      register uio_t       * uiop         // IO-uio pointer
   );

/*---------------------------------------------------------------------------*/
/*                Copy the contents of an mbuf chain to a uio                */
/*---------------------------------------------------------------------------*/

int
   mbufToUio(
      CIE_DEV_TYPE           devType     ,// I -Device Type Code
      register uio_t       * uiop        ,// IO-uio pointer
      register mbuf_t     ** chain        // IO-Ptr to head of mbuf chain
   );

/*---------------------------------------------------------------------------*/
/*         Search the IOCTL Dispatch Table for a given command code          */
/*---------------------------------------------------------------------------*/

IODT_ENTRY *
   searchIODTable(
      register int                cmd         ,// I -Command Code
      register const IODT_ENTRY * iodTable    ,// I -IOCTL Dispatch Table
      register                    iodtSize     // I -Number of entries in table
   );

/*---------------------------------------------------------------------------*/
/*                Format a string into its hex representation                */
/*---------------------------------------------------------------------------*/

char *
   hexStr(
      char                 * out         ,//  O-Output Buffer
      int                    olen        ,// I -Output width
      const char           * in          ,// I -Input buffer
      int                    ilen         // I -Input length
   );

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
   );

#endif
