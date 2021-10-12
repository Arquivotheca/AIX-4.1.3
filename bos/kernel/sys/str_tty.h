/* @(#)06 1.14 src/bos/kernel/sys/str_tty.h, sysxcommon, bos41J, 9521B_all 5/26/95 07:49:26 */
/*
 * COMPONENT_NAME: (sysxtty) Streams framework
 *
 * FUNCTIONS:
 *
 * ORIGINS: 83
 *
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

#ifndef _H_STR_TTY
#define	_H_STR_TTY

#include <sys/termio.h>
#include <sys/stream.h>
#include <sys/cfgodm.h>
#ifdef	_KERNEL
#include <sys/errno.h>
#include <sys/ioctl.h>		/* for TTNAMEMAX definition	*/
#include <termios.h>
#include <sys/trchkid.h>
#include <sys/atomic_op.h>

/*
 * Preprocessor option for TTY support in crash and lldb
 */
#define	TTYDBG	1

/*
 * Macro definition for atomic opertion on sysinfo fields.
 */
#define	sysinfo_add(x,y)	fetch_and_add(&(x), (y))

/* parity table */
extern const char partab[256];

struct speedtab {
        int sp_speed;
        int sp_code;
};

static struct speedtab compatspeeds[] = {
        38400,  15,
        19200,  14,
        9600,   13,
        4800,   12,
        2400,   11,
        1800,   10,
        1200,   9,
        600,    8,
        300,    7,
	200,	6,
        150,    5,
        134,    4,
        110,    3,
        75,     2,
        50,     1,
        0,      0,
        -1,     -1,
};

static int compatspcodes[16] = {
        0, 50, 75, 110, 134, 150, 200, 300, 600, 1200,
        1800, 2400, 4800, 9600, 19200, 38400,
};

#undef cfsetispeed
#define	cfsetispeed(termios_ptr, speed)					\
{									\
	if ((speed) < B0 || (speed) > B38400);				\
	else {								\
		(termios_ptr)->c_cflag &= ~_CIBAUD;			\
		(termios_ptr)->c_cflag |= ((speed) & _CBAUD) << _IBSHIFT; \
	}								\
};

#undef	cfsetospeed
#define	cfsetospeed(termios_ptr, speed)					\
{									\
	if ((speed) < B0 || (speed) > B38400);				\
	else {								\
		(termios_ptr)->c_cflag &= ~_CBAUD;			\
		(termios_ptr)->c_cflag |= ((speed) & _CBAUD);		\
	}								\
};

#undef	cfgetospeed
#define	cfgetospeed(t_ptr)	((t_ptr)->c_cflag&_CBAUD)
#undef	cfgetispeed
#define	cfgetispeed(t_ptr)	(((t_ptr)->c_cflag&_CIBAUD)>>_IBSHIFT)

#define TIOC_REQUEST	_IO('J', 0x91)	/* request for ioctl's */
#define TIOC_REPLY	_IO('J', 0x92)	/* reply for ioctl's */

/*
 * STREAM tioc module data structures for TIOC_REPLY
 */
struct tioc_reply {
	int tioc_cmd;		/* command */
	int tioc_size;		/* number of bytes to copy */
	int tioc_type;		/* type of ioctl */
};

/*
 * STREAM tioc module tioc_type
 */
#define TTYPE_NOCOPY	0	/* don't need any copies */
#define TTYPE_COPYIN	1	/* need a M_COPYIN */
#define TTYPE_COPYOUT	2	/* need a M_COPYOUT */
#define TTYPE_COPYINOUT	3	/* need both M_COPYIN and M_COPYOUT */
#define TTYPE_IMMEDIATE	4	/* use immediate value */

/*
 * STREAMS tty M_CTL commands
 */

#define TIOCGETMODEM	_IO('J', 0xa0)	/* get the modem state from driver */
#define	MC_CANONQUERY	_IO('J', 0xa1)	/* query the termios state */
#define MC_NO_CANON	_IO('J', 0xa2)	/* set pty's remote mode */
#define MC_DO_CANON	_IO('J', 0xa3)	/* reset pty's remote mode */
#define	MC_PART_CANON	_IO('J', 0xa4)	/* oflag, iflag and lflag of termios */
					/* are handled by driver or ldterm   */

