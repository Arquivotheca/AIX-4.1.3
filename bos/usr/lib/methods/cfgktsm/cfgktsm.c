static char sccsid[] = "@(#)14  1.6  src/bos/usr/lib/methods/cfgktsm/cfgktsm.c, inputdd, bos41J, 9519A_all 5/9/95 09:26:42";
/*
 *   COMPONENT_NAME: INPUTDD
 *
 *   FUNCTIONS: build_dds
 *		define_children
 *		device_specific
 *		download_microcode
 *		generate_minor
 *		make_special_files
 *		query_vpd
 *		setlocation
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993, 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "cfgktsm.h"

extern int defchild(char *, char *, char *);
extern char *gettok(char **);

/*
 * NAME     : build_dds 
 *
 * FUNCTION : This function builds the DDS for device driver
 *
 * NOTES :
 *	This function gets the values of the attributes from the ODM
 *	database, assigns the values to the DDS structure, and returns
 *  a pointer to the dds and its size.
 *	
 * RETURNS :
 * 	Returns  0 on success, > 0 on failure.
 */

int build_dds(lname,ddsptr,dds_len)
char	*lname;				/* logical name of the device	*/
char	**ddsptr;			/* pointer to dds structure	*/
int 	*dds_len;			/* size of dds structure	*/
{
	static  struct	ktsmdds	*dds;  	/* pointer to kbd_dds structure	*/
	struct	CuDv 	busobj ;	/* Customized object for bus	*/
	struct  CuDv    cudata;
	char	sstring[256];		/*  search criteria string	*/
    int     rc;
	char    *p;

	DEBUG_0("enter build_dds:\n")

	/* Allocate memory for dds structure  */
	dds = (struct ktsmdds *)malloc(sizeof(struct ktsmdds));
	if(dds == NULL){
		DEBUG_0("malloc failed\n")
		return(E_MALLOC);
	}

	/* Read Customized object of bus  */
	if(rc=Get_Parent_Bus(CuDv_CLASS, cudv.parent, &busobj)) {
	   DEBUG_0("error getting bus object \n");
	   return(rc);
	}

	/* save bus ID and slot number in dds                                  */
	GETATT( &dds->bus_id, 'l', busobj, "bus_id", NULL )
    dds->slot_addr = (atoi(pcudv.connwhere)-1) & 0x0f ;

	/* if we are a child of a card plugged directly into the MCA bus,      */
	/* get address, interrupt level and interrupt priority from our        */
	/* parent.                                                             */

	if (strcmp(pddv.subclass, MCA_SUBCLASS) == 0) {
	  GETATT( &dds->bus_io_addr, 'l', pcudv, "bus_io_addr", NULL )
	  GETATT( &dds->bus_intr_lvl, 'i', pcudv, "bus_intr_lvl", NULL )
	  GETATT( &dds->intr_priority, 'i', pcudv, "intr_priority", NULL )

	  /* base address specified by parent points to mouse registers,       */
	  /* keyboard regsiters start at a fix offset from base address        */
	  if (strcmp(pddv.type, KEYBOARD_TYPE) == 0) {
	    dds->bus_io_addr += KEYBOARD_OFFSET;
	  }
	}

	/* else parent is SIO or we are on ISA bus so get PIO address,         */
	/* interrupt level, and interrupt priority from our own ODM data       */

  	else {
	  GETATT( &dds->bus_io_addr, 'l', cudv, "bus_io_addr", NULL )
	  GETATT( &dds->bus_intr_lvl, 'i', cudv, "bus_intr_lvl", NULL )
	  GETATT( &dds->intr_priority, 'i', cudv, "intr_priority", NULL )

/* the following code is for rspc keyboard and mouse only                 */
/* the code passes the devno of the keyboard adapter to mousedd and the   */
/* devno of the mouse adapter to kbddd so that the drivers can find each  */
/* other's device switch table. In the device switch table is a pointer   */
/* to the adapter lock structure.                                         */

                                  /* only do if isa mouse or kbd adapter  */
	  if ((strcmp(pddv.type, "isa_mouse") == 0) ||
	       (strcmp(pddv.type, "isa_keyboard") == 0)) {
                                  /* if kbd adapter then look for mouse   */
	    if (strcmp(pddv.type, "isa_keyboard") == 0)
	      p = "PdDvLn = 'adapter/isa_sio/isa_mouse' AND status = 1";
	    else                      /* if mouse adapter then look for kbd   */
	      p = "PdDvLn = 'adapter/isa_sio/isa_keyboard' AND status = 1";
                                  /* get CuDv of sister adapter           */
	    if( (rc = (int)odm_get_first(CuDv_CLASS, p, &cudata)) < 0 ) {
	      DEBUG_0("fatal ODM error\n");
	      return(E_ODMGET);
	    }
	    else {
	      if( rc == 0 ) {
	        DEBUG_1("sister adapter not Available, %s\n", p);
	        dds->devno_link = -1;
	      }
	      else {                  /* get devno of sister adapter          */
	        if((rc=get_devno(cudata.name, &dds->devno_link)) != E_OK) {
	          DEBUG_1("could not get devno of %s", cudata.name);
	          return(rc);
	        }
	      }
	    }
	  }
	}

	dds->device_class = ADAPTER;

	*ddsptr = (caddr_t)dds;
	*dds_len = sizeof(struct ktsmdds);

#ifdef	CFGDEBUG
	dump_dds(*ddsptr,*dds_len);
#endif 

	DEBUG_0("build_dds successful\n")
	return(0);
}

