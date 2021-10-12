static char sccsid[] = "@(#)27 1.2 src/bos/usr/lib/methods/common/cfgboot_tools.c, cfgmethods, bos41J, 9523A_all 95/06/05 18:00:01";
/*
 * COMPONENT_NAME: (CFGMETHODS) cfgboot_tools.c - RSPC config/boot routines
 *
 * FUNCTIONS:	get_resid_index
 *		get_bootdev_odm_name
 *		mk_nv_list
 *		get_bootparms
 *		get_odm_name
 *		mk_nv_name
 *		get_smallest_bax
 *		pciquery
 *		iplcb_get
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mdio.h>
#include <sys/iplcb.h>
#include <sys/rosinfo.h>
#include <odmi.h>
#include <sys/cfgdb.h>
#include <sys/cfgodm.h>
#include <cf.h>
#include <cfgresid.h>

int get_resid_index(struct CuDv *, struct PdDv *, CFG_DEVICE *,int);
int get_bootdev_odm_name(char *, char *);
int mk_nv_list(struct CuDv *, struct PdDv *, char *);
int get_bootparms(struct CuDv *, struct PdDv *, char *);
static int get_odm_name(char *, struct CuDv *);
static int mk_nv_name(struct CuDv *, struct PdDv *, CFG_DEVICE *, int, char *);
static ulong get_smallest_bax(int);
static int pciquery(int, char *, ulong *);
static int iplcb_get(void *, int, int, int);



/*
 * get_resid_index: Matches an ODM device with corresponding device
 *		      entry in resid data.  Returns index into residual
 *		      data device array.  If no match is found, returns -1.
 */
int
get_resid_index(cudv,pddv,devp,num)
struct CuDv *cudv;
struct PdDv *pddv;
CFG_DEVICE *devp;
int	num;

