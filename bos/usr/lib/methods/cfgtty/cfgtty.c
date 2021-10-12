#ifndef lint
static char sccsid[] = "@(#)42 1.15.1.17 src/bos/usr/lib/methods/cfgtty/cfgtty.c, cfgtty, bos41J, 9525G_all 6/21/95 18:04:18";
#endif
/*
 * COMPONENT_NAME: (CFGTTY) Configure Method for streams based tty
 *
 * FUNCTIONS: main, err_exit, err_undo1, err_undo2
 *
 * ORIGINS: 27, 83
 *
 */
/*
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

#include <stdio.h>          /* standard I/O */
#include <stdlib.h>         /* standard C library */
#include <sys/types.h>      /* standard typedef */
#include <cf.h>             /* error messages */
#include <malloc.h>         /* struct mallinfo */
#include <errno.h>          /* standard error numbers */
#include <fcntl.h>          /* logical file system #define */
#include <string.h>
#include <sys/ldr.h>

#include <sys/cfgdb.h>      /* config #define */
#include <sys/sysmacros.h>
#include <sys/sysconfig.h>
#include <sys/device.h>
#include <sys/cfgodm.h>     /* config structures */
#include <termios.h>
 
#include <sys/termiox.h>
#include <sys/stropts.h>    /* Streams #define */
#include <sys/str_tty.h>

#include "cfgdebug.h"
#include "ttycfg.h"
#include "pparms.h"

/*
 * ==============================================================================
 * External functions declarations
 * ==============================================================================
 */
extern mid_t loadext();

/*
 * =============================================================================
 *                       LOCAL UTILITY ROUTINES
 * =============================================================================
 */
/*
 * -----------------------------------------------------------------------------
 *                       ERR_EXIT
 * -----------------------------------------------------------------------------
 * 
 * Closes any open object classes and terminates ODM.
 * Used to back out on an error.
 *
 * This routine is to be used only within this file.
 * The device specific routines for the various device
 * specific config methods  must not call this function.
 *
 * Return code: None
 * -----------------------------------------------------------------------------
 */
static void err_exit(exitcode)
int    exitcode;
{
    /* Close any open object class */
    odm_close_class(CuDv_CLASS);
    odm_close_class(PdDv_CLASS);
    odm_close_class(CuAt_CLASS);
    odm_close_class(PdAt_CLASS);

    /* Terminate the ODM */
    odm_terminate();
    exit(exitcode);
} /* End static void err_exit(...) */

/*
 * -----------------------------------------------------------------------------
 *                       ERR_UNDO1
 * -----------------------------------------------------------------------------
 *
 * Terminates the device.  Used to back out on an error.
 *
 * This routine is to be used only within this file.  The device
 * specific routines for the various device specific config methods
 * must not call this function.
 *
 * Return code: None
 * -----------------------------------------------------------------------------
 */

static void err_undo1(cfgPtr, whichDds)
struct cfg_dd * cfgPtr;         /* pointer to cfg structure */
enum dds_type * whichDds;       /* dds type */
{
    /* terminate device */
    /* kmid not set to 0 */
    cfgPtr->ddsptr = (caddr_t) whichDds;
    cfgPtr->ddslen = sizeof(*whichDds);
    cfgPtr->cmd = CFG_TERM;

    if (sysconfig(SYS_CFGDD, cfgPtr, sizeof(struct cfg_dd)) == -1) {
        DEBUG_0("cfgtty: error unconfiguring device\n");
    };
} /* End static void err_undo1(...) */

/*
 * -----------------------------------------------------------------------------
 *                       ERR_UNDO2
 * -----------------------------------------------------------------------------
 *
 * Terminates the device and unloads the driver, if the device has
 * a driver.  If the device does not have a driver, it simply
 * returns.  
 *
 * This routine is to be used only within this file.  The device
 * specific routines for the various device specific config methods
 * must not call this function.
 *
 * Return code: None
 * -----------------------------------------------------------------------------
 */

static void err_undo2(cfgPtr, majorNo, minorNo, whichDds)
struct cfg_dd * cfgPtr;         /* pointer to cfg structure */
long   majorNo, minorNo;        /* major and minor numbers */
enum dds_type * whichDds;       /* dds type */
{
    extern int    pop_modules();     /* similar acion as autopush command */

    if (pop_modules(majorNo, minorNo)) {
        DEBUG_0("cfgtty: modules pop failed.\n");
    };
    err_undo1(cfgPtr, whichDds); /* terminate device */
} /* End static void err_undo2(...) */

