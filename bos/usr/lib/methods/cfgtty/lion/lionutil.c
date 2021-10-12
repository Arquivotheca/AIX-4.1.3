#ifndef lint
static char sccsid[] = "@(#)82 1.3 src/bos/usr/lib/methods/cfgtty/lion/lionutil.c, cfgtty, bos41J, 9521B_all 5/26/95 07:49:32";
#endif
/*
 * COMPONENT_NAME: (CFGTTY) 
 *
 * FUNCTIONS: Run_Other_Method
 *            Generate_Minor
 *            Make_Special_Files
 *            Do_Other_Processing
 *            gen_lion_minor
 *            gen_std_minor
 *
 * ORIGINS: 83
 *
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

#include <stdio.h>          /* standard I/O */
#include <cf.h>             /* error messages */
#include <errno.h>          /* standard error numbers */
#include <fcntl.h>          /* logical file system #define */

#include <sys/mode.h>
#include <sys/cfgdb.h>      /* config #define */
#include <sys/sysmacros.h>
#include <sys/sysconfig.h>
#include <sys/device.h>
#include <sys/cfgodm.h>     /* config structures */

#include <sys/str_tty.h>

#include "ttycfg.h"
#include "cfgdebug.h"

/*
 * ==============================================================================
 * defines and strutures
 * ==============================================================================
 */
#define MKNOD_FLAGS   S_IFCHR|S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH

/* To build LION_DRIVER minor number */
#define LION_VTERM(a) ((a) | 0x40)
#define LION_XPAR(a)  ((a) | 0x80)

/* To build LION_DRIVER special files names */
#define VTERM_PREFIX  "v"
#define XPAR_PREFIX   "x"

/*
 * =============================================================================
 *                       FUNCTIONS USED BY CONFIGURATION METHOD
 * =============================================================================
 * 
 * All these functions are used by the streams based TTY configuration method.
 *
 * =============================================================================
 */

int Run_Other_Method(cusdev,parcusdev,buscusdev,parpredev,attrList,ipl_stat)
struct CuDv      *cusdev;     /* customized device data  */
struct CuDv      *parcusdev;  /* parent device customized data  */
struct CuDv      *buscusdev;  /* bus customized data  */
struct PdDv      *parpredev;  /* parent predefined data  */
struct attr_list *attrList;   /* list of attributes  */
int              *ipl_stat;   /* phase of ipl  */
{ return(0); }


int Do_Other_Processing(cusdev,parcusdev,buscusdev,parpredev,attrList)
struct CuDv      *cusdev;     /* customized device data  */
struct CuDv      *parcusdev;  /* parent device customized data  */
struct CuDv      *buscusdev;  /* bus customized data  */
struct PdDv      *parpredev;  /* parent predefined data  */
struct attr_list *attrList;   /* list of attributes  */
{ return(0); }


/*
 * -----------------------------------------------------------------------------
 *                       Generate_Minor
 * -----------------------------------------------------------------------------
 * This function generates the minor number(s) for the tty devices.
 *
 * First, we try to get existing (in ODM database) minors. If none are found,
 * we generate minor numbers and record them in the ODM CuDvDr database.
 *
 * Notes: Minor number is generated for the tty device being configured by
 * applying a special algorithm. The minor number is set in the database
 * by calling genminor() with the minor number to be used.
 *
 * Return code: Exits with 0 on success, ODM error code otherwise.
 * -----------------------------------------------------------------------------
 */
