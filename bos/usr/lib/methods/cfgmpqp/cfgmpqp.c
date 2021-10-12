static char sccsid[] = "@(#)06	1.8.3.1  src/bos/usr/lib/methods/cfgmpqp/cfgmpqp.c, cfgmethods, bos411, 9434B411a 6/15/94 14:38:35";
/*
 * COMPONENT_NAME: (CFGMETH) cfgmpqp
 *
 * FUNCTIONS: build_dds, generate_minor, make_special_files
 * FUNCTIONS: download_microcode, query_vpd, define_children
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/cfgodm.h>
#include <sys/stat.h>
#include <sys/cfgdb.h>
#include <cf.h>
#include <sys/mpqpdiag.h>
#include <sys/mpqp.h>
#include <sys/mpqpdd.h>
#include <sys/limits.h>
#include "cfgdebug.h"

extern long	*genminor();

/*
 * NAME: build_dds
 *
 * FUNCTION: Builds a DDS (Defined Data Structure) describing a device's
 *           characteristics to it's device driver.
 *
 * EXECUTION ENVIRONMENT:
 *     Operates as a device dependent subroutine called by the generic
 *     configure method for all devices.
 *
 * NOTES: Values are accessed from records in the CuDv and CuAt object
 *        classes, first from the device and parent objects in the CuDv object
 *        class, then from the objects for the device in the CuAt object class.
 *        Values are edited and and placed in the DDS structure which is
 *        returned to the calling generic configure.
 *
 * RETURNS: 0 - success
 *         >0 - failure
 *
 */

