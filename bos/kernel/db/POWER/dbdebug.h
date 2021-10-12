/* @(#)52	1.31	src/bos/kernel/db/POWER/dbdebug.h, sysdb, bos412, 9446C 11/17/94 14:43:00 */

#ifndef _h_DBDEBUG
#define _h_DBDEBUG

/*
 * COMPONENT_NAME: (SYSDB) Kernel Debugger
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
*/

#include <sys/lldebug.h>

/*
 * #defines for reason codes
 */

#define E_as_process 1  /* 02.04 allow page faulting for init. */
#define E_trap    2
#define E_panic  3             /* Ext. interrupt/SVC            */
#define E_sysabend  4
#define E_vmabend   5
#define E_touch   9                 /* 03.05 */
#define E_step   10
#define E_break 11
#define E_static_break 12
#define E_nostep 13
#define E_mchcheck 14         /* Machine check 02.02           */
#define E_pgmcheck 15         /* Program check 02.02           */
#define W_step 18		/* Watchpoint step               */
#define W_break 19		/* Watchpoint break              */
#define B_break 20		/* Bratpoint break              */
#define B_step 21		/* Bratpoint step               */
#ifdef _POWER_MP
#define E_break_not_for_me 16
#define E_selected_cpu 17
#define cpunb (db_get_processor_num())

#define NO_SELECTED_CPU -1

typedef enum { running = 1, stopped, debug_waiting, debugging} status_t ;

typedef enum {NONE = 1, stop, debug, resume } action_t ;

#else /* #ifdef _POWER_MP */

#define cpunb 0
#ifndef MAXCPU
#define MAXCPU 1
#endif /* #ifndef MAXCPU */

#endif /* #ifdef _POWER_MP */

/* Used to determin what kind of ext. int. (e_panic). */
#define keyseq 1
#define debsvc 2

/* Screen display types. */
#define DEFLTSCR 1
#define SREGSCR  2
#define FPREGSCR 3
#define MEMSCR	 4

/* Null module id. */
#define not_a_module	0xffffffff

/* Special display-in-use flags. */
#define USE_TTY  	0x80000000	/* async */
#define USE_SYM		0x00000800
#define TTY_PORT	0x000000ff	/* bit mask to determine tty number */
#define TTY_DEFAULT	0x00000000	/* default is port a */

/* for debabend */
#define IN	1
#define OUT	2

#define LAST_UVAR	16
#define FIRST_UVAR	0
#define LAST_DVAR	LAST_UVAR + 2	/* User variables + FX + ORG */
#define bl32 		"                              "
#define bl8 		"       "
#define DITTO_ID 2                     /* index of DITTO command */
#define INVALID_ID -1                  /* invalid id */
#define BRT 0x84        /* "b" or "brat" is valid               */
#define LSV 0x82        /* "l" or "s" is valid for watchpt      */
#define WTC 0x81	/* "w" or "watch" is valid		*/
#define HXV 0x80	/* hex value is valid			*/
#define DCV 0x40	/* decimal value is valid		*/
#define ADV 0x20	/* address value is valid		*/
#define ASV 0x10	/* "*" is valid				*/
#define BTV 0x08	/* "b" or "t" is valid			*/
#define RMV 0x04	/* address must be in real memory	*/
#define STV 0x02	/* quoted string is valid		*/
#define YNV 0x01	/* "y" or "n" is valid			*/
#define PLUS  "+"
#define MINUS "-"
#define TRUE   1
#define FALSE  0


/*
 *	typedef for the instruction size
 */
typedef	ulong	INSTSIZ;


struct QE {
	ulong	rsv1;
	ulong	pathid;
	char	rsv2;
	char	rsv3;
	struct	options {
		char	rsv4;
		char 	rsv5;
	} options;
	int	key;
	ulong	rsv6[4];
};

struct func {			/* struct to define information about a cmd */
	char *label;		/* command name */
	int  field[4];		/* command parameter definitions */
	char psflag;		/* parser structure passed ? */
	int  (*action)();	/* function name */
	char *text;		/* command description */
	} ;

struct mcs_pcs {
	uchar mcs1;
	uchar pcs1;
	} ;

struct rsn_code {
	ushort entry_reason;
	ushort abend_code;			/* might be key or svc */
	};

struct abend_status {
	ushort	rsv;
	char	abendmcs;
	char	abendpcs;
	} ;

struct descr  {                 /* descrp info from Debug_Opcode routine */
    caddr_t D_EA;               /* effective addr or -1 */
    caddr_t D_NSI;              /* "Fall Thru addr or -1 */
    caddr_t D_Target;           /* Branch target or -1 */
    char    D_Mnemonic[50];      /* Name */
    char    D_Brbit;            /* Br/Jump condition bits */
    char    D_Len;              /* Instruction length */
    } ;

struct db_vmsidata
{
        uint    rmapptr;        /* pointer to ram_bit_map        */
        int     rmapsize;       /* size in words of the bit map  */
        int     rmapblk;        /* bytes represented per bit     */
};

#define PSIZE           (1<<12)         /* page size in bytes */
#define SOFFSET         0x0fffffff      /* mask for offset in a segment  */



#define SEGREG       0x810000


#define IOSEGCK(x,y) (y = (debvars[IDSEGS+((x)<<28)].hv & 0x80000000))

#define PRINTBUFSIZE	1024

#ifdef _POWER_MP
/* Lock structure for db_lock_try and db_unlock */
struct db_lock {
	long lockword;
	long lockcount;
	int owning_cpu;
};

extern int db_lock_try(struct db_lock *lockstruct, int cpu_num);
extern int db_unlock(struct db_lock *lockstruct);
extern int db_get_processor_num();

#endif /* _POWER_MP */

#endif /* #ifndef _h_DBDEBUG */
