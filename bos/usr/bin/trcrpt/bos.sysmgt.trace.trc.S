# @(#)56      1.22  src/bos/usr/bin/trcrpt/bos.sysmgt.trace.trc.S, cmdtrace, bos412, 9446A412a 11/14/94 18:31:51
# COMPONENT_NAME: CMDTRACE   system trace logging and reporting facility
#
# FUNCTIONS: template file for trcrpt
#
# ORIGINS: 27, 83
#
# (C) COPYRIGHT International Business Machines Corp. 1993, 1994
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
# 
# LEVEL 1, 5 Years Bull Confidential Information
# 
* /etc/trcfmt
#
# FLIH and return from FLIH.
# RESUME serves as a return from FLIH.
# Level 5 flihs call the 'flih()' builtin to advance the Interrupt Depth.
# It the process that is resumed is an interrupt process, $procflg will
#   be non-zero and the mst will be the mst for the interrupt process.
#   The mst is a state save area for a process or interrupt.
# The "wait" process is written out as "IDLE".
# RESUME calls the resume() builtin to decrement the Interrupt Depth.
#
+100 1.1 L=INT "@FLIH" {{ $level = $HD }} \
        $level, \
        2 "MACHINE CHECK", \
        3 "DATA ACCESS PAGE FAULT", \
        4 "INSTRUCTION PAGE FAULT", \
        5 { "I/O INTERRUPT" flih($level) }, \
        6 "ALIGNMENT ERROR", \
        7 "PROGRAM CHECK", \
        8 "FLOATING POINT UNAVAILABLE", \
	0x0a "FLOATING POINT IMPRECISE", \
	0x20 "RUN-MODE INTERRUPT", \
	0x31 "DECREMENTER INTERRUPT",\
	0x32 "QUEUED INTERRUPT",\
        \* { flih $level }
+200 1.1 "@RESUME" {{ $procflg = $HD }} {{ $e = $EXECPATH }} \
        resume($procflg) \
        $procflg, \
        0  { $PRI, 7F "resume    IDLE", \* { `resume   $EXECPATH` } }, \
        \* { L=INT `resume interrupt process   mst=$D1` }


#
# SLIH and return from SLIH.
# The name of the slih function is looked up by the builtin slihlookup().
# If not found, the hex address of the slih function is output.
# The slihs are looked up in the /unix namelist file and in the list of
#   loaded kernel extensions added to the trace log be the trace demon.
#
+102 1.1 L=INT "@SLIH"             "slih" slihlookup($D1) \
        endtimer(0x102,0x102) starttimer(0x102,0x102) starttimer(0x103,0x103)

+103 1.1 L=INT "@RETURN FROM SLIH" "return from slih" \
        endtimer(0x103,0x103)

#
# SVC and return from SVC.
# The current SVC is set by setting the reserved register $SVC to the
#   hooked function descriptor. Using $SVC by itself will output the
#   name of the current SVC, or the hex address if it cannot be found.
# Don't print out return from SVC unless there is an error.
#
+101 1.0 L=SVC "@SYSTEM CALL" \
        {{ $SVC = $D1 }} \
        $SVC "LR = "$D2\
        starttimer(0x101,$TID)
+106 1.3 "@DISPATCH" \
        {{ $pidsv = $PID }} {{ $TID = $D2 }} {{ $PID = $D1 }} {{ $CPUID = $D5 }} {{ $e = $EXECPATH }} {{ $PRI = $D4%W16.31 }} \
        "dispatch:   " \
        $e, \
        swapper { "scheduler" }, \
	\* { $PRI, 7F { $SKIP }, \* { "cmd="$e%A14.14  "pid="prpid($PID)  "tid=" prpid($TID) G16 "priority="U2 "old_tid="$D3%D1 "old_priority="U2 "CPUID="$D5%U1 } } \
        $PRI, \
        7F { "idle time:" endtimer(0x10c,0x10C) } \
        endtimer(0x106,0x106) starttimer(0x106,0x106)
+10C 1.2 "@DISPATCH IDLE PROCESS" \
	 {{ $PID = $D1 }} {{ $TID = $D2 }} {{ $CPUID = $D5 }} \
        "dispatch:   idle process  pid="$D1%D1  "tid="$D2%D2 \
	 G16 "priority="U2 "old_tid="$D3%D1 "old_priority="U2 \
	 "CPUID="$D5%U1	\
        starttimer(0x10c,0x10C)
+11F 1.1 "@SET ON READY QUEUE"  \
	"setrq:   " \
       $D3, \
       7F { $SKIP }, \
       \* { "cmd="$D1%P4 "pid="prpid($D1) "tid="prpid($D2) "priority="$D3%D1 \
	    "policy="$D4%D1 }

#
# DISPATCH a process and DISPATCH the IDLE process.
# The current PID is set by setting the reserved register $PID to the
#   hooked process id.
# The reserved register $EXECPATH will output the name of the current
#   process, or the hex process id if it is not known.
# trcrpt associates process ids with filenames in the EXEC and FORK hooks.
#
# The IDLE process has its own dispatch hook because it usually gives little
#   information about the process state of the system and it floods the
#   real DISPATCH hook with 16 msec. clock interrupt / resume IDLE activity.
#
# SET ON READY QUEUE corresponds to a wakeup of that process. A DISPATCH
#   hook for this process will soon come.
#



+134 1.0 L=SVC "@EXEC SYSTEM CALL" \
        HT & 0x07, \
        01 { \
             $HD, \
             0 { "exec:   cmd="$CURRFILE  "pid="prpid($PID)  \
		 "tid="prpid($TID) pidinstall($PID,0,$CURRFILE,$TID,$CPUID,$PRI) }, \
             1 { trcgen } \
        }, \
        \* { G8 \
             $D1, \
             0 { {{ $EXECPATH = A14 }} G8 "exec:   "A14 }, \
             1 { "exec:   cmd="prargs($HD)  "pid="prpid($PID)  \
		"tid="prpid($TID) pidinstall($PID,0,$CURRFILE,$TID,$CPUID,$PRI) } \
        }

+139 1.0 L=SVC "@FORK SYSTEM CALL" \
       `fork:   pid=$D1%D1 tid=$D2%D1` pidinstall($D1,$PID,$EXECPATH,$D2,0xff,0xff)


# The lookuppn hook installs a filename/vnode pair into the trcrpt memory.
# It is a two part process because in the kernel, the vnode is accessed
# directory by directory, so that when the vnode is determined, the file
# pathname is no longer available.
# The builtin lookuppninstall1 installs the filename as the "current file"
#   with trcrpt.
# The builtin lookuppninstall2 installs the vnode    as the "current vnode"
#   and installs the current_vnode/current_file as a vnode/filename pair.
#
+107 1.0 "@FILENAME TO VNODE (lookuppn) " \
        HT & 0x07, \
        00 { G8 `lookuppn: $HD%S1` G8 lookuppninstall1($HD%S1) }, \
        \* { \
                $D1, \
                0  { `lookuppn: file not found` }, \
                \* { lookuppninstall2($D1) $NOPRINT } \
        }

#
# OPEN and CREATE associate a filename with a file descriptor (fd).
# The builtin fdinstall() installs the file descriptor and the
#   "current vnode" from lookuppn as a file_descriptor/vnode pair
#   for this process id.
# The builtin fdtofilename() looks up the filename for the given fd
#   for this process id. If not found, output nothing.
#
+15B 1.0 L=SVC "@OPEN SYSTEM CALL" \
        fdinstall($D1) \
        open fdtofilename($D1) fd=prfd($D1) openflags($D2) \
        $D2%o4 & 0400, 0400 { mode=filemode($D3) }

+130 1.0 L=SVC "@CREAT SYSTEM CALL" \
        creat lookuppninstall2($D1) $NOPRINT

#
# The next set of hooks is for tracing file activity from read/write SVCs
# to a device interrupt.
# 1. LFS read/write hooks the file descriptor. The vnode for this descriptor
#    has already been hooked by open.
# 2. PFS read/write hooks the vnode (vp) and the virtual address and length
#    of the io.
# 3. If a page fault, VMM start io hooks the virtual page and a buffer pointer
#    (bp).
# 4. LVM hooks the VMM bp with the LVM bp.
# 5. Device driver strategy hooks the LVM by with the "coalesced" bp.
# 6. Device driver start routine has the actual io command.
# 7. Device driver iodone hooks the bp for this io.
# The vnode is carried along at each bufinstall(), so that the filename
# can be looked up at any step along the way.
#
# Note: There are 3 entry/exit points in this path.
# 1. svc/return from svc
# 2. lvm start/lvm end
# 3. device driver start/device driver iodone
#
# Note on builtins:
# fdtofilename(fd)              look up filename by file descriptor
# vnodetofilename(vp)           look up filename by vnode
# vpagetofilename(vpage,sid)    look up filename by vpage and segment id
# sidtofilename(sid)            look up filename by segment id
# buftofilename(bp)             look up filename by buf struct
#
#
# Details on specific hooks:
#
# The LFS hooks are straightforward. They provide information closest
# to the program that is doing the read/write, giving the file descriptor,
# his target buffer, byte count, and filename.
# This is how the LFS hooks are called:
# read(fd,buf,count) -> TRCHKL5(HKWD_SYSC_READ,fd,vp,iov_base,count,buf);
#
# The PFS hook is a two step hook.
# First, hook the vnode. The inode hooked also but is not necessary for
#   a trace path.
# Second, hook the virtual address of the (internally mapped) file.
# This will used by the VMM hook if a page fault occurs during the access
#   of this mapped file.
# The builtin pfsrdwrinstall1(vp) sets the current vnode of this process
#   to vp.
# The builtin pfsrdwrinstall2(VA.S,count) creates a virtual address/vnode
#   structure to be filled in be VMM hooks if a page fault occurs.
#   to vp.
# This is how the PFS hooks are called:
#  TRCHKL2(HKWD_KERN_PFS|1,vp,ip);
#  TRCHKL4(HKWD_KERN_PFS|2,vaddr,bcount,sid,ip);
#
# The VMM hook.
# If a page fault occurs due to an access of a mapped file from PFS,
#   this will initiate io through a buffer pointer.
# The builtin vmmbufinstall() will lookup the vnode of the file through
#   the virtual page / sid and install the vnode and buf as a
#   vnode / bp pair, to be used by lvm on down.
# This is how this VMM hook is called:
#  TRCHKL5(HKWD_VMM_PAGEIO,sid,sibits,vpage,ppage,bp)
#   The sibits contain describes the type and some status about the segment
#   and are interpreted through the $1BF template subroutine.
#
# The LVM hook.
# The important hook here is the pstart hook, which takes the bp from the
#   VMM and converts it to the bp for the physical device.
# Also hooked is the pblock of the device and the device name.
# This is how this LVM hook is called:
#  TRCHKL5(HKWD_KERN_LVMSIMP|hkwd_LVM_PSTART,pbp,lbp,bflags,pblock,dev);
#
# Device driver hooks.
# These are in the device driver section. The file name is passed through
# the buf struct (bp).
#

+19C 1.0 L=SVC "@WRITE SYSTEM CALL" \
        `write($D1%D1,$D3%X4,$D4%X0)` fdtofilename($D1)

+163 1.0 L=SVC "@READ SYSTEM CALL" \
        `read($D1%D1,$D3%X4,$D4%X0)` fdtofilename($D1)

+10A 1.0 @KERN_PFS \
        $HD, \
        0x01  { `PFS rdwr (vp,ip)=($D1,$D2)` \
                        pfsrdwrinstall1($D1,$D2) vnodetofilename($D1) $BREAK }, \
        2  { {{ $h = "readi " }} }, \
        3  { {{ $h = "writei" }} }, \
        \* { `PFS mode=$HD` } \
        `PFS $h VA.S=$D3.$D2 bcount=$D4 ip=$D1` \
        pfsrdwrinstall2($D3,$D4,$D2,$D1) \
        {{ $vpage = $D3 / 4096 }} vpagetofilename($vpage,$D2)

+10B 1.0 "@LVM BUF STRUCT FLOW" \
{{ \
        lvmflags(flags) \
                BITFLAGS $flags, \
                0x0001 WRITEV, \
                0x0002 HWRELOC, \
                0x0004 UNSAFEREL, \
                0x0008 RORELOC, \
                0x0010 NO_MWC, \
                0x0020 MWC_RCV_OP, \
                0x0080 RESYNC_OP, \
                0x0100 AVOID_C1, \
                0x0200 AVOID_C2, \
                0x0400 AVOID_C3 \
}} \
        $HD%D1, \
        12 { `LVM rblocked:       lbp=$D1` },\
        1  { `LVM pend:           pbp=$D1 resid=$D4 error=$D3` bflags($D2) },\
        13 { `LVM lend:           lbp=$D1 resid=$D4 error=$D3` bflags($D2) },\
        11 { {{ $flgs = $D2 & 0xFFFF }} {{ $opts = $D2 / 0x10000 }} \
            `LVM lstart:` devtoname($D3) \
            `lbp=$D1 lblock=$D4 bcount=$D5` bflags($flgs) `opts:` \
	     lvmflags($opts) }, \
        2  { {{ $flgs = $D3 & 0xFFFF }} {{ $opts = $D3 / 0x10000 }} \
             `LVM pstart:` devtoname($D5) \
             `pblock=$D4 (lbp,pbp)=($D2,$D1)` bflags($flgs) `opts:` \
	     lvmflags($opts) bufinstall($D2,$D1) buftofilename($D2) }, \
        \*  { `LVM mode=$HD` }
+10D 1.0 "@FILE VFS AND INODE" \
        `vfs number=$HD, inode number=$D1`

#
# These are miscellaneous kernel events.
# xmalloc/xmfree is the kernel's virtual memory allocator.
#   The $SYMBOLVALUE/$SYMBOLNAME is an attempt to find a named heap
#   from the symbol table.
#
# vmm_forkcopy is called by fork to duplicate parts of a process.
#
# issig is hooked if there are signals ready to be received by the
# current process.
#
# Note: xmalloc/xmfree might be a good candidate for entry/exit
#       performance analysis, since it does involve a search algorithm,
#       it is called from mant parts of the system, and it a "terminal"
#       routine that does not call other routines.
#
+116 1.0 "@XMALLOC size,align,heap" {{ $SYMBOLVALUE = $D3 }} \
        `xmalloc($D1,$SYMBOLNAME)`

+117 1.0 "@XMFREE address,heap" \
        `xfree($D1,$D2)`

+118 1.0 "@FORKCOPY" vmm_forkcopy

+11E 1.0 L=KERN "@ISSIG"	\
        "issig:   pid="$D1%D1  "tid="$D2%D1  "t_cursig="prsignal($D3)

+169 1.0 "@SBREAK SYSTEM CALL" `sbreak   new dmax is $D1`

+120 1.0 "@ACCESS SYSTEM CALL" access mode=filemode($D2)

#
# These system calls have some meaning to RAS, and there are hooks
# to expand the important parameters.
#
+122 1.0 L=SVC "@ALARM SYSTEM CALL" {{ $secs = $D1 }} \
        $secs, \
        0  { "alarm off" }, \
        \* { `alarm $secs seconds` }

#
# File-oriented system calls
#
# hkwd_SYSC_FULLSTAT  1
# hkwd_SYSC_LOCKFX    2
# hkwd_SYSC_STAT      3
# hkwd_SYSC_MKNOD     4
# hkwd_SYSC_ACCESS    5
# hkwd_SYSC_CHOWN     6
# hkwd_SYSC_CHOWNX    7
# hkwd_SYSC_CLOSEX    8
# hkwd_SYSC_OPENX     9
# hkwd_SYSC_FCHMOD    10
# hkwd_SYSC_FCHOWN    11
# hkwd_SYSC_FCHOWNX   12
# hkwd_SYSC_FCLEAR    14
# hkwd_SYSC_FSYNC     15
# hkwd_SYSC_FTRUNCATE 16
# hkwd_SYSC_TRUNCATE  17
# hkwd_SYSC_IOCTLX    18
# hkwd_SYSC_READX     19
# hkwd_SYSC_WRITEX    20

+108 1.0 L=SVC "@FILE ORIENTED SYSTEM CALLS" \
        $HD%D1, \
        5  { access $CURRFILE filemode($D1) }, \
        10 { fchmod fdtofilename($D1) filemode($D2) }, \
        6  { chown  $CURRFILE uid=$D1%D1 gid=$D2%D1 }, \
        11 { fchown fdtofilename($D1) uid=$D2%D1 gid=$D3%D1 }, \
        7  { chownx $CURRFILE uid=$D1%D1 gid=$D2%D1 }, \
        12 { fchownx fdtofilename($D1) uid=$D2%D1 gid=$D3%D1 }, \
        16 { ftruncate fdtofilename($D1) to $D2 }, \
        17 { truncate $CURRFILE to $D1 }, \
        18 { ioctlx fdtofilename($D1) cmd=$D2 }, \
        2  { lockfx fdtofilename($D1) start=$D4 length=$D3 whence=$D2 }, \
        4  { mknod  $CURRFILE filemode($D1) }, \
        15 { fsync fdtofilename($D1) }, \
        19 { `readx($D1%D1,$D3%X4,$D4%X0)` fdtofilename($D1) }, \
        20 { `writex($D1%D1,$D3%X4,$D4%X0)` fdtofilename($D1) }, \
        9  { fdinstall($D1) openx fdtofilename($D1) fd=prfd($D1) filemode($D2) }

# access mode:
#       BITFLAGS $D1, \
#       4 "R", \
#       2 "W", \
#       1 "X", \
#       & 7 0 "(existence)"

+12E 1.0 L=SVC "@CLOSE SYSTEM CALL" close fdtofilename($D1) fd=prfd($D1) \
	lookuppninstall1('') lookuppninstall2(-1) fdinstall($D1)

+14C 1.0 L=SVC "@IOCTL SYSTEM CALL" \
        {{ $fd = $D1 }} \
        {{ $command = $D2 }} G10 {{ $ttycmdhi = X1 }} {{ $ttycmdlo = X1 }} \
        {{ $arg = $D3 }} \
        ioctl fd=prfd($D1) \
        $ttycmdhi, \
        54 { \
                $ttycmdlo, \
                01 { TCGETA  $BREAK }, \
                02 { TCSETA  $BREAK }, \
                03 { TCSETAW $BREAK }, \
                04 { TCSETAF $BREAK }, \
                05 { TCSBRK  arg=$arg $BREAK }, \
                06 { TCXONC  arg=$arg $BREAK }, \
                07 { TCXFLSH arg=$arg $BREAK } \
        } \
        command=$command arg=$arg

+154 1.0 L=SVC "@LSEEK SYSTEM CALL" \
        {{ $fd = $D1 }} {{ $offset = $D2 }} {{ $whence = $D3 }} \
        $whence, \
        0 { lseek fd=prfd($D1) to $offset } ,\
        1 { lseek fd=prfd($D1) relative $offset } ,\
        2 { lseek fd=prfd($D1) relative $offset from end of file } ,\
        3 { lseek fd=prfd($D1) offset=$offset whence=$whence "(invalid)" }

