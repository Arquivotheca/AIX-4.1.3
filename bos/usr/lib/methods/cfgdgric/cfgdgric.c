static char sccsid[] = "@(#)14	1.3  src/bos/usr/lib/methods/cfgdgric/cfgdgric.c, dd_artic, bos411, 9436B411a 9/1/94 14:59:37";
/*
 * COMPONENT_NAME: dd_artic -- ARTIC diagnostic driver.
 *
 * FUNCTIONS: main, err_exit, convert_att, getatt, generate_minor,
 *            att_changed, convert_seq, make_special_files
 *            err_undo, getbasetype, getadaptertype
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1993, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * FUNCTION: Configure method for ARTIC Diagnostic Device Driver
 *
 * interface:
 * cfgt1pmd -l <logical_name> [-<1|2>]
*/


/* header files needed for compilation */
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/cfgdb.h>
#include <sys/errno.h>
#include <sys/cfgodm.h>
#include <sys/sysmacros.h>
#include <sys/sysconfig.h>
#include <sys/mdio.h>
#include <cf.h>
#include <fcntl.h>
#include <sys/dartic.h>
#include <sys/darticdd.h>

/* Device data structure passed to driver's config entry point */
DARTIC_Adapter articdds;

/* This structure is used by the getatt function */

struct attr {
        char *attribute;
        char *value;
};
extern	int	errno;

/* main function code */
main(argc, argv, envp)
int     argc;
char    *argv[];
char    *envp[];

