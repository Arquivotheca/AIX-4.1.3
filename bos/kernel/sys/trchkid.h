/* @(#)08	1.96.2.1  src/bos/kernel/sys/trchkid.h, systrace, bos411, 9432A411a 8/5/94 19:09:40 */
/*
 * COMPONENT_NAME:            include/sys/trchkid.h
 *
 * FUNCTIONS: header file for system trace hookwords
 *
 * ORIGINS: 27, 83
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * LEVEL 1, 5 Years Bull Confidential Information
 */

/* All major hook id's should begin with HKWD_, and all minor hook id's
   should begin with hkwd_.
*/

#ifndef _H_TRCHKID
#define _H_TRCHKID

#include "sys/trcmacros.h"

#define HKWD_TRACE                0x00000000
#define HKWD_TRACE_SYNC           0x00000000
#define HKWD_TRACE_TRCON          0x00100000
#define HKWD_TRACE_TRCOFF         0x00200000
#define HKWD_TRACE_HEADER         0x00300000
#define HKWD_TRACE_NULL           0x00400000
#define HKWD_TRACE_LWRAP          0x00500000
#define HKWD_TRACE_TWRAP          0x00600000
#define HKWD_TRACE_UNDEFINED      0x00700000
#define HKWD_TRACE_DEFAULT        0x00800000
#define HKWD_TRACE_CALIB          0x00900000
#define HKWD_TRACE_UTIL           0x00A00000

#define HKWD_USER1                0x01000000
#define HKWD_USER2                0x02000000
#define HKWD_USER3                0x03000000
#define HKWD_USER4                0x04000000
#define HKWD_USER5                0x05000000
#define HKWD_USER6                0x06000000
#define HKWD_USER7                0x07000000
#define HKWD_USER8                0x08000000
#define HKWD_USER9                0x09000000
#define HKWD_USERA                0x0a000000
#define HKWD_USERB                0x0b000000
#define HKWD_USERC                0x0c000000
#define HKWD_USERD                0x0d000000
#define HKWD_USERE                0x0e000000
#define HKWD_USERAIX              0x0f000000

/*
 * These hooks were assigned at one time but are no longer used.
 */

#define HKWD_KERN                 0x10000000
#define HKWD_KERN_FLIH            0x10000000
#define HKWD_KERN_SVC             0x10100000    /* common SVC entry */
#define HKWD_KERN_SLIH            0x10200000
#define HKWD_KERN_SLIHRET         0x10300000
#define HKWD_KERN_SYSCRET         0x10400000    /* common SVC return */
#define HKWD_KERN_LVM             0x10500000    /* LVM bad blk/resync flow */
#define HKWD_KERN_DISPATCH        0x10600000
#define HKWD_LFS_LOOKUP           0x10700000    /* lookuppn */
#define HKWD_SYSC_LFS             0x10800000    /* file system system call */
#define HKWD_KERN_LFS             0x10900000
#define HKWD_KERN_PFS             0x10A00000
#define HKWD_KERN_LVMSIMP         0x10B00000    /* LVM simple request flow */
#define HKWD_KERN_IDLE            0x10C00000    /* Dispatching idle process */
#define HKWD_KERN_VFSINO          0x10d00000
#define HKWD_KERN_EOF             0x10f00000
#define HKWD_KERN_STDERR          0x11000000
#define HKWD_KERN_LOCKF           0x11100000
#define HKWD_KERN_LOCK            0x11200000	
#define HKWD_KERN_UNLOCK          0x11300000	
#define HKWD_KERN_LOCKALLOC       0x11400000	
#define HKWD_KERN_SETRECURSIVE    0x11500000	

#define HKWD_KERN_XMALLOC         0x11600000
#define HKWD_KERN_XMFREE          0x11700000
#define HKWD_KERN_FORKCOPY        0x11800000
#define HKWD_KERN_PIDSIG          0x11900000
#define HKWD_KERN_RCVSIGNAL       0x11a00000
#define HKWD_KERN_P_SLIH          0x11c00000
#define HKWD_KERN_SIG_SLIH        0x11d00000
#define HKWD_KERN_ISSIG           0x11e00000
#define HKWD_KERN_SORQ            0x11f00000
#define HKWD_KERN_RESUME          0x20000000
#define HKWD_KERN_HFT             0x20100000
#define HKWD_KERN_KTSM            0x20200000
#define HKWD_KERN_CON             0x20300000
#define HKWD_KERN_SCHED_SWAPIN	  0X20400000
#define HKWD_KERN_SCHED_SWAPOUT   0X20500000
#define HKWD_KERN_SCHED_POST	  0x20600000
#define HKWD_KERN_SCHED		  0x20700000
#define HKWD_KERN_SCHED_STAT	  0x20800000
#define HKWD_KERN_SCHED_STAT1	  0x20900000
#define HKWD_KERN_SCRUB_DISABLE	  0x20A00000
#define HKWD_KERN_SCRUB_ENABLE	  0x20B00000
#define HKWD_KERN_SCRUB_SEG	  0x20C00000
#define HKWD_KERN_SCRUB_SOFT	  0x20D00000
#define HKWD_KERN_LOCKL           0x20E00000
#define HKWD_KERN_UNLOCKL         0x20F00000

/*
 * "sub-hookids"
 */
#define hkwd_SYSC_FULLSTAT  1
#define hkwd_SYSC_LOCKFX    2
#define hkwd_SYSC_STAT      3
#define hkwd_SYSC_MKNOD     4
#define hkwd_SYSC_ACCESS    5
#define hkwd_SYSC_CHOWN     6
#define hkwd_SYSC_CHOWNX    7
#define hkwd_SYSC_CLOSEX    8
#define hkwd_SYSC_OPENX     9
#define hkwd_SYSC_FCHMOD    10
#define hkwd_SYSC_FCHOWN    11
#define hkwd_SYSC_FCHOWNX   12
#define hkwd_SYSC_FCLEAR    14
#define hkwd_SYSC_FSYNC     15
#define hkwd_SYSC_FTRUNCATE 16
#define hkwd_SYSC_TRUNCATE  17
#define hkwd_SYSC_IOCTLX    18
#define hkwd_SYSC_READX     19
#define hkwd_SYSC_WRITEX    20

#define hkwd_PFS_RDWR       1
#define hkwd_PFS_READI      2
#define hkwd_PFS_WRITEI     3

#define LOCK_TAKEN          1
#define LOCK_MISS           2
#define LOCK_RECURSIVE      3
#define LOCK_BUSY      	    4

#define SETRECURSIVE        1
#define CLEARRECURSIVE      2

/*
 * LVM device driver sub hookids
 *
 * #defines with a "X" in the comment are traced with "HKWD_KERN_LVMSIMP"
 * major hook.  All others use "HKWD_KERN_LVM".  This allows for better
 * performance filtering.
 */