int Generate_Minor(struct CuDv *devcusobj, struct CuDv *parcusobj,
		   struct CuDv *buscusobj, char *drvrname,
                   long maj_no, long **minorP)
{

	extern int gen_std_minor();
	long *minor_list;
	int  count;
	int  rc;

	DEBUG_0("Generate_Minor: entered for LION driver\n");

	/* see if we already have a minor number */
	if ((minor_list = getminor(maj_no,&count,devcusobj->name)) == NULL) {

		/*
		 * no minor numbers found for this device
		 * call the appropriate routine to create
		 * a minor number
		 */

		DEBUG_1("gen_min: no minors found for %s\n",devcusobj->name);

			DEBUG_0("gen_min: calling gen_lion_minor()\n");
			rc = gen_lion_minor(devcusobj,parcusobj,buscusobj,
						 maj_no,&minor_list);
		if (rc > 0){
                        /* error generating minor number */
                        DEBUG_1("gen_lion_minor failed, rc=%d\n",rc);
                        return(rc);
                }
                if (rc == -1){
                        DEBUG_0("gen_lion_minor: return -1\n");
                        /* minor number  in use */
                        *minorP = minor_list;
                        return(rc);
                }
	}

	DEBUG_1("Generate_Minor: return OK, minor=%d\n",*minor_list);

	*minorP = minor_list;
	return(0);

} /* end Generate_Minor(...) */


int gen_lion_minor(struct CuDv *cusobj,struct CuDv *parobj,struct CuDv *busobj,
		   long major, long **minP)
{
	static long minor_array[3];
	long   *minor_list;
	int    rc;

	DEBUG_3("gen_lion_minor: name=%s parent=%s major=%d\n",
		cusobj->name,parobj->name,major);

	/* call gen_std_minor to get the minor number for the normal tty */
	if ((rc = gen_std_minor(cusobj,parobj,busobj,major,&minor_list))) {

		/* failed to get the std minor number; return error */
		DEBUG_1("gen_lion_minor: gen_std_minor failed, rc=%d\n",rc);
		return(rc);

	}
	minor_array[0] = *minor_list;

	/*
	 * use the std minor number as the basis for the 
	 * virtual and transparent device minor numbers
	 */
	
	/* get the VTERM minor number */
	minor_array[1] = LION_VTERM(minor_array[0]);
	if (!(minor_list = genminor(cusobj->name,major,minor_array[1],1,1,1))){

	    /* see if we can steal this minor number */
	    if ((rc=steal_minor(cusobj->name,major,minor_array[1],&minor_list))!=0) {
		/*
		 * failed to get this minor number
		 * release any we do have and return error
		 */
		DEBUG_2("gen_lion_minor: error setting VTERM, dev=%s min=%d\n",
			cusobj->name,minor_array[1]);
		reldevno(cusobj->name,FALSE);
		return(rc);
	    }

	}

	/* get the XPAR minor number */
	minor_array[2] = LION_XPAR(minor_array[0]);
	if (!(minor_list = genminor(cusobj->name,major,minor_array[2],1,1,1))){

	    /* see if we can steal this minor number */
	    if ((rc=steal_minor(cusobj->name,major,minor_array[2],&minor_list))!=0) {
		/*
		 * failed to get this minor number
		 * release any we do have and return error
		 */
		DEBUG_2("gen_lion_minor: error setting XPAR, dev=%s min=%d\n",
			cusobj->name,minor_array[2]);
		reldevno(cusobj->name,FALSE);
		return(rc);
	    }

	}

	/*
	 * generated all minor numbers succesfully
	 * point caller to minor numbers and return OK
	 */
	DEBUG_3("gen_lion_minor: min 1 = %d, min 2 = %d, min 3 = %d\n",
		minor_array[0],minor_array[1],minor_array[2]);
	*minP = minor_array;
	return(0);

} /* end gen_lion_minor(...) */


/*
 * -----------------------------------------------------------------------------
 *                       Make_Special_Files
 * -----------------------------------------------------------------------------
 * This function makes the special files for the tty devices.
 *
 * This function operates as a device dependent subroutine called 
 * by the configure method for all devices.
 *
 * Return code: Exits with 0 on success, ODM error code otherwise.
 * -----------------------------------------------------------------------------
 */