{
extern  int     optind;         /* for getopt function */
extern  char    *optarg;        /* for getopt function */

        struct cfg_dd cfg;              /* sysconfig command structure */
        char    *logical_name;          /* logical name to configure */
        char    *phase1, *phase2;       /* ipl phase flags */
        char    sstring[256];           /* search criteria string */

        long    majorno, minorno;       /* major and minor numbers      */
        long    *minor_list;            /* list returned by getminor    */
        int     how_many;               /* number of minors in list     */

        struct Class *cusdev;           /* customized devices class ptr */
        struct Class *predev;           /* predefined devices class ptr */

        struct CuDv cusobj;             /* customized device object storage */
        struct PdDv preobj;             /* predefined device object storage */
        struct CuDv parobj;             /* customized device object storage */
        struct CuDv gparobj;            /* customized device object storage */
        struct CuDv dmyobj;             /* customized device object storage */

        ushort  devid;                  /* Device id - used at run-time       */
        int     ipl_phase;              /* ipl phase: 0=run,1=phase1,2=phase2 */
        int     slot;                   /* slot of adapters                 */
        int     subtype;                /* Adapter EIB identifier           */
        int     rc;                     /* return codes go here             */
        int     errflg,c,terminate_flg; /* used in parsing parameters       */
	char	*mountpoint;
	char	ddname[255];		/* Name of device driver	    */

        /* declarations for card query */
        char    busdev[32] ;

        /*****                                                          */
        /***** Parse Parameters                                         */
        /*****                                                          */
        ipl_phase = RUNTIME_CFG;
        errflg = 0;
	terminate_flg=0;
        logical_name = NULL;

        while ((c = getopt(argc,argv,"l:d12t")) != EOF) {
                switch (c) {
                case 'l':
                        if (logical_name != NULL)
                                errflg++;
                        logical_name = optarg;
                        break;
                case 'd':
                        ++articdds.debugflag;
                        break;
                case '1':
                        if (ipl_phase != RUNTIME_CFG)
                                errflg++;
                        ipl_phase = PHASE1;
                        break;
                case '2':
                        if (ipl_phase != RUNTIME_CFG)
                                errflg++;
                        ipl_phase = PHASE2;
                        break;
		case 't':
			/* Unload diagnostics driver */
			terminate_flg++;
			break;
                default:
                        errflg++;
                }
        }
        if (errflg)
                /* error parsing parameters */
                exit(E_ARGS);

        /*****                                                          */
        /***** Validate Parameters                                      */
        /*****                                                          */
        /* logical name must be specified */
        if (logical_name == NULL)
                exit(E_LNAME);

        /* start up odm */
        if (odm_initialize() == -1)
                /* initialization failed */
                exit(E_ODMINIT);

        /* lock the database */
        if (odm_lock("/etc/objrepos/config_lock",0) == -1)
                err_exit(E_ODMLOCK);

        /* open customized devices object class */
        if ((int)(cusdev = odm_open_class(CuDv_CLASS)) == -1)
                err_exit(E_ODMOPEN);

        /* search for customized object with this logical name */
        sprintf(sstring, "name = '%s'", logical_name);
        rc = (int)odm_get_first(cusdev,sstring,&cusobj);
        if (rc==0) {
                /* No CuDv object with this name */
                err_exit(E_NOCuDv);
        }
        else if (rc==-1) {
                /* ODM failure */
                err_exit(E_ODMGET);
        }

        /* open predefined devices object class */
        if ((int)(predev = odm_open_class(PdDv_CLASS)) == -1)
                err_exit(E_ODMOPEN);

        /* get predefined device object for this logical name */
        sprintf(sstring, "uniquetype = '%s'", cusobj.PdDvLn_Lvalue);
        rc = (int)odm_get_first(predev, sstring, &preobj);
        if (rc==0) {
                /* No PdDv object for this device */
                err_exit(E_NOPdDv);
        }
        else if (rc==-1) {
                /* ODM failure */
                err_exit(E_ODMGET);
        }

	if((mountpoint=(char *)getenv("CDRFS_DIR")) != (char *)NULL)
		sprintf(ddname, "%s/usr/lib/drivers/%s", mountpoint,
				preobj.DvDr);
	else
		sprintf(ddname, "%s", preobj.DvDr);

        /* close predefined device object class */
        if (odm_close_class(predev) == -1)
                err_exit(E_ODMCLOSE);

        /******************************************************************
          Check to see if the adapter is already configured (AVAILABLE).
          We actually go about the business of configuring the adapter
          only if it is not configured yet. Configuring the adapter
          refers to the process of checking parent status, resolving
          bus resources, and obtaining VPD(if necessary).
          ******************************************************************/
        if (cusobj.status == DEFINED) {

                /* The adapter is not configured */

                /* get the device's parent object */
                sprintf(sstring, "name = '%s'", cusobj.parent);
                rc = (int)odm_get_first(cusdev,sstring,&parobj);
                if (rc==0) {
                        /* Parent device not in CuDv */
                        err_exit(E_NOCuDvPARENT);
                }
                else if (rc==-1) {
                        /* ODM failure */
                        err_exit(E_ODMGET);
                }

                /* Parent MUST be available to continue */
                if (parobj.status != AVAILABLE)
                        /* parent is not AVAILABLE */
                        err_exit(E_PARENTSTATE);

                /* make sure that no other devices are configured     */
                /* at this location                                   */
                sprintf(sstring, "parent = '%s' AND connwhere = '%s' AND status = %d",
                        cusobj.parent, cusobj.connwhere, AVAILABLE);
                rc = (int)odm_get_first(cusdev,sstring,&dmyobj);
                if (rc == -1) {
                        /* odm failure */
                        err_exit(E_ODMGET);
                } else if (rc) {
                        /* Error: device config'd at this location */
                        err_exit(E_AVAILCONNECT);
                }

                /* get the device's grand parent object     */
                /* we need this to get the bus id attribute */
                sprintf(sstring, "name = '%s'", parobj.parent);
                rc = (int)odm_get_first(cusdev,sstring,&gparobj);
                if (rc==0) {
                        /* Parent device not in CuDv */
                        err_exit(E_NOCuDvPARENT);
                }
                else if (rc==-1) {
                        /* ODM failure */
                        err_exit(E_ODMGET);
                }

                /* Get adapter base type */
                sscanf(parobj.connwhere,"%d",&slot);
                if ((articdds.basetype = getbasetype(gparobj.name, slot)) == -1)
                   err_exit(E_NODETECT);

                /* Read _subtype from CuAt, and PdAt tables */
                if(( rc = getatt( &subtype, 'i', CuAt_CLASS,
                     PdAt_CLASS, parobj.name, parobj.PdDvLn_Lvalue,
                     "_subtype", (struct attr *) NULL )) > 0 ) err_exit (E_BADATTR);

                /* Generate adapter type based on base type and _subtype attr */
                articdds.adaptertype = getadaptertype(articdds.basetype,
                                                      subtype);

                /* Read busid from CuAt, and PdAt tables */
                if(( rc = getatt( &articdds.bus_id, 'i', CuAt_CLASS,
                     PdAt_CLASS, gparobj.name, gparobj.PdDvLn_Lvalue,
                     "bus_id", (struct attr *) NULL )) > 0 ) err_exit (E_BADATTR);

                /* Read attribute from CuAt, and PdAt tables */
                if(( rc = getatt( &articdds.basemem, 'i', CuAt_CLASS,
                     PdAt_CLASS, parobj.name, parobj.PdDvLn_Lvalue,
                     "bus_mem_addr", (struct attr *) NULL )) > 0 ) err_exit (E_BADATTR);

                /* Read attribute from CuAt, and PdAt tables */
                if(( rc = getatt( &articdds.baseio, 'i', CuAt_CLASS,
                     PdAt_CLASS, parobj.name, parobj.PdDvLn_Lvalue,
                     "bus_io_addr", (struct attr *) NULL )) > 0 ) err_exit (E_BADATTR);

                /* Read attribute from CuAt, and PdAt tables */
                if(( rc = getatt( &articdds.intlevel, 'i', CuAt_CLASS,
                     PdAt_CLASS, parobj.name, parobj.PdDvLn_Lvalue,
                     "bus_intr_lvl", (struct attr *) NULL )) > 0 ) err_exit (E_BADATTR);

                if ((articdds.basetype == PORTMASTER) ||
                    (articdds.basetype == SP5_ADAPTER))
                {
                   /* Read attribute from CuAt, and PdAt tables */
                   if(( rc = getatt( &articdds.dmalevel, 'i', CuAt_CLASS,
                        PdAt_CLASS, parobj.name, parobj.PdDvLn_Lvalue,
                        "dma_lvl", (struct attr *) NULL )) > 0 ) err_exit (E_BADATTR);

                   /* Read attribute from CuAt, and PdAt tables */
                   if(( rc = getatt( &articdds.dmamem, 'i', CuAt_CLASS,
                        PdAt_CLASS, parobj.name, parobj.PdDvLn_Lvalue,
                        "dma_bus_mem", (struct attr *) NULL )) > 0 ) err_exit (E_BADATTR);

                }

                /*
                 * Call loadext() to load device driver.
                 */
                if ((cfg.kmid=loadext(ddname,TRUE,FALSE)) == NULL) {
                        err_exit(E_LOADEXT);
                }

                /* Set remaining attributes */
                articdds.windowsize = 0x10000; /* 64 K window size */
                articdds.module_id = cfg.kmid;
                sscanf(parobj.connwhere,"%d",&articdds.slot);
                --articdds.slot;

                /*
                 * genmajor() will create a major number for this device
                 * if one has not been created yet. If one already exists,
                 * it will return that one.
                 */
                if ((majorno = genmajor(preobj.DvDr)) == -1) {
                        loadext(ddname, FALSE, FALSE);
                        err_exit(E_MAJORNO);
                }

                /*
                 * Get minor number.
                 */
                minor_list = getminor(majorno, &how_many, logical_name);
                if (minor_list == NULL || how_many == 0) {
                        if ((rc = generate_minor(logical_name, majorno, &minorno))
                             != E_OK) {
                                reldevno(logical_name, TRUE);
                                loadext(ddname, FALSE, FALSE);
                                err_exit(rc);
                        }
                } else
                        minorno = *minor_list;


                /*
                 * Create devno for this device
                 */
                cfg.devno = makedev(majorno, minorno);
                articdds.cardnumber = minor(cfg.devno);

                /*
                 * Now call sysconfig() to configure the driver.
                 */

                cfg.cmd = CFG_INIT;
                cfg.ddsptr = (caddr_t ) &articdds;
                cfg.ddslen = sizeof(DARTIC_Adapter);
                if (sysconfig(SYS_CFGDD,&cfg,sizeof(struct cfg_dd)) == -1) {
                        loadext(ddname, FALSE, FALSE);
                        err_exit(E_CFGINIT);
                }

                /*
                 * Now make the special files that this device will
                 * be accessed through.
                 */
                if ((rc=make_special_files(logical_name,cfg.devno)) != E_OK) {
                        err_undo(cfg.devno);
                        loadext(ddname, FALSE, FALSE);
                        err_exit(rc);
                }

                /*
                 * Update the customized object to AVAILABLE status.
                 */
                cusobj.status = AVAILABLE;
                cusobj.chgstatus = SAME;
                if ((rc = odm_change_obj(CuDv_CLASS, &cusobj)) < 0) {
                        err_exit(E_ODMUPDATE);
                }

        } else { /* end if (adapter is not AVAILABLE) then ... */
		if ( (terminate_flg) && (cusobj.status == AVAILABLE) ){

		/******************************************************
		  Call sysconfig() to "terminate" the device
		  If fails with EBUSY, then device instance is "open",
		  and device cannot be "unconfigured".  Any other errno
		  returned will be ignored since the driver MUST delete
		  the device even if it reports some other error.
		 ******************************************************/

			/* first, need to create devno for this device */
			majorno = genmajor(preobj.DvDr);
			if (majorno == -1)
				err_exit(E_MAJORNO);

			/* get minor number      */
			minor_list = getminor(majorno, &how_many, logical_name);
			if (minor_list == NULL || how_many == 0)
				err_exit(E_MINORNO);

			minorno = *minor_list;

			/* create devno for this device */
			cfg.devno = makedev(majorno,minorno);
			cfg.kmid = (mid_t)0;
			cfg.ddsptr = (caddr_t) NULL;
			cfg.ddslen = (int)0;
			cfg.cmd = CFG_TERM;

			if (sysconfig(SYS_CFGDD,&cfg,sizeof(struct cfg_dd)) == -1) {
				if (errno == EBUSY)
				/* device is in use and can not be unconfigured */
					err_exit(E_BUSY);

			/* Ignore other errno values because device driver */
			/* has to complete CFG_TERM operation except when  */
			/* device is busy. */
			}

			/************************************************
			  Call loadext() to unload device driver.
			  loadext() will unload the driver only if
			  device is last instance of driver configured.
			 ************************************************/
			cfg.kmid = loadext(ddname,FALSE,FALSE);
			if (cfg.kmid == NULL)
				/* error unloading device driver */
				err_exit(E_UNLOADEXT);
		        /*************************************************
			  Change the status field of device to "DEFINED"
			 *************************************************/
			cusobj.status = (short)DEFINED;

			if (odm_change_obj(cusdev,&cusobj) == -1)
			   /* Could not change customized device object */
	   			err_exit(E_ODMUPDATE);
												   }



	}
        /* close customized device object class */
        if (odm_close_class(cusdev) == -1)
                err_exit(E_ODMCLOSE);

        odm_terminate();
        exit(0);

}