#define hkwd_LVM_PEND           1       /* End of physical request      X */
#define hkwd_LVM_PSTART         2       /* Start of physical request    X */
#define hkwd_LVM_RELOCINGBLK    3       /* Encountered relocated blk      */
#define hkwd_LVM_OLDBADBLK      4       /* Old bad block                  */
#define hkwd_LVM_BADBLKDONE     5       /* Blk relocation complete        */
#define hkwd_LVM_NEWBADBLK      6       /* New bad block found            */
#define hkwd_LVM_SWRELOC        7       /* Software relocating bad blk    */
#define hkwd_LVM_RESYNCPP       10      /* Resyncing LP mirrors           */
#define hkwd_LVM_LSTART         11      /* Start of logical request     X */
#define hkwd_LVM_RBLOCKED       12      /* request blocked - conflicts  X */
#define hkwd_LVM_LEND           13      /* End of logical request       X */
#define hkwd_LVM_OPEN           14      /* LV open                        */
#define hkwd_LVM_CLOSE          15      /* LV close                       */
#define hkwd_LVM_READ           16      /* LV character device read       */
#define hkwd_LVM_WRITE          17      /* LV character device write      */
#define hkwd_LVM_IOCTL          18      /* LVM device driver ioctl        */
#define hkwd_LVM_UPD_BBDIR      19      /* Update BB directory. pb queued */
#define hkwd_LVM_BBDIREND       20      /* Iodone for BB directory I/O    */
#define hkwd_LVM_BBDIROP        21      /* Continue BB directory operation*/
#define hkwd_LVM_BBADD          22      /* Add BB directory entry on disk */
#define hkwd_LVM_BBDEL          23      /* Delete BB dir entry on disk    */
#define hkwd_LVM_BBUPD          24      /* Update BB dir entry on disk    */
#define hkwd_LVM_BBDIRDONE      25      /* Completion of BB dir updating  */
#define hkwd_LVM_SA_STRT        26      /* Put SA request on hold list    */
#define hkwd_LVM_SA_WRT         27      /* Build buf for next SA on wheel */
#define hkwd_LVM_SA_IODONE      28      /* iodone for VGSA write          */
#define hkwd_LVM_SA_CONT        29      /* continue writing VGSAs         */
#define hkwd_LVM_SA_RTN         30      /* return requests to callers     */
#define hkwd_LVM_SA_CONFIG      31      /* hd_config req for VGSA wheel   */
#define hkwd_LVM_CA_CHKCACH     32      /* check for a cache hit          */
#define hkwd_LVM_CA_CHKHIT      33      /* got a cache hit                */
#define hkwd_LVM_CA_WRT         34      /* start cache writes             */
#define hkwd_LVM_CA_WEND        35      /* cache write end                */
#define hkwd_LVM_MWCCWCOMP      36      /* MWCC write done. sched pending */
#define hkwd_LVM_CA_CLNUP       37      /* flush MWCC cache               */
#define hkwd_LVM_CA_TERM        38      /* logical cached request finished*/
#define hkwd_LVM_MVHLD          39      /* cache hold queue to pending    */
#define hkwd_LVM_SCHEDAVOID     40      /* scheduler called after avoiding*/

