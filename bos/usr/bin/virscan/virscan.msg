$ @(#)33	1.5  src/bos/usr/bin/virscan/virscan.msg, cmdsvir, bos411, 9428A410j 9/1/93 13:47:48
$
$ COMPONENT_NAME: CMDSVIR
$
$ FUNCTIONS: NONE
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989, 1991
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$quote "         define quote character.
$len
$set MS_VIRSCAN        Define initial set#
$
$
$ !!!!!!!!!!!!! COMMENTS TO TRANSLATORS !!!!!!!!!!!!!!!!
$
$ DO NOT translate messages that precede the comment 
$   "PC-DOS specific message string"
$
$ Virscan has a PC-DOS counterpart.  As such, many of the messages
$ that appear in this file, do not apply to this AIX version.  For this
$ reason do not translate the "PC-DOS specific" messages
$
$ Do not try to alter or translate %s, %d, %ld, as these are formatting
$ constructs.  They are used by the "virscan" program as format
$ specifications and will appear as character strings or decimals.
$ Do not try to alter or translate escaped characters such as
$ "\n" or "\t" as these often carry line positioning information.
$
$ Do not try to translate any occurence of the word "virscan".  Virscan
$ is the name of this program and is short for "Virus scanner".  The
$ function of this program is to scan files on your computer and compare
$ against known instances of computer viruses previously found.  A
$ warning message is displayed if a virus is found.
$
$ Do not try to change the construct "|" as this means "or"
$ !!!!!!!!!!!!!!!! END OF COMMENTS !!!!!!!!!!!!!!!!!!!!!
$
$ CRCVMARK6C40E508
$ The virus scanning program uses information on the previous line
$ to detect modifications to this file.
$
$

VERSION_FLAG	"VERSION=IUO"
$ Do not translate

COPYRIGHT	"\
(c) Copyright IBM Corporation 1989, 1990, 1991\n"

PRESS_ANY_KEY	"--- Press any key to continue ---"

PRESS_ENTER	"--- Press <enter> to continue ---"
$ The above word <enter> refers to the "Return or Enter" key 
$ found on middle right side of US keyboard

USAGE_01	"Usage: virscan options\n"
USAGE_02	"options: "
USAGE_03	"Explanation of options:\n"

USAGE_DOPTION 	"[-d]drive: || "
$ PC-DOS specific message string

USAGE_LOPTION	"-llistfile"
USAGE_TOPTION 	" | -tsinglefile | directory"
$ The previous options refer to the "-l" which must be succeeded by
$ a file that contains a list of files to scan, and the "-t" which must be
$ succeeded by a single file name which is to be scanned.
$ You may chose to not translate this.  

USAGE_BOPTION	" || -bdrive:"
$ PC-DOS specific message string

USAGE_SPQOPTION "\
\t[-ssignaturefile] [-ppositivefile] [-quiet] [-verbose]"
$ The previous options refer to the "-s" which must be succeeded by a
$ filename that will contain signatures of known viruses, the "-p" which
$ must be succeeded by a filename that will be used to output files
$ that test positive as viruses.  The options "-quiet" and "-verbose"
$ may be shortened to "-q" and "-v" respectively. 

USAGE_WAROPTION	"\
[-wwildcardfilespec] [-a] [-rdrive:] "
$ PC-DOS specific message string

USAGE_WAOPTION	"[-w\"wildcard\"] [-a] "
$ The previous option refers to the "-w" which must be succeeded by a
$ wildcard character string.  These strings contain character instances
$ that may match multiple characters, such as "*.*" matches "wild.wild"
$ or ".bogus"

USAGE_DRIVE01	"\
[-d]drive:\t: Scan a PC-DOS or OS/2 logical drive.\n"
$ PC-DOS specific message string

USAGE_B01	"\
-bdrive:\t: Scan system boot sector of specified logical drive.\n"
$ PC-DOS specific message string

USAGE_B02	"\
-bXX\t: Scan master boot record of specified physical drive (e.g. 80).\n"
$ PC-DOS specific message string

USAGE_L		"\
-l\t: Use this option to specify the name of a file containing a list\n\
\t  of files to check for viral signatures.\n"

USAGE_T		"\
-t\t: Use this option to specify the name of a single file to test for\n\
\t  viral signatures.\n"

USAGE_P		"\
-p\t: Build list of files that tested positive.  The default is\n\
\t  POSITIVE.VIR.\n"
$ PC-DOS specific message string

USAGE_PAIX	"\
-p\t: Build a list of files that tested positive.  The default is\n\
\t  \"positive.vir\".\n"
$ Do not remove or reposition the characters \" or try to 
$ translate "positive.vir"

USAGE_Q		"\
-q[uiet]\t: Reduce the number of messages displayed.\n"
$ You may chose to remove the characters "[uiet]" since the word "quiet"
$ may not be germane to your language

USAGE_V		"\
-v[erbose]\t: Maximize messages. Display a list of files and boot sectors as\n\
\t  they are scanned. Also forces hexadecimal display of any virus\n\
\t  signatures found. Please use this option to help diagnose the\n\
\t  problem if a scan terminates early due an to error.\n"
$ PC-DOS specific message string

USAGE_VAIX	"\
-v[erbose]\t: Maximize messages. Display a list of files as they\n\
\t  are scanned. Also forces hexadecimal display of any virus\n\
\t  signatures found. Please use this option to help diagnose the\n\
\t  problem if a scan terminates early due to error.\n"
$ You may chose to remove the characters "[erbose]" since "verbose" may not
$ be germane to your language.

USAGE_S		"\
-s\t: Use this option to specify the name of a (non-default) signature\n\
\t  file. VIRSIG.LST and ADDENDA.LST (if present) are the defaults.\n\
\t  If -s is specified, VIRSIG.LST and ADDENDA.LST are not\n\
\t  loaded (unless specified).\n"
$ PC-DOS specific message string

USAGE_SAIX	"\
-s\t: Use this option to specify the name of a non-default signature file\n\
\t  \"/etc/security/scan/virsig.lst\" and \"/etc/security/scan/addenda.lst\"\n\
\t  (if present) are the default signature files.  If -s is specified,\n\
\t  virsig.lst and addenda.lst are not loaded (unless specified).\n"
$ Do not try to translate the words "/etc/security/scan/virsig.lst" or 
$ "/etc/security/scan/addenda.lst".  Do not try to translate 
$ "addenda.lst" or "virsig.lst".  Do not remove the characters \"

USAGE_A		"\
-a\t: Scan all files on the indicated drives. (Same as '-w*.*')\n"
$ PC-DOS specific message string

USAGE_AAIX	"-a\t: Scan all files on the indicated filesystems.\n"
$ AIX specific message string

USAGE_M		"\
-m\t: Maybe detect mutants. Tries to detect small variations on the\n\
\t  viruses specified in the signature files.\n"

USAGE_W		"\
-w\t: Wildcard file specification. Applies to all indicated drives.\n\
\t  The default is '-w*.exe,*.com,*.ov?,*.ini,*.sys,*.bin'\n"
$ PC-DOS specific message string

USAGE_WAIX	"\
-w\t: Wildcard file specification.\n"
$ AIX specific message string

USAGE_G		"\
-g[uru]\t: If the file is an EXE file (determined by the presence of an EXE\n\
\t  header at the front of the file) then any signatures that are\n\
\t  found are reported, even if they are for viruses specified as\n\
\t  infecting COM files only. (By default, COM virus signatures in\n\
\t  EXE files are not reported.) Also, if an offset is specified for\n\
\t  the signature, report any signatures that are found, even if the\n\
\t  real offset of the signature doesn't match the offset specified\n\
\t  in the signature file.\n"
$ PC-DOS specific message string

USAGE_GMEM	"\
\t  Also, when scanning memory, report any signatures that are found,\n\
\t  even if the signature isn't marked with the 'scan memory'\n\
\t  keyphrase in the signature file.\n"
$ PC-DOS specific message string

USAGE_VV	"\
-vv\t: Very verbose. Like -v, except that a hex dump of boot sectors is\n\
\t  also displayed.\n"
$ PC-DOS specific message string

USAGE_R		"\
-rdrive:\t: Removable media. If this switch is specified for a drive, you\n\
\t  will be prompted to insert a diskette in the drive before the scan.\n\
\t  This allows you to scan multiple diskettes. For example, type:\n\
\t  virscan -ra: a:\n\
\t  to scan multiple diskettes in the A: drive.\n"
$ PC-DOS specific message string

USAGE_NP	"\
-np\t: No progress indicator. Do not display object names as they are\n\
\t  scanned.\n"
$ PC-DOS specific message string

USAGE_NST	"\
-nst\t: No self test. Do not perform self-test of VIRSCAN.EXE,\n\
\t  VIRSIG.LST, or VIRSCAN.MSG.\n"
$ PC-DOS specific message string

USAGE_NLA	"\
-nla\t: Do not display the banner containing the copyright notice,\n\
\t  or issue the associated prompt.\n"
$ PC-DOS specific message string

USAGE_NHMS	"\
-nhms\t: No high memory scan. Disable scanning for memory resident viruses\n\
\t  above absolute address A0000.\n"
$ PC-DOS specific message string

USAGE_NMS	"\
-nms\t: No memory scan. Completely disable scanning for memory\n\
\t  resident viruses.\n"
$ PC-DOS specific message string

USAGE_MEM	"\
-mem\t: Scan system memory. By default, virscan will scan memory\n\
\t  before it scans anything else. The -mem switch tells virscan\n\
\t  to scan memory even if it scans nothing else. Switches -nhms, -g\n\
\t  and -m all modify the behavior of the memory scan. By\n\
\t  default, virscan will only scan memory for certain viruses; if\n\
\t  the -g switch is used, when virscan scans memory for viruses,\n\
\t  it will scan for all of the viruses that it knows about.\n"
$ PC-DOS specific message string

USAGE_QQ	"\
-qq\t: Completely quiet operation. No messages at all will be\n\
\t  displayed, unless a fatal error occurs. The error level\n\
\t  will be set as documented, and can be used to determine\n\
\t  whether or not any virus signatures were found.\n"

USAGE_C		"\
-c[ont]\t: Continue scan if there is an error opening a file for scanning.\n"
$ PC-DOS specific message string

USAGE_Z		"\
-z\t: When the scan finishes, if any virus signatures were\n\
\t  found, wait for the user to press a key and beep once\n\
\t  per second.\n"

USAGE_E		"\
-e\t: Do not scan boot sectors unless explicitly specified\n\
\t  with the -b option. This is useful if there is a problem\n\
\t  scanning the boot sector of a drive (for instance,\n\
\t  a network drive). If this option is used, VIRSCAN will not\n\
\t  find boot sector viruses unless the -b option is used.\n"
$ PC-DOS specific message string

USAGE_NMBRS	"\
-nmbrs\t: Do not scan the master boot record of the first hard drive\n\
\t  unless explicitly specified with the -b option. This is\n\
\t  useful if there is a problem scanning the master boot record\n\
\t  for some reason, but other boot sectors can be scanned.\n"
$ PC-DOS specific message string

USAGE_CAD	"\
-cad\t: Continue if access denied. If this option is used,\n\
\t  VIRSCAN will continue the scan even if it is unable\n\
\t  to scan a particular subdirectory tree because access\n\
\t  was denied.\n"
$ PC-DOS specific message string

USAGE_NMUT	"\
-nmut\t: Disable the default mutant detection. Useful if there\n\
\t  are false virus warnings.\n"
 

PVERSION_BANNER "Virus Scanning Program Version 2.00.00\n"

PSUSAGE_01	"\
(c) Copyright International Business Machines Corporation 1989,1990,1991\n\
    Licensed Material - Program Property of IBM, All Rights Reserved.\n\n\
The Virus Scanning Program scans DOS and OS/2 files, and the boot\n\
sector of disks, for the signatures contained in the file\n\
VIRSIG.LST. These are signatures of known PC-DOS computer viruses.\n\
The Virus Scanning Program does *not* detect all possible viruses.\n"
$ PC-DOS specific message string

PSUSAGE_02	"\
(c) Copyright International Business Machines Corporation 1989,1990,1991\n\
    Licensed Material - Program Property of IBM, All Rights Reserved.\n\n\
The Virus Scanning Program scans DOS and OS/2 files, and the boot\n\
sectors of disks, for the signatures contained in the file\n\
VIRSIG.LST. These are signatures of known PC-DOS computer viruses.\n\
The Virus Scanning Program does *not* detect all possible viruses.\n"
$ PC-DOS specific message string

PSUSAGE_AIX	"\
(c) Copyright International Business Machines Corporation 1989,1990,1991\n\
    Licensed Material - Program Property of IBM, All Rights Reserved.\n\n\
The Virus Scanning Program scans system files, for the signatures \n\
contained in the file \"/etc/security/scan/virsig.lst\".  These are \n\
signatures of known computer viruses.  The Virus Scanning Program\n\
does *not* detect all possible viruses.\n"
$ Do not try to translate "/etc/security/scan/virsig.lst"

ISUSAGE_NB	"\
VIRSCAN scans DOS and OS/2 files for the signatures contained in\n\
the file VIRSIG.LST. These are signatures of known PC-DOS computer \n\
viruses. VIRSCAN does *not* detect all possible viruses.\n"
$ PC-DOS specific message string

ISUSAGE_A	"\
VIRSCAN scans DOS and OS/2 files, and the boot sector of disks, for\n\
the signatures contained in the file VIRSIG.LST. These are signatures\n\
of known PC-DOS computer viruses. VIRSCAN does *not* detect all\n\
possible viruses.\n"
$ PC-DOS specific message string

ISUSAGE_B	"\
VIRSCAN scans DOS and OS/2 files, and the boot sectors of disks, for\n\
the signatures contained in the file VIRSIG.LST. These are signatures\n\
of known PC-DOS computer viruses. VIRSCAN does *not* detect all\n\
possible viruses.\n"
$ PC-DOS specific message string

ISUSAGE_AIX	"\
Virscan scans system files, for the signatures constained in the file\n\
\"/etc/security/scan/virsig.lst\".  These are signatures of known\n\
computer viruses.  Virscan does *not* detect all possible viruses.\n"
$ Do not try to translate the words "/etc/security/scan/virsig.lst"

SUSAGE_A	"\
To scan all .EXE, .COM, .SYS, .OV?, .BIN, and .INI\nfiles (and the boot\n\
sector) on the C: drive, for instance, type\n\
virscan c:\n"
$ PC-DOS specific message string

SUSAGE_B	"\
To scan all .EXE, .COM, .SYS, .OV?, .BIN, and .INI\nfiles (and the boot\n\
sectors) on the C: drive, for instance, type\n\
virscan c:\n"
$ PC-DOS specific message string

SUSAGE_NB	"\
To scan all .EXE, .COM, .SYS, .OV?, .BIN, and .INI\nfiles on the C: drive,\n\
for instance, type\n\
virscan c:\n"
$ PC-DOS specific message string

SUSAGE_AIX	"\
To scan all files in the /usr filesystem, for instance, type\n\
virscan /usr\n"
$ Do not try to translate "/usr" or the string "virscan /usr"

SUSAGE_NDS	"\
To scan the single file 'possible.vir', for instance, type\n\
virscan -tpossible.vir\n"
$ Do not try to translate "possible.vir" or the string "virscan -tpossible.vir"

SUSAGE_01	"\
For help with command line options, type\n\
virscan ?\n"

SUSAGE_02       "\
For some usage examples, type\n\
virscan ??\n"

EUSAGE		"\
To scan the single file kumquat.exe, type\n\
virscan -tkumquat.exe\n"
$ PC-DOS specific message string

EUSAGE_AIX	"\
To scan the single file /usr/bin/vi, type\n\
virscan -t/usr/bin/vi\n\n"
$ Do not try to translate "/usr/bin/vi" or the string "virscan -t/usr/bin/vi"

EUSAGE_DSAIX	"\
To scan the /usr and /u filesystems, type\n\
virscan /usr /u\n\n"
$ Do not try to translate "/usr", "/u", or the string "virscan /usr /u"

EUSAGE_DSB01	"\
To scan *all* files the C: drive and put a list of any infected files or\n\
boot sector in the file 'infected.dat' (in the current directory), type\n\
virscan c: -a -pinfected.dat\n"
$ PC-DOS specific message string

EUSAGE_DSB02	"\
To scan *all* files the C: drive and put a list of any infected files or\n\
boot sectors in the file 'infected.dat' (in the current directory), type\n\
virscan c: -a -pinfected.dat\n"
$ PC-DOS specific message string

EUSAGE_DSNB01	"\
To scan *all* files on the C: drive and put a list of any infected\n\
files in the file 'infected.dat' (in the current directory), type\n\
virscan c: -a -pinfected.dat\n"
$ PC-DOS specific message string

EUSAGE_DS01AIX	"\
To scan *all* files /usr filesystem and put a list of any infected\n\
files in the file 'infected.dat' (in the current directory), type\n\
virscan /usr -a -pinfected.dat\n\n"
$ Do not try to translate "/usr", "infected.dat", or the string 
$ "virscan /usr -a -pinfected.dat"

EUSAGE_DS03     "\
To scan the single subdirectory C:\\UTIL, type\n\
virscan c:\\util\n"
$ PC-DOS specific message string

EUSAGE_DS03AIX	"\
To scan the single subdirectory /usr/bin, type\n\
virscan /usr/bin\n\n"
$ Do not try to translate "/usr/bin" or the string "virscan /usr/bin"

EUSAGE_B01	"\
To scan the system boot sector for C:, type\n\
virscan -bc:\n"
$ PC-DOS specific message string

EUSAGE_B02	"\
To scan the master boot record for the first hard drive, type\n\
virscan -b80\n"
$ PC-DOS specific message string

EUSAGE_03	"\
To scan each file in the list of files 'filelist.lst', type\n\
virscan -lfilelist.lst\n"
$ Do not try to translate "filelist.lst" or the string "virscan -lfilelist.lst"

INVALID_RDL     "\
(Invalid removable drive letter was specified.)\n"
$ PC-DOS specific message string

INVALID_SWITCH  "\
Invalid switch %s; type VIRSCAN for help.\n"
$ PC-DOS specific message string

INVALID_SW_AIX  "\
virscan: 0710-001 Invalid option %s; type \"virscan\" for help.\n"
$ Do not try to translate "virscan"

NO_SUCH_DRIVE   "Drive %c: does not exist\n"
$ PC-DOS specific message string

NO_FN_FILELIST  "\
virscan: 0710-002 Specify a file name containing a list of files to scan.\n"

NO_FN_FILE      "\
virscan: 0710-003 Specify a file name to test for viruses.\n"

PLA_02		"\
(c) Copyright International Business Machines Corporation 1989,1990,1991\n\
    Licensed Material - Program Property of IBM, All Rights Reserved.\n\
\n\
\t\tNOTICE TO USERS\n\
\n\
\tFor users in the United States and Puerto Rico:\n\
\tTHE VIRUS SCANNING PROGRAM IS LICENSED \"AS IS.\"  Your use of this\n\
\tprogram is subject to the IBM Program License Agreement for the Virus\n\
\tScanning Program, which is set out in the \"READ.ME\" file distributed\n\
\twith this program.\n\
\n\
\tFor users outside the United States and Puerto Rico:\n\
\tSee your representative or authorized supplier for contract terms.\n\
\n\
\tPress Y to accept the license agreement, or press any other key to exit.\n"
$ PC-DOS specific message string

NO_FN_SIGFILE   "\
virscan: 0710-004 Specify a virus signature file name.\n"

TESTING_FILE    "\
Testing (%s) for modifications."

FILE_MOD_MSG	"\
The program (%s) has been modified.\n\
This may be due to a virus, or to other causes.\n"

SIX_MONTHS	"\
According to the system clock, this version of VIRSCAN,\n\
which was released on %s, is at least 6 months old.\n\
Please obtain the latest version to detect viruses\n\
discovered since then.\n"
$ PC-DOS specific message string

LOADED_N_SIGS   "Loaded %d signatures\n"
LOADED_N_SIGFS  "Loaded %d signature fragments\n"

SSM_NOT_SUPPORT "\
Scan of system memory not supported in OS/2 protect mode.\n"
$ PC-DOS specific message string

MBRS_NOT_SUPP   "\
Scan of master boot record not supported in OS/2 protect mode\n"
$ PC-DOS specific message string

SCAN_COMPLETE01 "Scan completed."
SCAN_COMPLETE02 "Scan completed.\n"
NOTHING_SCANNED "Nothing was scanned.\n"
FILE_SCANNED    "%u file was scanned.\n"
FILES_SCANNED   "%u files were scanned.\n"

SBS_SCANNED     "%u system boot sector was scanned.\n"
$ PC-DOS specific message string

SBSS_SCANNED    "%u system boot sectors were scanned.\n"
$ PC-DOS specific message string

MBR_SCANNED     "%u master boot record was scanned.\n"
$ PC-DOS specific message string

MBRS_SCANNED    "%u master boot records were scanned.\n"
$ PC-DOS specific message string

SM_SCANNED      "System memory was scanned for viruses.\n"
$ PC-DOS specific message string


SM_PART_SCANNED "\
System memory was scanned for dangerous and/or well hidden resident viruses.\n"
$ PC-DOS specific message string


TOTAL_B_SCAN01  "Total bytes scanned = %lu, in %ld second.\n"
TOTAL_B_SCAN02  "Total bytes scanned = %lu, in %ld seconds.\n"
VSF_COUNT_11    "%d Viral signature found in %d object.\n"
VSF_COUNT_21    "%d Viral signatures found in %d object.\n"
VSF_COUNT_12    "%d Viral signature found in %d objects.\n"
VSF_COUNT_22    "%d Viral signatures found in %d objects.\n"

NO_VIRUS_FOUND  "\
No viruses listed in the signature files were found.\n"

PLEASE_REPORT01 "\n\
Please send a note to VIRUS at RHQVM03 immediately, reporting that you found\n\
this virus.  This will help us get the correct cleanup information to you,\n\
and will help us track the infection.  Also, please follow whatever local\n\
virus reporting procedures your site may have, which may include notifying\n\
your local Help Desk or Area Information Center. Make sure they have obtained\n\
a sample of the virus for analysis before removing the infection.\n"
$ Do not translate

RDL_NOT_SPEC    "\
(Removable drive letter was not specified.)\n"
$ PC-DOS specific message string

NOTHING_TO_SCAN "\
virscan: 0710-005 Specify a file, filelist, or directory to scan.\n"

STARTING_VSCAN  "Starting virus scan on %s"

INS_DISK_MSG    "\
Insert diskette to be scanned in drive %s and press any key to continue "
$ PC-DOS specific message string

CONT_SCAN_MSG   "(Continuing Scan)\n"

ERR_PARSE_LINE  "\
virscan: 0710-006 An error occurred parsing the following line:\n"

ODD_LEN_SIG     "\
virscan: 0710-007 An odd length signature was found.\n\
The last hexadecimal digit will be ignored.\n"

ERR_OPENING_W   "\
virscan: 0710-008 An error occurred opening (%s) for writing.\n"

REASON_FOR_ERR  "\
virscan: 0710-009 The reason for the error : %s\n"

ERR_OPENING_A   "\
virscan: 0710-010 An error occurred opening (%s) for append\n"

NO_SIMPLE_SIG   "\
(This virus does not have a simple signature.)"

THIS_FILE_MBIV  "\
This file may be infected with a variant of"

THIS_MBR_MBIV   "\
This master boot record may be infected with a variant of"
$ PC-DOS specific message string

THIS_SBS_MBIV   "\
This system boot sector may be infected with a variant of"
$ PC-DOS specific message string

SM_MCV          "System memory may contain a variant of"
$ PC-DOS specific message string

MBIV            "This may be infected with a variant of"
THIS_FILE_MBI   "This file may be infected with"

THIS_MBR_MBI    "\
This master boot record may be infected with"
$ PC-DOS specific message string

THIS_SBS_MBI    "\
This system boot sector may be infected with"
$ PC-DOS specific message string

SM_MC           "System memory may contain"
$ PC-DOS specific message string

MBI             "This may be infected with"

FOUND_IN_MEM	"\
This signature was found in system memory. The following message line is\n\
normally displayed when this virus is found in a file or boot sector:\n"
$ PC-DOS specific message string

MAY_IND_VARIANT "\
This may indicate the presence of a variant of the virus.\n"

FOUND_IN_MEM_03 "\n\
However, the signature was found in system memory, not in a file or\n\
in a boot sector.\n"
$ PC-DOS specific message string

UNABLE_TO_OPEN  "\
virscan: 0710-011 Unable to open (%s) for scanning.\n"

SCANNING_MEM    "Scanning system memory for%sviruses"
$ PC-DOS specific message string

SCANNING_ADDR_R "Scanning address range %05lx-%05lx\n"
$ PC-DOS specific message string

MALLOC_ERROR    "\
virscan: 0710-012 Out of memory: malloc() (%d) in get_signatures()\n"
$ Do not try to translate "malloc()" or "get_signatures()"

OUT_OF_MEMORY   "\
virscan: 0710-013 Out of memory for signatures.\n"

TRY_DECREASING  "Try decreasing the number of signatures\n"
OOM_MSG2        "Try running without -m option.\n"

SCANNING_SBS    "\
Scanning system boot sector of %c: for boot sector viruses"
$ PC-DOS specific message string

BS_NAME         "system boot sector of drive %s"
$ PC-DOS specific message string

SCANNING_MBR    "\
Scanning master boot record of physical drive %02X for boot sector viruses"
$ PC-DOS specific message string

MBR_NAME        "master boot record of drive %02X"
$ PC-DOS specific message string

OOM_STRDUP      "\
virscan: 0710-013 Out of memory: strdup() in output_positive_filename()\n"
$ Do not try to translate "strdup()" or "output_positive_filename()"

MMATCHED_BYTES  " (%d mismatched bytes)"
FRAGMENT        " (fragment)"
OFFSET          " (offset %ld (%lXH))"

BAD_VIRUS	"\
Warning, this virus may interact badly with virscan if the virus\n\
is resident in system memory. Before continuing, please verify\n\
that you have\n\
1) cold-booted from a write protected floppy diskette,\n\
2) are running the copy of virscan.exe on that diskette,\n\
3) have used virscan.exe to scan the diskette and have\n\
   verified that the diskette is free of any viruses,\n\
4) and did not run any programs from any other disk between\n\
   booting from that diskette and starting virscan.\n\n"
$ PC-DOS specific message string

YOU_HAVE_BEEN	"You have been warned.\n"
$ PC-DOS specific message string

PRESS_C_TO_CONT	"\
Press C to continue scan, or any other key to quit.\n"
$ Do not change "C"

ABORTING_SCAN   "Quiting scan.\n"

RESTART_SCAN    "\n\
Restart the scan with the -c option if you wish to ignore this error.\n"

CONTINUING_SCAN "\
virscan: 0710-014 This file cannot normally be opened. The scan will continue\n"

ERROR_READING_F "\
virscan: 0710-015 An error occurred reading (%s)\n"

ERROR_OPEN_SF   "\
virscan: 0710-016 An error occurred opening signature file (%s)\n"

ERROR_O_SF	"\
The signature file could not be read.  Make sure that it was \n\
properly specified and that it exists.\n\n"

TESTING_SIGFILE "\
Testing signature file (%s) for modifications."
$ PC-DOS specific message string

FILE_HAS_BEEN_M "The file (%s) has been modified.\n"
READING_SIGFILE "Reading signature file (%s)"
NO_SIGNATURES   "No signatures to test for.\n"
FOUND_SIG       "Found signature in (%s)"
FOUND_SIG_P     "Found partial signature in (%s)"
FOUND_SIG_F     "Found signature fragment in (%s)"
FOUND_SIG_PF    "Found partial signature fragment in (%s)"

FOUND_SIG_AO    "\
Found signature in (%s) at offset %ld (%lXH)\n"

FOUND_SIG_P_AO  "\
Found partial signature in (%s) at offset %ld (%lXH)\n"

FOUND_SIG_F_AO  "\
Found signature fragment in (%s) at offset %ld (%lXH)\n"

FOUND_SIG_PF_AO "\
Found partial signature fragment in (%s) at offset %ld (%lXH)\n"

BYTE_DIFFERENT  "\
(%d byte was different than in the reference signature.)\n"

BYTES_DIFFERENT "\
(%d bytes were different than in the reference signature.)\n"

HEX_DUMP_BS     "\n\
(Hex dump of boot sector to be scanned)\n"
$ PC-DOS specific message string

ONE_OR_MORE     "\n\
One or more virus signatures were found.\n"

ERR_READING_SBS "\
There was an error reading system boot sector for drive %s, INT 25 rc=%04X\n"
$ PC-DOS specific message string

ATYPICAL_BPS    "\
Drive %s has %d bytes per sector, which is atypical.\n"
$ PC-DOS specific message string

NEVER_FORMATTED "\
The drive may never have been DOS formatted.\n"
$ PC-DOS specific message string

SBS_NOT_SCANNED "\
The boot sector may not have been correctly scanned.\n"
$ PC-DOS specific message string

SIG_OVERFLOW_01 "\
virscan: 0710-016 The signature number %d in file (%s) is too long.\n"

SIG_OVERFLOW_02 "\
Maximum length for a signature is %d bytes.\n"

SIG_OVERFLOW_03 "\
virscan: 0710-017 The signature line can be %d bytes long\n"

NO_WILDCARDS_01 "\
virscan: 0710-018 Wildcard characters are not allowed in the first\n\
two signature bytes.\n"

NO_WILDCARDS_02 "\
virscan: 0710-018 The first four characters of signature strings must\n\
be hexadecimal digits.\n"

SHORT_SIG_01    "\
virscan: 0710-019 A signature of three bytes or less was found.\n"

SHORT_SIG_02    "\
Signatures must be four bytes or more in length.\n"

ERR_READING_SML "\
virscan: 0710-019 An error occurred reading the message line for signature\n\
number %d in (%s).\n"

ERR_READING_SFT "\
virscan: 0710-020 An error occurred reading the file types line for signature\n\
number %d in (%s).\n"

IUO_BANNER      "Internal Use Only\n"
$ Do not translate

VERSION_BANNER  "Virus scanner      Version 2.00.00\n"

FILE_NOT_FOUND  "virscan: 0710-021 Specify a valid file name.\n"

INV_PATH_SPEC   "virscan: 0710-022 Specify a valid path specification.\n"

TOO_MANY_OPEN_F "Too many open files\n"
$ PC-DOS specific message string

ACCESS_DENIED   "Access denied\n"
$ PC-DOS specific message string

INVALID_ACCESS  "Invalid access\n"
$ PC-DOS specific message string

DRIVE_NOT_READY "Drive not ready\n"
$ PC-DOS specific message string

NOT_DOS_DISK    "Not DOS Disk\n"
$ PC-DOS specific message string

SHARING_VIOLA   "Sharing violation\n"
$ PC-DOS specific message string

SHARING_BUF_EXC "Sharing buffer exceeded\n"
$ PC-DOS specific message string

NET_ACCESS_DENI "Network access denied\n"
$ PC-DOS specific message string

CANNOT_MAKE     "Cannot make\n"
$ PC-DOS specific message string

INVALID_PARM    "Invalid Parameter\n"
$ PC-DOS specific message string

DRIVE_LOCKED    "Drive locked\n"
$ PC-DOS specific message string

OPEN_FAILURE    "Open failed\n"
$ PC-DOS specific message string

DISK_FULL       "Disk full\n"
$ PC-DOS specific message string

FILENAME_EXC_RA "Filename exceeds range\n"
$ PC-DOS specific message string

BAD_NETPATH     "Bad netpath\n"
$ PC-DOS specific message string

UNIDENTIFIED_RC "unidentified rc=%d\n"
$ PC-DOS specific message string

PATH_CAUSING_ER "\
The pathname that caused the error was (%s)\n"
$ PC-DOS specific message string

INVALID_HANDLE  "Invalid handle\n"
$ PC-DOS specific message string

LOCK_VIOLATION  "Lock violation\n"
$ PC-DOS specific message string

BROKEN_PIPE     "Broken pipe\n"
$ PC-DOS specific message string

LOGFILE_VMSG    "Virscan version: %s"
LOGFILE_TDMSG   "Time/Date of start of scan: %s"

SCAN_TERMINATE	"\
virscan: 0710-023 Scan terminated early due to an error.\n\
Please use the \"-v\" option to help clarify why the error occurred.\n"

CANT_HAPPEN	"Can't happen here...\n"
$ Do not translate

SCAN_VIA_LIST	"\
virscan: 0710-024 An error occurred opening the list of files (%s)\n"

FILELIST_ERR	"\
The file list could not be read.  Make sure that it was\n\
properly specified and that it exists.\n\n"

BOOT_SCAN_ERR           "\
Error scanning system boot sector of drive %s\n"
$ PC-DOS specific message string

NETWORK_DRIVE		"\
"(This is normal if drive %s is a network drive.)\n"
$ PC-DOS specific message string

SCAN_ANOTHER            "\
Scan another diskette (Y/N)? "
$ PC-DOS specific message string

MASTER_ERR		"\
Error reading master boot record of physical drive %02X, INT 13 rc=%04X\n"
$ PC-DOS specific message string

MASTER_ERR_HARD		"\
Error reading master boot record of physical hard drive %02X, INT 13 rc=%04X\n"
$ PC-DOS specific message string

MASTER_NOT_SCANNED	"\
This master boot record was not scanned.\n"
$ PC-DOS specific message string

SCANNING		"Scanning %s\n"

INVALID_DIR 		"\
virscan: 0710-026 %s is an invalid directory.\n"

CANT_OPENDIR		"\
virscan: 0710-027 Error opening directory (%s)\n"

CANT_STAT		"\
virscan: 0710-028 A status error occurred on file (%s)\n"