/*
 * NAME     : generate_minor
 *
 * FUNCTION : This function generates a minor number
 *
 * RETURNS :
 *  0 = success
 *  >0 = errno.
 */

int generate_minor(lname,majorno,minordest)
char    *lname;         /* logical name of device       */
long    majorno;        /* major number of the device       */
long    *minordest;
{
	long    *minorno;

	DEBUG_0("generate minor\n")
	if((minorno = genminor(lname,majorno,-1,1,1,1)) == (long *)NULL )
	  return E_MINORNO;
	*minordest = *minorno;
	free(minorno);

	return 0;
}

/*
 * NAME     : make_special_files
 *
 * FUNCTION : This function creates a special file for the adapter
 *
 * NOTES :
 *      This function does nothing. Creation of special files is
 *      taken care of in the device specific config methods for
 *      the connected devices.
 *
 * RETURNS : 0
 */

int make_special_files(lname,devno)
char    *lname;         /* logical name of the device       */
dev_t   devno;          /* device number            */
{
	return 0;
}

/*
 * NAME     : download_microcode
 *
 * FUNCTION : This function download microcode if applicable
 *
 * NOTES :
 *      There is no microcode provision for the adapter so this
 *      routine does nothing
 *
 * RETURNS : 0
 */

int download_microcode(lname)
char    *lname;         /* logical name of the device       */
{
	return 0;
}

/*
 * NAME     : query_vpd
 *
 * FUNCTION : This function is used to get device specific VPD.
 *
 * NOTES :
 *      VPD is read by define_children() so this routine does nothing
 *
 * RETURNS : 0
 */
int query_vpd(newobj,kmid,devno,vpd_dest)
struct  CuDv    *newobj;        /* vpd info will be put in that         */
mid_t   kmid;                   /* kernel module id                     */
dev_t   devno;                  /* device number                        */
char    *vpd_dest;              /* Destination for vpd                  */

{
	return 0;
}

/*
 * NAME     : setlocation
 *
 * FUNCTION : This function sets location code
 *
 * RETURNS : 0 = success
 */

int setlocation(char location)
{

	char sstring[128];
	int rc = 0;

    DEBUG_1("enter setlocation with location %c\n", location)

	sprintf(sstring, "%s-0%c", pcudv.location, location) ;
	if (strcmp(cudv.location, sstring) != 0) {
	  DEBUG_0("updating location\n");
	  strcpy(cudv.location, sstring);
	  if (odm_change_obj(CuDv_CLASS,&cudv) == -1) {
	    DEBUG_0("Error updating location\n");
	    rc = E_ODMUPDATE;
	  }
	}
	return(rc);
}


