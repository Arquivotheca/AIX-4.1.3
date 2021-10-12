$ @(#)34	1.6.1.10  src/bos/sbin/helpers/v3fshelpers/op_check.msg, cmdfs, bos411, 9428A410k 7/14/94 10:16:28
$
$ COMPONENT_NAME: (CMDFS) commands that deal with the filesystem
$
$ FUNCTIONS:
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989, 1993
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$quote "

$set MS_OP_CHECK

$ Do not add newlines to these messages

ADJUST	"ADJUST"

ADJUSTED	"ADJUSTED"

ALDIR	"allocdir: cannot get inode %ld"
$ do not translate "allocdir"

ALLOCATED	"        --> %s Map bit %d allocated in map"

ALLOCINO	"allocino: cannot get inode %ld"
$ do not translate "allocino"

BADBLK	"Block %ld: %s Inode=%u fragment=%x"

BADENT	"Directory %s, '%s' entry %s"

BADFMAP	"Bad %s Map"

BADVERSION	"%%_OS_%% filesystem, Incompatible version number."

BLK1MAP	"Cannot get first block of %s Map"

BLKCLB	"Block number should be on a cluster boundary"

BLKCNTW	"Block count wrong, Inode=%u"

BLKMISS	"%ld blocks missing"

BLKNOT	"Block size not %d"

BLOCK	"Block"

CACHEP	"--- Cache Performance %ld/%ld with %d buffers"

CANTEXP	"Cannot expand %s: filesystem full"

CANTEXPDIR "Cannot expand %s."

CANTGET	"Cannot get %s directory"

CANTOPEN	"cannot open \"%s\".%s."

CANTRESUP	"Unable to read superblock"

CHECKING	"\n** Checking %s "

CLEAR1	"CLEAR"

CLEAR1ED	"CLEARED"

CLNMNT	"** %s cleanly - Check suppressed"

CNTMAPB	"Cannot map in block %ld"

CONTINUE "CONTINUE"

CONTINUED "CONTINUED"

CORRUPTFRAG	"corrupt disk address"

CORRUPTSUPER	"Corrupt superblock."

COUNTC	" count %d should be %d"

CREATEM	"CREATE"

CREATEMD	"CREATED"

DIRWORD	"directory"

DIRCONN	"Directory Inode=%u connected, parent was Inode=%u"

DIRCOR	"DIRECTORY CORRUPTED"

DIRLENM	"DIRECTORY %s: LENGTH %d NOT MULTIPLE OF %d"

DIRSHRT	"DIRECTORY TOO SHORT"

DUPBAD	"Duplicate or invalid"

DUPINRI	"Duplicate or invalid data in root inode"

DUPLICATE	"duplicate"

DUPOVER	"Duplicate table overflow."

EMBBAD	"Illegal character (\\0%o) embedded in pathname '%s'"

EMBNULL	"Embedded nulls in directory entry for '%s'"

ENTERSC	"Enter name of scratch file (%ld blks): "

EXCESD	"Excessive duplicate blocks Inode=%u"

EXCESSB	"Excessive invalid blocks Inode=%u"

EXCESSBADALLOC	"Excessive illegally allocated fragments (Inode=%u)"

EXCESSCORRUPT	"Excessive corrupt disk addresses (Inode=%u)"

EXCESSMIDFRAG	"Excessive partial blocks found before the last block (Inode=%u)"

EXPAND	"EXPAND"

EXPANDED	"EXPANDED"

FATIO	"Fatal I/O error"

FILE	"file"

FILENOFRAG	"Fragment allocated to file larger than 32k (Inode=%u)"

FIX	"FIX"

FIXED	"FIXED"

FOUNDB	"Found reference to file containing block %lu, name=%s"

FOUNDF	"Found reference to inode %u, name=%s"

FRAGBADALLOC	"illegally allocated fragment"

FSIZEER	"File size error Inode=%u"

FSYSBAD	"Filesystem integrity is not guaranteed"

FSYSSTAT	"--- Filesystem size: %6ld\tInode list size:%6ld"

FSYSSTAT1	"--- Cluster: %6d\tInodes:  %6u"

FTOOLARG	"Filesystem too large to be checked in memory"

IMAPER	"Cannot get inode for %s Map file"

INCOMPMAP	"Incompatible map version number"

INODE	"Inode"

INVLD	"invalid"

LNCNT	"link count %s"

LNKOVR	"Link count table overflow"

MAPBADAGSIZE	"	 --> Map page %d, agsize = %d"

MAPBADCLSIZE	"	 --> Map page %d, clsize = %d"

MAPBADVERS	"	 --> Map page %d, version = %d"

MAPINC	"%s Map inconsistent: size = %ld, mapsize = %ld."

MIDFRAG	"partial block found before the last block"

MISMATTREE	"Tree size has changed - cannot continue."

MISS	"is missing."

MNTCTLE	"mntctl failed: %s"

MNTD	"Mounted"

MNTFSYS	"MOUNTED FILE SYSTEM; "

MNTFSYS2	"Checking a mounted filesystem does not produce dependable results."

MODIFIED	"***** Filesystem was modified *****"

MODMOUNT	"*** Mounted filesystem has been modified - REBOOT (NO SYNC!) ***"

MTIMEM	"mode=%o\nsize=%ld mtime=%s "

NAME	"name"

NODATA	"(CONTAINS NO DATA)"

NODIRECT	"Cannot expand %s: no direct blocks left"

NOLEFT	"NO SPACE LEFT IN %s"

NOLFD	"There is no lost+found directory."

NOMEME	"Insufficient memory"

NOMKDIR	"CANNOT CREATE %s DIRECTORY"

NOSPACE	"out of space"

NOTAIX3	"\nNot a recognized filesystem type."

OWNER	" owner="

P_ALLOCATED	"        --> %s Map bit %d allocated in pmap, not in wmap"

PHASE0	"** Phase 0 - Check Log"

PHASE1	"** Phase 1 - Check Blocks and Sizes"

PHASE1B	"** Phase 1b - Rescan For More Duplicate Blocks"

PHASE2	"** Phase 2 - Check Pathnames"

PHASE3	"** Phase 3 - Check Connectivity"

PHASE4	"** Phase 4 - Check Reference Counts"

PHASEX	"** Phase %db - Salvage %s Map"

PHASEXX	"** Phase %d - Check %s Map"

PHYBLK	"Cannot read physical block %ld of %s Map"

RANGEE	"Inode number out of range"

REALLOCATE	"REALLOCATE"

REALLOCATED	"REALLOCATED"

RECON	"RECONNECT"

RECONED	"RECONNECTED"

REMOVE	"REMOVE"

REMOVED	"REMOVED"

RINOTDIR	"Root inode not directory"

RIUNALLOC	"Root inode unallocated."

RWERR1	"Cannot fbwrite: block %d"

RWERR2	"Cannot seek: block %d"

RWERR3	"Cannot read: block %d"

RWERR4	"Cannot write: block %d"

RWERR9	"unknown disk transfer error: block %d"

SALV	"SALVAGE"

SALVAGED	"SALVAGED"

SBLKDIR	"Superblock is marked dirty"

SCREQ	"Filesystem too large, scratch file required"

SCRWRE	"Write error on scratch file"

SIZECHK	"Size check: fsize %ld"

SIZEWRONG	"Size wrong, Inode=%u"

SNOTDIR	"%s IS NOT A DIRECTORY"

SUMMARY	"%d files %ld blocks %ld free"

TBLINCORE	"--- Tables allocated in memory: %ld bytes"

TBLPAGE	"--- Tables paged out to file: %s, %d buffers in cache"

TBLSIZ	"--- Table sizes: BM=%ld + ST=%ld + LC=%ld, TOTAL=%ld"

TERMIN	" (TERMINATED)"

TOOMANYI	"Too many -i specifications"

TOOMNYD	"Too many -d specifications"

UNALLOC	"Unallocated"

UNALLOCATED	"        --> %s Map bit %d un-allocated in map"

UNKFLAG	"\"%s\": unrecognized flag\n"

UNKFTYPE	"Unknown file type"

UNLABELFS	"unlabeled filesystem"

UNMNTD	"Unmounted"

UNREF	"Unreferenced"

UNREFS	"Unreferenced %s "

USETFLAG	"Insufficient memory, -t flag required"

WRONG1	"refers to the wrong inode."

WRSUP	"WRITING SUPPRESSED; "

ZEROLDIR	"ZERO LENGTH DIRECTORY"

LOGREDO_FAIL	"logredo failed, fsck continuing"