+152 1.0 L=SVC "@LOCKF SYSTEM CALL" \
        lockf fdtofilename($D1) fd=prfd($D1) \
        $D2, \
        0  { "unlock" }, \
        1  { "lock wait" }, \
        2  { "lock busy" }, \
        \* { cmd=$D2 } \
        $D3 bytes

+15F 1.0 "@PIPE SYSTEM CALL" \
        `pipe read_fd=prfd($D1) write_fd=prfd($D2)`

+14E 1.0 "@KILL SYSTEM CALL" \
        kill: signal prsignal($D2) to process $D1%D1 $D1%P4

+16F 1.0 SBREAK SYSTEM CALL  $DEFAULT

+137 1.0 "@FCNTL SYSTEM CALL" \
        fcntl fdtofilename($D1) \
        $D2, \
        0 { F_DUPFD original file descriptor $D3 }, \
        1 { 'F_GETFD' }, \
        2 { 'F_SETFD' }, \
        3 { 'F_GETFL' }, \
        4 { 'F_SETFL' }, \
        10 { 'F_CLOSEM' } \
        return value $D4

+135 1.0 L=SVC "@EXIT SYSTEM CALL" \
        {{ $waitstat = $D1 }} {{ $pid = $D2 }} {{ $lockct = $D3 }} \
        kexit() \
        exit wait_status=$waitstat lockct=$lockct

+13A 1.0 L=SVC "@FSTAT SYSTEM CALL" \
        fstat fdtofilename($D1) fd=prfd($D1)

+19B 1.0 L=SVC "@WAIT SYSTEM CALL" \
        {{ $rv = $D1 }} {{ $pid = $D2 }} {{ $pflag = $D3 }} {{ $wstat = $D4 }} \
        wait rv=$rv pid=$pid pflag=$pflag wstat=$wstat

+13E 1.0 L=SVC "@FULLSTAT SYSTEM CALL" \
        {{ $filename = X4 }} {{ $cmd = X4 }} {{ $buf = X4 }} \
        fullstat filename=$filename cmd=$cmd

+160 1.0 L=SVC "@PLOCK" \
        plock $EXECPATH \
        $D1, \
        0 "UNLOCK", \
        1 "PROCESS LOCK", \
        2 "TEXT SEGMENT LOCK", \
        4 "DATA SEGMENT DATLOCK"

#
# IPC hooks for shared memory, semaphores, and messages
# The most common hook here is the shmat call for mapping a file.
#

#
# trchk(SHMCTL,shmid,cmd,arg)
#
+174 1.0 "@SHMCTL SYSTEM CALL" \
        shmctl shmid=$D1%D1 \
        $D2, \
        0 {  IPC_RMID }, \
        1 { `IPC_SET  buf=$D3` }, \
        2 { `IPC_STAT buf=$D3` }, \
        6 { `SHM_SIZE buf=$D3` }

+175 1.0 "@SHMGET SYSTEM CALL" \
        shmget key=$D1 size=$D2 \
        BITFLAGS $D3, \
        01000 CREATE ,\
        02000 IPC ,\
        0400  "r" "-" ,\
        0200  "w" "-" ,\
        0100  "x" "-" ,\
        0040  "r" "-" ,\
        0020  "w" "-" ,\
        0010  "x" "-" ,\
        0004  "r" "-" ,\
        0002  "w" "-" ,\
        0001  "x" "-"

+176 1.0 "@SHMAT SYSTEM CALL" \
        shmat shmid=$D1%D1 \
        $D2, \
        0  {}, \
        \* { addr=$D2%X4 } \
        BITFLAGS $D3, \
        0x0800 "MAP", \
        0x1000 "RDONLY", \
        0x2000 "ROUND", \
        0x4000 "COPY-ON-WRITE"

+177 1.0 "@SHMDT SYSTEM CALL" \
        shmdt addr=$D1

+178 1.0 "@MSGCTL SYSTEM CALL" \
        msgctl msgid=$D1 cmd=$D2 buf=$D3

+179 1.0 "@MSGGET SYSTEM CALL" \
        msgget key=$D1

+17A 1.0 "@MSGSND SYSTEM CALL" \
        msgsnd msgid=$D1 msgp=$D2 size=$D3 flags=$D4

+17B 1.0 "@MSGRCV SYSTEM CALL" \
        msgrcv msgid=$D1 msgp=$D2 size=$D3 typ=$D4 flags=$D5

+17C 1.0 MSGXRCV $17B

+17D 1.0 "@SEMCTL SYSTEM CALL" \
        semctl semid=$D1 semnum=$D2 cmd=$D3 arg=$D4

+17E 1.0 "@SEMGET SYSTEM CALL" \
        semget key=$D1 nsems=$D2 flags=$D3

+17F 1.0 "@SEMOP SYSTEM CALL" \
        semop semid=$D1 sops=$D2 nops=$D3

#
# LVM hooks
# This hook are LVM "events", as opposed to the LVM flow control hooks
# for file tracing.
# Most of the events are bad block remapping.
#
# LVM_PEND        1     End of physical request X
# LVM_PSTART      2     Start of physical request X
# LVM_RELOCINGBLK 3     Encountered relocated blk
# LVM_OLDBADBLK   4     Old bad block
# LVM_BADBLKDONE  5     Blk relocation complete
# LVM_NEWBADBLK   6     New bad block found
# LVM_SWRELOC     7     Software relocating bad blk
# LVM_KDAEMON     8     Kick off a daemon request
# LVM_EDAEMON     9     End of a daemon request
# LVM_RESYNCPP    10    Resyncing LP mirrors
# LVM_LSTART      11    Start of logical request X
# LVM_RBLOCKED    12    request blocked - conflicts X
# LVM_LEND        13    End of logical request X
# LVM_OPEN        14    LV open
# LVM_CLOSE       15    LV close
# LVM_READ        16    LV character device read
# LVM_WRITE       17    LV character device write
# LVM_IOCTL       18    LVM device driver ioctl
# LVM_UPD_BBDIR   19    Update BB directory. pb queued
# LVM_BBDIREND    20    Iodone for BB directory I/O
# LVM_BBDIROP     21    Continue BB directory operation
# LVM_BBADD       22    Add BB directory entry on disk
# LVM_BBDEL       23    Delete BB dir entry on disk
# LVM_BBUPD       24    Update BB dir entry on disk
# LVM_BBDIRDONE   25    Completion of BB dir updating
# LVM_SA_STRT     26    New SA request put on hold list
# LVM_SA_WRT      27    Build buf struct for next SA on wheel
# LVM_SA_IODONE   28    iodone for VGSA write
# LVM_SA_CONT     29    continue writing VGSAs
# LVM_SA_RTN      30    return requests to callers
# LVM_SA_CONFIG   31    hd_config request for VGSA wheel
# LVM_CA_CHKCACH  32    check for a cache hit
# LVM_CA_CHKHIT   33    got a cache hit
# LVM_CA_WRT      34    start cache writes
# LVM_CA_WEND     35    cache write end
# LVM_MWCCWCOMP   36    MWCC write complete. sched pending
# LVM_CA_CLNUP    37    flush MWCC cache
# LVM_CA_TERM     38    logical (cached) request finis
# LVM_MVHLD       39    cache hold queue to pending
# LVM_SCHEDAVOID  40    scheduler called afer avoidance

+105 1.0 "@LVM EVENTS" \
        $HD, \
        0x03 { `LVM relocingblk:    pbp=$D1 pblock=$D2 relblock=$D3` },\
        0x04 { `LVM oldbadblock:    pbp=$D1 pblock=$D3 state=$D3` \
		bflags($D2) },\
        0x05 { `LVM badblkdone:     pbp=$D1` },\
        0x06 { `LVM newbadblock:    pbp=$D1 badblock=$D4 error=$D3` \
		bflags($D2) },\
        0x07 { `LVM swreloc:        pbp=$D1 status=$D2 error=$D3 retry=$D4` },\
        0x09 { {{ $smsk = $D4 & 0xFF }} {{ $pptmp = $D4 / 0x100 }} \
		{{ $ppstate = $pptmp & 0xFF }} {{ $strk = $D4 / 0x10000 }} \
	       `LVM resyncpp:       pbp=$D1 LP=$D3 sync_trk=$strk \
		sync_msk=$smsk` bflags($D2) },\
        0x0E { `LVM open:      ` devtoname($D1) flags=$D2 },\
        0x0F { `LVM close:     ` devtoname($D1) },\
        0x10 { `LVM read:      ` devtoname($D1) ext=$D2 },\
        0x11 { `LVM write:     ` devtoname($D1) ext=$D2 },\
        0x12 { `LVM ioctl:     ` devtoname($D1) cmd=$D2 arg=$D3 },\
        0x13 { `LVM upd_bbdir:  pb=$D1 bbop=$D2 bb_pbuf flags=$D3` },\
        0x14 { `LVM bbdirend:  ` devtoname($D1) `bb_pbuf:pb:flags=$D2` \
				:b_error=$D3 },\
        0x15 { `LVM bbdirop:   ` devtoname($D1)` bb_pbuf:pb_bbop=$D2` \
				"\n               " ` origpb:pb_bbop=$D3` },\
        0x16 { `LVM bbadd:      bb_pbuf ptr=$D1 orig pb_bad ptr=$D2` \
				"\n               " \
				`bad block=$D3 relblk=$D4 status=$D5` },\
        0x17 { `LVM bbdel:      orig pb_bad ptr=$D1  bad block=$D2` },\
        0x18 { `LVM bbupd:      bad_ptr=$D1 bad_ptr:blkno=$D2` \
				"\n               " \
				relblk=$D3 status=$D4 },\
        0x19 { `LVM bbdirdone:  origpb=$D1 pb_bbstat=$D2 pb_bbop=$D3` },\
        0x1a { `LVM sa_strt:    `devtoname($D1) `pbuf=$D2 type=$D3` \
				"\n               " `vg flags=$D4` },\
        0x1b { `LVM sa_wrt:     `devtoname($D1) `vg wheel_idx=$D2` \
				` vg whl_seq_num=$D3` \
				"\n               " \
				`dev=`devtoname($D4) ` lb b_blkno=$D5` },\
        0x1c { `LVM sa_iodone:  `devtoname($D4) ` lb=$D1 b_flags=$D2` \
				"\n               " \
				`b_error=$D3 b_dev=$D4` \
				` b_blkno=$D5` },\
        0x1d { `LVM sa_cont:    `devtoname($D1)` wheel_idx=$D2 \
				whl_seq_num=$D3` \
				"\n               " ` vg flags=$D4` \
				`sa_updated=$D5` },\
        0x1e { `LVM sa_rtn:     `devtoname($D1) `pb.b_dev=$D1` \
				`pb.b_blkno=$D2` \
				"\n               " `pb.b_error=$D3` \
				`pb.b_flags=$D4 pb_lbuf=$D5` },\
        0x1f { `LVM sa_config:  `devtoname($D1) `type=$D2 vg flags=$D3` },\
        0x20 { `LVM ca_chkcach: `devtoname($D1) `b_dev=$D1 b_blkno=$D2` \
				"\n               " \
				`ltg=$D3` },\
        0x21 { `LVM ca_chkhit:  `devtoname($D1) `b_dev=$D1 b_blkno=$D2` \
				"\n               " `ltg=$D3` },\
        0x22 { `LVM ca_wrt:     `devtoname($D1) ` pvol dev=$D1` },\
        0x23 { `LVM ca_wend:    `devtoname($D2) `pb=$D1 pvol dev=$D2` \
				"\n               " \
				`pb:flags=$D3 :b_error=$D4` },\
        0x24 { `LVM mwccwcomp:  `devtoname($D2) `lb_wait=$D1` \
				"\n               " \
				`lb_wait b_dev=$D2 lb_wait b_blkno=$D3` },\
        0x25 { `LVM ca_clnup:   `devtoname($D1) },\
        0x26 { `LVM ca_term:    lb=$D1` },\
        0x27 { `LVM mvhld:      `devtoname($D1) },\
        0x28 { `LVM schedavoid: count=$D1` },\
        \*   { `LVM mode=$HD` }

#
# VMM hooks
# The PGEXCT/ASSIGN/ZFOD sequence is very common, since a process
#   pages its unitialized data (sbrk) into memory.
# The PROTEXCT fault is common when a forked process does not exec,
#   such as sh, but pages in its local copy of the parent's data segment.
#
# The vmmsibits template function interprets the state of a segment.
# type 2 working_storage and delete_in_progress are the most common
#

+1B0 1.0 "@VMM_ASSIGN (assign virtual page to a physical page)" \
{{ \
vmmsibits(sibits) \
        $sibits & 0xC4800000, \
        0x84800000 "WS _delete", \
        0x80800000 "WS", \
        \* { \
                "\n    " \
                BITFLAGS $sibits, \
                0x08000000 "delete_pending", \
                0x04000000 "delete_in_progress", \
                0x02000000 "delete_when_iodone", \
                0x01000000 "process_private", \
                0x00800000 "working_storage", \
                0x00400000 "client_segment", \
                0x00200000 "persistent_storage", \
                0x00100000 "journalled", \
                0x00080000 "log", \
                0x00040000 "deferred_update", \
                0x00020000 "system_segment", \
                0x00010000 "pta_segment", \
                0x00008000 "commit_in_progress", \
                0x00004000 "modified", \
                & 0xC0000000 0xC0000000 "(type 3)", \
                & 0xC0000000 0x80000000 "(type 2)", \
                & 0xC0000000 0x40000000 "(type 1)", \
                & 0xC0000000 0x00000000 "(type 0)" \
        } \
}} \
        `VMM page assign:      V.S=$D3.$D1 ppage=$D4` vmmsibits($D2)

+1B1 1.0 "@VMM_DELETE (delete a virtual page)" \
        `VMM page delete:      V.S=$D3.$D1 ppage=$D4` vmmsibits($D2)

+1B2 1.0 "@VMM_PGEXCT (pagefault)" \
        `VMM pagefault:        V.S=$D3.$D1            ` vmmsibits($D2)

+1B3 1.0 "@VMM_PROTEXCT (protection fault)" \
        `VMM protection fault: V.S=$D3.$D1 ppage=$D4` vmmsibits($D2)

+1B4 1.0 "@VMM_LOCKEXCT (lockmiss)" \
        `VMM lockmiss:         V.S=$D3.$D1 ppage=$D4` vmmsibits($D2)

+1B5 1.0 "@VMM_RECLAIM" \
        $HD, \
        0 { `VMM reclaim:          V.S=$D3.$D1 ppage=$D4` vmmsibits($D2) }, \
        1 { `VMM ping-pong:        V.S=$D3.$D1 ppage=$D4` vmmsibits($D2) }

+1B6 1.0 @VMM_GETPARENT \
        `VMM getparent:        V.S=$D3.$D1 ppage=$D4` vmmsibits($D2)

+1B7 1.0 @VMM_COPYPARENT \
        `VMM copyparent:       V.S=$D3.$D1 ppage=$D4` vmmsibits($D2)

+1B8 1.0 "@VMM_VMAP (fault on a shared process private segment)" \
        `VMM vmapped page:     V.S=$D3.$D1 ppage=$D4` vmmsibits($D2)

+1B9 1.0 "@VMM_ZFOD (zero fill a page)" \
        `VMM zero filled page: V.S=$D3.$D1 ppage=$D4` vmmsibits($D2)

+1BA 1.0 @VMM_PAGEIO \
        `VMM start io:         V.S=$D3.$D1 ppage=$D4` vmmsibits($D2) \
        "\n    " `bp=$D5` bflags($HD) \
        vmmbufinstall($D3,$D1,$D5) \
        sidtofilename($D5) 

+1BB 1.0 "@VMM_SEGCREATE (segment create)" \
        `VMM segment creation:   S=$D1               ` vmmsibits($D2)

+1BC 1.0 "@VMM_SEGDELETE (segment delete)" \
        `VMM segment deletion:   S=$D1               ` vmmsibits($D2)

+1BD 1.0 @VMM_DALLOC \
        `VMM disk allocation:  V.S=$D3.$D1 dblk=$D5 ` vmmsibits($D2) \
        "\n    " `pdtx/devid=$D4`

+1BE 1.0 @VMM_PFEND \
        `VMM page fault end:   V.S=$D3.$D1 ppage=$D4` vmmsibits($D2) \
        "\n    " `error=$D5` bflags($HD)

+1BF 1.0 @VMM_EXCEPT \
        `VMM exception:        sregval=$D2 vaddr=$D3` vmmsibits($D1) \
        "\n    " `error=$D4 pid=$D5`

#
# Error Log
#
+1D1 1.0 ERRLG {{ $errno = $HDL }} {{ $mmdev = X2 }} {{ $cmd = X2 }} \
{{ \
errioctl1D1(cmd) \
        $cmd, \
        0x4501 "ERRIOC_STOP", \
        0x4502 "ERRIOC_SYNC", \
        \*     { `cmd=$cmd` } \
}} \
        $HDU%D1, \
        1  { erropen:  $errno%E1 }, \
        2  { errclose: $errno%E1 }, \
        3  { errioctl: $errno%E1 devtoname($D1) errioctl1D1($cmd) }, \
        4  { errread:  bad erec_length: $D1 bytes }, \
        5  { errread:  $errno%E1 }, \
        6  { errwrite  $errno%E1 }, \
        7  { errput }, \
        8  { `errput: buffer overflow: state=$errno` }, \
        9  { `errdd:  lockl from $errno already locked by $D1%P4` }, \
        10 { `errdd:  unlockl from $errno. not locked` }, \
        16 { `errdemon: cannot write to error log. error id=$D1` }, \
        \* { `ERRLG: unknown trace mode $HDU` }

#
# DUMP
#
+1D2 1.0 DUMP {{ $errno = $HDL }} {{ $mmdev = X2 }} {{ $cmd = X2 }} \
{{ \
dumpop1D2(op) \
        $op, \
        1 "DUMPINIT", \
        2 "DUMPSTART", \
        3 "DUMPWRITE", \
        4 "DUMPEND", \
        5 "DUMPTERM", \
        6 "DUMPQUERY", \
        \* { `dumpop: op=$op` } \
}} \
{{ \
dmpioctl1D2(cmd) \
        $cmd, \
        0x6401 "DMPSET_PRIM", \
        0x6402 "DMPSET_SEC", \
        0x6404 "DMPNOW_PRIM", \
        0x6405 "DMPNOW_SEC", \
        \*     { `dmpioctl: cmd=$cmd` } \
}} \
        $HDU%D1, \
        1  { dmpopen: $errno%E1 devtoname($D1) }, \
        3  { dmpioctl: $errno%E1 devtoname($D1) dmpioctl1D2($cmd) }, \
        4  { dmpdump: dumpop1D2($HDL) devtoname($D1) }, \
        5  { dmpadd: calling func is approxlookup($D1) }, \
        6  { dmpadd: return: $errno%E1 }, \
        7  { dmpdel: calling func is approxlookup($D1) }, \
        8  { dmpdel: $errno%E1 }, \
        9  { dmp_do: $HDL, 1 PRIMARY, 2 SECONDARY }, \
        10 { dmp_do: return:$errno%E1 }, \
        11 { dmpwrcdt: ptr=$D1 length=D2 }, \
        13 { dump_op: return: $errno%E1 }, \
        14 { dmpnull:  dumpop1D2($HDL) }, \
        15 { dmpfile:  dumpop1D2($HDL) }, \
        \* { `DUMP: unknown trace mode $HDU` }

