/* @(#)47       1.26.1.5  src/bos/kernel/sys/devinfo.h, sysio, bos41J, 9513A_all 3/28/95 17:04:20 */
/*
 * COMPONENT_NAME: (SYSIO) System I/O
 *
 * FUNCTIONS: IO structure declarations and label definitions.
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991,1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */




#ifndef _H_DEVINFO
#define _H_DEVINFO

#ifndef IFNAMSIZ
#define IFNAMSIZ        16
#endif

#ifndef IFF_UP
#define IFF_UP          0x01
#endif

#ifndef IFF_RUNNING
#define IFF_RUNNING     0x40
#endif

#ifndef IFF_PRIMARY
#define IFF_PRIMARY     0x80
#endif

#ifndef MAXIFS
#define MAXIFS          8
#endif

#include <sys/types.h>

/*
 * Device information
 */
struct devinfo
{       char    devtype;
        char    flags;
        char    devsubtype;             /* device sub type */
        union
        {   struct      /* for disks */
            {   short   bytpsec;        /* bytes per sector */
                short   secptrk;        /* sectors per track */
                short   trkpcyl;        /* tracks per cylinder */
                long    numblks;        /* blocks this partition */
                uint    segment_size;   /* segment size for transfer stats */
                uint    segment_count;  /* segment count for transfer stats */
                uint    byte_count;     /* byte count for transfer stats */
            } dk;
	    struct	/* for SCSI target mode */
	    {
		uchar	scsi_id;	/* SCSI ID of attached initiator */
		uchar	lun_id;		/* LUN of attached initiator */
					/*   (or 0 for target instance) */
		uint	buf_size;	/* size of receive buffer (bytes) */
		uint	num_bufs;	/* number of receive buffers */
		long	max_transfer;	/* max request to SCSI DD */
		dev_t	adapter_devno;	/* device maj/min of SCSI adapter */
	    } tmscsi;
            struct      /* for SCSI or IDE disks */
            {   short   blksize;        /* block size (in bytes) */
                long    numblks;        /* total number of blocks */
                long    max_request;    /* max request to DD      */
                uint    segment_size;   /* segment size for transfer stats */
                uint    segment_count;  /* segment count for transfer stats */
                uint    byte_count;     /* byte count for transfer stats */
            } scdk, idedk;
            struct      /* for memory mapped displays */
            {   char    capab;          /* capabilities */
                char    mode;           /* current mode */
                short   hres;           /* horizontal resolution */
                short   vres;           /* vertical resolution */
            } tt;
            struct      /* for ethernet/IEEE 802.3 interface                 */
            {
                unsigned int    broad_wrap; /* allows broadcast wrap of data */
                                            /* 1 = supported                 */
                                            /* 0 = not supported             */
                int             rdto;       /* Receive Data Transfer offset  */
                unsigned char   haddr[6];   /* burned in hardware address    */
                unsigned char   net_addr[6]; /* Current network address      */
            } ethernet;
            struct      /* for Token-Ring interfaces */
            {   unsigned short   speed; /* 4/16 Mbps ring speed */
                                        /* 0 = 4 Mbps  */
                                        /* 1 = 16 Mbps */
                unsigned int    broad_wrap;     /* supports broadcast */
                                                /* wrap of data? */
                                                /* TRUE = supported */
                                                /* FALSE = not supported */
                int      rdto;  /* Receive Data Transfer offset */
                unsigned char     haddr[6];      /* hardware address */
                unsigned char     net_addr[6];   /* Current network address */
            } token;
            struct      /* for Serial Optical Link interfaces */
	    {   unsigned int    broad_wrap;     /* supports broadcast */
                                                /* wrap of data? */
                                                /* TRUE = supported */
                                                /* FALSE = not supported */
                int	rdto;  /* Receive Data Transfer offset */
		uchar	processor_id;		/* source id # 	*/
            } sol;
	    struct	/* for FDDI interfaces */
	    {
		uint	broad_wrap;		/* support broadcast */
						/* wrap of data? */
						/* TRUE = supported */
						/* FALSE = not supported */
		int	rdto;	/* Receive Data Transfer offset */
		uint	attach_class;	/* Attachment Class */
					/* 0 = Single Attachment Station */
					/* 1 = Dual Attachment Station */
		uchar 	haddr[6];		/* hardware address */
		uchar	netaddr[6];		/* Current network addr */
	    } fddi;
            struct      /* for block i/o device */
            {
              struct
               {
                char    type;           /* hardware type: ethernet or
                                                token ring */
                char    if_flags;       /* up/down:  1=ATTACHED
                                                     2=RUNNING
                                                     3=PRIMARY INTERFACE */
                char    haddr[6];       /* hardware address: ethernet or
                                                token ring */
                long    mymach;         /* local IP adress */
                long    subnet_mask;    /* subnet mask */
                int     mtu;            /* maximum transmission unit */
                char    if_name[IFNAMSIZ]; /* name of interface         */
                int     if_remmtu;      /* MTU to remote */
                unsigned short  hdr_broadcast;  /* ring=0, local=1      */
               } lan[MAXIFS];
            } bio;
            struct      /* for magnetic tapes */
            {   short   type;           /* what flavor of tape */
                                        /* defined below */
            } mt;
            struct      /* for SCSI magnetic tapes */
            {   short   type;           /* what flavor of tape */
                                        /* defined below */
                int     blksize;        /* block size (in bytes) */
            } scmt;
            struct      /* for SCSI or IDE CDROMs */
            {   short   blksize;        /* block size (in bytes) */
                long    numblks;        /* total number of blocks */
            } sccd, idecd;
            struct      /* for IDE adapters */
            {   char    resv1;          /* reserved */
                long    max_transfer;   /* maximum transfer size allowed (in bytes) */
            } ide;
            struct      /* for SCSI adapters */
            {   char    card_scsi_id;   /* SCSI Id of the adapter */
                long    max_transfer;   /* maximum transfer size allowed (in bytes) */
            } scsi;
            struct                      /* dump information */
            {
                dev_t   primary;        /* primary dump device */
                dev_t   secondary;      /* secondary dump device */
                long    mdt_size;
            }   dump;
            struct /* for S/370 Channel PCA adapters */
            {
                uchar broad_wrap;       /* always zero */
                ulong rdto;             /* Receive Data Transfer Offset */
                ulong hi_adap_name;     /* Adapter hardware name */
                ulong lo_adap_name;     /*    "       "      "   */
                ulong hi_ucode_name;    /* Microcode name */
                ulong lo_ucode_name;    /*    "       "   */
                uchar chnl_speed;       /* interface speed, see cat_adapcfg_t */
            } pca;
        } un;
};