{
	int	rc;
	int	i;
	int	j;
	struct CuDv pcudv;
	struct CuAt cuat;
	struct PdAt pdat;
	char	sstr[128];
	int	pci_busnum;
	int	devfunc;
	ulong	ioaddr;
	int	cnt;
	CFG_iopack_t *iopkt;
	char	pnp[8];
	ulong	serial;


	if (!strcmp(pddv->subclass,"pci")) {
		/* Its a PCI device */

#ifdef BOOT_DEBUG
fprintf(stderr, "Its a PCI device\n");
#endif
		/* Get CuDv for parent bus */
		sprintf(sstr,"name=%s",cudv->parent);
		rc = (int)odm_get_first(CuDv_CLASS,sstr,&pcudv);
		if (rc == 0 || rc == -1)
			return(-1);

		/* Get PCI bus number attribute for parent bus */
		sprintf(sstr,"name=%s AND attribute=bus_number",pcudv.name);
		rc = (int)odm_get_first(CuAt_CLASS,sstr,&cuat);
		if (rc == -1) {
			return(-1);
		}
		else if (rc != 0) {
			pci_busnum = (int)strtoul(cuat.value, (char **)NULL, 0);
		}
		else {
			sprintf(sstr,"uniquetype=%s AND attribute=bus_number",
							pcudv.PdDvLn_Lvalue);
			rc = (int)odm_get_first(PdAt_CLASS,sstr,&pdat);
			if (rc == 0 || rc == -1)
				return(-1);
			pci_busnum = (int)strtoul(pdat.deflt, (char **)NULL, 0);
		}


		devfunc = (int)strtoul(cudv->connwhere, (char **)NULL, 10);

#ifdef BOOT_DEBUG
fprintf(stderr, "\tPCI bus number = %x\n\tDevFunc number = %x\n",pci_busnum,devfunc);
#endif

		/* A PCI device is matched using PCI bus number and DevFunc */
		for (i=0; i<num; i++) {
			if ((devp[i].deviceid.busid == PCI_DEVICE) &&
			    (devp[i].busaccess.pciaccess.busnumber
							== pci_busnum) &&
			    (devp[i].busaccess.pciaccess.devfuncnumber
							== devfunc)) {
				/* Found matching resid entry */
				return(i);
			}
		}

	}

	else if (!strcmp(pddv->subclass,"isa")) {
		/* Its an ISA device */

#ifdef BOOT_DEBUG
fprintf(stderr, "Its an ISA device\n");
#endif
		/* Get an I/O attribute for adapter */
		sprintf(sstr,"name=%s AND type LIKE O*",cudv->name);
		rc = (int)odm_get_first(CuAt_CLASS,sstr,&cuat);
		if (rc == -1) {
			return(-1);
		}
		else if (rc != 0) {
			ioaddr = strtoul(cuat.value, (char **)NULL, 0);
		}
		else {
			sprintf(sstr,"uniquetype=%s AND type LIKE O*",
							cudv->PdDvLn_Lvalue);
			rc = (int)odm_get_first(PdAt_CLASS,sstr,&pdat);
			if (rc == 0 || rc == -1)
				return(-1);
			ioaddr = strtoul(pdat.deflt, (char **)NULL, 0);
		}

#ifdef BOOT_DEBUG
fprintf(stderr, "\tI/O address = %x\n",ioaddr);
#endif

		/* An ISA device is matched using I/O address */
		for (i=0; i<num; i++) {
			if ((devp[i].deviceid.busid == ISA_DEVICE) &&
			    !(devp[i].deviceid.flags & INTEGRATED) &&
			    (!strcmp(devp[i].pnpid,pddv->devid))) {

#ifdef BOOT_DEBUG
fprintf(stderr, "\tFound matching PNP number\n");
#endif

				/* Get I/O packets from resid data */
				rc = get_io_packets(i, 'a', &cnt, &iopkt);
				if (rc)
					return(-1);

				for (j=0; j<cnt; j++) {
					if (iopkt[j].min == ioaddr) {
					/* Found matching resid entry */
						return(i);
					}
				}
			}
		}
	}

	else if (!strcmp(pddv->subclass,"isa_sio")) {
		/* Its an integrated ISA device */

#ifdef BOOT_DEBUG
fprintf(stderr, "Its an integrated ISA device\n");
#endif

		strncpy(pnp,cudv->connwhere,7);
		pnp[7] = '\0';
		serial = strtoul(&(cudv->connwhere[7]), (char **)NULL, 16);

#ifdef BOOT_DEBUG
fprintf(stderr,"\tPNP number = %s\n",pnp);
fprintf(stderr,"\tSerial number = %x\n",serial);
#endif

		/* Integrated ISA devices are matched on PNP and serial nums */
		for (i=0; i<num; i++) {
			if ((devp[i].deviceid.busid == ISA_DEVICE) &&
			    (devp[i].deviceid.flags & INTEGRATED) &&
			    (devp[i].deviceid.serialnum == serial) &&
			    (!strcmp(devp[i].pnpid,pnp))) {
				/* Found matching resid entry */
				return(i);
			}
		}
	}
#ifdef BOOT_DEBUG
fprintf(stderr, "Its an unsupported device type: {%s}\n",pddv->uniquetype);
#endif

	return(-1);
}


/*
 * get_bootdev_odm_name: Finds ODM device corresponding to device
 *		      identified by fw-boot-device
 */
int
get_bootdev_odm_name(bootlist,bootname)
char	*bootlist;
char	*bootname;
{
	char	*p;
	char	*dev_id;
	char	*end_list;
	struct CuDv cudv;
	int	rc;


	/* bootlist is a fw-boot-device style device identifier.  This */
	/* consists of a sequence of individual device identifiers that */
	/* identify all devices in path from bus to end boot device. */

#ifdef BOOT_DEBUG
fprintf(stderr, "fw-boot-device=%s\n",bootlist);
#endif

	/* Process each individual device in list */
	end_list = bootlist + strlen(bootlist);
	p = bootlist;
	while(p != end_list) {
		/* Skip initial 'slash' character */
		p++;

		/* Set up pointer to current device identifier */
		dev_id = p;

		/* Find start of next device identifier */
		while (*p != '/' && *p != '\0') p++;

		/* NULL terminate current device identifier */
		*p = '\0';

		/* Convert device identifier to a CuDv object */
		 rc = get_odm_name(dev_id,&cudv);
		if (rc)
			return(rc);
	}

	/* Found CuDv for boot device */
	strcpy(bootname,cudv.name);
	return(0);
}