#define HKWD_SYSC                 0x12000000
#define HKWD_SYSC_ACCESS          0x12000000
#define HKWD_SYSC_ACCT            0x12100000
#define HKWD_SYSC_ALARM           0x12200000
#define HKWD_SYSC_AUDIT           0x12300000
#define HKWD_SYSC_AUDITLOG        0x12400000
#define HKWD_SYSC_AUDITPROC       0x12500000
#define HKWD_SYSC_AUDITSYS        0x12600000
#define HKWD_SYSC_CHDIR           0x12700000
#define HKWD_SYSC_CHMAC           0x12800000
#define HKWD_SYSC_CHMOD           0x12900000
#define HKWD_SYSC_CHOWN           0x12a00000
#define HKWD_SYSC_CHOWNX          0x12b00000
#define HKWD_SYSC_CHPRIV          0x12c00000
#define HKWD_SYSC_CHROOT          0x12d00000
#define HKWD_SYSC_CLOSE           0x12e00000
#define HKWD_SYSC_CLOSEX          0x12f00000
#define HKWD_SYSC_CREAT           0x13000000
#define HKWD_SYSC_DISCLAIM        0x13100000
#define HKWD_SYSC_DUP             0x13300000
#define HKWD_SYSC_EXECVE          0x13400000
#define HKWD_SYSC__EXIT           0x13500000
#define HKWD_SYSC_FCLEAR          0x13600000
#define HKWD_SYSC_FCNTL           0x13700000
#define HKWD_SYSC_FFULLSTAT       0x13800000
#define HKWD_SYSC_FORK            0x13900000
#define HKWD_SYSC_FSTAT           0x13a00000
#define HKWD_SYSC_FSTATFS         0x13b00000
#define HKWD_SYSC_FSYNC           0x13c00000
#define HKWD_SYSC_FTRUNCATE       0x13d00000
#define HKWD_SYSC_FULLSTAT        0x13e00000
#define HKWD_SYSC_GETEGID         0x13f00000
#define HKWD_SYSC_GETEPRIV        0x14000000
#define HKWD_SYSC_GETEUID         0x14100000
#define HKWD_SYSC_GETGID          0x14200000
#define HKWD_SYSC_GETGROUPS       0x14300000
#define HKWD_SYSC_GETMPRIV        0x14400000
#define HKWD_SYSC_GETPGRP         0x14500000
#define HKWD_SYSC_GETPID          0x14600000
#define HKWD_SYSC_GETPPID         0x14700000
#define HKWD_SYSC_GETPRIV         0x14800000
#define HKWD_SYSC_GETTSTATE       0x14900000
#define HKWD_SYSC_GETUID          0x14a00000
#define HKWD_SYSC_GTTY            0x14b00000
#define HKWD_SYSC_IOCTL           0x14c00000
#define HKWD_SYSC_IOCTLX          0x14d00000
#define HKWD_SYSC_KILL            0x14e00000
#define HKWD_SYSC_KPRINTF         0x14f00000
#define HKWD_SYSC_LINK            0x15000000
#define HKWD_SYSC_LOADTBL         0x15100000
#define HKWD_SYSC_LOCKF           0x15200000
#define HKWD_SYSC_LOCKX           0x15300000
#define HKWD_SYSC_LSEEK           0x15400000
#define HKWD_SYSC_MACCTL          0x15500000
#define HKWD_SYSC_MKDIR           0x15600000
#define HKWD_SYSC_MKNOD           0x15700000
#define HKWD_SYSC_MNTCTL          0x15800000
#define HKWD_SYSC_MOUNT           0x15900000
#define HKWD_SYSC_NICE            0x15a00000
#define HKWD_SYSC_OPEN            0x15b00000
#define HKWD_SYSC_OPENX           0x15c00000
#define HKWD_SYSC_OUNAME          0x15d00000
#define HKWD_SYSC_PAUSE           0x15e00000
#define HKWD_SYSC_PIPE            0x15f00000
#define HKWD_SYSC_PLOCK           0x16000000
#define HKWD_SYSC_PROFIL          0x16100000
#define HKWD_SYSC_PTRACE          0x16200000
#define HKWD_SYSC_READ            0x16300000
#define HKWD_SYSC_READLINK        0x16400000
#define HKWD_SYSC_READX           0x16500000
#define HKWD_SYSC_REBOOT          0x16600000
#define HKWD_SYSC_RENAME          0x16700000
#define HKWD_SYSC_RMDIR           0x16800000
#define HKWD_SYSC_SBREAK          0x16900000
#define HKWD_SYSC_SELECT          0x16a00000
#define HKWD_SYSC_SETGID          0x16b00000
#define HKWD_SYSC_SETGROUPS       0x16c00000
#define HKWD_SYSC_SETMAC          0x16d00000
#define HKWD_SYSC_SETPGRP         0x16e00000
#define HKWD_SYSC_SETPRIO         0x16f00000
#define HKWD_SYSC_SETPRIV         0x17000000
#define HKWD_SYSC_SETREUID        0x17100000
#define HKWD_SYSC_SETTSTATE       0x17200000
#define HKWD_SYSC_SETUID          0x17300000
#define HKWD_SYSC_SHMCTL          0x17400000
#define HKWD_SYSC_SHMGET          0x17500000
#define HKWD_SYSC_SHMAT           0x17600000
#define HKWD_SYSC_SHMDT           0x17700000
#define HKWD_SYSC_MSGCTL          0x17800000
#define HKWD_SYSC_MSGGET          0x17900000
#define HKWD_SYSC_MSGSND          0x17a00000
#define HKWD_SYSC_MSGRCV          0x17b00000
#define HKWD_SYSC_MSGXRCV         0x17c00000
#define HKWD_SYSC_SEMCTL          0x17d00000
#define HKWD_SYSC_SEMGET          0x17e00000
#define HKWD_SYSC_SEMOP           0x17f00000
#define HKWD_SYSC_SIGACTION       0x18000000
#define HKWD_SYSC_SIGCLEANUP      0x18100000
#define HKWD_SYSC_SIGPROCMASK     0x18200000
#define HKWD_SYSC_SIGRETURN       0x18300000
#define HKWD_SYSC_SIGSTACK        0x18400000
#define HKWD_SYSC_SIGSUSPEND      0x18500000
#define HKWD_SYSC_STAT            0x18600000
#define HKWD_SYSC_STATFS          0x18700000
#define HKWD_SYSC_STATX           0x18800000
#define HKWD_SYSC_STIME           0x18900000
#define HKWD_SYSC_STTY            0x18a00000
#define HKWD_SYSC_SYMLINK         0x18b00000
#define HKWD_SYSC_SYNC            0x18c00000
#define HKWD_SYSC_TIME            0x18d00000
#define HKWD_SYSC_TIMES           0x18e00000
#define HKWD_SYSC_ULIMIT          0x18f00000
#define HKWD_SYSC_UMASK           0x19000000
#define HKWD_SYSC_UMOUNT          0x19100000
#define HKWD_SYSC_UNAME           0x19200000
#define HKWD_SYSC_UNAMEX          0x19300000
#define HKWD_SYSC_UNLINK          0x19400000
#define HKWD_SYSC_USRINFO         0x19500000
#define HKWD_SYSC_USTAT           0x19600000
#define HKWD_SYSC_UTIME           0x19700000
#define HKWD_SYSC_UTSSYS          0x19800000
#define HKWD_SYSC_UVMOUNT         0x19900000
#define HKWD_SYSC_VMOUNT          0x19a00000
#define HKWD_SYSC_WAIT            0x19b00000
#define HKWD_SYSC_WRITE           0x19c00000
#define HKWD_SYSC_WRITEX          0x19d00000
#define HKWD_SYSC_FCHMOD          0x19e00000
#define HKWD_SYSC_FCHOWN          0x19f00000
#define HKWD_SYSC_GETDIRENTRIES   0x1a000000
#define HKWD_SYSC_LSTAT           0x1a100000
#define HKWD_SYSC_UTIMES          0x1a200000
#define HKWD_SYSC_SLOOKUP         0x1a300000
#define HKWD_SYSC_GETRLIMIT       0x1a400000
#define HKWD_SYSC_SETRLIMIT       0x1a500000
#define HKWD_SYSC_GETRUSAGE       0x1a600000
#define HKWD_SYSC_GETPRIORITY     0x1a700000
#define HKWD_SYSC_SETPRIORITY     0x1a800000
#define HKWD_SYSC_ABSINTERVAL     0x1a900000
#define HKWD_SYSC_GETINTERVAL     0x1aa00000
#define HKWD_SYSC_GETTIMER        0x1ab00000
#define HKWD_SYSC_INCINTERVAL     0x1ac00000
#define HKWD_SYSC_RESTIMER        0x1ad00000
#define HKWD_SYSC_RESABS          0x1ae00000
#define HKWD_SYSC_RESINC          0x1af00000
#define HKWD_SYSC_SETTIMER        0x1f000000
#define HKWD_SYSC_GETARGS         0x1f100000
#define HKWD_SYSC_GETPROC         0x1f200000
#define HKWD_SYSC_GETUSER         0x1f300000
#define HKWD_SYSC_KNLIST          0x1f400000
#define HKWD_SYSC_SETSID          0x1f500000
#define HKWD_SYSC_SETPGID         0x1f600000
#define HKWD_SYSC_SETREGID        0x1f700000
#define HKWD_SYSC_SETPRI          0x1f800000


#define HKWD_VMM                  0x1b000000
#define HKWD_VMM_ASSIGN           0x1b000000
#define HKWD_VMM_DELETE           0x1b100000
#define HKWD_VMM_PGEXCT           0x1b200000
#define HKWD_VMM_PROTEXCT         0x1b300000
#define HKWD_VMM_LOCKEXCT         0x1b400000
#define HKWD_VMM_RECLAIM          0x1b500000
#define HKWD_VMM_PINGPONG         0x1b500001
#define HKWD_VMM_GETPARENT        0x1b600000
#define HKWD_VMM_COPYPARENT       0x1b700000
#define HKWD_VMM_VMAP             0x1b800000
#define HKWD_VMM_ZFOD             0x1b900000
#define HKWD_VMM_SIO              0x1ba00000
#define HKWD_VMM_PAGEOUT          0x1ba00000
#define HKWD_VMM_PAGEIN           0x1ba00001
#define HKWD_VMM_SEGCREATE        0x1bb00000
#define HKWD_VMM_SEGDELETE        0x1bc00000
#define HKWD_VMM_DALLOC           0x1bd00000
#define HKWD_VMM_PFENDOUT         0x1be00000
#define HKWD_VMM_PFENDIN          0x1be00001
#define HKWD_VMM_EXCEPT           0x1bf00000


