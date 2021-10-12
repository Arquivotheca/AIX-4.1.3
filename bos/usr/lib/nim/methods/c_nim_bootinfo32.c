static char sccsid[] = "@(#)76 1.1 src/bos/usr/lib/nim/methods/c_nim_bootinfo32.c, cmdnim, bos41J, 9516B_all 95/04/21 13:00:09";
/*
 * COMPONENT_NAME: (BOSBOOT) Base Operating System Boot
 *
 * FUNCTIONS: c_nim_bootinfo32
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1990, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *   LEVEL 1,  5 Years Bull Confidential Information
 *
 * This command is used to provide boot information used
 * to provide information about the key position and the
 * ROS network IPL support.  The code is directly derived from
 * src/usr/sbin/bootinfo/bootinfo.c.
 *
 * EXECUTION ENVIRONMENT
 *
 *	The bootinfo command is usually run from the
 *	RAM file system during boot and BOS installation.
 *	The command can also be executed from a running
 *	system.
 *
 *	The device configuration databases and NVRAM are
 *	used to extract required information. The ODMDIR
 *	environment must be set to the appropriate path.
 *
 */

#include <sys/types.h>
#include <sys/cfgdb.h>
#include <sys/cfgodm.h>
#include <sys/mdio.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <cf.h>
#include <odmi.h>
#include <lvm.h>
#include <macros.h>
#include <sys/iplcb.h>
#include <nl_types.h>
#include <locale.h>
#include <sys/errno.h>

#define MSGCAT		"bootinfo.cat"

int _system_configuration = 0;	/* Just used to resolve a dangling reference */

/*
 * function prototype declarations
 */
int iplcb_get(void *dest, int address, int num_bytes, int iocall);
int ros_bootable_adap(char *adap);


/* --------------------------------------------------------------------------*/
main(int argc, char *argv[])
{
extern int	optind;		/* for getopt function */
extern char	*optarg;	/* for getopt function */

/*
 * Option flags and variables
 */
int	errflg = 0;
nl_catd catd;
char	*msgfmt = NULL;
int	c = 0;
int	keyposn = 0;
int	key = 0;
int	rosbootable = 0;
char	*bootadap = NULL;
int	fd = 0;		/* file descriptor */
MACH_DD_IO	mdd;

	/*
	 * Internationalize!
	 */
	char *p;
	p = setlocale(LC_ALL, "");
	catd = catopen(MSGCAT, NL_CAT_LOCALE);

	/* parse command line parameters */
	while ((c = getopt(argc,argv,"kq:")) != EOF) {
		switch (c) {
		case 'k':
			keyposn = 1;
			break;
		case 'q':
			rosbootable = 1;
			bootadap = optarg;
			break;
		case '?':
			errflg = 1;
		}
	}

	if (errflg) {
		/* error parsing parameters */
		msgfmt = catgets(catd, 250, 1,
		"\nusage: c_nimbootinfo -k |-q adapter\n");
		fprintf(stderr, msgfmt);
		fprintf(stderr, msgfmt);
		exit(1);
	}

	if (keyposn) {
		/*
		 * DISPLAY VALUES:
		 *	1 - secure or locked position
		 *	2 - service
		 *	3 - normal
		 */
		if ((fd = open("/dev/nvram", O_RDONLY)) != -1)
		{
			mdd.md_incr = MV_WORD;
			mdd.md_data = (uchar *)&key;
			mdd.md_size = 1;
			errno=0;
			ioctl(fd, MIOGETKEY, &mdd);
			/* Let's try the direct route  - required for pre-325 */
			if (errno != 0) {
			    iplcb_get(&key,0x4000e7,1,MIOCCGET);
			    key>>=24;
			}
			close(fd);
		} else {
			exit(1);
		}

		key &= 0x03;
		if (key > 3)
			exit(1);
		printf("%d\n", key);
		exit(0);
	}

	/* Initialize the ODM */
	if (odm_initialize() == -1) {
		msgfmt = catgets(catd, 250, 21,
		"\n 0301-268 Object Data Manager initialize failed!\n");
		fprintf(stderr, msgfmt);
		exit(15);
	}

	if( rosbootable )
		printf("%d\n",ros_bootable_adap(bootadap));

	odm_terminate();
	exit(0);
}

