static char sccsid[] = "@(#)73	1.5  src/bos/usr/lib/methods/cfgampx/cfgampx.c, cfgcommo, bos411, 9439C411a 9/29/94 18:08:03";
/*
 * COMPONENT_NAME: (CFGMETH) cfgampx
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
#include "lducode.h"

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
 * NOTES: There is no DDS for the ARTIC2 adapter (DDS is built for the
 *        specific device drivers) this is then a NULL function.
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
 * NOTES: There is no minor number required for the ATRIC2 adapter,
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
 * NOTES: For the ARTIC2 adapter, there is no special character file on
 *        /dev, this function is then NULL.
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
 * FUNCTION: To download the micro code for the MP/2 or X.25 adapter.
 *
 * EXECUTION ENVIRONMENT:
 *     Operates as a device dependent subroutine called by the generic
 *     configure method for all devices.
 *
 * NOTES: For the ARTIC2 adapter, micro code is downloaded through the
 *        device driver, this function is then NULL.
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
 * FUNCTION: To query the ARTIC2 device for VPD information (Vital Product Data)
 *
 * EXECUTION ENVIRONMENT:
 *     Operates as a device dependent subroutine called by the generic
 *     configure method for all devices.
 *
 * NOTES: For the ARTIC2 adapter, VPD information is not currently supported,
 *        this function is then NULL.
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
 * NAME     : ucfg_ampx
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
 *      The articmpx adapter's state will be AVAILABLE at this time,
 *      so since there was an error detected the adapter's state should
 *      be changed to DEFINED.           
 *
 */

#define AMPX_UNIQUETYPE "uniquetype = 'adapter/mca/articmpx'"