/*
 * STREAMS Based sptr module M_PCPROTO commands
 */

#define LPWR     	('l'<<8)
#define	LPWRITE_ACK	(LPWR|31)	/* command in M_PCPROTO message */

typedef	int	OSR_STATUS;


/*
 * General status definitions for drivers and line discipline in case of
 * parity and framing error or break_interrupt from the adapter, or in case
 * of modem status changes.
 */
enum status {
    good_char, overrun, parity_error, framing_error, break_interrupt,
    cts_on, cts_off, dsr_on, dsr_off, ri_on, ri_off, cd_on, cd_off,
    cblock_buf, other_buf};

/*
 * tty trace support, the main tty hooks identificators to be used are
 * defined in the common header file "sys/trchkid.h". The subhooks used for
 * the tty are defined hereafter.
 * These hooks and subhooks are used in the macros Return and Enter, which
 * are defined in that file.
 */

/* TTY subhooks identificators */
#define	TTY_CONFIG	0x01
#define	TTY_OPEN	0x02
#define	TTY_CLOSE	0x03
#define	TTY_WPUT	0x04
#define	TTY_RPUT	0x05
#define	TTY_WSRV	0x06
#define	TTY_RSRV	0x07
#define	TTY_REVOKE	0x08	/* for streamhead tty			*/
#define	TTY_IOCTL	0x09	/* for ioctls				*/
#define	TTY_PROC	0x0a	/* for drivers				*/
#define	TTY_SERVICE	0x0b	/* for drivers				*/
#define	TTY_SLIH	0x0c	/* for drivers				*/
#define	TTY_OFFL	0x0d	/* for drivers				*/
#define	TTY_LAST	0x0e	/* can be used for any specific entry	*/

/* 
 * The parameters for the next macros are:
 *	w = TTY hookid | TTY subhookid
 *	dev = dev(type dev_t)
 *	ptr = address of the private data (q->q_ptr) of each module or driver.
 *	a, b, c : specific parameters for each subhook, defined below.
 *	retval : return value.
 *
 * Arguments for each subhook are :
 *	TTY_CONFIG:		a=command, no dev no ptr.
 *	TTY_OPEN:		a=oflag, b=sflag.
 *	TTY_CLOSE:		a=flag.
 *	TTY_WPUT:		a=@msg, b=message type.
 *	TTY_RPUT:		as TTY_WPUT.
 *	TTY_WSRV:		a=q_count.
 *	TTY_RSRV:		as TTY_WSRV.
 *	TTY_REVOKE:		a=flag.
 *	TTY_IOCTL:		a=ioctl command.
 *	TTY_PROC:		a=cmd, b=arg.
 *	TTY_SERVICE:		a=service command, b=arg.
 *	TTY_SLIH:		no dev, ptr=@struct intr, a=adapter type.
 *	TTY_OFFL:		ptr=@struct intr.
 */

#define	Enter(w, dev, ptr, a, b, c) 					\
	dev_t	DEV;							\
	int	PTR;							\
	int	Flag = 0;						\
	int	W;							\
	if (TRC_ISON(0)) {						\
		DEV = (dev);						\
		PTR = (ptr);						\
		Flag = 1;						\
		W = w;							\
		TRCHKGT(W, DEV, PTR, a, b, c);				\
	}

#define	Return(retval) {						\
	int	RET = (retval);						\
	int	Line = __LINE__;					\
	if (Flag)							\
		TRCHKGT(((W)|0x80), DEV, PTR, RET, Line, 0);		\
	return(RET);							\
}

#define Returnv {                                               \
        int     Line = __LINE__;                                        \
        if (Flag)                                                       \
                TRCHKGT(((W)|0x80), DEV, PTR, 0, Line, 0);              \
	return;							\
}

