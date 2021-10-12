$ @(#)18        1.1  src/bos/diag/da/pciscsi/dpciscsi.msg, dapciscsi, bos41J, 9514A_all 4/4/95 16:09:41
$
$    COMPONENT_NAME: DAPCISCSI
$
$    FUNCTIONS:
$
$    ORIGINS: 27
$
$    (C) COPYRIGHT International Business Machines Corp. 1995
$    All Rights Reserved
$    Licensed Materials - Property of IBM
$    US Government Users Restricted Rights - Use, duplication or
$    disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$
$ GENERAL NOTES
$
$ Do not translate %c, %d, %s, %x, %07X, or \t in any messages.  They
$   are used for word or number substitution and are noted in the
$   comments for the individual messages.  The 1$, 2$, 3$, etc,
$   within the substitutions are used to denote the order of the
$   substitutions.
$
$ These comments concern the TITLE LINES at the top the diagnostic screen.
$   The title must be in all capital letters. The first line
$   of the title cannot be longer than 70 characters starting from
$   column 1.  If the line is greater than 70, it may be continued on
$   the next line.  Leave line spacing as shown:  one blank line after
$   the last title line.  For example:
$
$ *****
$ TESTING PORT 12 OF THE 16-PORT ASYNCHRONOUS ADAPTER IN PLANAR SLOT 2
$ IN ADVANCED MODE
$
$ Please stand by.
$ *****
$
$ These comments concern the user ACTIONS in all caps.
$   If translations require the creation of new lines, begin the
$   new lines in the column immediately following the row of periods.
$   For example:
$
$ *****
$ ACTION.........one line of English might require several when translated, so
$                begin the next line at the same point of the previous line.
$ ACTION.........the next action follows with no blank line preceding it.
$ *****
$
$ The location of a resource is in the form of xx-xx-xx where x is an
$   alpha-numeric character.  The location is not translatable.  It is
$   an alpha-numeric descriptor of where the resource can be found.
$
$ END OF GENERAL NOTES

$quote "

$ =============================================================
$set FRU_SET

RMSG_212 "FIFO empty bit set."
$ FIFO = First In First Out

RMSG_213 "FIFO empty bit clear."
$ FIFO = First In First Out

RMSG_214 "FIFO full bit set."
$ FIFO = First In First Out

RMSG_215 "FIFO full bit clear."
$ FIFO = First In First Out

RMSG_216 "FIFO data miscompare."
$ FIFO = First In First Out

RMSG_217 "SCSI FIFO data miscompare."
$ FIFO = First In First Out

RMSG_218 "SCSI FIFO underflow."
$ FIFO = First In First Out

RMSG_219 "SCSI parity error."

RMSG_220 "SCSI FIFO flags error."
$ FIFO = First In First Out

RMSG_221 "Miscompare during the write/read of the configuration register."
$
RMSG_222 "Error during the write/read of the memory register."
$
RMSG_223 "Miscompare during the write/read of the memory I/O register."
$
RMSG_224 "Error reading the PCI configuration register."
$
RMSG_225 "Adapter POST failed."
$ POST = Power On Self Test

RMSG_230 "Arbitration test failed."
$
RMSG_231 "Function could not complete."
$
RMSG_232 "SCSI bus data miscompare."
$

RMSG_240 "No terminal power."
$ 

RMSG_301 "The parent device open failed."
$
RMSG_302 "Invalid device vendor ID."
$

RMSG_700 "Error Log Analysis indicates a PCI SCSI-2 adapter failure."
$
RMSG_800 "Error Log Analysis indicates a PCI SCSI-2 adapter PTC failure."
$
RMSG_802 "Error Log Analysis indicates a PCI SCSI-2 bus failure."
$

CAB_TERM_DEV   "Cable, terminator, SCSI device"
$
SCSI_SUBSYSTEM "SCSI Subsystem"
$
SW             "Software"
$

$ =============================================================
$set PCISCSI

PCI_SCSI_ADAPTER "PCI SCSI ADAPTER"
$ Device name used in DA messages.

USE_SCSI_PROCEDURE "%1$06.6lx \
ADDITIONAL INFORMATION FOR (%2$s) IN LOCATION %3$s\n\
\n\
A SCSI bus problem has been detected. Use the SCSI Problem Isolation\n\
Procedure for this adapter.\n\
\n\
For more information, refer to the Common Diagnostics and Service Guide\n\
for this system."
$ 1 = Screen number
$ 2 = Device name (scsi0)
$ 3 = Device location (00-00-00)
$

USE_PTC_PROCEDURE "%1$06.6lx \
ADDITIONAL INFORMATION FOR (%2$s) IN LOCATION %3$s\n\
\n\
A PTC problem has been detected. Use the SCSI Problem Isolation Procedure\n\
for this adapter.\n\
\n\
For more information, refer to the Common Diagnostics and Service Guide\n\
for this system."
$ 1 = Screen number
$ 2 = Device name (scsi0)
$ 3 = Device location (00-00-00)
$

USE_ERROR_REPORT_SA "%1$06.6lx \
ADDITIONAL INFORMATION FOR (%2$s) IN LOCATION %3$s\n\
\n\
Error Log Analysis has detected errors logged against this device\n\
within the last 24 hours. At this time the errors do not indicate\n\
a hardware problem that requires a hardware service action.\n\
\n\
For more information about the errors logged, run the Error Report\n\
Service Aid.\n\
\n\
For more information, refer to the Common Diagnostics and Service Guide\n\
for this system."
$ 1 = Screen number
$ 2 = Device name (scsi0)
$ 3 = Device location (00-00-00)
$

