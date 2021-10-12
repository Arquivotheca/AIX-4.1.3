/* @(#)46  1.6  src/bos/kernext/ient/i_entsupp.h, sysxient, bos411, 9438B411a 9/21/94 20:54:32 */
/****************************************************************************/
/*
 *   COMPONENT_NAME: SYSXIENT
 *
 *   FUNCTIONS: AIXTRACE
 *		COPYIN
 *		COPYOUT
 *		FREEMBUF
 *		M_INPAGE
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/****************************************************************************/
#ifndef _H_IENT_SUPP
#define _H_IENT_SUPP

/*
 * Macros to get/put caller's parameter block when only an address is supplied.
 * Useful for "arg" in ioctl and for extended paramters on other dd entries.
 * Value is 0 if ok, otherwise see errno.h.  Typical usage is:
 * if (rc = COPYIN (devflag, arg, &localdata, sizeof(localdata)))
 *    return (rc);
 */
#define COPYIN(dvf,usa,dda,siz)                               \
( ((char *)usa == (char *)NULL) ? (EFAULT) :                  \
      ( (dvf & DKERNEL) ? (bcopy(usa,dda,siz), 0) :           \
            ( ((rc = copyin(usa,dda,siz)) != 0) ? rc : 0 ) ) )

#define COPYOUT(dvf,dda,usa,siz)                              \
( ((char *)usa == (char *)NULL) ? (EFAULT) :                  \
      ( (dvf & DKERNEL) ? (bcopy(dda,usa,siz), 0) :           \
            ( ((rc = copyout(dda,usa,siz)) != 0) ? rc : 0 ) ) )
/*
 * This macro determines if the data portion of an mbuf resides within one
 * page -- if TRUE is returned, the data does not cross a page boundary. If
 * FALSE is returned, the data does cross a page boundary and cannot be 
 * d_mastered.
 */
#define M_INPAGE(m)                                             \
((((int)MTOD((m), uchar *) & ~(PAGESIZE - 1)) + PAGESIZE) >=    \
   ((int)MTOD((m), uchar *) + (m)->m_len))

#endif /* _H_IENT_SUPP */
