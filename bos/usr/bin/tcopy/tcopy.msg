$ @(#)05	1.6.1.1  src/bos/usr/bin/tcopy/tcopy.msg, cmdarch, bos411, 9428A410j 7/22/93 16:20:11
$
$ COMPONENT_NAME: (CMDARCH) archive files
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
$set MS_TCOPY
$               GLOBAL COMMENT FOR TRANSLATORS
$ Do not translate tcopy -- this is the command name.
$ Do not translate %d %s etc.
$ These are output formats for file names, directory
$ names, permission codes, command names, and the like,
$ depending on their context within each message.
$ The program replaces %d,%1$d,etc with a decimal number.
$ The program replaces %s,%1$s,etc with a string of
$ characters.

CANTOP "tcopy: 0511-550 Cannot open %s.\n"

COPYAB "tcopy: 0511-551 The tape copy has stopped before completion.\n"

ENDTAPE "tcopy: The end of the tape is reached.\n"

LENTOT "tcopy: The total tape length is %ld bytes.\n"

RBOUTF "tcopy: 0511-552 Tape damaged at File: %1$d; Record: %2$ld.\n"

STATFR "tcopy: Tape File: %1$d; Records: %2$ld to %3$ld; Size: %4$d.\n"
$ this is a tape status line for a copy of multiple records
$ It could appear for either the source tape or the destination tape

STATRS "tcopy: Tape File: %1$d; Record: %2$ld; Size %3$d.\n"
$ this is a tape status line for a copy of 1 record
$ It could appear for either the source tape or the destination tape

STATEOT "tcopy: File: %1$d; End of File after: %2$ld Records, %3$ld Bytes.\n"
$ EOF is an end of file status
$ It could appear for either the source tape or the destination tape

USAGE "Usage: tcopy SourceTape [DestinationTape]\n"

WEOF "tcopy: An End of File marker is written. The copy is complete.\n"
$ WEOF is no longer used, it is replaced by WEOFBAD since this message
$ is almost opposite of what has really occured.

WNOTR "tcopy: 0511-553 Bytes written (%1$d) not equal to bytes read (%2$d).\n"

WEOFBAD "tcopy: 0511-554 Write EOF failed"
$ WEOFBAD replaces the incorrect WEOF message.

WRITEBAD "tcopy: 0511-555 Write failed"
$ Writting the destination tape failed.