/*
 * NAME: err_exit
 *
 * FUNCTION: Closes any open object classes and terminates ODM.  Used to
 *           back out on an error.
 *
 * void
 *   err_exit( exitcode )
 *      exitcode = The error exit code.
 *
 * RETURNS:     None
 */

err_exit(exitcode)
char    exitcode;
{
        /* Close any open object class */
        odm_close_class(CuDv_CLASS);
        odm_close_class(PdDv_CLASS);
        odm_close_class(CuAt_CLASS);

        /* Terminate the ODM */
        odm_terminate();
        exit(exitcode);
}



/*
 *  NAME: getbasetype
 *
 *  FUNCTION:
 *      Return ARTIC adapter base type
 *
 *  INPUTS:
 *      bus     - The name of the bus device, for example, bus0.
 *      slot    - The slot number from the parent connection descriptor.
 *                It should be a value of 1 through 8.
 *
 * RETURNS: Returns adapter base type on success,
 *          -1 on Error
 */

int
getbasetype(bus, slot)
char *bus;
int slot;
{
        MACH_DD_IO mddRecord;
        char devname[16];
        uchar pos[2];
        int fd;
        int i;

        pos[0] = 0xff;
        pos[1] = 0xff;

        /* Decrement slot number found in database */
        slot == (slot--) & 0x0F;

        sprintf(devname,"/dev/%s", bus);
        if (0 > (fd = open(devname, O_RDWR)))
                return (-1);

        mddRecord.md_size = 2;
        mddRecord.md_incr = MV_BYTE;
        mddRecord.md_data = pos;
        mddRecord.md_addr = POSREG(0, slot);

        /* Read POS 0 and POS 1   */
        if (0 > ioctl(fd, MIOCCGET, &mddRecord))
                return (-1);

        close(fd);

        /* Determine base type from POS values */
        if ((pos[0] == PORTM_POS0) && (pos[1] == PORTM_POS1))
           return(PORTMASTER);

        if ((pos[0] == MP2_POS0) && (pos[1] == MP2_POS1))
           return(MULTIPORT_2);

        if ((pos[0] == SP5_POS0) && (pos[1] == SP5_POS1))
           return(SP5_ADAPTER);

        /* Return error if adapter is not in the ARTIC family */
        return (-1);
}