/* device types */
#define	DD_TMSCSI 'T'	/* SCSI target mode */
#define DD_LP   'P'     /* line printer */
#define DD_TAPE 'M'     /* mag tape */
#define DD_SCTAPE  'm'  /* SCSI tape */
#define DD_TTY  'T'     /* terminal */
#define DD_DISK 'R'     /* disk */
#define DD_CDROM 'C'    /* cdrom */
#define DD_DLC 'D'      /* Data Link Control */
#define DD_SCDISK  'r'  /* SCSI disk */
#define DD_RTC  'c'     /* real-time (calendar) clock */
#define DD_PSEU 'Z'     /* pseudo-device */
#define DD_NET  'N'     /* networks */
#define DD_EN   'E'     /* Ethernet interface */
#define DD_EM78 'e'     /* 3278/79 emulator */
#define DD_TR   't'     /* token ring */
#define DD_BIO  'B'     /* block i/o device */
#define DD_X25  'x'     /* X.25 DDN device driver */
#define DD_IEEE_3  '3'  /* IEEE 802.3 */
#define DD_SL   'S'     /* Serial line IP  */
#define DD_LO   'L'     /* Loopback IP     */
#define DD_DUMP 'd'     /* dump device driver */
#define DD_SCCD    'C'  /* SCSI CDROM */
#define DD_CIO  'n'     /* common communications device driver */
#define DD_BUS  'b'     /* I/O Bus device */
#define DD_HFT  'H'     /* HFT */
#define DD_INPUT 'I'    /* graphic input device */
#define DD_CON  'Q'     /* console */
#define DD_NET_DH 'h'   /* Network device handler */
#define DD_DISK_C 's'   /* Disk Controller */
#define DD_SOL	'o'	/* Serial Optical Link */
#define DD_CAT  'K'     /* S/370 parallel channel */
#define DD_FDDI 'F'	/* FDDI */
#define DD_SCRWOPT 'w'   /* SCSI R/W optical */

/* device sub-types */
#define DS_DLCETHER 'e' /* DLC - Standard Ethernet */
#define DS_DLC8023  'i' /* DLC - IEEE 802.3 Ethernet */
#define DS_DLCTOKEN 't' /* DLC - Token Ring */
#define DS_DLCSDLC  'd' /* DLC - SDLC */
#define DS_DLCQLLC  'q' /* DLC - X.25 Qualified LLC */
#define DS_DLCFDDI  'f' /* DLC - FDDI */
#define DS_LV   'l'     /* logical volume */
#define DS_PV   'p'     /* physical volume - hard disk */
#define DS_SCSI 'S'     /* SCSI adapter */
#define DS_IDE  'I'     /* IDE adapter  */
#define DS_PP   'p'     /* Parallel printer */
#define DS_SP   's'     /* Serial printer   */
#define	DS_TM	'T'	/* SCSI target mode */
#define DS_SDA   'h'    /* Serial DASD adapter */
#define DS_SDC   'c'     /* Serial DASD Controller */
#define DS_NFS	'N'	/* NFS device for swapping */
#define DS_CAT  'k'     /* S/370 parallel channel */


/* Type of tape drive */
#define DT_STREAM       1       /* Streaming tape drive */
#define DT_STRTSTP      2       /* Start-stop tape drive */

/* flags */
#define DF_FIXED 01     /* non-removable */
#define DF_RAND  02     /* random access possible */
#define DF_FAST  04
#define DF_CONC  0x08   /* Concurrent mode supported */

#endif /* _H_DEVINFO */
