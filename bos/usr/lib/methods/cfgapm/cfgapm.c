static char sccsid[] = "@(#)74	1.4.1.1  src/bos/usr/lib/methods/cfgapm/cfgapm.c, cfgcommo, bos41J, 9508A 2/8/95 16:49:00";
/*
 * COMPONENT_NAME: (CFGMETH) cfgapm
 *
 * FUNCTIONS: build_dds, generate_minor, make_special_files
 * FUNCTIONS: download_microcode, query_vpd, define_children
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1992
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <fcntl.h>
#include <stdio.h>
#include <sys/iocc.h>
#include <sys/mdio.h>
#include <sys/types.h>
#include <sys/cfgodm.h>
#include <sys/cfgdb.h>
#include <cf.h>
#include "cfgdebug.h"
#include <sys/artic.h>
#include <sys/mpqp.h>
#include <sys/mpqpdd.h>
extern int odmerrno;

#define S_LENGTH  256           /* length of buffer for get commands */

/*
 *
 * NAME: build_dds
 *
 * FUNCTION: Builds a DDS (Defined Data Structure) describing a device's
 *	characteristics to it's device driver.
 *
 * EXECUTION ENVIRONMENT:
 *	Operates as a device dependent subroutine called by the generic
 *	configure method for all devices.
 *
 * NOTES: There is no DDS for the MPQP adapter (DDS's are built for the child
 *        MPQP ports) this is then a NULL function.
 *
 * RETURNS: 0 - success, in all cases
 *
 */

int  build_dds( lognam, addr, len )
char	*lognam;		/* logical name of device */
uchar	*addr;			/* receiving pointer for DDS address */
long	*len;			/* receiving variable for DDS length */
{
	return 0;
}
/*
 *
 * NAME: generate_minor
 *
 * FUNCTION: To provide a unique minor number for the current device instance.
 *
 * EXECUTION ENVIRONMENT:
 *     Operates as a device dependent subroutine called by the generic
 *     configure method for all devices.
 *
 * NOTES: There is no minor number required for the MPQP adapter,
 *        this function is NULL.
 *
 * RETURNS: 0 - success, in all cases
 *
 */

int generate_minor( lognam, majorno, minordest )
char	*lognam;
long	majorno;
long	*minordest;
{
	return 0;
}

/*
 * NAME: make_special_files
 *
 * FUNCTION: To create the special character file on /dev for the current device
 *           instance.
 *
 * EXECUTION ENVIRONMENT:
 *     Operates as a device dependent subroutine called by the generic
 *     configure method for all devices.
 *
 * NOTES: For the MPQP adapter, there is no special character file on /dev, this
 *        function is then NULL.
 *
 * RETURNS: 0 - success, in all cases
 *
 */

int make_special_files( lognam, devno )
char	*lognam;
dev_t	devno;
{
	return 0;
}

/*
 *
 * NAME: download_microcode
 *
 * FUNCTION: To download the micro code for the MPQP adapter.
 *
 * EXECUTION ENVIRONMENT:
 *     Operates as a device dependent subroutine called by the generic
 *     configure method for all devices.
 *
 * NOTES: For the MPQP adapter, micro code must be downloaded through a port,
 *        this function is then NULL.
 *
 * RETURNS: 0 - success, in all cases
 *
 */

int  download_microcode( lognam )
char  *lognam;
{
	return 0;
}

/*
 *
 * NAME: query_vpd
 *
 * FUNCTION: To query the MPQP device for VPD information (Vital Product Data)
 *
 * EXECUTION ENVIRONMENT:
 *     Operates as a device dependent subroutine called by the generic
 *     configure method for all devices.
 *
 * NOTES: For the MPQP adapter, VPD information is not currently supported, this
 *        function is then NULL.
 *
 * RETURNS: 0 - success, in all cases
 *
 */

int  query_vpd( newobj, kmid, devno, vpd_dest )
char   *newobj;
mid_t   kmid;
dev_t	devno;
char	*vpd_dest;
{
	return 0;
}

/*
 * NAME     : ucfg_apm
 *
 * FUNCTION : This function changes the adapter's state from AVAILABLE to
 *	DEFINED when an error occurs.
 *
 * EXECUTION ENVIRONMENT :
 *      This function returns 0 on success, < 0 on failure.
 *      Operates as a device dependent subroutine called by the generic
 *      configure method for all devices.
 *
 * NOTES :
 *      The portmaster adapter's state will be AVAILABLE at this time,
 *      so since there was an error detected the adapter's state should
 *      be changed to DEFINED.           
 *
 */