+201 1.1 L=KERN @KERN_HFT G8 A25 "devno " A5 \n \t\t\t " channel " A3 \n
+202 1.1 L=KERN @KERN_KTSM G8 A25 "minor_number " A5 \n

# DD format is:
#       hookword
#               16 bits hook id
#               8 bits location code
#               8 bits errno value
#       first data word
#               32 bits devno
# rest of data words depend on trace location

# trace for parallel printer DD
# additional information is printed based on loc code, so save the loc
# this is done by assigning to the macro var and then backing up (R1).
# changed write open to 5 byte, added line to handle extra variables.
+1C8 1.0 "PPDD" \
        {{ $loc = X1 }} R1 \
        $DD1 \
        $loc, \
        01  { "flags:" X4 \
                } , \
        07  { "resid:" D4 "iovcnt:" D4 "offset:" X4 "fmode:" X4 \
                }


+1CA 1.0 "TAPEDD" \
        {{ $loc = X1 }} R1 \
        $DD1 \
        $loc, \
        19  { "command:" X4 "baddress:" X4 "bcount:" X4 \
                } , \
        1B  { "command:" X4 "baddress:" X4 "bcount:" X4 \
                }

# TRC_RNOT (0x724E4F54) /* cio_proc_recv -- kernel proc passed data    */
# TRC_RQUE (0x72515545) /* cio_proc_recv -- read data queued for user  */

+1CF 1.0 "C327DD" \
        {{ $loc = X1 }} R1 \
        $DD1 \
        $loc, \
        03  {  "chan:" X4 \
                } , \
        05  { "uiop:" X4 "chan:" X4 "ext:" X4 \
                } , \
        07  { "uiop:" X4 "chan:" X4 "ext:" X4 \
                } , \
        0B  { "event:" X4 "chan:" X4 \
                } , \
        11  { "name:" X4 "chan:" X4 \
                } , \
        21  { "ID:" O1 A4.8     \
                "d1=" X4 " d2=" X4 " d3=" X4 " d4=" X4  \
                }

+220 1.0 "FDDD" \
        $DD1

+223 1.0 "SCSIDD" \
        $DD1

+222 1.0 "BADISKDD" \
        $DD1

+221 1.0 "SCDISKDD" \
        $HDU, \
        0x0F { strategy: bp=$D2 starttimer(0x221,$D2) }, \
        0x1E { `coalesce: (bp,sc_bp)=($D2,$D3)` }, \
        0x1F { iodone: devtoname($D1) bp=$D2 buftofilename($D2) \
                endtimer(0x221,$D2) }, \
        0x17 { $D2, 0 { $SKIP } \
                bstart: devtoname($D1) bp=$D2 buftofilename($D2) \
                        pblock=$D4 bcount=$D5 bflags($D3) \
        }, \
        \*   { $DD1 }

+228 1.0 "SERDASDD" \
        $HDU, \
        0x0F { strategy: bp=$D2 starttimer(0x228,$D2) }, \
        0x1E { `coalesce: (bp,cmd)=($D2,$D3)` }, \
        0x1F { iodone: devtoname($D1) bp=$D2 buftofilename($D2) \
                endtimer(0x228,$D2) }, \
        0x17 { $D2, 0 { $SKIP } \
                bstart: devtoname($D1) bp=$D2 buftofilename($D2) \
                        pblock=$D4 bcount=$D5 bflags($D3) \
        }, \
        \*   { $DD1 }

+229 1.0 "TMSCSIDD" \
        $DD1

+226 1.0 "GIODD" \
        {{ $loc = X1 }} R1 \
        $DD1 \
        $loc, \
        03  {  "chan:" X4 \
                } , \
        05  { "uiop:" X4 "chan:" X4 "ext:" X4 \
                } , \
        07  { "uiop:" X4 "chan:" X4 "ext:" X4 \
                } , \
        0B  { "event:" X4 "chan:" X4 \
                } , \
        11  { "name:" X4 "chan:" X4 \
                } , \
        21  { "ID:" O1 A4.8     \
                "d1=" X4 " d2=" X4 " d3=" X4 " d4=" X4  \
                }

+280 1.0 "HIADD" \
   {{ $loc = X1 }} \
   $loc, \
   21  { "ID:" O1 A4.8  \
      "d1=" X4 " d2=" X4 " d3=" X4 " d4=" X4 \
      }

+1D0 0.0 DDSPEC_GRAPHIO            $DEFAULT

# DD subroutine to display standard locations
+DD1 1.0 "" \
        {{ $loc = X1 }} \
        $loc, \
        0x1  { "entry_open:" \
                $DD2 "rwflag:" X4 "chan:" X4 "ext:" X4 \
                } , \
        0x2  { "exit_open:" $DD2 } , \
        0x3  { "entry_close:" $DD2 } , \
        0x4  { "exit_close:" $DD2 } , \
        0x5  { "entry_read:" $DD2 } , \
        0x6  { "exit_read:" $DD2 } , \
        0x7  { "entry_write:" $DD2 } , \
        0x8  { "exit_write:" $DD2 } , \
        0x9  { "entry_ioctl:" \
                $DD2 "op:" X4 "flag:" X4 "chan:" X4 "ext:" X4 \
                } , \
        0xA  { "exit_ioctl:" $DD2 } , \
        0xB  { "entry_select:" $DD2 } , \
        0xC  { "exit_select:" $DD2 } , \
        0xD  { "entry_config:" \
                $DD2 "op:" X4 \
                } , \
        0xE  { "exit_config:" $DD2 } , \
        0xF  { "entry_strategy:" \
                $DD2 "bp:" X4 "flags: "X4 "block:" D4 "bcount:" D4 \
                } , \
        10  { "exit_strategy:" $DD2 $SKIP } , \
        11  { "entry_mpx:" $DD2 } , \
        12  { "exit_mpx:" \
                $DD2 "name:" X4 "chan:" D4 "oflag:" X4 \
                } , \
        13  { "entry_revoke:" $DD2 } , \
        14  { "exit_revoke:" $DD2 } , \
        15  { "entry_intr:" $DD2 } , \
        16  { "exit_intr:" $DD2 } , \
        17  { "entry_bstart:" $DD2 \
                        bp=$D2 pblock=$D4 bcount=$D5 bflags($D3) }, \
        18  { "exit_bstart:" $DD2 $SKIP} , \
        19  { "entry_cstart:" $DD2 } , \
        1A  { "exit_cstart:" $DD2 } , \
        1B  { "entry_iodone:" $DD2 $SKIP } , \
        1C  { "exit_iodone:" $DD2 $SKIP } , \
        1D  { "scsi_intr:" \
                $DD2 "sc_bufp:" X4 \
                }, \
        1E  { `coalesce: (bp,sc_bp)=($D2,$D3)` }, \
        1F  { iodone: devtoname($D1) bp=$D2 endtimer(0x221,$D2) }, \
        20  { "performance:" "flag:" X1 "ID:" X4 }

# DD subroutine to display errno & devno

+DD2 1.0 "" \
        "errno:" X1 "devno:" X4

#
# timer system calls
#

+121 1.0 L=SVC "@SYSC_ACCT" acct fname=$D1

+1A7 1.0 L=SVC "@GETPRIORITY SYSTEM CALL" \
        {{ $which = $D1 }} {{ $who = $D2 }} \
        getpriority \
        $which, \
        0 { \
                "of process" \
                $who, \
                0  { prpid($PID) $PID%P4 }, \
                \* { prpid($who) $who%P4 } \
        }, \
        1 { \
                "of process group" \
                $who, \
                0  { prpid($PID) $PID%P4 }, \
                \* { prpid($who) $who%P4 } \
        }, \
        2 { \
                "of uid" \
                $who, \
                0  { "(current process)" }, \
                \* { $who%D1 } \
        }

#1A8 1.0 L=SVC "@SETPRIORITY SYSTEM CALL" \
#       setpriority $D1%P4 to $D2%D1

+1A8 1.0 L=SVC "@SETPRIORITY SYSTEM CALL" \
        {{ $which = $D1 }} {{ $who = $D2 }} {{ $nice = $D3 }} \
        setpriority  \
        $which, \
        0 { \
                "of process" \
                $who, \
                0  { prpid($PID) $PID%P4 }, \
                \* { prpid($who) $who%P4 } \
        }, \
        1 { \
                "of process group" \
                $who, \
                0  { prpid($PID) $PID%P4 }, \
                \* { prpid($who) $who%P4 } \
        }, \
        2 { \
                "of uid" \
                $who, \
                0  { "(current process)" }, \
                \* { $who%D1 } \
        } \
        "to" $nice%D1

