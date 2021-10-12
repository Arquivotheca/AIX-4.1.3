/* @(#)25        1.1  src/bos/usr/lib/methods/common/cfgide.h, cfgmethods, bos41J, 9512A_all 3/20/95 22:29:31 */
#ifndef _H_CFGIDE
#define _H_CFGIDE
/*
 *   COMPONENT_NAME: CFGMETHODS
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
	ide definitions
*/
#define	DONE		1
#define	USED		1
#define	BB_BUS_ID	0x800c0020
/* default model names must be 40 characters */
#define	DEFLT_DISK_TYP	"Generic_IDE_Disk                        "
#define DEFLT_TAPE_TYP  "DEFAULT_TAPE                            "
#define DEFLT_CDROM_TYP "DFLT_IDE_CDROM                          "
#define DISK_OTHER_TYPE  "disk/ide/oidisk"
#define CDROM_OTHER_TYPE "cdrom/ide/oicdrom"
#define TAPE_OTHER_TYPE  "tape/ide/oit"
#define NULLPVID "00000000000000000000000000000000"
#define	IDE_VPDSIZE	256
#define	IDE_IDENTSIZE	512
#define	SUP_TIMEOUT	60	/* seconds */
#define	SUPI_TIMEOUT	30	/* seconds */
#define TUR_WAIT	2
#define MAX_ID		2

#define	IDE_DISK	0
#define IDE_TAPE	0x1
#define IDECDROM	0x05
#define	TUR_RETRYS	6
#define	PRT_OFFSET	11
#define	PRT_LEN		16
#define LENGTH_BYTE	4	/*where length of vpd is stored*/
#define	CRF	(S_IFCHR|S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)

#define	CHGDB_ONLY	1
#define	NO_PAGE		-1

#define RD_BASE         2	/* real devices 	*/
#define RD_INC          8
#define MN_BASE         32	/* model name		*/
#define MN_INC          4
#define CD_BASE         8	/* custom devices	*/
#define CD_INC          8

/* values for handling the IDE id map */
#define UNKNOWN 0
#define NEWDISK 1
#define EMPTY   2
#define DEVICE  3
#define MAXSID  7
#define NUMSIDS 8
#define PVIDSIZE 33
#define MODELNAMESIZE 41	/* IDE model names are 20 shorts */


/* structure for model name attribute information */
struct mna {
	char    value1[UNIQUESIZE];
	char    value2[MODELNAMESIZE];
};

/* structure for CuDv and pvid information */
struct cust_device {
	int     status;
	struct CuDv cudv;
	char    pvid[PVIDSIZE];
};

/* structure for information about a device found to be attached */
struct real_device {
	int     status;
	char    utype[UNIQUESIZE];
	char    pvid[PVIDSIZE];
	char    connect[3];
	uchar   dev_is_disk;
};


int
def_ide_children (char *lname, int ipl_phase);

#endif /* _H_CFGHIDE */
