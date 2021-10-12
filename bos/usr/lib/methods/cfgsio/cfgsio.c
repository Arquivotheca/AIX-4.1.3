static char sccsid[] = "@(#)63        1.9.1.11  src/bos/usr/lib/methods/cfgsio/cfgsio.c, cfgmethods, bos411, 9428A410j 5/20/94 10:38:16";
/*
 *   COMPONENT_NAME: CFGMETHODS
 *
 *   FUNCTIONS: build_dds
 *		dc_parse_attr
 *		dc_parse_vpd
 *		dc_use_attr
 *		dc_use_mca
 *		dc_use_vpd
 *		define_child
 *		define_children
 *		download_microcode
 *		generate_minor
 *		make_special_files
 *		query_vpd
 *		read_pos_id
 *		get_iplcb
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * Include files needed for this module follow
 */
#include <sys/types.h>
#include <stdio.h>
#include <sys/cfgdb.h>
#include <sys/cfgodm.h>
#include <sys/sysconfig.h>
#include <sys/systemcfg.h>
#include <sys/device.h>
#include <sys/iplcb.h>
#include <sys/rosinfo.h>
#include <cf.h>
#include <sys/mdio.h>
#include <string.h>
#include "cfgdebug.h"


#define GOT_CHILD  	-1
#define	DEVPKG_PREFIX	"devices"		/* device package prefix */

/* 
 *------------------------------------------------------------
 * Forward definitions for local functions
 *------------------------------------------------------------
 */
static int dc_parse_attr(char*, char**, char**, char**) ;
static int dc_use_attr(struct CuDv*) ;
static int dc_use_mca(struct CuDv*)  ;
static int define_child(char*, char*, struct PdDv*) ;
static ushort read_pos_id(char*, int) ;
static ushort get_iplcb(long*) ;

/* 
 *-------------------------------------------------------------
 * The following strings are used during children definition.
 * That are at module level so that multiple routines can
 * access them.
 *-------------------------------------------------------------
 */
char 		sepchars[]      = " "  ;


/*
 * NAME: generate_minor
 *
 * FUNCTION: Device dependant routine for generating the device minor number
 *
 * EXECUTION ENVIRONMENT:
 *  This routine is called from the device independent configuration
 *  method.
 *
 * RETURNS:
 *   This is a NULL function, so always returns 0.
 */
generate_minor(logical_name,majnum,minnum)
	char	*logical_name;
	long    majnum;
	long    *minnum;
{
	return(0);
}


/*
 * NAME: build_dds
 *
 * FUNCTION:
 *   Device dependent routine for building the dds structure.
 *
 * EXECUTION ENVIRONMENT:
 *   This routine is called from the device independent configuration
 *   method.
 *
 * RETURNS:
 *   This is a NULL function, so always returns 0.
 *
 */
build_dds(logical_name,ddsptr,ddslen)
	char *logical_name;
	char **ddsptr;
	int *ddslen;
{
	return(0);
}

/*
 * NAME: make_special_files
 *
 * FUNCTION: Device dependant routine creating the devices special files
 *   in /dev
 *
 * EXECUTION ENVIRONMENT:
 *  This routine is called from the device independent configuration
 *  method.
 *
 * RETURNS:
 *   This is a NULL function, so always returns 0.
 */
int
make_special_files(logical_name,devno)
	char	*logical_name;
	dev_t	devno;
{
	return(0);
}


/*
 * NAME: download_microcode
 *
 * FUNCTION: Device dependant routine for downloading microcode
 *
 * EXECUTION ENVIRONMENT:
 *  This routine is called from the device independent configuration
 *  method.
 *
 * NOTES: This is a NULL function for this device, so success is always
 *        returned.
 *
 * RETURNS:
 *   0 - success
 */
int
download_microcode(logical_name)
	char	*logical_name;
{
	return(0);
}


/*
 * NAME: query_vpd
 *
 * FUNCTION: Device dependent routine for obtaining VPD data for a device
 *
 * EXECUTION ENVIRONMENT:
 *  This routine is called from the device independent configuration
 *  method.
 *
 * NOTES: The resultant vpd is filtered through put_vpd
 *
 * RETURNS:
 *   0 - success
 *   >0 = errno
 */