#define Data(a, b, c)                                              \
    (Flag ? TRCHKGT(((W)|0x40), DEV, PTR, a, b, c), 0 : 0)

			
#endif	/* _KERNEL  */

/* tty ioctls commands  */
#define TIOCGETA        TCGETS
#define TIOCSETA        TCSETS
#define TIOCSETAW       TCSETSW
#define TIOCSETAF       TCSETSF

/*
 * General tty stream modules and drivers names as they should be recognized
 * at configuration time.
 */
/* Maximum device name lenght */
#define DEV_NAME_LN         16

/* These types are used at configuration time */
enum dds_type {             /* Which DDS type */
	LC_SJIS_DDS,            /* sjis lower converter module */
	LDTERM_DDS,             /* ldterm module */
	LION_ADAP_DDS,          /* lion driver (for adapters) */
	LION_LINE_DDS,          /* lion driver (for lines) */
	NLS_DDS,                /* nls module */
	PTY_DDS,                /* pty module */
	RS_ADAP_DDS,            /* rs driver (for adapters) */
	RS_LINE_DDS,            /* rs driver (for lines) */
	SPTR_DDS,               /* sptr module */
	TIOC_DDS,               /* tioc module */
	UC_SJIS_DDS,            /* sjis upper converter module */
        CXMA_ADAP_DDS,          /* cxma driver (for adapters) */
        CXMA_LINE_DDS,           /* cxma driver (for lines) */
        CXIA_ADAP_DDS,          /* cxia ISA driver (for adapters) */
        CXIA_LINE_DDS           /* cxia ISA driver (for lines) */
  };

/* Streams tty modules and drivers names	*/
enum module_names {
	tioc,			/* transparent ioctl module name	*/
	ldterm,			/* line discipline module name		*/
	pty,			/* pseudo tty driver module name	*/
	uc_sjis,		/* upper converter sjis module name	*/
	lc_sjis,		/* lower converter sjis module name	*/
	nls,			/* mapping discipline module name	*/
	sptr,			/* serial lie printer discipline name	*/
	rs,			/* rs driver name			*/
	lion,			/* lion driver name			*/
        cxma,                   /* 128 port driver name                 */
 };

/*
 * The next redefinitions are necessary for a non fixed bug.
 */
#define	bcopy(a,b,c)	bcopy(a,b,c)
#define	bzero(a,b)	bzero(a,b)

/*
 * Start streams tty debug definitions (ttydbg)
 * Especially for streamhead tty, modules, and drivers print functions.
 * For crash command.
 */
#ifndef	FMNAMESZ
#define FMNAMESZ	8		/* as in sys/stream.h */
#endif

/* bits passed in the arg parameter to the module/driver/... print function */
/* when tty_print recognize the corresponding option (-v, -o, ...)	    */
/* At the moment only TTYDBG_ARG_V is useful for the print functions	    */
#define TTYDBG_ARG_V	0x0001
#define TTYDBG_ARG_O	0x0002

/* Module/driver informations.						*/
/* To be used by streams based tty modules and drivers.			*/
struct	str_module_conf {
 	char	name[FMNAMESZ];		/* name of the module or driver	*/
 	char	type;			/* type of the module :		*/
					/* 'l' for aline discipline,	*/
					/* 'd' for a driver,		*/
					/* 'm' for other module,	*/
					/* 's' for a stream head.	*/
 	int	(*print_funcptr)();	/* address of the module/driver	*/
 					/* print function.		*/
  };
 
/* TTY's lines informations.						*/
/* To be used by streams based tty modules and drivers.			*/
struct	tty_to_reg {
 	dev_t	dev;			/* device major/minor number	*/
 	char	ttyname[TTNAMEMAX];	/* device name.			*/
 	char	name[FMNAMESZ];		/* module/driver name.		*/
 	void	*private_data;		/* pointer to the structure to	*/
 					/* print.			*/
 };
 
#define	STR_TTY_CNT	26		/* number of tty lines registered */
 					/* in a all_tty_s structure.	*/