#
# We have a name space shortage so I have a general subroutine 400
# which looks at $func to determine what function it should perform.
# These functions are listed as: 0 - print out dev number using data
# pointer, 1 - print out common routine stuff, 2 - print out
# open/ioctl "mode" argument.
#
# Because of limitations in who trcrpt works, every stanza which calls
# this stanza should have the "intro" to declare the variables.
# variables are positional so they should be predeclared in the same
# order to get inter-stanza stuff to work right.
#
#
#+400 1.0 "@ttyfmt" \
#        {{ $maj }} {{ $min }} {{ $chan }} {{ $how }} {{ $which }} {{ $name }} \
#        O1 {{ $how = B0.2 }} {{ $which = B0.6 }} \
#        {{ $maj = $D1 / 65536 }} \
#        {{ $min = $maj * 65536 }} {{ $min = $D1 - $min }} \
#        {{ $chan = $D2 }} \
#        "("$maj%X2","$min%X2","$chan%X2")" $name \
#        $which, \
#        0000 "config ", \
#        0001 "alloc ", \
#        0002 "open ", \
#        0003 "close ", \
#        0004 "read ", \
#        0005 "write ", \
#        0006 "ioctl ", \
#        0007 "select ", \
#        0008 "revoke ", \
#        0009 "mpx ", \
#        000A "input ", \
#        000B "output ", \
#        000C "service ", \
#        000D "proc ", \
#        000E "slih ", \
#        000F "offlevel " \
#        $how, \
#        0000 { $which, \
#            0000 { "cmd" $D3%D1 }, \
#            0001 { "cin" $D3 "cmd" $D4, \
#                0000 "push", \
#                0001 "pop", \
#                0002 "unconfig" }, \
#            0002 { "mode:" dopenflags($D3) ", ext" $D4%X4 }, \
#            0006 { "cmd" ioctlname($D3) \
#                   " arg" $D4%X4 "mode:" dopenflags($D5) }, \
#            0007 { "events:" BITFLAGS $D3, \
#                0x0001 "in", \
#                0x0002 "out", \
#                0x0004 "pri", \
#                0x8000 "sync" \
#                   ", revents" $D4%X4 }, \
#            0008 { "flag" $D3%D1 }, \
#            000A { "c" $D3%X1 $D4, \
#                0000 "good char" , \
#                0001 "overrun" , \
#                0002 "parity error" , \
#                0003 "framing error" , \
#                0004 "break interrupt" , \
#                0005 "cts on" , \
#                0006 "cts off" , \
#                0007 "dsr on" , \
#                0008 "dsr off" , \
#                0009 "ri on" , \
#                000A "ri off" , \
#                000B "cd on" , \
#                000C "cd off" , \
#                000D "cblock buf" , \
#                000E "other buf" }, \
#            000C { $D3, \
#                0000 { "proc" $D4, \
#                    0000 "output" , \
#                    0001 "suspend" , \
#                    0002 "resume" , \
#                    0003 "block" , \
#                    0004 "unblock" , \
#                    0005 "rflush" , \
#                    0006 "wflush" }, \
#                0001 { "set control" BITFLAGS $D4, \
#                        0x01 "TSDTR", \
#                        0x02 "TSRTS", \
#                        0x04 "TSCTS", \
#                        0x08 "TSDSR", \
#                        0x10 "TSRI ", \
#                        0x20 "TSCD " }, \
#                0002 { "get control" }, \
#                0003 { "get status" }, \
#                0004 { "sbaud" $D4%D4 }, \
#                0005 { "get baud" }, \
#                0006 { "set input baud" $D4%D4 }, \
#                0007 { "get input baud" }, \
#                0008 { "set bpc" {{ $dog = $D4 + 5 }} $dog%D4 }, \
#                0009 { "get bpc" }, \
#                000A { "set parity" $D4, \
#                    0000 "none", \
#                    0001 "odd", \
#                    0002 "mark", \
#                    0003 "even", \
#                    0004 "space" }, \
#                000B { "get parity" }, \
#                000C { "set stops" $D4, 0 "1", 1 "2" }, \
#                000D { "get stops" }, \
#                000E { "set break" }, \
#                000F { "clear break" }, \
#                0010 { "open" $D4, 0 "local", 1 "remote" }, \
#                0011 { "dopace" $D4, \
#                    0000 "again" , \
#                    0001 "xon" , \
#                    0002 "str" , \
#                    0003 "dtr" , \
#                    0004 "rts" }, \
#                0012 { "softpace" $D4, \
#                    0000 "remote off" , \
#                    0001 "remote any" , \
#                    0002 "remote on" , \
#                    0003 "remote str" , \
#                    0004 "local off" , \
#                    0005 "local on" , \
#                    0006 "local str" }, \
#                0013 { "softrchar" $D4 }, \
#                0014 { "softlchar" $D4 }, \
#                0015 { "softrstr" $D4 }, \
#                0016 { "softlstr" $D4 }, \
#                0017 { "hardrbits" $D4 }, \
#                0018 { "hardlbits" $D4 }, \
#                0019 { "loop" $D4, 0 "enter", 1 "exit" } }, \
#            000D { $D3, \
#                0000 "output" , \
#                0001 "suspend" , \
#                0002 "resume" , \
#                0003 "block" , \
#                0004 "unblock" , \
#                0005 "rflush" , \
#                0006 "wflush" }, \
#            000E { "intr" $D3 }, \
#            000F { "intr" $D3 } } \
#{{ \
#ttyreturn(which) \
#    $which, \
#    000B { $D3, 0000 "done ", \* "more output" }, \
#    000E { $D3, 0000 "serviced ", \* "no intr serviced" }, \
#    \* { "ret" $D3%E4 } \
#    "@" $D4%D4 \
#}} \
#{{ \
#posixcflag(flags) \
#    BITFLAGS $flags, \
#    & 0x0000000f 0x00000000 "B0", \
#    & 0x0000000f 0x00000001 "B50", \
#    & 0x0000000f 0x00000002 "B75", \
#    & 0x0000000f 0x00000003 "B110", \
#    & 0x0000000f 0x00000004 "B134", \
#    & 0x0000000f 0x00000005 "B150", \
#    & 0x0000000f 0x00000006 "B200", \
#    & 0x0000000f 0x00000007 "B300", \
#    & 0x0000000f 0x00000008 "B600", \
#    & 0x0000000f 0x00000009 "B1200", \
#    & 0x0000000f 0x0000000a "B1800", \
#    & 0x0000000f 0x0000000b "B2400", \
#    & 0x0000000f 0x0000000c "B4800", \
#    & 0x0000000f 0x0000000d "B9600", \
#    & 0x0000000f 0x0000000e "B19200", \
#    & 0x0000000f 0x0000000f "B38400", \
#    & 0x00000030 0x00000000 "CS5", \
#    & 0x00000030 0x00000010 "CS6", \
#    & 0x00000030 0x00000020 "CS7", \
#    & 0x00000030 0x00000030 "CS8", \
#    0x00000040 "CSTOPB", \
#    0x00000080 "CREAD", \
#    0x00000100 "PARENB", \
#    0x00000200 "PARODD", \
#    0x00000400 "HUPCL", \
#    0x00000800 "CLOCAL", \
#    & 0x000f0000 0x00010000 "Input B50", \
#    & 0x000f0000 0x00020000 "Input B75", \
#    & 0x000f0000 0x00030000 "Input B110", \
#    & 0x000f0000 0x00040000 "Input B134", \
#    & 0x000f0000 0x00050000 "Input B150", \
#    & 0x000f0000 0x00060000 "Input B200", \
#    & 0x000f0000 0x00070000 "Input B300", \
#    & 0x000f0000 0x00080000 "Input B600", \
#    & 0x000f0000 0x00090000 "Input B1200", \
#    & 0x000f0000 0x000a0000 "Input B1800", \
#    & 0x000f0000 0x000b0000 "Input B2400", \
#    & 0x000f0000 0x000c0000 "Input B4800", \
#    & 0x000f0000 0x000d0000 "Input B9600", \
#    & 0x000f0000 0x000e0000 "Input B19200", \
#    & 0x000f0000 0x000f0000 "Input B38400", \
#    0x00100000 "PAREXT" \
#}}
#
#+401 1.0 "@common tty" \
#        {{ $maj }} {{ $min }} {{ $chan }} {{ $how }} {{ $which }} \
#        {{ $name = "tty" }} \
#        $400 \
#        $which, \
#        0010 "ttyinit ", \
#        0011 "ttyfree ", \
#        0012 "ttynull ", \
#        0013 "ttcwait ", \
#        0014 "ttspgrp ", \
#        0015 "ttypath ", \
#        0016 "stack ctl ", \
#        0017 "unstack ctl ", \
#        0018 "getctlbytype ", \
#        0019 "getctlbyname ", \
#        001A "getdispbyname ", \
#        001B "getdispbytype ", \
#	001C "dispadd ", \
#	001D "dispdel ", \
#	001E "tty_do_offlevel ", \
#	001F "ttyofflevel " \
#        $how, \
#        0000 { $which, \
#            0010 { "disp" $D3%X4 }, \
#            0013 { $D3%P4 }, \
#            0015 { $D3, 0 "input", 1 "output", 2 "service" $D4%X4 }, \
#            0016 { "disp" $D3%X4 "mode" dopenflags($D4) "ext" $D5%X4 }, \
#            0017 { "ctl" $D3%X4 "ext" $D4%X4 }, \
#            0018 { "type" $D3%D4 }, \
#            0019 { "name" $D3%X4 }, \
#            001A { "name" $D3%X4 }, \
#            001B { "type" $D3%D4 } }, \
#        0002 { $which, \
#            0018 { "ctl" $D3%X4 @ $D4%D4 }, \
#            0019 { "ctl" $D3%X4 @ $D4%D4 }, \
#            001A { "disp" $D3%X4 @ $D4%D4 }, \
#            001B { "disp" $D3%X4 @ $D4%D4 }, \
#            \* { ttyreturn($which) } }
#
#+402 1.0 "@pty" \
#        {{ $maj }} {{ $min }} {{ $chan }} {{ $how }} {{ $which }} \
#        {{ $name = "pty" }} \
#        $400 \
#        $which, \
#        0010 "ptycreate ", \
#        0011 "ptcwakeup " \
#        $how, \
#        0000 { $which, \
#            0011 { "flag" dopenflags($D3) } }, \
#        0002 { ttyreturn($which) }
#
#+403 1.0 "@rs" \
#        {{ $maj }} {{ $min }} {{ $chan }} {{ $how }} {{ $which }} \
#        {{ $name = "rs" }} \
#        $400 \
#        $which, \
#        0010 "add ", \
#        0011 "delete ", \
#        0012 "nslih ", \
#        0013 "8slih ", \
#        0014 "RT8slih ", \
#        0015 "RT4slih ", \
#        0016 "RT4detect " \
#        $how, \
#        0000 { $which, \
#            0010 { "type" $D3%D4 }, \
#            0011 { "type" $D3%D4 }, \
#            0012 { "intr" $D3%X4 }, \
#            0013 { "intr" $D3%X4 }, \
#            0014 { "intr" $D3%X4 }, \
#            0015 { "intr" $D3%X4 }, \
#            0016 { "id_ptr" $D3%X4 } }, \
#        0002 { $which, \
#            0012 { ttyreturn(14) }, \
#            0013 { ttyreturn(14) }, \
#            0014 { ttyreturn(14) }, \
#            0015 { ttyreturn(14) }, \
#            \* { ttyreturn($which) } }
#
#+404 1.0 "@lion" \
#        {{ $maj }} {{ $min }} {{ $chan }} {{ $how }} {{ $which }} \
#        {{ $name = "lion" }} \
#        $400 \
#        $which, \
#        0010 "add ", \
#        0011 "del " \
#        $how, 0002 { ttyreturn($which) }
#
#+405 1.0 "@hft" \
#        {{ $maj }} {{ $min }} {{ $chan }} {{ $how }} {{ $which }} \
#        {{ $name = "hft" }} \
#        $400 \
#        $how, 0002 { ttyreturn($which) }
#
#+406 1.0 "@rts" \
#        {{ $maj }} {{ $min }} {{ $chan }} {{ $how }} {{ $which }} \
#        {{ $name = "rts" }} \
#        $400 \
#        $how, 0002 { ttyreturn($which) }
#
#+407 1.0 "@xon" \
#        {{ $maj }} {{ $min }} {{ $chan }} {{ $how }} {{ $which }} \
#        {{ $name = "xon" }} \
#        $400 \
#        $how, 0002 { ttyreturn($which) }
#
#+408 1.0 "@dtr" \
#        {{ $maj }} {{ $min }} {{ $chan }} {{ $how }} {{ $which }} \
#        {{ $name = "dtr" }} \
#        $400 \
#        $how, 0002 { ttyreturn($which) }
#
#+409 1.0 "@dtropen" \
#        {{ $maj }} {{ $min }} {{ $chan }} {{ $how }} {{ $which }} \
#        {{ $name = "dtropen" }} \
#        $400 \
#        $how, 0002 { ttyreturn($which) }
#
#+40a 1.0 "@att" \
#        {{ $maj }} {{ $min }} {{ $chan }} {{ $how }} {{ $which }} \
#        {{ $name = "att" }} \
#        $400 \
#        $which, \
#        0010 "nput ", \
#        0011 "canon ", \
#        0012 "hard2cflag ", \
#        0013 "cflag2hard ", \
#        0014 "wait ", \
#        0015 "getrawtimer ", \
#        0016 "getdelaytimer ", \
#        0017 "npt ", \
#        0018 "1put ", \
#        0019 "flush ", \
#        001A "restart ", \
#        001B "timer ", \
#        001C "timeo "\
#        $how, \
#        0000 { $which, \
#            0010 { "cp" $D3%X4 "chrcnt" $D4%D4 }, \
#            0011 { "mode" dopenflags($D3) }, \
#            0015 { $D3, 0000 "release ", 0001 "acquire" }, \
#            0016 { $D3, 0000 "release ", 0001 "acquire" }, \
#            0017 { "c" $D3%X1 }, \
#            0018 { "c" $D3%X1 "page" $D4%D1 }, \
#            0019 { "modes:" dopenflags($D3) } }, \
#        0002 { $which, \
#            0012 { posixcflag($D3) }, \
#            \* { ttyreturn($which) } }
#
#+40b 1.0 "@berk" \
#        {{ $maj }} {{ $min }} {{ $chan }} {{ $how }} {{ $which }} \
#        {{ $name = "bsd" }} \
#        $400 \
#        $which, \
#        0010 "wflush ", \
#        0011 "wait ", \
#        0012 "nread ", \
#        0013 "oldinput ", \
#        0014 "flush ", \
#        0015 "block ", \
#        0016 "restart ", \
#        0017 "pend ", \
#        0018 "oldoutput ", \
#        0019 "rub ", \
#        001A "rubo ", \
#        001B "retype ", \
#        001C "echo ", \
#        001D "out ", \
#        001E "wakeup " \
#        $how, \
#        0000 { $which, \
#            0013 { "c" $D3%X1 }, \
#            0014 { "mode:" dopenflags($D3) }, \
#            0018 { "c" $D3%X1 }, \
#            0019 { "c" $D3%X1 }, \
#            001A { "cnt" $D3%D4 }, \
#            001C { "c" $D3%X1 }, \
#            001D { "cp" $D3%X4 } }, \
#        0002 { $which, \
#            0012 { $D3%D1 "to read @" $D4%D1 }, \
#            \* { ttyreturn($which) } }
#
#+40c 1.0 "@posix" \
#        {{ $maj }} {{ $min }} {{ $chan }} {{ $how }} {{ $which }} \
#        {{ $name = "posix" }} \
#        $400 \
#        $which, \
#        0010 "wait ", \
#        0011 "nread ", \
#        0012 "hard2cflag ", \
#        0013 "oldinput ", \
#        0014 "flush ", \
#        0015 "block ", \
#        0016 "restart ", \
#        0017 "pend ", \
#        0018 "oldoutput ", \
#        0019 "rub ", \
#        001A "rubo ", \
#        001B "retype ", \
#        001C "echo ", \
#        001D "wakeup ", \
#        001E "timeo ", \
#        001F "timer " \
#        $how, \
#        0000 { $which, \
#            0013 { "c" $D3%X1 }, \
#            0014 { "mode:" dopenflags($D3) }, \
#            0018 { "c" $D3%X1 }, \
#            0019 { "c" $D3%X1 }, \
#            001A { "cnt" $D3%D4 }, \
#            001C { "c" $D3%X1 } }, \
#        0002 { $which, \
#            0011 { $D3%D1 "to read @" $D4%D1 }, \
#            0012 { posixcflag($D3) "@" $D4%D1 }, \
#            \* { ttyreturn($which) } }
#
#+40d 1.0 "@sptr" \
#        {{ $maj }} {{ $min }} {{ $chan }} {{ $how }} {{ $which }} \
#        {{ $name = "sptr" }} \
#        $400 \
#        $which, \
#        0010 "sleep ", \
#        0011 "sendlist ", \
#        0012 "sendff ", \
#        0013 "watchdog ", \
#        0014 "prnformat ", \
#        0015 "hard2cflag " \
#        $how, \
#        0000 { $which, \
#            0010 { "pid" $D3%D4 } }, \
#        0002 { ttyreturn($which) }
#
#+40e 1.0 "@nls" \
#        {{ $maj }} {{ $min }} {{ $chan }} {{ $how }} {{ $which }} \
#        {{ $name = "nls" }} \
#        $400 \
#        $how, 0002 { ttyreturn($which) }
#
#+40f 1.0 "@wtopen" \
#        {{ $maj }} {{ $min }} {{ $chan }} {{ $how }} {{ $which }} \
#        {{ $name = "wtopen" }} \
#        $400 \
#	$which, \
#	0010 "start ", \
#	0011 "closeit ", \
#	0012 "dsr fell ", \
#	0013 "dsr rose ", \
#	0014 "tmrget " \
#        $how, \
#	0000 { $which, \
#	    0014 { "timer" $D3%X4 "f" $D4%X4 } }, \
#	0002 { ttyreturn($which) }
#
#410 1.0 "@mockernut" \
#        {{ $maj }} {{ $min }} {{ $chan }} {{ $how }} {{ $which }} \
#        {{ $name = "mockernut" }} \
#        $400 \
#        $which, \
#        0010 "wmove ", \
#        0011 "event ", \
#        0012 "input ", \
#        0013 "download ", \
#        0014 "service ", \
#        0015 "param ", \
#        0016 "cmdb ", \
#        0017 "cmdw ", \
#        0018 "add ", \
#        0019 "del " \
#        $how, \
#        0000 { $which, \
#            0010 { "count" $D3 }, \
#	    0011 { "card" $D3 }, \
#            0013 { "card" $D3 }, \
#            0014 { "cmd" $D3 }, \
#            0015 { "dev" $D3 }, \
#            0016 { status($D3) "byte1" $D4%X1 "byte2" $D4%X1 }, \
#            0017 { status($D3) "word" $D4%X2 }, \
#            0018 { "type" $D3 } }, \
#        0001 { $which, \
#            0002 { $D3, \
#                0000 "ch_cflag" , \
#                0001 "ch_iflag" , \
#                0002 "un_oflag" , \
#                0003 "un_lflag" , \
#                0004 "waiting for carrier" \
#                        $D4%X2 }, \
#            0005 { "writing" $D3, \
#                0000 "chars to wrap" , \
#                0001 "remaining chars" \
#                        $D4%X2 }, \
#            0006 { $D3, \
#                0000 "ch_cflag" , \
#                0001 "ch_iflag" , \
#                0002 "un_oflag" , \
#                0003 "un_lflag" \
#                        $D4%X2 }, \
#            000C { $D3, \
#                0000 "ch_cflag" , \
#                0001 "ch_iflag" , \
#                0002 "un_oflag" , \
#                0003 "un_lflag" \
#                        $D4%X2 }, \
#	   0010 { "writing" $D3, \
#                0000 "chars to wrap", \
#                0001 "remaining chars" \
#                        $D4%X2 }, \
#            0015 { $D3, \
#                0000 "ch_cflag" , \
#                0001 "ch_iflag" , \
#                0002 "un_oflag" , \
#                0003 "un_lflag" , \
#                0004 "ch_cook" \
#                        $D4%X2 } }, \
#        0002 { ttyreturn($which) } \
#{{ \
#status(cmd) \
#        $cmd, \
#        0x80 "SRLOW ", \
#        0x81 "SRHIGH ", \
#        0x82 "FLUSHTX ", \
#        0x83 "PAUSETX ", \
#        0x84 "RESUMETX ", \
#        0x85 "SMINT ", \
#        0x86 "SAFLOWC ", \
#        0x87 "", \
#        0x88 "SBREAK ", \
#        0x89 "SMODEM ", \
#        0x8A "SIFLAG ", \
#        0x8B "SFLOWC ", \
#        0x8C "STLOW ", \
#        0x8E "RPAUSE ", \
#        0x8F "RRESUME ", \
#        0x92 "BUFSETALL ", \
#        0x93 "SOFLAG ", \
#        0x94 "SHFLOW ", \
#        0x95 "SCFLAG ", \
#	0x96 "SVNEXT " \
#}}

#
# Streams TTY format routine
#
# There is a general subroutine 400 to handle most of the tty functions
# except:
#	0008 - revoke which is handled by the sth individual stanza
#	0009 - ioctl which is handled by each individual module stanza
#	000E - "last" or generic case; to be added by each individual stanza
#		as needed
#
# Because of limitations in who trcrpt works, every stanza which calls
# this stanza should have the "intro" to declare the variables.
# Variables are positional so they should be predeclared in the same
# order to get inter-stanza stuff to work right.
#
+400 1.0 "@STTY " \
        {{ $maj }} {{ $min }} {{ $how }} {{ $which }} {{ $name }} \
        O1 {{ $how = B0.2 }} {{ $which = B0.6 }} \
        {{ $maj = $D1 / 65536 }} \
        {{ $min = $maj * 65536 }} {{ $min = $D1 - $min }} \
        "("$maj%X2","$min%X2")" $name \
        $which, \
        0001 "config ", \
        0002 "open ", \
        0003 "close ", \
        0004 "wput ", \
        0005 "rput ", \
        0006 "wsrv ", \
        0007 "rsrv ", \
        000A "proc ", \
        000B "service ", \
        000C "slih ", \
        000D "offlevel " \
        $how, \
        0000 { $which, \
            0001 { "cmd" $D3, \
		0x1 "CFG_INIT", \
		0x2 "CFG_TERM", \
		0x3 "CFG_QVPD", \
		0x4 "CFG_UCODE", \
		\* { $D3%X4 } \
		}, \
            0002 { "ptr" $D2%X4 "mode:" dopenflags($D3) \
                   "sflag:" $D4, \
                0000 "0", \
                0001 "MODOPEN", \
                0002 "CLONEOPEN", \
		\* { $D4 } }, \
            0003 { "ptr" $D2%X4 "mode:" dopenflags($D3) }, \
            0004 { "ptr" $D2%X4 "@msg" $D3%X4 \
		   "msg_type" messagetype($D4) \
		 }, \
            0005 { "ptr" $D2%X4 "@msg" $D3%X4 \
		   "msg_type" messagetype($D4) \
		 }, \
            0006 { "ptr" $D2%X4 "q_count" $D3 }, \
            0007 { "ptr" $D2%X4 "q_count" $D3 }, \
            000A { "ptr" $D2%X4 \
            	   $D3, \
                0000 "output" , \
                0001 "suspend" , \
                0002 "resume" , \
                0003 "block" , \
                0004 "unblock" , \
                0005 "rflush" , \
                0006 "wflush" }, \
            000B { "ptr" $D2%X4 \
                   $D3, \
                0000 { "proc" $D4, \
                    0000 "output" , \
                    0001 "suspend" , \
                    0002 "resume" , \
                    0003 "block" , \
                    0004 "unblock" , \
                    0005 "rflush" , \
                    0006 "wflush" }, \
                0001 { "set control" BITFLAGS $D4, \
                        0x01 "TSDTR", \
                        0x02 "TSRTS", \
                        0x04 "TSCTS", \
                        0x08 "TSDSR", \
                        0x10 "TSRI ", \
                        0x20 "TSCD " }, \
                0002 { "get control" }, \
                0003 { "get status" }, \
                0004 { "sbaud" $D4%D4 }, \
                0005 { "get baud" }, \
                0006 { "set input baud" $D4%D4 }, \
                0007 { "get input baud" }, \
                0008 { "set bpc" {{ $dog = $D4 + 5 }} $dog%D4 }, \
                0009 { "get bpc" }, \
                000A { "set parity" $D4, \
                    0000 "none", \
                    0001 "odd", \
                    0002 "mark", \
                    0003 "even", \
                    0004 "space" }, \
                000B { "get parity" }, \
                000C { "set stops" $D4, 0 "1", 1 "2" }, \
                000D { "get stops" }, \
                000E { "set break" }, \
                000F { "clear break" }, \
                0010 { "open" $D4, 0 "local", 1 "remote" }, \
                0011 { "dopace" $D4, \
                    0000 "again" , \
                    0001 "xon" , \
                    0002 "str" , \
                    0003 "dtr" , \
                    0004 "rts" }, \
                0012 { "softpace" $D4, \
                    0000 "remote off" , \
                    0001 "remote any" , \
                    0002 "remote on" , \
                    0003 "remote str" , \
                    0004 "local off" , \
                    0005 "local on" , \
                    0006 "local str" }, \
                0013 { "softrchar" $D4 }, \
                0014 { "softlchar" $D4 }, \
                0015 { "softrstr" $D4 }, \
                0016 { "softlstr" $D4 }, \
                0017 { "hardrbits" $D4 }, \
                0018 { "hardlbits" $D4 }, \
                0019 { "loop" $D4, 0 "enter", 1 "exit" } }, \
            000C { "rintr" $D2%X4 "adap_type" $D3, \
		0000 "Native io", \
		0001 "8/16 Port" }, \
            000D { "rintr" $D2%X4 } } \
{{ \
stty_ret(which) \
	$which, \
        0001 { "ret" $D3 "from line" $D4  }, \
        0002 { "ret" $D3 "from line" $D4  }, \
        0003 { "ret" $D3 "from line" $D4  }, \
        0004 { "ret" $D3 "from line" $D4  }, \
        0005 { "ret" $D3 "from line" $D4  }, \
        0006 { "ret" $D3 "from line" $D4  }, \
        0007 { "ret" $D3 "from line" $D4  }, \
        0008 { "ret" $D3 "from line" $D4  }, \
        0009 { "ret" $D3 "from line" $D4  }, \
        000A { "ret" $D3 "from line" $D4  }, \
        000B { "ret" $D3 "from line" $D4  }, \
        000C { "ret" $D3 "from line" $D4  }, \
        000D { "ret" $D3 "from line" $D4  }, \
        000E { "last "      "ret" $D3 "from line" $D4  }, \
	\* { "unknown subhook" $HD }    \
}} 

+401 1.0 "@STTY STRTTY" \
        {{ $maj }} {{ $min }} {{ $how }} {{ $which }} \
        {{ $name = "sth" }} \
	$400 \
	$which, \
	0008 "revoke ", \
	0009 "ioctl " \
	$how, \
	0000 { $which, \
		0008 { "flag" $D3 }, \
		0009 { "osr" $D2%X4 "cmd" ioctlname($D3) } }, \
	0002 { $which, \
		\* { stty_ret($which) } } 

+402 1.0 "@STTY LDTERM " \
        {{ $maj }} {{ $min }} {{ $how }} {{ $which }} \
        {{ $name = "ldterm" }} \
	$400 \
	$which, \
	0009 "ioctl " \
        $how, \
        0000  { $which, \
		0009 { "ptr" $D2%X4 "cmd" ioctlname($D3) } \
		}, \
        0002 { $which, \
		\* { stty_ret($which) } } 

+403 1.0 "@STTY SPTR " \
        {{ $maj }} {{ $min }} {{ $how }} {{ $which }} \
        {{ $name = "sptr" }} \
	$400 \
	$which, \
	0009 "ioctl " \
	$how, \
	0000 { $which, \
		0009 { "ptr" $D2%X4 "cmd" ioctlname($D3) \
			"rwflag" $D4 "ext" $D5 } }, \
	0002 { $which, \
		\* { stty_ret($which) } }

+404 1.0 "@STTY NLS " \
        {{ $maj }} {{ $min }} {{ $how }} {{ $which }} \
        {{ $name = "nls" }} \
	$400 \
	$which, \
	0009 "ioctl " \
	$how, \
	0000 { $which, \
		0009 { "ptr" $D2%X4 "cmd" ioctlname($D3) } }, \
	0002 { $which, \
		\* { stty_ret($which) } }

+405 1.0 "@STTY PTY" \
        {{ $maj }} {{ $min }} {{ $how }} {{ $which }} \
        {{ $name = "pty" }} \
	$400 \
	$which, \
	0009 "ioctl " \
	$how, \
	0000 { $which, \
		0009 { "ptr" $D2%X4 "cmd" ioctlname($D3) } }, \
	0002 { $which, \
		\* { stty_ret($which) } }

+406 1.0 "@STTY RS" \
        {{ $maj }} {{ $min }} {{ $how }} {{ $which }} \
        {{ $name = "rs" }} \
	$400 \
	$which, \
	0009 "ioctl " \
	$how, \
	0000 { $which, \
		0009 { "ptr" $D2%X4 "cmd" ioctlname($D3) } }, \
	0002 { $which, \
		\* { stty_ret($which) } }