/*
 * get_odm_name: Finds ODM device which is a child of "cudv" and
 *		 identified by substring from fw-boot-device
 */
static int
get_odm_name(dev_id, cudv)
char	*dev_id;
struct CuDv *cudv;
{
	char	*type;
	char	*addr;
	char	*parms;
	char	*p;
	int	rc;
	char	sstr[256];
	CFG_DEVICE *dp;
	int	num;
	CFG_pci_descriptor_t  *pci;
	int	i;
	int	busnum;
	int     cnt;
	int	devnum;
	int	func;
	int	sid;
	int	lun;
	int	connection;
	ulong	ioaddr;
	struct CuDv *cudvlist;
	struct listinfo cudv_info;
	struct CuAt *cuat;
	int     j;
	ulong	tst_ioaddr;
	ulong	min_io;


#ifdef BOOT_DEBUG
fprintf(stderr, "\tDevice string=%s\n",dev_id);
#endif

	/* Assume format of device identifier to be type@addr:parms */
	/* or just type@addr. */
	/* Parse into three pieces */
	type = dev_id;
	for (p=type; *p!='@'; p++);		/* Skip to '@' character */
	*p = '\0';

	addr = p + 1;
	for (p=type; *p!=':' && *p!='\0'; p++);	/* Skip to ':' character */
	if (*p == ':') {
		*p = '\0';
		p++;
	}
	parms = p;				/* Points to '\0' if no parms */


	/* Now start decoding */

	if (!strcmp(type,"pci")) {

		/* PCI system bus */

		ioaddr = strtoul(addr, (char **)NULL, 16);

		/* Get resid device table */
		if (get_resid_dev(&num, &dp) != 0) {
			return(-1);
		}

		/* Look for top level PCI bus */
		for (i=0; i<num; i++) {
			if ((dp[i].deviceid.busid == PROCESSOR_DEVICE) &&
			    (dp[i].deviceid.basetype == Bridge_Controller) &&
			    (dp[i].deviceid.subtype == PCI_Bridge)) {

				/* Found PCI bus, see if it has right address */
				min_io = get_smallest_bax(i);
				if (ioaddr != min_io)
					continue;

				/* Found correct PCI bus, get PCI bus number */
				get_pci_descriptor(i,'a', &cnt, &pci);
				busnum = pci->busnum;
				break;
			}
		}

		/* Get CuDv for the PCI bus */
		sprintf(sstr,"parent=sysplanar0 AND connwhere=4.%d AND status=1",busnum);
		rc = (int)odm_get_first(CuDv_CLASS,sstr,cudv);
		if (rc == 0 || rc == -1) {
			return(-1);
		}
	} /* End PCI system bus */

	else if (!strncmp(type,"pci",3)) {

		/* PCI device */

		/* Parse addr for device number and function number */
		/* addr will point to device number string, p to function */
		for (p=addr; *p!=','; p++);	/* Skip to ',' character */
		*p = '\0';
		p++;

		/* Get numeric values for device and function numbers */
		devnum = strtoul(addr, (char **)NULL, 16);
		func = strtoul(p, (char **)NULL, 16);

		/* Get CuDv for the PCI device */
		sprintf(sstr,"parent=%s AND connwhere=%d AND status=1",cudv->name,8*devnum+func);
		rc = (int)odm_get_first(CuDv_CLASS,sstr,cudv);
		if (rc == 0 || rc == -1) {
			return(-1);
		}
	} /* End PCI device */

