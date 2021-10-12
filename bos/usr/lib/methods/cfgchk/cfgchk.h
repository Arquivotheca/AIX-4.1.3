/* @(#)46	1.6  src/bos/usr/lib/methods/cfgchk/cfgchk.h, cfgmethods, bos411, 9428A410j 6/13/93 10:25:34 */
/*
 * COMPONENT_NAME: CFGMETH - Configuration Method
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifndef _H_CFGCHK
#define _H_CFGCHK


/****************************************************************
* Include file for cfgchk().  BB stands for the BlueBonnet	*
* SCSI adapter board.  TR stands for the Token Ring adapter.	*
* PN stands for Part Number.  VPD stands for Vital Product Data.*
* The DEFAULT messages must be in the same order as their 	*
* equivalents in cfgchk.msg.					*
****************************************************************/
#ifndef TRUE
  #define TRUE  1
  #define FALSE 0
#endif

#define SCREEN_DELAY 	15		/* # of seconds for msg		*/
#define MAX_MSG_LEN 	300		/* Max # of char's in error msg */
#define MAXSLOTS 	8		/* Max # of adapter slots	*/
#define VPD_PN_STR 	"PN"		/* Identifies part number in VPD*/

#define CLEAR_SCREEN() \
	printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n")

#define DEFAULT_ERR_MSG1 "\
*** WARNING! ***\n\n\
A POTENTIAL CONFLICT EXISTS IN YOUR SYSTEM HARDWARE\n\
CONFIGURATION.  OPERATING YOUR SYSTEM WITH THIS\n\
CONFIGURATION MAY RESULT IN UNDETECTED DATA LOSS.\n\
CONTACT YOUR SERVICE REPRESENTATIVE AND REPORT\n\
THE FOLLOWING SERVICE REQUEST NUMBER:\n%s\n"

#define DEFAULT_ERR_MSG2 "\
*** WARNING! ***\n\n\
A POTENTIAL CONFLICT EXISTS IN YOUR SYSTEM HARDWARE\n\
CONFIGURATION.  OPERATING YOUR SYSTEM WITH THIS\n\
CONFIGURATION MAY RESULT IN UNDETECTED DATA LOSS.\n\
CONTACT YOUR SERVICE REPRESENTATIVE AND REPORT\n\
THE FOLLOWING SERVICE REQUEST NUMBERS:\n%s\n"

#define DEFAULT_ERR_MSG3 "cfgchk():  Error retrieving ODM data.\n"
#define DEFAULT_ERR_MSG4 "cfgchk():  Error retrieving NVRAM data.\n"

#define DEFAULT_ERR_MSG5 "\
*** WARNING! ***\n\n\
THE MICROCODE PROGRAM REQUIRED BY A SYSTEM FEATURE\n\
CARD OR CARDS FAILED TO DOWNLOAD PROPERLY.  ALTHOUGH\n\
THE SYSTEM WILL CONTINUE TO RUN, OPERATING YOUR SYSTEM\n\
WITHOUT THE PROPER MICROCODE MAY RESULT IN UNDETECTED\n\
DATA LOSS.  SHUTDOWN AND RE-BOOT THE SYSTEM.  IF THE\n\
PROBLEM REOCCURS, PLEASE RUN YOUR SYSTEM DIAGNOSTICS\n\
PROGRAM ON THE RESOURCES IDENTIFIED BELOW TO DETERMINE\n\
THE PROBLEM:\n%s\n\n"

/****************************************************************
* The following are used to test for erroneous BB configurations*
****************************************************************/
#define BB_SRN "869-210"
#define BB_LVALUE "adapter/mca/hscsi"
#define OEM_LOWER 0x0001		/* Range of device ID's	used to	*/
#define OEM_UPPER 0x81ff		/* 	identify OEM cards.	*/
#define SIXTY_FOUR_PORT 0x61fd		/* 64-port card has OEM devid	*/ 
#define NUM_BB_PARTS 12
char *BB_part_nos[] =
	{
	"59F3527", "59f3527",
	"70F9735", "70f9735",
	"70F9794", "70f9794",
	"71F0114", "71f0114", 
	"71F0232", "71f0232",
	"71F1172", "71f1172"
	};


/****************************************************************
* The following are used to test for erroneous TR configurations*
****************************************************************/
#define TR_SRN "850-920"
#define TR_LVALUE  "adapter/mca/tokenring"
#define SK1_LVALUE "adapter/mca/colorgda"
#define SK2_LVALUE "adapter/mca/graygda"
#define NUM_TR_PARTS 10
char *TR_part_nos[] =
	{
	"39F7824", "39f7824",
	"53F6046", "53f6046",
	"53F6052", "53f6052",
	"53F6064", "53f6064",
	"74F4134", "74f4134"
	};

#endif /* end _H_CFGCHK  */
