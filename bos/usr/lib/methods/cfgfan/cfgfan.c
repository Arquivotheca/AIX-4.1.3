/*
 * @(#)51  1.2  src/bos/usr/lib/methods/cfgfan/cfgfan.c, cfgmethods, bos41B, 412_41B_sync 1/12/95 15:47:49
 *
 * COMPONENT_NAME: FGMETHODS
 *
 * FUNCTIONS: main, parse_options, donothing
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * LEVEL 1,  5 Years Bull Confidential Information
 */

#include <stdio.h>
#include <sys/mdio.h>
#include <locale.h>
#include <nl_types.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/cfgodm.h>
#include <sys/utsname.h>
#include <odmi.h>
#include <cf.h>

#define FAN_SLOW 0x02
#define FAN_FAST 0x03

#define NB_MAX_MCA_CARDS    7
#define NB_MAX_CPU_CARDS    4
#define NB_MAX_MEM_CARDS    4

#define NB_MAX_CABINETS 8
#define NB_SCSI_DEVICES 32

#define CPUCARDSTR "name like cpucard*"
#define MEMCARDSTR "name like mem*"
#define CABINETSTR "name like 'cabinet*' AND status=1"
#define SCSIDEVSTR "PdDvLn like '*/scsi/*' and status=1 and location like '00-0*'"
#define MCACARDSTR "PdDvLn like 'adapter/mca/*' and status=1 and location like '00-0*'"

#define FANS_STATUS 0x1

int 	cabinet;
int 	status = 0;
uchar	funct = FAN_FAST;
unsigned long len = 0;
unsigned char buffer[256];
static int errflg, vflag;

char odm_string[50];

struct  utsname unstr;

/*
 * it is a prototype version
 * TODO: 
 *      - Set and Check privilege for execution.
 *      - Walk through all cabinets
 *      - add option : -s (slow) -f(fast) -c CabNumber 
 *        to force speed fans for a target Cabinet
 *      - redirect outputs /var/adm/fans.act with date of changes
 *      - resolve problems of some mca boards in Defined state.
 *      - Cleaning for some trace not needed on verbose mode.
 *      - message catalogue. 
 *      -....
 */
