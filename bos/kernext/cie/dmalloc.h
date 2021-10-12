/* @(#)48   1.6  src/bos/kernext/cie/dmalloc.h, sysxcie, bos411, 9428A410j 4/1/94 15:49:30 */

/* 
 * 
 * COMPONENT_NAME: (SYSXCIE) COMIO Emulator
 * 
 * FUNCTIONS:
 * 
 *   dmallocInit
 *   dmallocTerm
 *   dmallocReport
 *   d_save
 *   d_release
 *   dmalloc
 *   dmfree
 *   d_get
 *   d_gethdr
 *   d_getclr
 *   d_clgetm
 *   d_free
 *   d_freem
 *   xmalloc
 *   xmfree
 *   m_get
 *   m_gethdr
 *   m_getclr
 *   m_clgetm
 *   m_free
 *   m_freem
 * 
 * DESCRIPTION:
 * 
 *    Debug Version of xmalloc/xmfree
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

#if !defined(DMALLOC_H) && defined(DMALLOC)
#define  DMALLOC_H

/*---------------------------------------------------------------------------*/
/*                       Memory Allocation Type Codes                        */
/*---------------------------------------------------------------------------*/

typedef  enum MEM_TYPE       MEM_TYPE;

enum MEM_TYPE
{
   XMALLOC,
   MBUF,
   MCLUST
};

/*---------------------------------------------------------------------------*/
/*                      Initialize the hash table arena                      */
/*---------------------------------------------------------------------------*/

void
   dmallocInit(
      unsigned long          htabSize    ,
      unsigned long          memSize
   );

/*---------------------------------------------------------------------------*/
/*                      Release Memory tracking storage                      */
/*---------------------------------------------------------------------------*/

void
   dmallocTerm(
      void
   );

/*---------------------------------------------------------------------------*/
/*                       Display storage usage report                        */
/*---------------------------------------------------------------------------*/

void
   dmallocReport(
      void
   );

/*---------------------------------------------------------------------------*/
/*                Save a memory allocation in the hash table                 */
/*---------------------------------------------------------------------------*/

void
   d_save(
      void                 * p           ,
      unsigned long          size        ,
      MEM_TYPE               type        ,
      register char        * file        ,
      register char        * func        ,
      register int           line
   );

/*---------------------------------------------------------------------------*/
/*                        Release a memory allocation                        */
/*---------------------------------------------------------------------------*/

void
   d_release(
      void                 * p           ,
      MEM_TYPE               type
   );

/*---------------------------------------------------------------------------*/
/*                         Debug version of xmalloc                          */
/*---------------------------------------------------------------------------*/

void *
   dmalloc(
      register int           size        ,
      register int           align       ,
      register void        * heap        ,
      register char        * file        ,
      register char        * func        ,
      register int           line
   );

/*---------------------------------------------------------------------------*/
/*                          Debug version of xmfree                          */
/*---------------------------------------------------------------------------*/

int
   dmfree(
      register void        * p           ,
      register void        * heap
   );

/*---------------------------------------------------------------------------*/
/*                          Debug version of m_get                           */
/*---------------------------------------------------------------------------*/

struct mbuf *
   d_get(
      int                    wait        ,// I -Wait Flag
      int                    type        ,// I -Block Type
      register char        * file        ,
      register char        * func        ,
      register int           line
   );

/*---------------------------------------------------------------------------*/
/*                         Debug version of m_gethdr                         */
/*---------------------------------------------------------------------------*/

struct mbuf *
   d_gethdr(
      int                    wait        ,// I -Wait Flag
      int                    type        ,// I -Block Type
      register char        * file        ,
      register char        * func        ,
      register int           line
   );

/*---------------------------------------------------------------------------*/
/*                         Debug version of m_getclr                         */
/*---------------------------------------------------------------------------*/

struct mbuf *
   d_getclr(
      int                    wait        ,// I -Wait Flag
      int                    type        ,// I -Block Type
      register char        * file        ,
      register char        * func        ,
      register int           line
   );

/*---------------------------------------------------------------------------*/
/*                         Debug version of m_getclr                         */
/*---------------------------------------------------------------------------*/

struct mbuf *
   d_clgetm(
      struct mbuf          * m           ,// IO-mbuf to attach to
      int                    wait        ,// I -Wait Flag
      int                    size        ,// I -Cluster Size
      register char        * file        ,
      register char        * func        ,
      register int           line
   );