	else if (!strcmp(type,"harddisk")
		 || !strcmp(type,"tape") 
		 || !strcmp(type,"cdrom")
		 || !strcmp(type,"floppy")) {

		/* Disk, tape, CDROM, or floppy */

		/* If the addr string has a ',' then device is SCSI */
		for (p=addr; *p!=',' && *p!='\0'; p++);
		if (*p == ',') {

			/* Its SCSI format with ID and LUN */
			*p = '\0';
			p++;
			sid = strtoul(addr, (char **)NULL, 16);
			lun = strtoul(p, (char **)NULL, 16);

			/* Set up ODM search arg for the SCSI device */
			sprintf(sstr,"parent=%s AND connwhere=%d,%d AND status=1",cudv->name,sid,lun);
		} /* End SCSI device */

		else {
			/* Its IDE or floppy */
			connection = strtoul(addr, (char **)NULL, 16);

			/* Set up ODM search arg for the device */
			sprintf(sstr,"parent=%s AND connwhere=%d AND status=1",cudv->name,connection);
		}

		/* Get CuDv for the device */
		rc = (int)odm_get_first(CuDv_CLASS,sstr,cudv);
		if (rc == 0 || rc == -1) {
			return(-1);
		}
	} /* End disk, tape, CDROM, or floppy */

	else {
		/* ISA device */

		/* addr string is the I/O address */
		ioaddr = strtoul(addr, (char **)NULL, 16);

		/* Get all children of the ISA bus */
		sprintf(sstr,"parent=%s AND status=1",cudv->name);
		cudvlist = odm_get_list(CuDv_CLASS,sstr,&cudv_info,16,1);
		if (cudvlist == NULL || (int)cudvlist == -1) {
			return(-1);
		}

		/* Get attributes for each device, look for matching I/O addr */
		for (i=0; i<cudv_info.num; i++) {
			cuat = getattr(cudvlist[i].name, NULL, TRUE, &cnt);
			for (j=0; j<cnt; j++) {
				if (cuat[j].type[0] == 'O') {
					tst_ioaddr = strtoul(cuat[j].value, (char **)NULL, 0);
					if (ioaddr == tst_ioaddr) {
						/* Found device */
						*cudv = cudvlist[i];
						free(cuat);
						odm_free_list(cudvlist,&cudv_info);
						return(0);
					}
				}
			}
			free(cuat);
		}

		odm_free_list(cudvlist,&cudv_info);
			return(-1);
	}

	return(0);
}


int
mk_nv_list(c_ptr,p_ptr,p)
struct CuDv *c_ptr;
struct PdDv *p_ptr;
char	*p;
{
	CFG_DEVICE *devp;
	int	num;
	struct CuDv *cudv_p;
	struct CuDv cudv;
	struct PdDv *pddv_p;
	struct PdDv pddv;
	char	sstr[128];
	char	n[256];
	char	tmp[512];
	int	rc;


	/* Initialize returned string to a NULL string in case of error */
	*p = '\0';

	/* get the number of devices and the device info from resid data */
	rc = get_resid_dev(&num, &devp);	/* libcfg function */
	if (rc)
		return(-1);

	cudv_p = c_ptr;
	pddv_p = p_ptr;
	while(cudv_p) {

		/* Make portion of name corresponding to CuDv object */
		rc = mk_nv_name(cudv_p,pddv_p,devp,num,n);
		if (rc)
			return(rc);

		/* Insert name in front of string to be returned */
		strcpy(tmp,p);
		strcpy(p,n);
		strcat(p,tmp);

		/* If not to system bus, then process next parent */
		if (strncmp(cudv_p->PdDvLn_Lvalue,"bus/sys/",8)) {
			/* Get parent CuDv and PdDv */
			sprintf(sstr,"name=%s", cudv_p->parent);
			rc = (int)odm_get_first(CuDv_CLASS,sstr,&cudv);
			if (rc==0) {
				/* failed to get an object */
				return(E_NOCuDv);
			}
			else if (rc==-1) {
				/* ODM error */
				return(E_ODMGET);
			}

			cudv_p = &cudv;

			sprintf(sstr,"uniquetype=%s", cudv_p->PdDvLn_Lvalue);
			rc = (int)odm_get_first(PdDv_CLASS,sstr,&pddv);
			if (rc==0) {
				/* failed to get an object */
				return(E_NOPdDv);
			}
			else if (rc==-1) {
				/* ODM error */
				return(E_ODMGET);
			}

			pddv_p = &pddv;
		}

		else {
			/* Processed all devices up to and including top bus */
			cudv_p = NULL;		/* Set ptr to NULL for exit */
		}
	}

	return(0);
}

