$ $ * @(#)47       1.5  src/bos/usr/lib/methods/common/cfgchk.msg, cfgmethods, bos411, 9433B411a 8/16/94 20:05:26
$
$   COMPONENT_NAME: CFGMETHODS
$
$   FUNCTIONS: 
$
$   ORIGINS: 27
$
$
$   (C) COPYRIGHT International Business Machines Corp. 1989,1994
$   All Rights Reserved
$   Licensed Materials - Property of IBM
$   US Government Users Restricted Rights - Use, duplication or
$   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ 
$
$ GENERAL NOTES
$
$ Do not translate %d, %s, %x, %07X, or \t in any messages.  They
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
$       This file contains the corresponding messages for the menus and
$	errors generated from the UMBLIST Utility.
$
$ END OF GENERAL NOTES

$quote "
$set ERROR_SET

ERROR1 "\
*** WARNING! ***\n\n\
A POTENTIAL CONFLICT EXISTS IN YOUR SYSTEM HARDWARE\n\
CONFIGURATION.  OPERATING YOUR SYSTEM WITH THIS\n\
CONFIGURATION MAY RESULT IN UNDETECTED DATA LOSS.\n\
CONTACT YOUR SERVICE REPRESENTATIVE AND REPORT\n\
THE FOLLOWING SERVICE REQUEST NUMBER:\n%s\n"
$
$ This message must be capitalized when tranlation permits.

ERROR2 "\
*** WARNING! ***\n\n\
A POTENTIAL CONFLICT EXISTS IN YOUR SYSTEM HARDWARE\n\
CONFIGURATION.  OPERATING YOUR SYSTEM WITH THIS\n\
CONFIGURATION MAY RESULT IN UNDETECTED DATA LOSS.\n\
CONTACT YOUR SERVICE REPRESENTATIVE AND REPORT\n\
THE FOLLOWING SERVICE REQUEST NUMBERS:\n%s\n"
$
$ This message must be capitalized when tranlation permits.

ERROR3 "cfgchk():  Error retrieving ODM data.\n"
$
$ "cfgchk()" is a routine name and should not be translated
$ ODM stands for Object Data Manager and does not have to be translated

ERROR4 "cfgchk():  Error retrieving NVRAM data.\n"
$
$ "cfgchk()" is a routine name and should not be translated
$ NVRAM stands for Non-Volatile Read Only Memory 
$ NVRAM does not have to be translated.

ERROR5 "\
*** WARNING! ***\n\n\
THE MICROCODE PROGRAM REQUIRED BY A SYSTEM FEATURE\n\
CARD OR CARDS FAILED TO DOWNLOAD PROPERLY.  ALTHOUGH\n\
THE SYSTEM WILL CONTINUE TO RUN, OPERATING YOUR SYSTEM\n\
WITHOUT THE PROPER MICROCODE MAY RESULT IN UNDETECTED\n\
DATA LOSS.  SHUTDOWN AND RE-BOOT THE SYSTEM.  IF THE\n\
PROBLEM REOCCURS, PLEASE RUN YOUR SYSTEM DIAGNOSTICS\n\
PROGRAM ON THE RESOURCES IDENTIFIED BELOW TO DETERMINE\n\
THE PROBLEM:\n%s\n\n"
$
$ This message must be capitalized when tranlation permits.

ERROR6 "\n\n\
THIS OPERATING SYSTEM PRODUCT IS NOT SUPPORTED ON THIS\n\
HARDWARE CONFIGURATION.\n\
Contact your Product or Service Representative and report\n\
the model code shown below to assist in obtaining the correct operating\n\
system product for this hardware configuration.\n\
MODEL CODE: %s\n\n\
*** SYSTEM HALTED ***\n\n"
$
$ The first sentence of this message should be capitalized when translation
$ permits.
