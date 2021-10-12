/* @(#)87    1.22  src/bos/kernel/sys/trcctl.h, systrace, bos411, 9428A410j 9/28/93 04:34:59 */
/*
 * @BULL_COPYRIGHT@
 */

/*
 * COMPONENT_NAME:            include/sys/trcctl.h
 *
 * FUNCTIONS:  header file for ioctl's to /dev/systrace and /dev/systrctl
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_TRCCTL
#define _H_TRCCTL

#define	TRCIOC ('C'<<8)

#define CFGMTRL (TRCIOC|0x1)
#define CFGMTRA (TRCIOC|0x2)
#define CFGMTRF (TRCIOC|0x3)
#define CFGBTR  (TRCIOC|0x4)

#define TRCSYNC (TRCIOC|0x7)
#define TRCON   (TRCIOC|0x8)
#define TRCOFF  (TRCIOC|0x9)
#define TRCSTOP (TRCIOC|0xA)
#define TRCSTAT (TRCIOC|0xB)
#define TRC_LOADER    (TRCIOC|0xC)
#define TRCIOC_CLOSE  (TRCIOC|0xD)
#define TRCIOC_LOGCFG (TRCIOC|0xE)
#define TRCIOC_LOGIO  (TRCIOC|0xF)

#define UTRCPRINT    (TRCIOC|0x10)
#define UTRCPANIC    (TRCIOC|0x11)
#define UTRCSNAPINFO (TRCIOC|0x14)
#define UTRCSNAPCTL  (TRCIOC|0x17)
#define UTRCCALIB    (TRCIOC|0x18)
#define UTRCHDR      (TRCIOC|0x19)

#define TRCIACTIVE	(TRCIOC|0x20)

#define MDEV_SYSTRACE  0
#define MDEV_SYSTRCTL  1
#define MDEV_SYSMEM    2
#define MDEV_SYSUTIL   3
#define MDEV_SYSNULL   4
#define MDEV_SYSTRACE1 5
#define MDEV_SYSTRACE2 6
#define MDEV_SYSTRACE3 7
#define MDEV_SYSTRCTL1 8
#define MDEV_SYSTRCTL2 9
#define MDEV_SYSTRCTL3 10
#define MDEV_SYSTRACE4 11
#define MDEV_SYSTRACE5 12
#define MDEV_SYSTRACE6 13
#define MDEV_SYSTRACE7 14
#define MDEV_SYSTRCTL4 15
#define MDEV_SYSTRCTL5 16
#define MDEV_SYSTRCTL6 17
#define MDEV_SYSTRCTL7 18

struct tr_stat {
	int trst_mode;
	int trst_cmd;
	int trst_ovfcount;
	int trst_iactive;
};

struct tr_struct {
	int tr_tbufsize;				/* trace data buffer size in bytes */
	unsigned char tr_events[32*16];	/* events to trace */
};

struct trc_lv {						/* loader values for TRC_LOADER */
	char *lv_start;
	int   lv_size;
	char  lv_name[16];
};

/* add bull */
struct wtrc_lv {
        char *lv_start;
        int   lv_size;
        char  lv_name[16];
        char  lv_fullname[125];
        unsigned int  lv_flags;
};
/* end bull */

#define TRC_NLV 100					/* max number of trc_lvs returned */

#define TRC_TBUFUMAX (1024 * 1024)	/* max size of trace buffer for user */

#endif /* _H_TRCCTL */