#define HKWD_DD                   0x1c000000
#define HKWD_DD_STRATEGY          0x1c000000
#define HKWD_DD_START             0x1c100000
#define HKWD_DD_INTR              0x1c200000
#define HKWD_DD_GETBLK            0x1c300000
#define HKWD_DD_IOWAIT            0x1c400000
#define HKWD_DD_IODONE            0x1c500000
#define HKWD_DD_PHYSIO            0x1c600000
#define HKWD_DD_ATTACH            0x1c700000
#define HKWD_DD_PPDD              0x1c800000    /* parallel printer DD */
#define HKWD_DD_CDDD              0x1c900000    /* cdrom DD */
#define HKWD_DD_TAPEDD            0x1ca00000    /* tape DD */
#define HKWD_DD_CIODD             0x1cb00000    /* cio DD */
#define HKWD_DD_ETHDD             0x1cc00000    /* cio PC  ethernet DD */
#define HKWD_DD_ENTDD             0x1cd00000    /* cio MCA ethernet DD */
#define HKWD_DD_TOKDD             0x1ce00000    /* cio MCA tokenring DD */
#define HKWD_DD_C327DD            0x1cf00000    /* 3270 DD */

#define HKWD_DD_FDDD              0x22000000    /* diskette DD */
#define HKWD_DD_SCDISKDD          0x22100000    /* SCSI Disk DD */
#define HKWD_DD_BADISKDD          0x22200000    /* Bus Attached Disk DD */
#define HKWD_DD_SCSIDD            0x22300000    /* SCSI Adapter DD    */
#define HKWD_DD_MPQPDD            0x22400000    /* multi-protocol     */
#define HKWD_DD_X25DD             0x22500000    /* X.25 Device Driver */
#define HKWD_DD_GIO               0x22600000    /* G.I. Joe */
#define HKWD_DD_X25PRF            0x22700000    /* X.25 performance  */
#define HKWD_DD_SERDASDD          0x22800000    /* Serial DASD Subsystem */
#define HKWD_DD_TMSCSIDD          0x22900000    /* SCSI Target Mode DD */
#define	HKWD_DD_SOL		  0x24800000	/* optic 2-port */
#define HKWD_DD_CATDD             0x35000000    /* S/370 Channel */

#define HKWD_DDSPEC               0x1d000000
#define HKWD_DDSPEC_GRAPHIO       0x1d000000
#define HKWD_RAS_ERRLG            0x1d100000
#define HKWD_RAS_DUMP             0x1d200000

#define HKWD_LPP_PCSIM            0x1d300000
#define hkwd_PCSIM_FILE   1
#define hkwd_PCSIM_DISP   2
#define hkwd_PCSIM_CNTL   3
#define hkwd_PCSIM_PCAT   4
#define hkwd_PCSIM_DSKT   5

#define HKWD_WHIP_API             0x1d400000
#define HKWD_WHIP_DAEMON          0x1d500000
#define HKWD_WHIP_DRIVER          0x1d600000
#define HKWD_WHIP_EMUL            0x1d700000
#define HKWD_WHIP_FILE            0x1d800000

#define HKWD_COMM                 0x1e000000
#define HKWD_COMM_EM78            0x1e000000

/*
 * Socket, Network, and Network Interface hook id and sub-hook id's
 */

#define HKWD_NETPERF		0x25000000

#define HKWD_NETERR		0x25100000 

#define HKWD_SYSC_TCPIP		0x25200000
#define hkwd_socket_in		1
#define hkwd_bind_in		2
#define hkwd_listen_in		3
#define hkwd_accept_in		4	
#define hkwd_connect_in		5
#define hkwd_socketpair_in	6
#define hkwd_sendto_in		7
#define hkwd_send_in		8
#define hkwd_sendmsg_in		9
#define hkwd_recvfrom_in	10
#define hkwd_recv_in		11
#define hkwd_recvmsg_in		12
#define hkwd_shutdown_in	13
#define hkwd_setsockopt_in	14
#define hkwd_getsockopt_in	15
#define hkwd_getsockname_in	16
#define hkwd_getpeername_in	17
#define hkwd_gethostid_in	18
#define hkwd_sethostid_in	19
#define hkwd_gethostname_in	20
#define hkwd_sethostname_in	21
#define hkwd_getdomainname_in	22
#define hkwd_setdomainname_in	23
#define hkwd_socket_out		24
#define hkwd_socketpair_out	25

#define HKWD_SOCKET		0x25300000
#define hkwd_socreate_in	1
#define hkwd_sobind_in		2
#define hkwd_solisten_in	3
#define hkwd_sofree_in		4
#define hkwd_soclose_in		5
#define hkwd_soclose_out	6
#define hkwd_soabort_in		7
#define hkwd_soaccept_in	8
#define hkwd_soaccept_out	9
#define hkwd_soconnect_in	10
#define hkwd_soconnect2_in	11
#define hkwd_soconnect2_out	12
#define hkwd_sodisconnect_in	13
#define hkwd_sodisconnect_out	14
#define hkwd_sosend_in 		15
#define hkwd_sosend_out 	16
#define hkwd_soreceive_in	17
#define hkwd_soreceive_out	18
#define hkwd_soshutdown_in	19
#define hkwd_sorflush_in	20
#define hkwd_sosetopt_in	21
#define hkwd_sosetopt_out	22
#define hkwd_sogetopt_in	23
#define hkwd_sogetopt_out	24
#define hkwd_sohasoutofband_in	25
#define hkwd_sohasoutofband_out	26
#define hkwd_socreate_out	27

#define HKWD_MBUF		0x25400000
#define hkwd_m_get_in		1
#define hkwd_m_get_out		2
#define hkwd_m_getclr_in	3
#define hkwd_m_getclr_out	4
#define hkwd_m_free_in		5
#define hkwd_m_free_out		6
#define hkwd_m_copy_in		7
#define hkwd_m_copy_out		8
#define hkwd_m_copydata_in	9
#define hkwd_m_copydata_out	10
#define hkwd_m_pullup_1		11
#define hkwd_m_pullup_2		12
#define hkwd_mlowintr_in	13
#define hkwd_mlowintr_out	14
#define hkwd_m_low_test		15
#define hkwd_m_clgetm_in	16
#define hkwd_m_clgetm_out	17
#define hkwd_m_getclustm_in	18
#define hkwd_m_getclustm_out	19
#define hkwd_m_clattach_in	20
#define hkwd_m_clattach_out	21
#define hkwd_m_prepend_get_in	22
#define hkwd_m_prepend_get_out	23
#define hkwd_m_pullup_get_in	24
#define hkwd_m_pullup_get_out	25
#define hkwd_m_copy_get_in	26
#define hkwd_m_copy_get_out	27