/*
 *  NAME: getadaptertype
 *
 *  FUNCTION:
 *      Return ARTIC adapter type
 *
 *  INPUTS:
 *      basetype - Adapter base type (eg Portmaster)
 *      subtype  - _subtype field from PdDv of parent
 *
 * RETURNS: Returns adapter type
 */

int
getadaptertype(basetype, subtype)
int basetype;
int subtype;
{

         switch(basetype)
         {

              case MULTIPORT_2:
                 switch(subtype)
                 {
                    case 1:
                    return(X25);
                    case 182:
                    return(MP2_4P232);
                    case 183:
                    return(MP2_6PSYNC);
                    case 184:
                    return(MP2_8P232);
                    case 185:
                    return(MP2_8P422);
                    case 186:
                    return(MP2_4P4224P232);
                    case 180:
                    return(MP2_UNRECOG_EIB);
                    default:
                    return(MP2_UNRECOG_EIB);
                 }
              break;

              case PORTMASTER:
                 switch(subtype)
                 {
                    case 1:
                    return(MPQP);
                    case 11:
                    return(PM_6PV35);
                    case 12:
                    return(PM_6PX21);
                    case 13:
                    return(PM_8P232);
                    case 14:
                    return(PM_8P422);
                    case 10:
                    return(PM_UNRECOG_EIB);
                    default:
                    return(PM_UNRECOG_EIB);
                 }
              break;

              case SP5_ADAPTER:
                 switch(subtype)
                 {
                    case 1:
                    return(SP5);
                    default:
                    return(PM_UNRECOG_EIB);
                 }
              break;
         }
}