#define APM_UNIQUETYPE "uniquetype = 'adapter/mca/portmaster'"

void ucfg_apm(lname)
char    *lname;                 /* logical name of the device           */
{
        struct  PdDv   pddvadptr;
        int rc;
        char string[S_LENGTH];

        if(( rc = odm_get_obj( PdDv_CLASS, APM_UNIQUETYPE, &pddvadptr, ODM_FIRST)) == 0 ) {
                DEBUG_2("ucfg_apm: get failed lname=%s rc=%d\n", lname ,rc)
                return;
        }

        sprintf( string, "-l %s ", lname );
        if(odm_run_method(pddvadptr.Unconfigure,string,NULL,NULL)){
                fprintf(stderr,"ucfg_apm: can't run %s\n", pddvadptr.Unconfigure);
                return;
        }

	return;
}
/*
 * NAME     : def_mpqpd
 *
 * FUNCTION : This function defines the children of the parent
 *	device assuming that this device is the mpqp 4 port card.
 *
 * EXECUTION ENVIRONMENT :
 *      This function returns 0 on success, < 0 on failure.
 *      Operates as a device dependent subroutine called by the generic
 *      configure method for all devices.
 *
 * NOTES :
 *      The portmaster adapter's child devices are the device drivers
 *      that may be configured to run with this adapter.  The standard
 *      device is the mpqp device driver.
 *
 */

#define MPQP_UNIQUETYPE "uniquetype = 'driver/portmaster/mpqp'"

int def_mpqd(char *lname)
{
	struct  PdDv   pddvport;
	int rc;
	char string[S_LENGTH];

	if(( rc = odm_get_obj( PdDv_CLASS, MPQP_UNIQUETYPE, &pddvport, ODM_FIRST )) == 0 ) {
		DEBUG_2("def_mpqd: get failed lname=%s rc=%d\n", lname ,rc)
		return E_NOPdDv;
	}

	sprintf( string, "-c driver -s portmaster -t mpqp -p %s -w 0 ", lname );
	if(odm_run_method(pddvport.Define,string,NULL,NULL)){
		fprintf(stderr,"cfgapm: can't run %s\n", pddvport.Define);
		return E_ODMRUNMETHOD;
	}

	return 0;

}

/*
 * NAME     : define_children
 *
 * FUNCTION : This function returns the names of devices that are
 *            attached (defined) to this device.  This functions
 *            searches the customized database for devices that have
 *            this device listed as the parent device.
 *
 * EXECUTION ENVIRONMENT :
 *      This function returns 0 on success, < 0 on failure.
 *      Operates as a device dependent subroutine called by the generic
 *      configure method for all devices.
 *
 * NOTES :
 *      The portmaster adapter's child devices are the device drivers
 *      that may be configured to run with this adapter.  The standard
 *      device is the mpqp device driver.
 *
 * RECOVERY OPERATION :
 *
 * DATA STRUCTURES :
 *
 * RETURNS :
 *      Returns 0 on success, > 0 on failure.
 */