/* These hook id's and sub hook id's are for Network Interface's */
#define HKWD_IFEN		0x25500000
#define HKWD_IFTR		0x25600000
#define HKWD_IFET		0x25700000
#define HKWD_IFXT		0x25800000
#define HKWD_IFSL		0x25900000
#define hkwd_statintr_in	1
#define hkwd_statintr_out	2	
#define hkwd_netintr_in		3
#define hkwd_netintr_out	4	
#define hkwd_attach_in		5
#define hkwd_attach_out		6
#define hkwd_detach_in		7
#define hkwd_detach_out		8
#define hkwd_init_in		9
#define hkwd_init_out		10
#define hkwd_ioctl_in		11
#define hkwd_ioctl_out		12
#define hkwd_output_in		13
#define hkwd_output_out		14
#define hkwd_reset_in		15
#define hkwd_reset_out		16
#define hkwd_rcv_in		17
#define hkwd_rcv_out		18
#define hkwd_incall_in		19
#define hkwd_incall_out		20	
#define hkwd_callcomp_in	21	
#define hkwd_callcomp_out	22
#define hkwd_clrind_in		23
#define hkwd_clrind_out		24
#define hkwd_slhiwat		25

/* This hook id is for TCP debug */
#define HKWD_TCPDBG		0x25a00000

#define HKWD_NET_MALLOC		0x34000000

#define hkwd_net_malloc		1
#define hkwd_net_free		2

/* These are TCPIP/FDDI related */
#define HKWD_IFCA		0x26000000
#define HKWD_IFSO		0x26100000
#define HKWD_IFFD		0x26200000
#define HKWD_DD_FDDIDD		0x26300000


/*
 * Data Link Control
 */
#define HKWD_SYSX_DLC_START       0x24000000 /* start link station    */
#define HKWD_SYSX_DLC_HALT        0x24100000 /* halt  link station    */
#define HKWD_SYSX_DLC_TIMER       0x24200000 /* timer                 */
#define HKWD_SYSX_DLC_XMIT        0x24300000 /* transmit packet       */
#define HKWD_SYSX_DLC_RECV        0x24400000 /* receive  packet       */
#define HKWD_SYSX_DLC_PERF        0x24500000 /* performance           */
#define HKWD_SYSX_DLC_MONITOR     0x24600000 /* monitor               */

#define HKWD_ODM_LIB              0x30000000 /* ODM code              */

/* hooks for STTY */
#define HKWD_STTY 		  0x40000000 
#define HKWD_STTY_STRTTY	  0x40100000 /* stream head tty */
#define HKWD_STTY_LDTERM 	  0x40200000 /* line discipline module */
#define HKWD_STTY_SPTR 		  0x40300000 /* serial printer discipline module */
#define HKWD_STTY_NLS 		  0x40400000 /* NLS mapping discipline module */
#define HKWD_STTY_PTY 		  0x40500000 /* pseudo tty driver */
#define HKWD_STTY_RS 		  0x40600000 /* RS driver */
#define HKWD_STTY_LION 		  0x40700000 /* LION driver */
#define HKWD_STTY_CXMA 		  0x40800000 /*  */
/* hooks till 0x40f00000 are reserved to stty for further use */

/* subhooks  for stty */
#define hkwd_TTY_CONFIG      0x01
#define hkwd_TTY_OPEN        0x02
#define hkwd_TTY_CLOSE       0x03
#define hkwd_TTY_WPUT        0x04
#define hkwd_TTY_RPUT        0x05
#define hkwd_TTY_WSRV        0x06
#define hkwd_TTY_RSRV        0x07
#define hkwd_TTY_REVOKE      0x08    /* for streamhead tty                   */
#define hkwd_TTY_IOCTL       0x09    /* for ioctls                           */
#define hkwd_TTY_PROC        0x0a    /* for drivers                          */
#define hkwd_TTY_SERVICE     0x0b    /* for drivers                          */
#define hkwd_TTY_SLIH        0x0c    /* for drivers                          */
#define hkwd_TTY_OFFL        0x0d    /* for drivers                          */
#define hkwd_TTY_LAST        0x0e    /* can be used for any specific entry   */


/***********************************************************************/
/*************** DISPLAY SUBSYSTEM TRACE HOOK DEFINITIONS ************-*/
/***********************************************************************/
/*
 *  DISPLAY SUBSYSTEM MAJOR TRACE HOOKS
 *
 *  The display subsystem is divided into functional components.
 *  Each functional component is given two trace hooks.  The first
 *  trace hook is designated HKWD_xxx_P and is for performance trace.
 *  The second hook is designated HKWD_xxx_D and is for debug trace.
 *  The components of the display subsystem, for tracing purposes,
 *  are: Virtual Terminal Subsystem (VTSS); Rendering Context
 *  Manager (RCM); Device Dependent Functions of VTSS and RCM (DDF);
 *  Graphics Adapter Interface (GAI), which is divided into its
 *  subcomponents, 2D (GAI2DM1), Resource Management (GAIRMS),
 *  3D Model 1 (GAI3DM1), and 3D Model 2 (GAI3DM2); Enhanced X-Windows
 *  (X11); IBM's extensions to X (X11EXT); the SGI-like GL API (GL);
 *  and IBM graPHIGS (GRAPHIGS).
 *
 *  The trace hooks for display occupy the range 500 to 51F.  Further,
 *  the range 520-52F has been set aside in case the display subsystem
 *  requires additional growth.
 *
 *
 */

#define  HKWD_DISPLAY_VTSS_P           0x50000000  /*  display group  */
#define  HKWD_DISPLAY_VTSS_D           0x50100000
#define  HKWD_DISPLAY_RCM_P            0x50200000
#define  HKWD_DISPLAY_RCM_D            0x50300000
#define  HKWD_DISPLAY_DDF_P            0x50400000
#define  HKWD_DISPLAY_DDF_D            0x50500000
#define  HKWD_DISPLAY_GAIRMS_P         0x50600000
#define  HKWD_DISPLAY_GAIRMS_D         0x50700000
#define  HKWD_DISPLAY_GAI2DM1_P        0x50800000
#define  HKWD_DISPLAY_GAI2DM1_D        0x50900000
#define  HKWD_DISPLAY_GAI3DM1_P        0x50a00000
#define  HKWD_DISPLAY_GAI3DM1_D        0x50b00000
#define  HKWD_DISPLAY_GAI3DM2_P        0x50c00000
#define  HKWD_DISPLAY_GAI3DM2_D        0x50d00000
#define  HKWD_DISPLAY_X11_P            0x50e00000
#define  HKWD_DISPLAY_X11_D            0x50f00000
#define  HKWD_DISPLAY_X11EXT_P         0x51000000
#define  HKWD_DISPLAY_X11EXT_D         0x51100000
#define  HKWD_DISPLAY_GL_P             0x51200000
#define  HKWD_DISPLAY_GL_D             0x51300000
#define  HKWD_DISPLAY_GRAPHIGS_P       0x51400000
#define  HKWD_DISPLAY_GRAPHIGS_D       0x51500000
#define  HKWD_ACPA_P                   0x51600000
#define  HKWD_ACPA_D                   0x51700000
#define  HKWD_DISPLAY_RESERVED_518     0x51800000
#define  HKWD_DISPLAY_RESERVED_519     0x51900000
#define  HKWD_DISPLAY_RESERVED_51A     0x51a00000
#define  HKWD_DISPLAY_RESERVED_51B     0x51b00000
#define  HKWD_DISPLAY_RESERVED_51C     0x51c00000
#define  HKWD_DISPLAY_RESERVED_51D     0x51d00000
#define  HKWD_DISPLAY_RESERVED_51E     0x51e00000
#define  HKWD_DISPLAY_RESERVED_51F     0x51f00000

