static char sccsid[] = "@(#)94	1.19.1.8  src/bos/usr/lib/methods/cfgbus/bcm.c, cmdbuscf, bos411, 9436C411a 9/8/94 08:35:34";
/*
 *   COMPONENT_NAME: (CMDBUSCF) Main Bus Configuration module
 *
 *   FUNCTIONS: bus_config_method
 *		main
 *		mk_sp_file
 *		set_busid
 *	        get_iplcb	
 *              setup_bimw_attr
 *	        chk_ccm	
 *	        open_classes	
 *	        close_classes	
 *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/cfgdb.h>
#include <sys/cfgodm.h>
#include <cf.h>
#include <sys/mode.h>
#include <sys/mdio.h>
#include <sys/iocc.h>
#include <sys/systemcfg.h>
#include <sys/iplcb.h>

#include "bcm.h"


struct  Class   *predev;        /* predefined devices class ptr */
struct  Class   *cusdev;        /* customized devices class ptr */
struct  Class   *preatt;        /* predefined attributes class ptr */
struct  Class   *cusatt;        /* customized attributes class ptr */

extern char *optarg;
int	    allpkg = 0 ;



main(argc, argv)
int argc;
char **argv;
{
	int i, c, rc;
	int phase = 0;
	char *busname = NULL;
	
	/* check if environment variable set to install all packages */
	if ( !strcmp(getenv("DEV_PKGNAME"),"ALL") )
		allpkg = 1;

	while (EOF != (c = getopt(argc, argv, "12dl:"))) {
		switch (c) {
		case '1':
			phase = PHASE1;
			break;
		case '2':
			phase = PHASE2;
			break;
		case 'd':
			prntflag = TRUE;
			trace_file = fopen( "BUS.out", "w" );
			break;
		case 'l':
			busname = optarg;
			break;
		default :
			exit(E_ARGS);
			break;
		}
	}


	/* Initialize the ODM */
	if (odm_initialize() == -1)
		exit(E_ODMINIT);

	/* Open object classes and keep open for multiple accesses */
	rc = open_classes();
	if (rc == 0)
		rc = bus_config_method(phase, busname);

	/* Close all object classes */
	(void)close_classes();

	/* Terminate the ODM */
	odm_terminate();

	exit( rc );
}


int
bus_config_method(phase, busname) 
int phase;
char *busname;
{
	int rc;
	char conf_list[2000], not_res_list[2000];
	struct CuDv cudv;
	char bus_devname[100];
	unsigned short card_table[CARD_TABLE_SIZE];
	char sstr[80];	/* ODM search string */


	/* Set the bus device's LED value */
	if (phase)
		setleds(BUS_CONFIG_METHOD);

	/* Get the bus's CuDv object */
	sprintf(sstr, "name=%s", busname);
	rc = (int)odm_get_first(CuDv_CLASS, sstr, &cudv);
	if (rc == -1)
		return(E_ODMGET);	/* ODM failure */
	else if (rc == 0)
		return(E_NOCuDv);	/* No CuDv for the bus */

	/* Set up bus's /dev name for later use */
	sprintf(bus_devname,"/dev/%s", busname);

	/* Only perform bus config operations if bus is DEFINED */
	if (cudv.status == DEFINED) {

		/* 
	 	 *  Make the special file for the bus.  Remember that
	 	 *  the Bus Id is the same as the connwhere.
 		 */
		if (!mk_sp_file(bus_devname, (int)(cudv.location[3] - '0')))
			return(E_MKSPECIAL);  /* Couldn't make special files */

		/* Try to set the busid attribute.  Abort if it fails. */
		if ((rc = set_busid(&cudv)) != 0)
			return(rc) ;

		/*
		 * Set up the bus_iocc_mem attribute value to have the  
		 * correct setting for IOCC or XIO machines. 
		 */
		if ((rc = setup_bimw_attr(busname, bus_devname)) != 0)
			return(rc) ;        

		/* set the bus status to available */
		cudv.status = AVAILABLE;
		if ((int)odm_change_obj(CuDv_CLASS, &cudv) == -1)
			return(E_ODMUPDATE);
	}


	/* Always walk bus to discover child adapters */
	busquery( card_table, CARD_TABLE_SIZE, bus_devname, phase);

	/* Restore bus device's LED value since busquery will change LEDs */
	if (phase)
		setleds(BUS_CONFIG_METHOD);

	/* Match up devices with CuDv objects in data base */
	if (rc = sync_bus(busname, phase, card_table, CARD_TABLE_SIZE))
		return(rc);

	/*  check if CCM device present and reserve CCM attrs if needed */
	if (rc = chk_ccm(busname))
		return(rc);

	/* Resolve bus resources */
	rc = busresolve(NULL, phase, conf_list, not_res_list, busname);
	if (rc)
		return (rc);

	/* Print the list of adapters that were successfully resolved and
	 * are to be configured to standard out.
	 */
	fprintf(stdout, "%s\n", conf_list);
	return(0);
} /* END bus_config_method */