static int
mk_nv_name(cudv, pddv, devp, num, n)
struct CuDv *cudv;
struct PdDv *pddv;
CFG_DEVICE *devp;
int	num;
char	*n;

{
	int	i;
	CFG_pci_descriptor_t  *pci;
	ulong	min_io;
	int	busnum;
	int	cnt;
	ulong	devid;
	ulong	v_id;
	ulong	d_id;
	ulong	devfunc;
	CFG_iopack_t *iopkt;
	int	index;
	int	rc;


	if (!strncmp(pddv->uniquetype,"bus/sys/",8)) {
		/* Top level bus...assume PCI bus */

		/* Get PCI bus number from connwhere */
		busnum = (uchar)strtoul(strchr(cudv->connwhere,'.')+1,NULL,10);

		/* Find PCI bus in resid data by matching PCI bus number */
		for (i=0; i<num; i++) {
			if ((devp[i].deviceid.busid == PROCESSOR_DEVICE) &&
			    (devp[i].deviceid.basetype == Bridge_Controller) &&
			    (devp[i].deviceid.subtype == PCI_Bridge)) {
				/* Check PCI bus num in bridge descriptor */
				get_pci_descriptor(i,'a', &cnt, &pci);
				if (pci->busnum == busnum) {
					/* Found correct device entry */
					/* Get addr translation descriptors */
					min_io = get_smallest_bax(i);
					break;
				}
			}
		}
		sprintf(n,"/pci@%x",min_io);
	}

	else if (!strcmp(pddv->subclass,"pci")) {

		/* Its a PCI device */
		devfunc = strtoul(cudv->connwhere,NULL,10);
		devid = strtoul(pddv->devid, (char **)NULL, 0);
		if (devid == 0) {
			/* devid in PdDv may be NULL for ISA bridges */
			/* So read bytes from PCI cfg space */
			rc = pciquery(devfunc,cudv->parent,&devid);
			if (rc == -1)
				return(-1);
		}

		v_id = ((devid & 0x00FF0000)>>8) | ((devid & 0xFF000000)>>24);
		d_id = ((devid & 0x0000FF00)>>8) | ((devid & 0x000000FF)<<8);

		sprintf(n,"/pci%x,%x@%x,%x",v_id,d_id,devfunc/8,devfunc&0x7);
	}

	else if (!strcmp(pddv->subclass,"isa") ||
					!strcmp(pddv->subclass,"isa_sio")) {

		/* Its an ISA device */
		index = get_resid_index(cudv,pddv,devp,num);
		if (index == -1)
			return(-1);

		/* Get I/O packets from resid data */
		rc = get_io_packets(index, 'a', &cnt, &iopkt);
		if (rc)
			return(-1);

		sprintf(n,"/%s@%x",devp[index].pnpid,iopkt[0].min);
	}

	else if (!strcmp(pddv->subclass,"scsi") ||
					!strcmp(pddv->subclass,"ide")) {
		if (!strcmp(pddv->class,"disk")) {
			sprintf(n,"/harddisk@%s",cudv->connwhere);
		}

		else if (!strcmp(pddv->class,"cdrom")) {
			sprintf(n,"/cdrom@%s",cudv->connwhere);
		}

		else if (!strcmp(pddv->class,"tape")) {
			sprintf(n,"/tape@%s",cudv->connwhere);
		}
	}

	else if (!strcmp(pddv->class,"diskette")) {
		sprintf(n,"/floppy@%s",cudv->connwhere);
	}

	else {
		/* Not a possible boot device */
		return(-1);
	}

	return(0);
}

/*
 * get_smallest_bax: Gets address translation descriptors from residual
 *		     data for the device whose index in the residual
 *		     data device table is "index".  It returns the smallest
 *		     system address for which there is an I/O mapping.  If
 *		     there are no I/O mapping, then it returns the smallest
 *		     system address for which there is a bus memory mapping.
 */