#define  SIO_KEY  "SIO"                 /* keyword to search in VPD */
int
query_vpd(newobj,kmid,devno,vpd)
        struct  CuDv    *newobj;
        mid_t           kmid;
        dev_t           devno;
        char            *vpd;
{
        int     rc;
        MACH_DD_IO
                write_record;
        char    sstring[30];
        char    key[6];
        uchar   tmpbyte;
        int     fd;             
        int     len;



        /* if PowerPC, there may be enet AVAILABLE */
        if (__power_pc() ) {

                /* set up for fixing ienet prob */
                sprintf( sstring, "/dev/%s", newobj->parent );

                if( (fd = open( sstring, O_RDWR )) < 0 )
                        return E_OPEN;

                write_record.md_size = 1;       /* write 1 byte */
                write_record.md_incr = MV_BYTE;
                write_record.md_data = &tmpbyte;
                write_record.md_addr = POSREG(6,0x0f);
                tmpbyte = (uchar)0x0;
                /* write to 0 pos reg 6 */
                if( ioctl( fd, MIOCCPUT, &write_record ) < 0 ) {
                        close(fd) ;
                        return E_DEVACCESS;
                }
                close(fd) ;
        }

	/* determine key word to search for in VPD buffer */
	if ( !strcmp(newobj->PdDvLn,"adapter/mca/sio") )
		/* type SIO models */
		strcpy(key,"ALL");
	else
		/* all other models */
		strcpy(key,"SIO");

        rc = get_vpd (vpd, key) ;
        if (rc)
                return(E_VPD);

        return(0);
}


/*
 * name: define_children
 *
 * function: routine for detecting and managing children of a logical
 *    device.  for this device, the adapters on the standard i/o card
 *    must be defined if they do not already exist in the customized
 *    database.  the names of all defined children adapters in the
 *    customized database are then written to stdout so they can be
 *    configured.
 *
 * execution environment:
 *  this routine is called from the device independent configuration
 *  method, cfgdevice.
 *
 * returns:
 *   0 - success (E_OK)
 *   E_ODMGET    - get of an object from ODM failed.
 *   E_FINDCHILD - the define method for a child returned in error.
 *   E_ODMUPDATE - update of an object in ODM failed.
 */
 
int allpkg = 0;				

int
define_children(logical_name,phase)
	char	*logical_name;
	int	phase;
{
    struct CuDv  cusobj ;
 
    char	 sstring[128];
    int 	 rc	 ;

    /* begin define_children */

    DEBUG_0("cfgsio: entering define_children()\n");

    if( ! strcmp(getenv("DEV_PKGNAME"),"ALL") )
                allpkg = 1;

    sprintf(sstring, "name='%s'", logical_name) ;
    rc = (int)odm_get_first(CuDv_CLASS, sstring, &cusobj) ;
    if(rc == -1)
    {
	DEBUG_0("cfgsio: error during ODM get") 
	return(E_ODMGET) ;
    }
    else if (rc == 0)
    {
	DEBUG_1("cfgsio: could not find device. search string is = \n%s\n",
	       sstring) ;
	return(E_ODMGET) ;
    }

   /*
    *---------------------------------------------------------
    * got the customized object for this device, use the 
    * "children" attribute.
    *---------------------------------------------------------
    */
    rc = dc_use_attr(&cusobj) ;

    if (rc == E_OK)
    {
       /*
	*------------------------------------------------------------
	* Define any integrated adapters.
	*------------------------------------------------------------
	*/
	rc =  dc_use_mca(&cusobj) ;
    }

    return(rc);
} /* END define_children */


/*
 * NAME: dc_parse_attr
 *
 * FUNCTION: This routine parses the "children" attribute.
 *    It returns addresses of each part of the triplet.
 *
 * EXECUTION ENVIRONMENT:
 *  This routine is called from within this module ONLY.
 *
 * RETURNS:
 *   0  - success (E_OK)
 *   -1 - List exhausted (this is also an OK return code).
 *   E_FINDCHILD - "children" attribute is not complete.
 */
