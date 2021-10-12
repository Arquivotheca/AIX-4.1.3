$ @(#)89        1.4  src/bos/sbin/helpers/v3fshelpers/op_extend.msg, cmdfs, bos411, 9428A410j 2/21/94 16:58:12
$
$ COMPONENT_NAME: (CMDFS) commands that deal with the filesystem
$
$ FUNCTIONS: op_extend msg file
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1993
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$quote "

$set MS_OP_EXTEND

$ Do not add newlines to these messages


NOTJFS		"%s:  0506-480  %s is not recognized as a JFS filesystem.\n"
$ Do not translate JFS

NOTVERSION	"%s:  0506-481  %s is not a supported JFS filesystem version.\n"
$ Do not translate JFS

CORRUPTJFS	"%s:  0506-482  %s contains a corrupt JFS filesystem superblock.\n"

NOREADSUPER	"%s:  506-483  Could not read the superblock on %s.\n"

EXTFAIL		"%s:  506-484  Extend failed. Free some space and try again.\n"

MALLOC		"%s:  506-485  malloc failed.\n"
$ do not translate malloc

RWERROR		"%1$s:  506-486  %2$s error on block %3$ld\n"

STANZA		"%s:  506-487  a %s stanza must exist for %s.\n"

MAXMAP		"%s:  506-488  new size is larger than MAXMAPSIZE (%d)\n"
$ Do not translate MAXMAPSIZE

TOOBIG_NBPI	"%s:  506-489 file system with nbpi = %d cannot exceed %u 512-byte blocks\n"

NOTBIG		"%s:  506-490  logical volume  %s is not large enough\n"

TOOBIG_FRAG	"%s:  506-491  A file system with frag = %d cannot exceed %u 512-byte blocks\n"