main(int argc, char **argv)
{
	int     nvram;
	MACH_DD_IO param;
	struct listinfo listmem;
	struct listinfo listcpu;
	struct listinfo listcab;
	struct listinfo listscsid;
	struct listinfo listmca;
	struct CuDv *cudv_mem_cards;
	struct CuDv *cudv_cpu_cards;
	struct CuDv *cudv_cabinet;
	struct CuDv *cudv_scsi_devices;
	struct CuDv *cudv_mca_cards;
	int i;
	int     rc;
	int     num_mem=0;
	int     num_cpu=0;
	int     num_cab=0;
	int     num_mca=0;
	int     num_scsi_devs=0;
	int 	dev_present=0;
	int 	dev_powered=0;
	int 	mask=0;


	struct  utsname *un = &unstr;
	char *p;
	
	parse_options(argc, argv);

	if (uname(un) < 0) {
		fprintf(stderr, "uname system call failed\n");
		exit(1);
	}
	if (vflag)
		 fprintf(stdout, "Model ID: %.*s\n", SYS_NMLN, un->machine);

	p = un->machine;
	/* 
	 * test mm field for Peg. A 601 (Deskside), fan control is
	 * specific to this model.
	 */
	if (0 != strncmp(p+8,"A0",2)) {
		fprintf(stderr,
			"Command not allowed to be executed on this model\n");
		exit(0);
	}
	
	odm_initialize();

	cudv_mem_cards = (struct CuDv *) odm_get_list(CuDv_CLASS, MEMCARDSTR,
		 &listmem, NB_MAX_MEM_CARDS, 1);
	if ((int) cudv_mem_cards == -1) {
		fprintf(stderr,
			"Error reading CuDv for Memory cards\n");
		exit(1);
	}
	num_mem = listmem.num;

	cudv_cpu_cards = (struct CuDv *) odm_get_list(CuDv_CLASS, CPUCARDSTR,
		 &listcpu, NB_MAX_CPU_CARDS, 1);
	if ((int) cudv_cpu_cards == -1) {
		fprintf(stderr,
			"Error reading CuDv for cpu cards\n");
		exit(1);
	}
	num_cpu = listcpu.num;

	cudv_mca_cards = (struct CuDv *) odm_get_list(CuDv_CLASS, MCACARDSTR,
		 &listmca, NB_MAX_MCA_CARDS, 1);
	if ((int) cudv_mca_cards == -1) {
		fprintf(stderr,
			"Error reading CuDv for mca cards\n");
		exit(1);
	}
	num_mca = listmca.num -1; /* forget adapter/mca/sio_XXXX */

	/* for future use */
	cudv_cabinet = (struct CuDv *) odm_get_list(CuDv_CLASS, CABINETSTR,
		 &listcab, NB_MAX_CABINETS, 1);
	if ((int) cudv_cabinet == -1) {
		fprintf(stderr,
			"Error reading CuDv for cabinets\n");
		exit(1);
	}
	num_cab = listcab.num;

	cudv_scsi_devices = (struct CuDv *) odm_get_list(CuDv_CLASS, SCSIDEVSTR,
		 &listscsid, NB_SCSI_DEVICES, 6);
	if ((int) cudv_scsi_devices == -1) {
		fprintf(stderr,
			"Error reading CuDv for scsi devices\n");
		exit(1);
	}
	num_scsi_devs = listscsid.num;

	if (vflag) {
		fprintf(stderr,"Number of memcards:       %d\n", num_mem);
		fprintf(stderr,"Number of cpucards:       %d\n", num_cpu);
		fprintf(stderr,"Number of mcacards:       %d\n", num_mca);
		fprintf(stderr,"Number of cabinets:       %d\n", num_cab);
		fprintf(stderr,"Number of scsi devices:   %d\n", num_scsi_devs);
	}
	
	/* open nvram to enable ioctls usage */
	nvram = open("/dev/nvram", O_RDWR);
	if (nvram == -1) {
		fprintf(stderr,"cfgfan: Unable to open /dev/nvram\n");
		exit(0);
	}

	if (num_cpu > 1) donothing();          /* cpuboads   available */
	else if (num_mem > 3) donothing();     /* memboards  available */
	     else if (num_mca > 3) 
	               /* mca boards available                         */
	               /* Some works should be done in this area       */
	               /* case removing adapters with rmdev command    */
	               /* for exp. removing slot location of cards     */
	               /* and not updating ODM database ans so on      */
	               /* We suppose now that if mca adapters presents */
                       /* and AVAILABLE and this state will not be     */  
	               /* changed once we boot the system.             */
	               donothing();
	          else {
	               if (num_scsi_devs <= 5 ){/* scsi devs  available */
	               	/* reduce fans speed                            */
	               	funct = FAN_SLOW;
                        if (vflag)
                                printf("Bring fans speed to SLOW\n");
	               }
	               else {
		       /* make sure that fans are FAST                 */
	               	funct = FAN_FAST;
                        if (vflag)
                                printf("Bring fans speed to FAST\n");
	                }
			/* We focus only on the base cabinet as it was asked  */
        		/* Let's ignore the fans status in the case of        */
			/* presence of others.                                */ 
			cabinet = 0;
			param.md_size = 1;
			param.md_data = (char *) buffer;
			param.md_incr = MV_BYTE;
			param.md_cbnum = cabinet;
			param.md_length = &len;
			param.md_cmd = funct;
			rc = ioctl(nvram, MPOWERSET, &param);
			if (rc == -1) {
				fprintf(stderr,"cfgfan: ioctl returns Error\n");
				close(nvram);
				exit(0);
			}
	          } /*else*/
	
	if (vflag) { 
		/* Simple check is fans speed went as we wanted */ 
		param.md_size = 256;
		param.md_incr = MV_BYTE;
		param.md_data = buffer;
		param.md_cbnum = cabinet;
		param.md_length = &len;
		len = 0;
		rc = ioctl(nvram, MPOWERGET, &param);
		if (rc == -1) {
			fprintf(stderr,"cfgfan: ioctl returns Error\n");
			close(nvram);
			exit(0);
		}
		status = (buffer[14] >>4) & FANS_STATUS;

		if (status) 
			printf("============= FANS in the cabinet [%d] in FAST mode =============\n",cabinet);
		else   
			printf("============= FANS in the cabinet [%d] in LOW mode ==============\n",cabinet);

		/* get number of devices presents and devices powered ON */ 	
		for (i=0; i<8; ++i) {
			mask = 1 << i;
			if (i<2) {
				if (!(buffer[11] & mask))
					dev_powered++;
			}
			if (i>=2 && i<4) {
				if (!(buffer[11] & mask))
					dev_present++;
			}
			if (!(buffer[8] & mask))
				dev_powered++;
			if (!(buffer[15] & mask))
				dev_powered++;
			if (!(buffer[9] & mask))
				dev_present++;
			if (!(buffer[16] & mask))
				dev_present++;
		}
		fprintf(stderr,"Devices present in cab %d : %d \n",
			 cabinet, dev_present);
		fprintf(stderr,"Devices powered in cab %d : %d \n",
			 cabinet, dev_powered);
	} /* verbose mode */

	close(nvram);

	odm_free_list(cudv_mem_cards,&listmem);
	odm_free_list(cudv_cpu_cards,&listcpu);
	odm_free_list(cudv_cabinet,&listcab);
	odm_free_list(cudv_scsi_devices,&listscsid);
	odm_free_list(cudv_mca_cards,&listmca);
	odm_terminate();

} /* main */
	
parse_options( int argc, char **argv)
{
	extern char *optarg;
	extern int optind;
	char   *getstr = "vh";
	char   *usage = "Usage: cfgfan [-v]\n";
	int     c;

	errflg = vflag = 0; 


	while ((c = getopt(argc, argv, getstr)) != EOF) {
		switch (c) {
			/* for future add options:                      */
			/*  -s(slow) -f(fast) -c(Cabnum)                */
			/* to force speed fans to SLOW or FAST a target */
			/* cabinet.                                     */
			case 'v':
				vflag++;
				break;
			case 'h':
			case '?':
			default:
				errflg++;
		} /* case */
			if (errflg) {
				fprintf(stderr, "%s\n", usage);
				exit(1);
			}
	}   /* while */

	argc -= optind;
	if (argc != 0) {
		fprintf(stderr,"%s\n", usage);
		exit(1);
	}
}  /* parse_options */

donothing()
{
	/* for verbose and traces purpose to be sent to /var/adm/fans.act */
	/* future use. Just return now.                                   */        
	return;
}