void ucfg_ampx(lname)
char	*lname;			/* logical name of the device           */
{
	struct  PdDv   pddvadptr;
	int rc;
	char string[S_LENGTH];

	if(( rc = odm_get_obj( PdDv_CLASS, AMPX_UNIQUETYPE, &pddvadptr, ODM_FIRST)) == 0 ) {
		DEBUG_2("ucfg_ampx: get failed lname=%s rc=%d\n", lname ,rc)
		return;
	}

	sprintf( string, "-l %s ", lname );
	if(odm_run_method(pddvadptr.Unconfigure,string,NULL,NULL)){
		fprintf(stderr,"ucfg_ampx: can't run %s\n", pddvadptr.Unconfigure);
		return;
	}

	return;
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
 *      The X.25 and multiport adapter's child devices are the device drivers
 *      that may be configured to run with this adapter.  The standard
 *      device is the X.25 device driver.
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
char	*lname;			/* logical name of the device           */
int	ipl_phase;		/* ipl phase                            */
{
	struct	Class	*predev ;	/* predefined devices class ptr */
	struct	Class	*cusdev ;	/* customized devices class ptr */
	struct	PdDv	preobj ;	/* predefined device object     */
	struct	CuDv	cusobj ;	/* customized device object     */
	char	sstring[S_LENGTH];	/* search criteria              */
	int	rc;			/* return code                  */
	char	*out_p;

	/* declarations for card query */
	char	busdev[32] ;
	int	fd ;
	struct	CuAt	*cusatt;
	ulong	ulong_val;
	char	msg_no[4];	/* string to hold NLS test msg num. */
	POS_INFO	posinfo;	/* pos register information */
	ulong	intr_lvl;	/* interrupt level */
        char	subtype[4];	/* adapter EIB type */
	MACH_DD_IO	mddRecord;
	uchar	eibid[2];	/* storage for EIB value */
	uchar	port[2];	/* storage for MP/2 port value */
	uchar	pos_regs[6];	/* storage for pos register value */
	uchar	temp_io[1];	/* storage for io register value */
	int	slot_num;
	static uchar intr_lookup[13] = {0xff,0xff,0xff,0,1,0xff,0xff,2,0xff,3,4,5,6};
	uchar	level,	card_num;
	int     board_up=0;		/* board is already up */
	int	c_i;


	DEBUG_0("cfgampx: define children\n");

	/* open customized device object class */
	if ((int)(cusdev=odm_open_class(CuDv_CLASS)) == -1) {
		/* error opening class */
		DEBUG_0("cfgampx: open of CuDv failed\n");
		ucfg_ampx(lname);
		err_exit(E_ODMOPEN);
	}

	DEBUG_0("cfgampx: card query\n");

	/* Get parent device and where connected */
	sprintf(sstring,"name = '%s'",lname);
	rc =(int)odm_get_first(cusdev,sstring,&cusobj);
	if (rc==0) {
		/* failed to get an object */
		DEBUG_0("cfgampx: No device CuDv object\n");
		ucfg_ampx(lname);
		err_exit(E_NOCuDv);
	}
	else if (rc==-1) {
		/* ODM error */
		DEBUG_0("cfgampx: ODM error getting CuDv object\n");
		ucfg_ampx(lname);
		err_exit(E_ODMGET);
	}

	DEBUG_3("cfgampx: name = %s, parent = %s, connwhere = %s\n",
		lname,cusobj.parent, cusobj.connwhere)

	/* Read attribute from CuAt, and PdAt tables */
	if(( rc = getatt( &posinfo.win_base_addr, 'i', CuAt_CLASS,
		PdAt_CLASS, lname, cusobj.PdDvLn_Lvalue,
		"bus_mem_addr", (struct attr *) NULL )) > 0 )
	{
		DEBUG_0("cfgampx: ODM error getting win_base_addr\n");
		ucfg_ampx(lname);
		return E_BADATTR;
	}

	/* Read attribute from CuAt, and PdAt tables */
	if(( rc = getatt( &posinfo.baseio, 'i', CuAt_CLASS,
		PdAt_CLASS, lname, cusobj.PdDvLn_Lvalue,
		"bus_io_addr", (struct attr *) NULL )) > 0 )
	{
		DEBUG_0("cfgampx: ODM error getting baseio\n");
		ucfg_ampx(lname);
		return E_BADATTR;
	}

	/* Read attribute from CuAt, and PdAt tables */
	if(( rc = getatt( &posinfo.win_size, 'i', CuAt_CLASS,
		PdAt_CLASS, lname, cusobj.PdDvLn_Lvalue,
		"window_size", (struct attr *) NULL )) > 0 )
	{
		DEBUG_0("cfgampx: ODM error getting win_size\n");
		ucfg_ampx(lname);
		return E_BADATTR;
	}

	/* Read attribute from CuAt, and PdAt tables */
	if(( rc = getatt( &intr_lvl, 'i', CuAt_CLASS,
		PdAt_CLASS, lname, cusobj.PdDvLn_Lvalue,
		"bus_intr_lvl", (struct attr *) NULL )) > 0 ) 
	{
		DEBUG_0("cfgampx: ODM error getting intr_lvl\n");
		ucfg_ampx(lname);
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
		DEBUG_0("cfgampx: ODM error getting _subtype\n");
		ucfg_ampx(lname);            
		return E_BADATTR;
	}

	sprintf(busdev, "/dev/%s", cusobj.parent) ;

	if (0 > (fd = open(busdev, O_RDWR)) )
	{
		DEBUG_0("cfgampx: Failed opening bus device\n");
		perror("[busquery]open()");
		fprintf(stderr, "Unable to open %s\n", busdev);
		ucfg_ampx(lname);
		return(E_DEVACCESS);
	}

	slot_num = atoi(cusobj.connwhere) - 1;
	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³ check to see if the _subtype attribute has ³
	³ already been set previously                ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	if( (strcmp( subtype, MP_GENERIC_MSG ) != 0))
	{
	    /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	    ³ If This is runtime config and subtype has already been     ³
	    ³ set, then we must assume the board is configured and not   ³
	    ³ interfere with any driver that may be loaded and using the ³
	    ³ card.                                                      ³
	    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	    if (ipl_phase == RUNTIME_CFG)
	    {
		board_up = 1;		/* board is up and running */

		/* set msg_no to be what's currently in subtype */
		strcpy(msg_no, subtype);

		/* Set the EIBID from subtype string */
		if (strcmp(subtype, X25_C2X_MSG) == 0) {
			eibid[0] = X25_C2X;
		} else if (strcmp(subtype, MP2_4PORT_R232_MSG) == 0) {
			eibid[0] = MP_4O8PORT_R232;
		} else if (strcmp(subtype, MP2_8PORT_R232_MSG) == 0) {
			eibid[0] = MP_4O8PORT_R232;
		} else if (strcmp(subtype, MP_6PORT_SYNC_MSG) == 0) {
			eibid[0] = MP_6PORT_SYNC;
		} else if (strcmp(subtype, MP_8PORT_R422_MSG) == 0) {
			eibid[0] = MP_8PORT_R422;
		} else if (strcmp(subtype, MP_8PORT_R232_R422_MSG) == 0) {
			eibid[0] = MP_8PORT_R232_R422;
		} else {
			eibid[0] = NOBOARD;
		}

	    } else {
		/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
		³ attempt to read the adapter to determine ³
		³ adapter type and EIB attached            ³
		ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
		mddRecord.md_size = 1;          /* build mdd record */
		mddRecord.md_incr = MV_BYTE;
		mddRecord.md_data = eibid;        /* addr of data */
		mddRecord.md_addr = posinfo.win_base_addr | EIB_LOCATION;

		if (0 > ioctl(fd, MIOBUSGET, &mddRecord))
		{
			/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
			³ attempt to read EIB failed, so identify ³
			³ this card only as a generic articampx   ³
			³ with no EIB.  This will be rechecked    ³
			³ when the POS registers have been set up ³
			ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
			DEBUG_1("cfgampx: Setting EIB to default %x\n", NOBOARD);
			eibid[0]=NOBOARD;
		}

	    switch (eibid[0])     /* set NLS text message number based on EIB */
	    {
		case X25_C2X:
			strcpy(msg_no,X25_C2X_MSG);     /* X.25 Co-Processor/2 */
			break;
		case MP_4O8PORT_R232:
			/* read the port information */

			mddRecord.md_size = 1;          /* build mdd record */
			mddRecord.md_incr = MV_BYTE;
			mddRecord.md_data = port;        /* addr of data */
			mddRecord.md_addr = posinfo.win_base_addr | MP2_PORT_LOCATION;

			if (0 > ioctl(fd, MIOBUSGET, &mddRecord))
			{
				DEBUG_1("cfgampx: Attempting to read addr %x\n",mddRecord.md_addr );
				perror("[busquery]ioctl()");
				fprintf(stderr, "Attempting to read addr %d\n",mddRecord.md_addr);
				close(fd);
				ucfg_ampx(lname);
				return(E_DEVACCESS);
			}

			/* determine number of ports */
			switch (port[0] & MP2_PORT_MASK)
			{
				case MP2_4PORT_R232:
					strcpy(msg_no,MP2_4PORT_R232_MSG);     /* MP/2 - 4 Port RS232 EIB */
					break;
				case MP2_8PORT_R232:
					strcpy(msg_no,MP2_8PORT_R232_MSG);     /* MP/2 - 8 Port RS232 EIB */
					break;
				case MP2_6PORT_R232:        /* MP/2 - 6 Port RS232 EIB */
				default:
					eibid[0]=NOBOARD;
			}

			break;
		case MP_6PORT_SYNC:
			strcpy(msg_no,MP_6PORT_SYNC_MSG);   /* MP/2 - 6 Port SYNC EIB */
			break;
		case MP_8PORT_R422:
			strcpy(msg_no,MP_8PORT_R422_MSG);   /* MP/2 - 8 Port RS422 EIB */
			break;
		case MP_8PORT_R232_R422:
			strcpy(msg_no,MP_8PORT_R232_R422_MSG);   /* MP/2 - 8 Port RS232/422 EIB */
			break;
		default:
			eibid[0]=NOBOARD; 
	        }
		/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
		³ check to see if this adapter is the ³
		³ same type as previously identified  ³
		ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
		if( strcmp( msg_no, subtype ) != 0 )
			eibid[0]=NOBOARD;
	    }
	}
	else
	{
		eibid[0]=NOBOARD;
	}

	if(eibid[0]!=NOBOARD || board_up)
		close(fd);
	else
	{
	    /* read pos reg 0 and 1 */

	    mddRecord.md_size = 2;          /* build mdd record */
	    mddRecord.md_incr = MV_BYTE;
	    mddRecord.md_addr = POSREG(0,slot_num);

	    mddRecord.md_data = pos_regs;        /* addr of data */
	    if (0 > ioctl(fd, MIOCCGET, &mddRecord))
	    {
		perror("[busquery]ioctl()");
		fprintf(stderr, "Attempting to read slot %c\n",
		    cusobj.connwhere);
		close(fd);
		ucfg_ampx(lname);
		return(E_DEVACCESS);
	    }

	    DEBUG_2("cfgampx: devid = %4.4x, slot_num = %d\n",
		  ((pos_regs[0] << 8) | pos_regs[1]),slot_num+1)

	    /* set pos reg 2, to enable adapter  */

	    mddRecord.md_size = 1;          /* build mdd record */
	    mddRecord.md_incr = MV_BYTE;
	    mddRecord.md_addr = POSREG(2,slot_num);

	    /* pos 2 - base addr, interrupt level, enable bit */
	    pos_regs[2] = (unsigned char) 0;
	    card_num = ((posinfo.baseio - 0x2a0) / 0x400);
	    pos_regs[2] = card_num << 4;
	    level = intr_lookup[intr_lvl];
	    pos_regs[2] |= (level << 1);
	    pos_regs[2] |= 0x01;

	    mddRecord.md_data = &pos_regs[2];        /* addr of data */
	    if (0 > ioctl(fd, MIOCCPUT, &mddRecord))
	    {
		perror("[busquery]ioctl()");
		fprintf(stderr, "Attempting to read slot %c\n",
		    cusobj.connwhere);
		close(fd);
		ucfg_ampx(lname);
		return(E_DEVACCESS);
	    }

	    DEBUG_2("cfgampx: write to POS 2 = %x, slot_num = %d\n",
		  pos_regs[2],slot_num+1)

	    /* set pos reg 3  */

	    mddRecord.md_size = 1;          /* build mdd record */
	    mddRecord.md_incr = MV_BYTE;
	    mddRecord.md_addr = POSREG(3,slot_num);

	    /* pos 3 - bits 13-20 of bus_mem_addr */
	    pos_regs[3] = (unsigned char) 0;
	    pos_regs[3] = (posinfo.win_base_addr & 0x0fe000) >> 13;

	    mddRecord.md_data = &pos_regs[3];        /* addr of data */
	    if (0 > ioctl(fd, MIOCCPUT, &mddRecord))
	    {
		perror("[busquery]ioctl()");
		fprintf(stderr, "Attempting to read slot %d\n",
		    cusobj.connwhere);
		close(fd);
		ucfg_ampx(lname);
		return(E_DEVACCESS);
	    }

	    DEBUG_1("cfgampx: write to POS 3 = %x\n", pos_regs[3])

	    /* set pos reg 4  */

	    mddRecord.md_size = 1;          /* build mdd record */
	    mddRecord.md_incr = MV_BYTE;
	    mddRecord.md_addr = POSREG(4,slot_num);

	    /* pos 4 - addressing and window size */
	    pos_regs[4] = (unsigned char) 0;
	    pos_regs[4] = (posinfo.win_base_addr & 0xf00000) >> 20; 

	    mddRecord.md_data = &pos_regs[4];        /* addr of data */
	    if (0 > ioctl(fd, MIOCCPUT, &mddRecord))
	    {
		perror("[busquery]ioctl()");
		fprintf(stderr, "Attempting to read slot %d\n",
		    cusobj.connwhere);
		close(fd);
		ucfg_ampx(lname);
		return(E_DEVACCESS);
	    }
	    DEBUG_1("cfgampx: write to POS 4 = %x\n", pos_regs[4])

	    /* window size in lowest three bits , 00=8K, 01=16K, 10=32K, 11=64K */
	    mddRecord.md_size = 1;          /* build mdd record */
	    mddRecord.md_incr = MV_BYTE;
	    mddRecord.md_addr = POSREG(5,slot_num);
	    switch(posinfo.win_size)
	    {
	      case 8*1024:
		pos_regs[5]=0;
		break;
	      case 16*1024:
		pos_regs[5]=1;
		break;
	      case 32*1024:
		pos_regs[5]=2;
		break;
	      case 64*1024:
		pos_regs[5]=3;
		break;
	      default:
		pos_regs[5]=3;
		break;
	    }
	    mddRecord.md_data = &pos_regs[5];        /* addr of data */

	    if (0 > ioctl(fd, MIOCCPUT, &mddRecord))
	    {
		perror("[busquery]ioctl()");
		fprintf(stderr, "Attempting to read slot %d\n",
		    cusobj.connwhere);
		close(fd);
		ucfg_ampx(lname);
		return(E_DEVACCESS);
	    }


	    /* write to I/O base registers to enable memory */

	    mddRecord.md_size = 1;          /* build mdd record */
	    mddRecord.md_incr = MV_BYTE;
	    mddRecord.md_addr = posinfo.baseio + 5;

	    /* write 0 to I/O reg + 5 */
	    temp_io[0] = (unsigned char) 0;

	    mddRecord.md_data = temp_io;        /* addr of data */

	    if (0 > ioctl(fd, MIOBUSPUT, &mddRecord))
	    {
		perror("[busquery]ioctl()");
		fprintf(stderr, "Attempting to read addr %d\n",
		    mddRecord.md_addr) ;
		ucfg_ampx(lname);
		return(E_DEVACCESS);
	    }

	    DEBUG_2("cfgampx: write I/O REG = %x at addr = %x\n",
		   temp_io[0],mddRecord.md_addr)

		/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
		³ wait (up to 25 sec) for PROM Ready bit to come ³
		³ on before attempting to access adapter memory  ³
		ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
        	for ( c_i = 25 ; c_i; c_i-- )
        	{

                	sleep( 1 );				/* wait 1 sec */

                	/* See if the adapter ROS indicates READY */
			mddRecord.md_size = 1;			/* build mdd record */
			mddRecord.md_incr = MV_BYTE;
			temp_io[0] = (unsigned char) 0;
			temp_io[0] = INITREG1;
			mddRecord.md_data = temp_io;		/* addr of data */
			mddRecord.md_addr = posinfo.baseio + PTRREG;

			if (0 > ioctl(fd, MIOBUSPUT, &mddRecord))
			{
				DEBUG_1("cfgampx: Attempting to write to addr %x\n",
					mddRecord.md_addr );
				perror("[busquery]ioctl()");
				fprintf(stderr, "Attempting write to addr %x\n",
			    		mddRecord.md_addr );
				close(fd);
				ucfg_ampx(lname);            
				return(E_DEVACCESS);
			}
			DEBUG_2("cfgampx: Wrote %x to addr %x\n", INITREG1,
				mddRecord.md_addr );
			mddRecord.md_size = 1;          /* build mdd record */
			mddRecord.md_incr = MV_BYTE;
			mddRecord.md_data = eibid;        /* addr of data */
			mddRecord.md_addr = posinfo.baseio | DREG;
			if (0 > ioctl(fd, MIOBUSGET, &mddRecord))
			{
				DEBUG_1("cfgampx: Attempting to read addr %x\n",
					mddRecord.md_addr );
				perror("[busquery]ioctl()");
				fprintf(stderr, "Attempting to read addr %x\n",
			    		mddRecord.md_addr) ;
				close(fd);
				ucfg_ampx(lname);            
				return(E_DEVACCESS);
			}
			DEBUG_2("cfgampx: Read %x at addr %x\n", eibid[0],
				mddRecord.md_addr );
			if ( eibid[0] & ROSREADY )
                        	break;
        	}
		if( c_i == 0 )
		{
		    DEBUG_0("cfgampx: Attempting to reset\n");
		    if(rc = reset(fd, &posinfo)){
			DEBUG_1("cfgampx: Failed reset() with return code=%d\n",
				rc);
			DEBUG_1("cfgampx: Attempting to read addr %x\n",
				mddRecord.md_addr );
			perror("[busquery]ioctl()");
			fprintf(stderr, "Attempting to read addr %x\n",
				mddRecord.md_addr) ;
			close(fd);
			ucfg_ampx(lname);            
			return(E_DEVACCESS);
		    }
		}
		else
		{
		    DEBUG_0("cfgampx: Attempting to reset\n");
		    if(rc = reset(fd, &posinfo)){
			DEBUG_1("cfgampx: Failed reset() with return code=%d\n",
				rc);
			close(fd);
			ucfg_ampx(lname);
			return(E_DEVACCESS);
		    }
		}

	    /* read the adapter to determine adapter type and EIB attached */
	    /* - first load adapter with microcode to force interface   */
	    /*   block to be loaded with correct EIB values (MP/2 has a */
	    /*   problem with this at the time of this coding)          */

	    /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	     ³ This code taken and modified from geteibid.c  ³
	     ³ module received from Boca, (David Carew) for  ³
	     ³ the purpose of the loading the required ucode.³
	     ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	    /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	     ³ Load ucode so it can update Interface Block   ³
	     ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	    if ( rc = load_ucode(fd, &posinfo,"/usr/lib/asw/geteibid.com",0))
	    {
	       DEBUG_1("cfgampx: Error from load_ucode(), rc = %d\n",rc)
	       close(fd);
		ucfg_ampx(lname);
	       return(E_NOUCODE);
	    }

	    /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	     ³ Give ucode some time to update Interface Block³
	     ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	    usleep(1000);

	    mddRecord.md_size = 1;          /* build mdd record */
	    mddRecord.md_incr = MV_BYTE;
	    mddRecord.md_data = eibid;        /* addr of data */
	    mddRecord.md_addr = posinfo.win_base_addr | EIB_LOCATION;

	    if (0 > ioctl(fd, MIOBUSGET, &mddRecord))
	    {
		perror("[busquery]ioctl()");
		fprintf(stderr, "Attempting to read addr %d\n",
		    mddRecord.md_addr) ;
		close(fd);
		ucfg_ampx(lname);
		return(E_DEVACCESS);
	    }

	    DEBUG_2("cfgampx: EIB ID of card in slot %d = %2X\n",
		      slot_num+1, eibid[0])

	    switch (eibid[0])     /* set NLS text message number based on EIB */
	    {
	      case X25_C2X:
		strcpy(msg_no,X25_C2X_MSG);     /* X.25 Co-Processor/2 */
		break;
	      case MP_4O8PORT_R232:
		/* read the port information */

		mddRecord.md_size = 1;          /* build mdd record */
		mddRecord.md_incr = MV_BYTE;
		mddRecord.md_data = port;        /* addr of data */
		mddRecord.md_addr = posinfo.win_base_addr | MP2_PORT_LOCATION;

		if (0 > ioctl(fd, MIOBUSGET, &mddRecord))
		{
			perror("[busquery]ioctl()");
			fprintf(stderr, "Attempting to read addr %d\n",
				mddRecord.md_addr) ;
			close(fd);
			ucfg_ampx(lname);
			rc = -2 ;
		}

		/* determine number of ports */
		switch (port[0] & MP2_PORT_MASK)
		{
		  case MP2_4PORT_R232:
		    strcpy(msg_no,MP2_4PORT_R232_MSG);     /* MP/2 - 4 Port RS232 EIB */
		    break;
		  case MP2_8PORT_R232:
		    strcpy(msg_no,MP2_8PORT_R232_MSG);     /* MP/2 - 8 Port RS232 EIB */
		    break;
		  case MP2_6PORT_R232:        /* MP/2 - 6 Port RS232 EIB */
		  default:
		    strcpy(msg_no,"180");   /* Unknown, do not change value */
		}

		break;
	      case MP_6PORT_SYNC:
		strcpy(msg_no,MP_6PORT_SYNC_MSG);   /* MP/2 - 6 Port SYNC EIB */
		break;
	      case MP_8PORT_R422:
		strcpy(msg_no,MP_8PORT_R422_MSG);   /* MP/2 - 8 Port RS422 EIB */
		break;
	      case MP_8PORT_R232_R422:
		strcpy(msg_no,MP_8PORT_R232_R422_MSG);   /* MP/2 - 8 Port RS232/422 EIB */
		break;
	      default:
		strcpy(msg_no,"180");   /* Unknown, do not change value */
	    }

	    DEBUG_3("cfgampx: addr = %x, value read = %x, msg_no =%s\n",
		mddRecord.md_addr,eibid[0],msg_no)

	    /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	     ³ After loading and running geteibid.com        ³
	     ³ Reset card so it returns to its initial state ³
	     ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	    if (rc = reset(fd, &posinfo))
	    {
		DEBUG_1("Error from reset(), rc = %d\n",rc)
		close(fd);
		ucfg_ampx(lname);
		return(E_DEVACCESS);
	    }

	    /* set pos reg 2, to disable adapter  */

	    mddRecord.md_size = 1;          /* build mdd record */
	    mddRecord.md_incr = MV_BYTE;
	    mddRecord.md_addr = POSREG(2,slot_num);

	    /* pos 2 - base addr, interrupt level, enable bit */
	    pos_regs[2] = (unsigned char) 0;
	    pos_regs[2] = card_num << 4;
	    pos_regs[2] |= (level << 1);

	    mddRecord.md_data = &pos_regs[2];        /* addr of data */
	    if (0 > ioctl(fd, MIOCCPUT, &mddRecord))
	    {
		perror("[busquery]ioctl()");
		fprintf(stderr, "Attempting to read slot %c\n",
		    cusobj.connwhere);
		close(fd);
		ucfg_ampx(lname);
		return(E_DEVACCESS);
	    }

	    DEBUG_1("cfgampx: write to POS 2 = %x\n", pos_regs[2])

	    close(fd);

	    /*------------------------------------------------------|
	     | get the customized attribute (_subtype) used to      |
	     | define which type of adapter and EIB pair we have    |
	     |------------------------------------------------------*/

	    if ( (( cusatt = get_attrval( lname, "_subtype", sstring,
		&ulong_val, &rc ) ) != NULL )
		&&
		( rc == E_OK ) )
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
			DEBUG_0("cfgampx: cannot update subtype\n");
			ucfg_ampx(lname);
			rc = E_ODMUPDATE;
			return( rc ) ;
		}
	    }
	    close(fd);
	}

	DEBUG_0("cfgampx: define children: check children\n")

	/* search CusDevices for customized object with this logical
	   name as parent */
	sprintf(sstring, "parent = '%s'", lname);
	if ((rc = (int)odm_get_obj(cusdev, sstring, &cusobj, TRUE)) == 0) {
		/* odm objects not found */
		DEBUG_0("cfgampx: no objects found\n")
		return(E_OK);
	}
	else if (rc == -1)
	{       DEBUG_1("cfgampx: couldn't get child of %s\n",lname)
		return(E_ODMGET);
	}
	DEBUG_1("cfgampx: name of child %s\n",cusobj.name)
	fprintf(stdout,"%s ",cusobj.name);

	rc = odm_close_class(cusdev);
	if(rc < 0){
		/* error closing object class */
		DEBUG_0("cfgampx: close object class CuDv failed\n")
		return(E_ODMCLOSE);
	}
	return(E_OK);
}