int Make_Special_Files(logicalName, driverName, majorNo, minorPtr)
char * logicalName;
char * driverName;
long   majorNo;
long * minorPtr;
{
    int    rc;
    dev_t  devno;
    char   file_name[ATTRVALSIZE];

    DEBUG_4("Make_Special_Files: lname=%s drvr=%s major=%d minor=%d\n",
	    logicalName,driverName,majorNo,*minorPtr);


    /* ================================================= */
    /* Make special files for device on 64 port adapters */
    /* Three special files are needed */
    /* ================================================= */


    /* "main" special file */
    devno = makedev(majorNo, *minorPtr);
    if ((rc = mk_sp_file(devno, logicalName, (long) MKNOD_FLAGS)) != 0) {
           DEBUG_1("Make_Special_Files: Error creating %s\n", logicalName);
          return (E_MKSPECIAL);
    }

    /* "vterm" special file */
    sprintf(file_name, "%s%s", VTERM_PREFIX, logicalName);
    minorPtr++;
    devno = makedev(majorNo, *minorPtr);
    if ((rc = mk_sp_file(devno, file_name, (long) MKNOD_FLAGS)) != 0) {
            DEBUG_1("Make_Special_Files: Error creating %s\n", file_name);
            return (E_MKSPECIAL);
    }

    /* "xpar" special file */
    sprintf(file_name, "%s%s", XPAR_PREFIX, logicalName);
    minorPtr++;
    devno = makedev(majorNo, *minorPtr);
    if ((rc = mk_sp_file(devno, file_name, (long) MKNOD_FLAGS)) != 0) {
           DEBUG_1("Make_Special_Files: Error creating %s\n", file_name);
            return (E_MKSPECIAL);
    }

    /* That's OK */
    return(0);

} /* End int Make_Special_Files(...) */

/*
 * -----------------------------------------------------------------------------
 *                       STEAL_MINOR
 * -----------------------------------------------------------------------------
 *
 * This function is called by any of the tty/lp specific minor number creation
 * routines when a genminor fails.
 *
 * This function determines if another tty or lp already having the requested
 * minor number was the reason for the failure. If this is the case it attempts
 * to "steal" the minor number by performing an odm_change of the CuDvDr
 * object. If this is successful, then the current tty now owns the minor #.
 *
 * Really the only case where this can happen (and work) is when there are two
 * devices defined for a given port and the device that the system first tries
 * to configure at reboot is not the one that had the entry in  CuDvDr when
 * the system was shutdown.
 *
 * This function returns 0 if successful. In addition, it sets the minP
 * parameter to point to the minor number requested. An odmerrno is returned
 * if the function fails.
 *
 * -----------------------------------------------------------------------------
 */
int steal_minor(char *lname, long major, long minor, long **minP)
{
   char   sstring[256];
   struct CuDvDr dvdrobj;
   static long min;
   int    rc;

   /* find the object that currently has this devno */
   sprintf(sstring,"resource = 'devno' and value1 = '%ld' and value2 = '%ld'",
	   major,minor);
   if ((rc = (int)odm_get_obj(CuDvDr_CLASS,sstring,&dvdrobj,ODM_FIRST)) == 0) {

	/* No object */
	DEBUG_1("steal_minor: CuDvDr crit=%s found no objects.\n", sstring);
	return E_NOCuOBJ;

    } else if (rc == -1) {

	/* odm error occurred */
	DEBUG_1("steal_minor: get_obj failed, crit = %s.\n",sstring);
        return E_ODMGET;

    }

    DEBUG_1 ("steal_minor: Conflict is with %s\n",dvdrobj.value3)

    /* determine if the device is a tty or serial printer */
    if ((!strncmp(dvdrobj.value3,"tty",3)) ||
        (!strncmp(dvdrobj.value3,"lp",2)) ) {

	/* the device is a tty/printer; attempt to steal the minor number */
	strcpy (dvdrobj.value3,lname);
	if ((rc=odm_change_obj(CuDvDr_CLASS,&dvdrobj))<0) {
	    /* error changing CuDvDr */
	    DEBUG_0 ("steal_minor: odm_change_obj failed\n");
	    return E_ODMUPDATE;
	}

	/*
	 * successfully stole the minor number
	 * set minP to point to the minor number for caller to use
	 * and return with successful return code
	 */
        min = minor;
        *minP = &min;
        return(0);

    }

    /* minor number held by non-tty/non-lp device; return error */
    return E_MINORNO;

}