/*
 * NAME: getatt
 *
 * FUNCTION: Reads an attribute from the customized database, predefined
 *      database, or change list.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This routine is linked into the device specific sections of the
 *      various config, and change methods. No global variables are used.
 *
 * NOTES:
 *
 * int
 *   getatt(dest_addr,dest_type,cuat_oc,pdat_oc,lname,utype,att_name,newatt )
 *
 *      dest_addr = pointer to the destination field.
 *      dest_type = The data type which the attribute is to be converted to
 *                    's' = string              rep=s
 *                    'b' = byte sequence       rep=s,  e.g. "0x56FFE67.."
 *                    'l' = long                rep=n
 *                    'i' = int                 rep=n
 *                    'h' = short (half)        rep=n
 *                    'c' = char                rep=n,or s
 *                    'a' = address             rep=n
 *      cuat_oc   = Customized Attribute Object Class.
 *      pdat_oc   = Predefined Attribute Object Class.
 *      lname     = Device logical name. ( or parent's logical name )
 *      utype     = Device uniquetype. ( or parent's uniquetype )
 *      att_name  = attribute name to retrieve from the Customized
 *                  Attribute Object Class.
 *      newatt    = New attributes to be scanned before reading database
 *
 *
 * RETURNS:
 *      0  = Successful
 *      <0 = Successful (for byte sequence only, = -ve no. of bytes)
 *      >0 = errno ( E_NOATTR = attribute not found )
 *
 */

int getatt(dest_addr,dest_type,cuat_oc,pdat_oc,lname,utype,att_name,newatt)
void            *dest_addr;     /* Address of destination                   */
char            dest_type;      /* Destination type                         */
struct  Class   *cuat_oc;       /* handle for Customized Attribute OC       */
struct  Class   *pdat_oc;       /* handle for Predefined Attribute OC       */
char            *lname;         /* device logical name                      */
char            *utype;         /* device unique type                       */
char            *att_name;      /* attribute name                           */
struct  attr    *newatt;        /* List of new attributes                   */
{
        struct  CuAt    cuat_obj;
        struct  PdAt    pdat_obj;
        struct  attr    *att_changed();
        struct  attr    *att_ptr;
        int             convert_seq();
        int             rc;
        char            srchstr[100];
        char            *val_ptr;
        char            rep;

        /* Note: We need an entry from customized, or predefined even if */
        /* an entry from newatt is going to be used because there is no  */
        /* representation (rep) in newatt                                */