/*----------------------------------------------------------------
 *  DISPLAY SUBSYSTEM MINOR TRACE HOOKS
 *
 *  Each hookword occupies 32 bits.  The most significant 12 bits are
 *  the major hook ID.  The next four bits are reserved.  The least
 *  significant 16 bits are for component use.  The Display Subsystem
 *  uses the 16 bits to encode certain flags and to encode a minor
 *  hook ID.
 *
 *  The general format for the 16 bits of the minor hook ID is:
 *
 *  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 *  |           routine ID              | func ID|ex|
 *  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 *    0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15
 *
 *
 *  The first twelve bits are a routine ID, giving a maximum of
 *  4096 routines per component.  The next three bits are the
 *  function ID within the component.  Function 000 is always
 *  reserved for the main entry/exit to the routine.  Functions
 *  001 to 110 are for internal functions within the routine.
 *  In this sense, function can be used to encode data as well.
 *  Function 111 is a special code that permits the first word
 *  of trace data to hold an expanded internal function code.
 *
 *  Bit 15, "ex", is set to 0 upon entry to the routine/function.
 *  The bit is set to 1 upon exit of the function.  This permits
 *  entry/exit tracing without reuse of the routine ID.
 *
 */

/*------------------------------------------------------------------
 *  VTSS MINOR TRACE HOOK DEFINITION
 *
 *  The VTSS component is has several major functional pieces, including
 *  the HFT (an AIX character device driver), the Screen Manager, the
 *  Mode Processor, the Virtual Display Driver, the Keyboard Device
 *  Driver, and other input device drivers.
 *
 *-----------------------------------------------------------------*/

#define  hkwd_DISPLAY_VTSS_HFT_OPEN          ( 0x001 << 4 )
#define  hkwd_DISPLAY_VTSS_HFT_CLOSE         ( 0x002 << 4 )
#define  hkwd_DISPLAY_VTSS_HFT_IOCTL         ( 0x003 << 4 )
#define  hkwd_DISPLAY_VTSS_HFT_CONFIG        ( 0x004 << 4 )
#define  hkwd_DISPLAY_VTSS_HFT_MULTIPLEX     ( 0x005 << 4 )
#define  hkwd_DISPLAY_VTSS_HFT_SELECT        ( 0x006 << 4 )
#define  hkwd_DISPLAY_VTSS_HFT_READ          ( 0x007 << 4 )
#define  hkwd_DISPLAY_VTSS_HFT_WRITE         ( 0x008 << 4 )
#define  hkwd_DISPLAY_VTSS_HFT_REVOKE        ( 0x009 << 4 )
#define  hkwd_DISPLAY_VTSS_HFT_INPUT         ( 0x00a << 4 )
#define  hkwd_DISPLAY_VTSS_HFT_OUTPUT        ( 0x00b << 4 )

#define  hkwd_DISPLAY_VTSS_VDD_ACT           ( 0x101 << 4 )
#define  hkwd_DISPLAY_VTSS_VDD_CFL           ( 0x102 << 4 )
#define  hkwd_DISPLAY_VTSS_VDD_CLR           ( 0x103 << 4 )
#define  hkwd_DISPLAY_VTSS_VDD_CPL           ( 0x104 << 4 )
#define  hkwd_DISPLAY_VTSS_VDD_DACT          ( 0x105 << 4 )
#define  hkwd_DISPLAY_VTSS_VDD_DEFC          ( 0x106 << 4 )
#define  hkwd_DISPLAY_VTSS_VDD_TERM          ( 0x107 << 4 )
#define  hkwd_DISPLAY_VTSS_VDD_INIT          ( 0x108 << 4 )
#define  hkwd_DISPLAY_VTSS_VDD_MOVC          ( 0x109 << 4 )
#define  hkwd_DISPLAY_VTSS_VDD_RDS           ( 0x10a << 4 )
#define  hkwd_DISPLAY_VTSS_VDD_TEXT          ( 0x10b << 4 )
#define  hkwd_DISPLAY_VTSS_VDD_SCR           ( 0x10c << 4 )
#define  hkwd_DISPLAY_VTSS_VDD_SETM          ( 0x10d << 4 )
#define  hkwd_DISPLAY_VTSS_VDD_STCT          ( 0x10e << 4 )

/*------------------------------------------------------------------
 *  GAI 2D Model 1 (GAI2DM1) MINOR TRACE HOOK DEFINITION
 *
 *  The minor IDs listed below are performance trace hooks
 *  for GAI 2D Model 1.
 *
 *  These functions are listed in the order specified in the             /
 *  2D Graphics Context Procedures structure.
 *
 *-----------------------------------------------------------------*/

#define  hkwd_DISPLAY_GAI2DM1_DESTROYGC             ( 1 << 4 )
#define  hkwd_DISPLAY_GAI2DM1_ANALYZEGC             ( 2 << 4 )
#define  hkwd_DISPLAY_GAI2DM1_COPYPIXTOWIN          ( 3 << 4 )
#define  hkwd_DISPLAY_GAI2DM1_COPYWINTOPIX          ( 4 << 4 )
#define  hkwd_DISPLAY_GAI2DM1_COPYWINTOWIN          ( 5 << 4 )
#define  hkwd_DISPLAY_GAI2DM1_COPYPIXBPTOWIN        ( 6 << 4 )
#define  hkwd_DISPLAY_GAI2DM1_COPYWINBPTOPIX        ( 7 << 4 )
#define  hkwd_DISPLAY_GAI2DM1_COPYWINBPTOWIN        ( 8 << 4 )
#define  hkwd_DISPLAY_GAI2DM1_POLYPOINT             ( 9 << 4 )
#define  hkwd_DISPLAY_GAI2DM1_POLYLINES             ( 10 << 4 )
#define  hkwd_DISPLAY_GAI2DM1_POLYSEGMENT           ( 11 << 4 )
#define  hkwd_DISPLAY_GAI2DM1_POLYRECTANGLE         ( 12 << 4 )
#define  hkwd_DISPLAY_GAI2DM1_POLYARC               ( 13 << 4 )
#define  hkwd_DISPLAY_GAI2DM1_FILLPOLYGON           ( 14 << 4 )
#define  hkwd_DISPLAY_GAI2DM1_POLYFILLRECT          ( 15 << 4 )
#define  hkwd_DISPLAY_GAI2DM1_POLYFILLARC           ( 16 << 4 )
#define  hkwd_DISPLAY_GAI2DM1_PUTIMAGE              ( 17 << 4 )
#define  hkwd_DISPLAY_GAI2DM1_IMAGETEXT8            ( 18 << 4 )
#define  hkwd_DISPLAY_GAI2DM1_POLYTEXT8             ( 19 << 4 )
#define  hkwd_DISPLAY_GAI2DM1_IMAGETEXT16           ( 20 << 4 )
#define  hkwd_DISPLAY_GAI2DM1_POLYTEXT16            ( 21 << 4 )
#define  hkwd_DISPLAY_GAI2DM1_PUSHPIXELS            ( 22 << 4 )