static
int
dc_parse_attr(char  *children,
	      char  **child  ,
	      char  **cwhere ,
	      char  **loc) 
{
    int		rc ;

/* BEGIN dc_parse_attr */

    *child  = NULL ;
    *cwhere = NULL ;
    *loc    = NULL ;
    rc = E_FINDCHILD ;

   /*
    *--------------------------------------------------------------- 
    * The list of children are kept in the values field of the 
    * "children" attribute.  The values field contains a space
    * seperated list of children.  It takes 3 tokens to define
    * each child.  They are
    *    uniquetype -> identifies the child
    *    connwhere  -> identifies where child can connect
    *    location   -> specifies what location code is to
    *    	       be used for this child.
    * This triplet exists for each child and is in the order listed.
    *----------------------------------------------------------------
    */

    if ((*child = strtok(children, sepchars)) != NULL)
    {
	if ((*cwhere = strtok((char *)0, sepchars)) != NULL)
	{
	    if ((*loc = strtok((char *)0, sepchars)) != NULL)
	    {
		DEBUG_0("cfgsio.dc_parse_attr: triplet complete=>\n") ;
		DEBUG_3("\tchild = %s\n\tconnwhere = %s\n\tloc = %s\n",
			*child, *cwhere, *loc) ;

		rc = GOT_CHILD ;
	    }
	}
    }
    else /* No tokens left in the string */
    {
       /* 
	*-------------------------------------------------------
	* This condition means that the list of children is
	* now empty.  It is assumed that this means the list
	* has been completely processed and therefore a good
	* return code is returned.
	*-------------------------------------------------------
	*/
	rc = E_OK ;
    }

#ifdef CFGDEBUG
    if (rc == E_FINDCHILD)
    {
	DEBUG_1(
		"cfgsio.dc_parse_attr: Definition incomplete,list=\n%s\n", 
		children) ;
	DEBUG_0("   current triplet =>\n") ;
	DEBUG_3("\tchild = %s\n\tconnwhere = %s\n\tloc = %s\n",
		*child, *cwhere, *loc) ;
    }
#endif

    return(rc) ;
} /* END dc_parse_attr */


/*
 * NAME: dc_use_attr
 *
 * FUNCTION: This routine defines children of SIO based on the
 *    "children" attribute.
 *
 * EXECUTION ENVIRONMENT:
 *  This routine is called from within this module ONLY.
 *
 * RETURNS:
 *   0 - success (E_OK)
 *   E_ODMGET    - Get of an object from ODM failed.
 *   E_FINDCHILD - "children" attribute is not complete.
 */
