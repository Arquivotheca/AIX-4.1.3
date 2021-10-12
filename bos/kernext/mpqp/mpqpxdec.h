/* @(#)51     1.1  src/bos/kernext/mpqp/mpqpxdec.h, sysxmpqp, bos411, 9434B411a 8/22/94 16:30:53 */
/*
 * COMPONENT_NAME: (MPQP) Multiprotocol Quad Port Device Driver
 *
 * FUNCTIONS: mpqpxdec.h - general header file
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifdef _POWER_MP
/*�����������������������������������������������������������������Ŀ
� global adapter device driver mp lock word                         �
�������������������������������������������������������������������*/
extern Simple_lock  mpqp_mp_lock;

/*�����������������������������������������������������������������Ŀ
� MP lock word for IOCTLs                                           �
�������������������������������������������������������������������
extern Simple_lock  mpqp_ioctl_lock; */

/*�����������������������������������������������������������������Ŀ
� MP lock word for interrupt handler                                �
�������������������������������������������������������������������*/
extern Simple_lock mpqp_intr_lock;

/*�����������������������������������������������������������������Ŀ
� MP lock word for internal tracing                                 �
�������������������������������������������������������������������*/
extern Simple_lock mpqp_trace_lock;
#endif /* _POWER_MP */