/*
 * NAME     : setup_bimw_attr()
 * FUNCTION : Sets the bimw attribute value to 0x0 on XIO machines
 *            and 0x10 on IOCC machines.  It identifies the amount of
 *            bus memory to reserve starting at the address of 0xfffff0.
 *            On IOCC machines the address range 0xfffff0-0xffffff must
 *            be reserved due to an IOCC problem.  If the bimw attribute
 *            is set to 0x0, no memory will be reserved.
 * INPUTS   :
 *      busname : bus being configured
 *      devname : bus's special file name
 *
 * RETURNS : 0     => successfully updated CuAt.
 *           other => error code indicating failure (capable of being
 *                    returned by the caller).
 */
int
setup_bimw_attr(char *busname, char *devname)
{

	struct CuAt *cuat;
	int cucnt; 
	char xio_value[] = "0x0";
	char iocc_value[] = "0x10";
	int fd;
	MACH_DD_IO mddRecord;
	ulong cfgreg;


	/* Get the width attribute */
	if ((cuat = getattr(busname, "bimw", 0, &cucnt)) == NULL) {
		return(E_NOATTR);
	}

	if (__power_pc()) {
		/* treat like XIO */
		strcpy(cuat->value, xio_value);
	}
	else /* __power_rs() */ {
		/* Check to see if it's XIO or IOCC */

		if ((fd = open(devname, O_RDWR)) == -1)
			return(E_OPEN); 

		mddRecord.md_size = 1;
		mddRecord.md_incr = MV_WORD;
		mddRecord.md_data = (uchar *)&cfgreg;
		mddRecord.md_addr = IO_IOCC + 0x10;
		if (ioctl(fd, MIOCCGET, &mddRecord) == -1) {
			close(fd);
			return(E_DEVACCESS); 
		}
		close(fd);

		if (cfgreg & XIOBIT)
			/* Its XIO */ 
			strcpy(cuat->value, xio_value);
		else
			/* Its IOCC */ 
			strcpy(cuat->value, iocc_value);
	}
	putattr(cuat);
	return (0); 
} /* END setup_bus_iocc_mem_attr() */

/* 
 * NAME     : mk_sp_file
 * FUNCTION : generates the special file for the bus. It selects       
 *            the minor number based on the bus number passed in
 *            (see below for more information).
 * INPUTS   :
 *      Fname  : file name to be created.
 *      BusNum : 1 byte bus number
 * NOTES    :
 *      Note that the minor number is tied to the bus 
 *      number as follows (this relationship is also 
 *      documented in sys/io/machdd/md.c for bos325 and
 *      src/bos/kernel/io/machdd/md.c for bos410) : 
 *       
 *       	 BUS_NUM    MINOR_NUMBER
 *        	  '0'	         0
 *        	  '1'           16
 *        	  '2'    	32
 *        	  '3'	        48
 * RETURNS : FALSE => could not create special file
 * 	     TRUE  => successfully created special file.
 */
int
mk_sp_file(Fname, bus_num)

char	*Fname;		/* name of special file to create (include /dev/) */
int	bus_num;	/* bus number                                     */

{
	int	rc;	/* return code returned by this routine.  */
	int	mode;	/* file mode mask used to create file.    */
	dev_t	devno;	/* device number to use when creating file*/
	int	minor;	/* device minor number                    */


	/* Compute device minor number and device number */
	minor = (bus_num << 4) & 0xf0;
	devno = (dev_t)(0x030000 | minor);   

	/* Make the bus special file */
	mode = S_IFCHR | S_IRUSR | S_IWUSR | S_IRGRP ;
	rc = unlink(Fname) ;
	rc = (mknod(Fname, mode, devno) == 0) ;

	return(rc) ;
}

