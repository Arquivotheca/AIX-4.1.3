$ @(#)40        1.7.7.2  src/bos/diag/util/umlc/umlc.msg, dsaumlc, bos41J, 9509A_all 2/28/95 16:07:46
$
$   COMPONENT_NAME: DSAUMLC
$
$   FUNCTIONS: Diagnostic message file.
$
$
$   ORIGINS: 27
$
$
$   (C) COPYRIGHT International Business Machines Corp. 1989,1995
$   All Rights Reserved
$   Licensed Materials - Property of IBM
$   US Government Users Restricted Rights - Use, duplication or
$   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
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
$       errors generated from the UMLC Utility.
$
$ END OF GENERAL NOTES

$ $
$ $ Main menu and common messages
$ $
$set MSET
$quote "

MLASTLINE "Move cursor to selection, then press Enter."
$
$ $ Menu Number 802605

MINSTALL_TITLE "PRODUCT TOPOLOGY SERVICE AID"
$
$ See general notes on length of title.
$
$ $ Menu Numbers 802602, 802604

MINSTALL_LAST  "When finished, press Enter."

PROCESS_MSG "\
PRODUCT TOPOLOGY SERVICE AID\n\n\
Processing Product Topology Data.\n\n\
Please stand by."
$
$ Leave line spacing as shown.
$ $ Menu Number 802600

YES_OPTION      "YES"
$
$ $ Menu Number 802605

NO_OPTION       "NO"
$
$ $ Menu Number 802605

MFORMAT_UPDATE_TITLE "\
PRODUCT TOPOLOGY SERVICE AID\n\n\
An error occurred while writing data to the Update Diskette.\n\n\
Do you want to write the Update Diskette again?"
$
$ $ Menu Number 802605
$ $ Uses YES, NO

MSDD "\
REMOVE.........the diskette, if any, from the diskette drive.\n\
INSERT.........the Product Topology Update Diskette into the drive.\n\n\
               If an EC, MES, or external device was installed, use\n\
               the Update Diskette that was shipped with the EC, MES,\n\
               or external device.  Otherwise, use one of the Update\n\
               Diskettes shipped with the system and stored in the\n\
               Service Information Binder."
$
$ See general notes on expanding ACTION lines.
$
$ $ Menu Number 802604
$ $ Uses MINSTALL_TITLE - PRODUCT TOPOLOGY SERVICE AID
$ $ Uses - When finished, press Enter.

EXIT_CANCEL "\
The Product Topology Service Aid detected a CANCEL or EXIT\n\
request, and is terminating.  Changes to the Product Topology\n\
data have not been saved and the Product Topology Update\n\
Diskette will not be created."
$
$ See general notes on expanding ACTION lines.
$
$ $ Menu 802602
$ $ Uses MINSTALL_TITLE - PRODUCT TOPOLOGY SERVICE AID
$ $ Uses END_LAST - To continue, press Enter.

RETURN "\n\
If you wish to return to the Product Topology Service Aid, press\n\
Enter or Cancel."
$
$ See general notes on expanding ACTION lines.
$ $ Menu Number 802602

EXIT "\n\
If you wish to exit the Product Topology Service Aid, press Exit."
$
$ See general notes on expanding ACTION lines.
$ $ Menu Number 802602

$
$set USET

MNEWLAST "\
When finished, or when no further information is available,\n\
use 'Commit' to accept the data."
$
$ $ Menu Numbers 802601, 802603

MNEWTITLE "\
PRODUCT TOPOLOGY SCSI DEVICE LOCATION\n\n\
A new SCSI device was detected that is not in the Product Topology\n\
Data.  Please enter the SCSI device location ('IN' for internal,\n\
'EX' for external) for the following device:\n\n\
     Title................ %1$s\n\
     Part Number.......... %2$s\n\
     Part Location........ %3$s\n\
     Resource name........ %4$s\n\
     EC Level............. %5$s\n\
     Serial Number........ %6$s\n\
     Displayable Message.. %7$s"
$
$ SCSI stands for Small Computer Systems Interface.
$ 'IN' and 'EX' are the two-letter SCSI location codes, and should not
$   be translated.
$ %1$s is a variable that will be substituted with the device name from
$   devices.cat.
$ Leave line spacing as shown.
$ $ Menu Number 802603


$
$set SSET

SYSTITLE "\
PRODUCT TOPOLOGY SYSTEM RECORD\n\n\
The following information lists the current system record data.\n\
Any data in brackets [] may be changed or added at this time.\n\
Menu items with an asterisk (*) require a valid data entry."
$
$ Leave line spacing as shown.  See general notes on length of title.
$ $ Menu Number 802601


LO_MSG "SCSI Device Location (Internal or External)..."
SE_MSG "* Machine serial number...ie. 2612345........."
TM_MSG "* Machine type ...........ie. 7015............"
T2_MSG "* Machine model...........ie. 98F............."
L1_MSG "* Name (may be individual or company) ........"
L2_MSG "  Address (number and street) ................"
L3_MSG "  Address (city and state) ..................."
L4_MSG "  Address (zip) .............................."
L5_MSG "  Contact Name ..............................."
L6_MSG "  Contact Phone ..........ie. 512-555-1212...."
SY_MSG "  System number...........ie. 0000HBF........."
CN_MSG "  Customer number.........ie. 1234567........."
EA_MSG "  Electronic address.........................."
$
$       123456789*123456789*123456789*123456789*12345| < 46 character line
$ Text should not be more than 46 characters per line.

$
$set ESET

MEM_ERR "\
** FATAL ERROR --> out of memory **"

INTERNAL_ERR "\
Internal Error in Product Topology Files - \
contact your support center"

LOAD_ERR "\
An error occurred while attempting to load the database - \
contact your support center"

INVALID_PARM_ERR "\n\n\
An invalid parameter was encountered.\n"

USAGE_ERR "\n\
The format of the umlc command is:\n\
   umlc [ -? | -h] | [ [[-e] -f filename] | [-r serial# type]]\n\n\
filename -- the name of the file that the Product\n\
            Topology Update data will be written\n\
            to in %1$s.\n\
            Filename should not contain any path\n\
            information.\n\
serial#  -- the serial number of this System.\n\
type     -- model number and type of this System.\n"
$
$ %1$s is the directory /etc/lpp/diagnostics/data

LO_ERR "\
You must identify the SCSI device as either\n\
internal or external before you can select\n\
'Commit' to continue."
$
$ SCSI stands for Small Computer Systems Interface

SE_ERR "\
The machine serial number field is missing or\n\
invalid.  A valid serial number must be entered\n\
before processing can continue.\n\n\
The format for a machine serial number is 2\n\
numeric digits (representing the plant of\n\
manufacture), followed by 5 alphanumeric\n\
characters.\n\n\
Some examples of a valid serial number include:\n\
2611010\n\
4406516\n\
901A041"

TYPE_ERR "\
The machine type field is missing or invalid.\n\
A valid machine type must be entered before\n\
processing can continue.\n\n\
The format for a machine type is 4 alphanumeric\n\
characters.\n\n\
Some examples of a valid machine type include:\n\
7015\n\
7012\n\
7011"

MODEL_ERR "\
The model number field is missing or invalid.\n\
A valid model number must be entered before\n\
processing can continue.\n\n\
The format for a model number is 3 alphanumeric\n\
characters.\n\n\
Some examples of a valid model number include:\n\
980\n\
M20\n\
53H"

NAME_ERR "\
The name field must be filled in.  The name field\n\
should contain the name of the company or person\n\
that owns this machine."

OUT_OF_SPACE "\
The root (/) file system does not contain enough\n\
free space to write the Product Topology data.\n\
Use SMIT or the 'chfs' command to add additional\n\
space to the root (/) file system, then re-run\n\
Product Topology."

NO_DIR_ERR "\
WARNING: The Product Topology Update Diskette\n\
will *NOT* be produced because an error occurred\n\
while attempting to create the directory:\n\
   /etc/lpp/diagnostics/data\n\n\
Please exit to a command line and attempt to\n\
create this directory.  If you are unable to\n\
create the directory, or if you are running from\n\
standalone removable media (CD, Tape, etc.),\n\
please contact your support center."


$
$set END_SET

END_TITLE "\
PRODUCT TOPOLOGY SERVICE AID"
$
$ $ Menu Number 802606

END_LAST "\
To continue, press Enter."
$
$ $ Menu Number 802606

END_TEXT "\
Processing is complete.\n\n\
RETURN.........the processed Update Diskette(s) to your service center.\n\n\
NOTE: If you used a blank formatted disk instead of one of the diskettes\n\
      from the Service Information Binder, please be sure to label the\n\
      newly created diskette as 'Product Topology Update Diskette'.\n\n\n\
  For service personnel and customers within the USA:\n\
      Place the Product Topology Update Diskette for each\n\
      machine you are installing into the self-addressed\n\
      pre-paid mailer provided and drop in any mailbox.\n\n\
  For service personnel outside the USA:\n\
      Follow your local procedures for return of the\n\
      Update Diskettes.\n\n\
  For customers outside the USA:\n\
      Place the Product Topology Update Diskette for each\n\
      machine you are installing into the self-addressed\n\
      pre-paid mailer provided, but DO NOT MAIL.  Give the\n\
      mailer to your service personnel or other customer\n\
      service representative on his or her next visit."
$
$ USA stands for United States of America.
$ Maintain spacing as shown.
$ $ Menu Number 802606
$ $ Uses END_LAST - To continue, press Enter.

$
$set HELP_SET

HELP_802601_OLD " "
$ This message has been replaced.  See "HELP_802601" below for new message.

HELP_802603 "\
The system has detected a new SCSI device,\n\
but the system VPD data does not adequately\n\
identify the device as being either internal\n\
or external.  The 'SCSI device location' field\n\
should be modified by pressing the 'List' key\n\
and selecting the correct location code from\n\
the list of choices -- 'IN' stands for internal,\n\
'EX' stands for external.  You must select\n\
either 'IN' or 'EX' before you are allowed to\n\
'Commit' the changes and continue processing."
$
$ 3456789*123456789*123456789*123456789*12345678|      < 49 character line
$
$ SCSI stands for Small Computer Systems Interface
$ VPD stands for Vital Product Data
$
$ Text should not be more than 49 characters per line.
$ Maintain spacing as shown.

HELP_802604 "\
The system is requesting an Update Diskette to\n\
output the new machine configuration. If you\n\
have just installed a new feature or resource,\n\
use the Update Diskette that was shipped with\n\
the resource.\n\n\
If more than one resource has been installed,\n\
select one of the Update Diskettes to output\n\
the data. Return this and all of the remaining\n\
Update Diskettes shipped with the new resources\n\
to your service center in the mailers provided.\n\n\
If no new features or resources are being\n\
installed at this time, use one of the Update\n\
Diskettes in the '4 pack' shipped with the\n\
system and stored in the Service Information\n\
Binder."
$
$ 3456789*123456789*123456789*123456789*12345678|      < 49 character line
$ Text should not be more than 49 characters per line.
$ Maintain spacing as shown.

HELP_802605 "\
An error has occurred while trying to write a\n\
diskette.  This may be because the diskette\n\
is bad, an incorrect diskette was placed in\n\
the drive, or the write-protect tab is in the\n\
read-only position. Check the diskette in\n\
the drive to ensure that the correct diskette\n\
has been inserted and that the write-protect\n\
tab is set to allow writing to the diskette.\n\n\
If the diskette in the drive appears to be\n\
the correct diskette, and the write-protect\n\
tab has been st to allow writing to the\n\
diskette, then the diskette may be bad.  If\n\
this is the case, or if the correct diskette\n\
cannot be located, then insert a blank\n\
formatted diskette into the diskette drive\n\
and select 'YES'.\n\n\
If you do not have the correct diskette or a\n\
blank formatted diskette, then select 'NO'.\n\n\
NOTE:  If a blank diskette is substituted\n\
  for the Update Diskette, then be sure and\n\
  mark the new diskette with 'Product Topology\n\
  Update Diskette' to indicate the contents of\n\
  the diskette."
$
$ 3456789*123456789*123456789*123456789*12345678|      < 49 character line
$ Text should not be more than 49 characters per line.
$ Maintain spacing as shown.

HELP_802601 "\
The information entered on this screen is used\n\
by the service personnel to identify the machine\n\
that the Product Topology data came from.\n\n\
The Machine serial number is a 7-character\n\
identification of the machine.  The serial number\n\
is unique for all machines manufactured.  The\n\
first two characters of the serial number are\n\
numeric (representing the plant of manufacture),\n\
and the remaining 5 characters are alphanumeric.\n\
This field is required, and once Committed cannot\n\
be changed.\n\n\
The Machine type is a 4-character identification\n\
of the system characteristics.  The format of\n\
the Machine type field is 4 alphanumeric\n\
characters.  This field is required, but may be\n\
changed at any time.\n\n\
The Model number is a 3-character identification\n\
that further refines the information obtained\n\
from the Machine type field.  If a processor\n\
upgrade has been applied to this machine, the\n\
model number for the processor upgrade should\n\
be used.  This field is required, but may be\n\
changed at any time.\n\n\
The Name and address fields are for the\n\
specification of machine location and contact\n\
information.  Please be as specific as possible\n\
when entering these fields.\n\n\
The Name field is for the name of the company or\n\
individual that this machine belongs to.  This\n\
field is required, but may be changed at any\n\
time.\n\n\
The Address fields are for the mailing address\n\
of the company or individual that this machine\n\
belongs to.\n\n\
The Contact Name field is for the name of the\n\
technical representative that should be\n\
contacted for any questions regarding this\n\
machine.\n\n\
The Contact Phone field is for the phone number\n\
of the contact person.  The phone number should\n\
also include the area code.\n\n\
The System number field is a number assigned by\n\
the order process.\n\n\
The Customer number field is the customer number\n\
of the customer placing the order.\n\n\
The Electronic address field is the address of\n\
this machine.  This field is used when\n\
Electronic Customer Support is being used to\n\
remotely access the Product Topology data.\n\
Examples of an electronic address include\n\
'129.35.200.200', or 'umlc.austin.ibm.com'"
$
$ 3456789*123456789*123456789*123456789*12345678|      < 49 character line
$ Text should not be more than 49 characters per line.
$ Maintain spacing as shown.

