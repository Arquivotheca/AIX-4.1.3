/* @(#)77	1.1  src/bos/usr/lib/methods/common/lducode.h, cfgmethods, bos411, 9428A410j 8/2/92 15:09:31 */
/*---------------------------------------------------------------------------
 *
 * COMPONENT_NAME: CFGMETHODS
 *
 * FUNCTIONS: lducode.h
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
 *
 *---------------------------------------------------------------------------
 */

/*�����������������������������������������������������������������������Ŀ
�                                                                         �
� PURPOSE: Data structure definitions for microcode loader and its        �
�          support routines                                               �
��������������������������������������������������������������������������*/

/*���������������������������������������������������������Ŀ
 � POS register information used by routines that access the�
 � adapters directly                                        �
 �����������������������������������������������������������*/
typedef struct
{
  uint win_base_addr;      /* Address of Shared Storage Window         */
  int  baseio;             /* Base I/O Address of adapter              */
  int  win_size;           /* Size (in bytes) of Shared Storage Window */
} POS_INFO;

/*�����������������������������������Ŀ
 � BCB information for an adapter task�
 �������������������������������������*/
typedef struct
{
  uchar  outpage;          /* Output buffer page             */
  ushort outoffset;        /* Output buffer offset           */
  uchar  inpage;           /* Input buffer page              */
  ushort inoffset;         /* Input buffer offset            */
  uchar  sspage;           /* Secondary status buffer page   */
  ushort ssoffset;         /* Secondary status buffer offset */
} BUFFER_ADDRS;