int gen_std_minor(struct CuDv *cusobj, struct CuDv *parobj, struct CuDv *busobj,
                  long major, long **minP)
{
    extern long *genminor();    /* generate minor number service */
    extern int get_slot();      /* returns slot location */
    extern int steal_minor();   /* handle genminor failure */
    extern int Get_Parent_Bus();/* searches up the device hierarchy until */
                                /* it finds a device with class bus */

    long * minor_ptr;           /* pointer to minor number returned */
    long   minor_no;            /* generated minor number */
    int    rc;                  /* return code, minor number set */
    int    slot;                /* slot number of parent(adapter) */
    int    port;                /* port number of tty on parent */
    ulong  busid ;              /* bus id that tty is attached to */
    int    how_many;            /* How many attributes are found */

                                /* ODM structures declarations */
    struct CuAt *  current_att;     /* current found attribute */


    DEBUG_3("gen_std_minor: name=%s parent=%s major=%d\n",
            cusobj->name,parobj->name,major);

    /* ============================================================= */
    /* Get port number - Used to tell if configuring sio port A, B or C */
    /* ============================================================= */
    port = atoi(&(cusobj->connwhere[strcspn(cusobj->connwhere,"0123456789")]));

    /* Decrement port number for sio ports */
    /* So they are 0,1,2 instead of 1,2 */
    if (cusobj->connwhere[0] == 's') {
        port--;
    }

    /* =============== */
    /* Get slot number */
    /* =============== */
    if ((rc = get_slot(parobj->connwhere, &slot)) != 0) {
        return(rc);
    }
    slot = (slot-1) & 0x0f;
    DEBUG_2("gen_std_minor: slot = %d, port = %d\n", slot, port);



    /* Get bus id */
    if ((current_att = getattr(busobj->name, "bus_id", FALSE, &how_many))
                             == NULL) {
        return(rc);
    }
    /*convert values to type ulong*/
    busid = strtoul(current_att->value, (char ** )NULL, 0);
    DEBUG_1("generate_minor: busid = %ld\n", busid);
    DEBUG_1("generate_minor: current_att->value = %s\n", current_att->value);
    busid = (((busid >> 20) & 0xff) - 0x20);

    /* Generate minor number */
    minor_no = (long) ((busid <<14) | (((slot+1)&0xf)<<8) | port);

    DEBUG_1("generate_minor: minor_no = %ld\n", minor_no);

    /* ========================================= */
    /* Set minor number for tty port in database */
    /* ========================================= */
    DEBUG_0("gen_std_minor: Trying to set in database:\n");
    DEBUG_3("gen_std_minor: deviceName = %s, major= %ld, minor_no = %ld\n",
             cusobj->name, major, minor_no);

    if ((minor_ptr=genminor(cusobj->name, major, minor_no, 1, 1, 1)) == NULL) {
        /* Failed to set minor number requested in CuDvDr Class */
        DEBUG_1("gen_std_minor: genminor failed minor=%ld\n",minor_no);

        /* see if we can steal this minor number */
        if ((rc = steal_minor(cusobj->name, major, minor_no, &minor_ptr)) != 0)
            return(rc);
    }

    /*
     * generated the minor number successfully
     * point the caller to the number and return OK
     */

    *minP = minor_ptr;
    return(0);
} /* End int gen_std_minor(...) */



