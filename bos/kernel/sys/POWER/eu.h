/* @(#)79	1.6  src/bos/kernel/sys/POWER/eu.h, sysxeu, bos411, 9428A410j 6/15/90 17:47:08 */
#ifndef _H_EU
#define _H_EU

/*
 * COMPONENT_NAME: (SYSXEU) device driver for async expansion unit
 *
 * FUNCTIONS: header file describing interface to async expansion unit
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#define EUMAXSLOT	8	/* valid slots range from 0 to 7 */

enum eu_ioctl {
    EU_GETPOS = 'e'<<14,	/* arg is pointer to euposcb struct */
    EU_SETPOS,			/* arg is pointer to euposcb struct */
    EU_GETID,			/* arg is pointer to euidcb struct */
    EU_ADD8,			/* arg is adapter number (0 - 7) */
    EU_DEL8,			/* arg is adapter number (0 - 7) */
    EU_ADD64,			/* arg is pointer to eu64cb struct */
    EU_DEL64,			/* arg is pointer to eu64cb struct */
    EU_SLOTVPD,			/* arg is pointer to euvpdcb struct */
    EU_ASSIST,			/* arg is pointer to function pointer */
    EU_DIAGS,			/* arg is pointer to eudiagcb struct */
 };

/*
 * structure of ioctl arg for EU_GETPOS  and EU_SETPOS
 */
struct euposcb {
	uchar slot;
	uchar reg;
	ushort value;
};

/*
 * structure of ioctl arg for EU_GETID
 */
struct euidcb {
	uchar slot;
	ushort id;		/* value from pos 0 and pos 1 */
};

/*
 * structure of ioctl arg for EU_ADD64
 *
 * partition specifies which 256K chunk of I/O channel memory 
 * to accept addresses for
 * 
 *	0 = accepts     0K -   256K (0x0000000 - 0x003ffff)
 *	1 = accepts   256K -   512K (0x0040000 - 0x007ffff)
 *	: : 		:	 :	    :	       :
 *     63 = accepts 16128K - 16384K (0x0fc0000 - 0x1000000)
 */
struct eu64cb {
	uchar slot;
	ushort partition;
};

/*
 * structure of ioctl arg for EU_SLOTVPD
 *
 * to be called from another kernel routine.  typically, another device
 * driver will use this to satisfy an application's CFG_QVPD request
 * for a card in the expansion unit.
 */
struct euvpdcb {
	uchar slot;
	struct uio *uio;
};

/*
 * prototype for function returned by EU_ASSIST
 * and masks for error paramter.
 * 
 * euassist is not exported; do not attempt to call euassist by name.
 * always use the pointer to the function returned by EU_ASSIST
 * 
 * int (*euassist)(int *error);
 */

#define EUACHECK	0x0001	/* data parity error detected by eu adapter */
#define EUAFEED		0x0002	/* card select feedback timeout */
#define EUPCHECK	0x0004	/* data parity error detected by eu planar */


/*
 * diagnostics philosophy
 *
 * diagnostic commands which could adversely effect expansion unit 
 * operations (commands ending in 'X') are allowed only when no adapters 
 * are configured in the expansion unit.  even then, the write-then-read
 * commands are temporary -- original register contents are saved/restored
 * around the write-then-read command.
 */

/*
 * list of diagnostic commands
 */
enum eucmd {
	EU_DIAGCREAD,		/* read comparator reg */
	EU_DIAGCWRITEX,		/* write-then-read comparator reg */
	EU_DIAGPREAD,		/* read POS reg	*/
	EU_DIAGPWRITEX,		/* write-then-read POS reg */
	EU_DIAGSREAD,		/* read steering reg */
	EU_DIAGSWRITEX,		/* write-then-read steering reg */
	EU_DIAGWRAP,		/* wrap data at wrap reg */
	EU_DIAGADD8,		/* euarg is adapter number (0 - 7) */
	EU_DIAGDEL8,		/* euarg is adapter number (0 - 7) */
	EU_DIAGADD64,		/* euarg is pointer to eu64cb struct */
	EU_DIAGDEL64,		/* euarg is pointer to eu64cb struct */
        EU_DIAGIO,        	/* to perform diag io to 8/16/64-Port adapt */
};