/* 
 * NAME     : set_busid
 * FUNCTION : Sets the "bus_id" attribute in CuAt to indicate 
 *            the correct busid for the bus being configured.
 * INPUTS   :
 *      cudv : the CuDv object of the bus being configured.
 *
 * RETURNS : 0     => successfully set "bus_id" attribute.
 * 	     other => error code indicating failure (capable of being
 *		      returned by the caller.
 */
int 
set_busid(cudv)
struct CuDv     *cudv;	       /* CuDv obj for bus being configured */

{
	char		sstr[128];
	struct CuAt	*cuat;
	struct PdAt	pdat;
	int		i, rc, offset, cnt;
	uint		loc_offset;
	ulong		cuat_busid, bus_id;
	IPL_DIRECTORY	iplcb_dir;	/* IPL control block directory */
	struct buc_info buc;		/* IPL cntrl block buc info section */
	ulong		buid;


	/* Get buid */

	if (__power_rs())
		buid = 0x20 | ( cudv->location[3] - '0' );

	else { /* Power PC */
		/*
		 * Read in the IPL Control Block directory 
		 * and get offset to first info buc section 
		 */
		rc = get_iplcb((char *)&iplcb_dir, 128,
					sizeof(iplcb_dir), MIOIPLCB);
		if (rc) return(rc);
		offset = iplcb_dir.buc_info_offset;

		/* 
		 * Read in the first info buc section of the IPL  
		 * Control Block so we can get number of structs
		 * for our loop counter, i  
		 */
		rc = get_iplcb((char *)&buc, offset, sizeof(buc), MIOIPLCB);
		if (rc) return(rc);

		/*
		 * Loop through buc array structures to find the BUID value 
		 */
		for (i = buc.num_of_structs; i--; offset += buc.struct_size) {

			/* Read each info buc section of the IPLCB */
			rc = get_iplcb((char *)&buc, offset,
						sizeof(buc), MIOIPLCB);
			if (rc) return(rc);

			/* Find buc entry for IOCC with location == busnum */
			if (buc.IOCC_flag) {
				/* Then this array struct is for an IOCC */

				/*
				 * An early version of IPL ROS does not build
				 * the struct with location field - check for
				 * that by comparing location field offset
				 * against struct size. If early version then
				 * only one IOCC BUC entry, so just use it.  
				 */

				loc_offset = (uint)buc.location - (uint)&buc;
				if (loc_offset < buc.struct_size)  {
				    /* Entry has valid location field */

				    if (cudv->location[3] == buc.location[2]) {
					/* This is entry containing BUID */
					buid =
					    (ulong)buc.buid_data[0].buid_value;
					break;
				    } 
				} 

				else {
				    /* IPL ROS early version */
				    /* This is only IOCC BUC struct */
				    buid = (ulong)buc.buid_data[0].buid_value; 
				    break;
				}
			}
		} /* end for */ 

		/* if did not find a buc that matched, can't continue  */
		if ( i < 0 )
			return (E_DEVACCESS);
    
	}  /* else if POWER PC */


	/* And after all that we can form the BUS ID */
	bus_id = 0x80000000 | (buid << 20);

	if (prntflag) fprintf(trace_file, "bus id = %x\n",bus_id);

 	/* Check bus_id in CuAt, redefine it if required */
	cuat = getattr(cudv->name, "bus_id", FALSE, &cnt);
	if (cuat == NULL)
		return(E_NOATTR);

	/* See if current busid is correct */
	cuat_busid = strtoul(cuat->value, (char **)NULL, 0);
	if (cuat_busid == bus_id)
		/* The bus_id is already correct */
		return (0);

	/* Need to set busid to correct value */
	sprintf(cuat->value, "0x%x", bus_id);

	putattr(cuat) ;
	return (0); 

} /* END set_busid */ 


/*
 * NAME: get_iplcb
 *
 * FUNCTION: Reads "num_bytes" bytes from the IPL control block.
 *           Bytes are read from the offset into the IPL
 *           control block and stored at address "dest".
 *
 * RETURNS:  error code.  0 means no error.
 */
int
get_iplcb(dest, offset, num_bytes, ioctl_type)
char	*dest;
int	offset;
int	num_bytes;
int	ioctl_type;
{
	int		fd;	/* file descriptor */
	MACH_DD_IO	mdd;

	if ((fd = open("/dev/nvram", O_RDONLY)) == -1)
		return(E_DEVACCESS);

	mdd.md_addr = offset;
	mdd.md_data = dest;
	mdd.md_size = num_bytes;
	mdd.md_incr = MV_BYTE;


	if (ioctl(fd, ioctl_type, &mdd) == -1) {
		close(fd);
		return(E_DEVACCESS);
	}

	close(fd);
	return(0);
}