static
int
dc_use_attr(struct CuDv	*cusobj)
{
#define	LOWENTRY 0x2010043

    struct PdDv     preobj  ;
    struct PdAt	    pdat    ;
 
    char	    sstring[256];
    char	    *Child  ;		/* UniqueType of child		*/
    char	    *Cwhere ;		/* Connwhere of child		*/
    char	    *Cloc   ;		/* Location code used by child. */
    long	    rack    ;           /* used to determine children   */
    int 	    rc	    ;
    char	    sio_type;           /* used to speed check of sio type*/
    char	    defchild;           /* should define child or not   */
    long            modelcode;          /* detailed mach model code     */


/* BEGIN dc_use_attr */

   /*
    *---------------------------------------------------------------- 
    * Set variables to control defining children.  The sio POSID is
    * the same in many cases, but the set of children varies.
    * sio_type indicates the type of sio begin configured (sio, sio_1
    * or sio_2).  model_code indicates the specific machine model
    * when sio_type is sio_2 - otherwise it is not used.  rack is
    * used to indicate whether machine is a rack system or not.
    *---------------------------------------------------------------- 
    */
    if (strstr(cusobj->PdDvLn_Lvalue, "sio_1") != NULL)
    {
	sio_type = (char)1 ;
    }
    else if (strstr(cusobj->PdDvLn_Lvalue, "sio_2") != NULL)
    {
	sio_type = (char)2 ;

        if ( ( rc = get_iplcb( &modelcode ) ) < 0 ) {
                DEBUG_0("Unable to obtain model code from /dev/nvram")
                return(rc);
        }

	DEBUG_1("dc_use_attr: modelcode = 0x%x\n", modelcode) ;
    }
    else
    {
	sio_type = (char)0 ;
    }

    if ((rc = getatt(&rack, 'l', CuAt_CLASS, PdAt_CLASS, "sys0", "sys/node/sys",
	       "modelcode", NULL)))
	return rc;
    rack = ((rack & 0x03) == 2) ;

    DEBUG_2("cfgsio.dc_use_attr: sio_type = %02x; rack = %02x\n",sio_type,rack)

   /*
    *-------------------------------------------------------------------
    * Get the attribute that defines the children for this SIO adapter.
    * An attribute is used so that the list of children can change
    * without causing requiring a code change.
    *-------------------------------------------------------------------
    */
    sprintf(sstring, "uniquetype='%s' AND attribute='children'", 
	    cusobj->PdDvLn_Lvalue) ;
    rc = (int)odm_get_first(PdAt_CLASS, sstring, &pdat) ;
    if(rc == -1)
    {
	DEBUG_0("cfgsio.dc_use_attr: Error during ODM get\n") 
	return(E_ODMGET) ;
    }
    else if (rc == 0)
    {
	DEBUG_1(
   "cfgsio.dc_use_attr: could not find attribute. Search string is = \n%s\n",
	       sstring) 
	return(E_ODMGET) ;
    }
    DEBUG_1("cfgsio.dc_use_attr: Got attribute.  Values field = \n%s\n",
	   pdat.values) ;

   /* 
    *----------------------------------------------------------------
    * Get the first child in the list and then loop through the list
    * defining each child as it is parsed out of the list
    *----------------------------------------------------------------
    */
    rc = dc_parse_attr(pdat.values, &Child, &Cwhere, &Cloc) ;
    while (rc == GOT_CHILD)
    {
	defchild = (char)TRUE ;
	if ((sio_type == 1) && (rack))
	{
	   /* 
	    *--------------------------------------------------------
	    * Skip children that don't exist on sio_1 planars that
	    * are in release 2 rack systems.
	    *--------------------------------------------------------
	    */
	    if ((strcmp(Child, "adapter/sio/keyboard") == 0) ||
		(strcmp(Child, "adapter/sio/tablet")   == 0) ||
		(strcmp(Child, "adapter/sio/mouse")    == 0))
	    {
		DEBUG_0("cfgsio.dc_use_attr: child not being defined!\n") ;
		defchild = (char)FALSE ;
		rc = E_OK ;
	    }
	}
	else if ((sio_type == 2) && (modelcode == LOWENTRY))
	{
	   /* 
	    *--------------------------------------------------------
	    * Skip children that don't exist on sio_2 planars that
	    * are in Low entry level systems.
	    *--------------------------------------------------------
	    */
	    if (strcmp(Child, "adapter/sio/fda_2") == 0)
	    {
		DEBUG_0("cfgsio.dc_use_attr: child not being defined!\n") ;
		defchild = (char)FALSE ;
		rc = E_OK ;
	    }
	}

	if (defchild)              	    /* define child only if told to */
	{
	    DEBUG_0("cfgsio.dc_use_attr: Defining child\n") 
	    
	    sprintf(sstring,"uniquetype = '%s'", Child);
	    rc=(int)odm_get_first(PdDv_CLASS, sstring, &preobj);

	    if ((rc == 0) || allpkg) 
	    {
	        /* Could not get predefined */
	        DEBUG_1("Allpkg set and/or No predefined device object for: %s\n",
		        sstring);
	        /* 
	         * No predefined OR allpkg is set.
	         *    Spit out package name and skip to
	         *    next child by not setting defchild.
	         */
		
		/* Because of the defchild flag, we will
		 *  	only come here if we do indeed
		 *	want to define this child.  Look
		 *	at the uniquetype and spit out the
		 *	correct package name.
		 */

		if ((strncmp(Child, "adapter/sio/keyboard", strlen("adapter/sio/keyboard")) == 0) ||
			(strncmp(Child, "adapter/sio/tablet", strlen("adapter/sio/tablet"))   == 0) ||
			(strncmp(Child, "adapter/sio/mouse", strlen("adapter/sio/mouse"))    == 0))
			fprintf(stdout, ":%s.sio.ktma ", DEVPKG_PREFIX);

		else if ((strncmp(Child, "adapter/sio/s1a", strlen("adapter/sio/s1a")) == 0) ||
			(strncmp(Child, "adapter/sio/s2a", strlen("adapter/sio/s2a"))   == 0))
			fprintf(stdout, ":%s.sio.sa ", DEVPKG_PREFIX);

		else if (strncmp(Child, "adapter/sio/fda", strlen("adapter/sio/fda")) == 0)
			fprintf(stdout, ":%s.sio.fda ", DEVPKG_PREFIX);

		else if (strncmp(Child, "adapter/sio/ppa", strlen("adapter/sio/ppa")) == 0)
			fprintf(stdout, ":%s.sio.ppa ", DEVPKG_PREFIX);

	    }

	    if (rc == -1) 
	    {
	        /* ODM failure */
	        DEBUG_0("cfgsio: ODM error getting PdDv object\n")
	        rc = E_ODMGET ;
	    }
	    else if (rc != 0) /* got Predefined obj back. */
	    {
	       /* 
		*-----------------------------------------------
		* Got a child's PdDv object, so define it.
		*-----------------------------------------------
		*/
		rc = define_child(cusobj->name, Cwhere, &preobj) ;
	    }
	}
       /* 
	*--------------------------------------------------
	* Get next child in attribute array 
	*--------------------------------------------------
	*/
        rc = dc_parse_attr((char *)0, &Child, &Cwhere, &Cloc) ;
    } /* end WHILE Loop */

    return(rc) ;
} /* END dc_use_attr */