#define	MODULEMAX	6		/* maximum number of modules	*/
 					/* pushed per line plus streamhead */
 
/* Pushed module structure						*/
/* To be used by ttydbg kernel extension, and crash command only.	*/
struct	pushed_modules {
 	struct	str_module_conf	*mod_ptr; /* pointer to the corresponding */
 					  /* per module structure.	  */
 	void	*private_data;		  /* private data structure to	  */
 					  /* print.			  */
 };
 
/* Per tty structure.							*/
/* To be used by ttydbg kernel extension, and crash command only.	*/
struct	per_tty {
 	dev_t	dev;			/* device major/minor.		*/
 	char	ttyname[TTNAMEMAX];	/* device name.			*/
 	struct	pushed_modules	mod_info[MODULEMAX];
 };

/* General tty table definition.					*/
/* To be used by ttydbg kernel extension and crash command only.	*/
struct	all_tty_s {
 	struct	per_tty	lists[STR_TTY_CNT];
 	struct	all_tty_s *next;
 };
 
/* Pointer to the general tty table.					*/
/* To be used by ttydbg kernel extension and crash command only.	*/
extern struct all_tty_s	*tty_list;

/* These types are used at tty_db_register() time */
enum pdpf_type {		/* Which pre-defined print function */
        CXMA_PDPF,			/* cxma driver */
	LDTERM_PDPF,			/* ldterm module */
	LION_PDPF,			/* lion driver */
	NLS_PDPF,			/* nls module */
	PTY_PDPF,			/* pty module */
	RS_PDPF,			/* rs driver */
	SPTR_PDPF,			/* sptr module */
	STR_TTY_PDPF,			/* stream head */
	FU1_PDPF,			/* futur use 1 */
	FU2_PDPF,			/* futur use 2 */
					/* last enum : don't use this value */
	LAST_PDPF	/* and always let it at the end */
  };

/*
 * Give use of function pointers declared and exported by the debugger lldb
 * (src/bos/kernel/db, dbkern.c and POWER/dbtty.c)
 * For the pse (streams frame work) loading purpose, only the symbol names of
 * registration functions are usefull (in str_tty.c). Then the debugger lldb
 * declares and exports the corresponding function pointers. These pointers
 * will be initialized with the right function addresses by the ttydbg
 * extension when its tty_db_config routine will be called with cmd==CFG_INIT.
 * Thus, the registration functions are accessed through pointers and the
 * macros defined hereafter provide that facility.
 */
extern int (* tty_db_register_ptr)();
extern int (* tty_db_unregister_ptr)();
extern int (* tty_db_open_ptr)();
extern int (* tty_db_close_ptr)();
/*
 * For now the following macros and the functions have the same name.
 * In a further release these macros should be defined (and used) with
 * names in all uppercase.
 */

#define	tty_db_register(p)	\
	(tty_db_register_ptr != 0 ? tty_db_register_ptr(p) : -1)

#define	tty_db_unregister(p)	\
	(tty_db_unregister_ptr != 0 ? tty_db_unregister_ptr(p) : -1)

#define	tty_db_open(p)	\
	(tty_db_open_ptr != 0 ? tty_db_open_ptr(p) : -1)

#define	tty_db_close(p)	\
	(tty_db_close_ptr != 0 ? tty_db_close_ptr(p) : -1)
/*
 * Outputs pagination
 */
#define	TTYDBG_NOMORE	-2

#if defined(_KERNEL) && defined(IN_TTYDBG)
#define	tty_db_nomore(X)	do {		\
	int X_cr;				\
	if ((X_cr = (X)) == TTYDBG_NOMORE) {	\
		return(X_cr);			\
	}					\
} while (0)
#else
#define	tty_db_nomore(X)	(X)
#endif	/* _KERNEL && IN_TTYDBG */


/* this structure is returned by get_attr_list in the tty config method */
struct  attr_list {
        int     attr_cnt;       /* number of attributes in list */
        struct  CuAt cuat[1];   /* array of CuAt like attr objects */
};

#endif	/* _H_STR_TTY */