+407 1.0 "@STTY LION"                  $DEFAULT

+1A4 1.0 L=SVC "@GETRLIMIT SYSTEM CALL" \
{{ \
prresource(resource) \
$resource, \
        0 "CPU TIME", \
        1 "MAX FILE SIZE", \
        2 "DATA SEGMENT SIZE", \
        3 "SIZE SIZE", \
        4 "CORE FILE SIZE", \
        5 "RESIDENT SET SIZE" \
}} \
        getrlimit resource=$D1 prresource($D1)

+1A5 1.0 L=SVC "@SETRLIMIT SYSTEM CALL" \
        setrlimit resource=$D1 prresource($D1)

+1A6 1.0 L=SVC "@GETRUSAGE SYSTEM CALL" \
        getrusage who=$D1%D1 \
        $D1, \
        0  "of self", \
        \* "of children"

+180 2.0 L=SVC "@SIGACTION SYSTEM CALL" \
        sigaction signal prsignal($D1) \n\
	addr new sigaction=$D2%X4 addr old sigaction=$D3%X4

+18F 1.0 L=SVC "@ULIMIT SYSTEM CALL" \
        ulimit \
        $D1%D1, \
        1 { `get fsize` }, \
        2 { `set fsize to $D2` }, \
        3 { `get data limit` }, \
        1004 { `set data limit to $D2` }, \
        1005 { `get stack` }, \
        1006 { `set stack limit to $D2` }, \
        1007 { "get RAWDIR compatibility mode (REALDIR)" }, \
        1008 { \
                $D2, \
                0  { "clear RAWDIR compatibility mode (REALDIR)" }, \
                \* { "set RAWDIR compatibility mode (REALDIR)" } \
        }, \
        1009 { "get TRUNCATES compatibility mode (SYSVLOOKUP)" }, \
        1010 { \
                $D2, \
                0  { "clear TRUNCATES compatibility mode (SYSVLOOKUP)" }, \
                \* { "set TRUNCATES compatibility mode (SYSVLOOKUP)" } \
        }

+195 1.0 L=SVC "@USRINFO SYSTEM CALL" "usrinfo"

+16E 1.0 "@SETPGRP" \
        HT & 0x07, 6 { `setpgid pid=prpid($D1) pgrp=$D2%D1` }, \* "SETPRGP"

+18E 1.0 "@TIMES" \
	"times:   "	\
        G8 {{ $u = X4 }} {{ $s = $X4 }} {{ $cu = X4 }} {{ $cs = X4 }} \
        `times u=$u%D1 s=$s%D1 cu=$cu%D1 cs=$cs%D1 (ticks)`

+1A9 1.0 "@ABSINTERVAL SYSTEM CALL" \
        absinterval timerid=$D1

+1AA 1.0 "@GETINTERVAL SYSTEM CALL" \
        getinterval timerid=$D1

+1AC 1.0 "@INCINTERVAL SYSTEM CALL" \
        incinterval timerid=$D1

+1AB 1.0 "@GETTIMER SYSTEM CALL" \
        gettimer timer_type=$D1

+1F0 1.0 "@SETTIMER SYSTEM CALL" \
        settimer timer_type=$D1

+1AF 1.0 "@RESINC SYSTEM CALL" \
        resinc timer_type=$D1

+1AE 1.0 "@RESABS SYSTEM CALL" \
        resabs timer_type=$D1

+1AD 1.0 "@RESTIMER SYSTEM CALL" \
        restimer timer_type=$D1

#**********************************************************************
# Templates for trcrpt control
# The trcrpt program expects these templates to be defined in every template
# file. If they are not present, it will not be able to generate a header,
# or more importantly, the "UNDEFINED TEMPLATE" event data. Any undefined
# traceid will just get a simple "undefined" report entry.
# In any case, the trcrpt program will maintain synchronization, as long as
# the hook types are valid (0 - 7).
# A 000 traceid will use the NULL template and also complain to the stderr.

+001 1.0 "TRACE ON" channel D2 {{$PID = $D1}} {{$TID = $TID}} O4 \n T4

+002 1.0 "TRACE OFF" channel D2 \n T4

# HEADER
# skip the d1 word with a O4.
+003 1.1 L=0 "@TRACE HEADER" G8 \
        $D1, \
        2  { $HD%S1 \n\n }, \
        \* { \
\n\
T4 \n\
System:  A32 A32 O32 Node: A32 \n\
Machine: A32 \n\
Internet Address: $IPADDR%X4 sinaddr($IPADDR) \n\
\n\
S4\n }

# Inserted by the trace daemon.
+005 1.0 "LOGFILE WRAPAROUND" $HD

# Inserted by the trcdd trace device driver.
+006 1.1 "TRACEBUFFER WRAPAROUND" $HD \
        $D1, \
        0 {}, \
        \* { `$D1 missed entries` }

# traceid 007 is known by trcrpt as the "undefined traceid" template
# Print out undefined traceids as well as possible
+007 1.0 "UNDEFINED TRACE ID" idx $LOGIDX0 \
        G0 {{ $hookword = X4 }} \
        G0 {{ $traceid = B0.12 }} \
        G2 {{ $length = X2 }} \
        traceid $traceid \n\
        hookword $hookword type HT \n\
        $DEFAULT

# traceid 008 is known by trcrpt as the $DEFAULT template
+008 1.1 "@DEFAULT TEMPLATE" G2 {{ $length = $HD }} \
        HT & 0x07, \
        01 { hookdata X2 }, \
        02 { hookdata X2 X4 }, \
        06 { hookdata X2 X4 X4 X4 X4 X4 }, \
        00 { d1 $D1 G8 LOOP $length {X0} }

+009 1.1 "@TRACE HOOK CALIBRATION" \
        HT & 0x07, \
        1  { trchk  }, \
        2  { trchkl }, \
        6  { trchkg }, \
        0  { trcgen }, \
        \* { type is HT }

#
# The .9 second hook is from a timeout in the trace device driver
#   so that rtc timer synchronization is not lost.
# The pidinstall hook installs process id / process name pairs.
# The hook is manually added by the trace demon by doing a   ps -e .
# The loaderinstall hook installs base address/size/filename triplets
# The hook is manually added by the trace demon by doing a TRC_LOADER
#  ioctl to the trcdd device device driver. (The TRC_LOADER ioctl
#  reads up to 100 loader_entry structures from the kernel's table
#  of kernel extensions).
# IPADDR is the Internet (129.02.13.99) address of this machine.
#
+00A 1.1 @TRACE_UTIL $NOPRINT \
        HT & 0x07, \
        0  { {{ $mode = $D1 }} }, \
        \* { {{ $mode = $HD }} } \
        $mode, \
        0 ".9 second", \
        1 { G8 pidinstall(X4,X4,A16,X4,X4,X4) {{ $PID = $D2 }} }, \
        2 { G8 loaderinstall(X4,X4,A16) }, \
        3 { {{ $PID = $D1 }} }, \
        4 { G8 deviceinstall(X4,A16) }, \
        5 { {{ $IPADDR = $D1 }} }

+10F 0.0 KERN_EOF                  $DEFAULT
+110 0.0 KERN_STDERR               $DEFAULT
+111 0.0 KERN_LOCKF                $DEFAULT
+11A 0.0 KERN_RCVSIGNAL            $DEFAULT
+11D 0.0 KERN_SIGDELIVER           $DEFAULT

+11C 1.0 L=KERN "@P_SLIH"	\
        "p_slih:   pid="$D1%D1  "tid="$D2%D1  "addr="$D3  "except="$D4%D1

+183 1.0 L=SVC "@SIGRETURN"	\
        "sigreturn:   pid="$D1%D1  "tid="$D2%D1

+119 1.0 L=KERN "@SENDSIGNAL"	\
        "pidsig:   pid="$D1%D1  "signal="prsignal($D2)  "lr="$D3

+181 1.0 "SIGCLEANUP"

# This hookid is for TCPIP network performance analysis
+250 1.0 '' \
 	$HD%D1, \
        1  { "SOCK  (entry) sendit" }, \
        2  { "SOCK  (entry) sosend" }, \
        3  { "SOCK  (entry) put data in send mbufs" }, \
        4  { "SOCK  (entry) main loop data to mbufs" }, \
        5  { `SOCK  (entry) uiomove len=$D1%D1` }, \
        6  { "SOCK  (entry) recvit" }, \
        7  { "SOCK  (entry) soreceive" }, \
        8  { "SOCK  (entry) soreceive while loop" }, \
        9  { "MBUF  (entry) m_pullup bcopy" }, \
        10 { "IP   (entry) looutput" }, \
        11 { "IP   (entry) raw_input" }, \
        12 { "IP   (entry) rip_output" }, \
        13 { "IP   (entry) in_cksum" }, \
        14 { "IP   (entry) in_pcblookup" }, \
        15 { "IP   (entry) ipintr" }, \
        16 { "IP   (entry) reassembly" }, \
        17 { "IP   (entry) ip_output" }, \
        18 { "IP   (entry) fragmentation" }, \
        19 { "TCP  (entry) reassembly" }, \
        20 { "TCP  (entry) tcp_input" }, \
        21 { "TCP  (entry) tcp_output" }, \
        22 { "TCP  (entry) send packet" }, \
        23 { "UDP  (entry) udp_input" }, \
        24 { "UDP  (entry) udp_output" }, \
        25 { "NETIF_EN   (entry) enet: en_output" }, \
        26 { "NETIF_EN  (entry) enet: en_recv" }, \
    	27 { "NETIF_TOK   (entry) token: ie5_output" }, \
        28 { "NETIF_TOK   (entry) token: ie5_recv" }, \
        29 { "NETIF_802.3   (entry) 802.3: ie3_output" }, \
        30 { "NETIF_802.3   (entry) 802.3: ie3_recv" }, \
        31 { "NETIF_X25   (entry) X.25:  xt_output" }, \
        32 { "NETIF_X25   (entry) X.25:  xt_recv" }, \
        33 { "NETIF_SLIP   (entry) slip:  sloutput" }, \
        34 { "NETIF_SLIP   (frame escape) slip:  slinput" }, \
        35 { "NETIF_SER   (entry) Net_Service: find_input_type" }, \
        101  { "SOCK  (rtn)  sendit" }, \
        102  { "SOCK  (rtn)  sosend" }, \
        103  { "SOCK  (rtn)  put data in send mbufs" }, \
        104  { "SOCK  (rtn)  main loop data to mbufs" }, \
        105  { "SOCK  (rtn)  uiomove" }, \
        106  { "SOCK  (rtn)  recvit" }, \
        107  { "SOCK  (rtn)  soreceive" }, \
        108  { "SOCK  (rtn)  soreceive while loop" }, \
        109  { "MBUF  (rtn)  m_pullup bcopy" }, \
        110 { "IP   (rtn)  looutput" }, \
        111 { "IP   (rtn)  raw_input" }, \
        112 { "IP   (rtn)  rip_output" }, \
        113 { "IP   (rtn)  in_cksum" }, \
        114 { "IP   (rtn)  in_pcblookup" }, \
        115 { "IP   (rtn)  ipintr" }, \
        116 { "IP   (rtn)  reassembly" }, \
        117 { "IP   (rtn)  ip_output" }, \
        118 { "IP   (rtn)  fragmentation" }, \
        119 { "TCP  (rtn)  reassembly" }, \
        120 { "TCP  (rtn)  tcp_input" }, \
        121 { "TCP  (rtn)  tcp_output" }, \
        122 { "TCP  (rtn)  send packet" }, \
        123 { "UDP  (rtn)  udp_input" }, \
        124 { "UDP  (rtn)  udp_output" }, \
        125 { "NETIF_EN   (rtn)  enet: en_output" }, \
        126 { "NETIF_EN   (rtn)  enet: en_recv" }, \
        127 { "NETIF_TOK   (rtn)  token: ie5_output" }, \
        128 { "NETIF_TOK   (rtn)  token: ie5_recv" }, \
        129 { "NETIF_802.3   (rtn)  802.3: ie3_output" }, \
        130 { "NETIF_802.3   (rtn)  802.3: ie3_recv" }, \
        131 { "NETIF_X25   (rtn)  X.25:  xt_output" }, \
        132 { "NETIF_X25   (rtn)  X.25:  xt_recv" }, \
        133 { "NETIF_SLIP   (rtn)  slip:  sl_output" }, \
        134 { "NETIF_SLIP   (frame end)  slip:  slinput" }, \
        135 { "NETIF_SER   (rtn) Net_Service: find_input_type" }, \
        \*  { unknown subhook $HD%D1 $ERROR }

# This hookid is for network errors 
+251 1.0 'NETERR ' \
 	$HD%D1, \
	0  {`CIO_OK ifp=$D1%X4` }, \
	1  {`CIO_BAD_MICROCODE ifp=$D1%X4` }, \
	2  {`CIO_BUF_OVFLW ifp=$D1%X4` }, \
	3  {`CIO_HARD_FAIL ifp=$D1%X4` }, \
	4  {`CIO_LOST_DATA ifp=$D1%X4` }, \
	5  {`CIO_NOMBUF ifp=$D1%X4` }, \
	6  {`CIO_NOT_STARTED ifp=$D1%X4` }, \
	7  {`CIO_TIMEOUT ifp=$D1%X4` }, \
	8  {`CIO_NET_RCVRY_ENTER ifp=$D1%X4` }, \
	9  {`CIO_NET_RCVRY_EXIT ifp=$D1%X4` }, \
	10 {`CIO_NET_RCVRY_MODE ifp=$D1%X4` }, \
	11 {`CIO_INV_CMD ifp=$D1%X4` }, \
	12 {`CIO_BAD_RANGE ifp=$D1%X4` }, \
	13 {`CIO_NETID_INV ifp=$D1%X4` }, \
	14 {`CIO_NETID_DUP ifp=$D1%X4` }, \
	15 {`CIO_NETID_FULL ifp=$D1%X4` }, \
	65 {`XT_BAD_CALL_ID ifp=$D1%X4` }, \
	66 {`XT_CLEAR ifp=$D1%X4` }, \
	67 {`XT_INV_CTR ifp=$D1%X4` }, \
	68 {`XT_NAME_USED ifp=$D1%X4` }, \
	69 {`XT_NOT_PVC ifp=$D1%X4` }, \
	70 {`XT_NO_ACK ifp=$D1%X4` }, \
	71 {`XT_NO_ACK_REQ ifp=$D1%X4` }, \
	72 {`XT_NO_LINK ifp=$D1%X4` }, \
	73 {`XT_NO_NAME ifp=$D1%X4` }, \
	74 {`XT_PROTOCOL ifp=$D1%X4` }, \
	75 {`XT_PVC_USED ifp=$D1%X4` }, \
	76 {`XT_RESET ifp=$D1%X4` }, \
	77 {`XT_TABLE ifp=$D1%X4` }, \
	78 {`XT_TOO_MANY_VCS ifp=$D1%X4` }, \
	79 {`XT_AUTH_LISTEN ifp=$D1%X4` }, \
	80 {`XT_BAD_PKT_TYPE ifp=$D1%X4` }, \
	81 {`XT_BAD_SESSION_TYPE ifp=$D1%X4` }, \
	100 {`invalid xmit complete intr ifp=$D1%X4` }, \
	101 {`find_arp_iftype() fail ifp=$D1%X4` }, \
	102 {`if_detach() fail ifp=$D1%X4` }, \
	103 {`find_input_type() fail ifp=$D1%X4` }, \
	104 {`no mbufs ifp=$D1%X4` }, \
	105 {`if not running ifp=$D1%X4` }, \
	106 {`clear indication ifp=$D1%X4` }, \
	107 {`unknown packet type ifp=$D1%X4` }, \
	108 {`NET_XMIT_FAIL ifp=$D1%X4` }, \
	109 {`NET_DETACH_FAIL ifp=$D1%X4` }, \
	110 {`ARP, wrong header ifp=$D1%X4` }, \
	111 {`ARP, unknown protocol ifp=$D1%X4` }, \
	112 {`ARP, ip broadcast address ifp=$D1%X4` }, \
	113 {`ARP, duplicate address ifp=$D1%X4` }, \
	114 {`ARP, arp table full ifp=$D1%X4` }, \
	115 { `slinput overrun error` }, \
 	116 { `slinput parity/framing error` }, \
 	117 { `slinput no frame escape/end` }, \
 	118 { `slinput slbtom rtn NULL` }, \
 	119 { `slinput ilen>slmtu` }, \
 	120 { `pdu_max < mtu+8, pdu_max=$D1%D4 mtu=$D2%D4` }, \
        \*  { unknown subhook $HD%D1 $ERROR }