/*
 * NAME: dc_use_mca
 *
 * FUNCTION: This routine handles the detection and definition
 *    of SIO children that are directly attached to the mca bus.
 *    These children are the set of integrated adapters.
 *
 * EXECUTION ENVIRONMENT:
 *  This routine is called from within this module ONLY.
 *
 * RETURNS:
 *   E_OK        - Successfully defined child.
 *   E_ODMGET    - ODM error occurred.
 *   E_FINDCHILD - "children" attribute is not complete.
 */
static
int
dc_use_mca(struct CuDv *cusobj) 
{
    struct PdDv pddv	 ;

    ushort	pos_id   ;
    int		slot	 ;
    int		rc       ;
    char	sstr[64] ;
    char	cwhere[4];

#define BASE_INT_SLOT 8
#define MAX_INT_SLOT 14

/* BEGIN dc_use_mca  */

    rc = E_OK ;

   /* 
    *-----------------------------------------------------------------------
    * Loop through all slots that could have integrated adapters in them.
    * These slots begin at internal slot 8 and goes to internal slot 
    * 14 (don't look at slot 15 - that is SIO's slot).
    * Abort the loop on ODM errors or define failures.
    *-----------------------------------------------------------------------
    */
    for (slot = BASE_INT_SLOT; (slot <= MAX_INT_SLOT) && (rc == E_OK); slot++)
    {
	pos_id = read_pos_id(cusobj->parent, slot);
	if (pos_id != 0xffff  &&  pos_id != 0 )
	{
	    sprintf(sstr, "subclass=sio and devid=0x%04x", pos_id) ;
	    rc = (int)odm_get_first(PdDv_CLASS, sstr, &pddv) ;
	    if (rc == 0)
	    {
		/* can't find it, print pkg name */
                   fprintf(stdout,":%s.mca.%02x%02x ", DEVPKG_PREFIX,
                                pos_id & 0xff, pos_id >> 8 );

		/* No PdDv object detected, skip to the next slot. */
		DEBUG_1("cfgsio.dc_use_mca: No PdDv object; criteria = %s\n",
			sstr) ;
	    }
	    else if (rc == -1)
	    {
		DEBUG_1("cfgsio.dc_use_mca: ODM get failed; criteria = %s\n",
			sstr) ;
		rc = E_ODMGET ;
	    }
	    else /* Got the PdDv object */
	    {	/* pkg variable set, print package name */
                if( allpkg ) fprintf(stdout,":%s.mca.%02x%02x ", DEVPKG_PREFIX,
                                pos_id & 0xff, pos_id >> 8 );

		sprintf(cwhere, "%d", slot+1) ;
		rc = define_child(cusobj->name, cwhere, &pddv) ;
	    }
	}

    } /* end FOR loop */

    return(rc) ;
} /* END dc_use_mca */


