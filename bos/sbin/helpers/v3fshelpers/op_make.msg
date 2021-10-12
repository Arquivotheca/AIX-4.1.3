$ @(#)37	1.7.1.8  src/bos/sbin/helpers/v3fshelpers/op_make.msg, cmdfs, bos411, 9428A410j 3/18/94 08:59:09
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
$
$ op_make's messages
$

$set MS_OPMAKE_ERR		error messages / return codes
$quote "

$ Do not add newlines to these messages.

FSHERR_FS2SMALL     "0506-226 The filesystem size is too small"

FSHERR_FS2BIG	    "0506-227 The filesystem size is bigger than the logical volume size"

FSHERR_BOOTBAD	    "0506-228 The boot program is not accessible"

FSHERR_BOOT2BIG	    "0506-229 The boot program is too big"

FSHERR_NOLABEL      "0506-230 Specify a filesystem label or volume label"

FSHERR_NOBOOT	    "0506-231 Specify a boot program"

FSHERR_PROTOAXS     "0506-232 Cannot find or access prototype file"

FSHERR_PROTOREAD    "0506-233 Cannot read prototype file"

FSHERR_PROTOPN      "0506-234 Cannot find or open prototype file"

FSHERR_PROTOSTAT    "0506-235 Cannot get information about prototype file"

FSHERR_PROTOEMPTY   "0506-236 The prototype file is empty"

FSHERR_PROTOERR     "0506-237 There is an error on line %d of the prototype file"

FSHERR_LOGCREATE    "0506-238 Cannot create log"

FSHERR_LOGFORM      "0506-239 Cannot format log"

FSHERR_FILE2BIG     "0506-240 A file is too large"

FSHERR_STRCPYFAIL   "0506-241 Internal error, strcpy failed"

FSHERR_FSTATFAIL    "0506-242 Internal error, fstat failed"

FSHERR_ALLOCMAPFAIL "0506-243 Internal error, alloc_map failed"

FSHERR_DBGETSIZE   "0506-244 Internal error, dbget called with invalid size"

FSHERR_BOOTPROGRW  "0506-245 Internal error, can't write out boot program"

FSHERR_PRCOUNTMODE  "0506-246 An unknown mode was passed to proto_count"

FSHERR_PRALLOCMODE  "0506-247 An unknown mode was passed to proto_alloc"

FSHERR_LKUPARGSBAD  "0506-248 Internal error, lookup's args are bad or Rootdir is unallocated"

FSHERR_LTOPSIZEBAD  "0506-249 Internal error, ltop called with inconsistent size"

FSHERR_BWRITESIZE   "0506-250 Internal error, bwrite called with inconsistent size"

FSHERR_DUP2FAIL		"0506-251 Internal error, dup2 failed"

FSHERR_EXECFAIL		"0506-253 Internal error, exec failed"

FSHERR_NBPI        "%s: 0506-252 A file system with nbpi = %d cannot exceed %d 512-byte blocks\n"


FSHERR_BADOPT		"0506-254 Invalid option specified."

FSHERR_BADSYNTAX	"0506-255 Invalid option syntax."

FSHERR_BADNBPI		"0506-256 The nbpi value must be 512, 1024, 2048, 4096, 8192, or 16384."

FSHERR_BADFRAGSIZE	"%s: 0506-257 The fragment size must be 512, 1024, 2048, or 4096."

FSHERR_NOINODES	    "0506-258 The filesystem ran out of inodes."

FSHERR_NOBLOCKS	    "0506-259 The filesystem ran out of blocks."

FSHERR_DEVIOCTLFAIL "0506-262 Internal error, ioctl of device failed."

FSHERR_GETPUTINOFAIL	"0506-263 Internal error, get/put inode failed."

FSHERR_GETPUTSUPFAIL    "0506-264 Internal error, get/put super failed."

FSHERR_ADDNAMEFAIL	"0506-265 Internal error, add_name failed."

FSHERR_NOTJFS		"0506-266 Not a JFS file system."

FSHERR_INVALVERS	"0506-267 Invalid superblock version number."

FSHERR_CORRUPTSUPER	"0506-268 Corrupt superblock."

FSHERR_BRDWRFAIL	"0506-269 Internal error, bread/bwrite failed."

FSHERR_LTOPFAIL		"0506-270 Internal error, ltop failed."

FSHERR_NEWBLKFAIL	"0506-271 Internal error, newblk failed."

FSHERR_COMPRESS		"0506-272 Unrecognized compression algorithm"

FSHERR_COMFRAGSIZE	"0506-273 Fragment size must be 2048 or less"

FSHERR_COMPROTO		"0506-274 Compression not supported for proto option"

FSHERR_FRAG	        "%s: 0506-275 A file system with frag = %d cannot exceed %d 512-byte blocks\n"

FSHERR_CANTMAKENBPI	"%s: A file system with size of %d 512-byte blocks cannot have\n\t    nbpi = %d.  Either increase file system size or decrease nbpi.\n"

FSHERR_COMPNOTLOADED	"0506-277 The compression algorithm is either not present or not loaded\n          into the kernel.\n"

$set MS_OPMAKE_WARN    warnings
$quote "

WARN_WASTE       "0506-260 Warning: %d (%d-byte) blocks wasted.\n"

WARN_MAXFS	"\n0506-276 JFS file systems have the following size limitations:\n"

WARN_FSMAX_TABLE	"\nNBPI\tFragment Size\t\tMaximum Size (512-byte blocks)\n"

$set MS_OPMAKE_STAT    status messages
$quote "

AT "at"

STAT_SHORTFORMAT "%-13s %s\n"

STAT_INDENTFMT   "  %s\n"

STAT_LONGFMT     "  %-15s %s\n"

STAT_LONGFMT1    "  %-15s %d\n"

STAT_DEV         "Device %s:\n"

STAT_MOUNT       "Mount point:"

STAT_LAB         "Label:"

STAT_VOLAB       "Volume Label:"

STAT_BOOT        "Boot program:"

STAT_STDMT       "Standard empty filesystem"

STAT_SIZE1       "  %-15s %d %d-byte (UBSIZE) blocks\n"

STAT_SIZE2       "Size:"

STAT_INODES      "Initial Inodes:"



$set MS_OPMAKE_PROTO    prototype file failures
$quote "

PROTO_AXS          "access failed"

PROTO_STAT         "stat failed"

PROTO_OPEN         "open failed"

PROTO_DUPNAME      "duplicate name"

PROTO_NOBRANCH     "branch does not exist"

PROTO_BRANCHNOTDIR "branch is not a directory"

PROTO_MATCHDIR     "matched a directory"

PROTO_NOTARGET     "target not found"

PROTO_PREMATUREOF  "premature EOF"

PROTO_BADMODELEN   "bad mode length"

PROTO_BADMODECHAR  "bad mode character"

PROTO_BADCONV      "bad conversion"

PROTO_INTERNALMEM  "out of memory"

PROTO_INTERNALSCPY "strncpy failed"