+252 1.0 L=SVC "SOCK" \
{{ \
sodomain(qqq) \
	$qqq%D1, \
	0   AF_UNSPEC, \
	1   AF_UNIX, \
	2   AF_INET, \
	3   AF_IMPLINK, \
	4   AF_PUP, \
	5   AF_CHAOS, \
	6   AF_NS, \
	7   AF_ISO, \
	8   AF_ECMA, \
	9   AF_DATAKIT, \
	10  AF_CCITT, \
	11  AF_SNA, \
	12  AF_DECnet, \
	13  AF_DLI, \
	14  AF_LAT, \
	15  AF_HYLINK, \
	16  AF_APPLETALK, \
	17  AF_ROUTE, \
	18  AF_LINK, \
	19  AF_XTP, \
	20  AF_INTF, \ 
	21  AF_RIF, \
	22  AF_MAX, \
	\*  {$qqq%D1} \
}} \
{{ \
sotype(rrr) \
	$rrr%D1, \
	1   SOCK_STREAM, \
	2   SOCK_DGRAM, \
	3   SOCK_RAW, \
	4   SOCK_RDM, \
	5   SOCK_SEQPACKET, \
	\*  {$rrr%D1} \
}} \
{{ \
soproto(sss) \
	$sss%D2, \
	0   IPPROTO_IP, \
	1   IPPROTO_ICMP, \
	3   IPPROTO_GGP, \
	6   IPPROTO_TCP, \
	8   IPPROTO_EGP, \
	12  IPPROTO_PUP, \
	17  IPPROTO_UDP, \
	22  IPPROTO_IDP, \
	29  IPPROTO_TP, \
	80  IPPROTO_EON, \
	255 IPPROTO_RAW, \
	256 IPPROTO_MAX, \
	\*  {$sss%D2} \
}} \
{{ \
soflag(uuu) \
	$uuu%X4, \
	0x1   MSG_OOB, \
	0x2   MSG_PEEK, \
	0x4   MSG_DONTROUTE, \
	0x8   MSG_EOR, \
	0x10  MSG_TRUNC, \
	0x20  MSG_CTRUNC, \
	0x40  MSG_WATTALL, \
	\*  {$uuu%X4} \
}} \
{{ \
sohow(vvv) \
	$vvv%D1, \
	0   DISABLE_RECV, \
	1   DISABLE_SEND, \
	2   DISABLE_RECV&SEND, \
	\*  {$vvv%D1} \
}} \
{{ \
soopt(www) \
	$www%X4, \
	0x0001   SO_DEBUG,\
	0x0002   SO_ACCEPTCONN, \
	0x0004   SO_REUSEADDR, \
	0x0008   SO_KEEPALIVE, \
	0x0010   SO_DONTROUTE, \
	0x0020   SO_BROADCAST, \
	0x0040   SO_USELOOPBACK, \
	0x0080   SO_LINGER, \
	0x0100   SO_OOBINLINE, \
	0x1001   SO_SNDBUF, \
	0x1002   SO_RCVBUF, \
	0x1003   SO_SNDLOWAT, \
	0x1004   SO_RCVLOWAT, \
	0x1005   SO_SNDTIMEO, \
	0x1006   SO_RCVTIMEO, \
	0x1007   SO_ERROR, \
	0x1008   SO_TYPE, \
	\*  {$www%X4} \
}} \ 
 	$HD%D1, \
 	1  { `socket ` dom=sodomain($D1) type=sotype($D2) proto=soproto($D3) }, \
 	2  { `bind sofd=$D1%D4 name=$D2%X4 namelen=$D3%D4` }, \
 	3  { `listen sofd=$D1%D4 backlog=$D2%D4` }, \
 	4  { `accept sofd=$D1%D4 name=$D2%X4 namelen=$D3%D4` }, \
 	5  { `connect sofd=$D1%D4 name=$D2%X4 namelen=$D3%D4` }, \
 	6  { `socketpair ` dom=sodomain($D1) type=sotype($D2) proto=soproto($D3) `socktvect=$D4%X4 ` }, \
 	7  { `sendto sofd=$D1%D4 mesg=$D2%X4 len=$D3%D4 ` flag=soflag($D4) `to=$D5%X4 ` }, \
 	8  { `send sofd=$D1%D4 mesg=$D2%X4 len=$D3%D4 ` flag=soflag($D4) }, \
 	9  { `sendmsg sofd=$D1%D4 mesg=$D2%X4 ` flag=soflag($D3) }, \
 	10 { `recvfrom sofd=$D1%D4 buf=$D2%X4 buflen=$D3%D4 ` flag=soflag($D4) `from=$D5%X4` }, \
 	11 { `recv sofd=$D1%D4 buf=$D2%X4 buflen=$D3%D4 ` flag=soflag($D4) }, \
 	12 { `recvmsg sofd=$D1%D4 mesg=$D2%X4 ` flag=soflag($D3) }, \
 	13 { `shutdown sofd=$D1%D4 ` how=sohow($D2) }, \
 	14 { `setsockopt sofd=$D1%D4 level=$D2%D4 ` optname=soopt($D3) `optvalue=$D4%D1 optlen=$D5%D4` }, \
 	15 { `getsockopt sofd=$D1%D4 level=$D2%D4 ` optname=soopt($D3) `optvalue=$D4%D1 optlen=$D5%D4` }, \
 	16 { `getsockname sofd=$D1%D4 name=$D2%X4 namelen=$D3%D4` }, \
 	17 { `getpeername sofd=$D1%D4 name=$D2%X4 namelen=$D3%D4` }, \
 	18 { "gethostid"  }, \
 	19 { `sethostid hostid=$D1%X4` }, \
 	20 { `gethostname name=$D1%X4 namelen=$D2%D4` }, \
 	21 { `sethostname name=$D1%X4 namelen=$D2%D4` }, \
 	22 { `getdomainname name=$D1%X4 namelen=$D2%D4` }, \
 	23 { `setdomainname name=$D1%X4 namelen=$D2%D4` }, \
	24 { `return from socket sofd=$D1%D1, soaddr=$D2%X4` \
		lookuppninstall1('SOCKET') \
		lookuppninstall2($D2) fdinstall($D1)  }, \
	25 { `return from socketpair sofd1=$D1%D1, sofd2=$D2%D1, soaddr1=$D3%X4, soaddr2=$D4%X4` \
		lookuppninstall1('SOCKET') \
		lookuppninstall2($D3) fdinstall($D1) \
		lookuppninstall2($D4) fdinstall($D2) }, \
 	\* {  unknown subhook $HD $ERROR }

+253 1.0 'SOCK' \
 	$HD%D1, \
 	1  { `socreate ` dom=sodomain($D1) `soaddr=$D2%X4` type=sotype($D3) proto=soproto($D4) }, \
 	2  { `sobind soaddr=$D1%X4 name=$D2%X4` }, \
 	3  { `solisten soaddr=$D1%X4 backlog=$D2%D1` }, \
 	4  { `sofree soaddr=$D1%X4` }, \
 	5  { `soclose soaddr=$D1%X4` }, \
 	6  { `return from soclose soaddr=$D1%X4` }, \
 	7  { `soabort soaddr=$D1%X4` }, \
 	8  { `soaccept soaddr=$D1%X4 name=$D2%X4` }, \
 	9  { `return from soaccept error=$D1%D1` }, \
 	10 { `soconnect soaddr=$D1%X4 name=$D2%X4` }, \
 	11 { `soconnect2 soaddr1=$D1%X4 soaddr2=$D2%X4` }, \
 	12 { soconnect2_out }, \
 	13 { `sodisconnect soaddr=$D1%X4` }, \
 	14 { `return from sodisconnect error=$D1%D1` }, \
 	15 { `sosend soaddr=$D1%X4 sndaddr=$D2%X4 uio=$D3%X4 top=$D4%D1 contrl=$D5%X4` }, \
 	16 { `return from sosend error=$D1%D1` }, \
 	17 { `soreceive soaddr=$D1%X4 paddr=$D2%X4 uio=$D3%X4 mpo=$D4%D1 contrl=$D5%X4` }, \
 	18 { `return from soreceive error=$D1%D1` }, \
 	19 { `soshutdown soaddr=$D1%X4 ` how=sohow($D2) }, \
 	20 { `sorflush soaddr=$D1%X4` }, \
 	21 { `sosetopt soaddr=$D1%X4 level=$D2%D1` optname=soopt($D3) `m0=$D4%X4` }, \
 	22 { `return from sosetopt error=$D1%D1` }, \
 	23 { `sogetopt soaddr=$D1%X4 level=$D2%D1 ` optname=soopt($D3) `m0=$D4%X4` }, \
 	24 { return from sogetopt }, \
 	25 { `sohasoutofband soaddr=$D1%X4` }, \
 	26 { return from sohasoutofband }, \
	27 { `return from socreate soaddr=$D1%X4` }, \
 	\* { unknown subhook $HD $ERROR }

+254 1.0 'MBUF' \
{{ \
waittype(ppp) \
	$ppp%D1, \
	0    M_DONTWAIT, \
	1    M_WAIT, \
	\*  {$ppp%D1} \
}} \
{{ \
mbuftype(ttt) \
	$ttt%D1, \
	0   MT_FREE, \
	1   MT_DATA, \
	2   MT_HEADER, \
	3   MT_SOCKET, \
	4   MT_PCB, \
	5   MT_RTABLE, \
	6   MT_HTABLE, \
	7   MT_ATABLE, \
	8   MT_SONAME, \
	10  MT_SOOPTS, \
	11  MT_FTABLE, \
	12  MT_RIGHTS, \
	13  MT_IFADDR, \
	14  MT_CONTROL, \
	15  MT_OOBDATA, \
	\*  {$ttt%D1} \
}} \
 	$HD%D1, \
 	1  { `m_get ` canwait=waittype($D1) type=mbuftype($D2) `callfrom=$D3%X4` }, \ 
 	2  { `return from m_get mbuf=$D1%X4 dataptr=$D2%X4` }, \
 	3  { `m_getclr ` canwait=waittype($D1) type=mbuftype($D2) `callfrom=$D3%X4` }, \ 
 	4  { `return from m_getclr mbuf=$D1%X4 dataptr=$D2%X4` }, \
 	5  { `m_free mbuf=$D1%X4 dataptr=$D2%X4 callfrom=$D3%X4` `callfrom2=$D4%X4` `pid=$D5%D1 ($D5%P4)`}, \
 	6  { `return from m_free mbuf=$D1%X4` }, \
 	7  { `m_copy mbuf=$D1%X4 offset=$D2%D1 len=$D3%D1` }, \
 	8  { `return from m_copy top=$D1%X4` }, \
 	9  { `m_copydata mbuf=$D1%X4 offset=$D2%D1 len=$D3%D1 cpaddr=$D4%X4` }, \
 	10 { "return from m_copydata" }, \
 	11 { "m_pullup bcopy" }, \
 	12 { "m_pullup bcopy end" }, \
 	13 { "mbuf low intr" }, \
 	14 { "return from mbuf low intr" }, \
 	15 { "m_low: schedule mlowintr" }, \
	16 { `m_clgetm ` canwait=waittype($D1) `mbuf=$D2%X4` `callfrom=$D3%X4` `callfrom2=$D4%X4` `pid=$D5%D1 ($D5%P4)` }, \
	17 { `return from m_clgetm mbuf=$D1%X4 dataptr=$D2%X4` }, \
        18 { `m_getclustm ` canwait=waittype($D1) type=mbuftype($D2) `callfrom=$ D3%X4` `callfrom2=$D4%X4` `pid=$D5%D1 ($D5%P4)` }, \
        19 { `return from m_getclustm mbuf=$D1%X4 dataptr=$D2%X4` }, \
        20 { `m_clattach ` canwait=waittype($D1) `callfrom=$D2%X4` `callfrom2=$D 3%X4` `pid=$D4%D1 ($D4%P4)` }, \
        21 { `return from m_clattach mbuf=$D1%X4 dataptr=$D2%X4` }, \
        22 { `m_prepend ` canwait=waittype($D1) `callfrom=$D2%X4` `callfrom2=$D 3%X4` `pid=$D4%D1 ($D4%P4)` }, \
        23 { `return from m_prepend mbuf=$D1%X4 dataptr=$D2%X4` }, \
        24 { `m_pullup ` canwait=waittype($D1) `callfrom=$D2%X4` `callfrom2=$D 3%X4` `pid=$D4%D1 ($D4%P4)` }, \
        25 { `return from m_pullup mbuf=$D1%X4 dataptr=$D2%X4` }, \
        26 { `m_copy ` canwait=waittype($D1) `callfrom=$D2%X4` `callfrom2=$D 3%X4` `pid=$D4%D1 ($D4%P4)` }, \
        27 { `return from m_copy mbuf=$D1%X4 dataptr=$D2%X4` }, \
 	\* {  unknown subhook $HD $ERROR }

+255 1.0 'NETIF_EN' \
 	$HD%D1, \
 	1  { `en_statintr (entry)  ifp=$D1%X4, sbp_option=$D2%D1` }, \
 	2  { `en_statintr (rtn)` }, \
 	3  { `en_netintr (entry)  ifp=$D1%X4, status=$D2%D1` }, \
 	4  { `en_netintr (rtn)` }, \
 	5  { `en_attach (entry)  unit=$D1%D1` }, \
 	6  { `en_attach (rtn)` }, \
 	7  { `en_detach (entry)  ifp=$D1%X4` }, \
 	8  { `en_detach (rtn)` }, \
 	9  { `en_init (entry)` }, \
 	10 { `en_init (rtn)` }, \
 	11 { `en_ioctl (entry)  ifp=$D1%X4, cmd=$D2%D1 data=$D3%X4` }, \
 	12 { `en_ioctl (rtn) error=$D1%D1` }, \
 	13 { `en_output(entry) ifp=$D1%X4 m=$D2%X4 family=$D3%D1 `ipaddr=sinaddr($D4) }, \
 	14 { `en_output (rtn) error=$D1%D1` }, \
 	15 { `en_reset (entry)` }, \
 	16 { `en_reset (rtn)` }, \
 	17 { `en_recv (entry)  m=$D1%X4, ifp=$D2%X4` }, \
 	18 { `en_recv (rtn)` }, \
 	\* {  unknown subhook $HD $ERROR }

+256 1.0 'NETIF_TOK' \
 	$HD%D1, \
 	1  { `ie5_statintr (entry)  ifp=$D1%X4, sbp_option=$D2%D1` }, \
 	2  { `ie5_statintr (rtn)` }, \
 	3  { `ie5_netintr (entry)  ifp=$D1%X4, status=$D2%D1` }, \
 	4  { `ie5_netintr (rtn)` }, \
 	5  { `ie5_attach (entry)  unit=$D1%D1` }, \
 	6  { `ie5_attach (rtn)` }, \
 	7  { `ie5_detach (entry)  ifp=$D1%X4` }, \
 	8  { `ie5_detach (rtn)` }, \
 	9  { `ie5_init (entry)` }, \
 	10 { `ie5_init (rtn)` }, \
 	11 { `ie5_ioctl (entry)  ifp=$D1%X4, cmd=$D2%D1 data=$D3%X4` }, \
 	12 { `ie5_ioctl (rtn) error=$D1%D1` }, \
 	13 { `ie5_outpt(entry) ifp=$D1%X4 m=$D2%X4 fmly=$D3%D1 `ipaddr=sinaddr($D4) }, \
 	14 { `ie5_outpt (rtn) error=$D1%D1` }, \
 	15 { `ie5_reset (entry)` }, \
 	16 { `ie5_reset (rtn)` }, \
 	17 { `ie5_recv (entry)  m=$D1%X4, ifp=$D2%X4` }, \
 	18 { `ie5_recv (rtn)` }, \
 	\* {  unknown subhook $HD $ERROR }

+257 1.0 'NETIF_802.3' \
 	$HD%D1, \
 	1  { `ie3_statintr (entry)  ifp=$D1%X4, sbp_option=$D2%D1` }, \
 	2  { `ie3_statintr (rtn)` }, \
 	3  { `ie3_netintr (entry)  ifp=$D1%X4, status=$D2%D1` }, \
 	4  { `ie3_netintr (rtn)` }, \
 	5  { `ie3_attach (entry)  unit=$D1%D1` }, \
 	6  { `ie3_attach (rtn)` }, \
 	7  { `ie3_detach (entry)  ifp=$D1%X4` }, \
 	8  { `ie3_detach (rtn)` }, \
 	9  { `ie3_init (entry)` }, \
 	10 { `ie3_init (rtn)` }, \
 	11 { `ie3_ioctl (entry)  ifp=$D1%X4, cmd=$D2%D1 data=$D3%X4` }, \
 	12 { `ie3_ioctl (rtn) error=$D1%D1` }, \
 	13 { `ie3_outpt(entry) ifp=$D1%X4 m=$D2%X4 fmly=$D3%D1 `ipaddr=sinaddr($D4) }, \
 	14 { `ie3_outpt (rtn) error=$D1%D1` }, \
 	15 { `ie3_reset (entry)` }, \
 	16 { `ie3_reset (rtn)` }, \
 	17 { `ie3_recv (entry)  m=$D1%X4, ifp=$D2%X4` }, \
 	18 { `ie3_recv (rtn)` }, \
 	\* {  unknown subhook $HD $ERROR }

+258 1.0 'NETIF_X25' \
 	$HD%D1, \
 	1  { `xt_statintr (entry)  ifp=$D1%X4, sbp_option=$D2%D1` }, \
 	2  { `xt_statintr (rtn)` }, \
 	3  { `xt_netintr (entry)  ifp=$D1%X4, status=$D2%D1` }, \
 	4  { `xt_netintr (rtn)` }, \
 	5  { `xt_attach (entry)  unit=$D1%D1` }, \
 	6  { `xt_attach (rtn)` }, \
 	7  { `xt_detach (entry)  ifp=$D1%X4` }, \
 	8  { `xt_detach (rtn)` }, \
 	9  { `xt_init (entry)` }, \
 	10 { `xt_init (rtn)` }, \
 	11 { `xt_ioctl (entry)  ifp=$D1%X4, cmd=$D2%D1 data=$D3%X4` }, \
 	12 { `xt_ioctl (rtn) error=$D1%D1` }, \
 	13 { `xt_outpt(entry) ifp=$D1%X4 m=$D2%X4 fmly=$D3%D1 `ipaddr=sinaddr($D4) }, \
 	14 { `xt_outpt(rtn) error=$D1%D1` }, \
 	15 { `xt_reset (entry)` }, \
 	16 { `xt_reset (rtn)` }, \
 	17 { `xt_recv (entry)  m=$D1%X4, ifp=$D2%X4` }, \
 	18 { `xt_recv (rtn)` }, \
 	19 { `xt_incoming_call(entry) m=$D1%X4 ifp=$D2%X4 callid=$D3%D1 sessid=$D4%D1` }, \
 	20 { `xt_incoming_call (rtn)` }, \
 	21 { `xt_call_complete (entry)  ifp=$D2%X4 sessid=$D2%D1 m=$D3%X4` }, \
 	22 { `xt_call_complete (rtn)` }, \
 	23 { `xt_clear_indication (entry)  ifp=$D2%X4 sessid=$D2%D1` }, \
 	24 { `xt_clear_indication (rtn)` }, \
 	\* {  unknown subhook $HD $ERROR }

+259 1.0 'NETIF_SER' \
 	$HD%D1, \
 	5  { `slattach (entry)  unit=$D1%D1` }, \
 	6  { `slattach (rtn)` }, \
 	7  { `sl_detach (entry)  ifp=$D1%X4` }, \
 	8  { `sl_detach (rtn)` }, \
 	9  { `slinit (entry)` }, \
 	10 { `slinit (rtn)` }, \
 	11 { `slioctl (entry)  ifp=$D1%X4, cmd=$D2%D1 data=$D3%X4` }, \
 	12 { `slioctl (rtn) error=$D1%D1` }, \
 	13 { `sloutput(entry) ifp=$D1%X4 m=$D2%X4 family=$D3%D1 `ipaddr=sinaddr($D4) }, \
 	14 { `sloutput (rtn) error=$D1%D1` }, \
 	15 { `slreset (entry)` }, \
 	16 { `slreset (rtn)` }, \
 	25 { `slstart slip HIWAT reached ifp=$D1%X4 hiwat=$D2%D1` }, \
 	\* {  unknown subhook $HD $ERROR }

+25A 1.0 'TCPDBG' \
 	$HD%D1, \
 	1  { `TA_INPUT tp=$D1%X4  ostate=$D2%D1  flags=$D3%D1` }, \
 	2  { `TA_OUPUT tp=$D1%X4  ostate=$D2%D1  flags=$D3%D1` }, \
 	3  { `TA_USER req=$D1%D1` }, \
 	4  { `TA_RESPOND tp=$D1%X4  ostate=$D2%D1  flags=$D3%D1` }, \
 	5  { `TA_DROP tp=$D1%X4  ostate=$D2%D1  flags=$D3%D1` }, \
 	6  { `seq=$D1%U4  ack=$D2%X4  len=$D3%U4` }, \
 	7  { `rcvnxt=$D1%U4  rcvwnd=$D2%U4  snduna=$D3%U4 sndnxt=$D4%U4 sndmax=$D5%U4` }, \
 	8  { `sndwl1=$D1%U4  sndwl2=$D2%U4  sndwnd=$D3%U4` }, \
 	\* {  unknown subhook $HD $ERROR }