/*
 * NAME: define_child
 *
 * FUNCTION: This routine defines a child to the system if
 *    it does not already exist in ODM.
 *
 * EXECUTION ENVIRONMENT:
 *  This routine is called from within this module ONLY.
 *
 * RETURNS:
 *   0 - success (E_OK)
 *   E_ODMGET    - Get of an object from ODM failed.
 *   E_FINDCHILD - The 'define' method returned an error.
 *   E_ODMUPDATE - Update of an object in ODM failed.
 */
static
int
define_child(char	  *parent_name,
	     char	  *connwhere,
	     struct PdDv  *preobj)

{
    struct CuDv	    cusobj ;
    int 	    rc	    ;
    char	    sstr[128];
    char	    *outp,*errp;

/* BEGIN define_child */
   /*
    *-------------------------------------------------------
    * See if a child already exists at the given location.
    *-------------------------------------------------------
    */
    sprintf(sstr , "parent = '%s' and connwhere = '%s' and PdDvLn = '%s'",
            parent_name,connwhere,preobj->uniquetype);

    DEBUG_1("cfgsio.define_child: checking for device at connection: %s\n",
	    sstr);
    rc = (int)odm_get_first(CuDv_CLASS, sstr, &cusobj);

    if (rc == 0) 
    {
       /*
	*-----------------------------------------------------
	* Child not defined for this location; define it.
	*-----------------------------------------------------
	*/

	sprintf(sstr,"-c %s -s %s -t %s -p %s -w %s",
	    preobj->class, preobj->subclass, preobj->type,
	    parent_name, connwhere);
	DEBUG_1("cfgsio.define_child: Running define %s\n",sstr);

	if (odm_run_method(preobj->Define,sstr,&outp,&errp) != 0) 
	{
	    DEBUG_0("cfgsio.define_child: Define method returned an error\n") ;
	    return(E_FINDCHILD);
	}
    } 
    else if (rc == -1) 
    {
       /* ODM failure getting CuDv for connection check*/
	DEBUG_0("Error getting ODM object.\n");
	return(E_ODMGET);

    } 
    else 
    {
        if (cusobj.status == DEFINED)
	{
	   /* 
	    *------------------------------------------------------------
	    * Device has already been defined, but it is not yet in the 
	    * Available state; so set change status.
	    *------------------------------------------------------------
	    */
	    if (cusobj.chgstatus == MISSING) 
	    {
		cusobj.chgstatus = SAME;
		if (odm_change_obj(CuDv_CLASS,&cusobj) == -1) 
		{
		    DEBUG_0("Error updating change status.\n");
		    return(E_ODMUPDATE);
		}
	    }
	}
	outp = cusobj.name ;
    }

    /* write child name to stdout if all went well */
    fprintf(stdout, "%s\n", outp);
    return(0) ;

} /* END define_child */



/*
 * NAME: read_pos_id
 *
 * FUNCTION: This routine reads the POS ID from POS regs 0 & 1 for
 *    the card in the given slot.
 *
 * EXECUTION ENVIRONMENT:
 *  This routine is called from within this module ONLY.
 *
 * RETURNS:
 *   0        - ERROR occured while reading POS.
 *   pos_id   - the 4 hex digit pos id from POS regs 0 & 1.
 */