/*---------------------------------------------------------------------------*/
/*                          Debug version of m_free                          */
/*---------------------------------------------------------------------------*/

struct mbuf *
   d_free(
      struct mbuf          * m           ,
      register char        * file        ,
      register char        * func        ,
      register int           line
   );

/*---------------------------------------------------------------------------*/
/*                         Debug version of m_freem                          */
/*---------------------------------------------------------------------------*/

void
   d_freem(
      struct mbuf          * m           ,
      register char        * file        ,
      register char        * func        ,
      register int           line
   );

/*---------------------------------------------------------------------------*/
/*                   Macro to redirect xmalloc to dmalloc                    */
/*---------------------------------------------------------------------------*/

/*:::::::::::::::::: Pseudo-function prototype for macro ::::::::::::::::::::

void *
   xmalloc(
      int                    bytes       ,// I -Number of bytes
      int                    align       ,// I -Alignment
      void                 * heap         // I -Ptr to heap
   );

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/

#define  xmalloc(bytes,align,heap) dmalloc(bytes,align,heap,          \
                                           __FILE__,__FUNC__,__LINE__)

/*---------------------------------------------------------------------------*/
/*                     Macro to redirect xmfree to dfree                     */
/*---------------------------------------------------------------------------*/

/*:::::::::::::::::: Pseudo-function prototype for macro ::::::::::::::::::::

void
   xmfree(
      void                 * ptr         ,// I -Ptr to area to be freed
      void                 * heap         // I -Ptr to heap
   );

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/

#define  xmfree(ptr,heap)          dmfree(ptr,heap)

/*---------------------------------------------------------------------------*/
/*                     Macro to redirect m_get to d_get                      */
/*---------------------------------------------------------------------------*/

/*:::::::::::::::::: Pseudo-function prototype for macro ::::::::::::::::::::

void
   m_get(
      int                    wait        ,// I -Wait Flag
      int                    type         // I -mbuf Type
   );

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/

#define  m_get(wait,type)         d_get(wait,type,__FILE__,__FUNC__,__LINE__)

/*---------------------------------------------------------------------------*/
/*                  Macro to redirect m_gethdr to d_gethdr                   */
/*---------------------------------------------------------------------------*/

/*:::::::::::::::::: Pseudo-function prototype for macro ::::::::::::::::::::

void
   m_gethdr(
      int                    wait        ,// I -Wait Flag
      int                    type         // I -mbuf Type
   );

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/

#define  m_gethdr(wait,type)      d_gethdr(wait,type,__FILE__,__FUNC__,__LINE__)

/*---------------------------------------------------------------------------*/
/*                     Macro to redirect m_getclr to d_getclr                */
/*---------------------------------------------------------------------------*/

/*:::::::::::::::::: Pseudo-function prototype for macro ::::::::::::::::::::

void
   m_getclr(
      int                    wait        ,// I -Wait Flag
      int                    type         // I -mbuf Type
   );

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/

#define  m_getclr(wait,type)      d_getclr(wait,type,__FILE__,__FUNC__,__LINE__)

/*---------------------------------------------------------------------------*/
/*                     Macro to redirect m_clgetm to d_clgetm                */
/*---------------------------------------------------------------------------*/

/*:::::::::::::::::: Pseudo-function prototype for macro ::::::::::::::::::::

void
   m_clgetm(
      struct mbuf          * m           ,// IO-mbuf to attach to
      int                    wait        ,// I -Wait Flag
      int                    size         // I -Cluster size
   );

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/

#define  m_clgetm(m,wait,size)    d_clgetm(m,wait,size,__FILE__,__FUNC__,__LINE__)

/*---------------------------------------------------------------------------*/
/*                    Macro to redirect m_free to d_free                     */
/*---------------------------------------------------------------------------*/

/*:::::::::::::::::: Pseudo-function prototype for macro ::::::::::::::::::::

void
   m_free(
      void                 * p            // IO-Mbuf to be freed
   );

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/

#define  m_free(p)                d_free(p,__FILE__,__FUNC__,__LINE__)

/*---------------------------------------------------------------------------*/
/*                    Macro to redirect m_freem to d_freem                   */
/*---------------------------------------------------------------------------*/

/*:::::::::::::::::: Pseudo-function prototype for macro ::::::::::::::::::::

void
   m_freem(
      void                 * p            // IO-Mbuf chain to be freed
   );

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/

#define  m_freem(p)               d_freem(p,__FILE__,__FUNC__,__LINE__)

#endif