/*
 * =============================================================================
 *                       MAIN
 * =============================================================================
 * 
 * This process is executed to "configure" a tty device in streams environment.
 * 
 * The termios and termiox are built in this function
 * and given (if necessary) to DDS built functions.
 *
 * This process is invoked when a tty device instance is to be 
 * "configured". The invokation will be made by a high level
 * configuration command or by the configuration manager at IPL.
 *
 * Interface: cfgtty -l <logical_name> <phase>
 *
 * Return code: Exits with 0 on success, ODM error code otherwise.
 * =============================================================================
 */

main(argc,argv,envp)
     int    argc;
     char    *argv[];
     char    *envp[];
{
    extern int    push_modules();    /* similar acion as autopush command */
    extern long * getminor();        /* gets the minor numbers */
    extern long   generate_minor();  /* generates minor number */
    extern long   genmajor();        /* generates major number */
    extern char * optarg;            /* for getopt function */
    extern int set_termiox();        /* build of termiox structure */
    extern int set_termios();        /* build of termios structure */
    extern int set_inittab();        /* Update of initab file */
    extern int getatt();  
    

    char * logical_name;              /* logical name to configure */
    char   sstring[256];              /* search criteria pointer */
    char   modules[ATTRVALSIZE];      /* modules to push for this tty */
    char   login[ATTRVALSIZE];        /* login attribute */
    char   load_mod[ATTRVALSIZE];     /* load module path */

    long   majorno, minorno;          /* major and minor numbers */
    long * minor_ptr;                 /* pointer to minor number */
    int    ipl_phase;                 /* ipl phase */
    int    return_code;               /* return code and counter */
    int    current_opt, option_error; /* used in parsing parameters */
    enum   dds_type which_dds;        /* used to unconfigured drivers */
    int  scratch;                     /* scratch value for getatt*/

    struct cfg_dd  cfg;               /* sysconfig command structure for dd */
    struct termios built_termios;     /* termios storage */
    struct termiox built_termiox;     /* termiox storage */
    struct  attr_list * attrib_list;  /* attribute list            */


    /* ODM structures declarations */
    struct Class * cus_dev_class;     /* customized devices class ptr */
    struct Class * pre_dev_class;     /* predefined devices class ptr */
    struct Class * cus_att_class;     /* customized attribute class ptr */
    struct Class * pre_att_class;     /* predefined attribute class ptr */

    struct CuDv    cus_dev;           /* customized device object storage */
    struct PdDv    pre_dev;           /* predefined device object storage */
    struct PdDv    par_pre_dev; /* predefined device parent object storage */
    struct CuDv    par_cus_dev; /* customized parent device object storage */
    struct CuDv    bus_cus_dev;       /* customized bus object storage */

    struct CuDv     dmy_dev;          /* customized device object storage */
    struct PdAt     dmy_dev2;         /* temp device object storage */

    int     (*loadrc)(char *,uint,char *);


    /* ============================== */
    /* set up initial variable values */
    /* ============================== */
    ipl_phase = RUNTIME_CFG;
    option_error = 0;
    logical_name = NULL;

    /* ================ */
    /* Parse Parameters */
    /* ================ */
    while ((current_opt = getopt(argc,argv,"l:12")) != EOF) {
        switch (current_opt) {
          case 'l':
            if (logical_name != NULL) {
                option_error++;
            } 
            logical_name = optarg;
            break;
            
          case '1':
            if (ipl_phase != RUNTIME_CFG) {
                option_error++;
            } 
            ipl_phase = PHASE1;
            break;
            
          case '2':
            if (ipl_phase != RUNTIME_CFG) {
                option_error++;
            } 
            ipl_phase = PHASE2;
            break;
            
          default:
            option_error++;
        }
    }
    if (option_error) {
        /* error parsing parameters */
        DEBUG_0("cfgtty: command line error\n");
        exit(E_ARGS);
    } 

    /* =========================================================== */
    /* Check command-line parameters for validity                  */
    /* For config methods this includes making sure that a logical */
    /* name was specified, and that both phase flags were not set. */
    /* The customized devices object class must also be searched   */
    /* to see if the logical name is valid.                        */
    /* =========================================================== */
    if (logical_name == NULL) {
        DEBUG_0("cfgtty: no logical name\n");
        exit(E_LNAME);
    } 

    /* start up odm */
    if (odm_initialize() == -1) {
        DEBUG_0("cfgtty: bad odm initialization\n");
        exit(E_ODMINIT);
    } 

    /* open customized devices object class (CuDv) */
    if ((int)(cus_dev_class = odm_open_class(CuDv_CLASS)) == -1) {
        DEBUG_0("cfgtty: CuDv open class error\n");
        err_exit(E_ODMOPEN);
    } 

    /* ================================================= */
    /* Get the Customized Devices Object for this device */
    /* ================================================= */
    sprintf(sstring, "name = '%s'", logical_name);
    return_code = (int)odm_get_obj(cus_dev_class, sstring, &cus_dev, ODM_FIRST);
    switch (return_code) {
      case 0: /* odm objects not found */
        DEBUG_1("cfgtty: CuDv %s found no objects.\n",sstring);
        err_exit(E_NOCuDv);
        break;  /* To avoid bug if err_exit is suppressed */
        
      case -1: /* odm error occurred */
        DEBUG_1("cfgtty: get_obj failed, %s.\n", sstring);
        err_exit(E_ODMGET);
        break;  /* To avoid bug if err_exit is suppressed */
        
      default: /* odm object found ==> That's OK */
        DEBUG_1("cfgtty: CuDv %s found objects.\n",sstring);
    } /* End switch (return_code) */

    /* ================================================================= */
    /* Get the predefined object for this device. This object is located */
    /* searching the predefined devices object class based on the unique */
    /* type link descriptor in the customized device. */
    /* ================================================================= */

    /* open predefined devices object class (PdDv) */
    if ((int)(pre_dev_class = odm_open_class(PdDv_CLASS)) == -1) {
        DEBUG_0("cfgtty: PdDv open class error\n");
        err_exit(E_ODMOPEN);
    } 

    /* search Predefined devices for object with this unique type */
    sprintf(sstring, "uniquetype = '%s'", cus_dev.PdDvLn_Lvalue);
    return_code = (int)odm_get_obj(pre_dev_class, sstring, &pre_dev, ODM_FIRST);
    switch (return_code) {
      case 0: /* odm objects not found */
        DEBUG_1("cfgtty: PdDv crit=%s found no objects.\n",sstring);
        err_exit(E_NOPdDv);
        break;  /* To avoid bug if err_exit is suppressed */
        
      case -1: /* odm error occurred */
        DEBUG_1("cfgtty: get_obj failed, crit=%s.\n",sstring);
        err_exit(E_ODMGET);
        break;  /* To avoid bug if err_exit is suppressed */
        
      default:  /* odm object found ==> That's OK */
        DEBUG_1("cfgtty: PdDv uniquetype = %s found objects.\n",
                 cus_dev.PdDvLn_Lvalue);
    } /* End switch (return_code) */

     

    /* ============================================================ */
    /* If this device is being configured during an ipl phase, then */
    /* display this device's LED value on the system LEDs. */
    /* ============================================================ */
    if (ipl_phase != RUNTIME_CFG) {
        /* display LED value */
        setleds(pre_dev.led);
    } 

    /* ============================================================ */    
    /* Check to see if the device is already configured. */
    /*  We actually go about the business of configuring the device */
    /*  only if the device is not configured yet i.e. DEFINED. */
    /* ============================================================ */    
    if (cus_dev.status == (short) DEFINED) {

        /* ==================================================== */        
        /* The device is not available to the system yet. Now */
        /* check to make sure that the device's relations will */
        /* allow it to be configured. In particular, make sure */
        /* that the parent is configured (AVAILABLE), and that */
        /* no other devices are configured at the same location. */
        /* Get the device's parent object */
        /* ==================================================== */        
        sprintf(sstring, "name = '%s'", cus_dev.parent);
        return_code = (int)odm_get_obj(cus_dev_class, sstring, &par_cus_dev, ODM_FIRST);
        switch (return_code) {
          case 0: /* odm objects not found */
            DEBUG_1("cfgtty: CuDv %s found no objects.\n",
                     sstring);
            err_exit(E_NOCuDvPARENT);
            break;  /* To avoid bug if err_exit is suppressed */
            
          case -1: /* odm error occurred */
            DEBUG_1("cfgtty: get_obj failed, %s.\n",
                     sstring);
            err_exit(E_ODMGET);
            break;  /* To avoid bug if err_exit is suppressed */

            default : /* odm object found ==> That's OK */
            DEBUG_1("cfgtty: CuDv %s found objects.\n",
                     sstring);
        } /* End switch (return_code) */
        if (par_cus_dev.status != (short) AVAILABLE) {
            DEBUG_1("cfgtty: parent %s is not AVAILABLE\n",
                            cus_dev.parent);
            err_exit(E_PARENTSTATE);
        }
        
        /* Make sure that no devices are config'd at this location */
        sprintf(sstring,
                "parent = '%s' AND connwhere = '%s' AND status = %d",
                cus_dev.parent,cus_dev.connwhere,AVAILABLE);
        return_code = (int)odm_get_obj(cus_dev_class,sstring,&dmy_dev,ODM_FIRST);
        switch (return_code) {
          case 0: /* odm objects not found ==> That's OK */
            DEBUG_1("cfgtty: No CuDv %s configured at this location.\n",
                     sstring);
            break;

          case -1: /* odm error occurred */
            DEBUG_1("cfgtty: get_obj %s failed\n", sstring);
            err_exit(E_ODMGET);
            break;  /* To avoid bug if err_exit is suppressed */

          default: /* Error: device configured at this location */
            DEBUG_1("cfgtty: %s : Device is configured at this location\n",
                    sstring);
            err_exit(E_AVAILCONNECT);
        } /* End switch (return_code) */

        /* ========================================== */
        /* Get the parent's Predefined Devices Object */
        /* ========================================== */
        sprintf(sstring, "uniquetype = '%s'", par_cus_dev.PdDvLn_Lvalue);
        return_code = (int)odm_get_obj(pre_dev_class,sstring,&par_pre_dev,ODM_FIRST);
        switch (return_code) {
          case 0: /* odm objects not found */
            DEBUG_1("cfgtty: PdDv %s found no objects.\n",
                     sstring);
            err_exit(E_NOPdOBJ);
            break;  /* To avoid bug if err_exit is suppressed */
            
          case -1 : /* odm error occurred */
            DEBUG_1("cfgtty: get_obj failed, %s.\n",
                     sstring);
            err_exit(E_ODMGET);
            break;  /* To avoid bug if err_exit is suppressed */
            
          default: /* odm objects found ==> That's OK */
            DEBUG_1("cfgtty: PdDv %s found.\n",
                     sstring);
        }
        
        /* =================================================== */
        /* Get the driver for the tty from the parent(adapter) */
        /* predefined object field. */
        /* Get the modules to push from this tty customized */
        /* attribute object class. */
        /* =================================================== */
        if (!strcmp(par_pre_dev.DvDr, "")) {
                DEBUG_1("cfgtty: No driver found for %s\n",
                         par_pre_dev.uniquetype);
                err_exit(E_INVATTR);
        } 

        if ((int)(cus_att_class = odm_open_class(CuAt_CLASS)) == -1) {
            DEBUG_0("cfgtty: CuAt open class error\n");
            err_exit(E_ODMOPEN);
        } 
        
        if ((int)(pre_att_class = odm_open_class(PdAt_CLASS)) == -1) {
            DEBUG_0("cfgtty: PdAt open class error\n");
            err_exit(E_ODMOPEN);
        } 
       

    	/* ============================================================ */    
        /* Get list of attributes                                       */
    	/* ============================================================ */    

	attrib_list = (struct attr_list *) get_attr_list_e(logical_name,
                             pre_dev.uniquetype, par_pre_dev.uniquetype,
                             &scratch,40);

	if ((struct attr_list *) attrib_list == NULL) {
                DEBUG_0 ("cfgtty : failed to get list\n");
		err_exit(scratch);
	}
        


    	/* ============================================================ */    
        /* get the parent bus object for later use by genminor          */
    	/* ============================================================ */    
	if ((return_code = Get_Parent_Bus(cus_dev_class,&par_cus_dev.parent,
				 &bus_cus_dev)) != 0) {
              DEBUG_1("cfgtty: get parent bus object failed; return_code = %d\n"
                                       , return_code);
       		err_exit(return_code);
    	}

    	/* ============================================================ */    
        /* get module attributes for pushing modules later*/
    	/* ============================================================ */    
  	if (return_code = (getatt(attrib_list,MODULES_ATT,&modules,
        	          's', &scratch)) != 0) {
             DEBUG_3("cfgtty: getatt '%s' for %s fails with error %x\n",
                     MODULES_ATT, cus_dev.name, return_code);
             err_exit(E_NOATTR);
        }
    	/* ============================================================ */    
    	/* Check to see if a load module exists                         */
    	/* ============================================================ */    
  	if (return_code = (getatt(attrib_list,LOAD_MODULE,&load_mod,
        	          's', &scratch)) != 0) {
             DEBUG_3("cfgtty: getatt '%s' for %s fails with error %x\n",
                     LOAD_MODULE, cus_dev.name, return_code);
             err_exit(E_NOATTR);
        }
	
    	/* ============================================================ */    
    	/* Load the module                                              */
    	/* ============================================================ */    
     	if (load_mod[0] != '\0') {

                if (!(loadrc = load(load_mod,0,NULL))) {
                   fprintf(stderr,"     %s: %s\n",LOAD_MODULE,load_mod);
                   err_exit(E_ATTRVAL);
                }
                if (loadbind(0,(void *)main,(void *)loadrc) == -1) {
                   fprintf(stderr,"     %s: %s\n",LOAD_MODULE,load_mod);
                   err_exit(E_ATTRVAL);
                }

 	}

        /* ========================================================= */
        /* Skip the rest of this code by running a different method  */
        /* -1 = success , exit                                       */
        /*  0 = no operation was performed, or preprocessing ok      */
        /* ========================================================= */

        return_code = Run_Other_Method(&cus_dev, &par_cus_dev, &bus_cus_dev,
                                    &par_pre_dev, attrib_list, &ipl_phase); 
	if (return_code > 0) {
            DEBUG_1("cfgtty: Run_Other_Method failed, rc=%d\n",return_code);
            err_exit(return_code);
        }
        else if (return_code == -1){
             /* success */
    	     err_exit(E_OK);
        }

        /* ==============================================  */
        /* Call loadext() to test if the driver is loaded. */
        /* It must have been by parent adapter             */
        /* ==============================================  */
        DEBUG_1 ("cfgtty: Testing if %s is loaded\n", par_pre_dev.DvDr);
        if ((cfg.kmid = loadext(par_pre_dev.DvDr, FALSE, TRUE))
							 == (mid_t)NULL) {
            DEBUG_1("cfgtty: The '%s' driver has not been loaded\n",
                    par_pre_dev.DvDr);
            err_exit(E_LOADEXT);
        }
        
        /* ================================================ */
        /* Call genmajor() to get a major number for        */
        /* this device. The major number has been generated */
        /* during adapter configuration                     */
        /* ================================================ */
        if ((majorno = genmajor(par_pre_dev.DvDr)) == (long) -1) {
            /* error allocating major number */
            err_exit(E_MAJORNO);
        }
        
        /* ===================================== */
        /* We generate or retrieve minor numbers */
        /* ===================================== */

        return_code = Generate_Minor(&cus_dev, &par_cus_dev,
				 &bus_cus_dev, par_pre_dev.DvDr,majorno,
				&minor_ptr); 
        if (return_code > 0) { 
            DEBUG_1("cfgtty: Generate_Minor failed, rc=%d\n",return_code);
            /* first release minor number */
            err_exit(return_code);
        }
        if (return_code == -1) { 
	    minorno = *minor_ptr;
            /* see if we can steal this minor number */
            DEBUG_0("cfgtty: Generate_Minor attempt to steal minor\n" );
            if ((return_code = steal_minor(cus_dev.name, majorno, minorno,
                                                           &minor_ptr)) != 0)
            	err_exit(return_code);
        }


	/* Even if we have several minor numbers (LION_DRIVER), we're using */
	/* the first number for devno */
	/* For LION_DRIVER, the "main" minor is the lowest minor */
	/* and the 'generate_minor' routine returns minor numbers */
	/* sorted in ascending order */

	minorno = *minor_ptr;
        
        /* ============================ */
        /* create devno for this device */
        /* ============================ */
        cfg.devno = makedev(majorno,minorno);
       

 
        /* ================================================ */
        /* Now make the special files that this device will */
        /* be accessed through.                             */
        /* ================================================ */
        if ((return_code = Make_Special_Files(cus_dev.name, par_pre_dev.DvDr
                            ,majorno, minor_ptr)) != 0) {
            /* error making special files */
            DEBUG_0("cfgtty: Make_Special_Files failed.\n");
            err_exit(return_code);
        }

        /* ================================================ */
        /* Build of termios and termiox structures          */
        /* These structures are built here because they are */
        /* used by several drivers                          */
        /* ================================================ */

        if (return_code = set_termios(&cus_dev, &built_termios,attrib_list)) {
            DEBUG_0("cfgtty: Build of termios failed\n");
            err_exit(return_code);
        }
        else {
            DEBUG_0("cfgtty: Build of termios succeeded\n");
        }

        if (return_code = set_termiox(&cus_dev, &built_termiox,attrib_list)) {
            DEBUG_0("cfgtty: Build of termiox failed\n");
            err_exit(return_code);
        }
        else {
            DEBUG_0("cfgtty: Build of termiox succeeded\n");
        }


        /* =========================== */
        /* Configuration of the driver */
        /* =========================== */
        /* Build DDS for tty depending on driver attribute of parent */

        if ((return_code = Build_DDS(&cus_dev, &cfg.ddsptr,
                    &cfg.ddslen, &built_termios, &built_termiox,
                    attrib_list)) != 0){
            /* error building dds */
            DEBUG_1("cfgtty: error building dds for %s\n",logical_name);
            err_exit(return_code);
        }


        /* ==================================================== */
        /* Now call sysconfig() to configure the driver         */
        /* ==================================================== */
        DEBUG_1("cfgtty: Initializing Device with cfg.kmid=0x%x\n", cfg.kmid);
        cfg.cmd = CFG_INIT;    
        if (sysconfig(SYS_CFGDD,&cfg,sizeof(struct cfg_dd)) == -1) {
            /* error configuring driver */
            DEBUG_1("cfgtty: errno = %d",errno);
            err_exit(E_CFGINIT);
        };

        /* ==================================================== */
        /* Autopush and configuration of useful streams modules */
        /* ==================================================== */

        DEBUG_1("cfgtty: Minor number created 0x%x\n", minorno);
        DEBUG_1("cfgtty: majorno number created 0x%x\n", majorno);
        DEBUG_1("cfgtty: Trying to push streams modules: %s\n", modules);
        if (return_code = push_modules(&cus_dev, modules, majorno, minorno)) {
            DEBUG_0("cfgtty: modules push failed.\n");
            err_undo1(&cfg, &which_dds);
            err_exit(return_code);
        }
        else {
            DEBUG_0("cfgtty: modules push succeeded.\n");
        }

        /* ===================================== */
        /*  Do anything else that needs doing    */
        /*  0 = success                          */
        /* ===================================== */

        return_code = Do_Other_Processing(&cus_dev, &par_cus_dev,&bus_cus_dev,
                                         &par_pre_dev, attrib_list); 
	if (return_code != 0) {	
            DEBUG_1("cfgtty: Do_Other_Processing failed, rc=%d\n",
                                                            return_code);
            err_undo2(&cfg, majorno, minorno, &which_dds);
            err_exit(return_code);
	}

        /* ====================================================== */
        /* Update the customized object to reflect the device's */
        /* current status. The device status field should be */
        /* changed to AVAILABLE. */
        /* ====================================================== */
        cus_dev.status = (short) AVAILABLE;
        if (cus_dev.chgstatus == MISSING) {
            cus_dev.chgstatus = SAME ;
        } 
        if ((return_code = odm_change_obj(cus_dev_class, &cus_dev)) < 0) {
            /* change object failed */
            DEBUG_1("cfgtty: update %s failed.\n", logical_name);
            err_exit(E_ODMUPDATE);
        } 
        /* ================================= */
        /* Make/Remove inittab entry for tty */
        /* ================================= */


        if (!strcmp(pre_dev.class, TTY_PDDV_CLASS)) {
                if ((return_code = getatt(attrib_list, LOGIN_ATT, &login, 's',
                                          &scratch)) != 0) {
                        DEBUG_1("update_inittab: getatt of %s attribute failed\
n",LOGIN_ATT);
                        err_undo2(&cfg, majorno, minorno, &which_dds);
                        err_exit(return_code) ;
                };


                if (return_code = set_inittab(&cus_dev, &login, ipl_phase)) {
                        DEBUG_0("cfgtty: set_inittab failed\n");
                        err_undo2(&cfg, majorno, minorno, &which_dds);
                        err_exit(return_code);
                }
        }


        
    } /* End if (cus_dev.status == (short) DEFINED) */

    /* ====================================================== */
    /* Config method is finished at this point. Terminate the */
    /* ODM, and exit with a good return code. */
    /* err_exit closes odm */
    /* ====================================================== */
    err_exit(E_OK);

} /* End main(...) */