typedef int eucmd_t;

/*
 * structure of ioctl arg for EU_DIAGS
 */
struct eudiagcb {
	eucmd_t eucmd;		/* diagnostic command */
	void *euarg;		/* pointer to command structure */
};

/*
 * command structure pointed to by euarg for
 * EU_DIAGCREAD and EU_DIAGCWRITEX commands
 */
struct eudiagcomp {
	uchar slot;		/* slot (0 - 7) corresponds to comparator reg */
	ushort value;
};
 
/*
 * command structure pointed to by euarg for
 * EU_DIAGPREAD and EU_DIAGPWRITEX commands
 */
struct eudiagpos {
	uchar which;		/* 0 = adapter, otherwise planar */
	uchar reg;
	ushort value;
};

/*
 * command structure pointed to by euarg for
 * EU_DIAGSREAD and EU_DIAGSWRITEX commands
 */
struct eudiagsteer {
	uchar which;		/* 0 = adapter, otherwise planar */
	ushort value;
};

/*
 * command structure pointed to by euarg for
 * EU_DIAGWRAP command
 */
struct eudiagwrap {
	uchar which;		/* 0 = adapter, otherwise planar */
	uchar width;		/* 0 = 8-bit data, otherwise 16-bit data */
	int count;		/* number of width-size units to wrap */
	void *buffer;
};
 
/*
 * command structure pointed to by euarg for
 * EU_DIAGIO command
 *
 * philosophy:
 * 
 * - register for an adapter (EU_DIAGADD8 / EU_DIAGADD64)
 * - setup eudiagio struct for desired type of cycle to the address
 *   space belonging to the adapter
 * - expansion unit logic should respond for adapter, but adapter should
 *   not respond -- expansion unit should then pull channel check
 * - io exception handler will return type of error in "error" field
 * - unregister the adapter (EU_DIAGDEL8 / EU_DIAGDEL64)
 */
struct eudiagio {
	uchar iomem;		/* 0 = IO cycle, otherwise memory cycle */
	uchar rw;		/* 0 = read, otherwise write */
	uchar width;		/* 0 = 8-bit data, otherwise 16-bit data */
	ushort value; 
	uchar arbcomp;		/* for IO cycles, corresponds to adapter number.
  				 * for mem cycles, corresponds to comparator
				 * register
				 */
	ulong seg;		/* seg reg for access (usually 0x820c0060) */
	ulong offset;
	int error;		/* same values as returned by euassist */
};

typedef struct eudiagcb *eudp_t;
typedef struct eudiagcomp *eudcp_t;
typedef struct eudiagpos *eudpp_t;
typedef struct eudiagsteer *eudsp_t;
typedef struct eudiagwrap *eudwp_t;
typedef struct eudiagio *eudiop_t;


/*
 * the following information is used for initial configuration of the
 * async expansion unit.
 */
#define EU_AID		0x10e0		/* adapter should have this card id */
#define EU_PID		0xd4ed		/* planar should have this card id */
#define EU_BASE		0x210		/* base address of the adapter */

struct eu_dds {
    char eu_name[16];			/* resource name */
#ifndef REAL_EU_CONFIG
    mid_t eu_kmid;			/* kmid id until real config works */
#endif
    ushort eu_id;			/* card id */
    uchar eu_slot;			/* slot number */
    ulong eu_nseg;			/* seg reg for normal access */
    ulong eu_iseg;			/* seg reg for id access */
    uchar eu_delay;			/* extended cycle delay */
};

#define EU_IOCC(slot) (0x00400000+((slot)<<16))

#endif /* _H_EU */