        /* SEARCH FOR ENTRY IN CUSTOMIZED ATTRIBUTE CLASS */

        sprintf(srchstr, "name = '%s' AND attribute = '%s'", lname, att_name );

        if( cuat_oc == (struct Class *)NULL )
                rc = 0;
        else
                rc = odm_get_obj( cuat_oc, srchstr, &cuat_obj, TRUE );

        if( rc == 0 )
        {
                /* OBJECT NOT FOUND, SEARCH IN PREDEFINED ATTRIBUTE CLASS */

                sprintf(srchstr, "uniquetype = '%s' AND attribute = '%s'",
                        utype, att_name );

                if((rc=odm_get_obj( pdat_oc, srchstr, &pdat_obj, TRUE ))==0)
                {
                        return(E_NOATTR);
                }
                else if ( rc == -1 )
                {
                        return(E_ODMGET);
                }
                /* USE THE PREDEFINED ENTRY ( for now ) */

                val_ptr = pdat_obj.deflt;
                rep = pdat_obj.rep[strcspn(pdat_obj.rep,"sn")];

        }
        else if ( rc == -1 )
        {
                return(E_ODMGET);
        }
        else
        {
                /* USE THE CUSTOMIZED ENTRY ( for now ) */

                val_ptr = cuat_obj.value;
                rep = cuat_obj.rep[strcspn(cuat_obj.rep,"sn")];
        }

        /* CHECK TO SEE IF THIS ATTRIBUTE IS IN CHANGED LIST */

        if( ( att_ptr = att_changed(newatt,att_name))!=NULL)
                val_ptr = att_ptr->value;

        /* CONVERT THE DATA TYPE TO THE DESTINATION TYPE */

        return (convert_att( dest_addr, dest_type, val_ptr, rep ));
}

/*
 * NAME: convert_att
 *
 * FUNCTION: This routine converts attributes into different data types
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Generally this routine is called by getatt(), but it is available
 *      to other procedures which need to convert data which may not also
 *      be represented in the database.
 *      No global variable are used, so this may be dynamically linked.
 *
 * RETURNS:
 *
 *       0 = Successful
 *      <0 = Successful (for byte sequence only, = -ve no. of bytes)
 *      >0 = errno
 */

int convert_att( dest_addr, dest_type, val_ptr, rep )
void    *dest_addr;             /* Address of destination                   */
char    dest_type;              /* Destination type                         */
char    *val_ptr;               /* Address of source                        */
char    rep;                    /* Representation of source ('s', or 'n')   */
{

        if( rep == 's' )
        {
                switch( dest_type )
                {
                case 's':
                        strcpy( (char *)dest_addr, val_ptr );
                        break;
                case 'c':
                        *(char *)dest_addr = *val_ptr;
                        break;
                case 'b':
                        return ( convert_seq( val_ptr, (char *)dest_addr ) );
                default:
                        return (E_BADATTR);
                }
        }
        else if( rep == 'n' )
        {
                switch( dest_type )
                {
                case 'l':
                        *(long *)dest_addr =
                                strtoul( val_ptr, (char **)NULL, 0);
                        break;
                case 'i':
                        *(int *)dest_addr =
                                (int)strtoul( val_ptr, (char **)NULL, 0);
                        break;
                case 'h':
                        *(short *)dest_addr =
                                (short)strtoul( val_ptr, (char **)NULL, 0);
                        break;
                case 'c':
                        *(char *)dest_addr =
                                (char)strtoul( val_ptr, (char **)NULL, 0);
                        break;
                case 'a':
                        *(void **)dest_addr =
                                (void *)strtoul( val_ptr, (char **)NULL, 0);
                        break;
                default:
                        return (E_BADATTR);
                }
        }
        else
        {
                return (E_BADATTR);
        }
        return 0;
}

/*
 * NAME: att_changed
 *
 * FUNCTION: Searches for an attribute in the new_attributes list
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Routines calling att_changed should include pparms.h for
 *      the definition of struct attr.
 *      This routine uses no global variables.
 *
 * NOTES:
 *
 *      if the list of changed attributes (at) is a NULL pointer, the
 *      routine accepts that there are no parameters in the list.
 *      Generally, the list consists of a sequence of attributes with
 *      the last attribute having a name of NULL.
 */