+300 1.0 "@ODM EVENTS"\
        $HD, \
        1       "ODM:   ADDR_CLXN",\
        2       "ODM:   ADD_VCHAR",\
        3       "ODM:   BREAKCRIT",\
        4       "ODM:   CHANGE_VCHAR",\
        5       "ODM:   CLOSE_CLXN",\
        6       "ODM:   CMPKMCH",\
        7       "ODM:   CREATE_CLXN",\
        8       "ODM:   DESTROY_CLXN",\
        9       "ODM:   GET_OFFSETS",\
        A       "ODM:   GET_STRING_DBOFF",\
        B       "ODM:   GET_VCHAR",\
        C       "ODM:   INIT_CLASS",\
        D       "ODM:   INIT_CLXN",\
        E       "ODM:   INVOKE",\
        F       "ODM:   MOUNT_CLXN",\
        10      "ODM:   NOTE_CLASS",\
        11      "ODM:   ODM_ADD_OBJ",\
        12      "ODM:   ODM_CHANGE_OBJ",\
        13      "ODM:   ODM_CLOSE_CLASS",\
        14      "ODM:   ODM_CREATE_CLASS",\
        15      "ODM:   ODM_FREE_LIST",\
        16      "ODM:   ODM_GET_BY_ID",\
        17      "ODM:   ODM_GET_FIRST",\
        18      "ODM:   ODM_GET_LIST",\
        19      "ODM:   ODM_GET_NEXT",\
        1A      "ODM:   ODM_GET_OBJ",\
        1B      "ODM:   ODM_INITIALIZE",\
        1C      "ODM:   ODM_MOUNT_CLASS",\
        1D      "ODM:   ODM_OPEN_CLASS",\
        1E      "ODM:   ODM_RM_BY_ID",\
        1F      "ODM:   ODM_RM_CLASS",\
        20      "ODM:   ODM_RM_OBJ",\
        21      "ODM:   ODM_SET_PATH",\
        22      "ODM:   ODM_SET_PERMS",\
        23      "ODM:   ODM_TERMINATE",\
        24      "ODM:   OPEN_CLXNP",\
        25      "ODM:   RAW_ADDR_CLASS",\
        26      "ODM:   RAW_ADD_OBJ",\
        27      "ODM:   RAW_ADD_STR",\
        28      "ODM:   RAW_CLOSE_CLASS",\
        29      "ODM:   RAW_CLOSE_CLXN",\
        2A      "ODM:   RAW_FIND_BYID",\
        2B      "ODM:   RAW_FIND_OBJ",\
        2C      "ODM:   RAW_RM_OBJ",\
        2D      "ODM:   VERIFY_CLASS_STRUCTURE",\
        \*      "ODM:   INVALID MODULE ID"\
        $D1,\
        0  " ENTRY",\
        1  " EXIT",\
        \*      "   INVALID DATA VALUE"

+234 1.0 L=KERN "@CLOCK"	\
        "clock:   iar="$D1  "lr="$D2	\
	endtimer(0x234,0x234) starttimer(0x234,0x234)


