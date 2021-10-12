/* @(#)95       1.2  src/bos/kernext/audio/acpa/acpadiags.h, sysxacpa, bos411, 9428A410j 7/11/91 18:54:12 */
/*
 * COMPONENT_NAME: SYSXACPA     Multimedia Audio Capture and Playback Adapter
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* These are the defines needed for diagnostics. */
#define ACPA_DIAGS      0x200           /* the diagnostics bit */
#define ACPA_DIAG1      ACPA_DIAGS | 1  /* first diagnostics setting */

/* 8-bit structure for 8-bit diagnostics requests. */
typedef struct _acpa_regs8
{
  uchar offset;                 /* offset to register's address */
  uchar data;                   /* the data for this request */
} acpa_regs8;

/* 16-bit structure for 16-bit diagnostics requests. */
typedef struct _acpa_regs16
{
  ushort offset;                /* offset to register's address */
  ushort data;                  /* the data for this request */
} acpa_regs16;

/* These are the defined ioctl commands for diagnostics. */
#define AUDIO_DIAGS8_READ       200     /* 8-bit read ioctl command */
#define AUDIO_DIAGS8_WRITE      201     /* 8-bit write ioctl command */
#define AUDIO_DIAGS16_READ      202     /* 16-bit read ioctl command */
#define AUDIO_DIAGS16_WRITE     203     /* 16-bit write ioctl command */
#define AUDIO_DIAGS_PUT         204     /* set interrupt counter */
#define AUDIO_DIAGS_GET         205     /* read interrupt counter */