/*------------------------------------------------------------------
 *  M-ACPA/A MINOR TRACK HOOK DEFINITION
 *
 *  The minor IDs listed below are performance trace hooks
 *  for the Mulitmedia Audio Capture and Playback Adapter.
 *
 *-----------------------------------------------------------------*/

#define  hkwd_ACPA_CFGINIT                      ( 1 << 4 )
#define  hkwd_ACPA_CFGTERM                      ( 2 << 4 )
#define  hkwd_ACPA_OPEN                         ( 3 << 4 )
#define  hkwd_ACPA_CLOSE                        ( 4 << 4 )
#define  hkwd_ACPA_INTERRUPT                    ( 5 << 4 )
#define  hkwd_ACPA_READ                         ( 6 << 4 )
#define  hkwd_ACPA_WRITE                        ( 7 << 4 )
#define  hkwd_ACPA_IOCTL                        ( 8 << 4 )
#define  hkwd_ACPA_MPX                          ( 9 << 4 )
#define  hkwd_ACPA_SELECT                       ( 10 << 4 )
#define  hkwd_ACPA_INIT                         ( 11 << 4 )
#define  hkwd_ACPA_CHANGE                       ( 12 << 4 )
#define  hkwd_ACPA_START                        ( 13 << 4 )
#define  hkwd_ACPA_STOP                         ( 14 << 4 )
#define  hkwd_ACPA_PAUSE                        ( 15 << 4 )
#define  hkwd_ACPA_RESUME                       ( 16 << 4 )
#define  hkwd_ACPA_STATUS                       ( 17 << 4 )
#define  hkwd_ACPA_WAIT                         ( 18 << 4 )
#define  hkwd_ACPA_BUFFER                       ( 19 << 4 )
#define  hkwd_ACPA_LOAD                         ( 20 << 4 )
#define  hkwd_ACPA_DEVSTART                     ( 21 << 4 )
#define  hkwd_ACPA_DEVSTOP                      ( 22 << 4 )
#define  hkwd_ACPA_INITZ                        ( 23 << 4 )
#define  hkwd_ACPA_TIMEDOUT                     ( 24 << 4 )

/***********************************************************************/
/************ END OF DISPLAY SUBSYSTEM TRACE HOOK DEFINITIONS ********-*/
/***********************************************************************/

/* Netbios */

#define HKWD_NB_TRACE_NCB 		0x28300000
#define HKWD_NB_TRACE_IOB		0x28400000
#define HKWD_NB_TRACE_OPEN 	 	0x28500000
#define HKWD_NB_TRACE_COMM	 	0x28600000
#define HKWD_NB_TRACE_WRITE 		0x28700000
#define HKWD_NB_TRACE_READ 		0x28800000
#define HKWD_NB_TRACE_DATA	 	0x28900000
#define HKWD_NB_TRACE_MIF	 	0x28a00000
#define HKWD_NB_TRACE_VALUE		0x28b00000

/* Hook for kernel profiling */
#define HKWD_KERN_PROF		0x23400000  /* capture iar at clock intr */

/* Hooks for COBOL */
#define HKWD_COB                        0x23500000
#define HKWD_COB_RTE                    0x23600000
#define HKWD_COB_APPL                   0x23700000

/* Hooks for PSLA */
#define HKWD_PSLA_OPEN                  0x27200000
#define HKWD_PSLA_CLOSE                 0x27300000
#define HKWD_PSLA_READ                  0x27400000
#define HKWD_PSLA_WRITE                 0x27500000
#define HKWD_PSLA_IOCTL                 0x27600000
#define HKWD_PSLA_INTR                  0x27700000
#define HKWD_PSLA_CONF			0x27800000

/* Hook for NCS */
#define HKWD_NCS_NCK                    0x27900000

/* Hooks for SNA */
#define HKWD_SNA_COMM                   0x27000000
#define HKWD_SNA_API                    0x27100000
#define HKWD_GSNA_API                   0x28100000
#define HKWD_SNALIB_API                 0x28200000

/* Hooks for SYSXMSLA */
#define HKWD_HIA			0x28000000

/* Hook for Netview */
#define HKWD_NETVIEW_6000               0x29000000

/* Hook for Hursley */
#define HKWD_HSTC_PRD                   0x29100000

/* Hook for SYSXVCA */
#define HKWD_VCA_DD			0x29200000