static
ushort
read_pos_id(char *bus_name,
	    int  slot)
{
    MACH_DD_IO	mddio   ;
    int		fd      ;
    int		rc      ;
    union
    {
        uchar	bytes[2];
	ushort  id      ;
    } pos ;
    char	bus[16] ;

/* BEGIN read_pos_id */

    pos.id = 0xffff ;

   /*
    *------------------------------------------------------
    * Must read POS through the bus on which you are 
    * attached.
    *------------------------------------------------------
    */
    sprintf(bus, "/dev/%s", bus_name) ;
    if ((fd = open(bus, O_RDWR)) < 0)
    {
	DEBUG_2("cfgsio.read_pos_id: Open of '%s' failed with rc = %d\n",
		bus, fd) ;
    }
    else
    {
	mddio.md_size = 1 ;
	mddio.md_incr = MV_BYTE ;

	/* read the 1st byte of the POS id */
	mddio.md_data = &pos.bytes[0] ;
	mddio.md_addr = POSREG(0, slot) ;
	if ((rc = ioctl(fd, MIOCCGET, &mddio)) < 0)
	{
	    DEBUG_2("cfgsio.read_pos_id: Error reading POS 0 from %s slot %d;",
		    bus, slot) ;
            DEBUG_1("rc = %d\n", rc) ;
	}
	else
	{
	    /* read the 2nd byte of the POS id */
	    mddio.md_data = &pos.bytes[1] ;
	    mddio.md_addr = POSREG(1, slot) ;
	    if ((rc = ioctl(fd, MIOCCGET, &mddio)) < 0)
	    {
		DEBUG_2(
		    "cfgsio.read_pos_id: Error reading POS 1 from %s slot %d;",
		    bus, slot) ;
		DEBUG_1("rc = %d\n", rc) ;
	    }
	}
	close(fd) ;
    }

    return(pos.id) ;
} /* END read_pos_id */


/*^L
 * NAME: get_iplcb
 *
 * FUNCTION: This routine reads the iplcb for the model code and
 *    returns the value.
 *
 * EXECUTION ENVIRONMENT:
 *  This routine is called from within this module ONLY.
 *
 * RETURNS:
 *   0 on success
 *   on error, error return code
 */
static
ushort
get_iplcb(long  *modelcode)
{
        IPL_DIRECTORY   iplcb_dir;        /* IPL control block directory  */
        IPL_INFO        iplcb_info;       /* IPL control block info section*/
        MACH_DD_IO      mdd;              /* machdd ioctl access struct   */
        int             fd;



        if ((fd = open("/dev/nvram",0)) < 0)  {
                DEBUG_0("Unable to open /dev/nvram")
                return(E_DEVACCESS);
        }

       /*
        * Get the IPL CB directory - so we can access other parts of IPLCB
        */
        mdd.md_addr = 128 ;
        mdd.md_data = (uchar *)&iplcb_dir ;
        mdd.md_size = sizeof(iplcb_dir) ;
        mdd.md_incr = MV_BYTE ;

        DEBUG_0("Calling mdd ioctl for iplcb_dir\n")
        if (ioctl(fd, MIOIPLCB, &mdd)) {
            DEBUG_0("Error reading IPL-Ctrl block directory")
            return(E_DEVACCESS);
        }
       /*
        * Get the IPL info section - it contains the model codes.
        */
        mdd.md_addr = iplcb_dir.ipl_info_offset ;
        mdd.md_data = (uchar *)&iplcb_info ;
        mdd.md_size = sizeof(iplcb_info) ;
        mdd.md_incr = MV_BYTE ;

        DEBUG_0("Calling mdd ioctl for ipl_info\n")
        if (ioctl(fd, MIOIPLCB, &mdd))
        {
            DEBUG_0("Error reading IPL-Ctrl block info section")
            return(E_DEVACCESS);
        }

        *modelcode = iplcb_info.model ;
        close(fd) ;
        return( 0 );
}