+278 1.0 L=KERN "@PSLA DR. CONFIG CALL " \
	D2, \
	0 "psla config entry" {dev #=$D1 cmd=$D2%D4, 1 "CFG_INIT ", \
          2 "CFG_TERM ", 3 "CFG_QVPD " struct uio ptr=$D3}, \
        1 "psla config return", \
	2 "ERROR-psla config return" {errno=E4}, \
	3 "psla config return, already configured", \
	4 "ERROR-psla config, in uimove" {errno=E4 dds ptr=$D2 \
          dds size=$D3%D4 uio struct ptr=$D4}, \
	5 "ERROR-psla config, in define" {errno=E4 dev #=$D2 dds ptr=$D3}, \
	6 "ERROR-psla config, in devswadd" {errno=E4}, \
	7 "ERROR-psla config, in i_init", \
	8 "ERROR-psla config, in xmalloc" {errno=E4}, \
	9 "ERROR-psla config return" {errno=E4,dev mnr#=$D2}, \
       10 "ERROR-psla config, in loadpos" {errno=E4} 

+272 1.0 L=KERN "@PSLA DR. OPEN(X) CALL " \
	D2, \
	0 "psla open(x) entry" {dev#=$D1 mode=$D2 channel=$D3 opnparms \
          	struct ptr=$D4}, \  
	1 "psla open(x) return", \
	2 "ERROR-psla open(x) return" {errno=E4 dev mnr#=$D2}, \
	3 "ERROR-psla open(x) return, diag. mode" {errno=E4}, \  
	4 "psla open(x) return" {errno=E4 dev mnr#=$D2 status=\
		$D3%D4, \
		1 "IO NOT ALLOWED",\
		2 "GSW NOT CONF",\
		3 "SWITCHED TO HOST",\
		4 "NOT YET READY"}	

+273 1.0 L=KERN "@PSLA DR. CLOSE CALL" \
	D2, \
	0 "psla close entry" {dev#=$D1 mode=$D2 channel=$D3 opnparms \
          	struct ptr=$D4}, \  
	1 "psla close return", \
	2 "ERROR-psla close return" {errno=E4 dev mnr#=$D2}

+274 1.0 L=KERN "@PSLA DR. READ CALL" \
	D2, \
	0 "psla read entry" {dev#=$D1 uio struct ptr=$D2 channel=$D3 \
		rw_args union ptr=$D4}, \
	1 "psla read return", \
	2 "ERROR-psla read return" {errno=E4 dev mnr#=$D2}, \
        3 "ERROR-psla read return" {errno=E4 dev mnr#=$D2 dev mode=$D3}, \
	4 "ERROR-psla read return" {errno=E4 dev mnr#=$D2 byte count=$D3%D4}

+275 1.0 L=KERN "@PSLA DR. WRITE CALL" \
	D2, \
	0 "psla write entry" {dev#=$D1 uio struct ptr=$D2 \
		channel=$D3 lrw_args union ptr=$D4}, \
	1 "psla write return", \
	2 "ERROR-psla ioctl return" {errno=E4 dev mnr#=$D2}, \
        3 "ERROR-psla ioctl return" {errno=E4 dev mnr#=$D2 dev mode=$D3}, \
	4 "ERROR-psla ioctl return" {errno=E4 dev mnr#=$D2 byte count=$D3%D4}

+276 1.0 L=KERN "@PSLA DR. IOCTL CALLS " \
{{ \
ioctlcmds(cmd) \
	$cmd%D1, \
	10 "G_SET_MEM", \
 	11 "G_ALARM", \
	12 "G_STOP", \
	13 "G_START", \
	14 "G_SET_BF", \
	15 "G_SBF_START", \
	16 "G_SET_CUR", \
	17 "G_RESET_CUR", \
	18 "G_SET_IND", \
	19 "G_SET_IND_ON", \
	20 "G_SET_IND_OFF", \
	21 "G_WRITESF", \
	22 "G_DEF_MEM", \
	23 "G_DEL_MEM", \
	24 "G_REN_MEM", \
	25 "G_LOAD_BLNK", \
	26 "G_LOAD_LINE", \
	27 "G_READ_CUR", \
	28 "G_SENSE", \
	29 "G_LOAD_MEM_AREA", \
	31 "G_IOCTL", \
	32 "MSLA_START_DIAG", \
        33 "MSLA_QUERY_DIAG", \
        34 "MSLA_STOP_DIAG", \
	35 "MSLA_GET_ADDR", \
	36 "MSLA_RET_ADDR", \
	37 "MSLA_MOD_POS", \
	38 "MSLA_LOAD_UCODE", \
	39 "MSLA_START_DMA", \
	40 "MSLA_STOP_DMA", \
	65 "K_ENABLE", \
	66 "K_DISABLE", \
	67 "K_ENA_SIG", \
	68 "K_DIS_SIG", \
	69 "K_POLL", \
	70 "K_WAIT", \
	71 "K_FLUSH", \
	72 "K_REQUEST", \
	73 "K_STOP_DEVICE", \
	74 "K_LCW", \
	75 "FPGI_CONNECT", \
	76 "FPGI_DISCONNECT", \
	77 "FPGI_INFREE", \
	78 "FPGI_SEND", \
	79 "FPGI_WAIT", \
	/* "UNKNOWN input type" \
}} \
{{ \
intypes(itypes) \
	BITFLAGS $itypes, \
	0x01 "GPICK", \
	0x02 "GANK", \
	0x04 "GPFK", \
	0x08 "GLPFK", \
	0x10 "GTABLET", \
	0x20 "GSMI", \
	0x40 "GGEOP", \
	0x80 "GPGM_ERR" \
}} \
	D2, \
	0 "psla ioctl entry" {dev#=$D1 G11 ioctlcmds(X1) \
          "," ctl_args union ptr=$D3 flags=$D4}, \
	1 "psla ioctl return", \
	2 "ERROR-psla ioctl return" {errno=E4 dev mnr#=$D2}, \
	3 "ERROR-psla ioctl return" {errno=E4 dev mnr#=$D2 G15 ioctlcmds(X1) \
	  "," ctl_args union ptr=$D4}, \
	4 "ERROR-psla ioctl return" {errno=E4 dev mnr#=$D2 G15 ioctlcmds(X1) \
	  "," blnk_number=D4 blnk_pattern=D4}, \
	5 "ERROR-psla ioctl return" {errno=E4 dev mnr#=$D2 G15 ioctlcmds(X1) \
	  "," data length=D4}, \
	6 "ERROR-psla ioctl return" {errno=E4 dev mnr#=$D2 G15 ioctlcmds(X1) \
          "," input type=D4, 0 "GPICK", 1 "GANK", 2 "GPFK", 3 "GLPFK", \
          4 "GTABLET", 5 "GSMI", 6 "GGEOP", 7 "GPGM_ERR"}, \
	7 "ERROR-psla ioctl return" {errno=E4 dev mnr#=$D2 ntr. queue ptr=$D3 \
          input type=$D4%D4, 0 "GPICK", 1 "GANK", 2 "GPFK", 3 "GLPFK", \
          4 "GTABLET", 5 "GSMI", 6 "GGEOP", 7 "GPGM_ERR"}, \
	8 "ERROR-psla ioctl return" {errno=E4 dev mnr#=$D2 G15 ioctlcmds(X1) \
          "," G19 input types=intypes(X1)},\
	9 "ERROR-psla ioctl return" {errno=E4 dev mnr#=$D2 G15 ioctlcmds(X1)}, \ 
	10 "ERROR-psla ioctl return" {errno=E4 dev mnr#=$D2 G15 \
           ioctlcmds(X1) "," struct fld ptr=$D4 write struc len=$D5%D4}, \
	11 "psla ioctl" {dev mnr#=$D1 G11 ioctlcmds(X1) G15 \
           "," input types=intypes(X1)} 

+277 1.0 L=INT "@PSLA INTERRUPT HANDLER " \
{{ \
cmdcccodes(ccodes) \
	$ccodes, \
	0x04 "RdSense,", \
	0x05 "WrStruct,", \
	0x09 "WrMemArea,", \
	0x0A "RdMemArea,", \
	0x0E "Rmi,", \
	0x2B "SelcRdMemArea,", \
	0xF7 "SetMode,", \
	0xFB "SelcWrMemArea,", \
	0xFF "UnDefCmd," \
}} \
{{ \
initcccodes(intcode) \
	$intcodes, \
	0x01 "StartCmd,", \
	0x02 "StopCmd,", \
	0x03 "EnterMonMode,", \
	0x04 "ConfigDataPres,", \
	0x05 "DevNotPres,", \
	0x06 "DevNotResp,", \
	0x07 "LinkDown," \
}} \
{{ \
intrcodes(icodes) \
	$icodes, \
	0x00 {Cmd G15 ccc=cmdcccodes(X1)}, \
	0x01 {LinkSwitch G15 ccc=X1, 0x00 "connect host", \
             0x01 "connect pc"}, \
	0x02 "Diag,", \
	0x03 "Debug,", \
	0x04 {Init G15 ccc=initcccodes(X1)}, \
	0x05 "Progcode,", \
	0x06 "UnSolDevReq,", \
	0x07 "AdapterErr," \
}} \
{{ \
status(stat) \
	BITFLAGS $stat, \
 	0x80 "attention ", \
	0x40 "stat modifier ", \
	0x20 "adapter ready ", \
	0x10 "busy ", \
	0x08 "adapter end ", \
	0x04 "device end ", \
	0x02 "unit check ", \
	0x01 "unit exception " \
}} \
{{ \
pslaflags(flags) \
   	BITFLAGS $flags, \
	0x8000 "Solo in prog", \
	0x4000 "Rmi in prog", \
	0x2000 "Retry in prog", \
	0x1000 "Ipl_req", \
	0x0800 "Ipl in prog", \
	0x0400 "Dma enabled", \
	0x0200 "Start in prog", \
	0x0100 "stop in prog", \
	0x0080 "Need start", \
	0x0040 "Need stop", \
	0x0020 "Fpgi solo in prog", \
	0x0010 "Intio in prog" \
}} \
{{ \
retcode(rcode) \
	$rcode, \
	0x80 "Link monitor mode", \
	0x50 "No RMI data", \
	0x51 "No 5080 present", \
	0x52 "No resp. to req.", \
	0x70 "Host link down", \
	0x3F "Sense from bad async", \
        0x3E "Sense from bad SIO", \
	0x34 "Sense area not defined", \
	0x35 "Sense area full", \
	0x36 "Sense timeout occured" \
}} \
	D2, \
	0 "psla intr. hdlr entry" {struct intr ptr=$D1}, \
	1 "psla intr. data:" {dev mnr#=$D1 G11 icc=intrcodes(X1) \n\
          G19 status=status(X1) "," G22 pslaflags(X2)}, \ 
	2 "psla intr hdlr return, not psla intr" {G7 stat reg=X1}, \
	3 "psla intr hdlr return, diag. mode, parity check", \
	4 "psla intr hdlr return, parity check, not sftwr intr.", \
	5 "psla intr hdlr return,diag. mode", \
	6 "psla intr hdlr return, not a sftwr intr.", \
	7 "psla intr hdlr return" {G7 retcode(X1)}, \
	8 "ERROR-Fatal, psla intr hdlr return", \
        9 "ERROR-device not open", \
       10 "psla intr hdlr return" {type=D4, 5 "GSMI", 6 "GGEOP" \
	  "," intr. queue ptr=$D2}, \
       11 "psla intr hdlr," { \
          input type=D4, 0 "GPICK", 1 "GANK", 2 "GPFK", 3 "GLPFK", \
          4 "GTABLET", 5 "GSMI", 6 "GGEOP", 7 "GPGM_ERR"}

+502 1.0 L=SVC "GSC " \
	{{ $subfunc = $HD / 16 }} \
	{{ $x = $subfunc * 16 }} \
	{{ $type = $HD - $x }} \
	$type,	\
		0 "Entry to ", \
		1 "Exit from " \
	$subfunc%D2,   \
		1 "MAKE_GP", \
		2 "UNMAKE_GP", \
		3 "SET_GP_PRIORITY", \
		4 "CREATE_RCX", \
		5 "DELETE_RCX", \
		6 "CREATE_WIN_GEOM", \
		7 "DELETE_WIN_GEOM", \
		8 "UPDATE_WIN_GEOM", \
		9 "CREATE_WIN_ATTR", \
		10 "DELETE_WIN_ATTR", \
		11 "UPDATE_WIN_ATTR", \
		12 "BIND_WINDOW", \
		13 "SET_RCX", \
		14 "LOCK_HW", \
		15 "UNLOCK_HW", \
		16 "LOCK_DOMAIN", \
		17 "UNLOCK_DOMAIN", \
		18 "CREATE_RCXP", \
		19 "DELETE_RCXP", \
		20 "ASSOCIATE_RCXP", \
		21 "DISASSOCIATE_RCXP", \
		29 "COMMAND_LIST", \
		30 "DMA_SERVICE", \
		49 "PERFORMANCE_TRACE", \
		69 "ASYNC_EVENT", \
		70 "GET_EVENTS", \
		71 "EVENT_BUFFER", \
		72 "WAIT_EVENT", \
		73 "ENABLE_EVENT", \
		74 "CREATE_COLORMAP", \
		75 "DELETE_COLORMAP", \
		76 "UPDATE_COLORMAP",	 \
		240 "GP_FAULT", \
		241 "GP_DISPATCH", \
		242 "RCX_SWITCH_DONE", \
		243 "GP_GIVEUP_TS", \
		\* { $subfunc%D2 }

+503 1.0 L=SVC "Graphics System Call " \
	{{ $subfunc = $HD / 16 }} \
	{{ $x = $subfunc * 16 }} \
	{{ $type = $HD - $x }} \
	$type,	\
		0 "Entry ", \
		1 "Exit " \
	$subfunc%D2,   \
		50 "DEV_DEP_FUN"

+204 1.0 L=KERN "@SWAPPER swapin process" \
	"sched swapin:   pid="$D1%D4  "thrashing="$D2%D4

+205 1.0 L=KERN "@SWAPPER swapout process" \
	"sched swapout:   pid="$D1%D4  "tid=" $D2%D4  "p_suspended="$D3%D4 \
	"n_active="$D4%D4  "v_min_process="$D5%D4

+206 1.0 L=KERN "@SWAPPER post process for suspension" \
	"sched post:   pid=" $D1%D4  "p_repage="$D2%D4  \
        "v_proc_repage="$D3%D4  "u_minflt="$D4%D4  "u_majflt="$D5%D4

+207 1.0 L=KERN "@SWAPPER sched stats" \
	"sched sys stats:   thrashing="$D1%D4  \
	"v_repage_hi="$D2%D4  "sysrepage=" $D3%D4  \
	"pgflts="$D4%D4  "pgstls="$D5%D4

+208 1.0 L=KERN "@SWAPPER process stats" \
	"sched proc stats:   nrun="$D1%D4  "nsusp="$D2%D4  \
	"npageio="$D3%D4  "nevents="$D4%D4  "nstarting=" $D5%D4

+209 1.0 L=KERN "@SWAPPER sched stats" \
	"sched sys stats:   pageins="$D1%D4  \
	"pageouts="$D2%D4  "pgspgins=" $D3%D4 \
	"pgspgouts="$D4%D4

+20A 1.0 L=KERN "@MEMORY SCRUBBING disable" \
	"scrub disable"

+20B 1.0 L=KERN "@MEMORY SCRUBBING enable" \
	"scrub enable"

+20C 1.0 L=KERN "@MEMORY SCRUBBING choose segment of memory" \
	"scrub memory:   start="$D1 "end="$D2

+20D 1.0 L=KERN "@MEMORY SCRUBBING report single bit errors" \
	"scrub report:   sbssr="$D1  "sbsr="$D2  "sbar="$D3

+20E 1.0 L=KERN "@LOCKL locks a conventional process lock" \
	"lockl		lock addr=" $D1 "lock val=" $D2%D4 \
	"pid=" $D3%D4 "return addr=" $D4 "flags=" $D5

+20F 1.0 L=KERN "@UNLOCKL unlocks a conventional process lock" \
	"unlockl		lock addr=" $D1 "lock val=" $D2%D4 \
	"pid=" $D3%D4 "return addr=" $D4

+292 1.0 "VCA DEVICE DRIVER" \
	{{ $loc = X1 }} \
	$loc, \
	0x1 { "entry_vca_open:" \
		"errno:" X1 "dev:" X4 \
		"flag:" X4 "channel:" X4 "ext:" X4}, \
	0x2 { "exit_vca_open:" \
		"errno:" X1 "dev:" X4 \
		"flag:" X4 "channel:" X4 "ext:" X4}, \
	0x3 { "entry_vca_close:" \
		"errno:" X1 "dev:" X4 \
		"flag:" X4 "ext:" X4 }, \
	0x4 { "exit_close:" \
		"errno:" X1 "dev:" X4 \
		"flag:" X4 "ext:" X4 }, \
	0x5 { "entry_vca_read:" \
		"errno:" X1 "dev:" X4 \
		"uiop:" X4 "mpschan:" X4 "ext:" X4}, \
	0x6 { "exit_vca_read:" \
		"errno:" X1 "dev:" X4 \
		"uiop:" X4 "mpschan:" X4 "ext:" X4}, \
	0x7 { "entry_vca_write:" \
		"errno:" X1 "dev:" X4 \
		"uiop:" X4 "mpschan:" X4 "ext:" X4}, \
	0x8 { "exit_vca_write:" \
		"errno:" X1 "dev:" X4 \
		"uiop:" X4 "mpschan:" X4 "ext:" X4}, \
	0x9 { "entry_vca_config:" \
		"errno:" X1 "dev:" X4 \
		"cmd:" X4 "uiop:" X4 }, \
	0xA { "exit_vca_config:" \
		"errno:" X1 "dev:" X4 \
		"cmd:" X4 "uiop:" X4 }, \
	0xB { "entry_vca_ioctl:" \
		"errno:" X1 "dev:" X4 \
		"cmd:" X4 "arg:" X4 "flag:" X4 "channel:" X4}, \
	0xC { "exit_vca_ioctl:" \
		"errno:" X1 "dev:" X4 \
		"cmd:" X4 "arg:" X4 "flag:" X4 "channel:" X4}, \
	0xD { "entry_vca_setup:" \
		"errno:" X1 "current_vca:" X4 }, \
	0xE { "exit_vca_setup:" \
		"errno:" X1 "current_vca:" X4 }, \
	0xF { "entry_vca_memcopy:" \
		"errno:" X1 "current_vca:" X4 \
		"offset:" X4 "count:" X4 "direction:" X4 }, \
	0x10 { "exit_vca_memcopy:" \
		"errno:" X1 "current_vca:" X4 \
		"offset:" X4 "count:" X4 "direction:" X4 }, \
	0x11 { "entry_mem_setup:" \
		"errno:" X1 "current_vca:" X4 \
		"paddr:" X4 "block:" X4 "taram:" X2 }, \
	0x12 { "exit_mem_setup:" \
		"errno:" X1 "current_vca:" X4 \
		"paddr:" X4 "block:" X4 "taram:" X2 }, \
	0x13 { "entry_mem_unset:" \
		"errno:" X1 "current_vca:" X4 \
		"vaddr:" X4 }, \
	0x14 { "exit_mem_unset:" \
		"errno:" X1 "current_vca:" X4 \
		"vaddr:" X4 }, \
	0x15 { "entry_getstatus:" \
		"errno:" X1 "current_vca:" X4 }, \
	0x16 { "exit_getstatus:" \
		"errno:" X1 "current_vca:" X4 }, \
	0x17 { "entry_setmask:" \
		"errno:" X1 "current_vca:" X4 "mask:" X4 }, \
	0x18 { "exit_setmask:" \
		"errno:" X1 "current_vca:" X4 "mask:" X4 }, \
	0x19 { "entry_capture_wait:" \
		"errno:" X1 "current_vca:" X4 "seconds:" X4 }, \
	0x1A { "exit_capture_wait:" \
		"errno:" X1 "current_vca:" X4 "seconds:" X4 }, \
	0x1B { "entry_intr_enable:" \
		"errno:" X1 "current_vca:" X4 }, \
	0x1C { "exit_intr_enable:" \
		"errno:" X1 "current_vca:" X4 }, \
	0x1D { "entry_intr_disable:" \
		"errno:" X1 "current_vca:" X4 }, \
	0x1E { "exit_intr_disable:" \
		"errno:" X1 "current_vca:" X4 }, \
	0x1F { "entry_vca_set_mode:" \
		"errno:" X1 "current_vca:" X4 "vca_mode_ptr:" X4 }, \
	0x20 { "exit_vca_set_mode:" \
		"errno:" X1 "current_vca:" X4 "vca_mode_ptr:" X4 }, \
	0x21 { "entry_vca_set_controls:" \
		"errno:" X1 "current_vca:" X4 "vca_controls_ptr:" X4 }, \
	0x22 { "exit_vca_set_controls:" \
		"errno:" X1 "current_vca:" X4 "vca_controls_ptr:" X4 }, \
	0x23 { "entry_vca_set_cursor:" \
		"errno:" X1 "current_vca:" X4 "vca_cursor_ptr:" X4 }, \
	0x24 { "exit_vca_set_cursor:" \
		"errno:" X1 "current_vca:" X4 "vca_cursor_ptr:" X4 }, \
	0x25 { "entry_vca_set_window:" \
		"errno:" X1 "current_vca:" X4 "vca_window_ptr:" X4 }, \
	0x26 { "exit_vca_set_window:" \
		"errno:" X1 "current_vca:" X4 "vca_window_ptr:" X4 }, \
	0x27 { "entry_vca_set_dacs:" \
		"errno:" X1 "current_vca:" X4 "vca_dacs_ptr:" X4 }, \
	0x28 { "exit_vca_set_dacs:" \
		"errno:" X1 "current_vca:" X4 "vca_dacs_ptr:" X4 }, \
	0x29 { "entry_vca_altdac:" \
		"errno:" X1 "current_vca:" X4 "dacno:" X4 "color:" X2 }, \
	0x2A { "exit_vca_altdac:" \
		"errno:" X1 "current_vca:" X4 "dacno:" X4 "color:" X2 }, \
	0x2B { "entry_vca_get_mode:" \
		"errno:" X1 "current_vca:" X4 "vca_mode_ptr:" X4 }, \
	0x2C { "exit_vca_get_mode:" \
		"errno:" X1 "current_vca:" X4 "vca_mode_ptr:" X4 }, \
	0x2D { "entry_vca_get_controls:" \
		"errno:" X1 "current_vca:" X4 "vca_controls_ptr:" X4 }, \
	0x2E { "exit_vca_get_controls:" \
		"errno:" X1 "current_vca:" X4 "vca_controls_ptr:" X4 }, \
	0x2F { "entry_vca_get_cursor:" \
		"errno:" X1 "current_vca:" X4 "vca_cursor_ptr:" X4 }, \
	0x30 { "exit_vca_get_cursor:" \
		"errno:" X1 "current_vca:" X4 "vca_cursor_ptr:" X4 }, \
	0x31 { "entry_vca_get_window:" \
		"errno:" X1 "current_vca:" X4 "vca_window_ptr:" X4 }, \
	0x32 { "exit_vca_get_window:" \
		"errno:" X1 "current_vca:" X4 "vca_window_ptr:" X4 }, \
	0x33 { "entry_vca_get_dacs:" \
		"errno:" X1 "current_vca:" X4 "vca_dacs_ptr:" X4 }, \
	0x34 { "exit_vca_get_dacs:" \
		"errno:" X1 "current_vca:" X4 "vca_dacs_ptr:" X4 }, \
	0x35 { "entry_readdac:" \
		"errno:" X1 "current_vca:" X4 "dacno:" X4 }, \
	0x36 { "exit_readdac:" \
		"errno:" X1 "current_vca:" X4 "dacno:" X4 }, \
	0x37 { "entry_vca_set_dac_defaults:" \
		"errno:" X1 "current_vca:" X4 "dacs_ptr:" X4 }, \
	0x38 { "exit_vca_set_dac_defaults:" \
		"errno:" X1 "current_vca:" X4 "dacs_ptr:" X4 }, \
	0x39 { "entry_vca_intr:" \
		"errno:" X1 "handler:" X4 }, \
	0x3A { "exit_vca_intr:" \
		"errno:" X1 "handler:" X4 }, \
	0x3B { "entry_vca_outb:" \
		"errno:" X1 "port:" X4 "data:" X1 }, \
	0x3C { "exit_vca_outb:" \
		"errno:" X1 "port:" X4 "data:" X1 }, \
	0x3D { "entry_vca_inb:" \
		"errno:" X1 "port:" X4 }, \
	0x3E { "exit_vca_inb:" \
		"errno:" X1 "port:" X4 }, \
	0x3F { "entry_vca_timed_out:" \
		"errno:" X1 "w:" X4 }, \
	0x40 { "exit_vca_timed_out:" \
		"errno:" X1 "w:" X4 }, \
	0x41 { "entry_vca_altreg:" \
		"errno:" X1 "current_vca:" X4  "reg:" X4 "value:" X1 }, \
	0x42 { "exit_vca_altreg:" \
		"errno:" X1 "current_vca:" X4  "reg:" X4 "value:" X1 }, \
	0x43 {  "entry_vca_readreg:"\
		"errno:" X1 "current_vca:" X4 "reg:" X4 }, \
	0x44 { "exit_vca_readreg:" \
		"errno:" X1 "current_vca:" X4 "reg:" X4 }, \
	0x45 { "entry_vca_kset_cursor:" \
		"errno:" X1 "current_vca:" X4 "vca_cursor_ptr:" X4 }, \
	0x46 { "exit_vca_kset_cursor:" \
		"errno:" X1 "current_vca:" X4 "vca_cursor_ptr:" X4 }, \
	0x47 { "entry_vca_kget_cursor:" \
		"errno:" X1 "current_vca:" X4 "vca_cursor_ptr:" X4 }, \
	0x48 { "exit_vca_kget_cursor:" \
		"errno:" X1 "current_vca:" X4 "vca_cursor_ptr:" X4 }, \
	0x49 { "entry_vca_delay:" \
		"errno:" X1 "current_vca:" X4 }, \
	0x4A { "exit_vca_delay:" \
		"errno:" X1 "current_vca:" X4 }, \
	0x4B { "entry_vca_initialize:" \
		"errno:" X1 "current_vca:" X4 }, \
	0x4C { "exit_vca_initialize:" \
		"errno:" X1 "current_vca:" X4 }, \
	0x4D { "entry_vcaerr:" \
		"errno:" X1 "errid:" X4 "lname:" X4 "line_num:" X4 \
		"dmodule:" X4 "fmodule:" X4}, \
	0x4E { "exit_vcaerr:" \
		"errno:" X1 "errid:" X4 "lname:" X4 "line_num:" X4 \
		"dmodule:" X4 "fmodule:" X4}

+7FF 1.0 'STREAMS (PSE)' \
    $HD%D1, \
    01 { `getmsg($D1%D4, $D2%X4, $D3%X4, $D4%X4)` }, \
    02 { `getmsg returned $D1%D4` }, \
    03 { `getpmsg($D1%D4, $D2%X4, $D3%X4, $D4%X4, $D4%X4)` }, \
    04 { `getpmsg returned $D1%D4` }, \
    05 { `putmsg($D1%D4, $D2%X4, $D3%X4, $D4%X4)` }, \
    06 { `putmsg returned $D1%D4` }, \
    07 { `putpmsg($D1%D4, $D2%X4, $D3%X4, $D4%X4, $D4%X4)` }, \
    08 { `putmsg returned $D1%D4` }, \
    09 { `mps_open($D1%X4, $D2%X4, $D3%X4, $D4%X4)` }, \
    10 { `mps_open returned $D1%D4)` }, \
    11 { `mps_clone($D1%X4, $D2%X4, $D3%X4, $D4%X4)` }, \
    12 { `mps_clone returned $D1%D4` }, \
    13 { `mps_close($D1%X4, $D2%X4)` }, \
    14 { `mps_close returned $D1%D4` }, \
    15 { `mps_read($D1%X4, $D2%X4, $D3%X4, $D4%X4)` }, \
    16 { `mps_read returned $D1%D4` }, \
    17 { `mps_write($D1%X4, $D2%X4, $D3%X4, $D4%X4)` }, \
    18 { `mps_write returned $D1%D4` }, \
    19 { `mps_ioctl($D1%X4, $D2%X4, $D3%X4, $D4%X4, $D5%X4)` }, \
    20 { `mps_ioctl returned $D1%D4` }, \
    21 { `mps_poll($D1%X4, $D2%X4, $D3%X4, $D4%X4)` }, \
    22 { `mps_poll returned $D1%D4` }, \
    \* { `unknown STREAMS subhook $HD $ERROR` }

+112 1.0  L=KERN "@LOCK" \
        "lock:          " $HD & 0x000F , \
        1 { "lock " }, \
        2 { "miss " }, \
        3 { "recu " }, \
        4 { "busy " }, \
        9 { "dlock " }, \
        A { "dmiss " }, \
        B { "drecu " }, \
        C { "dbusy " }, \
	\* { "error HD=" $HD } \
	"lock addr=" $D1	" lock status=" $D2 \
	$D3, \
	1 { "requested_mode=LOCK_SWRITE "}, \
	2 { "requested_mode=LOCK_CWRITE "}, \
	3 { "requested_mode=LOCK_READ "}, \
	4 { "requested_mode=LOCK_UPGRADE "},\
	5 { "requested_mode=LOCK_DOWNGRADE "}, \
	\* { "unknown requested mode "} \
	"return addr="$D4 \
	{{ $z1 = $D5%W16.31 }} \
	{{ $z2 = $D5%W0.15 }} \
	"name=" $z1%X2"."$z2%X2
+113 1.0 L=KERN "@UNLOCK" \
	"unlock:	"\
	"lock addr=" $D1       " lock status=" $D2 \
	"return addr=" $D3 \
	{{ $z1 = $D4%W16.31 }} \
        {{ $z2 = $D4%W0.15 }} \
        "name=" $z1%X2"."$z2%X2
+114 1.0 L=KERN "@LOCKALLOC" \
	"lockalloc:	" \
	"lock_addr=" $D1 	" name=" $D2%X2"."$D3%X2 \
	" return_addr=" $D4
+115 1.0 L=KERN "@SETRECURSIVE" \
	$HD, \
	1 { "SETRECURSIVE		" }, \
	2 { "CLEARRECURSIVE		" }, \
	\* { "error HD=" $HD } \
	" lock_addr=" $D1 " return_addr=" $D2
+230 1.0 L=KERN "@MUTEX LOCK (libpthreads)" \
	"pthread_mutex_lock		lock_addr=" $D1 \
	$D2, \
	0 { "lock = REQUESTED" }, \
	1 { "lock = FIRST GOT" }, \
	2 { "lock = GOT " }, \
	3 { "lock = GOT after thread_tsleep" }, \
	\* { "lock = NOT GOT" } \
	"lock owner=" $D3%D4
+231 1.0 L=KERN "@MUTEX UNLOCK (libpthreads)" \
	"pthread_mutex_unlock 		lock_addr=" $D1 "lock owner= " $D2%D4
+232 1.0 L=KERN "@SPIN LOCK (libpthreads)" \
	"pthread_spin_lock 		lock_addr=" $D1 \
	$D2, \
	0 { "lock = REQUESTED" }, \
        1 { "lock = FIRST GOT" }, \
        2 { "lock = GOT " }, \
        3 { "lock = GOT after thread_tsleep" }, \
        \* { "lock = NOT GOT" }
+233 1.0 L=KERN "@SPIN UNLOCK (libpthreads)" \
	"pthread_spin_unlock 		lock_addr=" $D1
+104 1.0 L=SVC "@RETURN FROM SYSTEM CALL" {{ $errno = $D1 }} \
        $errno, \
        0  { `return from $SVC` }, \
        \* { `return from $SVC. error $errno%E4` } \
        {{ $SVC = 0 }} \
        endtimer(0x101,$TID)
+460 1.0 L=KERN "@ASSERT WAIT"	\
	"e_assert_wait:   tid="$D1%D1  "anchor="$D2  "flag="$D3%D1  "lr="$D4
+461 1.0 L=KERN "@CLEAR WAIT"	\
	"e_clear_wait:   tid="$D1%D1  "anchor="$D2  "result="$D3%D1  "lr="$D4
+462 1.0 L=KERN "@THREAD BLOCK"	\
	"e_block_thread:   tid="$D1%D1  "anchor="$D2  "t_flags="$D3  "lr="$D4
+463 1.0 L=KERN "@EMPSLEEP"	\
	"e_mpsleep:   tid="prpid($TID)  "anchor="$D1  "timeout="$D2%D1\
	"lock="$D3  "flags="$D4%D1  "lr="$D5%D1
+464 1.0 L=KERN "@EWAKEUPONE"	\
	"e_wakeup_one:   tid="$D1%D1   "anchor="$D2  "lr="$D3
+465 1.0 L=SVC "@THREAD_CREATE SYSTEM CALL" \
	"thread_create:   pid="$D1%D1  "tid="$D2%D1  "priority="$D3%D1  \
	"policy="$D4%D1 pidinstall($D1,0,$EXECPATH,$D2,0xff,0xff)
+466 1.0 L=KERN "@KTHREAD_START"	\
	"kthread_start:   pid="$D1%D1  "tid="$D2%D1  "priority="$D3%D1  \
	"policy="$D4%D1  "func="$D5
+467 1.0 L=SVC "@THREAD_TERMINATE SYSTEM CALL"	\
	"thread_terminate:   pid="$D1%D1  "tid="$D2%D1
+468 1.0 L=KERN "@KSUSPEND"	\
	"ksuspend:   tid="$D1%D1  "p_suspended="$D2%D1  "p_active="$D3%D1
+469 1.0  L=SVC "@THREAD_SETSTATE" \
        "thread_setstate:   tid="$D1%D1  "t_state="prstate($D2)  "t_flags="$D3  \
	"priority="$D4%D1  "policy="$D5%D1
+46A 1.0 L=SVC "@THREAD_TERMINATE_ACK"	\
	"thread_terminate_ack:   current_tid="$D1%D1  "target_tid="$D2%D1
+46B 1.0 L=SVC "@THREAD_SETSCHED"	\
	"thread_setsched:   pid="$D1%D1  "tid="$D2%D1  "priority="$D3%D1 \
	"policy="$D4%D1
+46C 1.0 L=SVC "@TIDSIG"	\
        "tidsig:   pid="$D1%D1  "tid="$D2%D1  "signal="prsignal($D3)  "lr="$D4
+46D 1.0 L=SVC "@WAIT_ON_LOCK"	\
        "wait_on_lock:   pid="$D1%D1  "tid="$D2%D1  "lockaddr="$D3
+46E 1.0 L=SVC "@WAKEUP_LOCK"	\
        "wakeup_lock:   lockaddr="$D1 "waiters="$D2%D1

# This hookid is for TCPIP malloc trace
+340 1.0 '' \
	$HD%D1,	\
        1  { `NET_MALLOC size=$D1%X4 addr=$D2%X4 caller1=$D3%X4 caller2=$D4%X4` }, \
        2  { `NET_FREE   size=$D1%X4 addr=$D2%X4 caller1=$D3%X4 caller2=$D4%X4` }, \
        \*  { unknown subhook $HD%D1 $ERROR }