int
define_children(lname,ipl_phase)
char    *lname;                 /* logical name of the device           */
int     ipl_phase;              /* ipl phase                            */
{
	struct  Class   *predev ;       /* predefined devices class ptr */
	struct  Class   *cusdev ;       /* customized devices class ptr */
	struct  PdDv    preobj ;        /* predefined device object     */
	struct  CuDv    cusobj ;        /* customized device object     */
	char    sstring[S_LENGTH];      /* search criteria              */
	int     rc;                     /* return code                  */
	char    *out_p;
	uchar   mpqp_4_port=0;

	/* declarations for card query */
	char    busdev[32] ;
	int     fd ;
	struct  CuAt    *cusatt;
	ulong   ulong_val;
	char    msg_no[4];      /* string to hold NLS test msg num. */
	ulong   bus_io_addr;    /* bus i/o register address */
	ulong   bus_mem_addr;   /* bus memory address */
	ulong   intr_lvl;       /* interrupt level */
	char    subtype[4];     /* adapter EIB type */
	MACH_DD_IO mddRecord;
	uchar pos[2];           /* storage for EIB value */
	uchar temp_pos[1];      /* storage for pos register value */
	int slot_num;
	int c_i;

	DEBUG_0("cfgapm: define children\n")

	/* open customized device object class */
	if ((int)(cusdev=odm_open_class(CuDv_CLASS)) == -1) {
		/* error opening class */
		DEBUG_0("cfgapm: open of CuDv failed\n")
		err_exit(E_ODMOPEN);
	}

	DEBUG_0("cfgapm: card query\n")

	/* Get parent device and where connected */
	sprintf(sstring,"name = '%s'",lname);
	rc =(int)odm_get_first(cusdev,sstring,&cusobj);
	if (rc==0) {
		/* failed to get an object */
		DEBUG_0("cfgapm: No device CuDv object\n")
		err_exit(E_NOCuDv);
	}
	else if (rc==-1) {
		/* ODM error */
		DEBUG_0("cfgapm: ODM error getting CuDv object\n")
		err_exit(E_ODMGET);
	}

	DEBUG_3("cfgapm: name = %s, parent = %s, connwhere = %s\n",
	    lname,cusobj.parent, cusobj.connwhere)

	/* Read attribute from CuAt, and PdAt tables */
	if(( rc = getatt( &bus_mem_addr, 'i', CuAt_CLASS,
	    PdAt_CLASS, lname, cusobj.PdDvLn_Lvalue,
	    "bus_mem_addr", (struct attr *) NULL )) > 0 ) 
	{
		DEBUG_0("cfgapm: ODM error getting bus_mem_addr\n")
		ucfg_apm(lname);            
		return E_BADATTR;
	}

	/* Read attribute from CuAt, and PdAt tables */
	if(( rc = getatt( &bus_io_addr, 'i', CuAt_CLASS,
	    PdAt_CLASS, lname, cusobj.PdDvLn_Lvalue,
	    "bus_io_addr", (struct attr *) NULL )) > 0 ) 
	{
		DEBUG_0("cfgapm: ODM error getting bus_io_addr\n")
		ucfg_apm(lname);            
		return E_BADATTR;
	}

	/* Read attribute from CuAt, and PdAt tables */
	if(( rc = getatt( &intr_lvl, 'i', CuAt_CLASS,
	    PdAt_CLASS, lname, cusobj.PdDvLn_Lvalue,
	    "bus_intr_lvl", (struct attr *) NULL )) > 0 ) 
	{
		DEBUG_0("cfgapm: ODM error getting bus_intr_lvl\n")
		ucfg_apm(lname);            
		return E_BADATTR;
	}

	/* Read attribute from CuAt, and PdAt tables */
        if ( (( cusatt = get_attrval( lname, "_subtype",
                sstring, &ulong_val, &rc ) ) != NULL ) &&
                ( rc == E_OK ) ) 
	{

                strcpy( subtype, cusatt->value );
        }
	else
	{
		DEBUG_0("cfgapm: ODM error getting _subtype\n")
		ucfg_apm(lname);            
		return E_BADATTR;
	}

	sprintf(busdev, "/dev/%s", cusobj.parent) ;

	if (0 > (fd = open(busdev, O_RDWR)) )
	{
		DEBUG_0("cfgapm: Failed opening bus device\n")
		perror("[busquery]open()");
		fprintf(stderr, "Unable to open %s\n", busdev) ;
		ucfg_apm(lname);            
		return(E_DEVACCESS);
	}
	slot_num = atoi(cusobj.connwhere) - 1;
	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³ check to see if the _subtype attribute has ³
	³ already been set previously                ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	if( ipl_phase == RUNTIME_CFG )	
	{
		if( strcmp( subtype, PM_GENERIC_MSG ) != 0 )
		{
			/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
			³ since this adapter is already ³
			³ up and running, we assume the ³
			³ previous subtype is correct.  ³
			ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
			strcpy(msg_no, subtype);
			pos[0]=0x00;
			if( strcmp(msg_no, PM_4PORT_MPQP_MSG) == 0)
			{
				mpqp_4_port=1;
			}
		}
		else
		{
			/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
			³ attempt to read the adapter to determine ³
			³ adapter type and EIB attached            ³
			ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
			mddRecord.md_size = 1;          /* build mdd record */
			mddRecord.md_incr = MV_BYTE;
			mddRecord.md_data = pos;        /* addr of data */
			mddRecord.md_addr = bus_mem_addr | EIB_LOCATION;
			if (0 > ioctl(fd, MIOBUSGET, &mddRecord))
			{
				/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
				³ attempt to read EIB failed, so identify ³
				³ this card only as a generic portmaster  ³
				³ with no EIB.  This will be rechecked    ³
				³ when the POS registers have been set up ³
				ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
				DEBUG_1("cfgapm: Setting EIB to default %x\n", NOBOARD)
				pos[0]=NOBOARD;
			}

			switch (pos[0])     /* set NLS text message number based on EIB */
			{
			case PM_4PORT_MPQP:
				mpqp_4_port=1;
                		strcpy(msg_no,PM_4PORT_MPQP_MSG);   /* Portmaster with MPQP EIB */
                		break;
        		case PM_6PORT_V35:
                		strcpy(msg_no,PM_6PORT_V35_MSG);    /* Portmaster w/ 6-Port V.35 EIB */
                		break;
        		case PM_6PORT_X21:
                		strcpy(msg_no,PM_6PORT_X21_MSG);    /* Portmaster w/ 6-Port X.21 EIB */
                		break;
        		case PM_8PORT_R232:
                		strcpy(msg_no,PM_8PORT_R232_MSG);    /* Portmaster w/ 8-Port RS232 EIB */
                		break;
        		case PM_8PORT_R422:
                		strcpy(msg_no,PM_8PORT_R422_MSG);    /* Portmaster w/ 8-Port RS422 EIB */
                		break;
			default:
				pos[0]=NOBOARD;
			}
		}
		/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
		³ check to see if this adapter is the ³
		³ same type as previously identified  ³
		ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
		if( strcmp( msg_no, subtype ) != 0 )
		{
			pos[0]=NOBOARD;
		}
	}
	else
	{
		pos[0]=NOBOARD;
	}

	if(pos[0]!=NOBOARD)
	{
		close(fd);
	}
	else
	{

		/* set pos reg 2, to enable adapter  */
		mddRecord.md_size = 1;          /* build mdd record */
		mddRecord.md_incr = MV_BYTE;
		mddRecord.md_addr = POSREG(2,slot_num);

		/* pos 2 - base addr, interrupt level, enable bit */
		temp_pos[0] = (unsigned char) 0;
		temp_pos[0] = (bus_io_addr - 0x2a0) >> 6;
		switch (intr_lvl)
		{
		case 3:
			temp_pos[0] |= P2_INT3;
			break;
		case 4:
			temp_pos[0] |= P2_INT4;
			break;
		case 7:
			temp_pos[0] |= P2_INT7;
			break;
		case 9:
			temp_pos[0] |= P2_INT9;
			break;
		case 10:
			temp_pos[0] |= P2_INT10;
			break;
		case 11:
			temp_pos[0] |= P2_INT11;
			break;
		case 12:
			temp_pos[0] |= P2_INT12;
			break;
		default:
			break;                /* should never get here */
		}

		temp_pos[0] |= (P2_ENABLE | P2_SYNC_CHCK);

		mddRecord.md_data = temp_pos;        /* addr of data */
		if (0 > ioctl(fd, MIOCCPUT, &mddRecord))
		{
			perror("[busquery]ioctl()");
			fprintf(stderr, "Attempting to read slot %d\n",
			    cusobj.connwhere);
			close(fd);
			ucfg_apm(lname);            
			return(E_DEVACCESS);
		}

		/* set pos reg 3  */

		mddRecord.md_size = 1;          /* build mdd record */
		mddRecord.md_incr = MV_BYTE;
		mddRecord.md_addr = POSREG(3,slot_num);

		/* pos 3 - bits 13-20 of bus_mem_addr */
		temp_pos[0] = (unsigned char) 0;
		temp_pos[0] = (bus_mem_addr & 0x001f0000) >> 13;

		mddRecord.md_data = temp_pos;        /* addr of data */
		if (0 > ioctl(fd, MIOCCPUT, &mddRecord))
		{
			perror("[busquery]ioctl()");
			fprintf(stderr, "Attempting to read slot %d\n",
			    cusobj.connwhere);
			close(fd);
			ucfg_apm(lname);            
			return(E_DEVACCESS);
		}

		/* set pos reg 4  */

		mddRecord.md_size = 1;          /* build mdd record */
		mddRecord.md_incr = MV_BYTE;
		mddRecord.md_addr = POSREG(4,slot_num);

		/* pos 4 - addressing and window size */
		temp_pos[0] = (unsigned char) 0;
		temp_pos[0] = (bus_mem_addr & 0xc0000000) >> 27;
		temp_pos[0] |= (bus_mem_addr & 0x00e00000) >> 21;
		temp_pos[0] |= P4_WSIZ_64K;

		mddRecord.md_data = temp_pos;        /* addr of data */
		if (0 > ioctl(fd, MIOCCPUT, &mddRecord))
		{
			perror("[busquery]ioctl()");
			fprintf(stderr, "Attempting to write to slot %d\n",
			    cusobj.connwhere);
			close(fd);
			ucfg_apm(lname);            
			return(E_DEVACCESS);
		}

		/* write to I/O base registers to enable memory */

		mddRecord.md_size = 1;          /* build mdd record */
		mddRecord.md_incr = MV_BYTE;
		mddRecord.md_addr = bus_io_addr + 5;

		/* write 0 to I/O reg + 5 */
		temp_pos[0] = (unsigned char) 0;

		mddRecord.md_data = temp_pos;        /* addr of data */

		if (0 > ioctl(fd, MIOBUSPUT, &mddRecord))
		{
			perror("[busquery]ioctl()");
			fprintf(stderr, "Attempting to write to addr %x\n",
			    mddRecord.md_addr) ;
			close(fd);
			ucfg_apm(lname);            
                        return(E_DEVACCESS);
		}

		/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
		³ wait (up to 25 sec) for PROM Ready bit to come ³
		³ on before attempting to access adapter memory  ³
		ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
        	for ( c_i = 25 ; c_i; c_i-- )
        	{

                	sleep( 1 );                            /* wait 1 sec */

                	/* See if the adapter ROS indicates READY */
			mddRecord.md_size = 1;          /* build mdd record */
			mddRecord.md_incr = MV_BYTE;
			temp_pos[0] = (unsigned char) 0;
			temp_pos[0] = INITREG1;
			mddRecord.md_data = temp_pos;        /* addr of data */
			mddRecord.md_addr = bus_io_addr + PTRREG;
			if (0 > ioctl(fd, MIOBUSPUT, &mddRecord))
			{
				DEBUG_1("cfgapm: Attempting to write to addr %x\n",
					mddRecord.md_addr );
				perror("[busquery]ioctl()");
				fprintf(stderr, "Attempting write to addr %x\n",
			    		mddRecord.md_addr );
				close(fd);
				ucfg_apm(lname);            
				return(E_DEVACCESS);
			}
			DEBUG_2("cfgapm: Wrote %x to addr %x\n", INITREG1,
				mddRecord.md_addr );
			mddRecord.md_size = 1;          /* build mdd record */
			mddRecord.md_incr = MV_BYTE;
			mddRecord.md_data = pos;        /* addr of data */
			mddRecord.md_addr = bus_io_addr | DREG;
			if (0 > ioctl(fd, MIOBUSGET, &mddRecord))
			{
				DEBUG_1("cfgapm: Attempting to read addr %x\n",
					mddRecord.md_addr );
				perror("[busquery]ioctl()");
				fprintf(stderr, "Attempting to read addr %x\n",
			    		mddRecord.md_addr) ;
				close(fd);
				ucfg_apm(lname);            
				return(E_DEVACCESS);
			}
			DEBUG_2("cfgapm: Read %x at addr %x\n", pos[0],
				mddRecord.md_addr );
			if ( pos[0] & ROSREADY )
                        	break;
        	}
		if( c_i == 0 )
		{
			DEBUG_1("cfgapm: Attempting to read addr %x\n",
				mddRecord.md_addr );
			perror("[busquery]ioctl()");
			fprintf(stderr, "Attempting to read addr %x\n",
				mddRecord.md_addr) ;
			close(fd);
			ucfg_apm(lname);            
			return(E_DEVACCESS);
		}

		/* read the adapter to determine adapter type and EIB attached */

		mddRecord.md_size = 1;          /* build mdd record */
		mddRecord.md_incr = MV_BYTE;
		mddRecord.md_data = pos;        /* addr of data */
		mddRecord.md_addr = bus_mem_addr | EIB_LOCATION;

		if (0 > ioctl(fd, MIOBUSGET, &mddRecord))
		{
			perror("[busquery]ioctl()");
			fprintf(stderr, "Attempting to read addr %x\n",
			    mddRecord.md_addr) ;
			close(fd);
			ucfg_apm(lname);            
                        return(E_DEVACCESS);
		}

		DEBUG_2("cfgapm: addr = %x, value read = %x\n",
		    mddRecord.md_addr,pos[0])

		/* write pos reg 2, to disable adapter  */

		mddRecord.md_size = 1;          /* build mdd record */
		mddRecord.md_incr = MV_BYTE;
		mddRecord.md_addr = POSREG(2,slot_num);

		/* pos 2 - base addr, interrupt level, enable bit */
		temp_pos[0] = (unsigned char) 0;
		temp_pos[0] = (bus_io_addr - 0x2a0) >> 6;
		switch (intr_lvl)
		{
		case 3:
			temp_pos[0] |= P2_INT3;
			break;
		case 4:
			temp_pos[0] |= P2_INT4;
			break;
		case 7:
			temp_pos[0] |= P2_INT7;
			break;
		case 9:
			temp_pos[0] |= P2_INT9;
			break;
		case 10:
			temp_pos[0] |= P2_INT10;
			break;
		case 11:
			temp_pos[0] |= P2_INT11;
			break;
		case 12:
			temp_pos[0] |= P2_INT12;
			break;
		default:
			break;                /* should never get here */
		}

		temp_pos[0] |= (P2_SYNC_CHCK);

		mddRecord.md_data = temp_pos;        /* addr of data */
		if (0 > ioctl(fd, MIOCCPUT, &mddRecord))
		{
			perror("[busquery]ioctl()");
			fprintf(stderr, "Attempting to read slot %d\n",
			    cusobj.connwhere);
			close(fd);
			ucfg_apm(lname);            
			return(E_DEVACCESS);
		}


		/* set NLS text message number based on EIB */
		switch (pos[0])
		{
		case PM_4PORT_MPQP:
			mpqp_4_port=1;
			strcpy(msg_no,PM_4PORT_MPQP_MSG);     /* Portmaster with MPQP EIB */
			break;
		case PM_6PORT_V35:
			strcpy(msg_no,PM_6PORT_V35_MSG);    /* Portmaster w/ 6-Port V.35 EIB */
			break;
		case PM_6PORT_X21:
			strcpy(msg_no,PM_6PORT_X21_MSG);    /* Portmaster w/ 6-Port X.21 EIB */
			break;
		case PM_8PORT_R232:
			strcpy(msg_no,PM_8PORT_R232_MSG);    /* Portmaster w/ 8-Port RS232 EIB */
			break;
		case PM_8PORT_R422:
			strcpy(msg_no,PM_8PORT_R422_MSG);    /* Portmaster w/ 8-Port RS422 EIB */
			break;
		default:
			strcpy(msg_no,"10");    /* Unknown, do not change value */
		}


		DEBUG_3("cfgapm: addr = %x, value read = %x, msg_no =%s\n",
	    	mddRecord.md_addr,pos[0],msg_no)
		/*------------------------------------------------------|
	 	| get the customized attribute (_subtype) used to      |
	 	| define which type of adapter and EIB pair we have    |
	 	|------------------------------------------------------*/

		if ( (( cusatt = get_attrval( lname, "_subtype", sstring,
	    	&ulong_val, &rc ) ) != NULL )
	    	&& ( rc == E_OK ) )
		{

			/*----------------------------------------------|
		 	| we got a pointer, therefore we  have a       |
		 	| _subtype attribute.  Update the value.       |
		 	| CuAt updates always take strings as values.  |
		 	|----------------------------------------------*/
	
			strcpy( cusatt->value, msg_no );
			if ( putattr( cusatt ) < 0 )
			{
				close(fd);
				DEBUG_0("cfgapm: cannot update subtype\n")
				rc = E_ODMUPDATE;
				ucfg_apm(lname);            
				return( rc ) ;
			}
		}
		close(fd);
	}
	
	DEBUG_0("cfgapm: define children: check children\n")

	/* search CusDevices for customized object with this logical
	   name as parent */
	sprintf(sstring, "parent = '%s'", lname);
	if ((rc = (int)odm_get_obj(cusdev, sstring, &cusobj, TRUE)) == 0) {
		if(mpqp_4_port)
			return(def_mpqd(lname));
		/* odm objects not found */
		DEBUG_0("cfgapm: no objects found\n")
		return(E_OK);
	}
	else if (rc == -1)
	{       
		DEBUG_1("cfgapm: couldn't get child of %s\n",lname)
		ucfg_apm(lname);            
		return(E_ODMGET);
	}
	DEBUG_1("cfgapm: name of child %s\n",cusobj.name)
	fprintf(stdout,"%s ",cusobj.name);

	rc = odm_close_class(cusdev);
	if(rc < 0){
		/* error closing object class */
		DEBUG_0("cfgapm: close object class CuDv failed\n")
		return(E_ODMCLOSE);
	}
	return(E_OK);
}