struct attr *att_changed(at,attname)
struct  attr *at;
char    *attname;
{
        struct  attr *p = at;

        if( at != NULL )
                while(p->attribute != NULL)
                {
                        if(strcmp(p->attribute,attname) == 0)
                                return (p);
                        p++;
                }
        return (struct attr *)NULL;
}

/*
 * NAME: convert_seq
 *
 * FUNCTION: Converts a hex-style string to a sequence of bytes
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This routine uses no global variables
 *
 * NOTES:
 *
 *      The string to be converted is of the form
 *      "0xFFAAEE5A567456724650789789ABDEF678"  (for example)
 *      This would put the code FF into the first byte, AA into the second,
 *      etc.
 *
 * RETURNS: No of bytes, or -3 if error.
 *
 */

int convert_seq( source, dest )
char *source;
uchar *dest;
{
        char    byte_val[5];    /* e.g. "0x5F\0"        */
        int     byte_count = 0;
        uchar   tmp_val;
        char    *end_ptr;

        strcpy( byte_val, "0x00" );

        if( *source == '\0' )   /* Accept empty string as legal */
                return 0;

        if( *source++ != '0' )
                return (E_BADATTR);
        if( tolower(*source++) != 'x' )
                return (E_BADATTR);

        while( ( byte_val[2] = *source ) && ( byte_val[3] = *(source+1) ) )
        {
                source += 2;

                /* be careful not to store illegal bytes in case the
                 * destination is of exact size, and the source has
                 * trailing blanks
                 */

                tmp_val = (uchar) strtoul( byte_val, &end_ptr, 0 );
                if( end_ptr != &byte_val[4] )
                        break;
                *dest++ = tmp_val;
                byte_count++;
        }

        return (byte_count);
}

/*
 * NAME: generate_minor
 *
 * FUNCTION: Device dependent routine for generating the device minor number
 *
 * RETURNS:
 *   minor number success
 *   E_MINORNO on failure
 */

int
generate_minor(lname, majno, minorno)
char    *lname;     /* logical device name */
long    majno;      /* device major number */
long    *minorno;   /* device minor number */
{
        long    *minorptr;

        /*
         * use genminor() to create and reserve the minor
         * numbers used by this device.
         */

        minorptr = genminor(lname, majno, -1, 1, 1, 1);

        if ( minorptr == (long *)NULL )
                /* error generating minor number */
                return(E_MINORNO);

        *minorno = *minorptr;
        return(E_OK);
}


/*
 * NAME: make_special_files
 *
 * FUNCTION:
 *      This routine creates whatever special files are needed for
 *      the Sandpiper V diagnostic device driver.
 *
 * RETURNS:
 *   0 - success
 *   positive return code on failure
 */

int
make_special_files(lname, devno)
char    *lname;         /* logical device name */
dev_t devno;            /* major/minor number */
{
        struct stat buf;
        char deventry[32];
        extern int      errno;

        /*
         *      Check to see if /dev/xxxx exists, and has the correct
         *      device numbers.  If it does not exist, or does not have
         *      the correct major/minor device number, then remove it (unlink)
         *      and recreate it.
         */

        sprintf(deventry,"/dev/%s",lname);
        if (stat(deventry, &buf) == 0)
        {
           if (buf.st_dev != devno)
              unlink(deventry);
           else
              return(E_OK);
        }

        if (mknod(deventry, S_IFMPX | S_IFCHR | 00666 , devno) == -1)
           return(E_MKSPECIAL);
        else
           return(E_OK);

}

/*
 * NAME: err_undo
 *
 * FUNCTION: Terminates the device.  Used to back out on an error.
 *
 * NOTES:
 *
 * void
 *   err_undo( devno )
 *      devno = The device's devno.
 *
 * RETURNS:
 *               None
 */
err_undo(devno)
dev_t   devno;                          /* The device's devno */
{
        struct cfg_dd cfg;              /* sysconfig command structure */

        /* set up structure to terminate the device */
        cfg.devno = devno;
        cfg.kmid = (mid_t)0;
        cfg.ddsptr = (caddr_t) NULL;
        cfg.ddslen = (int)0;
        cfg.cmd = CFG_TERM;

        /* call sysconfig to terminate the device */
        sysconfig(SYS_CFGDD, &cfg, sizeof(struct cfg_dd ));
        return;
}