static ulong
get_smallest_bax(index)
int	index;
{
	int	rc;
	int	num;
	int	i;
	CFG_bax_descriptor_t	*ptr;
	unsigned long long	min_io;
	unsigned long long	min_mem;


	min_io = min_mem = -1;

	/* Get all vendor packets for bus */
	if (get_bax_descriptor(index, 'a', &num, &ptr) != 0)
		return(-1);

	/* Look for smallest I/O address translation descriptor */
	for (i=0; i<num; i++) {
		if (ptr->conv == 2) {
			if (ptr->sys < min_io)
				min_io = ptr->sys;
		}
		else if (ptr->conv == 1) {
			if (ptr->sys < min_mem)
				min_mem = ptr->sys;
		}
		ptr++;
	}

	if (min_io == -1)
		return(min_mem);

	return(min_io);
}

static int
pciquery(devfunc,bus,devid)
int	devfunc;
char	*bus;
ulong	*devid;
{
	int	rc;
	MACH_DD_IO      mddRecord;
	int     fd;
	char	devbus[32];


	/* build mdd record */
	mddRecord.md_size = 1;
	mddRecord.md_incr = MV_WORD;
	mddRecord.md_addr = 0;
	mddRecord.md_sla = devfunc;
	mddRecord.md_data = (uchar*)devid;

	sprintf(devbus,"/dev/%s",bus);
	if ((fd = open(devbus, O_RDWR, 0)) < 0) {
		return(-1);
	}

	rc = ioctl(fd, MIOPCFGET, &mddRecord);
	close(fd);
	return(rc);
}


/*
 * get_bootparms: Determines if device identified by 'cudv' is the boot
 *		device.  If it is, the parameters at the end of the
 *		fw-boot-device NVRAM variable are returned in 'parms'.
 *		If it is not the boot device, a NULL string is returned
 *		in 'parms'.
 */
int
get_bootparms(cudv, pddv, parms)
struct CuDv *cudv;
struct PdDv *pddv;
char	*parms;
{
	IPL_DIRECTORY iplcb_dir;	/* IPL control block directory */
	IPL_INFO iplcb_info;		/* IPL control blk info section */
	char	bl[2048];
	char	test_bl[2048];
	char	*p1;
	int	rc;
	int	num;
	CFG_DEVICE *devp;


	*parms = '\0';

	/* Get directory section of IPL control block		*/
	/* it starts at offset of 128 into IPL control block	*/
	if (iplcb_get(&iplcb_dir,128,sizeof(iplcb_dir),MIOIPLCB)) {
		return(-1);
	}

	/* Get iplinfo structure. */
	if (iplcb_get(&iplcb_info,iplcb_dir.ipl_info_offset,
					sizeof(iplcb_info), MIOIPLCB)) {
		return(-1);
	}

	strcpy(bl, &(iplcb_info.previpl_device[3]));

#ifdef BOOT_DEBUG
fprintf(stderr,"boot device string from IPL CB: '%s'\n",bl);
#endif

	/* Find last device in boot device string */
	p1 = &bl[strlen(bl)-1];
	while(*p1 != '/') p1--;

	/* Now find parameters, if any, and replace ':' with NULL */
	while(*p1 != ':' && *p1 != '\0') p1++;
	if (*p1 == ':') {
		*p1 = '\0';
		p1++;
	}
	/* p1 now points to parm string or '\0' */

	rc = get_resid_dev(&num, &devp);
	if (rc)
		return(-1);

	rc = mk_nv_list(cudv, pddv, test_bl);
	if (rc)
		return(-1);

	if (!strcmp(bl,test_bl))
		strcpy(parms,p1);

	return(0);
}



/*
 * iplcb_get: Read in section of the IPL control block.  The directory
 *		section starts at offset 128.
 */
static int
iplcb_get(dest, address, num_bytes, iocall)
void	*dest;
int	address;
int	num_bytes;
int	iocall;
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