int  build_dds( lognam, addr, len )
char  *lognam;				/* logical name of device */
uchar **addr;				/* receiving pointer for DDS address */
long  *len;				/* receiving variable for DDS length */
{  
	int	rc,			/* return code			    */
		result;			/* work variable		    */
	long	objtest = 0;		/* existance test, 1-test, 0-don't  */
	char	sstring[512];		/* working string		    */
	struct	CuDv	cudvport,	/* object record for port           */
                        cudvdriv,       /* object record for parent dev drvr */
			cudvadap,	/* object record for parent of port */
			cudvbus;	/* object record for parent of adap */
	t_mpqp_dds	*dds;		/* pointer to DDS structure	    */
	ulong		bus_num;	/* bus number			    */

	dds = (t_mpqp_dds *) malloc( sizeof(t_mpqp_dds) );
	if (dds == NULL)
		return(E_MALLOC);	/* report allocation error */

	/* Driver requires dds be cleared: */
	memset( dds, 0, sizeof(t_mpqp_dds) );

	/* get the object record for the port */
	sprintf( sstring, "name = '%s'", lognam );
	if(( rc = odm_get_obj( CuDv_CLASS, sstring, &cudvport, ODM_FIRST ))==0)
		return( E_NOCuDv );
	else if ( rc == -1 )
		return( E_ODMGET );

	/* fill in resource name fields */
	strcpy(dds->dds_vpd.devname,lognam);


	/* scan field to extract port number */
	if (sscanf( cudvport.connwhere, "%d", &result ) != 1 )
		return(E_INVCONNECT);

	dds->dds_dvc.port_num = result;

	DEBUG_1("Port Number is %d\n", dds->dds_dvc.port_num )

        /* read the parent device driver object */
        sprintf( sstring, "name = '%s'", cudvport.parent );
        if(( rc = odm_get_obj( CuDv_CLASS, sstring, &cudvdriv, ODM_FIRST ))==0)
                return( E_NOCuDv );
        else if ( rc == -1 )
                return( E_ODMGET );
 
        /* fill in resource name fields */
        strcpy(dds->dds_vpd.adpt_name,cudvdriv.parent);

	/* read the parent adapter object */
	sprintf( sstring, "name = '%s'", cudvdriv.parent );
	if(( rc = odm_get_obj( CuDv_CLASS, sstring, &cudvadap, ODM_FIRST ))==0)
		return( E_NOCuDv );
	else if ( rc == -1 )
		return( E_ODMGET );

	/* scan field to extract slot number */
	if (sscanf( cudvadap.connwhere, "%d", &dds->dds_hdw.slot_num ) != 1 )
		return(E_INVCONNECT);
	dds->dds_hdw.slot_num--; /* connwhere = 1,2,.. slot_num = 0,1,.. */

	DEBUG_1( "Slot number is %d after decrement\n", dds->dds_hdw.slot_num )

	/* get the object record for the parent of the adapter */
	sprintf( sstring, "name = '%s'", cudvadap.parent );
	if(( rc = odm_get_obj( CuDv_CLASS, sstring, &cudvbus, ODM_FIRST ))==0)
		return( E_NOCuDv );
	else if ( rc == -1 )
		return( E_ODMGET );
	
	/* Read attributes from CuAt, and PdAt tables */
	if(( rc = getatt( &dds->dds_hdw.bus_mem_addr, 'i', CuAt_CLASS,
		PdAt_CLASS, cudvdriv.parent, cudvadap.PdDvLn_Lvalue,
		"bus_mem_addr", (struct attr *) NULL )) > 0 ) return rc;
	if(( rc = getatt( &dds->dds_hdw.tcw_bus_mem_addr, 'i', CuAt_CLASS,
		PdAt_CLASS, cudvdriv.parent, cudvadap.PdDvLn_Lvalue,
		"dma_bus_mem", (struct attr *) NULL )) > 0 ) return rc;
	if(( rc = getatt( &dds->dds_hdw.bus_intr_lvl, 'i', CuAt_CLASS,
		PdAt_CLASS, cudvdriv.parent, cudvadap.PdDvLn_Lvalue,
		"bus_intr_lvl", (struct attr *) NULL )) > 0 ) return rc;
	if(( rc = getatt( &dds->dds_hdw.bus_io_addr, 'i', CuAt_CLASS,
		PdAt_CLASS, cudvdriv.parent, cudvadap.PdDvLn_Lvalue,
		"bus_io_addr", (struct attr *) NULL )) > 0 ) return rc;
	if(( rc = getatt( &dds->dds_hdw.dma_lvl, 'h', CuAt_CLASS,
		PdAt_CLASS, cudvdriv.parent, cudvadap.PdDvLn_Lvalue,
		"dma_lvl", (struct attr *) NULL )) > 0 ) return rc;
	if(( rc = getatt( &dds->dds_dvc.rdto, 'h', CuAt_CLASS,
		PdAt_CLASS, cudvport.name, cudvport.PdDvLn_Lvalue,
		"rdto", (struct attr *) NULL )) > 0 ) return rc;
	if(( rc = getatt( &dds->dds_dvc.max_rx_bsiz, 'h', CuAt_CLASS,
		PdAt_CLASS, cudvport.name, cudvport.PdDvLn_Lvalue,
		"max_rx_bsiz", (struct attr *) NULL )) > 0 ) return rc;
	if(( rc = getatt( &dds->dds_ras.err_thresh.tx_err_thresh, 'l', 
		CuAt_CLASS, PdAt_CLASS, cudvport.name, cudvport.PdDvLn_Lvalue,
		"tx_err_thresh", (struct attr *) NULL )) > 0 ) return rc;
	if(( rc = getatt( &dds->dds_ras.err_thresh.rx_err_thresh, 'l', 
		CuAt_CLASS, PdAt_CLASS, cudvport.name, cudvport.PdDvLn_Lvalue,
		"rx_err_thresh", (struct attr *) NULL )) > 0 ) return rc;
	if(( rc = getatt( &dds->dds_ras.err_thresh.tx_err_percent, 'l', 
		CuAt_CLASS, PdAt_CLASS, cudvport.name, cudvport.PdDvLn_Lvalue,
		"tx_err_percent", (struct attr *) NULL )) > 0 ) return rc;
	if(( rc = getatt( &dds->dds_ras.err_thresh.rx_err_percent, 'l', 
		CuAt_CLASS, PdAt_CLASS, cudvport.name, cudvport.PdDvLn_Lvalue,
		"rx_err_percent", (struct attr *) NULL )) > 0 ) return rc;
	if(( rc = getatt( &dds->dds_ras.err_thresh.tx_underrun_thresh, 'l', 
		CuAt_CLASS, PdAt_CLASS, cudvport.name, cudvport.PdDvLn_Lvalue,
		"tx_urun_thresh", (struct attr *) NULL )) > 0 ) return rc;
	if(( rc = getatt( &dds->dds_ras.err_thresh.tx_cts_drop_thresh, 'l', 
		CuAt_CLASS, PdAt_CLASS, cudvport.name, cudvport.PdDvLn_Lvalue,
		"tx_ctsdp_thresh", (struct attr *) NULL )) > 0 ) return rc;
	if(( rc = getatt( &dds->dds_ras.err_thresh.tx_cts_timeout_thresh, 'l', 
		CuAt_CLASS, PdAt_CLASS, cudvport.name, cudvport.PdDvLn_Lvalue,
		"tx_ctsto_thresh", (struct attr *) NULL )) > 0 ) return rc;
	if(( rc = getatt( &dds->dds_ras.err_thresh.tx_fs_timeout_thresh, 'l', 
		CuAt_CLASS, PdAt_CLASS, cudvport.name, cudvport.PdDvLn_Lvalue,
		"tx_fsto_thresh", (struct attr *) NULL )) > 0 ) return rc;
	if(( rc = getatt( &dds->dds_ras.err_thresh.rx_overrun_err_thresh, 'l', 
		CuAt_CLASS, PdAt_CLASS, cudvport.name, cudvport.PdDvLn_Lvalue,
		"rx_orun_thresh", (struct attr *) NULL )) > 0 ) return rc;
	if(( rc = getatt( &dds->dds_ras.err_thresh.rx_abort_err_thresh, 'l', 
		CuAt_CLASS, PdAt_CLASS, cudvport.name, cudvport.PdDvLn_Lvalue,
		"rx_abort_thresh", (struct attr *) NULL )) > 0 ) return rc;
	if(( rc = getatt( &dds->dds_ras.err_thresh.rx_frame_err_thresh, 'l', 
		CuAt_CLASS, PdAt_CLASS, cudvport.name, cudvport.PdDvLn_Lvalue,
		"rx_frame_thresh", (struct attr *) NULL )) > 0 ) return rc;
	if(( rc = getatt( &dds->dds_ras.err_thresh.rx_par_err_thresh, 'l', 
		CuAt_CLASS, PdAt_CLASS, cudvport.name, cudvport.PdDvLn_Lvalue,
		"rx_par_thresh", (struct attr *) NULL )) > 0 ) return rc;
	if(( rc = getatt( &dds->dds_ras.err_thresh.rx_dma_bfr_err_thresh, 'l', 
		CuAt_CLASS, PdAt_CLASS, cudvport.name, cudvport.PdDvLn_Lvalue,
		"rx_dma_thresh", (struct attr *) NULL )) > 0 ) return rc;
	if(( rc = getatt( &dds->dds_hdw.intr_priority, 'h', CuAt_CLASS,
		PdAt_CLASS, cudvdriv.parent, cudvadap.PdDvLn_Lvalue,
		"intr_priority", (struct attr *) NULL )) > 0 ) return rc;
	if(( rc = getatt( &dds->dds_hdw.bus_id, 'i', CuAt_CLASS,
		PdAt_CLASS, cudvadap.parent, cudvbus.PdDvLn_Lvalue,
		"bus_id", (struct attr *) NULL )) > 0 ) return rc;

	bus_num = (((dds->dds_hdw.bus_id >> 20) & 0xff) - 0x20);
        dds->dds_hdw.slot_num += bus_num * NUM_SLOTS;

	*addr = (caddr_t)(dds);		/* output the DDS address and length */
	*len  = sizeof(t_mpqp_dds);

	dump_dds( dds, sizeof(t_mpqp_dds) );

	return(0);
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
 * NOTES: Simply calls the generic generate minor number routine.
 *
 * RETURNS: 0 - success
 *         >0 - failure
 *
 */

int	generate_minor( lognam, majorno, minordest )
char	*lognam;
long	majorno;
long	*minordest;
{
	long   *rc;

	rc = genminor( lognam, majorno, -1, 1, 1, 1 );
	if( (int)rc == NULL )
		return E_MINORNO;

	*minordest = *rc;

	return 0;
}

/*
 *
 * NAME: make_special_file
 *
 * FUNCTION: To create the special character file in /dev for the current
 *           device instance.
 *
 * EXECUTION ENVIRONMENT:
 *
 *     Operates as a device dependent subroutine called by the generic
 *     configure method for all devices.
 *
 * NOTES: Simply calls the generic create special file routine.
 *
 * RETURNS: 0 - success
 *         >0 - failure
 *
 */

int make_special_files( lognam, devno )

char	*lognam;
dev_t	devno;

{
	long   typemode;		/* type and mode of created file */
	long   rc;			/* return code */



	typemode = S_IRUSR | S_IFCHR	/* set type for char special file */
		| S_IRGRP | S_IROTH	/* set mode for rw------- permisions */
		| S_IWUSR | S_IFMPX
		| S_IWGRP | S_IWOTH;

	/* call the general routine */

	rc = mk_sp_file( devno, lognam, typemode );

	return(rc);
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
 * NOTES: If this is the FIRST port configured on an adapter, the microcode
 *        file named by the "microcode_file" attribute of the CuAt object
 *        class will be opened, read into a buffer and then ioctl'd down to
 *        the adapter through the current port.
 *
 * RETURNS: 0 - success
 *         >0 - failure
 *
 */

int	download_microcode( lognam )
char	*lognam;
{
#ifdef CFGDEBUG
	extern int errno, sys_nerr;
	extern char *sys_errlist[];
#endif
	char	port_fn[PATH_MAX];	/* port file name */
	char	*mcode_fn;		/* microcode file name */
	char	sstring[512];		/* working string */
	uchar	*buffer;		/* microcode buffer */
	int	rc;			/* return code */
	long	length;			/* microcode length */
	long	objtest = 0;		/* existance test, 1-test, 0-don't */
	int	port_fd = 0,		/* port file descriptor */
		mcode_fd = 0;		/* uCode file descriptor */
	struct	CuDv	cudvport,	/* CuDv record for port */
			cudvsib;	/* CuDv record for adapter */
	struct	CuAt	*cuatptr;
	t_rw_cmd	iocb;		/* communications block for ioctl */
	int		how_many;

	/* read CuDv record for current port */
	sprintf( sstring, "name = '%s'", lognam );
	if(( rc = odm_get_obj( CuDv_CLASS, sstring, &cudvport, ODM_FIRST ))==0)
		return( E_NOCuDv );
	else if ( rc == -1 )
		return( E_ODMGET );
	/* if 1 or more sibling ports exist, then the microcode for the */
	/* adapter has already been loaded */
	sprintf( sstring, "parent = '%s' AND status = %d", cudvport.parent,
		AVAILABLE );
	if(( rc = odm_get_obj( CuDv_CLASS, sstring, &cudvsib, ODM_FIRST ))==-1)
		return( E_ODMGET );
	else if ( rc != 0 )
		return 0;
	objtest = 0;

	cuatptr = getattr( cudvport.parent, "ucode", FALSE, &how_many );

	if( cuatptr == ( struct CuAt * )NULL )
		return E_NOATTR;
	if(( mcode_fd = open( cuatptr->value, O_RDONLY ) ) == -1 )
		return E_NOUCODE;
	if((length = lseek( mcode_fd, 0L, 2 ))==-1)
		return E_NOUCODE;

	DEBUG_1("Microcode length is %d\n", length )
	if(lseek( mcode_fd, 0L, 0 )==-1)
		return E_NOUCODE;
	if((buffer = malloc( length ))==NULL)
		return E_MALLOC;
	if(read( mcode_fd, buffer, length )==-1)
		return E_NOUCODE;
	if(close( mcode_fd )==-1)
		return E_NOUCODE;
	sprintf( port_fn, "/dev/%s", lognam );
	DEBUG_1("OPENING %s\n", port_fn )
	if((port_fd = open( port_fn, O_RDWR )) == -1 )
	{
		DEBUG_0("OPEN FAILED\n")
	 	return E_DEVACCESS;
	}
	iocb.length  = length;		/* set control block for ioctl */
	iocb.mem_off = 0x10100;
	iocb.usr_buf = buffer;
	/* call ioctl to download micro code */
	if( ioctl( port_fd, Q_RASW, &iocb ) <0 )
		return E_UCODE;
	free( buffer );			/* free the buffer area */
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
 * NOTES: For the MPQP adapter, VPD information is not currently supported,
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
	return(0);
}

/*
 *
 * NAME: define_children
 *
 * FUNCTION: To manage the children of the device.
 *
 * EXECUTION ENVIRONMENT:
 *     Operates as a device dependent subroutine called by the generic
 *     configure method for all devices.
 *
 * NOTES: The MPQP ports do not have children, this function is then NULL.
 *
 * RETURNS: 0 - success, in all cases
 *
 */

int  define_children( lognam, iplphs )

char  *lognam;
int    iplphs;

{
	return(0);
}

