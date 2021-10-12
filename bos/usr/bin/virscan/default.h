/* @(#)57	1.4  src/bos/usr/bin/virscan/default.h, cmdsvir, bos411, 9428A410j 9/8/93 12:40:21 */
/*
 * COMPONENT_NAME: CMDSVIR
 *
 * FUNCTIONS: NONE
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#define MSG_VERSION_FLAG	"VERSION=IUO"

#define MSG_COPYRIGHT		"\
(c) Copyright IBM Corporation 1989, 1990, 1991\n"

#define MSG_PRESS_ANY_KEY	"--- Press any key to continue ---"
#define MSG_PRESS_ENTER		"--- Press <enter> to continue ---"
#define MSG_USAGE_01		"Usage: virscan options\n"
#define MSG_USAGE_02		"options: "
#define MSG_USAGE_03		"Explanation of switches:\n"

#define MSG_USAGE_DOPTION 	"[-d]drive: || "

#define MSG_USAGE_LOPTION	"-llistfilename"
#define MSG_USAGE_TOPTION 	" | -tsinglefilename | directory"

#define MSG_USAGE_BOPTION	" || -bdrive:"

#define MSG_USAGE_SPQOPTION 	"\
\t[-ssigfilename] [-pposfilename] [-quiet] [-verbose]"

#define MSG_USAGE_WAROPTION	"\
[-wwildcardfilespec] [-a] [-rdrive:] "

#define MSG_USAGE_WAOPTION	"[-w\"wildcardfilespec\"] [-a] "

#define MSG_USAGE_DRIVE01	"\
[-d]drive:\t: Scan a PC-DOS or OS/2 logical drive.\n"

#define MSG_USAGE_B01		"\
-bdrive:\t: Scan system boot sector of specified logical drive.\n"

#define MSG_USAGE_B02		"\
-bXX\t: Scan master boot record of specified physical drive (e.g. 80).\n"

#define MSG_USAGE_L		"\
-l\t: Use this option to specify the name of a file containing a list\n\
\t  of files to check for viral signatures.\n"

#define MSG_USAGE_T		"\
-t\t: Use this option to specify the name of a single file to test for\n\
\t  viral signatures.\n"

#define MSG_USAGE_P		"\
-p\t: Build list of files that tested positive.  The default is\n\
\t  POSITIVE.VIR.\n"

#define MSG_USAGE_PAIX		"\
-p\t: Build a list of files that tested positive.  The default is\n\
\t  \"positive.vir\".\n"

#define MSG_USAGE_Q		"\
-q[uiet]\t: Reduce the number of messages displayed.\n"

#define MSG_USAGE_V		"\
-v[erbose]\t: Maximize messages. Display a list of files and boot sectors as\n\
\t  they are scanned. Also forces hexadecimal display of any virus\n\
\t  signatures found. Please use this option to help diagnose the\n\
\t  problem if a scan terminates early due to an error.\n"

#define MSG_USAGE_VAIX		"\
-v[erbose]\t: Maximize messages. Display a list of files as they\n\
\t  are scanned. Also forces hexadecimal display of any virus\n\
\t  signatures found. Please use this option to help diagnose the\n\
\t  problem if a scan terminates early due to an error.\n"

#define MSG_USAGE_S		"\
-s\t: Use this option to specify the name of a (non-default) signature\n\
\t  file. VIRSIG.LST and ADDENDA.LST (if present) are the defaults.\n\
\t  If -s is specified, VIRSIG.LST and ADDENDA.LST are not\n\
\t  loaded (unless specified).\n"

#define MSG_USAGE_SAIX		"\
-s\t: Use this option to specify the name of a non-default signature file.\n\
\t  \"/etc/security/scan/virsig.lst\" and \"/etc/security/scan/addenda.lst\"\n\
\t  (if present) are the default signature files.  If -s is specified,\n\
\t  virsig.lst and addenda.lst are not loaded (unless specified).\n"

#define MSG_USAGE_A		"\
-a\t: Scan all files on the indicated drives. (Same as '-w*.*')\n"

#define MSG_USAGE_AAIX		"\
-a\t: Scan all files on the indicated filesystems.\n"

#define MSG_USAGE_M		"\
-m\t: Maybe detect mutants. Tries to detect small variations on the\n\
\t  viruses specified in the signature files.\n"

#define MSG_USAGE_W		"\
-w\t: Wildcard file specification. Applies to all indicated drives.\n\
\t  The default is '-w*.exe,*.com,*.ov?,*.ini,*.sys,*.bin'\n"

#define MSG_USAGE_WAIX		"\
-w\t: Wildcard file specification.\n"

#define MSG_USAGE_G		"\
-g[uru]\t: If the file is an EXE file (determined by the presence of an EXE\n\
\t  header at the front of the file) then any signatures that are\n\
\t  found are reported, even if they are for viruses specified as\n\
\t  infecting COM files only. (By default, COM virus signatures in\n\
\t  EXE files are not reported.) Also, if an offset is specified for\n\
\t  the signature, report any signatures that are found, even if the\n\
\t  real offset of the signature doesn't match the offset specified\n\
\t  in the signature file.\n"

#define MSG_USAGE_GMEM		"\
\t  Also, when scanning memory, report any signatures that are found,\n\
\t  even if the signature isn't marked with the 'scan memory'\n\
\t  keyphrase in the signature file.\n"

#define MSG_USAGE_VV		"\
-vv\t: Very verbose. Like -v, except that a hex dump of boot sectors is\n\
\t  also displayed.\n"

#define MSG_USAGE_R		"\
-rdrive:\t: Removable media. If this switch is specified for a drive, you\n\
\t  will be prompted to insert a diskette in the drive before the scan.\n\
\t  This allows you to scan multiple diskettes. For example, type:\n\
\t  virscan -ra: a:\n\
\t  to scan multiple diskettes in the A: drive.\n"

#define MSG_USAGE_NP		"\
-np\t: No progress indicator. Do not display object names as they are\n\
\t  scanned.\n"

#define MSG_USAGE_NST		"\
-nst\t: No self test. Do not perform self-test of VIRSCAN.EXE,\n\
\t  VIRSIG.LST, or VIRSCAN.MSG.\n"

#define MSG_USAGE_NLA		"\
-nla\t: Do not display the banner containing the copyright notice,\n\
\t  or issue the associated prompt.\n"

#define MSG_USAGE_NHMS		"\
-nhms\t: No high memory scan. Disable scanning for memory resident viruses\n\
\t  above absolute address A0000.\n"

#define MSG_USAGE_NMS		"\
-nms\t: No memory scan. Completely disable scanning for memory\n\
\t  resident viruses.\n"

#define MSG_USAGE_MEM		"\
-mem\t: Scan system memory. By default, virscan will scan memory\n\
\t  before it scans anything else. The -mem switch tells virscan\n\
\t  to scan memory even if it scans nothing else. Switches -nhms, -g\n\
\t  and -m all modify the behavior of the memory scan. By\n\
\t  default, virscan will only scan memory for certain viruses; if\n\
\t  the -g switch is used, when virscan scans memory for viruses,\n\
\t  it will scan for all of the viruses that it knows about.\n"

#define MSG_USAGE_QQ		"\
-qq\t: Completely quiet operation. No messages at all will be\n\
\t  displayed, unless a fatal error occurs. The error level\n\
\t  will be set as documented, and can be used to determine\n\
\t  whether or not any virus signatures were found.\n"

#define MSG_USAGE_C		"\
-c[ont]\t: Continue scan if there is an error opening a file for scanning.\n"

#define MSG_USAGE_Z		"\
-z\t: When the scan finishes, if any virus signatures were\n\
\t  found, wait for the user to press a key and beep once\n\
\t  per second.\n"

#define MSG_USAGE_E		"\
-e\t: Do not scan boot sectors unless explicitly specified\n\
\t  with the -b option. This is useful if there is a problem\n\
\t  scanning the boot sector of a drive (for instance,\n\
\t  a network drive). If this option is used, VIRSCAN will not\n\
\t  find boot sector viruses unless the -b option is used.\n"

#define MSG_USAGE_NMBRS		"\
-nmbrs\t: Do not scan the master boot record of the first hard drive\n\
\t  unless explicitly specified with the -b option. This is\n\
\t  useful if there is a problem scanning the master boot record\n\
\t  for some reason, but other boot sectors can be scanned.\n"

#define MSG_USAGE_CAD		"\
-cad\t: Continue if access denied. If this option is used,\n\
\t  VIRSCAN will continue the scan even if it is unable\n\
\t  to scan a particular subdirectory tree because access\n\
\t  was denied.\n"

#define MSG_USAGE_NMUT		"\
-nmut\t: Disable the default mutant detection. Useful if there\n\
\t  are false virus warnings.\n"

#define MSG_PVERSION_BANNER "Virus Scanning Program Version 2.00.00\n"

#define MSG_PSUSAGE_01		"\
(c) Copyright International Business Machines Corporation 1989,1990,1991\n\
    Licensed Material - Program Property of IBM, All Rights Reserved.\n\n\
The IBM Virus Scanning Program scans DOS and OS/2 files, and the boot\n\
sector of disks, for the signatures contained in the file\n\
VIRSIG.LST. These are signatures of known PC-DOS computer viruses.\n\
The IBM Virus Scanning Program does *not* detect all possible viruses.\n"

#define MSG_PSUSAGE_02		"\
(c) Copyright International Business Machines Corporation 1989,1990,1991\n\
    Licensed Material - Program Property of IBM, All Rights Reserved.\n\n\
The IBM Virus Scanning Program scans DOS and OS/2 files, and the boot\n\
sectors of disks, for the signatures contained in the file\n\
VIRSIG.LST. These are signatures of known PC-DOS computer viruses.\n\
The IBM Virus Scanning Program does *not* detect all possible viruses.\n"

#define MSG_PSUSAGE_AIX		"\
(c) Copyright International Business Machines Corporation 1989,1990,1991\n\
    Licensed Material - Program Property of IBM, All Rights Reserved.\n\n\
The IBM Virus Scanning Program scans AIX files, for the signatures \n\
contained in the file \"/etc/security/scan/virsig.lst\".  These are\n\
signatures of known computer viruses.  The IBM Virus Scanning Program\n\
does *not* detect all possible viruses.\n"

#define MSG_ISUSAGE_NB		"\
VIRSCAN scans DOS and OS/2 files for the signatures contained in\n\
the file VIRSIG.LST. These are signatures of known PC-DOS computer \n\
viruses. VIRSCAN does *not* detect all possible viruses.\n"

#define MSG_ISUSAGE_A		"\
VIRSCAN scans DOS and OS/2 files, and the boot sector of disks, for\n\
the signatures contained in the file VIRSIG.LST. These are signatures\n\
of known PC-DOS computer viruses. VIRSCAN does *not* detect all\n\
possible viruses.\n"

#define MSG_ISUSAGE_B		"\
VIRSCAN scans DOS and OS/2 files, and the boot sectors of disks, for\n\
the signatures contained in the file VIRSIG.LST. These are signatures\n\
of known PC-DOS computer viruses. VIRSCAN does *not* detect all\n\
possible viruses.\n"

#define MSG_ISUSAGE_AIX		"\
Virscan scans AIX files, for the signatures constained in the file\n\
\"/etc/security/scan/virsig.lst\".  These are signatures of known \n\
computer viruses.  Virscan does *not* detect all possible viruses.\n"

#define MSG_SUSAGE_A		"\
To scan all .EXE, .COM, .SYS, .OV?, .BIN, and .INI\nfiles (and the boot\n\
sector) on the C: drive, for instance, type\n\
virscan c:\n"

#define MSG_SUSAGE_B		"\
To scan all .EXE, .COM, .SYS, .OV?, .BIN, and .INI\nfiles (and the boot\n\
sectors) on the C: drive, for instance, type\n\
virscan c:\n"

#define MSG_SUSAGE_NB		"\
To scan all .EXE, .COM, .SYS, .OV?, .BIN, and .INI\nfiles on the C: drive,\n\
for instance, type\n\
virscan c:\n"

#define MSG_SUSAGE_AIX		"\
To scan all files in the /usr filesystem, for instance, type\n\
virscan /usr\n"

#define MSG_SUSAGE_NDS		"\
To scan the single file 'possible.vir', for instance, type\n\
virscan -tpossible.vir\n"

#define MSG_SUSAGE_01		"\
For help with command line options, type\n\
virscan ?\n"

#define MSG_SUSAGE_02       	"\
For some usage examples, type\n\
virscan ??\n"

#define MSG_EUSAGE		"\
To scan the single file kumquat.exe, type\n\
virscan -tkumquat.exe\n"

#define MSG_EUSAGE_AIX		"\
To scan the single file /usr/bin/vi, type\n\
virscan -t/usr/bin/vi\n\n"

#define MSG_EUSAGE_DSAIX	"\
To scan the /usr and /u filesystems, type\n\
virscan /usr /u\n\n"

#define MSG_EUSAGE_DSB01	"\
To scan *all* files the C: drive and put a list of any infected files or\n\
boot sector in the file 'infected.dat' (in the current directory), type\n\
virscan c: -a -pinfected.dat\n"

#define MSG_EUSAGE_DSB02	"\
To scan *all* files the C: drive and put a list of any infected files or\n\
boot sectors in the file 'infected.dat' (in the current directory), type\n\
virscan c: -a -pinfected.dat\n"

#define MSG_EUSAGE_DSNB01	"\
To scan *all* files on the C: drive and put a list of any infected\n\
files in the file 'infected.dat' (in the current directory), type\n\
virscan c: -a -pinfected.dat\n"

#define MSG_EUSAGE_DS01AIX	"\
To scan *all* files /usr filesystem and put a list of any infected\n\
files in the file 'infected.dat' (in the current directory), type\n\
virscan /usr -a -pinfected.dat\n\n"

#define MSG_EUSAGE_DS03     	"\
To scan the single subdirectory C:\\UTIL, type\n\
virscan c:\\util\n"

#define MSG_EUSAGE_DS03AIX	"\
To scan the single subdirectory /usr/bin, type\n\
virscan /usr/bin\n\n"

#define MSG_EUSAGE_B01		"\
To scan the system boot sector for C:, type\n\
virscan -bc:\n"

#define MSG_EUSAGE_B02		"\
To scan the master boot record for the first hard drive, type\n\
virscan -b80\n"

#define MSG_EUSAGE_03		"\
To scan each file in the list of files 'filelist.lst', type\n\
virscan -lfilelist.lst\n"

#define MSG_INVALID_RDL     	"\
(Invalid removable drive letter was specified.)\n"

#define MSG_INVALID_SWITCH  	"\
Invalid switch %s; type VIRSCAN for help.\n"

#define MSG_INVALID_SW_AIX  	"\
Invalid switch %s; type \"virscan\" for help.\n"

#define MSG_NO_SUCH_DRIVE   	"Drive %c: does not exist\n"

#define MSG_NO_FN_FILELIST  	"\
(File name of list of files was not specified.)\n"
#define MSG_NO_FN_FILE      	"\
(File name of file to test was not specified.)\n"

#define MSG_PLA_02		"\
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

#define MSG_NO_FN_SIGFILE  	"\
(Signature file name was not specified.)\n"

#define MSG_TESTING_FILE    	"\
Testing (%s) for modifications."

#define MSG_FILE_MOD_MSG	"\
The program (%s) has been modified.\n\
This may be due to a virus, or to other causes.\n"

#define MSG_SIX_MONTHS		"\
According to the system clock, this version of VIRSCAN,\n\
which was released on %s, is at least 6 months old.\n\
Please obtain the latest version to detect viruses\n\
discovered since then.\n"

#define MSG_LOADED_N_SIGS   	"Loaded %d signatures\n"
#define MSG_LOADED_N_SIGFS  	"Loaded %d signature fragments\n"

#define MSG_SSM_NOT_SUPPORT 	"\
Scan of system memory not supported in OS/2 protect mode.\n"

#define MSG_MBRS_NOT_SUPP   	"\
Scan of master boot record not supported in OS/2 protect mode\n"

#define MSG_SCAN_COMPLETE01 	"Scan completed."
#define MSG_SCAN_COMPLETE02 	"Scan completed.\n"
#define MSG_NOTHING_SCANNED 	"Nothing was scanned.\n"
#define MSG_FILE_SCANNED    	"%u file was scanned.\n"
#define MSG_FILES_SCANNED   	"%u files were scanned.\n"
#define MSG_SBS_SCANNED     	"%u system boot sector was scanned.\n"
#define MSG_SBSS_SCANNED    	"%u system boot sectors were scanned.\n"
#define MSG_MBR_SCANNED     	"%u master boot record was scanned.\n"
#define MSG_MBRS_SCANNED    	"%u master boot records were scanned.\n"
#define MSG_SM_SCANNED      	"System memory was scanned for viruses.\n"

#define MSG_SM_PART_SCANNED 	"\
System memory was scanned for dangerous and/or well hidden resident viruses.\n"

#define MSG_TOTAL_B_SCAN01  	"Total bytes scanned = %lu, in %ld second.\n"
#define MSG_TOTAL_B_SCAN02  	"Total bytes scanned = %lu, in %ld seconds.\n"
#define MSG_VSF_COUNT_11    	"%d Viral signature found in %d object.\n"
#define MSG_VSF_COUNT_21    	"%d Viral signatures found in %d object.\n"
#define MSG_VSF_COUNT_12    	"%d Viral signature found in %d objects.\n"
#define MSG_VSF_COUNT_22    	"%d Viral signatures found in %d objects.\n"

#define MSG_NO_VIRUS_FOUND  	"\
No viruses listed in the signature files were found.\n"

#define MSG_PLEASE_REPORT01 	"\n\
Please send a note to VIRUS at RHQVM03 immediately, reporting that you found\n\
this virus.  This will help us get the correct cleanup information to you,\n\
and will help us track the infection.  Also, please follow whatever local\n\
virus reporting procedures your site may have, which may include notifying\n\
your local Help Desk or Area Information Center. Make sure they have obtained\n\
a sample of the virus for analysis before removing the infection.\n"

#define MSG_RDL_NOT_SPEC    	"\
(Removable drive letter was not specified.)\n"

#define MSG_NOTHING_TO_SCAN 	"Nothing to scan. (option -h for help)\n"
#define MSG_STARTING_VSCAN  	"Starting virus scan on %s"

#define MSG_INS_DISK_MSG    	"\
Insert diskette to be scanned in drive %s and press any key to continue "

#define MSG_CONT_SCAN_MSG  	"(Continuing Scan)\n"
#define MSG_ERR_PARSE_LINE  	"\
Error parsing line. The line causing the problem follows:\n"

#define MSG_ODD_LEN_SIG     	"\
warning - odd length signature - last hexadecimal digit ignored\n"

#define MSG_ERR_OPENING_W   	"Error opening (%s) for write.\n"
#define MSG_REASON_FOR_ERR  	"Reason for error : %s\n"
#define MSG_ERR_OPENING_A   	"Error opening (%s) for append\n"

#define MSG_NO_SIMPLE_SIG   	"\
(This virus does not have a simple signature.)"

#define MSG_THIS_FILE_MBIV  	"\
This file may be infected with a variant of"

#define MSG_THIS_MBR_MBIV   	"\
This master boot record may be infected with a variant of"

#define MSG_THIS_SBS_MBIV   	"\
This system boot sector may be infected with a variant of"

#define MSG_SM_MCV          	"System memory may contain a variant of"
#define MSG_MBIV            	"This may be infected with a variant of"
#define MSG_THIS_FILE_MBI   	"This file may be infected with"

#define MSG_THIS_MBR_MBI    	"\
This master boot record may be infected with"

#define MSG_THIS_SBS_MBI    	"\
This system boot sector may be infected with"

#define MSG_SM_MC           	"System memory may contain"
#define MSG_MBI             	"This may be infected with"

#define MSG_FOUND_IN_MEM	"\
This signature was found in system memory. The following message line is\n\
normally displayed when this virus is found in a file or boot sector:\n"

#define MSG_MAY_IND_VARIANT 	"\
This may indicate the presence of a variant of the virus.\n"

#define MSG_FOUND_IN_MEM_03 	"\n\
However, the signature was found in system memory, not in a file or\n\
in a boot sector.\n"

#define MSG_UNABLE_TO_OPEN  	"\
Virscan was unable to open (%s) for scanning.\n"

#define MSG_SCANNING_MEM    	"Scanning system memory for%sviruses"
#define MSG_SCANNING_ADDR_R 	"Scanning address range %05lx-%05lx\n"

#define MSG_MALLOC_ERROR    	"\
Out of memory error: malloc() (%d) in get_signatures()\n"

#define MSG_OUT_OF_MEMORY   	"\
The program ran out of memory for signatures.\n"

#define MSG_TRY_DECREASING  	"Try decreasing the number of signatures\n"
#define MSG_OOM_MSG2        	"Try running without -m option.\n"

#define MSG_SCANNING_SBS    	"\
Scanning system boot sector of %c: for boot sector viruses"

#define MSG_BS_NAME          	"system boot sector of drive %s"

#define MSG_SCANNING_MBR    	"\
Scanning master boot record of physical drive %02X for boot sector viruses"

#define MSG_MBR_NAME        	"master boot record of drive %02X"

#define MSG_OOM_STRDUP      	"\
Out of memory error: strdup() in output_positive_filename()\n"

#define MSG_MMATCHED_BYTES  	" (%d mismatched bytes)"
#define MSG_FRAGMENT        	" (fragment)"
#define MSG_OFFSET          	" (offset %ld (%lXH))"

#define MSG_BAD_VIRUS		"\
Warning, this virus may interact badly with virscan if the virus\n\
is resident in system memory. Before continuing, please verify\n\
that you have\n\
1) cold-booted from a write protected floppy diskette,\n\
2) are running the copy of virscan.exe on that diskette,\n\
3) have used virscan.exe to scan the diskette and have\n\
verified that the diskette is free of any viruses,\n\
4) and did not run any programs from any other disk between\n\
booting from that diskette and starting virscan.\n\n\"

#define MSG_YOU_HAVE_BEEN   	"You have been warned.\n"

#define MSG_PRESS_C_TO_CONT	"\
Press C to continue scan, or any other key to abort.\n"

#define MSG_ABORTING_SCAN   	"Aborting scan.\n"

#define MSG_RESTART_SCAN    	"\n\
Restart the scan with the -C option if you wish to ignore this error.\n"

#define MSG_CONTINUING_SCAN 	"\
(Continuing scan. This file cannot normally be opened.)\n"

#define MSG_ERROR_READING_F 	"Error reading (%s)\n"
#define MSG_ERROR_OPEN_SF   	"Error opening signature file (%s)\n"

#define MSG_ERROR_O_SF		"\
The signature file could not be read. Make sure that it was\n\
properly specified and that it exists.\n\n"

#define MSG_TESTING_SIGFILE 	"\
Testing signature file (%s) for modifications."

#define MSG_FILE_HAS_BEEN_M 	"The file (%s) has been modified.\n"
#define MSG_READING_SIGFILE 	"Reading signature file (%s)"
#define MSG_NO_SIGNATURES   	"No signatures to test for.\n"
#define MSG_FOUND_SIG       	"Found signature in (%s)"
#define MSG_FOUND_SIG_P     	"Found partial signature in (%s)"
#define MSG_FOUND_SIG_F     	"Found signature fragment in (%s)"
#define MSG_FOUND_SIG_PF    	"Found partial signature fragment in (%s)"

#define MSG_FOUND_SIG_AO    	"\
Found signature in (%s) at offset %ld (%lXH)\n"

#define MSG_FOUND_SIG_P_AO  	"\
Found partial signature in (%s) at offset %ld (%lXH)\n"

#define MSG_FOUND_SIG_F_AO  	"\
Found signature fragment in (%s) at offset %ld (%lXH)\n"

#define MSG_FOUND_SIG_PF_AO 	"\
Found partial signature fragment in (%s) at offset %ld (%lXH)\n"

#define MSG_BYTE_DIFFERENT  	"\
(%d byte was different than in the reference signature.)\n"

#define MSG_BYTES_DIFFERENT 	"\
(%d bytes were different than in the reference signature.)\n"

#define MSG_HEX_DUMP_BS     	"\n\
(Hex dump of boot sector to be scanned)\n"

#define MSG_ONE_OR_MORE     	"\n\
One or more virus signatures were found.\n"

#define MSG_ERR_READING_SBS 	"\
There was an error reading system boot sector for drive %s, INT 25 rc=%04X\n"

#define MSG_ATYPICAL_BPS    	"\
Drive %s has %d bytes per sector, which is atypical.\n"

#define MSG_NEVER_FORMATTED 	"\
The drive may never have been DOS formatted.\n"

#define MSG_SBS_NOT_SCANNED 	"\
The boot sector may not have been correctly scanned.\n"

#define MSG_SIG_OVERFLOW_01 	"\
Overflow: signature number %d in file (%s) is too long.\n"

#define MSG_SIG_OVERFLOW_02 	"\
Maximum length for a signature is %d bytes.\n"

#define MSG_SIG_OVERFLOW_03 	"\
The signature line in the signature file can be %d bytes long\n"

#define MSG_NO_WILDCARDS_01 	"\
No wildcard characters allowed in first two signature bytes.\n"

#define MSG_NO_WILDCARDS_02 	"\
First 4 characters of signature strings must be hexadecimal digits.\n"

#define MSG_SHORT_SIG_01    	"\
Signature of length 3 or less found\n"

#define MSG_SHORT_SIG_02    	"\
Signatures must be four bytes or more in length.\n"

#define MSG_ERR_READING_SML 	"\
Error reading message line for signature number %d in (%s).\n"

#define MSG_ERR_READING_SFT 	"\
Error reading file types line for signature %d in (%s).\n"

#define MSG_IUO_BANNER      	"IBM internal use Only\n"
#define MSG_VERSION_BANNER  	"Virus scanner      Version 2.00.00\n"
#define MSG_FILE_NOT_FOUND  	"File not found\n"
#define MSG_INV_PATH_SPEC   	"Invalid path specification.\n"
#define MSG_TOO_MANY_OPEN_F 	"Too many open files\n"
#define MSG_ACCESS_DENIED   	"Access denied\n"
#define MSG_INVALID_ACCESS  	"Invalid access\n"
#define MSG_DRIVE_NOT_READY 	"Drive not ready\n"

#define MSG_NOT_DOS_DISK    	"Not DOS Disk\n"

#define MSG_SHARING_VIOLA   	"Sharing violation\n"
#define MSG_SHARING_BUF_EXC 	"Sharing buffer exceeded\n"
#define MSG_NET_ACCESS_DENI 	"Network access denied\n"
#define MSG_CANNOT_MAKE     	"Cannot make\n"
#define MSG_INVALID_PARM    	"Invalid Parameter\n"
#define MSG_DRIVE_LOCKED    	"Drive locked\n"
#define MSG_OPEN_FAILURE    	"Open failed\n"
#define MSG_DISK_FULL       	"Disk full\n"
#define MSG_FILENAME_EXC_RA 	"Filename exceeds range\n"
#define MSG_BAD_NETPATH     	"Bad netpath\n"
#define MSG_UNIDENTIFIED_RC 	"unidentified rc=%d\n"

#define MSG_PATH_CAUSING_ER 	"\
The pathname that caused the error was (%s)\n"

#define MSG_INVALID_HANDLE  	"Invalid handle\n"
#define MSG_LOCK_VIOLATION  	"Lock violation\n"
#define MSG_BROKEN_PIPE     	"Broken pipe\n"
#define MSG_LOGFILE_VMSG    	"Virscan version: %s"
#define MSG_LOGFILE_TDMSG   	"Time/Date of start of scan: %s"

#define MSG_SCAN_TERMINATE  	"\
Scan terminated early due to error\n\
Please use the \"v\" option to help clarify why the error occurred.\n" 

#define MSG_CANT_HAPPEN		"Can't happen here...\n"

#define MSG_SCAN_VIA_LIST   	"\
scan_via_listfile(): error opening list of files (%s)\n"

#define MSG_FILELIST_ERR    	"\
The file list could not be read. Make sure that it was\n\
properly specified and that it exists.\n"

#define MSG_BOOT_SCAN_ERR	"\
Error scanning system boot sector of drive %s\n"

#define MSG_NETWORK_DRIVE	"\
(This is normal if drive %s is a network drive.)\n"

#define MSG_SCAN_ANOTHER	"\
Scan another diskette (Y/N)? "

#define MSG_MASTER_ERR          "\
Error reading master boot record of physical drive %02X, INT 13 rc=%04X\n"

#define MSG_MASTER_ERR_HARD     "\
Error reading master boot record of physical hard drive %02X, INT 13 rc=%04X\n"

#define MSG_MASTER_NOT_SCANNED	"\
This master boot record was not scanned.\n"

#define MSG_SCANNING		"Scanning %s\n"

#define MSG_INVALID_DIR 	"%s is an invalid directory.\n"

#define MSG_CANT_OPENDIR	"Cannot open directory (%s)\n"

#define MSG_CANT_STAT		"A status error occurred on file (%s)\n"