/*
 * NAME     : device_specific
 *
 * FUNCTION : This function allows for device specific code to be
 *            executed.
 *
 * NOTES :
 *      This routine is used to "fixup"  location code of device so that
 *      "K" is port number of keyboard and "M" is port number of mouse.
 *      Normally this function is done during define_children() step of
 *      parent but if this child is defined via mkdev and parent is 
 *      MCA adapter card then location code will need to "fixed up".
 *
 * RETURNS : 0
 */

int device_specific()
{
	int rc = 0;

    DEBUG_0("enter device_specific\n")

	if (strcmp(pddv.subclass, MCA_SUBCLASS) == 0) {
	  if (strcmp(pddv.type, KEYBOARD_TYPE) == 0)
        rc =  setlocation(KEYBOARD_LOCATION);
	  else
	    if (strcmp(pddv.type, MOUSE_TYPE) == 0)
          rc =  setlocation(MOUSE_LOCATION);
	}

	return(rc);
}

/*
 * NAME     : define_children 
 *
 * FUNCTION : This function defines children attached to the adapter
 *
 * PROCEDURE:
 *            1) execute CFG_QVPD to get list of connected devices
 *            2) for each device returned from CFG_QVPD
 *                 - see if device is defined, if so output to stdout
 *                 - if not defined, run define method
 *
 * RETURNS :
 * 	0 for success, >0 = errno.
 */

int define_children(lname,ipl_phase)
char	*lname;			/* logical name of the device 		*/
int	ipl_phase;		/* ipl phase				*/
{

	struct cfg_dd cfg;           /* Used to call DD entry point  */
	struct CuDvDr cudvdr;        /* customized device driver obj */
	char   sstring[128];         /* receive area for strings     */ 
	char   *vpd_data;            /* receive area for VPD strings */ 
	char   *uniquetype;          /* child unique type            */
	char   *connection;          /* child connection             */
	char   *p;
	int    i, rc;

    DEBUG_0("enter define_children\n")

/* if adapter is already available then cfgcommon does not load     */
/* driver so kmid and devno have to be generated here               */

    if (!loaded_dvdr) {
	  /* get name of device driver  */
	  if (rc = get_dvdr_name()) {
	    DEBUG_0("get_dvdr_name failed\n")
		return(rc);
	  }
	  /* get kmid of device driver */
	  if ((dvdr[0] == 0) ||
	      ((kmid = loadext(dvdr, FALSE, TRUE)) == NULL)) {
	    DEBUG_0("loadext failed \n")
	    return(E_LOADEXT);
	  }
	  /* get device number         */
	  sprintf(sstring, "value3 = '%s' AND resource = devno", lname);
	  rc = (int)odm_get_first(CuDvDr_CLASS, sstring, &cudvdr);
	  if( rc <= 0 ) {
	    DEBUG_1("error getting cudvdr object for %s\n", lname)
	    return(E_ODMGET);
	  }
	  devno = makedev((ulong)strtoul(cudvdr.value1, (char **)NULL, 0),
	            (ulong)strtoul(cudvdr.value2, (char **)NULL, 0));
	}

/* call sysconfig() to get VPD data                                      */
/* VPD data is used to transfer list of children to config method        */

	DEBUG_0("call sysconfig() - CFG_QVPD\n")
	DEBUG_1("kmid = %x\n", kmid)
	DEBUG_1("devno = %x\n", devno)

	cfg.kmid = kmid;
	cfg.devno = devno;
	cfg.cmd = CFG_QVPD;
	cfg.ddsptr = sstring;
	cfg.ddslen = sizeof(sstring);
	if ( sysconfig(SYS_CFGDD, &cfg, sizeof(struct cfg_dd )) == -1 ) {
	  DEBUG_0("error querying VPD\n")
	  return(E_VPD);
	}

    /* define the children              */
	p = sstring;
	do {
	  uniquetype = gettok(&p);
	  connection = gettok(&p);
	  rc = defchild(lname, uniquetype, connection);
	} while (*p && !rc);

	DEBUG_0( "\nexit define_children()\n" );

	return(rc);
}