/* VCA sub hook ids */
#define hkwd_VCA_ENTRY_OPEN			0x1
#define hkwd_VCA_EXIT_OPEN			0x2
#define hkwd_VCA_ENTRY_CLOSE			0x3
#define hkwd_VCA_EXIT_CLOSE			0x4
#define hkwd_VCA_ENTRY_READ			0x5
#define hkwd_VCA_EXIT_READ			0x6
#define hkwd_VCA_ENTRY_WRITE			0x7
#define hkwd_VCA_EXIT_WRITE			0x8
#define hkwd_VCA_ENTRY_CONFIG			0x9
#define hkwd_VCA_EXIT_CONFIG			0xA
#define hkwd_VCA_ENTRY_IOCTL			0xB
#define hkwd_VCA_EXIT_IOCTL			0xC
#define hkwd_VCA_ENTRY_VCA_SETUP		0xD
#define hkwd_VCA_EXIT_VCA_SETUP			0xE
#define hkwd_VCA_ENTRY_VCA_MEMCOPY		0xF
#define hkwd_VCA_EXIT_VCA_MEMCOPY		0x10
#define hkwd_VCA_ENTRY_MEM_SETUP		0x11
#define hkwd_VCA_EXIT_MEM_SETUP			0x12
#define hkwd_VCA_ENTRY_MEM_UNSET		0x13
#define hkwd_VCA_EXIT_MEM_UNSET			0x14
#define hkwd_VCA_ENTRY_GETSTATUS		0x15
#define hkwd_VCA_EXIT_GETSTATUS			0x16
#define hkwd_VCA_ENTRY_SETMASK			0x17
#define hkwd_VCA_EXIT_SETMASK			0x18
#define hkwd_VCA_ENTRY_CAPTURE_WAIT		0x19
#define hkwd_VCA_EXIT_CAPTURE_WAIT		0x1A
#define hkwd_VCA_ENTRY_INTR_ENABLE		0x1B
#define hkwd_VCA_EXIT_INTR_ENABLE		0x1C
#define hkwd_VCA_ENTRY_INTR_DISABLE		0x1D
#define hkwd_VCA_EXIT_INTR_DISABLE		0x1E
#define hkwd_VCA_ENTRY_VCA_SET_MODE		0x1F
#define hkwd_VCA_EXIT_VCA_SET_MODE		0x20
#define hkwd_VCA_ENTRY_VCA_SET_CONTROLS		0x21
#define hkwd_VCA_EXIT_VCA_SET_CONTROLS		0x22
#define hkwd_VCA_ENTRY_VCA_SET_CURSOR		0x23
#define hkwd_VCA_EXIT_VCA_SET_CURSOR		0x24
#define hkwd_VCA_ENTRY_VCA_SET_WINDOW		0x25
#define hkwd_VCA_EXIT_VCA_SET_WINDOW		0x26
#define hkwd_VCA_ENTRY_VCA_SET_DACS		0x27
#define hkwd_VCA_EXIT_VCA_SET_DACS		0x28
#define hkwd_VCA_ENTRY_ALTDAC			0x29
#define hkwd_VCA_EXIT_ALTDAC			0x2A
#define hkwd_VCA_ENTRY_VCA_GET_MODE		0x2B
#define hkwd_VCA_EXIT_VCA_GET_MODE		0x2C
#define hkwd_VCA_ENTRY_VCA_GET_CONTROLS		0x2D
#define hkwd_VCA_EXIT_VCA_GET_CONTROLS		0x2E
#define hkwd_VCA_ENTRY_VCA_GET_CURSOR		0x2F
#define hkwd_VCA_EXIT_VCA_GET_CURSOR		0x30
#define hkwd_VCA_ENTRY_VCA_GET_WINDOW		0x31
#define hkwd_VCA_EXIT_VCA_GET_WINDOW		0x32
#define hkwd_VCA_ENTRY_VCA_GET_DACS		0x33
#define hkwd_VCA_EXIT_VCA_GET_DACS		0x34
#define hkwd_VCA_ENTRY_READDAC			0x35
#define hkwd_VCA_EXIT_READDAC			0x36
#define hkwd_VCA_ENTRY_VCA_SET_DAC_DEFAULTS	0x37
#define hkwd_VCA_EXIT_VCA_SET_DAC_DEFAULTS	0x38
#define hkwd_VCA_ENTRY_VCA_INTR			0x39
#define hkwd_VCA_EXIT_VCA_INTR			0x3A
#define hkwd_VCA_ENTRY_VCA_OUTB			0x3B
#define hkwd_VCA_EXIT_VCA_OUTB			0x3C
#define hkwd_VCA_ENTRY_VCA_INB			0x3D
#define hkwd_VCA_EXIT_VCA_INB			0x3E
#define hkwd_VCA_ENTRY_VCA_TIMED_OUT		0x3F
#define hkwd_VCA_EXIT_VCA_TIMED_OUT		0x40
#define hkwd_VCA_ENTRY_VCA_ALTREG		0x41
#define hkwd_VCA_EXIT_VCA_ALTREG		0x42
#define hkwd_VCA_ENTRY_VCA_READREG		0x43
#define hkwd_VCA_EXIT_VCA_READREG		0x44
#define hkwd_VCA_ENTRY_VCA_KSET_CURSOR		0x45
#define hkwd_VCA_EXIT_VCA_KSET_CURSOR		0x46
#define hkwd_VCA_ENTRY_VCA_KGET_CURSOR		0x47
#define hkwd_VCA_EXIT_VCA_KGET_CURSOR		0x48
#define hkwd_VCA_ENTRY_VCA_DELAY		0x49
#define hkwd_VCA_EXIT_VCA_DELAY			0x4A
#define hkwd_VCA_ENTRY_VCA_INITIALIZE		0x4B
#define hkwd_VCA_EXIT_VCA_INITIALIZE		0x4C
#define hkwd_VCA_ENTRY_VCAERR			0x4D
#define hkwd_VCA_EXIT_VCAERR			0x4E

/*
 * STREAMS (PSE) Hookwords
 */

#define	HKWD_PSE		0x7FF00000

/* system calls */
#define	hkwd_pse_getmsg_in	0x01
#define	hkwd_pse_getmsg_out	0x02
#define	hkwd_pse_getpmsg_in	0x03
#define	hkwd_pse_getpmsg_out	0x04
#define	hkwd_pse_putmsg_in	0x05
#define	hkwd_pse_putmsg_out	0x06
#define	hkwd_pse_putpmsg_in	0x07
#define	hkwd_pse_putpmsg_out	0x08

/* driver entry points */
#define	hkwd_pse_open_in	0x09
#define	hkwd_pse_open_out	0x0a
#define	hkwd_pse_clone_in	0x0b
#define	hkwd_pse_clone_out	0x0c
#define	hkwd_pse_close_in	0x0d
#define	hkwd_pse_close_out	0x0e
#define	hkwd_pse_read_in	0x0f
#define	hkwd_pse_read_out	0x10
#define	hkwd_pse_write_in	0x11
#define	hkwd_pse_write_out	0x12
#define	hkwd_pse_ioctl_in	0x13
#define	hkwd_pse_ioctl_out	0x14
#define	hkwd_pse_poll_in	0x15
#define	hkwd_pse_poll_out	0x16

/* hooks for libpthread library */
#define HKWD_PTHREAD_MUTEX_LOCK 	0x23000000 
#define HKWD_PTHREAD_MUTEX_UNLOCK 	0x23100000
#define HKWD_PTHREAD_SPIN_LOCK  	0x23200000
#define HKWD_PTHREAD_SPIN_UNLOCK 	0x23300000

/* hooks for thread-based services */
#define HKWD_KERN_ASSERTWAIT		0x46000000
#define HKWD_KERN_CLEARWAIT		0x46100000
#define HKWD_KERN_THREADBLOCK		0x46200000
#define HKWD_KERN_EMPSLEEP		0x46300000
#define HKWD_KERN_EWAKEUPONE		0x46400000
#define HKWD_SYSC_CRTHREAD		0x46500000
#define HKWD_KERN_KTHREADSTART		0x46600000
#define HKWD_SYSC_TERMTHREAD  		0x46700000
#define HKWD_KERN_KSUSPEND    		0x46800000
#define HKWD_SYSC_THREADSETSTATE    	0x46900000
#define HKWD_SYSC_THREADTERM_ACK    	0x46A00000
#define HKWD_SYSC_THREADSETSCHED    	0x46B00000
#define HKWD_KERN_TIDSIG    		0x46C00000
#define HKWD_KERN_WAITLOCK  		0x46D00000
#define HKWD_KERN_WAKEUPLOCK		0x46E00000

/* Hook for PCMCIA Socket Services */
#define	HKWD_PCMCIA_SS			0x23000000
/* Hook for PCMCIA Card Services */
#define	HKWD_PCMCIA_CS			0x34300000

#endif /* _H_TRCHKID */