/* --------------------------------------------------------------------------*/
/*
 * iplcb_get: Read in section of the IPL control block.  The directory
 *		section starts at offset 128.
 */
int
iplcb_get(void *dest, int address, int num_bytes, int iocall)
{
	int		fd;		/* file descriptor */
	MACH_DD_IO	mdd;


	if ((fd = open("/dev/nvram",0)) < 0) {
		return(-1);
	}

	mdd.md_addr = (long)address;
	mdd.md_data = dest;
	mdd.md_size = (long)num_bytes;
	mdd.md_incr = MV_BYTE;

	if (ioctl(fd,iocall,&mdd)) {
		return(-1);
	}

	close(fd);
	return(0);
}

/* --------------------------------------------------------------------------*/
/*
 * ros_bootable_adap: For a given adapter, output 1 if ROS
 *		      supports booting from the adapter.
 *		      Otherwise it returns 0.
 *		      This is meant to be used by NIM for ROS emulation.
 */
int
ros_bootable_adap(char *adap)
{
IPL_DIRECTORY iplcb_dir;	/* IPL control block directory */
GLOBAL_DATA iplcb_glob;		/* IPL control blk GLOBAL DATA  */
struct  adapt_info *adapter_info;
char	sstr[256];
struct CuDv cudv;
struct PdDv pddv;
int	rc, busid, slot, boot_ok, array_indx;

	boot_ok = 0;   /* assume device does NOT support boot */
	sprintf(sstr, "name='%s'", adap);
	if( (rc = (int)odm_get_first(CuDv_CLASS,sstr, &cudv)) == -1 ) {
			perror("Error reading CuDv Object Class");
			exit(1);
	}
	if( !rc ) exit(1);

	sprintf(sstr, "uniquetype='%s'", cudv.PdDvLn_Lvalue);
	if( (rc = (int)odm_get_first(PdDv_CLASS,sstr, &pddv)) == -1 ) {
			perror("Error reading PdDv Object Class");
			exit(1);
	}
	if( !rc ) exit(1);

	/*  if subclass is not mca or sio, cannot boot from them */
	if( strcmp(pddv.subclass,"mca") && strcmp(pddv.subclass,"sio"))
		return(0);

	/* Some of the devices on sio have non-numeric connwhere values.
	   atoi() will return 0 and set errno for these, but slot will
	   work out ok for sio.  */
	slot = atoi(cudv.connwhere);
	busid = cudv.location[3] - '0';
	if( slot > 15 ) return(0); 			   /* sgabus */
	if( !slot) {
		/* return bootable for sio and diskette adapter */
		if( !strncmp(cudv.PdDvLn_Lvalue,"adapter/mca/sio",15) ||
		    !strncmp(cudv.PdDvLn_Lvalue,"adapter/sio/fda",15) )
			return(1); /* sio or fda adapter */
		else    return(0); /* other adapters at slot 0 is not bootable*/
	}
	slot--;				/* adjust 1 for hw slot number */

	/*
	 * get directory section of IPL control block
	 * it starts at offset of 128 into IPL control block
	 */
	if (iplcb_get(&iplcb_dir,128,sizeof(iplcb_dir),MIOIPLCB)) {
		perror("Error reading IPL-Ctrl block directory");
		return(0);  /* Does not support boot */
	}

	if ((int)(&iplcb_dir.global_offset) <
		((int)(&iplcb_dir) + iplcb_dir.ipl_info_offset-128)) {
		/*
		 * get GLOBAL Data structure.
		 */
		if (iplcb_get(&iplcb_glob,iplcb_dir.global_offset,
				sizeof(iplcb_glob),MIOIPLCB)) {
			perror("Error reading IPL-Ctrl block Global section");
			return(0);  /* Does not support boot */
		}

		array_indx = busid * (MAX_SLOT_NUM +1) + slot;
		adapter_info = (struct adapt_info *)
				(&iplcb_glob.fm2_adapt_info[array_indx]);
		boot_ok = (adapter_info->supports_ipl==0) ? 0 : 1;
	}
	else {
		/* return bootable for  bluebonnet, lace or badisk */
		if( !strcmp(cudv.PdDvLn_Lvalue,"adapter/mca/hscsi") ||
		    !strcmp(cudv.PdDvLn_Lvalue,"disk/mca/badisk"))
			boot_ok = 1;
	}

	return(boot_ok);
}