/*
 * NAME: chk_ccm
 *
 * FUNCTION: Checks if the FRS_Add_MCA_Device routine has
 * 	     ran and added a special predefined attribute and 
 *	     CuAt attribute.  If so, CuAt entries are added
 *  	     to reserve bus memory.  If CuAt entries
 *	     are present without the "frs"
 *	     attribute, then delete the attributes, as they should not
 *	     be used to reserve bus memory.
 *
 * RETURNS:  error code.  0 means no error.
 */
int
chk_ccm ( busname )
char *busname;
{
	struct	CuAt	cuat;
	struct	CuDv	cudv;
	struct  PdAt    *pdatptr;       /* ptr to list of PdAt objects */
	struct  listinfo   db_list;     /* list info of PdAt objects */
	char		criteria[MAX_CRITELEM_LEN];
	int		rc;
	int		i;
	int		reserve_mem;


	/* get list of PdAt frs attributes */
        pdatptr = odm_get_list(PdAt_CLASS, 
		  "uniquetype like adapter/mca/* and attribute=frs", 
		  &db_list, 2, 1);
        if ( (int)pdatptr == -1) {
                return(E_ODMGET);
        }

	reserve_mem=FALSE;
        /* loop through objects found in PdAt */
        for (i=0; i< db_list.num; i++) {
			
		/* get the corresponding CuDv */
		sprintf(criteria,"PdDvLn=%s",pdatptr[i].uniquetype);
		rc = (int)odm_get_first(CuDv_CLASS, criteria, &cudv);
		if (rc == -1) {
			return (E_ODMGET);
		}
		if (rc != 0) {
			/* check if CuAt frs attribute present */
			sprintf(criteria,"name=%s and attribute=frs",
				cudv.name);
			rc = (int)odm_get_first(CuAt_CLASS, criteria,
			      &cuat);
			if (rc == -1) {
				return (E_ODMGET);
			}
			if (rc == 0) {  /* CuAt frs attribute not present */
				/* reserve bus memory */
				reserve_mem=TRUE;
				break;
			}
		}
	}

	if (reserve_mem) {

		/* build CuAt to add */

		strcpy(cuat.name, busname);
		strcpy(cuat.attribute, "rcrw1");
		strcpy(cuat.value, "0x4000");
		strcpy(cuat.type, "W");
		strcpy(cuat.generic, "");
		strcpy(cuat.rep, "nl");
		cuat.nls_index = 0;

		/* add 3 attributes for the CCM bus memory */
		rc = putattr(&cuat);

		strcpy(cuat.attribute, "rcrw2");
		strcpy(cuat.value, "0x100000");

		putattr(&cuat);

		strcpy(cuat.attribute, "rcrw3");
		strcpy(cuat.value, "0x2000");

		putattr(&cuat);
	}
	else {
		/* delete the CCM bus memory objects */
		sprintf(criteria,"name=%s and attribute like rcrw*", busname);
		odm_rm_obj(CuAt_CLASS,criteria);
	}
	return(0);	
}

/*
 * NAME: open_classes
 *
 * FUNCTION: Opens the four object classes: PdDv, CuDv, PdAt, CuAt.
 * 	     They are kept open for multiple accesses.
 *
 * RETURNS:  error code.  0 means no error.
 */
int
open_classes ( void )
{

	/* open predefined devices object class */
	if ((int)(predev = odm_open_class(PdDv_CLASS)) == -1)
		return(E_ODMOPEN);

	/* open customized devices object class */
	if ((int)(cusdev = odm_open_class(CuDv_CLASS)) == -1)
		return(E_ODMOPEN);

	/* open predefined attribute object class */
	if ((int)(preatt = odm_open_class(PdAt_CLASS)) == -1)
		return(E_ODMOPEN);

	/* open customized attribute object class */
	if ((int)(cusatt = odm_open_class(CuAt_CLASS)) == -1)
		return(E_ODMOPEN);

	return(0);
}

/*
 * NAME: close_classes
 *
 * FUNCTION: Closes the four object classes: PdDv, CuDv, PdAt, CuAt.
 *
 * RETURNS:  error code.  0 means no error.
 */
int
close_classes ( void )
{
	odm_close_class(CuAt_CLASS);
	odm_close_class(PdAt_CLASS);
	odm_close_class(CuDv_CLASS);
	odm_close_class(PdDv_CLASS);
	return(0);
}
